/*************************************************************************
 *
 *  $RCSfile: XMLSectionImportContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dvo $ $Date: 2000-10-19 10:25:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_TEXTIMP_HXX_
#include "txtimp.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_PRSTYLEI_HXX_
#include "prstylei.hxx"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTCONTENT_HPP_
#include <com/sun/star/text/XTextContent.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_CONTROLCHARACTER_HPP_
#include <com/sun/star/text/ControlCharacter.hpp>
#endif


using ::rtl::OUString;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::container::XNamed;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;

TYPEINIT1( XMLSectionImportContext, SvXMLImportContext );

const sal_Char sAPI_TextSection[] = "com.sun.star.text.TextSection";
const sal_Char sAPI_IsProtected[] = "IsProtected";

enum XMLSectionToken {
    XML_TOK_SECTION_STYLE_NAME,
    XML_TOK_SECTION_NAME,
    XML_TOK_SECTION_PROTECTED
};

static __FAR_DATA SvXMLTokenMapEntry aSectionTokenMap[] =
{
    { XML_NAMESPACE_TEXT, sXML_style_name, XML_TOK_SECTION_STYLE_NAME },
    { XML_NAMESPACE_TEXT, sXML_name, XML_TOK_SECTION_NAME },
    { XML_NAMESPACE_TEXT, sXML_protected, XML_TOK_SECTION_PROTECTED },
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
        sIsProtected(RTL_CONSTASCII_USTRINGPARAM(sAPI_IsProtected)),
        sStyleName(),
        sName(),
        sEmpty(),
        bValid(sal_False),
        bProtected(sal_False)
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
                xFactory->createInstance( sTextSection );
            if (xIfc.is())
            {
                Reference<XPropertySet> xPropSet(xIfc, UNO_QUERY);

                // save PropertySet (for CreateChildContext)
                xSectionPropertySet = xPropSet;

                // name
                Reference<XNamed> xNamed(xPropSet, UNO_QUERY);
                xNamed->setName(sName);

                // protected?
                Any aAny;
                aAny.setValue( &bProtected, ::getBooleanCppuType() );
                xPropSet->setPropertyValue( sIsProtected, aAny );

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

                // insert X and paragraph mark; then insert
                // section over the space character, and delete the
                // last paragraph when closing a section.
                Reference<XTextRange> xStart =
                    rHelper->GetCursor()->getStart();
                OUString sMarkerString(RTL_CONSTASCII_USTRINGPARAM("X"));
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
            }
        }
    }
}

void XMLSectionImportContext::ProcessAttributes(
    const Reference<XAttributeList> & xAttrList )
{
    SvXMLTokenMap aTokenMap(aSectionTokenMap);

    sal_Int32 nLength = xAttrList->getLength();
    for(sal_Int32 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );

        switch (aTokenMap.Get(nPrefix, sLocalName))
        {
            case XML_TOK_SECTION_STYLE_NAME:
                sStyleName = xAttrList->getValueByIndex(nAttr);
                break;
            case XML_TOK_SECTION_NAME:
                sName = xAttrList->getValueByIndex(nAttr);
                bValid = sal_True;
                break;
            case XML_TOK_SECTION_PROTECTED:
                sal_Bool bTmp;
                if (SvXMLUnitConverter::convertBool(bTmp,
                                     xAttrList->getValueByIndex(nAttr)))
                {
                    bProtected = bTmp;
                }
                break;
            default:
                ; // ignore
                break;
        }
    }
}

void XMLSectionImportContext::EndElement()
{
    // get rid of last paragraph
    UniReference<XMLTextImportHelper> rHelper = GetImport().GetTextImport();
    rHelper->GetCursor()->goRight(1, sal_False);
    rHelper->GetCursor()->goLeft(1, sal_True);
    rHelper->GetText()->insertString(rHelper->GetCursorAsRange(),
                                     sEmpty, sal_True);

    // and delete second marker
    rHelper->GetCursor()->goRight(1, sal_True);
    rHelper->GetText()->insertString(rHelper->GetCursorAsRange(),
                                     sEmpty, sal_True);
}

SvXMLImportContext* XMLSectionImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    // section-source (-dde) elements
    if (XML_NAMESPACE_TEXT == nPrefix)
    {
        if (rLocalName.equalsAsciiL(sXML_section_source,
                                    sizeof(sXML_section_source)-1))
        {
            pContext = new XMLSectionSourceImportContext(GetImport(),
                                                         nPrefix, rLocalName,
                                                         xSectionPropertySet);
        }
        else if (rLocalName.equalsAsciiL(sXML_section_source_dde,
                                         sizeof(sXML_section_source_dde)-1))
        {
            pContext = new XMLSectionSourceDDEImportContext(GetImport(),
                                                          nPrefix, rLocalName,
                                                          xSectionPropertySet);
        }
    }

    // otherwise: text context
    if (NULL == pContext)
    {
        pContext = GetImport().GetTextImport()->CreateTextChildContext(
            GetImport(), nPrefix, rLocalName, xAttrList);

        // if that fails, default context
        if (NULL == pContext)
        {
            pContext = new SvXMLImportContext( GetImport(),
                                               nPrefix, rLocalName );
        }
    }

    return pContext;
}

