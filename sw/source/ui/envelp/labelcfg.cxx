/*************************************************************************
 *
 *  $RCSfile: labelcfg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2001-01-16 10:42:30 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _LABELCFG_HXX
#include <labelcfg.hxx>
#endif
#ifndef _LABIMP_HXX
#include <labimp.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

#define C2U(cChar) OUString::createFromAscii(cChar)
/* -----------------------------15.01.01 11:17--------------------------------

 ---------------------------------------------------------------------------*/
SwLabelConfig::SwLabelConfig() :
    ConfigItem(C2U("Office.Labels/Manufacturer"))
{
    aNodeNames = GetNodeNames(OUString());
}
/* -----------------------------06.09.00 16:50--------------------------------

 ---------------------------------------------------------------------------*/
SwLabelConfig::~SwLabelConfig()
{
}
/* -----------------------------06.09.00 16:43--------------------------------

 ---------------------------------------------------------------------------*/
void    SwLabelConfig::Commit()
{
    // the config item is not writable yet
}
/* -----------------------------15.01.01 11:42--------------------------------

 ---------------------------------------------------------------------------*/
Sequence<OUString> lcl_CreatePropertyNames(const OUString& rPrefix)
{
    Sequence<OUString> aProperties(10);
    OUString* pProperties = aProperties.getArray();
    for(sal_Int32 nProp = 0; nProp < 10; nProp++)
        pProperties[nProp] = rPrefix;

    pProperties[ 0] += C2U("Name");
    pProperties[ 1] += C2U("Continuous");
    pProperties[ 2] += C2U("HorizontalDistance");
    pProperties[ 3] += C2U("VerticalDistance");
    pProperties[ 4] += C2U("Width");
    pProperties[ 5] += C2U("Height");
    pProperties[ 6] += C2U("LeftMargin");
    pProperties[ 7] += C2U("UpperMargin");
    pProperties[ 8] += C2U("Columns");
    pProperties[ 9] += C2U("Rows");
    return aProperties;
}
//-----------------------------------------------------------------------------
SwLabRec* lcl_CreateSwLabRec(Sequence<Any>& rValues, const OUString& rManufacturer)
{
    SwLabRec* pNewRec = new SwLabRec;
    const Any* pValues = rValues.getConstArray();
    OUString sTmp;
    sal_Int32 nTmp;
    pNewRec->aMake = rManufacturer;
    for(sal_Int32 nProp = 0; nProp < 10; nProp++)
    {
        if(pValues[nProp].hasValue())
        {
            switch(nProp)
            {
                case 0: pValues[nProp] >>= sTmp; pNewRec->aType = sTmp; break;
                case 1: pNewRec->bCont = *(sal_Bool*)pValues[nProp].getValue();break;
                case 2: pValues[nProp] >>= nTmp; pNewRec->lHDist = MM100_TO_TWIP(nTmp); break;
                case 3: pValues[nProp] >>= nTmp; pNewRec->lVDist = MM100_TO_TWIP(nTmp);     break;
                case 4: pValues[nProp] >>= nTmp; pNewRec->lWidth = MM100_TO_TWIP(nTmp);     break;
                case 5: pValues[nProp] >>= nTmp; pNewRec->lHeight = MM100_TO_TWIP(nTmp);    break;
                case 6: pValues[nProp] >>= nTmp; pNewRec->lLeft = MM100_TO_TWIP(nTmp);      break;
                case 7: pValues[nProp] >>= nTmp; pNewRec->lUpper = MM100_TO_TWIP(nTmp);     break;
                case 8: pValues[nProp] >>= pNewRec->nCols;      break;
                case 9: pValues[nProp] >>= pNewRec->nRows;      break;
            }
        }
    }
    return pNewRec;
}
//-----------------------------------------------------------------------------
void    SwLabelConfig::FillLabels(const OUString& rManufacturer, SwLabRecs& rLabArr)
{
    const Sequence<OUString> aLabels = GetNodeNames(rManufacturer);
    const OUString* pLabels = aLabels.getConstArray();
    for(sal_Int32 nLabel = 0; nLabel < aLabels.getLength(); nLabel++)
    {
        OUString sPrefix(rManufacturer);
        sPrefix += C2U("/");
        sPrefix += pLabels[nLabel];
        sPrefix += C2U("/");
        Sequence<OUString> aPropNames = lcl_CreatePropertyNames(sPrefix);
        Sequence<Any>   aValues = GetProperties(aPropNames);
        SwLabRec* pNewRec = lcl_CreateSwLabRec(aValues, rManufacturer);
        rLabArr.C40_INSERT( SwLabRec, pNewRec, rLabArr.Count() );
    }
}


