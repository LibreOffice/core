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

#include "animvariantcontext.hxx"

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include <osl/diagnose.h>

#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustring.hxx>

#include "oox/helper/attributelist.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/core/fragmenthandler.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/colorchoicecontext.hxx"
#include "pptfilterhelpers.hxx"
#include "tokens.hxx"

using ::rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace ppt {

    AnimVariantContext::AnimVariantContext( ContextHandler& rParent, sal_Int32 aElement, Any & aValue )
        : ContextHandler( rParent )
            , mnElement( aElement )
            , maValue( aValue )
    {
    }

    AnimVariantContext::~AnimVariantContext( ) throw( )
    {
    }

    void SAL_CALL AnimVariantContext::endFastElement( sal_Int32 aElement )
        throw ( SAXException, RuntimeException)
    {
        if( ( aElement == mnElement ) && maColor.isUsed() )
        {
            maValue = makeAny( maColor.getColor( getFilter() ) );
        }
    }


    Reference< XFastContextHandler >
    SAL_CALL AnimVariantContext::createFastChildContext( ::sal_Int32 aElementToken,
                             const Reference< XFastAttributeList >& xAttribs )
        throw ( SAXException, RuntimeException )
    {
        Reference< XFastContextHandler > xRet;
        AttributeList attribs(xAttribs);

        switch( aElementToken )
        {
        case NMSP_PPT|XML_boolVal:
        {
            bool val = attribs.getBool( XML_val, false );
            maValue = makeAny( val );
            break;
        }
        case NMSP_PPT|XML_clrVal:
            xRet.set( new ::oox::drawingml::ColorContext( *this, maColor ) );
            // we'll defer setting the Any until the end.
            break;
        case NMSP_PPT|XML_fltVal:
        {
            double val = attribs.getDouble( XML_val, 0.0 );
            maValue = makeAny( val );
            break;
        }
        case NMSP_PPT|XML_intVal:
        {
            sal_Int32 val = attribs.getInteger( XML_val, 0 );
            maValue = makeAny( val );
            break;
        }
        case NMSP_PPT|XML_strVal:
        {
            OUString val = attribs.getString( XML_val, OUString() );
            convertMeasure( val ); // ignore success or failure if it fails, use as is
            maValue = makeAny( val );
            break;
        }
        default:
            break;
        }

        if( !xRet.is() )
            xRet.set( this );

        return xRet;
    }



} }
