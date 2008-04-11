/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FPServiceInfo.hxx,v $
 * $Revision: 1.4 $
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
