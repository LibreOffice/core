/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ProgressBarHelper.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 13:32:17 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"





//___________________________________________________________________

#ifndef _XMLOFF_PROGRESSBARHELPER_HXX
#include <xmloff/ProgressBarHelper.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif

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
#ifndef PRODUCT
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
#ifndef PRODUCT
        else if (!bFailure)
        {
            DBG_ERROR("tried to set a wrong value on the progressbar");
            bFailure = sal_True;
        }
#endif
    }
}

