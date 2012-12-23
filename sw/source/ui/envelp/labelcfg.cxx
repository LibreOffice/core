/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <swtypes.hxx>
#include <labelcfg.hxx>
#include <labimp.hxx>
#include <comphelper/string.hxx>
#include <unotools/configpaths.hxx>

#include <unomid.h>

using namespace utl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

using ::rtl::OUString;

SwLabelConfig::SwLabelConfig() :
    ConfigItem("Office.Labels/Manufacturer")
{
    aNodeNames = GetNodeNames(OUString());
}

SwLabelConfig::~SwLabelConfig()
{
}

void    SwLabelConfig::Commit()
{
    // the config item is not writable yet
}

void SwLabelConfig::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

static Sequence<OUString> lcl_CreatePropertyNames(const OUString& rPrefix)
{
    Sequence<OUString> aProperties(2);
    OUString* pProperties = aProperties.getArray();
    for(sal_Int32 nProp = 0; nProp < 2; nProp++)
        pProperties[nProp] = rPrefix;

    pProperties[ 0] += "Name";
    pProperties[ 1] += "Measure";
    return aProperties;
}

static SwLabRec* lcl_CreateSwLabRec(Sequence<Any>& rValues, const OUString& rManufacturer)
{
    SwLabRec* pNewRec = new SwLabRec;
    const Any* pValues = rValues.getConstArray();
    OUString sTmp;
    pNewRec->aMake = rManufacturer;
    pNewRec->lPWidth = 0;
    pNewRec->lPHeight = 0;
    for(sal_Int32 nProp = 0; nProp < rValues.getLength(); nProp++)
    {
        if(pValues[nProp].hasValue())
        {
            switch(nProp)
            {
                case 0: pValues[nProp] >>= sTmp; pNewRec->aType = sTmp; break;
                case 1:
                {
                    //all values are contained as colon-separated 1/100 mm values
                    //except for the continuous flag ('C'/'S')
                    pValues[nProp] >>= sTmp;
                    String sMeasure(sTmp);
                    sal_uInt16 nTokenCount = comphelper::string::getTokenCount(sMeasure, ';');
                    for(sal_uInt16 i = 0; i < nTokenCount; i++)
                    {
                        String sToken(sMeasure.GetToken(i, ';' ));
                        int nVal = sToken.ToInt32();
                        switch(i)
                        {
                            case  0 : pNewRec->bCont = sToken.GetChar(0) == 'C'; break;
                            case  1 : pNewRec->lHDist    = MM100_TO_TWIP(nVal);  break;
                            case  2 : pNewRec->lVDist    = MM100_TO_TWIP(nVal);  break;
                            case  3 : pNewRec->lWidth    = MM100_TO_TWIP(nVal);  break;
                            case  4 : pNewRec->lHeight   = MM100_TO_TWIP(nVal);  break;
                            case  5 : pNewRec->lLeft     = MM100_TO_TWIP(nVal);  break;
                            case  6 : pNewRec->lUpper    = MM100_TO_TWIP(nVal);  break;
                            case  7 : pNewRec->nCols     = nVal;                 break;
                            case  8 : pNewRec->nRows     = nVal;                 break;
                            case  9 : pNewRec->lPWidth   = MM100_TO_TWIP(nVal);  break;
                            case 10 : pNewRec->lPHeight  = MM100_TO_TWIP(nVal);  break;
                        }
                    }
                }
                break;
            }
        }
    }
    // lines added for compatibility with custom label defintions saved before patch 44516
    if (pNewRec->lPWidth == 0 || pNewRec->lPHeight == 0)
    {
        // old style definition (no paper dimensions), calculate probable values
        pNewRec->lPWidth = 2 * pNewRec->lLeft + (pNewRec->nCols - 1) * pNewRec->lHDist + pNewRec->lWidth;
        pNewRec->lPHeight = ( pNewRec->bCont ? pNewRec->nRows * pNewRec->lVDist : 2 * pNewRec->lUpper + (pNewRec->nRows - 1) * pNewRec->lVDist + pNewRec->lHeight );
    }
    return pNewRec;
}

static Sequence<PropertyValue> lcl_CreateProperties(
    Sequence<OUString>& rPropNames, const SwLabRec& rRec)
{
    const OUString* pNames = rPropNames.getConstArray();
    Sequence<PropertyValue> aRet(rPropNames.getLength());
    PropertyValue* pValues = aRet.getArray();
    OUString sColon(";");

    for(sal_Int32 nProp = 0; nProp < rPropNames.getLength(); nProp++)
    {
        pValues[nProp].Name = pNames[nProp];
        switch(nProp)
        {
            case 0: pValues[nProp].Value <<= OUString(rRec.aType); break;
            case 1:
            {
                OUString sTmp;
                sTmp += ( rRec.bCont ? "C" : "S" );                           sTmp += sColon;
                sTmp += OUString::valueOf(TWIP_TO_MM100(rRec.lHDist) );       sTmp += sColon;
                sTmp += OUString::valueOf(TWIP_TO_MM100(rRec.lVDist));        sTmp += sColon;
                sTmp += OUString::valueOf(TWIP_TO_MM100(rRec.lWidth)  );      sTmp += sColon;
                sTmp += OUString::valueOf(TWIP_TO_MM100(rRec.lHeight) );      sTmp += sColon;
                sTmp += OUString::valueOf(TWIP_TO_MM100(rRec.lLeft)   );      sTmp += sColon;
                sTmp += OUString::valueOf(TWIP_TO_MM100(rRec.lUpper)  );      sTmp += sColon;
                sTmp += OUString::valueOf(rRec.nCols   );                     sTmp += sColon;
                sTmp += OUString::valueOf(rRec.nRows   );                     sTmp += sColon;
                sTmp += OUString::valueOf(TWIP_TO_MM100(rRec.lPWidth)  );     sTmp += sColon;
                sTmp += OUString::valueOf(TWIP_TO_MM100(rRec.lPHeight)  );
                pValues[nProp].Value <<= sTmp;
            }
            break;
        }
    }
    return aRet;
}

void    SwLabelConfig::FillLabels(const OUString& rManufacturer, SwLabRecs& rLabArr)
{
    OUString sManufacturer(wrapConfigurationElementName(rManufacturer));
    const Sequence<OUString> aLabels = GetNodeNames(sManufacturer);
    const OUString* pLabels = aLabels.getConstArray();
    for(sal_Int32 nLabel = 0; nLabel < aLabels.getLength(); nLabel++)
    {
        OUString sPrefix(sManufacturer);
        sPrefix += "/";
        sPrefix += pLabels[nLabel];
        sPrefix += "/";
        Sequence<OUString> aPropNames = lcl_CreatePropertyNames(sPrefix);
        Sequence<Any>   aValues = GetProperties(aPropNames);
        SwLabRec* pNewRec = lcl_CreateSwLabRec(aValues, rManufacturer);
        rLabArr.push_back( pNewRec );
    }
}

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
            sPrefix += "/";
            sPrefix += pLabels[nLabel];
            sPrefix += "/";
            Sequence<OUString> aProperties(1);
            aProperties.getArray()[0] = sPrefix;
            aProperties.getArray()[0] += "Name";
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

static sal_Bool lcl_Exists(const OUString& rNode, const Sequence<OUString>& rLabels)
{
    const OUString* pLabels = rLabels.getConstArray();
    for(sal_Int32 i = 0; i < rLabels.getLength(); i++)
        if(pLabels[i] == rNode)
            return sal_True;
    return sal_False;
}

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
            OSL_FAIL("New configuration node could not be created");
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
        sPrefix += "/";
        sPrefix += pLabels[nLabel];
        sPrefix += "/";
        Sequence<OUString> aProperties(1);
        aProperties.getArray()[0] = sPrefix;
        aProperties.getArray()[0] += "Name";
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
    if(sFoundNode.isEmpty())
    {
        sal_Int32 nIndex = aLabels.getLength();
        OUString sPrefix("Label");
        sFoundNode = sPrefix;
        sFoundNode += OUString::valueOf(nIndex);
        while(lcl_Exists(sFoundNode, aLabels))
        {
            sFoundNode = sPrefix;
            sFoundNode += OUString::valueOf(nIndex++);
        }
    }
    OUString sPrefix(wrapConfigurationElementName(rManufacturer));
    sPrefix += "/";
    sPrefix += sFoundNode;
    sPrefix += "/";
    Sequence<OUString> aPropNames = lcl_CreatePropertyNames(sPrefix);
    Sequence<PropertyValue> aPropValues = lcl_CreateProperties(aPropNames, rRec);
    SetSetProperties(wrapConfigurationElementName(rManufacturer), aPropValues);

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
