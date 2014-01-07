/*
 * Software License Agreement (Apache License)
 *
 * Copyright (c) 2013, Southwest Research Institute
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <industrial_extrinsic_cal/ceres_blocks.h>
#include <boost/shared_ptr.hpp>

using std::string;
using boost::shared_ptr;

namespace industrial_extrinsic_cal
{
CeresBlocks::CeresBlocks()
{
}
CeresBlocks::~CeresBlocks()
{
  clearCamerasTargets();
}
void CeresBlocks::clearCamerasTargets()
{
  static_cameras_.clear();
  static_targets_.clear();
  moving_cameras_.clear();
  moving_targets_.clear();
}
P_BLOCK CeresBlocks::getStaticCameraParameterBlockIntrinsics(string camera_name)
{
  // static cameras should have unique name
  BOOST_FOREACH(shared_ptr<Camera> camera, static_cameras_)
  {
    if (camera_name == camera->camera_name_)
    {
      P_BLOCK intrinsics = &(camera->camera_parameters_.pb_intrinsics[0]);
      return (intrinsics);
    }
  }
  return (NULL);
}
P_BLOCK CeresBlocks::getMovingCameraParameterBlockIntrinsics(string camera_name)
{
  // we use the intrinsic parameters from the first time the camera appears in the list
  // subsequent cameras with this name also have intrinsic parameters, but these are
  // never used as parameter blocks, only their extrinsics are used
  BOOST_FOREACH(shared_ptr<MovingCamera> moving_camera, moving_cameras_)
  {
    if (camera_name == moving_camera->cam->camera_name_)
    {
      P_BLOCK intrinsics = &(moving_camera->cam->camera_parameters_.pb_intrinsics[0]);
      return (intrinsics);
    }
  }
  return (NULL);
}
P_BLOCK CeresBlocks::getStaticCameraParameterBlockExtrinsics(string camera_name)
{
  // static cameras should have unique name
  BOOST_FOREACH(shared_ptr<Camera> camera, static_cameras_)
  {
    if (camera_name == camera->camera_name_)
    {
      P_BLOCK extrinsics = &(camera->camera_parameters_.pb_extrinsics[0]);
      return (extrinsics);
    }
  }
  return (NULL);

}
P_BLOCK CeresBlocks::getMovingCameraParameterBlockExtrinsics(string camera_name, int scene_id)
{
  BOOST_FOREACH(shared_ptr<MovingCamera> camera, moving_cameras_)
  {
    if (camera_name == camera->cam->camera_name_ && scene_id == camera->scene_id)
    {
      P_BLOCK extrinsics = &(camera->cam->camera_parameters_.pb_extrinsics[0]);
      return (extrinsics);
    }
  }
  return (NULL);

}
P_BLOCK CeresBlocks::getStaticTargetPoseParameterBlock(string target_name)
{
  BOOST_FOREACH(shared_ptr<Target> target, static_targets_)
  {
    if (target_name == target->target_name)
    {
      P_BLOCK pose = &(target->pose.pb_pose[0]);
      return (pose);
    }
  }
  return (NULL);
}
P_BLOCK CeresBlocks::getStaticTargetPointParameterBlock(string target_name, int point_id)
{
  BOOST_FOREACH(shared_ptr<Target> target, static_targets_)
  {
    if (target_name == target->target_name)
    {
      P_BLOCK point_position = &(target->pts[point_id].pb[0]);
      return (point_position);
    }
  }
  return (NULL);
}
P_BLOCK CeresBlocks::getMovingTargetPoseParameterBlock(string target_name, int scene_id)
{
  BOOST_FOREACH(shared_ptr<MovingTarget> moving_target, moving_targets_)
  {
    if (target_name == moving_target->targ->target_name && scene_id == moving_target->scene_id)
    {
      P_BLOCK pose = &(moving_target->targ->pose.pb_pose[0]);
      return (pose);
    }
  }
  return (NULL);
}
P_BLOCK CeresBlocks::getMovingTargetPointParameterBlock(string target_name, int pnt_id)
{
  // note scene_id unnecessary here since regarless of scene th point's location relative to
  // the target frame does not change
  BOOST_FOREACH(shared_ptr<MovingTarget> moving_target, moving_targets_)
  {
    if (target_name == moving_target->targ->target_name)
    {
      P_BLOCK point_position = &(moving_target->targ->pts[pnt_id].pb[0]);
      return (point_position);
    }
  }
  return (NULL);
}

bool CeresBlocks::addStaticCamera(shared_ptr<Camera> camera_to_add)
{
  BOOST_FOREACH(shared_ptr<Camera> cam, static_cameras_)
  {
    if (cam->camera_name_ == camera_to_add->camera_name_)
      return (false); // camera already exists
  }
  static_cameras_.push_back(camera_to_add);
  return (true);
}
bool CeresBlocks::addStaticTarget(shared_ptr<Target> target_to_add)
{
  BOOST_FOREACH(shared_ptr<Target> targ, static_targets_)
  {
    if (targ->target_name == target_to_add->target_name)
      return (false); // target already exists
  }
  static_targets_.push_back(target_to_add);
  return (true);
}
bool CeresBlocks::addMovingCamera(shared_ptr<Camera> camera_to_add, int scene_id)
{
  BOOST_FOREACH(shared_ptr<MovingCamera> cam, moving_cameras_)
  {
    if (cam->cam->camera_name_ == camera_to_add->camera_name_ && cam->scene_id == scene_id)
      return (false); // camera already exists
  }
  // this next line allocates the memory for a moving camera
  shared_ptr<MovingCamera> temp_moving_camera = boost::make_shared<MovingCamera>();
  // this next line allocates the memory for the actual camera
  shared_ptr<Camera> temp_camera = boost::make_shared<Camera>(camera_to_add->camera_name_, camera_to_add->camera_parameters_,
                                                       true);
  temp_moving_camera->cam = temp_camera;
  temp_moving_camera->scene_id = scene_id;
  moving_cameras_.push_back(temp_moving_camera);
  return (true);
}
bool CeresBlocks::addMovingTarget(shared_ptr<Target> target_to_add, int scene_id)
{
  BOOST_FOREACH(shared_ptr<MovingTarget> targ, moving_targets_)
  {
    if (targ->targ->target_name == target_to_add->target_name && targ->scene_id == scene_id)
      return (false); // target already exists
  }
  shared_ptr<MovingTarget> temp_moving_target = boost::make_shared<MovingTarget>();
  shared_ptr<Target> temp_camera = boost::make_shared<Target>();
  temp_moving_target->targ = target_to_add;
  temp_moving_target->scene_id = scene_id;
  moving_targets_.push_back(temp_moving_target);
  return (true);
}


}// end namespace industrial_extrinsic_cal


