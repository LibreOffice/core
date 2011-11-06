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




#ifndef _FPSERVICEINFO_HXX_
#define _FPSERVICEINFO_HXX_

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

// the service name is a description of a set of
// interfaces (is the same as component categories in COM)

#define TMPL95_FILEOPEN_READONLY_VERSION_BOX_ID       1000
#define TMPL2000_FILEOPEN_READONLY_VERSION_BOX_ID     1001

#define TMPL95_FILEOPEN_LINK_PREVIEW_BOX_ID           2000
#define TMPL2000_FILEOPEN_LINK_PREVIEW_BOX_ID         2001

#define TMPL95_FILEOPEN_AUTOEXT_TEMPLATE_BOX_ID       3000
#define TMPL2000_FILEOPEN_AUTOEXT_TEMPLATE_BOX_ID     3001

#define TMPL95_FILESAVE_AUTOEXT_PASSWORD_BOX_ID       4000
#define TMPL2000_FILESAVE_AUTOEXT_PASSWORD_BOX_ID     4001

#define TMPL95_AUTOEXT_PASSWORD_FILTEROPTION_BOX      5000
#define TMPL2000_AUTOEXT_PASSWORD_FILTEROPTION_BOX    5001

#define TMPL95_PLAY_PUSHBUTTON                        6000
#define TMPL2000_PLAY_PUSHBUTTON                      6001

#define TMPL95_AUTOEXT_SELECTION_BOX                  7000
#define TMPL2000_AUTOEXT_SELECTION_BOX                7001

#define TMPL95_FILEOPEN_LINK_PREVIEW_BOX_SIMPLE_ID    8000
#define TMPL2000_FILEOPEN_LINK_PREVIEW_BOX_SIMPLE_ID  8001

#define TMPL95_FILESAVE_AUTOEXT                       9000
#define TMPL2000_FILESAVE_AUTOEXT                     9001

// the service names
#define FILE_PICKER_SERVICE_NAME  "com.sun.star.ui.dialogs.SystemFilePicker"

// the implementation names
#define FILE_PICKER_IMPL_NAME  "com.sun.star.ui.dialogs.Win32FilePicker"

// the registry key names
// a key under which this service will be registered, Format: -> "/ImplName/UNO/SERVICES/ServiceName"
//                        <     Implementation-Name    ></UNO/SERVICES/><    Service-Name           >
#define FILE_PICKER_REGKEY_NAME  "/com.sun.star.ui.dialogs.Win32FilePicker/UNO/SERVICES/com.sun.star.ui.dialogs.SystemFilePicker"

#endif
