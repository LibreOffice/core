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

#include "XMLChangedRegionImportContext.hxx"

#include "XMLChangeElementImportContext.hxx"

#include <com/sun/star/uno/Reference.h>

#include <com/sun/star/util/DateTime.hpp>


#include "xmlimp.hxx"

#include "xmlnmspe.hxx"

#include "nmspmap.hxx"


#include "xmluconv.hxx"
namespace binfilter {


using namespace ::binfilter::xmloff::token;

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::text::XTextCursor;
using ::com::sun::star::util::DateTime;
using ::com::sun::star::xml::sax::XAttributeList;


    
TYPEINIT1(XMLChangedRegionImportContext, SvXMLImportContext);

XMLChangedRegionImportContext::XMLChangedRegionImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName) :
        SvXMLImportContext(rImport, nPrefix, rLocalName),
        bMergeLastPara(sal_True)
{
}

XMLChangedRegionImportContext::~XMLChangedRegionImportContext()
{
}

void XMLChangedRegionImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    // process attributes: id
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr), 
                              &sLocalName );

        const OUString sValue = xAttrList->getValueByIndex(nAttr);
        if ( XML_NAMESPACE_TEXT == nPrefix ) 
        {
            if( IsXMLToken( sLocalName, XML_ID ) )
            {
                sID = sValue;
            }
            else if( IsXMLToken( sLocalName, XML_MERGE_LAST_PARAGRAPH ) )
            {
                sal_Bool bTmp;
                if( SvXMLUnitConverter::convertBool(bTmp, sValue) )
                {
                    bMergeLastPara = bTmp;
                }
            }
        }
    }
}

SvXMLImportContext* XMLChangedRegionImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList)
{
    SvXMLImportContext* pContext = NULL;

    if (XML_NAMESPACE_TEXT == nPrefix)
    {
        if ( IsXMLToken( rLocalName, XML_INSERTION ) || 
             IsXMLToken( rLocalName, XML_DELETION ) || 
             IsXMLToken( rLocalName, XML_FORMAT_CHANGE ) )
        {
            // create XMLChangeElementImportContext for all kinds of changes
            pContext = new XMLChangeElementImportContext(
               GetImport(), nPrefix, rLocalName, 
               IsXMLToken( rLocalName, XML_DELETION ),
               *this);
        }
        // else: it may be a text element, see below
    }

    if (NULL == pContext)
    {
        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName, 
                                                          xAttrList);

        // was it a text element? If not, use default!
        if (NULL == pContext)
        {
            pContext = SvXMLImportContext::CreateChildContext(
                nPrefix, rLocalName, xAttrList);
        }
    }

    return pContext;
}

void XMLChangedRegionImportContext::EndElement()
{
    // restore old XCursor (if necessary)
    if (xOldCursor.is())
    {
        // delete last paragraph 
        // (one extra paragraph was inserted in the beginning)
        UniReference<XMLTextImportHelper> rHelper = 
            GetImport().GetTextImport();
        rHelper->DeleteParagraph();

        GetImport().GetTextImport()->SetCursor(xOldCursor);
        xOldCursor = NULL;
    }
}

void XMLChangedRegionImportContext::SetChangeInfo(
    const OUString& rType,
    const OUString& rAuthor,
    const OUString& rComment,
    const OUString& rDate)
{
    DateTime aDateTime;
    if (SvXMLUnitConverter::convertDateTime(aDateTime, rDate))
    {
        GetImport().GetTextImport()->RedlineAdd(
            rType, sID, rAuthor, rComment, aDateTime, bMergeLastPara);
    }
}

void XMLChangedRegionImportContext::UseRedlineText()
{
    // if we haven't already installed the redline cursor, do it now
    if (! xOldCursor.is())
    {
        // get TextImportHelper and old Cursor
        UniReference<XMLTextImportHelper> rHelper(GetImport().GetTextImport());
        Reference<XTextCursor> xCursor( rHelper->GetCursor() );

        // create Redline and new Cursor 
        Reference<XTextCursor> xNewCursor = 
            rHelper->RedlineCreateText(xCursor, sID);

        if (xNewCursor.is())
        {
            // save old cursor and install new one
            xOldCursor = xCursor;
            rHelper->SetCursor( xNewCursor );
        }
        // else: leave as is
    }
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
