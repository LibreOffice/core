/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_ROWHEIGHTCONTEXT_HXX
#define INCLUDED_SC_INC_ROWHEIGHTCONTEXT_HXX

#include "scdllapi.h"

#include <tools/fract.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/outdev.hxx>

#include <vector>

#include "segmenttree.hxx"

namespace sc {

class SC_DLLPUBLIC RowHeightContext
{
    ScFlatUInt16RowSegments maHeights;

    double mfPPTX;
    double mfPPTY;
    Fraction maZoomX;
    Fraction maZoomY;
    VclPtr<OutputDevice> mpOutDev;

    sal_uInt16 mnExtraHeight;
    bool mbForceAutoSize; /// whether to set height to optimal even when the manual height flag is set.

public:
    RowHeightContext(
        double fPPTX, double fPPTY, const Fraction& rZoomX, const Fraction& rZoomY,
        OutputDevice* pOutDev );

    ~RowHeightContext();

    double getPPTX() const { return mfPPTX;}
    double getPPTY() const { return mfPPTY;}
    const Fraction& getZoomX() const { return maZoomX;}
    const Fraction& getZoomY() const { return maZoomY;}

    OutputDevice* getOutputDevice() { return mpOutDev;}

    void setExtraHeight( sal_uInt16 nH );
    sal_uInt16 getExtraHeight() const { return mnExtraHeight;}

    void setForceAutoSize( bool b );
    bool isForceAutoSize() const { return mbForceAutoSize;}

    ScFlatUInt16RowSegments& getHeightArray();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
