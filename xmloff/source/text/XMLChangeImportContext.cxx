/*************************************************************************
 *
 *  $RCSfile: XMLChangeImportContext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dvo $ $Date: 2001-06-29 21:07:21 $
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

#ifndef _XMLOFF_XMLCHANGEIMPORTCONTEXT_HXX
#include "XMLChangeImportContext.hxx"
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGE_HPP_
#include <com/sun/star/text/XTextRange.hpp>
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

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::text::XTextRange;
using ::com::sun::star::xml::sax::XAttributeList;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_CHANGE_ID;

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
            OUString& rID = xAttrList->getValueByIndex(nAttr);

            // call for bStart and bEnd (may both be true)
            if (bIsStart)
                rHelper->RedlineSetCursor(rID,sal_True,bIsOutsideOfParagraph);
            if (bIsEnd)
                rHelper->RedlineSetCursor(rID,sal_False,bIsOutsideOfParagraph);

            // outside of paragraph and still open? set open redline ID
            if (bIsStart)
            {
                if (bIsOutsideOfParagraph || !bIsEnd)
                    rHelper->SetOpenRedlineId(rID);
                else
                    rHelper->ResetOpenRedlineId();
            }
        }
        // else: ignore
    }
}
