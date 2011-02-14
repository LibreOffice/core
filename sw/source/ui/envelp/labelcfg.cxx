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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <swtypes.hxx>
#include <labelcfg.hxx>
#include <labimp.hxx>
#include <unotools/configpathes.hxx>

#include <unomid.h>

using namespace utl;
using namespace rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

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

void SwLabelConfig::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

/* -----------------------------15.01.01 11:42--------------------------------

 ---------------------------------------------------------------------------*/
Sequence<OUString> lcl_CreatePropertyNames(const OUString& rPrefix)
{
    Sequence<OUString> aProperties(2);
    OUString* pProperties = aProperties.getArray();
    for(sal_Int32 nProp = 0; nProp < 2; nProp++)
        pProperties[nProp] = rPrefix;

    pProperties[ 0] += C2U("Name");
    pProperties[ 1] += C2U("Measure");
    return aProperties;
}
//-----------------------------------------------------------------------------
SwLabRec* lcl_CreateSwLabRec(Sequence<Any>& rValues, const OUString& rManufacturer)
{
    SwLabRec* pNewRec = new SwLabRec;
    const Any* pValues = rValues.getConstArray();
    OUString sTmp;
    pNewRec->aMake = rManufacturer;
    for(sal_Int32 nProp = 0; nProp < rValues.getLength(); nProp++)
    {
        if(pValues[nProp].hasValue())
        {
            switch(nProp)
            {
                case 0: pValues[nProp] >>= sTmp; pNewRec->aType = sTmp; break;
                case 1:
                {
//all values are contained as colon-separated 1/100 mm values except for the
//continuous flag ('C'/'S')
                    pValues[nProp] >>= sTmp;
                    String sMeasure(sTmp);
                    sal_uInt16 nTokenCount = sMeasure.GetTokenCount(';');
                    for(sal_uInt16 i = 0; i < nTokenCount; i++)
                    {
                        String sToken(sMeasure.GetToken(i, ';' ));
                        int nVal = sToken.ToInt32();
                        switch(i)
                        {
                            case 0 : pNewRec->bCont = sToken.GetChar(0) == 'C'; break;
                            case 1 : pNewRec->lHDist    = MM100_TO_TWIP(nVal);break;
                            case 2 : pNewRec->lVDist    = MM100_TO_TWIP(nVal);break;
                            case 3 : pNewRec->lWidth    = MM100_TO_TWIP(nVal);break;
                            case 4 : pNewRec->lHeight   = MM100_TO_TWIP(nVal); break;
                            case 5 : pNewRec->lLeft     = MM100_TO_TWIP(nVal);break;
                            case 6 : pNewRec->lUpper    = MM100_TO_TWIP(nVal);break;
                            case 7 : pNewRec->nCols     = nVal; break;
                            case 8 : pNewRec->nRows     = nVal; break;
                        }
                    }
                }
                break;
            }
        }
    }
    return pNewRec;
}
//-----------------------------------------------------------------------------
Sequence<PropertyValue> lcl_CreateProperties(
    Sequence<OUString>& rPropNames, const SwLabRec& rRec)
{
    const OUString* pNames = rPropNames.getConstArray();
    Sequence<PropertyValue> aRet(rPropNames.getLength());
    PropertyValue* pValues = aRet.getArray();
    OUString sColon(C2U(";"));

    for(sal_Int32 nProp = 0; nProp < rPropNames.getLength(); nProp++)
    {
        pValues[nProp].Name = pNames[nProp];
        switch(nProp)
        {
            case 0: pValues[nProp].Value <<= OUString(rRec.aType); break;
            case 1:
            {
                OUString sTmp;
                sTmp += C2U( rRec.bCont ? "C" : "S");                            sTmp += sColon;
                sTmp += OUString::valueOf(TWIP_TO_MM100(rRec.lHDist) );           sTmp += sColon;
                sTmp += OUString::valueOf(TWIP_TO_MM100(rRec.lVDist));            sTmp += sColon;
                sTmp += OUString::valueOf(TWIP_TO_MM100(rRec.lWidth)  );          sTmp += sColon;
                sTmp += OUString::valueOf(TWIP_TO_MM100(rRec.lHeight) );          sTmp += sColon;
                sTmp += OUString::valueOf(TWIP_TO_MM100(rRec.lLeft)   );          sTmp += sColon;
                sTmp += OUString::valueOf(TWIP_TO_MM100(rRec.lUpper)  );          sTmp += sColon;
                sTmp += OUString::valueOf(rRec.nCols   );sTmp += sColon;
                sTmp += OUString::valueOf(rRec.nRows   );
                pValues[nProp].Value <<= sTmp;
            }
            break;
        }
    }
    return aRet;
}
//-----------------------------------------------------------------------------
void    SwLabelConfig::FillLabels(const OUString& rManufacturer, SwLabRecs& rLabArr)
{
    OUString sManufacturer(wrapConfigurationElementName(rManufacturer));
    const Sequence<OUString> aLabels = GetNodeNames(sManufacturer);
    const OUString* pLabels = aLabels.getConstArray();
    for(sal_Int32 nLabel = 0; nLabel < aLabels.getLength(); nLabel++)
    {
        OUString sPrefix(sManufacturer);
        sPrefix += C2U("/");
        sPrefix += pLabels[nLabel];
        sPrefix += C2U("/");
        Sequence<OUString> aPropNames = lcl_CreatePropertyNames(sPrefix);
        Sequence<Any>   aValues = GetProperties(aPropNames);
        SwLabRec* pNewRec = lcl_CreateSwLabRec(aValues, rManufacturer);
        rLabArr.C40_INSERT( SwLabRec, pNewRec, rLabArr.Count() );
    }
}
/* -----------------------------23.01.01 11:36--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool    SwLabelConfig::HasLabel(const rtl::OUString& rManufacturer, const rtl::OUString& rType)
{
    const OUString* pNode = aNodeNames.getConstArray();
    sal_Bool bFound = sal_False;
    for(sal_Int32 nNode = 0; nNode < aNodeNames.getLength() && !bFound; nNode++)
    {
        if(pNode[nNode] == rManufacturer)
            bFound = sal_True;
    }
    if(bFound)
    {
        OUString sManufacturer(wrapConfigurationElementName(rManufacturer));
        const Sequence<OUString> aLabels = GetNodeNames(sManufacturer);
        const OUString* pLabels = aLabels.getConstArray();
        for(sal_Int32 nLabel = 0; nLabel < aLabels.getLength(); nLabel++)
        {
            OUString sPrefix(sManufacturer);
            sPrefix += C2U("/");
            sPrefix += pLabels[nLabel];
            sPrefix += C2U("/");
            Sequence<OUString> aProperties(1);
            aProperties.getArray()[0] = sPrefix;
            aProperties.getArray()[0] += C2U("Name");
            Sequence<Any>   aValues = GetProperties(aProperties);
            const Any* pValues = aValues.getConstArray();
            if(pValues[0].hasValue())
            {
                OUString sTmp;
                pValues[0] >>= sTmp;
                if(rType == sTmp)
                    return sal_True;
            }
        }
    }
    return sal_False;
}
/* -----------------------------23.01.01 11:36--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool lcl_Exists(const OUString& rNode, const Sequence<OUString>& rLabels)
{
    const OUString* pLabels = rLabels.getConstArray();
    for(sal_Int32 i = 0; i < rLabels.getLength(); i++)
        if(pLabels[i] == rNode)
            return sal_True;
    return sal_False;
}
//-----------------------------------------------------------------------------
void SwLabelConfig::SaveLabel(  const rtl::OUString& rManufacturer,
        const rtl::OUString& rType, const SwLabRec& rRec)
{
    const OUString* pNode = aNodeNames.getConstArray();
    sal_Bool bFound = sal_False;
    for(sal_Int32 nNode = 0; nNode < aNodeNames.getLength() && !bFound; nNode++)
    {
        if(pNode[nNode] == rManufacturer)
            bFound = sal_True;
    }
    if(!bFound)
    {
        if(!AddNode(OUString(), rManufacturer))
        {
            DBG_ERROR("New configuration node could not be created");
            return ;
        }
        else
        {
            aNodeNames = GetNodeNames(OUString());
        }
    }

    OUString sManufacturer(wrapConfigurationElementName(rManufacturer));
    const Sequence<OUString> aLabels = GetNodeNames(sManufacturer);
    const OUString* pLabels = aLabels.getConstArray();
    OUString sFoundNode;
    for(sal_Int32 nLabel = 0; nLabel < aLabels.getLength(); nLabel++)
    {
        OUString sPrefix(sManufacturer);
        sPrefix += C2U("/");
        sPrefix += pLabels[nLabel];
        sPrefix += C2U("/");
        Sequence<OUString> aProperties(1);
        aProperties.getArray()[0] = sPrefix;
        aProperties.getArray()[0] += C2U("Name");
        Sequence<Any>   aValues = GetProperties(aProperties);
        const Any* pValues = aValues.getConstArray();
        if(pValues[0].hasValue())
        {
            OUString sTmp;
            pValues[0] >>= sTmp;
            if(rType == sTmp)
            {
                sFoundNode = pLabels[nLabel];
                break;
            }
        }
    }
    // if not found - generate a unique node name
    if(!sFoundNode.getLength())
    {
        sal_Int32 nIndex = aLabels.getLength();
        OUString sPrefix(C2U("Label"));
        sFoundNode = sPrefix;
        sFoundNode += OUString::valueOf(nIndex);
        while(lcl_Exists(sFoundNode, aLabels))
        {
            sFoundNode = sPrefix;
            sFoundNode += OUString::valueOf(nIndex++);
        }
    }
    OUString sPrefix(wrapConfigurationElementName(rManufacturer));
    sPrefix += C2U("/");
    sPrefix += sFoundNode;
    sPrefix += C2U("/");
    Sequence<OUString> aPropNames = lcl_CreatePropertyNames(sPrefix);
    Sequence<PropertyValue> aPropValues = lcl_CreateProperties(aPropNames, rRec);
    SetSetProperties(wrapConfigurationElementName(rManufacturer), aPropValues);

}


