 /*************************************************************************
 *
 *  $RCSfile: impdialog.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 10:28:43 $
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

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGE_HPP_
#include <com/sun/star/text/XTextRange.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif

// ----------------
// - ImpPDFDialog -
// ----------------

using namespace ::com::sun::star;

ImpPDFDialog::ImpPDFDialog( Window* pParent, ResMgr& rResMgr, Sequence< PropertyValue >& rFilterData, const Any& rSelection ) :
    ModalDialog( pParent, ResId( RID_PDF_OLD_EXPORT_DLG, &rResMgr ) ),
    maBtnOK( this, ResId( BT_OLD_OK ) ),
    maBtnCancel( this, ResId( BT_OLD_CANCEL ) ),
    maBtnHelp( this, ResId( BT_OLD_HELP ) ),
    maFlPages( this, ResId( FL_OLD_PAGES ) ),
    maRbAll( this, ResId( RB_OLD_ALL ) ),
    maRbRange( this, ResId( RB_OLD_RANGE ) ),
    maRbSelection( this, ResId( RB_OLD_SELECTION ) ),
    maEdPages( this, ResId( ED_OLD_PAGES ) ),
    maFlCompression( this, ResId( FL_OLD_COMPRESSION ) ),
    maRbScreen( this, ResId( RB_OLD_SCREEN ) ),
    maRbPrint( this, ResId( RB_OLD_PRINT ) ),
    maRbPress( this, ResId( RB_OLD_PRESS ) ),
    maConfigItem( String( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/PDF/Export/" ) ), &rFilterData ),
    maSelection( rSelection )
{
    const ULONG nCompressMode = maConfigItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "CompressMode" ) ), 1 );

    FreeResource();
    maRbRange.SetToggleHdl( LINK( this, ImpPDFDialog, TogglePagesHdl ) );

    maRbAll.Check();
    TogglePagesHdl( NULL );

    sal_Bool bHasSelection = maSelection.hasValue();
    if ( bHasSelection )
    {
        Reference< drawing::XShapes > xShapes;
        if ( ( maSelection >>= xShapes ) == sal_False ) // XShapes is always a selection
        {
            // even if nothing is selected in writer the selection is not empty
            Reference< container::XIndexAccess > xIndexAccess;
            if ( maSelection >>= xIndexAccess )
            {
                sal_Int32 nLen = xIndexAccess->getCount();
                if ( !nLen )
                    bHasSelection = sal_False;
                else if ( nLen == 1 )
                {
                    Reference< text::XTextRange > xTextRange( xIndexAccess->getByIndex( 0 ), UNO_QUERY );
                    if ( xTextRange.is() && ( xTextRange->getString().getLength() == 0 ) )
                        bHasSelection = sal_False;
                }
            }
        }
    }
    maRbSelection.Enable( bHasSelection );

    switch( nCompressMode )
    {
        case( 0 ): maRbScreen.Check(); break;
        case( 2 ): maRbPress.Check(); break;

        default:
            maRbPrint.Check();
        break;
    }
}

// -----------------------------------------------------------------------------

ImpPDFDialog::~ImpPDFDialog()
{
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

    if( maRbRange.IsChecked() )
    {
        aRet[ aRet.getLength() - 1 ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "PageRange" ) );
        aRet[ aRet.getLength() - 1 ].Value <<= OUString( maEdPages.GetText() );
    }
    else if( maRbSelection.IsChecked() )
    {
        aRet[ aRet.getLength() - 1 ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Selection" ) );
        aRet[ aRet.getLength() - 1 ].Value <<= maSelection;
    }

    return aRet;
}

// -----------------------------------------------------------------------------

IMPL_LINK( ImpPDFDialog, TogglePagesHdl, void*, p )
{
    maEdPages.Enable( maRbRange.IsChecked() );
    maEdPages.SetReadOnly( !maRbRange.IsChecked() );

    return 0;
}
