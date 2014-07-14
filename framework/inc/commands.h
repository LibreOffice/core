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



#ifndef __FRAMEWORK_COMMANDS_H_
#define __FRAMEWORK_COMMANDS_H_

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#include <macros/generic.hxx>
#include <vos/process.hxx>
#include <rtl/ustring.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

/*-************************************************************************************************************//**
    @short          These values describe some command line arguments of our office.
                    We need it e.g. to detect running mode of it - plugin, headless ...
*//*-*************************************************************************************************************/

#define COMMAND_PLUGIN    DECLARE_ASCII("-plugin"     ) /// office is plugged into a browser
#define COMMAND_HEADLESS  DECLARE_ASCII("-headless"   ) /// office runs in headless mode for scripting
#define COMMAND_INVISIBLE DECLARE_ASCII("-invisible"  ) /// office runs in invisible mode which suppress splash screen and first empry document
#define COMMAND_SERVER    DECLARE_ASCII("-server"     ) /// office runs as server
}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_TARGETS_H_
