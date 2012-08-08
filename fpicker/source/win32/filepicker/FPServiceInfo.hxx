/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#ifndef _FPSERVICEINFO_HXX_
#define _FPSERVICEINFO_HXX_

// the service name is a description of a set of
// interfaces (is the same as component categories in COM)

#define TMPL2000_FILEOPEN_READONLY_VERSION_BOX_ID     1001

#define TMPL2000_FILEOPEN_LINK_PREVIEW_BOX_ID         2001

#define TMPL2000_FILEOPEN_AUTOEXT_TEMPLATE_BOX_ID     3001

#define TMPL2000_FILESAVE_AUTOEXT_PASSWORD_BOX_ID     4001

#define TMPL2000_AUTOEXT_PASSWORD_FILTEROPTION_BOX    5001

#define TMPL2000_PLAY_PUSHBUTTON                      6001

#define TMPL2000_AUTOEXT_SELECTION_BOX                7001

#define TMPL2000_FILEOPEN_LINK_PREVIEW_BOX_SIMPLE_ID  8001

#define TMPL2000_FILESAVE_AUTOEXT                     9001

// the service names
#define FILE_PICKER_SERVICE_NAME  "com.sun.star.ui.dialogs.SystemFilePicker"

// the implementation names
#define FILE_PICKER_IMPL_NAME  "com.sun.star.ui.dialogs.Win32FilePicker"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
