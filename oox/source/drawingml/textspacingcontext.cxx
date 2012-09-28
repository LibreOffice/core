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

#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/textspacing.hxx"
#include "textspacingcontext.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;


namespace oox { namespace drawingml {

    TextSpacingContext::TextSpacingContext( ContextHandler& rParent, TextSpacing & aSpacing )
        : ContextHandler( rParent )
        , maSpacing( aSpacing )
    {
        maSpacing.bHasValue = true;
    }

    void TextSpacingContext::endFastElement( sal_Int32 /*nElement*/ )
        throw ( SAXException, RuntimeException )
    {
    }

    Reference< XFastContextHandler > TextSpacingContext::createFastChildContext( ::sal_Int32 aElement,
            const Reference< XFastAttributeList >& xAttribs )
        throw ( SAXException, RuntimeException )
    {
        Reference< XFastContextHandler > xRet;
        switch( aElement )
        {
        case A_TOKEN( spcPct ):
            maSpacing.nUnit = TextSpacing::PERCENT;
            maSpacing.nValue = GetPercent( xAttribs->getValue( XML_val ) );
            break;
        case A_TOKEN( spcPts ):
            maSpacing.nUnit = TextSpacing::POINTS;
            maSpacing.nValue = GetTextSpacingPoint( xAttribs->getValue( XML_val ) );
            break;
        default:
            break;
        }
        if ( !xRet.is() )
            xRet.set( this );
        return xRet;
    }


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
