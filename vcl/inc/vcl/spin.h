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



#ifndef _SV_SPIN_H
#define _SV_SPIN_H

#include <vcl/sv.h>

class Rectangle;
class OutputDevice;

// -----------------------------------------------------------------------

#define SPIN_DELAY      280
#define SPIN_SPEED      60

// -----------------------------------------------------------------------

void ImplDrawSpinButton( OutputDevice* pOutDev,
                         const Rectangle& rUpperRect,
                         const Rectangle& rLowerRect,
                         sal_Bool bUpperIn, sal_Bool bLowerIn,
                         sal_Bool bUpperEnabled = sal_True,
                         sal_Bool bLowerEnabled = sal_True,
                         sal_Bool bHorz = sal_False, sal_Bool bMirrorHorz = sal_False );

#endif  // _SV_SPIN_H
