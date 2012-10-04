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

#include <dialmgr.hxx>
#include <svx/svxdlg.hxx>
#include <cuires.hrc>
#include "insrc.hxx"
#include "insrc.hrc"

bool SvxInsRowColDlg::isInsertBefore() const
{
    return !aAfterBtn.IsChecked();
}

sal_uInt16 SvxInsRowColDlg::getInsertCount() const
{
    return static_cast< sal_uInt16 >( aCountEdit.GetValue() );
}

SvxInsRowColDlg::SvxInsRowColDlg(Window* pParent, bool bCol, const rtl::OString& sHelpId )
    : ModalDialog( pParent, CUI_RES(DLG_INS_ROW_COL) ),
    aCount( this, CUI_RES( FT_COUNT ) ),
    aCountEdit( this, CUI_RES( ED_COUNT ) ),
    aInsFL( this, CUI_RES( FL_INS ) ),
    aBeforeBtn( this, CUI_RES( CB_POS_BEFORE ) ),
    aAfterBtn( this, CUI_RES( CB_POS_AFTER ) ),
    aPosFL( this, CUI_RES( FL_POS ) ),
    aRow(CUI_RESSTR(STR_ROW)),
    aCol(CUI_RESSTR(STR_COL)),
    aOKBtn( this, CUI_RES( BT_OK ) ),
    aCancelBtn( this, CUI_RES( BT_CANCEL ) ),
    aHelpBtn( this, CUI_RES( BT_HELP ) ),
    bColumn( bCol )
{
    FreeResource();
    SetText( bColumn ? aCol : aRow );
    SetHelpId( sHelpId );
}

short SvxInsRowColDlg::Execute(void)
{
    return ModalDialog::Execute();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
