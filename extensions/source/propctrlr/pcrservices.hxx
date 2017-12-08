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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PCRSERVICES_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PCRSERVICES_HXX

#include <sal/config.h>

#include <sal/types.h>

extern "C" {

void createRegistryInfo_ButtonNavigationHandler();
void createRegistryInfo_CellBindingPropertyHandler();
void createRegistryInfo_DefaultFormComponentInspectorModel();
void createRegistryInfo_DefaultHelpProvider();
void createRegistryInfo_EFormsPropertyHandler();
void createRegistryInfo_EditPropertyHandler();
void createRegistryInfo_EventHandler();
void createRegistryInfo_FormComponentPropertyHandler();
void createRegistryInfo_FormController();
void createRegistryInfo_FormGeometryHandler();
void createRegistryInfo_GenericPropertyHandler();
void createRegistryInfo_MasterDetailLinkDialog();
void createRegistryInfo_OControlFontDialog();
void createRegistryInfo_OPropertyBrowserController();
void createRegistryInfo_OTabOrderDialog();
void createRegistryInfo_ObjectInspectorModel();
void createRegistryInfo_StringRepresentation();
void createRegistryInfo_SubmissionPropertyHandler();
void createRegistryInfo_XSDValidationPropertyHandler();

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
