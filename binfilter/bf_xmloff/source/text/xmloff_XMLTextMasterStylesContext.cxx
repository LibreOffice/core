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



#ifndef _XMLOFF_XMLIMP_HXX 
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX 
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLTEXTMASTERPAGECONTEXT_HXX
#include "XMLTextMasterPageContext.hxx"
#endif
#ifndef _XMLTEXTMASTERSTYLESCONTEXT_HXX
#include "XMLTextMasterStylesContext.hxx"
#endif
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

using ::binfilter::xmloff::token::IsXMLToken;
using ::binfilter::xmloff::token::XML_MASTER_PAGE;

// ------------------------------------------------------------------------

TYPEINIT1( XMLTextMasterStylesContext, SvXMLStylesContext );

sal_Bool XMLTextMasterStylesContext::InsertStyleFamily( sal_uInt16 ) const
{
    return sal_True;
}

XMLTextMasterStylesContext::XMLTextMasterStylesContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList ) :
    SvXMLStylesContext( rImport, nPrfx, rLName, xAttrList )
{
}

XMLTextMasterStylesContext::~XMLTextMasterStylesContext()
{
}

SvXMLStyleContext *XMLTextMasterStylesContext::CreateStyleChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLStyleContext *pContext = 0;

    if( XML_NAMESPACE_STYLE == nPrefix &&
        IsXMLToken( rLocalName, XML_MASTER_PAGE ) &&
         InsertStyleFamily( XML_STYLE_FAMILY_MASTER_PAGE ) )
        pContext = new XMLTextMasterPageContext(
                        GetImport(), nPrefix, rLocalName,
                          xAttrList,
                        !GetImport().GetTextImport()->IsInsertMode() );

    // any other style will be ignored here!

    return pContext;
}

SvXMLStyleContext *XMLTextMasterStylesContext::CreateStyleStyleChildContext(
        sal_uInt16 nFamily,
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    return 0;
}
}//end of namespace binfilter
