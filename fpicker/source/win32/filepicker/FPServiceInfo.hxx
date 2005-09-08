/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FPServiceInfo.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:37:18 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
