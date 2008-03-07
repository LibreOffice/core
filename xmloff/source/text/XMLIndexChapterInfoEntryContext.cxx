/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLIndexChapterInfoEntryContext.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 11:52:44 $
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


#ifndef _XMLOFF_XMLINDEXCHAPTERINFOENTRYCONTEXT_HXX_
#include "XMLIndexChapterInfoEntryContext.hxx"
#endif

#ifndef _XMLOFF_XMLINDEXTEMPLATECONTEXT_HXX_
#include "XMLIndexTemplateContext.hxx"
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif

#ifndef _XMLOFF_TEXTIMP_HXX_
#include <xmloff/txtimp.hxx>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_CHAPTERFORMAT_HPP_
#include <com/sun/star/text/ChapterFormat.hpp>
#endif


using namespace ::com::sun::star::text;
using namespace ::xmloff::token;

using ::rtl::OUString;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::PropertyValues;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::com::sun::star::xml::sax::XAttributeList;



TYPEINIT1( XMLIndexChapterInfoEntryContext, XMLIndexSimpleEntryContext);

XMLIndexChapterInfoEntryContext::XMLIndexChapterInfoEntryContext(
    SvXMLImport& rImport,
    XMLIndexTemplateContext& rTemplate,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    sal_Bool bT ) :
        XMLIndexSimpleEntryContext(rImport,
                                   (bT ? rTemplate.sTokenEntryNumber
                                          : rTemplate.sTokenChapterInfo),
                                   rTemplate, nPrfx, rLocalName),
        nChapterInfo(ChapterFormat::NAME_NUMBER),
        bChapterInfoOK(sal_False),
        bTOC( bT ),
        nOutlineLevel( 0 ),
        bOutlineLevelOK(sal_False)
{
}

XMLIndexChapterInfoEntryContext::~XMLIndexChapterInfoEntryContext()
{
}

static const SvXMLEnumMapEntry aChapterDisplayMap[] =
{
    { XML_NAME,                     ChapterFormat::NAME },
    { XML_NUMBER,                   ChapterFormat::NUMBER },
    { XML_NUMBER_AND_NAME,          ChapterFormat::NAME_NUMBER },
// not supported by the template:
//  { XML_PLAIN_NUMBER_AND_NAME,    ChapterFormat::NO_PREFIX_SUFFIX },
    { XML_PLAIN_NUMBER,             ChapterFormat::DIGIT },
    { XML_TOKEN_INVALID,            0 }
};

void XMLIndexChapterInfoEntryContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    // handle both, style name and bibliography info
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );
        if (XML_NAMESPACE_TEXT == nPrefix)
        {
            if ( IsXMLToken( sLocalName, XML_STYLE_NAME ) )
            {
                sCharStyleName = xAttrList->getValueByIndex(nAttr);
                bCharStyleNameOK = sal_True;
            }
            else if ( IsXMLToken( sLocalName, XML_DISPLAY ) )//i53420, always true, in TOC as well
            {
                sal_uInt16 nTmp;
                if (SvXMLUnitConverter::convertEnum(
                    nTmp, xAttrList->getValueByIndex(nAttr),
                    aChapterDisplayMap))
                {
                    nChapterInfo = nTmp;
                    bChapterInfoOK = sal_True;
                }
            }
            else if ( IsXMLToken( sLocalName, XML_OUTLINE_LEVEL ) )
            {
                sal_Int32 nTmp;

                if (SvXMLUnitConverter::convertNumber(nTmp, xAttrList->getValueByIndex(nAttr)))
                {
//control on range is carried out in the UNO level
                    nOutlineLevel = static_cast<sal_uInt16>(nTmp);
                    bOutlineLevelOK = sal_True;
                }
            }
        }
    }

    // if we have a style name, set it!
    if (bCharStyleNameOK)
    {
        nValues++;
    }

    // if we have chaper info, set it!
    if (bChapterInfoOK)
    {
        nValues++;
    }
    if (bOutlineLevelOK)
        nValues++;
}

void XMLIndexChapterInfoEntryContext::FillPropertyValues(
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::PropertyValue> & rValues)
{
    // entry name and (optionally) style name in parent class
    XMLIndexSimpleEntryContext::FillPropertyValues(rValues);

    sal_Int32 nIndex = bCharStyleNameOK ? 2 : 1;

    if( bChapterInfoOK )
    {
        // chapter info field
        rValues[nIndex].Name = rTemplateContext.sChapterFormat;
        Any aAny;
        aAny <<= nChapterInfo;
        rValues[nIndex].Value = aAny;
        nIndex++;
    }
    if( bOutlineLevelOK )
    {
        rValues[nIndex].Name = rTemplateContext.sChapterLevel;
        Any aAny;
        aAny <<= nOutlineLevel;
        rValues[nIndex].Value = aAny;
    }
}
