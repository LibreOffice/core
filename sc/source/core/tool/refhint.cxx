/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <refhint.hxx>

namespace sc {

RefHint::RefHint( Type eType ) : SfxSimpleHint(SC_HINT_REFERENCE), meType(eType) {}
RefHint::~RefHint() {}

RefHint::Type RefHint::getType() const
{
    return meType;
}

RefMovedHint::RefMovedHint( const ScRange& rRange, const ScAddress& rMove, const sc::RefUpdateContext& rCxt ) :
    RefHint(Moved), maRange(rRange), maMoveDelta(rMove), mrCxt(rCxt) {}

RefMovedHint::~RefMovedHint() {}

const ScRange& RefMovedHint::getRange() const
{
    return maRange;
}

const ScAddress& RefMovedHint::getDelta() const
{
    return maMoveDelta;
}

const sc::RefUpdateContext& RefMovedHint::getContext() const
{
    return mrCxt;
}

RefColReorderHint::RefColReorderHint( const sc::ColRowReorderMapType& rColMap, SCTAB nTab, SCROW nRow1, SCROW nRow2 ) :
    RefHint(ColumnReordered), mrColMap(rColMap), mnTab(nTab), mnRow1(nRow1), mnRow2(nRow2) {}

RefColReorderHint::~RefColReorderHint() {}

const sc::ColRowReorderMapType& RefColReorderHint::getColMap() const
{
    return mrColMap;
}

SCTAB RefColReorderHint::getTab() const
{
    return mnTab;
}

SCROW RefColReorderHint::getStartRow() const
{
    return mnRow1;
}

SCROW RefColReorderHint::getEndRow() const
{
    return mnRow2;
}

RefRowReorderHint::RefRowReorderHint( const sc::ColRowReorderMapType& rRowMap, SCTAB nTab, SCCOL nCol1, SCCOL nCol2 ) :
    RefHint(RowReordered), mrRowMap(rRowMap), mnTab(nTab), mnCol1(nCol1), mnCol2(nCol2) {}

RefRowReorderHint::~RefRowReorderHint() {}

const sc::ColRowReorderMapType& RefRowReorderHint::getRowMap() const
{
    return mrRowMap;
}

SCTAB RefRowReorderHint::getTab() const
{
    return mnTab;
}

SCCOL RefRowReorderHint::getStartColumn() const
{
    return mnCol1;
}

SCCOL RefRowReorderHint::getEndColumn() const
{
    return mnCol2;
}

RefStartListeningHint::RefStartListeningHint() : RefHint(StartListening) {}
RefStartListeningHint::~RefStartListeningHint() {}

RefStopListeningHint::RefStopListeningHint() : RefHint(StopListening) {}
RefStopListeningHint::~RefStopListeningHint() {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
