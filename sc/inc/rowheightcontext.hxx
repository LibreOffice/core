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

#include <tools/rational.hxx>

class OutputDevice;

namespace sc {

class SC_DLLPUBLIC RowHeightContext
{
    double mfPPTX;
    double mfPPTY;
    boost::rational<sal_Int64> maZoomX;
    boost::rational<sal_Int64> maZoomY;
    OutputDevice* mpOutDev;

    sal_uInt16 mnExtraHeight;
    bool mbForceAutoSize; /// whether to set height to optimal even when the manual height flag is set.

public:
    RowHeightContext(
        double fPPTX, double fPPTY, const boost::rational<sal_Int64>& rZoomX, const boost::rational<sal_Int64>& rZoomY,
        OutputDevice* pOutDev );

    ~RowHeightContext();

    double getPPTX() const { return mfPPTX;}
    double getPPTY() const { return mfPPTY;}
    const boost::rational<sal_Int64>& getZoomX() const { return maZoomX;}
    const boost::rational<sal_Int64>& getZoomY() const { return maZoomY;}

    OutputDevice* getOutputDevice() { return mpOutDev;}

    void setExtraHeight( sal_uInt16 nH );
    sal_uInt16 getExtraHeight() const { return mnExtraHeight;}

    void setForceAutoSize( bool b );
    bool isForceAutoSize() const { return mbForceAutoSize;}
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
