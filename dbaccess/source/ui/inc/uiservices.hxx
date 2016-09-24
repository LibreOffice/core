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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_UISERVICES_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_UISERVICES_HXX

#include <sal/config.h>

#include <sal/types.h>

extern "C" {

void SAL_CALL createRegistryInfo_ComposerDialogs();
void SAL_CALL createRegistryInfo_CopyTableWizard();
void SAL_CALL createRegistryInfo_DBContentLoader();
void SAL_CALL createRegistryInfo_LimitBoxController();
void SAL_CALL createRegistryInfo_OAdvancedSettingsDialog();
void SAL_CALL createRegistryInfo_OBrowser();
void SAL_CALL createRegistryInfo_OColumnControl();
void SAL_CALL createRegistryInfo_OColumnControlModel();
void SAL_CALL createRegistryInfo_ODBApplication();
void SAL_CALL createRegistryInfo_ODBTypeWizDialog();
void SAL_CALL createRegistryInfo_ODBTypeWizDialogSetup();
void SAL_CALL createRegistryInfo_ODataSourcePropertyDialog();
void SAL_CALL createRegistryInfo_ODirectSQLDialog();
void SAL_CALL createRegistryInfo_OFormGridView();
void SAL_CALL createRegistryInfo_OInteractionHandler();
void SAL_CALL createRegistryInfo_OQueryControl();
void SAL_CALL createRegistryInfo_ORelationControl();
void SAL_CALL createRegistryInfo_OSQLMessageDialog();
void SAL_CALL createRegistryInfo_OTableControl();
void SAL_CALL createRegistryInfo_OTableFilterDialog();
void SAL_CALL createRegistryInfo_OTextConnectionSettingsDialog();
void SAL_CALL createRegistryInfo_OToolboxController();
void SAL_CALL createRegistryInfo_OUserSettingsDialog();
void SAL_CALL createRegistryInfo_OViewControl();
void SAL_CALL createRegistryInfo_SbaXGridControl();

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
