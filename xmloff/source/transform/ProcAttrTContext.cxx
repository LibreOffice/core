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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include "ProcAttrTContext.hxx"
#include "MutableAttrList.hxx"
#include "TransformerBase.hxx"
#include "TransformerActions.hxx"
#include <xmloff/nmspmap.hxx>

using ::rtl::OUString;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

TYPEINIT1( XMLProcAttrTransformerContext, XMLTransformerContext );

XMLProcAttrTransformerContext::XMLProcAttrTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
       sal_uInt16 nActionMap ) :
    XMLTransformerContext( rImp, rQName ),
    m_aElemQName( rQName ),
    m_nActionMap( nActionMap )
{
}

XMLProcAttrTransformerContext::XMLProcAttrTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken,
       sal_uInt16 nActionMap ) :
    XMLTransformerContext( rImp, rQName ),
    m_aElemQName( rImp.GetNamespaceMap().GetQNameByKey( nPrefix,
                            ::xmloff::token::GetXMLToken( eToken ) ) ),
    m_nActionMap( nActionMap )
{
}

XMLProcAttrTransformerContext::~XMLProcAttrTransformerContext()
{
}

void XMLProcAttrTransformerContext::StartElement(
        const Reference< XAttributeList >& rAttrList )
{
    Reference< XAttributeList > xAttrList( rAttrList );
    GetTransformer().ProcessAttrList( xAttrList, m_nActionMap, sal_False );
    GetTransformer().GetDocHandler()->startElement( m_aElemQName, xAttrList );
}

void XMLProcAttrTransformerContext::EndElement()
{
    GetTransformer().GetDocHandler()->endElement( m_aElemQName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
