/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLSectionFootnoteConfigImport.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 11:11:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#ifndef _XMLOFF_XMLSECTIONFOOTNOTECONFIGIMPORT_HXX
#include "XMLSectionFootnoteConfigImport.hxx"
#endif

#ifndef _RTL_USTRING
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_NUMBERINGTYPE_HPP_
#include <com/sun/star/style/NumberingType.hpp>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include "xmlprmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_PROPMAPPINGTYPES_HXX
#include "maptype.hxx"
#endif

#ifndef _XMLOFF_XMLNUMI_HXX
#include "xmlnumi.hxx"
#endif

#ifndef _XMLOFF_TEXTPRMAP_HXX_
#include "txtprmap.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

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
