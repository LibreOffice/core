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

#include "drawingml/clrschemecontext.hxx"
#include "oox/core/xmlfilterbase.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

static void setClrMap( const ::oox::AttributeList& rAttributes,
            ClrMap& rClrMap, sal_Int32 nToken )
{
    if ( rAttributes.hasAttribute( nToken ) )
    {
        sal_Int32 nMappedToken = rAttributes.getToken( nToken, 0 );
        rClrMap.setColorMap( nToken, nMappedToken );
    }
}

clrMapContext::clrMapContext( ContextHandler2Helper& rParent,
    const ::oox::AttributeList& rAttributes, ClrMap& rClrMap )
: ContextHandler2( rParent )
{
    setClrMap( rAttributes, rClrMap, XML_bg1 );
    setClrMap( rAttributes, rClrMap, XML_tx1 );
    setClrMap( rAttributes, rClrMap, XML_bg2 );
    setClrMap( rAttributes, rClrMap, XML_tx2 );
    setClrMap( rAttributes, rClrMap, XML_accent1 );
    setClrMap( rAttributes, rClrMap, XML_accent2 );
    setClrMap( rAttributes, rClrMap, XML_accent3 );
    setClrMap( rAttributes, rClrMap, XML_accent4 );
    setClrMap( rAttributes, rClrMap, XML_accent5 );
    setClrMap( rAttributes, rClrMap, XML_accent6 );
    setClrMap( rAttributes, rClrMap, XML_hlink );
    setClrMap( rAttributes, rClrMap, XML_folHlink );
}

clrSchemeColorContext::clrSchemeColorContext( ContextHandler2Helper& rParent, ClrScheme& rClrScheme, sal_Int32 nColorToken ) :
    ColorContext( rParent, *this ),
    mrClrScheme( rClrScheme ),
    mnColorToken( nColorToken )
{
}

clrSchemeColorContext::~clrSchemeColorContext()
{
    mrClrScheme.setColor( mnColorToken, getColor( getFilter().getGraphicHelper() ) );
}

clrSchemeContext::clrSchemeContext( ContextHandler2Helper& rParent, ClrScheme& rClrScheme ) :
    ContextHandler2( rParent ),
    mrClrScheme( rClrScheme )
{
}

ContextHandlerRef clrSchemeContext::onCreateContext(
        sal_Int32 nElement, const AttributeList& )
{
    switch( nElement )
    {
        case A_TOKEN( dk1 ):
        case A_TOKEN( lt1 ):
        case A_TOKEN( dk2 ):
        case A_TOKEN( lt2 ):
        case A_TOKEN( accent1 ):
        case A_TOKEN( accent2 ):
        case A_TOKEN( accent3 ):
        case A_TOKEN( accent4 ):
        case A_TOKEN( accent5 ):
        case A_TOKEN( accent6 ):
        case A_TOKEN( hlink ):
        case A_TOKEN( folHlink ):
            return new clrSchemeColorContext( *this, mrClrScheme, getBaseToken( nElement ) );
    }
    return nullptr;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
