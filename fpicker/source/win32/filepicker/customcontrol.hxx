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



#ifndef _CUSTOMCONTROL_HXX_
#define _CUSTOMCONTROL_HXX_

#include <sal/types.h>

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

//-----------------------------------
//
//-----------------------------------

class CCustomControl
{
public:
    virtual ~CCustomControl();

    // align the control to a reference object/control
    virtual void SAL_CALL Align() = 0;

    virtual void SAL_CALL SetFont(HFONT hFont) = 0;

    virtual void SAL_CALL AddControl(CCustomControl* aCustomControl);
    virtual void SAL_CALL RemoveControl(CCustomControl* aCustomControl);
    virtual void SAL_CALL RemoveAllControls();
};

#endif
