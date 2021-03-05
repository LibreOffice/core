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

#include <osl/diagnose.h>

#include <swrenamexnameddlg.hxx>

using namespace ::com::sun::star;

SwRenameXNamedDlg::SwRenameXNamedDlg(weld::Widget* pParent,
            uno::Reference< container::XNamed > & xN,
            uno::Reference< container::XNameAccess > & xNA )
    : GenericDialogController(pParent, "modules/swriter/ui/renameobjectdialog.ui", "RenameObjectDialog")
    , xNamed(xN)
    , xNameAccess(xNA)
    , m_xNewNameED(m_xBuilder->weld_entry("entry"))
    , m_xOk(m_xBuilder->weld_button("ok"))
{
    m_xNewNameED->connect_insert_text(LINK(this, SwRenameXNamedDlg, TextFilterHdl));

    OUString sTmp(m_xDialog->get_title());
    m_xNewNameED->set_text(xNamed->getName());
    m_xNewNameED->select_region(0, -1);
    sTmp += xNamed->getName();
    m_xDialog->set_title(sTmp);

    m_xOk->connect_clicked(LINK(this, SwRenameXNamedDlg, OkHdl));
    m_xNewNameED->connect_changed(LINK(this, SwRenameXNamedDlg, ModifyHdl));
    m_xOk->set_sensitive(false);
}

IMPL_LINK(SwRenameXNamedDlg, TextFilterHdl, OUString&, rTest, bool)
{
    rTest = m_aTextFilter.filter(rTest);
    return true;
}

IMPL_LINK_NOARG(SwRenameXNamedDlg, OkHdl, weld::Button&, void)
{
    try
    {
        xNamed->setName(m_xNewNameED->get_text());
    }
    catch (const uno::RuntimeException&)
    {
        OSL_FAIL("name wasn't changed");
    }
    m_xDialog->response(RET_OK);
}

IMPL_LINK(SwRenameXNamedDlg, ModifyHdl, weld::Entry&, rEdit, void)
{
    OUString sTmp(rEdit.get_text());

    m_xOk->set_sensitive(!sTmp.isEmpty()
                  && !xNameAccess->hasByName(sTmp)
                  && (!xSecondAccess.is() || !xSecondAccess->hasByName(sTmp))
                  && (!xThirdAccess.is() || !xThirdAccess->hasByName(sTmp))
                 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
