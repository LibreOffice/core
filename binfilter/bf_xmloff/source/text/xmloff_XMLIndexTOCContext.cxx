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


#ifndef _XMLOFF_XMLINDEXTOCCONTEXT_HXX_
#include "XMLIndexTOCContext.hxx"
#endif






#ifndef _COM_SUN_STAR_TEXT_XRELATIVETEXTCONTENTINSERT_HPP_
#include <com/sun/star/text/XRelativeTextContentInsert.hpp>
#endif


#ifndef _XMLOFF_XMLINDEXTOCSOURCECONTEXT_HXX_
#include "XMLIndexTOCSourceContext.hxx"
#endif

#ifndef _XMLOFF_XMLINDEXOBJECTSOURCECONTEXT_HXX_
#include "XMLIndexObjectSourceContext.hxx"
#endif

#ifndef _XMLOFF_XMLINDEXALPHABETICALSOURCECONTEXT_HXX_
#include "XMLIndexAlphabeticalSourceContext.hxx"
#endif

#ifndef _XMLOFF_XMLINDEXUSERSOURCECONTEXT_HXX_
#include "XMLIndexUserSourceContext.hxx"
#endif

#ifndef _XMLOFF_XMLINDEXBIBLIOGRAPHYSOURCECONTEXT_HXX_
#include "XMLIndexBibliographySourceContext.hxx"
#endif


#ifndef _XMLOFF_XMLINDEXILLUSTRATIONSOURCECONTEXT_HXX_
#include "XMLIndexIllustrationSourceContext.hxx"
#endif

#ifndef _XMLOFF_XMLINDEXBODYCONTEXT_HXX_
#include "XMLIndexBodyContext.hxx"
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

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif


#ifndef _XMLOFF_PRSTYLEI_HXX_ 
#include "prstylei.hxx"
#endif

#ifndef _XMLOFF_XMLERROR_HXX
#include "xmlerror.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif



#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
namespace binfilter {


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::binfilter::xmloff::token;

using ::rtl::OUString;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::lang::IllegalArgumentException;


TYPEINIT1(XMLIndexTOCContext, SvXMLImportContext);

static const sal_Char* aIndexServiceMap[] =
{ 
    "com.sun.star.text.ContentIndex",
    "com.sun.star.text.DocumentIndex",
    "com.sun.star.text.TableIndex",
    "com.sun.star.text.ObjectIndex",
    "com.sun.star.text.Bibliography",
    "com.sun.star.text.UserIndex",
    "com.sun.star.text.IllustrationsIndex"
};

static const sal_Char* aIndexSourceElementMap[] =
{ 
    sXML_table_of_content_source,
    sXML_alphabetical_index_source,
    sXML_table_index_source,
    sXML_object_index_source,
    sXML_bibliography_source,
    sXML_user_index_source,
    sXML_illustration_index_source
};

SvXMLEnumMapEntry __READONLY_DATA aIndexTypeMap[] =
{
    { XML_TABLE_OF_CONTENT,	    TEXT_INDEX_TOC },
    { XML_ALPHABETICAL_INDEX,	TEXT_INDEX_ALPHABETICAL },
    { XML_TABLE_INDEX, 		    TEXT_INDEX_TABLE },
    { XML_OBJECT_INDEX,		    TEXT_INDEX_OBJECT },
    { XML_BIBLIOGRAPHY,		    TEXT_INDEX_BIBLIOGRAPHY },
    { XML_USER_INDEX,			TEXT_INDEX_USER },
    { XML_ILLUSTRATION_INDEX,	TEXT_INDEX_ILLUSTRATION },
    { XML_TOKEN_INVALID,        0 }
};


XMLIndexTOCContext::XMLIndexTOCContext(
    SvXMLImport& rImport, 
    sal_uInt16 nPrfx,
    const OUString& rLocalName ) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        pSourceElementName(NULL),
        bValid(sal_False),
        xBodyContextRef(),
        sTitle(RTL_CONSTASCII_USTRINGPARAM("Title")),
        sIsProtected(RTL_CONSTASCII_USTRINGPARAM("IsProtected")),
        sName(RTL_CONSTASCII_USTRINGPARAM("Name"))
{
    if (XML_NAMESPACE_TEXT == nPrfx)
    {
        sal_uInt16 nTmp;
        if (SvXMLUnitConverter::convertEnum(nTmp, rLocalName, aIndexTypeMap))
        {
            // check for array index: 
            DBG_ASSERT(nTmp >= 0, "index too low");
            DBG_ASSERT(nTmp < (sizeof(aIndexServiceMap)/sizeof(sal_Char*)), 
                       "index too high");
            DBG_ASSERT(sizeof(aIndexServiceMap) == 
                       sizeof(aIndexSourceElementMap), 
                       "service and source element maps must be same size");

            eIndexType = (enum IndexTypeEnum)nTmp;
            pSourceElementName = aIndexSourceElementMap[eIndexType];
            bValid = sal_True;
        }
    }
}

XMLIndexTOCContext::~XMLIndexTOCContext()
{
}

void XMLIndexTOCContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    if (bValid)
    {
        // create table of content (via MultiServiceFactory)
        Reference<XMultiServiceFactory> xFactory(GetImport().GetModel(),
                                                 UNO_QUERY);
        if( xFactory.is() )	
        {
            Reference<XInterface> xIfc = 
                xFactory->createInstance(
                    OUString::createFromAscii(aIndexServiceMap[eIndexType]));
            if( xIfc.is() ) 
            {
                // get Property set
                Reference<XPropertySet> xPropSet(xIfc, UNO_QUERY);
                xTOCPropertySet = xPropSet;

                // insert section
                // a) insert section
                //    The inserted index consists of an empty paragraph 
                //    only, as well as an empty paragraph *after* the index
                // b) insert marker after index, and put Cursor inside of the 
                //    index

                // preliminaries
#ifndef DBG_UTIL
                OUString sMarker(RTL_CONSTASCII_USTRINGPARAM(" "));
#else
                OUString sMarker(RTL_CONSTASCII_USTRINGPARAM("Y"));
#endif
                UniReference<XMLTextImportHelper> rImport = 
                    GetImport().GetTextImport();

                // a) insert index
                Reference<XTextContent> xTextContent(xIfc, UNO_QUERY);
                try
                {
                    GetImport().GetTextImport()->InsertTextContent(
                        xTextContent);
                }
                catch( IllegalArgumentException e )
                {
                    // illegal argument? Then we can't accept indices here!
                    Sequence<OUString> aSeq(1);
                    aSeq[0] = GetLocalName();
                    GetImport().SetError( 
                        XMLERROR_FLAG_ERROR | XMLERROR_NO_INDEX_ALLOWED_HERE,
                        aSeq, e.Message, NULL );

                    // set bValid to false, and return prematurely
                    bValid = false;
                    return;
                }

                // b) insert marker and move cursor
                rImport->InsertString(sMarker);
                rImport->GetCursor()->goLeft(2, sal_False);
            }
        }

        // finally, check for redlines that should start at
        // the section start node
        if( bValid )
            GetImport().GetTextImport()->
                RedlineAdjustStartNodeCursor(sal_True);

        // find text:style-name attribute and set section style
        // find text:protected and set value
        // find text:name and set value (if not empty)
        sal_Int16 nCount = xAttrList->getLength();
        sal_Bool bProtected = sal_False;
        OUString sIndexName;
        for(sal_Int16 nAttr = 0; nAttr < nCount; nAttr++)
        {
            OUString sLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
                GetKeyByAttrName( xAttrList->getNameByIndex(nAttr), 
                                  &sLocalName );
            if ( XML_NAMESPACE_TEXT == nPrefix)
            {
                if ( IsXMLToken( sLocalName, XML_STYLE_NAME ) )
                {
                    XMLPropStyleContext* pStyle = 
                        GetImport().GetTextImport()->FindSectionStyle(
                            xAttrList->getValueByIndex(nAttr));
                    if (pStyle != NULL)
                    {
                        pStyle->FillPropertySet( xTOCPropertySet );
                    }
                }
                else if ( IsXMLToken( sLocalName, XML_PROTECTED ) )
                {
                    sal_Bool bTmp;
                    if ( SvXMLUnitConverter::convertBool( 
                         bTmp, xAttrList->getValueByIndex(nAttr) ) )
                    {
                        bProtected = bTmp;
                    }
                }
                else if ( IsXMLToken( sLocalName, XML_NAME ) )
                {
                    sIndexName = xAttrList->getValueByIndex(nAttr);
                }
            }
        }
        Any aAny;
        aAny.setValue( &bProtected, ::getBooleanCppuType() );
        xTOCPropertySet->setPropertyValue( sIsProtected, aAny );

        if (sIndexName.getLength() > 0)
        {
            aAny <<= sIndexName;
            xTOCPropertySet->setPropertyValue( sName, aAny );
        }
    }
}

void XMLIndexTOCContext::EndElement()
{
    // complete import of index by removing the markers (if the index
    // was actually inserted, that is)
    if( bValid )
    {
        // preliminaries
        OUString sEmpty;
        UniReference<XMLTextImportHelper> rHelper= GetImport().GetTextImport();

        // get rid of last paragraph (unless it's the only paragraph)
        rHelper->GetCursor()->goRight(1, sal_False);
        if( xBodyContextRef.Is() &&
            ((XMLIndexBodyContext*)&xBodyContextRef)->HasContent() )
        {
            rHelper->GetCursor()->goLeft(1, sal_True);
            rHelper->GetText()->insertString(rHelper->GetCursorAsRange(), 
                                             sEmpty, sal_True);
        }

        // and delete second marker
        rHelper->GetCursor()->goRight(1, sal_True);
        rHelper->GetText()->insertString(rHelper->GetCursorAsRange(), 
                                         sEmpty, sal_True);	

        // check for Redlines on our end node
        GetImport().GetTextImport()->RedlineAdjustStartNodeCursor(sal_False);
    }
}

SvXMLImportContext* XMLIndexTOCContext::CreateChildContext( 
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    if (bValid)
    {
        if (XML_NAMESPACE_TEXT == nPrefix)
        {
            if ( IsXMLToken( rLocalName, XML_INDEX_BODY ) )
            {
                pContext = new XMLIndexBodyContext(GetImport(), nPrefix,
                                                   rLocalName);
                if ( !xBodyContextRef.Is() || 
                     !((XMLIndexBodyContext*)&xBodyContextRef)->HasContent() )
                {
                    xBodyContextRef = pContext;
                }
            }
            else if (0 == rLocalName.compareToAscii(pSourceElementName))
            {
                // instantiate source context for the appropriate index type
                switch (eIndexType)
                {
                    case TEXT_INDEX_TOC:
                        pContext = new XMLIndexTOCSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_OBJECT:
                        pContext = new XMLIndexObjectSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_ALPHABETICAL:
                        pContext = new XMLIndexAlphabeticalSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_USER:
                        pContext = new XMLIndexUserSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_BIBLIOGRAPHY:
                        pContext = new XMLIndexBibliographySourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_TABLE:
                        pContext = new XMLIndexTableSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_ILLUSTRATION:
                        pContext = new XMLIndexIllustrationSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    default:
                        DBG_ERROR("index type not implemented");
                        break;
                }
            }
            // else: ignore
        }
        // else: no text: namespace -> ignore
    }
    // else: not valid -> ignore

    // default: ignore
    if (pContext == NULL)
    {
        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName,
                                                          xAttrList);
    }

    return pContext;
}
}//end of namespace binfilter
