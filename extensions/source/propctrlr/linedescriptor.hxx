/*************************************************************************
 *
 *  $RCSfile: linedescriptor.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-06 15:50:43 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _EXTENSIONS_PROPCTRLR_LINEDESCRIPTOR_HXX_
#define _EXTENSIONS_PROPCTRLR_LINEDESCRIPTOR_HXX_

#ifndef _EXTENSIONS_PROPCTRLR_BRWCONTROL_HXX_
#include "brwcontrol.hxx"
#endif

//............................................................................
namespace pcr
{
//............................................................................

    class IBrowserControl;
    //========================================================================
    //= OLineDescriptor
    //========================================================================
    struct OLineDescriptor
    {
        String                      sName;
        String                      sTitle;
        String                      sValue;
        ::std::vector< String>      aListValues;
        void*                       pDataPtr;
        IBrowserControl*            pControl;
        BrowserControlType          eControlType;
        sal_uInt32                  nHelpId;
        sal_uInt32                  nUniqueButtonId;
        sal_uInt16                  nDigits;    // for numeric fields
        sal_Int32                   nMinValue;  // for numeric fields only
        sal_Int32                   nMaxValue;  // for numeric fields only

        sal_Bool                    bUnknownValue   :1;
        sal_Bool                    bHasDefaultValue:1;
        sal_Bool                    bHasBrowseButton:1;
        sal_Bool                    bIsHyperlink    :1;
        sal_Bool                    bIsLocked       :1;
        sal_Bool                    bHaveMinMax     :1;

        OLineDescriptor()
            :eControlType(BCT_UNDEFINED)
            ,nHelpId(0)
            ,bUnknownValue(sal_False)
            ,bHasDefaultValue(sal_False)
            ,bHasBrowseButton(sal_False)
            ,bIsHyperlink(sal_False)
            ,bIsLocked(sal_False)
            ,pDataPtr(NULL)
            ,pControl(NULL)
            ,nDigits(0)
            ,nUniqueButtonId(0)
            ,nMinValue(0)
            ,nMaxValue(-1)
            ,bHaveMinMax(sal_False)
        {
        }

        // does not copy theValues
        // TODO: (fs) why?
        OLineDescriptor(const OLineDescriptor& rData)
            :eControlType(rData.eControlType)
            ,sValue(rData.sValue)
            ,sTitle(rData.sTitle)
            ,sName(rData.sName)
            ,nHelpId(rData.nHelpId)
            ,bUnknownValue(rData.bUnknownValue)
            ,bHasDefaultValue(rData.bHasDefaultValue)
            ,bHasBrowseButton(rData.bHasBrowseButton)
            ,bIsHyperlink(rData.bIsHyperlink)
            ,bIsLocked(rData.bIsLocked)
            ,pDataPtr(rData.pDataPtr)
            ,pControl(rData.pControl)
            ,nDigits(rData.nDigits)
            ,nUniqueButtonId(rData.nUniqueButtonId)
            ,nMinValue(rData.nMinValue)
            ,nMaxValue(rData.nMaxValue)
            ,bHaveMinMax(rData.bHaveMinMax)
        {
        }
    };

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_LINEDESCRIPTOR_HXX_

