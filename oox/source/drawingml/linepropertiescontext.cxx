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

#include "oox/drawingml/linepropertiescontext.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"

using ::rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

// CT_LineProperties

namespace oox { namespace drawingml {
// ---------------------------------------------------------------------

LinePropertiesContext::LinePropertiesContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs,
    LineProperties& rLineProperties ) throw()
: ContextHandler( rParent )
, mrLineProperties( rLineProperties )
{
    AttributeList aAttribs( xAttribs );
    mrLineProperties.moLineWidth = aAttribs.getInteger( XML_w );
    mrLineProperties.moLineCompound = aAttribs.getToken( XML_cmpd );
    mrLineProperties.moLineCap = aAttribs.getToken( XML_cap );
}

LinePropertiesContext::~LinePropertiesContext()
{
}

Reference< XFastContextHandler > LinePropertiesContext::createFastChildContext( sal_Int32 nElement, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    AttributeList aAttribs( xAttribs );
    switch( nElement )
    {
        // LineFillPropertiesGroup
        case A_TOKEN( noFill ):
        case A_TOKEN( solidFill ):
        case A_TOKEN( gradFill ):
        case A_TOKEN( pattFill ):
            xRet = FillPropertiesContext::createFillContext( *this, nElement, xAttribs, mrLineProperties.maLineFill );
        break;

        // LineDashPropertiesGroup
        case A_TOKEN( prstDash ):  // CT_PresetLineDashProperties
            mrLineProperties.moPresetDash = aAttribs.getToken( XML_val );
        break;
        case A_TOKEN( custDash ):  // CT_DashStopList
            xRet = this;
        break;
        case A_TOKEN( ds ):
            mrLineProperties.maCustomDash.push_back( LineProperties::DashStop(
                aAttribs.getInteger( XML_d, 0 ), aAttribs.getInteger( XML_sp, 0 ) ) );
        break;

        // LineJoinPropertiesGroup
        case A_TOKEN( round ):
        case A_TOKEN( bevel ):
        case A_TOKEN( miter ):
            mrLineProperties.moLineJoint = getToken( nElement );
        break;

        case A_TOKEN( headEnd ):  // CT_LineEndProperties
        case A_TOKEN( tailEnd ):  // CT_LineEndProperties
        {                         // ST_LineEndType
            bool bTailEnd = nElement == A_TOKEN( tailEnd );
            LineArrowProperties& rArrowProps = bTailEnd ? mrLineProperties.maEndArrow : mrLineProperties.maStartArrow;
            rArrowProps.moArrowType = aAttribs.getToken( XML_type );
            rArrowProps.moArrowWidth = aAttribs.getToken( XML_w );
            rArrowProps.moArrowLength = aAttribs.getToken( XML_len );
        }
        break;
    }
    return xRet;
}

} }
