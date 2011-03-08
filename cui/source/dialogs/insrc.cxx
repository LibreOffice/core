/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

SvxInsRowColDlg::SvxInsRowColDlg(Window* pParent, bool bCol, ULONG nHelpId )
    : ModalDialog( pParent, CUI_RES(DLG_INS_ROW_COL) ),
    aCount( this, CUI_RES( FT_COUNT ) ),
    aCountEdit( this, CUI_RES( ED_COUNT ) ),
    aInsFL( this, CUI_RES( FL_INS ) ),
    aBeforeBtn( this, CUI_RES( CB_POS_BEFORE ) ),
    aAfterBtn( this, CUI_RES( CB_POS_AFTER ) ),
    aPosFL( this, CUI_RES( FL_POS ) ),
    aRow(CUI_RES(STR_ROW)),
    aCol(CUI_RES(STR_COL)),
    aOKBtn( this, CUI_RES( BT_OK ) ),
    aCancelBtn( this, CUI_RES( BT_CANCEL ) ),
    aHelpBtn( this, CUI_RES( BT_HELP ) ),
    bColumn( bCol )
{
    FreeResource();
    String aTmp( GetText() );
    if( bColumn )
    {
        aTmp += aCol;
    }
    else
    {
        aTmp += aRow;
    }
    SetText( aTmp );
    SetHelpId( nHelpId );
}

short SvxInsRowColDlg::Execute(void)
{
    return ModalDialog::Execute();
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
