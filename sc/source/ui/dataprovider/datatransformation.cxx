/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "datatransformation.hxx"

#include "document.hxx"

namespace sc {

DataTransformation::~DataTransformation()
{
}

ColumnRemoveTransformation::ColumnRemoveTransformation(SCCOL nCol):
    mnCol(nCol)
{
}

ColumnRemoveTransformation::~ColumnRemoveTransformation()
{
}

void ColumnRemoveTransformation::Transform(ScDocument& rDoc)
{
    rDoc.DeleteCol(0, 0, MAXROW, 0, mnCol, 1);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
