/*************************************************************************
 *
 *  $RCSfile: ProgressBarHelper.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: sab $ $Date: 2001-04-06 08:14:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop


//___________________________________________________________________

#ifndef _XMLOFF_PROGRESSBARHELPER_HXX
#include "ProgressBarHelper.hxx"
#endif

using namespace ::com::sun::star;

const sal_Int32 nDefaultProgressBarRange = 1000000;
const sal_Int16 nProgressStep = 2;

ProgressBarHelper::ProgressBarHelper(const ::com::sun::star::uno::Reference < ::com::sun::star::task::XStatusIndicator>& xTempStatusIndicator,
                                    const ::rtl::OUString& rText)
    : nOldPercent(0),
    nRange(nDefaultProgressBarRange),
    xStatusIndicator(xTempStatusIndicator)
{
//  if (xStatusIndicator.is())
//      xStatusIndicator->setText(rText);
//      xStatusIndicator->start(rText, nProgressBarRange);
}

ProgressBarHelper::~ProgressBarHelper()
{
//  if (xStatusIndicator.is())
//      xStatusIndicator->end();
}

void ProgressBarHelper::SetValue(sal_Int32 nTempValue)
{
    nValue = nTempValue;
    if (xStatusIndicator.is())
    {
        double fValue(nValue);
        double fNewValue ((fValue * nRange) / nReference);
        double fPercent ((fNewValue * 100) / nRange);
        if ((sal_Int16)fPercent >= (nOldPercent + nProgressStep))
        {
            xStatusIndicator->setValue((sal_Int32)fNewValue);
            nOldPercent = (sal_Int16)fPercent;
        }
    }
}

