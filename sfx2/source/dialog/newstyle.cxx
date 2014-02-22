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

// PRIVATE METHODES ------------------------------------------------------

IMPL_LINK( SfxNewStyleDlg, OKHdl, Control *, pControl )
{
    (void)pControl; //unused
    const OUString aName( m_pColBox->GetText() );
    SfxStyleSheetBase* pStyle = rPool.Find( aName, rPool.GetSearchFamily(), SFXSTYLEBIT_ALL );
    if ( pStyle )
    {
        if ( !pStyle->IsUserDefined() )
        {
            InfoBox( this, SfxResId( MSG_POOL_STYLE_NAME ) ).Execute();
            return 0;
        }

        if ( RET_YES == aQueryOverwriteBox.Execute() )
            EndDialog( RET_OK );
    }
    else
        EndDialog( RET_OK );

    return 0;
}



IMPL_LINK_INLINE_START( SfxNewStyleDlg, ModifyHdl, ComboBox *, pBox )
{
    m_pOKBtn->Enable( !comphelper::string::remove(pBox->GetText(), ' ').isEmpty() );
    return 0;
}
IMPL_LINK_INLINE_END( SfxNewStyleDlg, ModifyHdl, ComboBox *, pBox )

// CTOR / DTOR -----------------------------------------------------------

SfxNewStyleDlg::SfxNewStyleDlg( Window* pParent, SfxStyleSheetBasePool& rInPool ) :

    ModalDialog( pParent, "CreateStyleDialog", "sfx/ui/newstyle.ui" ),

    aQueryOverwriteBox  ( this, SfxResId( MSG_OVERWRITE ) ),

    rPool( rInPool )

{
    get(m_pColBox, "stylename");
    m_pColBox->set_width_request(m_pColBox->approximate_char_width() * 25);
    m_pColBox->set_height_request(m_pColBox->GetTextHeight() * 10);
    get(m_pOKBtn, "ok");

    m_pOKBtn->SetClickHdl(LINK(this, SfxNewStyleDlg, OKHdl));
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
