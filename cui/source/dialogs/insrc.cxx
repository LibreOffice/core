/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cui.hxx"

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
    SetHelpId( sHelpId );
}

short SvxInsRowColDlg::Execute(void)
{
    return ModalDialog::Execute();
}



