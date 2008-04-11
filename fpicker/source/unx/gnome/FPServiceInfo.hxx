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

// the service names
#define FILE_PICKER_SERVICE_NAME "com.sun.star.ui.dialogs.GtkFilePicker"
#define FOLDER_PICKER_SERVICE_NAME "com.sun.star.ui.dialogs.GtkFolderPicker"

// the implementation names
#define FILE_PICKER_IMPL_NAME    "com.sun.star.ui.dialogs.SalGtkFilePicker"
#define FOLDER_PICKER_IMPL_NAME    "com.sun.star.ui.dialogs.SalGtkFolderPicker"

// the registry key names
// a key under which this service will be registered,
// Format: -> "/ImplName/UNO/SERVICES/ServiceName"
//  <Implementation-Name></UNO/SERVICES/><Service-Name>
#define FILE_PICKER_REGKEY_NAME  "/com.sun.star.ui.dialogs.SalGtkFilePicker/UNO/SERVICES/com.sun.star.ui.dialogs.GtkFilePicker"
#define FOLDER_PICKER_REGKEY_NAME  "/com.sun.star.ui.dialogs.SalGtkFolderPicker/UNO/SERVICES/com.sun.star.ui.dialogs.GtkFolderPicker"

#endif
