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

#include <tools/debug.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <rtl/ustrbuf.hxx>

#include <xmloff/xmlmetae.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"

#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/sax/XSAXSerializable.hpp>

#include <sax/tools/converter.hxx>

#include <comphelper/sequenceasvector.hxx>
#include <unotools/docinfohelper.hxx>

#include <string.h>


using namespace com::sun::star;
using namespace ::xmloff::token;


//-------------------------------------------------------------------------

static void lcl_AddTwoDigits( OUStringBuffer& rStr, sal_Int32 nVal )
{
    if ( nVal < 10 )
        rStr.append( sal_Unicode('0') );
    rStr.append( nVal );
}

OUString
SvXMLMetaExport::GetISODateTimeString( const util::DateTime& rDateTime )
{
    //  return ISO date string "YYYY-MM-DDThh:mm:ss"

    OUStringBuffer sTmp;
    sTmp.append( (sal_Int32) rDateTime.Year );
    sTmp.append( sal_Unicode('-') );
    lcl_AddTwoDigits( sTmp, rDateTime.Month );
    sTmp.append( sal_Unicode('-') );
    lcl_AddTwoDigits( sTmp, rDateTime.Day );
    sTmp.append( sal_Unicode('T') );
    lcl_AddTwoDigits( sTmp, rDateTime.Hours );
    sTmp.append( sal_Unicode(':') );
    lcl_AddTwoDigits( sTmp, rDateTime.Minutes );
    sTmp.append( sal_Unicode(':') );
    lcl_AddTwoDigits( sTmp, rDateTime.Seconds );

    return sTmp.makeStringAndClear();
}

//-------------------------------------------------------------------------

void SvXMLMetaExport::SimpleStringElement( const OUString& rText,
        sal_uInt16 nNamespace, enum XMLTokenEnum eElementName )
{
    if ( !rText.isEmpty() ) {
        SvXMLElementExport aElem( mrExport, nNamespace, eElementName,
                                  sal_True, sal_False );
        mrExport.Characters( rText );
    }
}

void SvXMLMetaExport::SimpleDateTimeElement( const util::DateTime & rDate,
        sal_uInt16 nNamespace, enum XMLTokenEnum eElementName )
{
    if (rDate.Month != 0) { // invalid dates are 0-0-0
        OUString sValue = GetISODateTimeString( rDate );
        if ( !sValue.isEmpty() ) {
            SvXMLElementExport aElem( mrExport, nNamespace, eElementName,
                                      sal_True, sal_False );
            mrExport.Characters( sValue );
        }
    }
}

void SvXMLMetaExport::_MExport()
{
    //  generator
    {
        SvXMLElementExport aElem( mrExport, XML_NAMESPACE_META, XML_GENERATOR,
                                  sal_True, sal_True );
        mrExport.Characters( ::utl::DocInfoHelper::GetGeneratorString() );
    }

    //  document title
    SimpleStringElement  ( mxDocProps->getTitle(),
                           XML_NAMESPACE_DC, XML_TITLE );

    //  description
    SimpleStringElement  ( mxDocProps->getDescription(),
                           XML_NAMESPACE_DC, XML_DESCRIPTION );

    //  subject
    SimpleStringElement  ( mxDocProps->getSubject(),
                           XML_NAMESPACE_DC, XML_SUBJECT );

    //  created...
    SimpleStringElement  ( mxDocProps->getAuthor(),
                           XML_NAMESPACE_META, XML_INITIAL_CREATOR );
    SimpleDateTimeElement( mxDocProps->getCreationDate(),
                           XML_NAMESPACE_META, XML_CREATION_DATE );

    //  modified...
    SimpleStringElement  ( mxDocProps->getModifiedBy(),
                           XML_NAMESPACE_DC, XML_CREATOR );
    SimpleDateTimeElement( mxDocProps->getModificationDate(),
                           XML_NAMESPACE_DC, XML_DATE );

    //  printed...
    SimpleStringElement  ( mxDocProps->getPrintedBy(),
                           XML_NAMESPACE_META, XML_PRINTED_BY );
    SimpleDateTimeElement( mxDocProps->getPrintDate(),
                           XML_NAMESPACE_META, XML_PRINT_DATE );

    //  keywords
    const uno::Sequence< OUString > keywords = mxDocProps->getKeywords();
    for (sal_Int32 i = 0; i < keywords.getLength(); ++i) {
        SvXMLElementExport aKwElem( mrExport, XML_NAMESPACE_META, XML_KEYWORD,
                                    sal_True, sal_False );
        mrExport.Characters( keywords[i] );
    }

    //  document language
    {
        OUString sValue = LanguageTag( mxDocProps->getLanguage()).getBcp47( false);
        if (!sValue.isEmpty()) {
            SvXMLElementExport aElem( mrExport, XML_NAMESPACE_DC, XML_LANGUAGE,
                                      sal_True, sal_False );
            mrExport.Characters( sValue );
        }
    }

    //  editing cycles
    {
        SvXMLElementExport aElem( mrExport,
                                  XML_NAMESPACE_META, XML_EDITING_CYCLES,
                                  sal_True, sal_False );
        mrExport.Characters( OUString::valueOf(
            static_cast<sal_Int32>(mxDocProps->getEditingCycles()) ) );
    }

    //  editing duration
    //  property is a int32 (seconds)
    {
        sal_Int32 secs = mxDocProps->getEditingDuration();
        SvXMLElementExport aElem( mrExport,
                                  XML_NAMESPACE_META, XML_EDITING_DURATION,
                                  sal_True, sal_False );
        OUStringBuffer buf;
        ::sax::Converter::convertDuration(buf, util::Duration(
                    false, 0, 0, 0, secs/3600, (secs%3600)/60, secs%60, 0));
        mrExport.Characters(buf.makeStringAndClear());
    }

    //  default target
    const OUString sDefTarget = mxDocProps->getDefaultTarget();
    if ( !sDefTarget.isEmpty() )
    {
        mrExport.AddAttribute( XML_NAMESPACE_OFFICE, XML_TARGET_FRAME_NAME,
                               sDefTarget );

        //! define strings for xlink:show values
        const XMLTokenEnum eShow = sDefTarget == "_blank" ? XML_NEW : XML_REPLACE;
        mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, eShow );

        SvXMLElementExport aElem( mrExport,
                                  XML_NAMESPACE_META,XML_HYPERLINK_BEHAVIOUR,
                                  sal_True, sal_False );
    }

    //  auto-reload
    const OUString sReloadURL = mxDocProps->getAutoloadURL();
    const sal_Int32 sReloadDelay = mxDocProps->getAutoloadSecs();
    if (sReloadDelay != 0 || !sReloadURL.isEmpty())
    {
        mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF,
                              mrExport.GetRelativeReference( sReloadURL ) );

        OUStringBuffer buf;
        ::sax::Converter::convertDuration(buf, util::Duration(false, 0, 0, 0,
                sReloadDelay/3600, (sReloadDelay%3600)/60, sReloadDelay%60, 0));
        mrExport.AddAttribute( XML_NAMESPACE_META, XML_DELAY,
            buf.makeStringAndClear());

        SvXMLElementExport aElem( mrExport, XML_NAMESPACE_META, XML_AUTO_RELOAD,
                                  sal_True, sal_False );
    }

    //  template
    const OUString sTplPath = mxDocProps->getTemplateURL();
    if ( !sTplPath.isEmpty() )
    {
        mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONREQUEST );

        //  template URL
        mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF,
                              mrExport.GetRelativeReference(sTplPath) );

        //  template name
        mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TITLE,
                              mxDocProps->getTemplateName() );

        //  template date
        mrExport.AddAttribute( XML_NAMESPACE_META, XML_DATE,
                GetISODateTimeString( mxDocProps->getTemplateDate() ) );

        SvXMLElementExport aElem( mrExport, XML_NAMESPACE_META, XML_TEMPLATE,
                                  sal_True, sal_False );
    }

    //  user defined fields
    uno::Reference< beans::XPropertyAccess > xUserDefined(
        mxDocProps->getUserDefinedProperties(), uno::UNO_QUERY_THROW);
    const uno::Sequence< beans::PropertyValue > props =
        xUserDefined->getPropertyValues();
    for (sal_Int32 i = 0; i < props.getLength(); ++i) {
        OUStringBuffer sValueBuffer;
        OUStringBuffer sType;
        if (!::sax::Converter::convertAny(sValueBuffer, sType, props[i].Value))
        {
            continue;
        }
        mrExport.AddAttribute( XML_NAMESPACE_META, XML_NAME, props[i].Name );
        mrExport.AddAttribute( XML_NAMESPACE_META, XML_VALUE_TYPE,
                              sType.makeStringAndClear() );
        SvXMLElementExport aElem( mrExport, XML_NAMESPACE_META,
                                  XML_USER_DEFINED, sal_True, sal_False );
        mrExport.Characters( sValueBuffer.makeStringAndClear() );
    }

    const uno::Sequence< beans::NamedValue > aDocStatistic =
            mxDocProps->getDocumentStatistics();
    // write document statistic if there is any provided
    if ( aDocStatistic.getLength() )
    {
        for ( sal_Int32 nInd = 0; nInd < aDocStatistic.getLength(); nInd++ )
        {
            sal_Int32 nValue = 0;
            if ( aDocStatistic[nInd].Value >>= nValue )
            {
                OUString aValue = OUString::valueOf( nValue );
                if ( aDocStatistic[nInd].Name == "TableCount" )
                    mrExport.AddAttribute(
                        XML_NAMESPACE_META, XML_TABLE_COUNT, aValue );
                else if ( aDocStatistic[nInd].Name == "ObjectCount" )
                    mrExport.AddAttribute(
                        XML_NAMESPACE_META, XML_OBJECT_COUNT, aValue );
                else if ( aDocStatistic[nInd].Name == "ImageCount" )
                    mrExport.AddAttribute(
                        XML_NAMESPACE_META, XML_IMAGE_COUNT, aValue );
                else if ( aDocStatistic[nInd].Name == "PageCount" )
                    mrExport.AddAttribute(
                        XML_NAMESPACE_META, XML_PAGE_COUNT, aValue );
                else if ( aDocStatistic[nInd].Name == "ParagraphCount" )
                    mrExport.AddAttribute(
                        XML_NAMESPACE_META, XML_PARAGRAPH_COUNT, aValue );
                else if ( aDocStatistic[nInd].Name == "WordCount" )
                    mrExport.AddAttribute(
                        XML_NAMESPACE_META, XML_WORD_COUNT, aValue );
                else if ( aDocStatistic[nInd].Name == "CharacterCount" )
                    mrExport.AddAttribute(
                        XML_NAMESPACE_META, XML_CHARACTER_COUNT, aValue );
                else if ( aDocStatistic[nInd].Name == "CellCount" )
                    mrExport.AddAttribute(
                        XML_NAMESPACE_META, XML_CELL_COUNT, aValue );
                else
                {
                    DBG_ASSERT( sal_False, "Unknown statistic value!\n" );
                }
            }
        }
        SvXMLElementExport aElem( mrExport,
            XML_NAMESPACE_META, XML_DOCUMENT_STATISTIC, sal_True, sal_True );
    }
}

//-------------------------------------------------------------------------

static const char *s_xmlns  = "xmlns";
static const char *s_xmlns2 = "xmlns:";
static const char *s_meta   = "meta:";
static const char *s_href   = "xlink:href";

SvXMLMetaExport::SvXMLMetaExport(
        SvXMLExport& i_rExp,
        const uno::Reference<document::XDocumentProperties>& i_rDocProps ) :
    mrExport( i_rExp ),
    mxDocProps( i_rDocProps ),
    m_level( 0 ),
    m_preservedNSs()
{
    DBG_ASSERT( mxDocProps.is(), "no document properties" );
}

SvXMLMetaExport::~SvXMLMetaExport()
{
}

void SvXMLMetaExport::Export()
{
    uno::Reference< xml::sax::XSAXSerializable> xSAXable(mxDocProps,
        uno::UNO_QUERY);
    if (xSAXable.is()) {
        ::comphelper::SequenceAsVector< beans::StringPair > namespaces;
        const SvXMLNamespaceMap & rNsMap(mrExport.GetNamespaceMap());
        for (sal_uInt16 key = rNsMap.GetFirstKey();
             key != USHRT_MAX; key = rNsMap.GetNextKey(key)) {
            beans::StringPair ns;
            const OUString attrname = rNsMap.GetAttrNameByKey(key);
            if (attrname.matchAsciiL(s_xmlns2, strlen(s_xmlns2))) {
                ns.First  = attrname.copy(strlen(s_xmlns2));
            } else if (attrname.equalsAsciiL(s_xmlns, strlen(s_xmlns))) {
                // default initialized empty string
            } else {
            OSL_FAIL("namespace attribute not starting with xmlns unexpected");
            }
            ns.Second = rNsMap.GetNameByKey(key);
            namespaces.push_back(ns);
        }
        xSAXable->serialize(this, namespaces.getAsConstList());
    } else {
        // office:meta
        SvXMLElementExport aElem( mrExport, XML_NAMESPACE_OFFICE, XML_META,
                                  sal_True, sal_True );
        // fall back to using public interface of XDocumentProperties
        _MExport();
    }
}

// ::com::sun::star::xml::sax::XDocumentHandler:
void SAL_CALL
SvXMLMetaExport::startDocument()
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    // ignore: has already been done by SvXMLExport::exportDoc
    DBG_ASSERT( m_level == 0, "SvXMLMetaExport: level error" );
}

void SAL_CALL
SvXMLMetaExport::endDocument()
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    // ignore: will be done by SvXMLExport::exportDoc
    DBG_ASSERT( m_level == 0, "SvXMLMetaExport: level error" );
}

// unfortunately, this method contains far too much ugly namespace mangling.
void SAL_CALL
SvXMLMetaExport::startElement(const OUString & i_rName,
    const uno::Reference< xml::sax::XAttributeList > & i_xAttribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{

    if (m_level == 0) {
        // namepace decls: default ones have been written at the root element
        // non-default ones must be preserved here
        const sal_Int16 nCount = i_xAttribs->getLength();
        for (sal_Int16 i = 0; i < nCount; ++i) {
            const OUString name(i_xAttribs->getNameByIndex(i));
            if (name.matchAsciiL(s_xmlns, strlen(s_xmlns))) {
                bool found(false);
                const SvXMLNamespaceMap & rNsMap(mrExport.GetNamespaceMap());
                for (sal_uInt16 key = rNsMap.GetFirstKey();
                     key != USHRT_MAX; key = rNsMap.GetNextKey(key)) {
                    if (name.equals(rNsMap.GetAttrNameByKey(key))) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    m_preservedNSs.push_back(beans::StringPair(name,
                        i_xAttribs->getValueByIndex(i)));
                }
            }
        }
        // ignore the root: it has been written already
        ++m_level;
        return;
    }

    if (m_level == 1) {
        // attach preserved namespace decls from root node here
        for (std::vector<beans::StringPair>::const_iterator iter =
                m_preservedNSs.begin(); iter != m_preservedNSs.end(); ++iter) {
            const OUString ns(iter->First);
            bool found(false);
            // but only if it is not already there
            const sal_Int16 nCount = i_xAttribs->getLength();
            for (sal_Int16 i = 0; i < nCount; ++i) {
                const OUString name(i_xAttribs->getNameByIndex(i));
                if (ns.equals(name)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                mrExport.AddAttribute(ns, iter->Second);
            }
        }
    }

    // attach the attributes
    if (i_rName.matchAsciiL(s_meta, strlen(s_meta))) {
        // special handling for all elements that may have
        // xlink:href attributes; these must be made relative
        const sal_Int16 nLength = i_xAttribs->getLength();
        for (sal_Int16 i = 0; i < nLength; ++i) {
            const OUString name (i_xAttribs->getNameByIndex (i));
            OUString value(i_xAttribs->getValueByIndex(i));
            if (name.matchAsciiL(s_href, strlen(s_href))) {
                value = mrExport.GetRelativeReference(value);
            }
            mrExport.AddAttribute(name, value);
        }
    } else {
        const sal_Int16 nLength = i_xAttribs->getLength();
        for (sal_Int16 i = 0; i < nLength; ++i) {
            const OUString name  (i_xAttribs->getNameByIndex(i));
            const OUString value (i_xAttribs->getValueByIndex(i));
            mrExport.AddAttribute(name, value);
        }
    }

    // finally, start the element
    // #i107240# no whitespace here, because the DOM may already contain
    // whitespace, which is not cleared when loading and thus accumulates.
    mrExport.StartElement(i_rName, (m_level > 1) ? sal_False : sal_True);
    ++m_level;
}

void SAL_CALL
SvXMLMetaExport::endElement(const OUString & i_rName)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    --m_level;
    if (m_level == 0) {
        // ignore the root; see startElement
        return;
    }
    DBG_ASSERT( m_level >= 0, "SvXMLMetaExport: level error" );
    mrExport.EndElement(i_rName, sal_False);
}

void SAL_CALL
SvXMLMetaExport::characters(const OUString & i_rChars)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    mrExport.Characters(i_rChars);
}

void SAL_CALL
SvXMLMetaExport::ignorableWhitespace(const OUString & /*i_rWhitespaces*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    mrExport.IgnorableWhitespace(/*i_rWhitespaces*/);
}

void SAL_CALL
SvXMLMetaExport::processingInstruction(const OUString & i_rTarget,
    const OUString & i_rData)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    // ignore; the exporter cannot handle these
    (void) i_rTarget;
    (void) i_rData;
}

void SAL_CALL
SvXMLMetaExport::setDocumentLocator(const uno::Reference<xml::sax::XLocator>&)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    // nothing to do here, move along...
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
