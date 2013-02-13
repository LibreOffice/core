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
#include <rtl/bootstrap.hxx>
#include <unotools/configpaths.hxx>
#include <xmlreader/xmlreader.hxx>

#include <unomid.h>

using namespace utl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

static inline void lcl_assertEndingItem(xmlreader::XmlReader& reader)
{
#if OSL_DEBUG_LEVEL > 0
    int nsId;
    xmlreader::Span name;
    assert(reader.nextItem(xmlreader::XmlReader::TEXT_NONE, &name, &nsId)
            == xmlreader::XmlReader::RESULT_END);
#else
    (void)reader;
#endif
}

static inline OUString lcl_getValue(xmlreader::XmlReader& reader,
                                    const xmlreader::Span& span)
{
    int nsId;
    xmlreader::Span name;
    xmlreader::XmlReader::Result res;
    res = reader.nextItem(xmlreader::XmlReader::TEXT_NONE, &name, &nsId);
    assert(res == xmlreader::XmlReader::RESULT_BEGIN && name.equals(span));
    res = reader.nextItem(xmlreader::XmlReader::TEXT_RAW, &name, &nsId);
    assert(res == xmlreader::XmlReader::RESULT_TEXT);
    (void) res; (void) span;
    OUString sTmp = name.convertFromUtf8();
    lcl_assertEndingItem(reader);
    return sTmp;
}

SwLabelConfig::SwLabelConfig() :
    ConfigItem("Office.Labels/Manufacturer")
{
    OUString uri("$BRAND_BASE_DIR/share/labels/labels.xml");
    rtl::Bootstrap::expandMacros(uri);
    xmlreader::XmlReader reader(uri);
    int nsId;
    xmlreader::Span name;
    xmlreader::XmlReader::Result res;
    OUString sManufacturer;
    OUString sName;
    OUString sMeasure;

    res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);
    assert(res == xmlreader::XmlReader::RESULT_BEGIN &&
                name.equals(RTL_CONSTASCII_STRINGPARAM("manufacturers")));
    res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);
    while (res != xmlreader::XmlReader::RESULT_END)
    {
        // Opening manufacturer
        assert(res == xmlreader::XmlReader::RESULT_BEGIN &&
                name.equals(RTL_CONSTASCII_STRINGPARAM("manufacturer")));
        // Get the name
        reader.nextAttribute(&nsId, &name);
        assert(nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                name.equals(RTL_CONSTASCII_STRINGPARAM("name")));
        sManufacturer = reader.getAttributeValue(false).convertFromUtf8();

        for(;;) {
            // Opening label or ending manufacturer
            res = reader.nextItem(
                    xmlreader::XmlReader::TEXT_NONE, &name, &nsId);
            if (res == xmlreader::XmlReader::RESULT_END)
                break;
            assert(res == xmlreader::XmlReader::RESULT_BEGIN &&
                    name.equals(RTL_CONSTASCII_STRINGPARAM("label")));
            // Get name value
            sName = lcl_getValue(reader,
                    xmlreader::Span(RTL_CONSTASCII_STRINGPARAM("name")));
            // Get measure value
            sMeasure = lcl_getValue(reader,
                    xmlreader::Span(RTL_CONSTASCII_STRINGPARAM("measure")));
            // Ending label mark
            lcl_assertEndingItem(reader);
            m_aLabels[sManufacturer][sName] = sMeasure;
        }
        // Get next manufacturer or end
        res = reader.nextItem(
                xmlreader::XmlReader::TEXT_NONE, &name, &nsId);
    };
    res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);
    assert(res == xmlreader::XmlReader::RESULT_DONE);

    FillManufacturers();
}

SwLabelConfig::~SwLabelConfig()
{
}

// the config item is not writable ?:
void SwLabelConfig::Commit() {}

void SwLabelConfig::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

void SwLabelConfig::FillManufacturers()
{
    m_aManufacturers.clear();
    for (std::map< OUString, std::map<OUString, OUString> >::iterator it =
            m_aLabels.begin(); it != m_aLabels.end(); ++it)
        m_aManufacturers.push_back( it->first );

    const com::sun::star::uno::Sequence<rtl::OUString>& rMan = GetNodeNames(OUString());
    const rtl::OUString* pMan = rMan.getConstArray();
    for(sal_Int32 nMan = 0; nMan < rMan.getLength(); nMan++)
        if (m_aLabels.find( pMan[nMan] ) == m_aLabels.end())
            m_aManufacturers.push_back( pMan[nMan] );
}

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

static SwLabRec* lcl_CreateSwLabRec(const OUString& rType, const OUString& rMeasure, const OUString& rManufacturer)
{
    SwLabRec* pNewRec = new SwLabRec;
    pNewRec->aMake = rManufacturer;
    pNewRec->lPWidth = 0;
    pNewRec->lPHeight = 0;
    pNewRec->aType = rType;
    //all values are contained as colon-separated 1/100 mm values
    //except for the continuous flag ('C'/'S')
    String sMeasure(rMeasure);
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
                sTmp += rRec.bCont ? OUString("C") : OUString("S");           sTmp += sColon;
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
        const Any* pValues = aValues.getConstArray();
        OUString sType;
        OUString sMeasure;
        if (aValues.getLength() >= 1)
            if(pValues[0].hasValue())
                pValues[0] >>= sType;
        if (aValues.getLength() >= 2)
            if(pValues[1].hasValue())
                pValues[1] >>= sMeasure;
        // Remove default value if we have one from configuration
        if(m_aLabels.find(rManufacturer) != m_aLabels.end())
            m_aLabels[rManufacturer].erase(sType);
        rLabArr.push_back( lcl_CreateSwLabRec(sType, sMeasure, rManufacturer) );
    }
    // Add default labels
    if (m_aLabels.find(rManufacturer) == m_aLabels.end())
        return;
    for (std::map<OUString, OUString>::iterator it =
            m_aLabels[rManufacturer].begin();
            it != m_aLabels[rManufacturer].end(); ++it)
        rLabArr.push_back( lcl_CreateSwLabRec(it->first, it->second, rManufacturer) );
}

sal_Bool    SwLabelConfig::HasLabel(const rtl::OUString& rManufacturer, const rtl::OUString& rType)
{
    if (m_aLabels.find(rManufacturer) != m_aLabels.end())
        if (m_aLabels[rManufacturer].find(rType) != m_aLabels[rManufacturer].end())
            return true;

    bool bFound = false;
    for (size_t nNode = 0; nNode < m_aManufacturers.size() && !bFound; nNode++)
    {
        if (m_aManufacturers[nNode] == rManufacturer)
            bFound = true;
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

static bool lcl_Exists(const OUString& rNode, const Sequence<OUString>& rLabels)
{
    const OUString* pLabels = rLabels.getConstArray();
    for(sal_Int32 i = 0; i < rLabels.getLength(); i++)
        if(pLabels[i] == rNode)
            return true;
    return false;
}

void SwLabelConfig::SaveLabel(  const rtl::OUString& rManufacturer,
        const rtl::OUString& rType, const SwLabRec& rRec)
{
    bool bFound = false;
    for (size_t nNode = 0; nNode < m_aManufacturers.size() && !bFound; nNode++)
    {
        if (m_aManufacturers[nNode] == rManufacturer)
            bFound = true;
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
            FillManufacturers();
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
