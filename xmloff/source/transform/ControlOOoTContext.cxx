/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "ControlOOoTContext.hxx"
#include "IgnoreTContext.hxx"
#include "MutableAttrList.hxx"
#include "ActionMapTypesOOo.hxx"
#include "TransformerBase.hxx"


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

XMLControlOOoTransformerContext::XMLControlOOoTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLTransformerContext( rImp, rQName )
{
}

void XMLControlOOoTransformerContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{
    m_xAttrList = new XMLMutableAttributeList( rAttrList, true );
}

rtl::Reference<XMLTransformerContext> XMLControlOOoTransformerContext::CreateChildContext(
        sal_uInt16 /*nPrefix*/,
        const OUString& /*rLocalName*/,
        const OUString& rQName,
        const Reference< XAttributeList >& rAttrList )
{
    rtl::Reference<XMLTransformerContext> pContext;

    if( m_aElemQName.isEmpty() )
    {
        pContext.set(new XMLIgnoreTransformerContext( GetTransformer(),
                                                    rQName,
                                                    false, false ));
        m_aElemQName = rQName;
        static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
                ->AppendAttributeList( rAttrList );
        GetTransformer().ProcessAttrList( m_xAttrList,
                                          OOO_FORM_CONTROL_ACTIONS,
                                          false );
        GetTransformer().GetDocHandler()->startElement( m_aElemQName,
                                                        m_xAttrList );
    }
    else
    {
        pContext.set(new XMLIgnoreTransformerContext( GetTransformer(),
                                                    rQName,
                                                    true, true ));
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
    if( !m_aElemQName.isEmpty() )
        XMLTransformerContext::Characters( rChars );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
