 /*************************************************************************
 *
 *  $RCSfile: impdialog.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ka $ $Date: 2002-08-22 11:43:30 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "impdialog.hxx"
#include "impdialog.hrc"

// ----------------
// - ImpPDFDialog -
// ----------------

ImpPDFDialog::ImpPDFDialog( Window* pParent, ResMgr& rResMgr, Sequence< PropertyValue >& rFilterData ) :
    ModalDialog( pParent, ResId( DLG_PDFEXPORT, &rResMgr ) ),
    maBtnOK( this, ResId( BT_OK ) ),
    maBtnCancel( this, ResId( BT_CANCEL ) ),
    maBtnHelp( this, ResId( BT_HELP ) ),
    maFlRange( this, ResId( FL_RANGE ) ),
    maRbAll( this, ResId( RB_ALL ) ),
    maRbPages( this, ResId( RB_PAGES ) ),
    maRbSelection( this, ResId( RB_SELECTION ) ),
    maEdPages( this, ResId( ED_PAGES ) ),
    maFlCompression( this, ResId( FL_COMPRESSION ) ),
    maRbScreen( this, ResId( RB_SCREEN ) ),
    maRbPrint( this, ResId( RB_PRINT ) ),
    maRbPress( this, ResId( RB_PRESS ) ),
    maConfigItem( String( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/PDF/Export/" ) ), &rFilterData )
{
    FreeResource();
    maRbPages.SetToggleHdl( LINK( this, ImpPDFDialog, TogglePagesHdl ) );
}

// -----------------------------------------------------------------------------

ImpPDFDialog::~ImpPDFDialog()
{
}

// -----------------------------------------------------------------------------

void ImpPDFDialog::Init( sal_Bool bEnableSelection )
{
    const ULONG nCompressMode = maConfigItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "CompressMode" ) ), 0 );

    maRbAll.Check();
    TogglePagesHdl( NULL );
    maRbSelection.Enable( bEnableSelection );

    switch( nCompressMode )
    {
        case( 1 ): maRbPrint.Check(); break;
        case( 2 ): maRbPress.Check(); break;

        default:
            maRbScreen.Check();
        break;
    }
}

// -----------------------------------------------------------------------------

Sequence< PropertyValue > ImpPDFDialog::GetFilterData()
{
    sal_Int32 nCompressMode;

    if( maRbPrint.IsChecked() )
        nCompressMode = 1;
    else if( maRbPress.IsChecked() )
        nCompressMode = 2;
    else
        nCompressMode = 0;

    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "CompressMode" ) ), nCompressMode );

    Sequence< PropertyValue > aRet( maConfigItem.GetFilterData() );

    aRet.realloc( aRet.getLength() + 1 );
    aRet[ aRet.getLength() - 1 ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "PageSelectionRange" ) );

    if( maRbPages.IsChecked() )
        aRet[ aRet.getLength() - 1 ].Value <<= OUString( maEdPages.GetText() );
    else if( maRbSelection.IsChecked() )
        aRet[ aRet.getLength() - 1 ].Value <<= OUString( RTL_CONSTASCII_USTRINGPARAM( "Selection" ) );

    return aRet;
}

// -----------------------------------------------------------------------------

IMPL_LINK( ImpPDFDialog, TogglePagesHdl, void*, p )
{
    maEdPages.Enable( maRbPages.IsChecked() );
    maEdPages.SetReadOnly( !maRbPages.IsChecked() );

    return 0;
}
