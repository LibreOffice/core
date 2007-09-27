/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmtatr1.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 09:24:14 $
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
#include "precompiled_sw.hxx"



#include <hintids.hxx>
#ifndef _ERRHDL_HXX //autogen
#include <errhdl.hxx>
#endif
#ifndef _FMTHBSH_HXX //autogen
#include <fmthbsh.hxx>
#endif


/*************************************************************************
|*
|*    class SwFmtHardBlank
|*
|*    Beschreibung      Dokument 1.20
|*    Ersterstellung    JP 23.11.90
|*    Letzte Aenderung  JP 20.02.91
|*
*************************************************************************/

SwFmtHardBlank::SwFmtHardBlank( sal_Unicode cCh, BOOL )
    : SfxPoolItem( RES_TXTATR_HARDBLANK ),
    cChar( cCh )
{
//  ASSERT( !bCheck || (' ' != cCh && '-' != cCh),
//          "Invalid character for the HardBlank attribute - "
//          "must be a normal unicode character" );
}

int SwFmtHardBlank::operator==( const SfxPoolItem& rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return cChar == ((SwFmtHardBlank&)rAttr).GetChar();
}

SfxPoolItem* SwFmtHardBlank::Clone( SfxItemPool* ) const
{
    return new SwFmtHardBlank( *this );
}

/*************************************************************************
|*
|*    class SwFmtSoftHyph
|*
|*    Beschreibung      Dokument 1.20
|*    Ersterstellung    JP 23.11.90
|*    Letzte Aenderung  JP 20.02.91
|*
*************************************************************************/

SwFmtSoftHyph::SwFmtSoftHyph()
    : SfxPoolItem( RES_TXTATR_SOFTHYPH )
{
}

int SwFmtSoftHyph::operator==( const SfxPoolItem& ) const
{
//  ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return TRUE;
}

SfxPoolItem* SwFmtSoftHyph::Clone( SfxItemPool* ) const
{
    return new SwFmtSoftHyph( *this );
}


