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

#include <swtypes.hxx>
#include <globals.hrc>

#include <utlui.hrc>
#include <../../uibase/utlui/unotools.hrc>
#include <unoprnms.hxx>
#include <osl/diagnose.h>
#include <vcl/msgbox.hxx>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/view/XScreenCursor.hpp>
#include <com/sun/star/view/DocumentZoomType.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>
#include <shellio.hxx>
#include <docsh.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <swmodule.hxx>
#include <unocrsr.hxx>

#include "swrenamexnameddlg.hxx"

using namespace ::com::sun::star;

SwRenameXNamedDlg::SwRenameXNamedDlg( vcl::Window* pWin,
            uno::Reference< container::XNamed > & xN,
            uno::Reference< container::XNameAccess > & xNA )
    : ModalDialog(pWin, "RenameObjectDialog",
        "modules/swriter/ui/renameobjectdialog.ui")
    , xNamed(xN)
    , xNameAccess(xNA)
{
    get(m_pNewNameED, "entry");
    m_pNewNameED->SetTextFilter(&m_aTextFilter);
    get(m_pOk, "ok");

    OUString sTmp(GetText());
    m_pNewNameED->SetText(xNamed->getName());
    m_pNewNameED->SetSelection(Selection(SELECTION_MIN, SELECTION_MAX));
    sTmp += xNamed->getName();
    SetText(sTmp);

    m_pOk->SetClickHdl(LINK(this, SwRenameXNamedDlg, OkHdl));
    m_pNewNameED->SetModifyHdl(LINK(this, SwRenameXNamedDlg, ModifyHdl));
    m_pOk->Enable(false);
}

SwRenameXNamedDlg::~SwRenameXNamedDlg()
{
    disposeOnce();
}

void SwRenameXNamedDlg::dispose()
{
    m_pNewNameED.clear();
    m_pOk.clear();
    ModalDialog::dispose();
}

IMPL_LINK_NOARG_TYPED(SwRenameXNamedDlg, OkHdl, Button*, void)
{
    try
    {
        xNamed->setName(m_pNewNameED->GetText());
    }
    catch (const uno::RuntimeException&)
    {
        OSL_FAIL("name wasn't changed");
    }
    EndDialog(RET_OK);
}

IMPL_LINK_TYPED(SwRenameXNamedDlg, ModifyHdl, Edit&, rEdit, void)
{
    OUString sTmp(rEdit.GetText());

    m_pOk->Enable(!sTmp.isEmpty()
                  && !xNameAccess->hasByName(sTmp)
                  && (!xSecondAccess.is() || !xSecondAccess->hasByName(sTmp))
                  && (!xThirdAccess.is() || !xThirdAccess->hasByName(sTmp))
                 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
