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
#include "precompiled_sfx2.hxx"

// INCLUDE ---------------------------------------------------------------
#include <svl/style.hxx>
#ifndef GCC
#endif

#include <sfx2/newstyle.hxx>
#include "dialog.hrc"
#include "newstyle.hrc"
#include "sfx2/sfxresid.hxx"

// PRIVATE METHODES ------------------------------------------------------

IMPL_LINK( SfxNewStyleDlg, OKHdl, Control *, pControl )
{
    (void)pControl; //unused
    const String aName( aColBox.GetText() );
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

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxNewStyleDlg, ModifyHdl, ComboBox *, pBox )
{
    aOKBtn.Enable( pBox->GetText().EraseAllChars().Len() > 0 );
    return 0;
}
IMPL_LINK_INLINE_END( SfxNewStyleDlg, ModifyHdl, ComboBox *, pBox )

// CTOR / DTOR -----------------------------------------------------------

SfxNewStyleDlg::SfxNewStyleDlg( Window* pParent, SfxStyleSheetBasePool& rInPool ) :

    ModalDialog( pParent, SfxResId( DLG_NEW_STYLE_BY_EXAMPLE ) ),

    aColFL              ( this, SfxResId( FL_COL ) ),
    aColBox             ( this, SfxResId( LB_COL ) ),
    aOKBtn              ( this, SfxResId( BT_OK ) ),
    aCancelBtn          ( this, SfxResId( BT_CANCEL ) ),
    aQueryOverwriteBox  ( this, SfxResId( MSG_OVERWRITE ) ),

    rPool( rInPool )

{
    FreeResource();

    aOKBtn.SetClickHdl(LINK(this, SfxNewStyleDlg, OKHdl));
    aColBox.SetModifyHdl(LINK(this, SfxNewStyleDlg, ModifyHdl));
    aColBox.SetDoubleClickHdl(LINK(this, SfxNewStyleDlg, OKHdl));
    aColBox.SetAccessibleName(SfxResId(FL_COL));

    SfxStyleSheetBase *pStyle = rPool.First();
    while ( pStyle )
    {
        aColBox.InsertEntry(pStyle->GetName());
        pStyle = rPool.Next();
    }
}

// -----------------------------------------------------------------------

__EXPORT SfxNewStyleDlg::~SfxNewStyleDlg()
{
}

