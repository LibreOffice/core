/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fmtatr1.cxx,v $
 * $Revision: 1.6 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#include <hintids.hxx>
#include <errhdl.hxx>
#include <fmthbsh.hxx>


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


