/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLChangeImportContext.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 11:03:06 $
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
