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

#include "XMLSectionFootnoteConfigImport.hxx"

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/style/NumberingType.hpp>
#include <sal/log.hxx>
#include <sax/tools/converter.hxx>
#include <utility>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/txtprmap.hxx>


using namespace ::xmloff::token;
using namespace ::com::sun::star::style;

using ::std::vector;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;


XMLSectionFootnoteConfigImport::XMLSectionFootnoteConfigImport(
    SvXMLImport& rImport,
    sal_Int32 /*nElement*/,
    vector<XMLPropertyState> & rProps,
    rtl::Reference<XMLPropertySetMapper> xMapperRef) :
        SvXMLImportContext(rImport),
        rProperties(rProps),
        rMapper(std::move(xMapperRef))
{
}

XMLSectionFootnoteConfigImport::~XMLSectionFootnoteConfigImport()
{
}

void XMLSectionFootnoteConfigImport::startFastElement(
    sal_Int32 /*nElement*/,
    const Reference<css::xml::sax::XFastAttributeList> & xAttrList)
{
    bool bNumOwn = false;
    bool bNumRestart = false;
    bool bEndnote = false;
    sal_Int16 nNumRestartAt = 0;
    OUString sNumPrefix;
    OUString sNumSuffix;
    OUString sNumFormat;
    OUString sNumLetterSync;

    // iterate over xattribute list and fill values
    for (auto &aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        switch(aIter.getToken())
        {
            case XML_ELEMENT(TEXT, XML_START_VALUE):
            {
                sal_Int32 nTmp;
                if (::sax::Converter::convertNumber(nTmp, aIter.toView()))
                {
                    nNumRestartAt = static_cast< sal_Int16 >( nTmp ) - 1;
                    bNumRestart = true;
                }
                break;
            }
            case XML_ELEMENT(TEXT, XML_NOTE_CLASS):
            {
                if( IsXMLToken( aIter, XML_ENDNOTE ) )
                    bEndnote = true;
                break;
            }
            case XML_ELEMENT(STYLE, XML_NUM_PREFIX):
            {
                sNumPrefix = aIter.toString();
                bNumOwn = true;
                break;
            }
            case XML_ELEMENT(STYLE, XML_NUM_SUFFIX):
            {
                sNumSuffix = aIter.toString();
                bNumOwn = true;
                break;
            }
            case XML_ELEMENT(STYLE, XML_NUM_FORMAT):
            {
                sNumFormat = aIter.toString();
                bNumOwn = true;
                break;
            }
            case XML_ELEMENT(STYLE, XML_NUM_LETTER_SYNC):
            {
                sNumLetterSync = aIter.toString();
                bNumOwn = true;
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    // OK, now we have all values and can fill the XMLPropertyState vector

    sal_Int32 nIndex = rMapper->FindEntryIndex( bEndnote ?
        CTF_SECTION_ENDNOTE_NUM_OWN : CTF_SECTION_FOOTNOTE_NUM_OWN );
    XMLPropertyState aNumOwn( nIndex, css::uno::Any(bNumOwn) );
    rProperties.push_back( aNumOwn );

    nIndex = rMapper->FindEntryIndex( bEndnote ?
        CTF_SECTION_ENDNOTE_NUM_RESTART : CTF_SECTION_FOOTNOTE_NUM_RESTART );
    XMLPropertyState aNumRestart( nIndex, css::uno::Any(bNumRestart) );
    rProperties.push_back( aNumRestart );

    nIndex = rMapper->FindEntryIndex( bEndnote ?
        CTF_SECTION_ENDNOTE_NUM_RESTART_AT :
        CTF_SECTION_FOOTNOTE_NUM_RESTART_AT );
    XMLPropertyState aNumRestartAtState( nIndex, css::uno::Any(nNumRestartAt) );
    rProperties.push_back( aNumRestartAtState );

    sal_Int16 nNumType = NumberingType::ARABIC;
    GetImport().GetMM100UnitConverter().convertNumFormat( nNumType,
                                                    sNumFormat,
                                                    sNumLetterSync );
    nIndex = rMapper->FindEntryIndex( bEndnote ?
        CTF_SECTION_ENDNOTE_NUM_TYPE : CTF_SECTION_FOOTNOTE_NUM_TYPE );
    XMLPropertyState aNumFormatState( nIndex, css::uno::Any(nNumType) );
    rProperties.push_back( aNumFormatState );

    nIndex = rMapper->FindEntryIndex( bEndnote ?
        CTF_SECTION_ENDNOTE_NUM_PREFIX : CTF_SECTION_FOOTNOTE_NUM_PREFIX );
    XMLPropertyState aPrefixState( nIndex, css::uno::Any(sNumPrefix) );
    rProperties.push_back( aPrefixState );

    nIndex = rMapper->FindEntryIndex( bEndnote ?
        CTF_SECTION_ENDNOTE_NUM_SUFFIX : CTF_SECTION_FOOTNOTE_NUM_SUFFIX );
    XMLPropertyState aSuffixState( nIndex, css::uno::Any(sNumSuffix) );
    rProperties.push_back( aSuffixState );

    nIndex = rMapper->FindEntryIndex( bEndnote ?
        CTF_SECTION_ENDNOTE_END : CTF_SECTION_FOOTNOTE_END );
    XMLPropertyState aEndState( nIndex, css::uno::Any(true) ); // we're inside the element, so this is true
    rProperties.push_back( aEndState );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
