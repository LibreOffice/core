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

#ifndef GENERAL_DEFINES_HXX
#define GENERAL_DEFINES_HXX

#include "macros.hxx"

#ifndef STATUSBAR_CONTROLLER_IMPL_NAME
#error  You must define STATUSBAR_CONTROLLER_IMPL_NAME
#endif

#ifndef STATUSBAR_CONTROLLER_SERVICE_NAME
#error  You must define STATUSBAR_CONTROLLER_SERVICE_NAME
#endif

#ifndef EXTENSION_IDENTIFIER
#error  You must define EXTENSION_IDENTIFIER
#endif

#define OUSTR_COMMAND_UNO_MODIFIED_STATUS           C2U(".uno:ModifiedStatus")
#define OUSTR_COMMAND_UNO_SAVE                      C2U(".uno:Save")

#define OUSTR_SERVICENAME_GRAPHICPROVIDER           C2U("com.sun.star.graphic.GraphicProvider")
#define OUSTR_SERVICENAME_URLTRANSFORMER            C2U("com.sun.star.util.URLTransformer")

#endif
