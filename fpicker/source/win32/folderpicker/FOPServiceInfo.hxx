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




#ifndef _FOPSERVICEINFO_HXX_
#define _FOPSERVICEINFO_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

// the service name is a description of a set of
// interfaces (is the same as component categories in COM)

// the service names
#define FOLDER_PICKER_SERVICE_NAME  "com.sun.star.ui.dialogs.SystemFolderPicker"

// the implementation names
#define FOLDER_PICKER_IMPL_NAME  "com.sun.star.ui.dialogs.Win32FolderPicker"

// the registry key names
// a key under which this service will be registered, Format: -> "/ImplName/UNO/SERVICES/ServiceName"
//                        <     Implementation-Name    ></UNO/SERVICES/><    Service-Name           >
#define FOLDER_PICKER_REGKEY_NAME  "/com.sun.star.ui.dialogs.Win32FolderPicker/UNO/SERVICES/com.sun.star.ui.dialogs.SystemFolderPicker"

#endif
