/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"





//___________________________________________________________________
#include <xmloff/ProgressBarHelper.hxx>
#include <tools/debug.hxx>
#include <xmloff/xmltoken.hxx>

#include <stdlib.h>

using namespace ::com::sun::star;

const sal_Int32 nDefaultProgressBarRange = 1000000;
const float fProgressStep = 0.5;

ProgressBarHelper::ProgressBarHelper(const ::com::sun::star::uno::Reference < ::com::sun::star::task::XStatusIndicator>& xTempStatusIndicator,
                                    const sal_Bool bTempStrict)
: xStatusIndicator(xTempStatusIndicator)
, nRange(nDefaultProgressBarRange)
, nReference(100)
, nValue(0)
, fOldPercent(0.0)
, bStrict(bTempStrict)
, bRepeat(sal_True)
#ifdef DBG_UTIL
, bFailure(sal_False)
#endif
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
#if OSL_DEBUG_LEVEL > 0
            // workaround for toolchain bug on solaris/x86 Sun C++ 5.5
            // just call some function here
            (void) abs(nValue);
#endif
            nValue = static_cast< sal_Int32 >(fValue);
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

            xmloff::token::IncRescheduleCount();

            xStatusIndicator->setValue((sal_Int32)fNewValue);

            xmloff::token::DecRescheduleCount();

            // #95181# disabled, because we want to call setValue very often to enable a good reschedule
//          double fPercent ((fNewValue * 100) / nRange);
//          if (fPercent >= (fOldPercent + fProgressStep))
//          {
//              xStatusIndicator->setValue((sal_Int32)fNewValue);
//              fOldPercent = fPercent;
//          }
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

