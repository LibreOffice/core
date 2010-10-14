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
#include "ProcAddAttrTContext.hxx"
#include "MutableAttrList.hxx"
#include "TransformerBase.hxx"
#include <xmloff/nmspmap.hxx>

using ::rtl::OUString;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

TYPEINIT1( XMLProcAddAttrTransformerContext, XMLProcAttrTransformerContext);

XMLProcAddAttrTransformerContext::XMLProcAddAttrTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken,
        sal_uInt16 nActionMap,
        sal_uInt16 nAPrefix,
           ::xmloff::token::XMLTokenEnum eAToken,
           ::xmloff::token::XMLTokenEnum eVToken ) :
    XMLProcAttrTransformerContext( rImp, rQName, nPrefix,  eToken, nActionMap ),
    m_aAttrQName( rImp.GetNamespaceMap().GetQNameByKey( nAPrefix,
                                    ::xmloff::token::GetXMLToken( eAToken ) ) ),
    m_aAttrValue( ::xmloff::token::GetXMLToken( eVToken ) )
{
}

XMLProcAddAttrTransformerContext::~XMLProcAddAttrTransformerContext()
{
}

void XMLProcAddAttrTransformerContext::StartElement(
        const Reference< XAttributeList >& rAttrList )
{
    Reference< XAttributeList > xAttrList( rAttrList );
    XMLMutableAttributeList *pMutableAttrList =
        GetTransformer().ProcessAttrList( xAttrList, GetActionMap(),
                                          sal_False );
    if( !pMutableAttrList )
    {
        pMutableAttrList = new XMLMutableAttributeList;
        xAttrList = pMutableAttrList;
    }
    pMutableAttrList->AddAttribute( m_aAttrQName, m_aAttrValue );
    GetTransformer().GetDocHandler()->startElement( GetElemQName(), xAttrList );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
