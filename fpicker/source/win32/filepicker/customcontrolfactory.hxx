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



#ifndef _CUSTOMCONTROLFACTORY_HXX_
#define _CUSTOMCONTROLFACTORY_HXX_

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

//-----------------------------------
// forward declaration
//-----------------------------------

class CCustomControl;

//-----------------------------------
//
//-----------------------------------

class CCustomControlFactory
{
public:

    // The CCustomControl instances will be created on the heap
    // and the client is responsible for deleting this instances
    // (he adopts ownership)
    virtual CCustomControl* CreateCustomControl(HWND aControlHandle, HWND aParentHandle);

    virtual CCustomControl* CreateCustomControlContainer();
};

#endif
