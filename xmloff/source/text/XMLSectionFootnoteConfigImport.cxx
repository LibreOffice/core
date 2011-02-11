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
#include "precompiled_xmloff.hxx"
#include "XMLSectionFootnoteConfigImport.hxx"

#ifndef _RTL_USTRING
#include <rtl/ustring.hxx>
#endif
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlprmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmlnumi.hxx>
#include <xmloff/txtprmap.hxx>
#include <tools/debug.hxx>

#include <vector>


using namespace ::xmloff::token;
using namespace ::com::sun::star::style;

using ::rtl::OUString;
using ::std::vector;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;


TYPEINIT1(XMLSectionFootnoteConfigImport, SvXMLImportContext);


XMLSectionFootnoteConfigImport::XMLSectionFootnoteConfigImport(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    vector<XMLPropertyState> & rProps,
    const UniReference<XMLPropertySetMapper> & rMapperRef) :
        SvXMLImportContext(rImport, nPrefix, rLocalName),
        rProperties(rProps),
        rMapper(rMapperRef)
{
}

XMLSectionFootnoteConfigImport::~XMLSectionFootnoteConfigImport()
{
}

void XMLSectionFootnoteConfigImport::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    sal_Bool bEnd = sal_True;   // we're inside the element, so this is true
    sal_Bool bNumOwn = sal_False;
    sal_Bool bNumRestart = sal_False;
    sal_Bool bEndnote = sal_False;
    sal_Int16 nNumRestartAt = 0;
    OUString sNumPrefix;
    OUString sNumSuffix;
    OUString sNumFormat;
    OUString sNumLetterSync;

    // iterate over xattribute list and fill values
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );
        OUString sAttrValue = xAttrList->getValueByIndex(nAttr);

        if (XML_NAMESPACE_TEXT == nPrefix)
        {
            if (IsXMLToken(sLocalName, XML_START_VALUE))
            {
                sal_Int32 nTmp;
                if (SvXMLUnitConverter::convertNumber(nTmp, sAttrValue))
                {
                    nNumRestartAt = static_cast< sal_Int16 >( nTmp ) - 1;
                    bNumRestart = sal_True;
                }
            }
            else if( IsXMLToken( sLocalName, XML_NOTE_CLASS ) )
            {
                if( IsXMLToken( sAttrValue, XML_ENDNOTE ) )
                    bEndnote = sal_True;
            }
        }
        else if (XML_NAMESPACE_STYLE == nPrefix)
        {
            if (IsXMLToken(sLocalName, XML_NUM_PREFIX))
            {
                sNumPrefix = sAttrValue;
                bNumOwn = sal_True;
            }
            else if (IsXMLToken(sLocalName, XML_NUM_SUFFIX))
            {
                sNumSuffix = sAttrValue;
                bNumOwn = sal_True;
            }
            else if (IsXMLToken(sLocalName, XML_NUM_FORMAT))
            {
                sNumFormat = sAttrValue;
                bNumOwn = sal_True;
            }
            else if (IsXMLToken(sLocalName, XML_NUM_LETTER_SYNC))
            {
                sNumLetterSync = sAttrValue;
                bNumOwn = sal_True;
            }
        }
    }

    // OK, now we have all values and can fill the XMLPropertyState vector
    Any aAny;

    aAny.setValue( &bNumOwn, ::getBooleanCppuType() );
    sal_Int32 nIndex = rMapper->FindEntryIndex( bEndnote ?
        CTF_SECTION_ENDNOTE_NUM_OWN : CTF_SECTION_FOOTNOTE_NUM_OWN );
    XMLPropertyState aNumOwn( nIndex, aAny );
    rProperties.push_back( aNumOwn );

    aAny.setValue( &bNumRestart, ::getBooleanCppuType() );
    nIndex = rMapper->FindEntryIndex( bEndnote ?
        CTF_SECTION_ENDNOTE_NUM_RESTART : CTF_SECTION_FOOTNOTE_NUM_RESTART );
    XMLPropertyState aNumRestart( nIndex, aAny );
    rProperties.push_back( aNumRestart );

    aAny <<= nNumRestartAt;
    nIndex = rMapper->FindEntryIndex( bEndnote ?
        CTF_SECTION_ENDNOTE_NUM_RESTART_AT :
        CTF_SECTION_FOOTNOTE_NUM_RESTART_AT );
    XMLPropertyState aNumRestartAtState( nIndex, aAny );
    rProperties.push_back( aNumRestartAtState );

    sal_Int16 nNumType = NumberingType::ARABIC;
    GetImport().GetMM100UnitConverter().convertNumFormat( nNumType,
                                                    sNumFormat,
                                                    sNumLetterSync );
    aAny <<= nNumType;
    nIndex = rMapper->FindEntryIndex( bEndnote ?
        CTF_SECTION_ENDNOTE_NUM_TYPE : CTF_SECTION_FOOTNOTE_NUM_TYPE );
    XMLPropertyState aNumFormatState( nIndex, aAny );
    rProperties.push_back( aNumFormatState );

    aAny <<= sNumPrefix;
    nIndex = rMapper->FindEntryIndex( bEndnote ?
        CTF_SECTION_ENDNOTE_NUM_PREFIX : CTF_SECTION_FOOTNOTE_NUM_PREFIX );
    XMLPropertyState aPrefixState( nIndex, aAny );
    rProperties.push_back( aPrefixState );

    aAny <<= sNumSuffix;
    nIndex = rMapper->FindEntryIndex( bEndnote ?
        CTF_SECTION_ENDNOTE_NUM_SUFFIX : CTF_SECTION_FOOTNOTE_NUM_SUFFIX );
    XMLPropertyState aSuffixState( nIndex, aAny );
    rProperties.push_back( aSuffixState );

    aAny.setValue( &bEnd, ::getBooleanCppuType() );
    nIndex = rMapper->FindEntryIndex( bEndnote ?
        CTF_SECTION_ENDNOTE_END : CTF_SECTION_FOOTNOTE_END );
    XMLPropertyState aEndState( nIndex, aAny );
    rProperties.push_back( aEndState );
}
