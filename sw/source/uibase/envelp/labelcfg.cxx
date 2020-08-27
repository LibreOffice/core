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

#include <memory>
#include <config_folders.h>

#include <swtypes.hxx>
#include <labelcfg.hxx>
#include <rtl/bootstrap.hxx>
#include <tools/UnitConversion.hxx>
#include <unotools/configpaths.hxx>
#include <xmlreader/xmlreader.hxx>
#include <comphelper/sequence.hxx>
#include <osl/diagnose.h>

#include <com/sun/star/beans/PropertyValue.hpp>

using namespace utl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

static void lcl_assertEndingItem(xmlreader::XmlReader& reader)
{
    int nsId;
    xmlreader::Span name;
    xmlreader::XmlReader::Result res;
    res = reader.nextItem(xmlreader::XmlReader::Text::NONE, &name, &nsId);
    assert(res == xmlreader::XmlReader::Result::End);
    (void) res;
}

static OUString lcl_getValue(xmlreader::XmlReader& reader,
                                    const xmlreader::Span& span)
{
    int nsId;
    xmlreader::Span name;
    xmlreader::XmlReader::Result res;
    res = reader.nextItem(xmlreader::XmlReader::Text::NONE, &name, &nsId);
    assert(res == xmlreader::XmlReader::Result::Begin && name == span);
    res = reader.nextItem(xmlreader::XmlReader::Text::Raw, &name, &nsId);
    assert(res == xmlreader::XmlReader::Result::Text);
    (void) res; (void) span;
    OUString sTmp = name.convertFromUtf8();
    lcl_assertEndingItem(reader);
    return sTmp;
}

static Sequence<OUString> lcl_CreatePropertyNames(const OUString& rPrefix)
{
    Sequence<OUString> aProperties(2);
    OUString* pProperties = aProperties.getArray();
    std::fill(aProperties.begin(), aProperties.end(), rPrefix);

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
            xmlreader::XmlReader::Text::NONE, &name, &nsId);
    assert(
        res == xmlreader::XmlReader::Result::Begin
        && name == "manufacturers");
    res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);
    while (res != xmlreader::XmlReader::Result::End)
    {
        // Opening manufacturer
        assert(
            res == xmlreader::XmlReader::Result::Begin
            && name == "manufacturer");
        // Get the name
        (void)reader.nextAttribute(&nsId, &name);
        assert(
            nsId == xmlreader::XmlReader::NAMESPACE_NONE
            && name == "name");
        sManufacturer = reader.getAttributeValue(false).convertFromUtf8();

        for(;;) {
            // Opening label or ending manufacturer
            res = reader.nextItem(
                    xmlreader::XmlReader::Text::NONE, &name, &nsId);
            if (res == xmlreader::XmlReader::Result::End)
                break;
            assert(
                res == xmlreader::XmlReader::Result::Begin
                && name == "label");
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
                xmlreader::XmlReader::Text::NONE, &name, &nsId);
    };
    res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);
    assert(res == xmlreader::XmlReader::Result::Done);

    // add to m_aLabels and m_aManufacturers the custom labels
    const Sequence<OUString>& rMan = GetNodeNames( OUString() );
    for ( const OUString& rManufacturer : rMan )
    {
        const Sequence<OUString> aLabels = GetNodeNames( rManufacturer );
        for( const OUString& rLabel : aLabels )
        {
            OUString sPrefix = rManufacturer + "/" + rLabel + "/";
            Sequence<OUString> aPropNames = lcl_CreatePropertyNames( sPrefix );
            Sequence<Any>   aValues = GetProperties( aPropNames );
            const Any* pValues = aValues.getConstArray();
            if (aValues.getLength() >= 1)
                if(pValues[0].hasValue())
                    pValues[0] >>= sName;
            if (aValues.getLength() >= 2)
                if(pValues[1].hasValue())
                    pValues[1] >>= sMeasure;
            if ( m_aLabels.find( rManufacturer ) == m_aLabels.end() )
                m_aManufacturers.push_back( rManufacturer );
            m_aLabels[rManufacturer][sName].m_aMeasure = sMeasure;
            m_aLabels[rManufacturer][sName].m_bPredefined = false;
        }
    }
}

SwLabelConfig::~SwLabelConfig()
{
}

// the config item is not writable ?:
void SwLabelConfig::ImplCommit() {}

void SwLabelConfig::Notify( const css::uno::Sequence< OUString >& ) {}

static std::unique_ptr<SwLabRec> lcl_CreateSwLabRec(const OUString& rType, const OUString& rMeasure, const OUString& rManufacturer)
{
    std::unique_ptr<SwLabRec> pNewRec(new SwLabRec);
    pNewRec->m_aMake = rManufacturer;
    pNewRec->m_nPWidth = 0;
    pNewRec->m_nPHeight = 0;
    pNewRec->m_aType = rType;
    //all values are contained as colon-separated 1/100 mm values
    //except for the continuous flag ('C'/'S') and nCols, nRows (sal_Int32)
    sal_Int32 nTok{0};
    sal_Int32 nIdx{rMeasure.isEmpty() ? -1 : 0};
    while (nIdx>=0)
    {
        const OUString sToken(rMeasure.getToken(0, ';', nIdx));
        int nVal = sToken.toInt32();
        switch(nTok++)
        {
            case  0 : pNewRec->m_bCont = sToken[0] == 'C'; break;
            case  1 : pNewRec->m_nHDist    = convertMm100ToTwip(nVal);  break;
            case  2 : pNewRec->m_nVDist    = convertMm100ToTwip(nVal);  break;
            case  3 : pNewRec->m_nWidth    = convertMm100ToTwip(nVal);  break;
            case  4 : pNewRec->m_nHeight   = convertMm100ToTwip(nVal);  break;
            case  5 : pNewRec->m_nLeft     = convertMm100ToTwip(nVal);  break;
            case  6 : pNewRec->m_nUpper    = convertMm100ToTwip(nVal);  break;
            case  7 : pNewRec->m_nCols     = nVal;                 break;
            case  8 : pNewRec->m_nRows     = nVal;                 break;
            case  9 : pNewRec->m_nPWidth   = convertMm100ToTwip(nVal);  break;
            case 10 : pNewRec->m_nPHeight  = convertMm100ToTwip(nVal);  break;
        }
    }
    // lines added for compatibility with custom label definitions saved before patch fdo#44516
    if (pNewRec->m_nPWidth == 0 || pNewRec->m_nPHeight == 0)
    {
        // old style definition (no paper dimensions), calculate probable values
        pNewRec->m_nPWidth = 2 * pNewRec->m_nLeft + (pNewRec->m_nCols - 1) * pNewRec->m_nHDist + pNewRec->m_nWidth;
        pNewRec->m_nPHeight = ( pNewRec->m_bCont ? pNewRec->m_nRows * pNewRec->m_nVDist : 2 * pNewRec->m_nUpper + (pNewRec->m_nRows - 1) * pNewRec->m_nVDist + pNewRec->m_nHeight );
    }
    return pNewRec;
}

static Sequence<PropertyValue> lcl_CreateProperties(
    Sequence<OUString> const & rPropNames, OUString& rMeasure, const SwLabRec& rRec)
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
            case 0: pValues[nProp].Value <<= rRec.m_aType; break;
            case 1:
            {
                rMeasure.clear();
                rMeasure += rRec.m_bCont ? OUStringLiteral( u"C" ) : OUStringLiteral( u"S" );      rMeasure += sColon;
                rMeasure += OUString::number( convertTwipToMm100( rRec.m_nHDist ) );   rMeasure += sColon;
                rMeasure += OUString::number( convertTwipToMm100( rRec.m_nVDist ) );   rMeasure += sColon;
                rMeasure += OUString::number( convertTwipToMm100( rRec.m_nWidth ) );   rMeasure += sColon;
                rMeasure += OUString::number( convertTwipToMm100( rRec.m_nHeight ) );  rMeasure += sColon;
                rMeasure += OUString::number( convertTwipToMm100( rRec.m_nLeft ) );    rMeasure += sColon;
                rMeasure += OUString::number( convertTwipToMm100( rRec.m_nUpper ) );   rMeasure += sColon;
                rMeasure += OUString::number( rRec.m_nCols );                     rMeasure += sColon;
                rMeasure += OUString::number( rRec.m_nRows );                     rMeasure += sColon;
                rMeasure += OUString::number( convertTwipToMm100( rRec.m_nPWidth ) );  rMeasure += sColon;
                rMeasure += OUString::number( convertTwipToMm100( rRec.m_nPHeight ) );
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
    for (const auto& rEntry : m_aLabels[rManufacturer])
        rLabArr.push_back( lcl_CreateSwLabRec(rEntry.first, rEntry.second.m_aMeasure, rManufacturer) );
}

bool    SwLabelConfig::HasLabel(const OUString& rManufacturer, const OUString& rType)
{
    return ( ( m_aLabels.find(rManufacturer) != m_aLabels.end() ) &&
             ( m_aLabels[rManufacturer].find(rType) != m_aLabels[rManufacturer].end() ) );
}

// label is always saved as a custom label
// predefined labels can NOT be overwritten by custom labels with same manufacturer/name
void SwLabelConfig::SaveLabel( const OUString& rManufacturer,
        const OUString& rType, const SwLabRec& rRec )
{
    OUString sFoundNode;
    bool bManufacturerNodeFound;
    if ( m_aLabels.find( rManufacturer ) == m_aLabels.end() ||
         !GetNodeNames( rManufacturer ).hasElements() )
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
        sFoundNode = sPrefix + OUString::number( nIndex );
        while ( comphelper::findValue(aLabels, sFoundNode) != -1 )
        {
            sFoundNode = sPrefix + OUString::number(nIndex++);
        }
    }
    else
    {
        // get the appropriate node
        OUString sManufacturer( wrapConfigurationElementName( rManufacturer ) );
        const Sequence<OUString> aLabels = GetNodeNames( sManufacturer );
        for (const OUString& rLabel : aLabels)
        {
            OUString sPrefix = sManufacturer + "/" + rLabel + "/";
            Sequence<OUString> aProperties { sPrefix };
            aProperties.getArray()[0] += "Name";
            Sequence<Any> aValues = GetProperties( aProperties );
            const Any* pValues = aValues.getConstArray();
            if ( pValues[0].hasValue() )
            {
                OUString sTmp;
                pValues[0] >>= sTmp;
                if ( rType == sTmp )
                {
                    sFoundNode = rLabel;
                    break;
                }
            }
        }
    }

    OUString sPrefix = wrapConfigurationElementName( rManufacturer ) +
        "/" + sFoundNode + "/";
    Sequence<OUString> aPropNames = lcl_CreatePropertyNames( sPrefix );
    OUString sMeasure;
    Sequence<PropertyValue> aPropValues = lcl_CreateProperties( aPropNames, sMeasure, rRec );
    SetSetProperties( wrapConfigurationElementName( rManufacturer ), aPropValues );

    //update m_aLabels
    m_aLabels[rManufacturer][rType].m_aMeasure = sMeasure;
    m_aLabels[rManufacturer][rType].m_bPredefined = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
