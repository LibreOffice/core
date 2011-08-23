/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifdef _MSC_VER
#pragma hdrstop
#endif


//___________________________________________________________________

#include "ProgressBarHelper.hxx"

#include <tools/debug.hxx>
namespace binfilter {

using namespace ::com::sun::star;

const sal_Int32 nDefaultProgressBarRange = 1000000;
const float fProgressStep = 0.5;

ProgressBarHelper::ProgressBarHelper(const ::com::sun::star::uno::Reference < ::com::sun::star::task::XStatusIndicator>& xTempStatusIndicator,
                                    const sal_Bool bTempStrict)
    : fOldPercent(0.0),
    nRange(nDefaultProgressBarRange),
    xStatusIndicator(xTempStatusIndicator),
    nReference(100),
    nValue(0),
#ifdef DBG_UTIL
    bFailure(sal_False),
#endif
    bStrict(bTempStrict),
    bRepeat(sal_True)
{
}

ProgressBarHelper::~ProgressBarHelper()
{
}

sal_Int32 ProgressBarHelper::ChangeReference(sal_Int32 nNewReference)
{
    if((nNewReference > 0) && (nNewReference != nReference))
    {
        if (nReference)
        {
            double fPercent(nNewReference / nReference);
            double fValue(nValue * fPercent);
            nValue = sal_Int32(nValue);
            nReference = nNewReference;
        }
        else
        {
            nReference = nNewReference;
            nValue = 0;
        }
    }
    return nValue;
}

void ProgressBarHelper::SetValue(sal_Int32 nTempValue)
{
    if (xStatusIndicator.is() && (nReference > 0))
    {
        if ((nTempValue >= nValue) && (!bStrict || (bStrict && (nTempValue <= nReference))))
        {
            // #91317# no progress bar with values > 100%
            if (nTempValue > nReference)
            {
                if (!bRepeat)
                    nValue = nReference;
                else
                {
//                    xStatusIndicator->end();
//                    xStatusIndicator->start();
                    xStatusIndicator->reset();
                    nValue = 0;
                }
            }
            else
                nValue = nTempValue;

            double fValue(nValue);
            double fNewValue ((fValue * nRange) / nReference);
            xStatusIndicator->setValue((sal_Int32)fNewValue);

            // #95181# disabled, because we want to call setValue very often to enable a good reschedule
//			double fPercent ((fNewValue * 100) / nRange);
//			if (fPercent >= (fOldPercent + fProgressStep))
//			{
//				xStatusIndicator->setValue((sal_Int32)fNewValue);
//				fOldPercent = fPercent;
//			}
        }
#ifdef DBG_UTIL
        else if (!bFailure)
        {
            DBG_ERROR("tried to set a wrong value on the progressbar");
            bFailure = sal_True;
        }
#endif
    }
}

}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
