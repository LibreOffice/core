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


#ifndef _CHART_COMMONDEFINES_HXX
#define _CHART_COMMONDEFINES_HXX

namespace chart
{

//=========================================================================
//
//=========================================================================
#define CHART_3DOBJECT_SEGMENTCOUNT ((sal_Int32)32)
#define FIXED_SIZE_FOR_3D_CHART_VOLUME (10000.0)
//There needs to be a little distance betweengrid lines and walls in 3D, otherwise the lines are partly hidden by the walls
#define GRID_TO_WALL_DISTANCE (1.0)

const double    ZDIRECTION = 1.0;
const sal_Int32 AXIS2D_TICKLENGTH = 150;//value like in old chart
const sal_Int32 AXIS2D_TICKLABELSPACING = 100;//value like in old chart


}//end namespace chart
#endif
