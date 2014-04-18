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

#include <scdllapi.h>

#include <tools/fract.hxx>

class OutputDevice;

namespace sc {

class SC_DLLPUBLIC RowHeightContext
{
    double mfPPTX;
    double mfPPTY;
    Fraction maZoomX;
    Fraction maZoomY;
    OutputDevice* mpOutDev;

    sal_uInt16 mnExtraHeight;
    bool mbForceAutoSize; /// whether to set height to optimal even when the manual height flag is set.

public:
    RowHeightContext(
        double fPPTX, double fPPTY, const Fraction& rZoomX, const Fraction& rZoomY,
        OutputDevice* pOutDev );

    ~RowHeightContext();

    double getPPTX() const;
    double getPPTY() const;
    const Fraction& getZoomX() const;
    const Fraction& getZoomY() const;

    OutputDevice* getOutputDevice();

    void setExtraHeight( sal_uInt16 nH );
    sal_uInt16 getExtraHeight() const;

    void setForceAutoSize( bool b );
    bool isForceAutoSize() const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
