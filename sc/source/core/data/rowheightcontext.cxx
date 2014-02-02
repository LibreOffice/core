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

double RowHeightContext::getPPTX() const
{
    return mfPPTX;
}

double RowHeightContext::getPPTY() const
{
    return mfPPTY;
}

const Fraction& RowHeightContext::getZoomX() const
{
    return maZoomX;
}

const Fraction& RowHeightContext::getZoomY() const
{
    return maZoomY;
}

OutputDevice* RowHeightContext::getOutputDevice()
{
    return mpOutDev;
}

void RowHeightContext::setExtraHeight( sal_uInt16 nH )
{
    mnExtraHeight = nH;
}

sal_uInt16 RowHeightContext::getExtraHeight() const
{
    return mnExtraHeight;
}

void RowHeightContext::setForceAutoSize( bool b )
{
    mbForceAutoSize = b;
}

bool RowHeightContext::isForceAutoSize() const
{
    return mbForceAutoSize;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
