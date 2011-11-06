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



#ifndef ACCESSIBILITY_HELPER_FACTORY_HXX
#define ACCESSIBILITY_HELPER_FACTORY_HXX

#include <toolkit/helper/accessiblefactory.hxx>
#include <svtools/accessiblefactory.hxx>

/** this is the entry point to retrieve a factory for the toolkit-level Accessible/Contexts supplied
    by this library

    This function implements the factory function needed in toolkit
    (of type GetStandardAccComponentFactory).
*/
extern "C" void* SAL_CALL getStandardAccessibleFactory();

/** this is the entry point to retrieve a factory for the svtools-level Accessible/Contexts supplied
    by this library

    This function implements the factory function needed in svtools
    (of type GetSvtAccessibilityComponentFactory).
*/
extern "C" void* SAL_CALL getSvtAccessibilityComponentFactory();

#endif // ACCESSIBILITY_HELPER_FACTORY_HXX
