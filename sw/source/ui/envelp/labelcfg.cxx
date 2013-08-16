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

#include <config_folders.h>

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
    int nsId;
    xmlreader::Span name;
    xmlreader::XmlReader::Result res;
    res = reader.nextItem(xmlreader::XmlReader::TEXT_NONE, &name, &nsId);
    assert(res == xmlreader::XmlReader::RESULT_END);
    (void) res;
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

SwLabelConfig::SwLabelConfig() :
    ConfigItem("Office.Labels/Manufacturer")
{
    OUString uri("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/labels/labels.xml");
    rtl::Bootstrap::expandMacros(uri);
    xmlreader::XmlReader reader(uri);
    int nsId;
    xmlreader::Span name;
    xmlreader::XmlReader::Result res;
    OUString sManufacturer;
    OUString sName;
    OUString sMeasure;

    // fill m_aLabels and m_aManufacturers with the predefined labels
    res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);
    assert(
        res == xmlreader::XmlReader::RESULT_BEGIN
        && name.equals("manufacturers"));
    res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);
    while (res != xmlreader::XmlReader::RESULT_END)
    {
        // Opening manufacturer
        assert(
            res == xmlreader::XmlReader::RESULT_BEGIN
            && name.equals("manufacturer"));
        // Get the name
        reader.nextAttribute(&nsId, &name);
        assert(
            nsId == xmlreader::XmlReader::NAMESPACE_NONE
            && name.equals("name"));
        sManufacturer = reader.getAttributeValue(false).convertFromUtf8();

        for(;;) {
            // Opening label or ending manufacturer
            res = reader.nextItem(
                    xmlreader::XmlReader::TEXT_NONE, &name, &nsId);
            if (res == xmlreader::XmlReader::RESULT_END)
                break;
            assert(
                res == xmlreader::XmlReader::RESULT_BEGIN
                && name.equals("label"));
            // Get name value
            sName = lcl_getValue(reader, xmlreader::Span("name"));
            // Get measure value
            sMeasure = lcl_getValue(reader, xmlreader::Span("measure"));
            // Ending label mark
            lcl_assertEndingItem(reader);
            if ( m_aLabels.find( sManufacturer ) == m_aLabels.end() )
                m_aManufacturers.push_back( sManufacturer );
            m_aLabels[sManufacturer][sName].m_aMeasure = sMeasure;
            m_aLabels[sManufacturer][sName].m_bPredefined = true;
        }
        // Get next manufacturer or end
        res = reader.nextItem(
                xmlreader::XmlReader::TEXT_NONE, &name, &nsId);
    };
    res = reader.nextItem(
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);
    assert(res == xmlreader::XmlReader::RESULT_DONE);

    // add to m_aLabels and m_aManufacturers the custom labels
    const Sequence<OUString>& rMan = GetNodeNames( OUString() );
    const OUString* pMan = rMan.getConstArray();
    for ( sal_Int32 nMan = 0; nMan < rMan.getLength(); nMan++ )
    {
        sManufacturer = pMan[nMan];
        const Sequence<OUString> aLabels = GetNodeNames( sManufacturer );
        const OUString* pLabels = aLabels.getConstArray();
        for( sal_Int32 nLabel = 0; nLabel < aLabels.getLength(); nLabel++ )
        {
            OUString sPrefix( sManufacturer );
            sPrefix += "/";
            sPrefix += pLabels[nLabel];
            sPrefix += "/";
            Sequence<OUString> aPropNames = lcl_CreatePropertyNames( sPrefix );
            Sequence<Any>   aValues = GetProperties( aPropNames );
            const Any* pValues = aValues.getConstArray();
            if (aValues.getLength() >= 1)
                if(pValues[0].hasValue())
                    pValues[0] >>= sName;
            if (aValues.getLength() >= 2)
                if(pValues[1].hasValue())
                    pValues[1] >>= sMeasure;
            if ( m_aLabels.find( sManufacturer ) == m_aLabels.end() )
                m_aManufacturers.push_back( sManufacturer );
            m_aLabels[sManufacturer][sName].m_aMeasure = sMeasure;
            m_aLabels[sManufacturer][sName].m_bPredefined = false;
        }
    }
}

SwLabelConfig::~SwLabelConfig()
{
}

// the config item is not writable ?:
void SwLabelConfig::Commit() {}

void SwLabelConfig::Notify( const ::com::sun::star::uno::Sequence< OUString >& ) {}

static SwLabRec* lcl_CreateSwLabRec(const OUString& rType, const OUString& rMeasure, const OUString& rManufacturer)
{
    SwLabRec* pNewRec = new SwLabRec;
    pNewRec->aMake = rManufacturer;
    pNewRec->lPWidth = 0;
    pNewRec->lPHeight = 0;
    pNewRec->aType = rType;
    //all values are contained as colon-separated 1/100 mm values
    //except for the continuous flag ('C'/'S') and nCols, nRows (sal_Int32)
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
    // lines added for compatibility with custom label definitions saved before patch fdo#44516
    if (pNewRec->lPWidth == 0 || pNewRec->lPHeight == 0)
    {
        // old style definition (no paper dimensions), calculate probable values
        pNewRec->lPWidth = 2 * pNewRec->lLeft + (pNewRec->nCols - 1) * pNewRec->lHDist + pNewRec->lWidth;
        pNewRec->lPHeight = ( pNewRec->bCont ? pNewRec->nRows * pNewRec->lVDist : 2 * pNewRec->lUpper + (pNewRec->nRows - 1) * pNewRec->lVDist + pNewRec->lHeight );
    }
    return pNewRec;
}

static Sequence<PropertyValue> lcl_CreateProperties(
    Sequence<OUString>& rPropNames, OUString& rMeasure, const SwLabRec& rRec)
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
                rMeasure = "";
                rMeasure += rRec.bCont ? OUString( "C" ) : OUString( "S" );      rMeasure += sColon;
                rMeasure += OUString::valueOf( TWIP_TO_MM100( rRec.lHDist ) );   rMeasure += sColon;
                rMeasure += OUString::valueOf( TWIP_TO_MM100( rRec.lVDist ) );   rMeasure += sColon;
                rMeasure += OUString::valueOf( TWIP_TO_MM100( rRec.lWidth ) );   rMeasure += sColon;
                rMeasure += OUString::valueOf( TWIP_TO_MM100( rRec.lHeight ) );  rMeasure += sColon;
                rMeasure += OUString::valueOf( TWIP_TO_MM100( rRec.lLeft ) );    rMeasure += sColon;
                rMeasure += OUString::valueOf( TWIP_TO_MM100( rRec.lUpper ) );   rMeasure += sColon;
                rMeasure += OUString::valueOf( rRec.nCols );                     rMeasure += sColon;
                rMeasure += OUString::valueOf( rRec.nRows );                     rMeasure += sColon;
                rMeasure += OUString::valueOf( TWIP_TO_MM100( rRec.lPWidth ) );  rMeasure += sColon;
                rMeasure += OUString::valueOf( TWIP_TO_MM100( rRec.lPHeight ) );
                pValues[nProp].Value <<= rMeasure;
            }
            break;
        }
    }
    return aRet;
}

// function fills SwLabDlg with label definitions for manufacturer rManufacturer
void    SwLabelConfig::FillLabels(const OUString& rManufacturer, SwLabRecs& rLabArr)
{
    if (m_aLabels.find(rManufacturer) == m_aLabels.end())
        return;
    for (std::map<OUString, SwLabelMeasure>::iterator it = m_aLabels[rManufacturer].begin();
            it != m_aLabels[rManufacturer].end(); ++it)
        rLabArr.push_back( lcl_CreateSwLabRec(it->first, it->second.m_aMeasure, rManufacturer) );
}

sal_Bool    SwLabelConfig::HasLabel(const OUString& rManufacturer, const OUString& rType)
{
    return ( ( m_aLabels.find(rManufacturer) != m_aLabels.end() ) &&
             ( m_aLabels[rManufacturer].find(rType) != m_aLabels[rManufacturer].end() ) );
}

static bool lcl_Exists(const OUString& rNode, const Sequence<OUString>& rLabels)
{
    const OUString* pLabels = rLabels.getConstArray();
    for(sal_Int32 i = 0; i < rLabels.getLength(); i++)
        if(pLabels[i] == rNode)
            return true;
    return false;
}

// label is always saved as a custom label
// predefined labels can NOT be overwritten by custom labels with same manufacturer/name
void SwLabelConfig::SaveLabel( const OUString& rManufacturer,
        const OUString& rType, const SwLabRec& rRec )
{
    OUString sFoundNode;
    bool bManufacturerNodeFound;
    if ( m_aLabels.find( rManufacturer ) == m_aLabels.end() ||
         GetNodeNames( rManufacturer ).getLength() == 0 )
    {
        bManufacturerNodeFound = false;
        // manufacturer node does not exist, add (and also to m_aManufacturers)
        if ( !AddNode( OUString(), rManufacturer ) )
        {
            OSL_FAIL("New configuration node could not be created");
            return ;
        }
        m_aManufacturers.push_back( rManufacturer );
    }
    else
        bManufacturerNodeFound = true;

    if ( !bManufacturerNodeFound ||
         m_aLabels[rManufacturer].find( rType ) == m_aLabels[rManufacturer].end() )
    {
        // type does not yet exist, add to config
        const Sequence<OUString> aLabels = GetNodeNames( rManufacturer );
        sal_Int32 nIndex = aLabels.getLength();
        OUString sPrefix( "Label" );
        sFoundNode = sPrefix;
        sFoundNode += OUString::valueOf( nIndex );
        while ( lcl_Exists( sFoundNode, aLabels ) )
        {
            sFoundNode = sPrefix;
            sFoundNode += OUString::valueOf(nIndex++);
        }
    }
    else
    {
        // get the appropiate node
        OUString sManufacturer( wrapConfigurationElementName( rManufacturer ) );
        const Sequence<OUString> aLabels = GetNodeNames( sManufacturer );
        const OUString* pLabels = aLabels.getConstArray();
        for (sal_Int32 nLabel = 0; nLabel < aLabels.getLength(); nLabel++)
        {
            OUString sPrefix( sManufacturer );
            sPrefix += "/";
            sPrefix += pLabels[nLabel];
            sPrefix += "/";
            Sequence<OUString> aProperties(1);
            aProperties.getArray()[0] = sPrefix;
            aProperties.getArray()[0] += "Name";
            Sequence<Any> aValues = GetProperties( aProperties );
            const Any* pValues = aValues.getConstArray();
            if ( pValues[0].hasValue() )
            {
                OUString sTmp;
                pValues[0] >>= sTmp;
                if ( rType == sTmp )
                {
                    sFoundNode = pLabels[nLabel];
                    break;
                }
            }
        }
    }

    OUString sPrefix( wrapConfigurationElementName( rManufacturer ) );
    sPrefix += "/";
    sPrefix += sFoundNode;
    sPrefix += "/";
    Sequence<OUString> aPropNames = lcl_CreatePropertyNames( sPrefix );
    OUString sMeasure;
    Sequence<PropertyValue> aPropValues = lcl_CreateProperties( aPropNames, sMeasure, rRec );
    SetSetProperties( wrapConfigurationElementName( rManufacturer ), aPropValues );

    //update m_aLabels
    m_aLabels[rManufacturer][rType].m_aMeasure = sMeasure;
    m_aLabels[rManufacturer][rType].m_bPredefined = false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
