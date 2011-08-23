/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include "XMLTextMarkImportContext.hxx"

#include "rtl/ustring.hxx"

#include <tools/debug.hxx>

#include "xmluconv.hxx"


#include "xmlimp.hxx"

#include "nmspmap.hxx"

#include "xmlnmspe.hxx"


#include <com/sun/star/text/XTextContent.hpp>



#include <com/sun/star/container/XNamed.hpp>
namespace binfilter {


using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::xml::sax;
using namespace ::binfilter::xmloff::token;

using rtl::OUString;
    
TYPEINIT1( XMLTextMarkImportContext, SvXMLImportContext);

XMLTextMarkImportContext::XMLTextMarkImportContext(
    SvXMLImport& rImport, 
    XMLTextImportHelper& rHlp,
    sal_uInt16 nPrefix,
    const OUString& rLocalName ) :
        SvXMLImportContext(rImport, nPrefix, rLocalName),
        rHelper(rHlp)
{
}

enum lcl_MarkType { TypeReference, TypeReferenceStart, TypeReferenceEnd,
                    TypeBookmark, TypeBookmarkStart, TypeBookmarkEnd };

static SvXMLEnumMapEntry __READONLY_DATA lcl_aMarkTypeMap[] =
{
    { XML_REFERENCE_MARK,			TypeReference },
    { XML_REFERENCE_MARK_START,	    TypeReferenceStart },
    { XML_REFERENCE_MARK_END,		TypeReferenceEnd },
    { XML_BOOKMARK,				    TypeBookmark },
    { XML_BOOKMARK_START,			TypeBookmarkStart },
    { XML_BOOKMARK_END,			    TypeBookmarkEnd },
    { XML_TOKEN_INVALID,    		0 },
};

void XMLTextMarkImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    const OUString sAPI_reference_mark(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.ReferenceMark"));
    const OUString sAPI_bookmark(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.Bookmark"));


    OUString sName;

    if (FindName(GetImport(), xAttrList, sName))
    {
        sal_uInt16 nTmp;
        if (SvXMLUnitConverter::convertEnum(nTmp, GetLocalName(), 
                                            lcl_aMarkTypeMap))
        {
            switch ((lcl_MarkType)nTmp)
            {
                case TypeReference:
                    // export point reference mark
                    CreateAndInsertMark(GetImport(),
                                        sAPI_reference_mark,
                                        sName,
                                       rHelper.GetCursorAsRange()->getStart());
                    break;

                case TypeBookmark:
                    // export point bookmark
                    CreateAndInsertMark(GetImport(),
                                        sAPI_bookmark,
                                        sName,
                                       rHelper.GetCursorAsRange()->getStart());
                    break;

                case TypeBookmarkStart:
                    // save XTextRange for later construction of bookmark
                    rHelper.InsertBookmarkStartRange(
                        sName, rHelper.GetCursorAsRange()->getStart());
                    break;

                case TypeBookmarkEnd:
                {
                    // get old range, and construct
                    Reference<XTextRange> xStartRange;
                    if (rHelper.FindAndRemoveBookmarkStartRange(xStartRange, 
                                                                sName))
                    {
                        Reference<XTextRange> xEndRange(
                            rHelper.GetCursorAsRange()->getEnd());

                        // check if beginning and end are in same XText
                        if (xStartRange->getText() == xEndRange->getText())
                        {
                            // create range for insertion
                            Reference<XTextCursor> xInsertionCursor =
                                rHelper.GetText()->createTextCursorByRange(
                                    xEndRange);
                            xInsertionCursor->gotoRange(xStartRange, sal_True);
                            
                            //DBG_ASSERT(! xInsertionCursor->isCollapsed(), 
                            // 				"we want no point mark");
                            // can't assert, because someone could
                            // create a file with subsequence
                            // start/end elements

                            Reference<XTextRange> xInsertionRange(
                                xInsertionCursor, UNO_QUERY);

                            // insert reference
                            CreateAndInsertMark(GetImport(),
                                                sAPI_bookmark,
                                                sName, 
                                                xInsertionRange);
                        }
                        // else: beginning/end in different XText -> ignore!
                    }
                    // else: no start found -> ignore!
                    break;
                }

                case TypeReferenceStart:
                case TypeReferenceEnd:	
                    DBG_ERROR("reference start/end are handled in txtparai !");
                    break;

                default:
                    DBG_ERROR("unknown mark type");
                    break;
            }
        }
    }
}


void XMLTextMarkImportContext::CreateAndInsertMark(
    SvXMLImport& rImport,
    const OUString& sServiceName,
    const OUString& sMarkName,
    const Reference<XTextRange> & rRange)
{
    // create mark
    Reference<XMultiServiceFactory> xFactory(rImport.GetModel(),UNO_QUERY);
    if( xFactory.is() )	
    {
        Reference<XInterface> xIfc = xFactory->createInstance(sServiceName);
        
        // set name
        Reference<XNamed> xNamed(xIfc, UNO_QUERY);
        if (xNamed.is())
        {
            xNamed->setName(sMarkName);

            // cast to XTextContent and attach to document
            Reference<XTextContent> xTextContent(xIfc, UNO_QUERY);
            if (xTextContent.is())
            {
                // if inserting marks, bAbsorb==sal_False will cause
                // collapsing of the given XTextRange.
                rImport.GetTextImport()->GetText()->insertTextContent(rRange, 
                                                     xTextContent, sal_True);
            }
        }
    }
}

sal_Bool XMLTextMarkImportContext::FindName(
    SvXMLImport& rImport,
    const Reference<XAttributeList> & xAttrList,
    OUString& sName)
{
    sal_Bool bNameOK;

    // find name attribute first
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr), 
                              &sLocalName );

        if ( (XML_NAMESPACE_TEXT == nPrefix) &&
             IsXMLToken(sLocalName, XML_NAME)   )
        {
            sName = xAttrList->getValueByIndex(nAttr);
            bNameOK = sal_True;
        }
    }

    return bNameOK;
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
