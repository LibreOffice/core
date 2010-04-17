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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include "ControlOOoTContext.hxx"
#include "IgnoreTContext.hxx"
#include "MutableAttrList.hxx"
#include "xmloff/xmlnmspe.hxx"
#include "ActionMapTypesOOo.hxx"
#include "ElemTransformerAction.hxx"
#include "TransformerActions.hxx"
#ifndef _XMLOFF_TRANSFORMERBASE_HXX
#include "TransformerBase.hxx"
#endif

using ::rtl::OUString;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

TYPEINIT1( XMLControlOOoTransformerContext, XMLTransformerContext );

XMLControlOOoTransformerContext::XMLControlOOoTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLTransformerContext( rImp, rQName )
{
}

XMLControlOOoTransformerContext::~XMLControlOOoTransformerContext()
{
}

void XMLControlOOoTransformerContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{
    m_xAttrList = new XMLMutableAttributeList( rAttrList, sal_True );
}

XMLTransformerContext *XMLControlOOoTransformerContext::CreateChildContext(
        sal_uInt16 /*nPrefix*/,
        const OUString& /*rLocalName*/,
        const OUString& rQName,
        const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerContext *pContext = 0;

    if( !m_aElemQName.getLength() )
    {
        pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                    rQName,
                                                    sal_False, sal_False );
        m_aElemQName = rQName;
        static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
                ->AppendAttributeList( rAttrList );
        GetTransformer().ProcessAttrList( m_xAttrList,
                                          OOO_FORM_CONTROL_ACTIONS,
                                          sal_False );
        GetTransformer().GetDocHandler()->startElement( m_aElemQName,
                                                        m_xAttrList );
    }
    else
    {
        pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                    rQName,
                                                    sal_True, sal_True );
    }
    return pContext;
}

void XMLControlOOoTransformerContext::EndElement()
{
    GetTransformer().GetDocHandler()->endElement( m_aElemQName );
}

void XMLControlOOoTransformerContext::Characters( const OUString& rChars )
{
    // ignore
    if( m_aElemQName.getLength() )
        XMLTransformerContext::Characters( rChars );
}
