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

#include <hints.hxx>
#include <utility>

// ScPaintHint - info what has to be repainted

ScPaintHint::ScPaintHint( const ScRange& rRng, PaintPartFlags nPaint, tools::Long nMaxWidthAffectedHint ) :
    SfxHint( SfxHintId::ScPaint ),
    aRange( rRng ),
    nParts( nPaint ),
    nWidthAffectedHint(nMaxWidthAffectedHint)
{
}

ScPaintHint::~ScPaintHint()
{
}

// ScUpdateRefHint - update references

ScUpdateRefHint::ScUpdateRefHint( UpdateRefMode eMode, const ScRange& rR,
                                    SCCOL nX, SCROW nY, SCTAB nZ ) :
    SfxHint(SfxHintId::ScUpdateRef),
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

//      ScLinkRefreshedHint - a link has been refreshed

ScLinkRefreshedHint::ScLinkRefreshedHint() :
    SfxHint(SfxHintId::ScLinkRefreshed), nLinkType( ScLinkRefType::NONE )
{
}

ScLinkRefreshedHint::~ScLinkRefreshedHint()
{
}

void ScLinkRefreshedHint::SetSheetLink( const OUString& rSourceUrl )
{
    nLinkType = ScLinkRefType::SHEET;
    aUrl = rSourceUrl;
}

void ScLinkRefreshedHint::SetDdeLink(
            const OUString& rA, const OUString& rT, const OUString& rI )
{
    nLinkType = ScLinkRefType::DDE;
    aDdeAppl  = rA;
    aDdeTopic = rT;
    aDdeItem  = rI;
}

void ScLinkRefreshedHint::SetAreaLink( const ScAddress& rPos )
{
    nLinkType = ScLinkRefType::AREA;
    aDestPos = rPos;
}

//      ScAutoStyleHint - STYLE() function has been called

ScAutoStyleHint::ScAutoStyleHint( const ScRange& rR, OUString aSt1,
                                        sal_uLong nT, OUString aSt2 ) :
    SfxHint(SfxHintId::ScAutoStyle),
    aRange( rR ),
    aStyle1(std::move( aSt1 )),
    aStyle2(std::move( aSt2 )),
    nTimeout( nT )
{
}

ScAutoStyleHint::~ScAutoStyleHint()
{
}

ScDBRangeRefreshedHint::ScDBRangeRefreshedHint( const ScImportParam& rP )
    : SfxHint(SfxHintId::ScDBRangeRefreshed), aParam(rP)
{
}
ScDBRangeRefreshedHint::~ScDBRangeRefreshedHint()
{
}

ScDataPilotModifiedHint::ScDataPilotModifiedHint( OUString aName )
    : SfxHint(SfxHintId::ScDataPilotModified), maName(std::move(aName))
{
}
ScDataPilotModifiedHint::~ScDataPilotModifiedHint()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
