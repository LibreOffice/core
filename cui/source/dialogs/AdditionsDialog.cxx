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
#include <AdditionsDialog.hxx>

using namespace css::uno;

using namespace css::frame;

AdditionsDialog::AdditionsDialog(weld::Widget* pParent, Reference<XModel> xModel,
                                 bool bEditExisting)
    : GenericDialogController(pParent, "cui/ui/additionsdialog.ui", "AdditionsDialog")
    , m_xInstallButton(m_xBuilder->weld_button("btnInstall"))
    , m_xExtensionName(m_xBuilder->weld_label("labelExtensionName"))
    , m_xAuthor(m_xBuilder->weld_label("labelAuthor"))
    , m_xIntro(m_xBuilder->weld_label("labelIntro"))
{
    m_xAuthor->set_label("Additions Dialog Test");
    m_xInstallButton->connect_clicked(LINK(this, AdditionsDialog, InstallButtonTestHdl));
      
    if (!bEditExisting)
    {
          m_xIntro->setlabel("bEditExisting is false!");
    }  
}

AdditionsDialog::~AdditionsDialog() {}

IMPL_LINK_NOARG(AdditionsDialog, InstallButtonTestHdl, weld::Button&, void)
{
    m_xExtensionName->set_label("Additional Dialog Test is successfully completed!");
}
