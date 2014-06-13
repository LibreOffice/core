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


RefMovedHint::RefMovedHint( const ScRange& rRange, const ScAddress& rMove ) :
    RefHint(Moved), maRange(rRange), maMoveDelta(rMove) {}

RefMovedHint::~RefMovedHint() {}



RefColReorderHint::RefColReorderHint( const sc::ColReorderMapType& rColMap, SCTAB nTab, SCROW nRow1, SCROW nRow2 ) :
    RefHint(ColumnReordered), mrColMap(rColMap), mnTab(nTab), mnRow1(nRow1), mnRow2(nRow2) {}

RefColReorderHint::~RefColReorderHint() {}





}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
