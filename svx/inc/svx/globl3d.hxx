/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _E3D_GLOBL3D_HXX
#define _E3D_GLOBL3D_HXX

#include <tools/solar.h>

//#define ESODEBUG
#define E3D_STREAMING

const double PrecisionLimit = 1.0e-14;

const sal_uInt32 E3dInventor = sal_uInt32('E')*0x00000001+
                           sal_uInt32('3')*0x00000100+
                           sal_uInt32('D')*0x00010000+
                           sal_uInt32('1')*0x01000000;

const sal_uInt16 E3D_SCENE_ID       = 1;
const sal_uInt16 E3D_CUBEOBJ_ID     = 2;
const sal_uInt16 E3D_SPHEREOBJ_ID   = 3;
const sal_uInt16 E3D_EXTRUDEOBJ_ID  = 4;
const sal_uInt16 E3D_LATHEOBJ_ID    = 5;
const sal_uInt16 E3D_POLYGONOBJ_ID  = 6;

#endif      // _E3D_GLOBL3D_HXX
