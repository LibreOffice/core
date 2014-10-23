/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rowheightcontext.hxx>

namespace sc {

RowHeightContext::RowHeightContext(
    double fPPTX, double fPPTY, const Fraction& rZoomX, const Fraction& rZoomY,
    OutputDevice* pOutDev ) :
    mfPPTX(fPPTX), mfPPTY(fPPTY),
    maZoomX(rZoomX), maZoomY(rZoomY),
    mpOutDev(pOutDev),
    mnExtraHeight(0),
    mbForceAutoSize(false) {}

RowHeightContext::~RowHeightContext() {}

void RowHeightContext::setExtraHeight( sal_uInt16 nH )
{
    mnExtraHeight = nH;
}

void RowHeightContext::setForceAutoSize( bool b )
{
    mbForceAutoSize = b;
}

std::vector<sal_uInt16>& RowHeightContext::getHeightArray()
{
    return maHeights;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
