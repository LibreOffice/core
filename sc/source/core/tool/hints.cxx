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

#include "hints.hxx"

// -----------------------------------------------------------------------

TYPEINIT1(ScPaintHint, SfxHint);
TYPEINIT1(ScUpdateRefHint, SfxHint);
TYPEINIT1(ScPointerChangedHint, SfxHint);
TYPEINIT1(ScLinkRefreshedHint, SfxHint);
TYPEINIT1(ScAutoStyleHint, SfxHint);
TYPEINIT1(ScDBRangeRefreshedHint, SfxHint);
TYPEINIT1(ScDataPilotModifiedHint, SfxHint);

// -----------------------------------------------------------------------
// ScPaintHint - info what has to be repainted
// -----------------------------------------------------------------------

ScPaintHint::ScPaintHint( const ScRange& rRng, sal_uInt16 nPaint ) :
    aRange( rRng ),
    nParts( nPaint ),
    bPrint( sal_True )
{
}

ScPaintHint::~ScPaintHint()
{
}

// -----------------------------------------------------------------------
// ScUpdateRefHint - update references
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
// ScPointerChangedHint - pointer has become invalid
// -----------------------------------------------------------------------


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
            const String& rA, const String& rT, const String& rI, sal_uInt8 nM )
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
                                        sal_uLong nT, const String& rSt2 ) :
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


ScDataPilotModifiedHint::ScDataPilotModifiedHint( const String& rName )
    : maName(rName)
{
}
ScDataPilotModifiedHint::~ScDataPilotModifiedHint()
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
