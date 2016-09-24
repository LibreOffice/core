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

#include <comphelper/string.hxx>

#include <svl/style.hxx>

#include <sfx2/newstyle.hxx>
#include "dialog.hrc"
#include <sfx2/sfxresid.hxx>
#include <vcl/layout.hxx>

// PRIVATE METHODES ------------------------------------------------------

IMPL_LINK_NOARG_TYPED( SfxNewStyleDlg, OKClickHdl, Button*, void )
{
    OKHdl(*m_pColBox);
}
IMPL_LINK_NOARG_TYPED( SfxNewStyleDlg, OKHdl, ComboBox&, void )
{
    const OUString aName( m_pColBox->GetText() );
    SfxStyleSheetBase* pStyle = rPool.Find( aName, rPool.GetSearchFamily() );
    if ( pStyle )
    {
        if ( !pStyle->IsUserDefined() )
        {
            ScopedVclPtrInstance<MessageDialog>( this, SfxResId( STR_POOL_STYLE_NAME ), VclMessageType::Info )->Execute();
            return;
        }

        if ( RET_YES == aQueryOverwriteBox->Execute() )
            EndDialog( RET_OK );
    }
    else
        EndDialog( RET_OK );
}

IMPL_LINK_TYPED( SfxNewStyleDlg, ModifyHdl, Edit&, rBox, void )
{
    m_pOKBtn->Enable( !rBox.GetText().replaceAll(" ", "").isEmpty() );
}

SfxNewStyleDlg::SfxNewStyleDlg( vcl::Window* pParent, SfxStyleSheetBasePool& rInPool )
    : ModalDialog(pParent, "CreateStyleDialog", "sfx/ui/newstyle.ui")
    , aQueryOverwriteBox(VclPtr<MessageDialog>::Create(this, SfxResId(STR_QUERY_OVERWRITE),
                                           VclMessageType::Question, VCL_BUTTONS_YES_NO))
    , rPool(rInPool)
{
    get(m_pColBox, "stylename");
    m_pColBox->set_width_request(m_pColBox->approximate_char_width() * 25);
    m_pColBox->set_height_request(m_pColBox->GetTextHeight() * 10);
    get(m_pOKBtn, "ok");

    m_pOKBtn->SetClickHdl(LINK(this, SfxNewStyleDlg, OKClickHdl));
    m_pColBox->SetModifyHdl(LINK(this, SfxNewStyleDlg, ModifyHdl));
    m_pColBox->SetDoubleClickHdl(LINK(this, SfxNewStyleDlg, OKHdl));

    SfxStyleSheetBase *pStyle = rPool.First();
    while ( pStyle )
    {
        m_pColBox->InsertEntry(pStyle->GetName());
        pStyle = rPool.Next();
    }
}

SfxNewStyleDlg::~SfxNewStyleDlg()
{
    disposeOnce();
}

void SfxNewStyleDlg::dispose()
{
    aQueryOverwriteBox.disposeAndClear();
    m_pColBox.clear();
    m_pOKBtn.clear();
    ModalDialog::dispose();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
