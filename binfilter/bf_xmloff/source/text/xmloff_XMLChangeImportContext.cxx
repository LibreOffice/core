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

#ifndef _XMLOFF_XMLCHANGEIMPORTCONTEXT_HXX
#include "XMLChangeImportContext.hxx"
#endif


#ifndef _TOOLS_DEBUG_HXX 
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

namespace binfilter {

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::text::XTextRange;
using ::com::sun::star::xml::sax::XAttributeList;
using ::binfilter::xmloff::token::IsXMLToken;
using ::binfilter::xmloff::token::XML_CHANGE_ID;

TYPEINIT1( XMLChangeImportContext, SvXMLImportContext );

XMLChangeImportContext::XMLChangeImportContext(
    SvXMLImport& rImport,
    sal_Int16 nPrefix,
    const OUString& rLocalName,
    sal_Bool bStart,
    sal_Bool bEnd,
    sal_Bool bOutsideOfParagraph) :
        SvXMLImportContext(rImport, nPrefix, rLocalName),
        bIsStart(bStart),
        bIsEnd(bEnd),
        bIsOutsideOfParagraph(bOutsideOfParagraph)
{
    DBG_ASSERT(bStart || bEnd, "Must be either start, end, or both!");
}

XMLChangeImportContext::~XMLChangeImportContext()
{
}

void XMLChangeImportContext::StartElement(
    const Reference<XAttributeList>& xAttrList)
{
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr), 
                              &sLocalName );
        if ( (XML_NAMESPACE_TEXT == nPrefix) &&
             IsXMLToken( sLocalName, XML_CHANGE_ID ) )
        {
            // Id found! Now call RedlineImportHelper

            // prepare parameters
            UniReference<XMLTextImportHelper> rHelper = 
                GetImport().GetTextImport();
            OUString sID = xAttrList->getValueByIndex(nAttr);

            // call for bStart and bEnd (may both be true)
            if (bIsStart)
                rHelper->RedlineSetCursor(sID,sal_True,bIsOutsideOfParagraph);
            if (bIsEnd)
                rHelper->RedlineSetCursor(sID,sal_False,bIsOutsideOfParagraph);

            // outside of paragraph and still open? set open redline ID
            if (bIsOutsideOfParagraph)
            {
                rHelper->SetOpenRedlineId(sID);
            }
        }
        // else: ignore
    }
}
}//end of namespace binfilter
