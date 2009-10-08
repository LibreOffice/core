/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: hints.cxx,v $
 * $Revision: 1.6.32.2 $
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
#include "precompiled_sc.hxx"



#include "hints.hxx"

// -----------------------------------------------------------------------

TYPEINIT1(ScPaintHint, SfxHint);
TYPEINIT1(ScUpdateRefHint, SfxHint);
TYPEINIT1(ScPointerChangedHint, SfxHint);
TYPEINIT1(ScLinkRefreshedHint, SfxHint);
TYPEINIT1(ScAutoStyleHint, SfxHint);
TYPEINIT1(ScDBRangeRefreshedHint, SfxHint);

// -----------------------------------------------------------------------
//      ScPaintHint - Angabe, was neu gezeichnet werden muss
// -----------------------------------------------------------------------

ScPaintHint::ScPaintHint( const ScRange& rRng, USHORT nPaint ) :
    aRange( rRng ),
    nParts( nPaint ),
    bPrint( TRUE )
{
}

ScPaintHint::~ScPaintHint()
{
}

// -----------------------------------------------------------------------
//      ScUpdateRefHint - Referenz-Updaterei
// -----------------------------------------------------------------------

ScUpdateRefHint::ScUpdateRefHint( UpdateRefMode eMode, const ScRange& rR,
                                    SCsCOL nX, SCsROW nY, SCsTAB nZ ) :
    eUpdateRefMode( eMode ),
    aRange( rR ),
    nDx( nX ),
    nDy( nY ),
    nDz( nZ )
{
}

ScUpdateRefHint::~ScUpdateRefHint()
{
}

// -----------------------------------------------------------------------
//      ScPointerChangedHint - Pointer ist ungueltig geworden
// -----------------------------------------------------------------------

//UNUSED2008-05  ScPointerChangedHint::ScPointerChangedHint( USHORT nF ) :
//UNUSED2008-05      nFlags( nF )
//UNUSED2008-05  {
//UNUSED2008-05  }

ScPointerChangedHint::~ScPointerChangedHint()
{
}

// -----------------------------------------------------------------------
//      ScLinkRefreshedHint - a link has been refreshed
// -----------------------------------------------------------------------

ScLinkRefreshedHint::ScLinkRefreshedHint() :
    nLinkType( SC_LINKREFTYPE_NONE ),
    nDdeMode( 0 )
{
}

ScLinkRefreshedHint::~ScLinkRefreshedHint()
{
}

void ScLinkRefreshedHint::SetSheetLink( const String& rSourceUrl )
{
    nLinkType = SC_LINKREFTYPE_SHEET;
    aUrl = rSourceUrl;
}

void ScLinkRefreshedHint::SetDdeLink(
            const String& rA, const String& rT, const String& rI, BYTE nM )
{
    nLinkType = SC_LINKREFTYPE_DDE;
    aDdeAppl  = rA;
    aDdeTopic = rT;
    aDdeItem  = rI;
    nDdeMode  = nM;
}

void ScLinkRefreshedHint::SetAreaLink( const ScAddress& rPos )
{
    nLinkType = SC_LINKREFTYPE_AREA;
    aDestPos = rPos;
}

// -----------------------------------------------------------------------
//      ScAutoStyleHint - STYLE() function has been called
// -----------------------------------------------------------------------

ScAutoStyleHint::ScAutoStyleHint( const ScRange& rR, const String& rSt1,
                                        ULONG nT, const String& rSt2 ) :
    aRange( rR ),
    aStyle1( rSt1 ),
    aStyle2( rSt2 ),
    nTimeout( nT )
{
}

ScAutoStyleHint::~ScAutoStyleHint()
{
}


ScDBRangeRefreshedHint::ScDBRangeRefreshedHint( const ScImportParam& rP )
    : aParam(rP)
{
}
ScDBRangeRefreshedHint::~ScDBRangeRefreshedHint()
{
}


