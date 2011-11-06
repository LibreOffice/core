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



#ifndef INCLUDED_SVX_HELPERMINIMALDEPTH_HXX
#define INCLUDED_SVX_HELPERMINIMALDEPTH_HXX

#include <sal/types.h>

//////////////////////////////////////////////////////////////////////////////
// predefines

class E3dCompoundObject;
class E3dScene;

//////////////////////////////////////////////////////////////////////////////
/** support extracting the minimal depth of a 3d object in it's scene

    @param rObject
    The 3D Object from which the minimal depth needs to be calculated. The scene
    is defined by the object already

    @return
    The minimal depth of this object in unified ViewCoordinates. This is the
    Z-Coordinate of one object point in the range of [0.0 .. 1.0]. ViewCoordinates
    means the transformations (esp. rotation) of the scene are taken into account

*/
// support extracting the minimal depth of a 3d object in it's scene

double getMinimalDepthInViewCoordinates(const E3dCompoundObject& rObject);

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_SVX_HELPERMINIMALDEPTH_HXX

// eof
