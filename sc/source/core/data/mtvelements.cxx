/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "mtvelements.hxx"
#include "globalnames.hxx"

namespace sc {

CellTextAttr::CellTextAttr() :
    mnTextWidth(TEXTWIDTH_DIRTY),
    mnScriptType(SC_SCRIPTTYPE_UNKNOWN) {}

CellTextAttr::CellTextAttr(const CellTextAttr& r) :
    mnTextWidth(r.mnTextWidth),
    mnScriptType(r.mnScriptType) {}

CellTextAttr::CellTextAttr(sal_uInt16 nTextWidth, sal_uInt8 nScriptType) :
    mnTextWidth(nTextWidth),
    mnScriptType(nScriptType) {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
