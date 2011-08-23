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

#ifndef _XMLOFF_XMLSECTIONIMPORTCONTEXT_HXX_
#include "XMLSectionImportContext.hxx"
#endif

#ifndef _XMLOFF_XMLSECTIONSOURCEIMPORTCONTEXT_HXX_
#include "XMLSectionSourceImportContext.hxx"
#endif

#ifndef _XMLOFF_XMLSECTIONSOURCEDDEIMPORTCONTEXT_HXX_
#include "XMLSectionSourceDDEImportContext.hxx"
#endif


#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif


#ifndef _XMLOFF_NMSPMAP_HXX 
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif


#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_PRSTYLEI_HXX_ 
#include "prstylei.hxx"
#endif


#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_ 
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTCONTENT_HPP_
#include <com/sun/star/text/XTextContent.hpp>
#endif



#ifndef _COM_SUN_STAR_TEXT_CONTROLCHARACTER_HPP_ 
#include <com/sun/star/text/ControlCharacter.hpp>
#endif
namespace binfilter {


using ::rtl::OUString;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::container::XNamed;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::binfilter::xmloff::token;


TYPEINIT1( XMLSectionImportContext, SvXMLImportContext );

const sal_Char sAPI_TextSection[] = "com.sun.star.text.TextSection";
const sal_Char sAPI_IndexHeaderSection[] = "com.sun.star.text.IndexHeaderSection";
const sal_Char sAPI_IsProtected[] = "IsProtected";
const sal_Char sAPI_Condition[] = "Condition";
const sal_Char sAPI_IsVisible[] = "IsVisible";
const sal_Char sAPI_IsCurrentlyVisible[] = "IsCurrentlyVisible";
const sal_Char sAPI_ProtectionKey[] = "ProtectionKey";

enum XMLSectionToken 
{
    XML_TOK_SECTION_STYLE_NAME,
    XML_TOK_SECTION_NAME,
    XML_TOK_SECTION_CONDITION,
    XML_TOK_SECTION_DISPLAY,
    XML_TOK_SECTION_PROTECT,
    XML_TOK_SECTION_PROTECTION_KEY,
    XML_TOK_SECTION_IS_HIDDEN
};

static __FAR_DATA SvXMLTokenMapEntry aSectionTokenMap[] =
{
    { XML_NAMESPACE_TEXT, XML_STYLE_NAME, XML_TOK_SECTION_STYLE_NAME },
    { XML_NAMESPACE_TEXT, XML_NAME, XML_TOK_SECTION_NAME },
    { XML_NAMESPACE_TEXT, XML_CONDITION, XML_TOK_SECTION_CONDITION },
    { XML_NAMESPACE_TEXT, XML_DISPLAY, XML_TOK_SECTION_DISPLAY },
    { XML_NAMESPACE_TEXT, XML_PROTECTED, XML_TOK_SECTION_PROTECT },
    { XML_NAMESPACE_TEXT, XML_PROTECTION_KEY, XML_TOK_SECTION_PROTECTION_KEY},
    { XML_NAMESPACE_TEXT, XML_IS_HIDDEN, XML_TOK_SECTION_IS_HIDDEN },
    // compatibility with SRC629 (or earlier) versions
    { XML_NAMESPACE_TEXT, XML_PROTECT, XML_TOK_SECTION_PROTECT },
    XML_TOKEN_MAP_END
};


// section import: This one is fairly tricky due to a variety of
// limits of the core or the API. The main problem is that if you
// insert a section within another section, you can't move the cursor
// between the ends of the inner and the enclosing section. To avoid
// these problems, additional markers are first inserted and later deleted.
XMLSectionImportContext::XMLSectionImportContext(
    SvXMLImport& rImport, 
    sal_uInt16 nPrfx,
    const OUString& rLocalName ) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        xStartRange(),
        xEndRange(),
        xSectionPropertySet(),
        sTextSection(RTL_CONSTASCII_USTRINGPARAM(sAPI_TextSection)),
        sIndexHeaderSection(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_IndexHeaderSection)),
        sCondition(RTL_CONSTASCII_USTRINGPARAM(sAPI_Condition)),
        sIsVisible(RTL_CONSTASCII_USTRINGPARAM(sAPI_IsVisible)),
        sProtectionKey(RTL_CONSTASCII_USTRINGPARAM(sAPI_ProtectionKey)),
        sIsProtected(RTL_CONSTASCII_USTRINGPARAM(sAPI_IsProtected)),
        sIsCurrentlyVisible(RTL_CONSTASCII_USTRINGPARAM(sAPI_IsCurrentlyVisible)),
        sStyleName(),
        sName(),
        sCond(),
        sEmpty(),
        bValid(sal_False),
        bCondOK(sal_False),
        bIsVisible(sal_True),
        bSequenceOK(sal_False),
        bProtect(sal_False),
        bHasContent(sal_False),
        bIsCurrentlyVisible(sal_True),
        bIsCurrentlyVisibleOK(sal_False)
{
}

XMLSectionImportContext::~XMLSectionImportContext()
{
}

void XMLSectionImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    // process attributes
    ProcessAttributes(xAttrList);

    // process index headers:
    sal_Bool bIsIndexHeader = IsXMLToken( GetLocalName(), XML_INDEX_TITLE );
    if (bIsIndexHeader)
    {
        bValid = sal_True;
    }

    UniReference<XMLTextImportHelper> rHelper = GetImport().GetTextImport();

    // valid?
    if (bValid)
    {
        // create text section (as XPropertySet)
        Reference<XMultiServiceFactory> xFactory(
            GetImport().GetModel(),UNO_QUERY);
        if (xFactory.is())
        {
            Reference<XInterface> xIfc = 
                xFactory->createInstance( bIsIndexHeader ? sIndexHeaderSection 
                                                        : sTextSection );
            if (xIfc.is()) 
            {
                Reference<XPropertySet> xPropSet(xIfc, UNO_QUERY);

                // save PropertySet (for CreateChildContext)
                xSectionPropertySet = xPropSet;

                // name
                Reference<XNamed> xNamed(xPropSet, UNO_QUERY);
                xNamed->setName(sName);

                // stylename?
                if (sStyleName.getLength() > 0)
                {
                    XMLPropStyleContext* pStyle = rHelper->
                        FindSectionStyle(sStyleName);
                    
                    if (pStyle != NULL)
                    {
                        pStyle->FillPropertySet( xPropSet );
                    }
                }

                // IsVisible and condition (not for index headers)
                if (! bIsIndexHeader)
                {
                    Any aAny;
                    aAny.setValue( &bIsVisible, ::getBooleanCppuType() );
                    xPropSet->setPropertyValue( sIsVisible, aAny );

                    // #97450# hidden sections must be hidden on reload
                    // For backwards compatibilty, set flag only if it is 
                    // present
                    if( bIsCurrentlyVisibleOK )
                    {
                        aAny.setValue( &bIsCurrentlyVisible, 
                                       ::getBooleanCppuType() );
                        xPropSet->setPropertyValue( sIsCurrentlyVisible, aAny);
                    }

                    if (bCondOK)
                    {
                        aAny <<= sCond;
                        xPropSet->setPropertyValue( sCondition, aAny );
                    }
                }

                // password (only for regular sections)
                if ( bSequenceOK &&
                     IsXMLToken(GetLocalName(), XML_SECTION) )
                {
                    Any aAny;
                    aAny <<= aSequence;
                    xPropSet->setPropertyValue(sProtectionKey, aAny);
                }

                // protection
                Any aAny;
                aAny.setValue( &bProtect, ::getBooleanCppuType() );
                xPropSet->setPropertyValue( sIsProtected, aAny );

                // insert marker, <paragraph>, marker; then insert
                // section over the first marker character, and delete the
                // last paragraph (and marker) when closing a section.
                Reference<XTextRange> xStart = 
                    rHelper->GetCursor()->getStart();
#ifndef DBG_UTIL
                static const sal_Char sMarker[] = " ";
#else
                static const sal_Char sMarker[] = "X";
#endif
                OUString sMarkerString(RTL_CONSTASCII_USTRINGPARAM(sMarker));
                rHelper->InsertString(sMarkerString);
                rHelper->InsertControlCharacter( 
                    ControlCharacter::APPEND_PARAGRAPH );
                rHelper->InsertString(sMarkerString);
                
                // select first marker
                rHelper->GetCursor()->gotoRange(xStart, sal_False);
                rHelper->GetCursor()->goRight(1, sal_True);

                // convert section to XTextContent
                Reference<XTextContent> xTextContent(xSectionPropertySet, 
                                                     UNO_QUERY);

                // and insert (over marker)
                rHelper->GetText()->insertTextContent( 
                    rHelper->GetCursorAsRange(), xTextContent, sal_True );

                // and delete first marker (in section)
                rHelper->GetText()->insertString( 
                    rHelper->GetCursorAsRange(), sEmpty, sal_True);

                // finally, check for redlines that should start at
                // the section start node
                rHelper->RedlineAdjustStartNodeCursor(sal_True); // start ???
            }
        }
    }
}

void XMLSectionImportContext::ProcessAttributes(
    const Reference<XAttributeList> & xAttrList )
{
    SvXMLTokenMap aTokenMap(aSectionTokenMap);
    
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr), 
                              &sLocalName );
        OUString sAttr = xAttrList->getValueByIndex(nAttr);

        switch (aTokenMap.Get(nPrefix, sLocalName))
        {
            case XML_TOK_SECTION_STYLE_NAME:
                sStyleName = sAttr;
                break;
            case XML_TOK_SECTION_NAME:
                sName = sAttr;
                bValid = sal_True;
                break;
            case XML_TOK_SECTION_CONDITION:
                sCond = sAttr;
                bCondOK = sal_True;
                break;
            case XML_TOK_SECTION_DISPLAY:
                if (IsXMLToken(sAttr, XML_TRUE))
                {
                    bIsVisible = sal_True;
                }
                else if ( IsXMLToken(sAttr, XML_NONE) ||
                          IsXMLToken(sAttr, XML_CONDITION) )
                {
                    bIsVisible = sal_False;
                }
                // else: ignore
                break;
            case XML_TOK_SECTION_IS_HIDDEN:
                {
                    sal_Bool bTmp;
                    if (SvXMLUnitConverter::convertBool(bTmp, sAttr))
                    {
                        bIsCurrentlyVisible = !bTmp;
                        bIsCurrentlyVisibleOK = sal_True;
                    }
                }
                break;
            case XML_TOK_SECTION_PROTECTION_KEY:
                SvXMLUnitConverter::decodeBase64(aSequence, sAttr);
                bSequenceOK = sal_True;
                break;
            case XML_TOK_SECTION_PROTECT:
            {
                sal_Bool bTmp;
                if (SvXMLUnitConverter::convertBool(bTmp, sAttr))
                {
                    bProtect = bTmp;
                }
                break;
            }
            default:
                ; // ignore
                break;
        }
    }
}

void XMLSectionImportContext::EndElement()
{
    // get rid of last paragraph 
    // (unless it's the only paragraph in the section)
    UniReference<XMLTextImportHelper> rHelper = GetImport().GetTextImport();
    rHelper->GetCursor()->goRight(1, sal_False);
    if (bHasContent)
    {
        rHelper->GetCursor()->goLeft(1, sal_True);
        rHelper->GetText()->insertString(rHelper->GetCursorAsRange(), 
                                         sEmpty, sal_True);
    }

    // and delete second marker
    rHelper->GetCursor()->goRight(1, sal_True);
    rHelper->GetText()->insertString(rHelper->GetCursorAsRange(), 
                                     sEmpty, sal_True);	

    // check for redlines to our endnode
    rHelper->RedlineAdjustStartNodeCursor(sal_False);
}

SvXMLImportContext* XMLSectionImportContext::CreateChildContext( 
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    // section-source (-dde) elements
    if ( (XML_NAMESPACE_TEXT == nPrefix) &&
         IsXMLToken(rLocalName, XML_SECTION_SOURCE) )
    {
        pContext = new XMLSectionSourceImportContext(GetImport(),
                                                     nPrefix, rLocalName, 
                                                     xSectionPropertySet);
    } 
    else if ( (XML_NAMESPACE_OFFICE == nPrefix) &&
              IsXMLToken(rLocalName, XML_DDE_SOURCE) )
    {
        pContext = new XMLSectionSourceDDEImportContext(GetImport(),
                                                        nPrefix, rLocalName,
                                                        xSectionPropertySet);
    }
    else
    {
        // otherwise: text context
        pContext = GetImport().GetTextImport()->CreateTextChildContext(
            GetImport(), nPrefix, rLocalName, xAttrList,
            XML_TEXT_TYPE_SECTION );

        // if that fails, default context
        if (NULL == pContext)
        {
            pContext = new SvXMLImportContext( GetImport(), 
                                               nPrefix, rLocalName );
        }
        else
            bHasContent = sal_True;
    }

    return pContext;
}

}//end of namespace binfilter
