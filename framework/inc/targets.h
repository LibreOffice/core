/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: targets.h,v $
 * $Revision: 1.10 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
