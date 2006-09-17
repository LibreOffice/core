/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: newstyle.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 16:34:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

// INCLUDE ---------------------------------------------------------------

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef GCC
#endif

#include "newstyle.hxx"
#include "dialog.hrc"
#include "newstyle.hrc"
#include "sfxresid.hxx"

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

    aColFL              ( this, ResId( FL_COL ) ),
    aColBox             ( this, ResId( LB_COL ) ),
    aOKBtn              ( this, ResId( BT_OK ) ),
    aCancelBtn          ( this, ResId( BT_CANCEL ) ),
    aQueryOverwriteBox  ( this, ResId( MSG_OVERWRITE ) ),

    rPool( rInPool )

{
    FreeResource();

    aOKBtn.SetClickHdl(LINK(this, SfxNewStyleDlg, OKHdl));
    aColBox.SetModifyHdl(LINK(this, SfxNewStyleDlg, ModifyHdl));
    aColBox.SetDoubleClickHdl(LINK(this, SfxNewStyleDlg, OKHdl));

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

