/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SheetView.hxx>
#include <table.hxx>

namespace sc
{
SheetView::SheetView(ScTable* pTable)
    : mpTable(pTable)
{
}

ScTable* SheetView::getTablePointer() const { return mpTable; }
bool SheetView::isValid() const { return mpTable; }
SCTAB SheetView::getTableNumber() const
{
    assert(mpTable);
    return mpTable->GetTab();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
