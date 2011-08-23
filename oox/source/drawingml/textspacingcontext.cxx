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

#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/textspacing.hxx"
#include "oox/core/namespaces.hxx"
#include "textspacingcontext.hxx"
#include "tokens.hxx"


using namespace ::oox::core;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;


namespace oox { namespace drawingml {

    TextSpacingContext::TextSpacingContext( ContextHandler& rParent, TextSpacing & aSpacing )
        : ContextHandler( rParent )
        , maSpacing( aSpacing )
    {
        maSpacing.bHasValue = sal_True;
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
        case NMSP_DRAWINGML|XML_spcPct:
            maSpacing.nUnit = TextSpacing::PERCENT;
            maSpacing.nValue = GetPercent( xAttribs->getValue( XML_val ) );
            break;
        case NMSP_DRAWINGML|XML_spcPts:
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
