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



#ifndef __FRAMEWORK_TARGETS_H_
#define __FRAMEWORK_TARGETS_H_

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#include <macros/generic.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  Values for special frame search ... sTargetFrameName of findFrame() or queryDispatch() or loadComponentFromURL()
//_________________________________________________________________________________________________________________

#define SPECIALTARGET_ASCII_SELF            "_self"                             // The frame himself is searched.
#define SPECIALTARGET_ASCII_PARENT          "_parent"                           // The direct parent frame is searched.
#define SPECIALTARGET_ASCII_TOP             "_top"                              // Search at ouer parents for the first task (if any exist) or a frame without a parent.
#define SPECIALTARGET_ASCII_BLANK           "_blank"                            // Create a new task.
#define SPECIALTARGET_ASCII_DEFAULT         "_default"                          // Create a new task or recycle an existing one
#define SPECIALTARGET_ASCII_BEAMER          "_beamer"                           // special frame in hierarchy
#define SPECIALTARGET_ASCII_MENUBAR         "_menubar"                          // special target for menubars
#define SPECIALTARGET_ASCII_HELPAGENT       "_helpagent"                        // special target for the help agent window
#define SPECIALTARGET_ASCII_HELPTASK        "OFFICE_HELP_TASK"                  // special name for our help task

#define SPECIALTARGET_SELF                  DECLARE_ASCII(SPECIALTARGET_ASCII_SELF       )
#define SPECIALTARGET_PARENT                DECLARE_ASCII(SPECIALTARGET_ASCII_PARENT     )
#define SPECIALTARGET_TOP                   DECLARE_ASCII(SPECIALTARGET_ASCII_TOP        )
#define SPECIALTARGET_BLANK                 DECLARE_ASCII(SPECIALTARGET_ASCII_BLANK      )
#define SPECIALTARGET_DEFAULT               DECLARE_ASCII(SPECIALTARGET_ASCII_DEFAULT    )
#define SPECIALTARGET_BEAMER                DECLARE_ASCII(SPECIALTARGET_ASCII_BEAMER     )
#define SPECIALTARGET_MENUBAR               DECLARE_ASCII(SPECIALTARGET_ASCII_MENUBAR    )
#define SPECIALTARGET_HELPAGENT             DECLARE_ASCII(SPECIALTARGET_ASCII_HELPAGENT  )
#define SPECIALTARGET_HELPTASK              DECLARE_ASCII(SPECIALTARGET_ASCII_HELPTASK   )

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_TARGETS_H_
