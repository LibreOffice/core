/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FOPServiceInfo.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:48:43 $
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
