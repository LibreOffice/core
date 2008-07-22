/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: linepropertiescontext.cxx,v $
 * $Revision: 1.5 $
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
        case NMSP_DRAWINGML|XML_noFill:
        case NMSP_DRAWINGML|XML_solidFill:
        case NMSP_DRAWINGML|XML_gradFill:
        case NMSP_DRAWINGML|XML_pattFill:
            xRet = FillPropertiesGroupContext::StaticCreateContext( *this, nElement, xAttribs, mrLineProperties.maLineFill );
        break;

        // LineDashPropertiesGroup
        case NMSP_DRAWINGML|XML_prstDash:   // CT_PresetLineDashProperties
            mrLineProperties.moPresetDash = aAttribs.getToken( XML_val );
        break;
        case NMSP_DRAWINGML|XML_custDash:   // CT_DashStopList
        break;

        // LineJoinPropertiesGroup
        case NMSP_DRAWINGML|XML_round:
        case NMSP_DRAWINGML|XML_bevel:
        case NMSP_DRAWINGML|XML_miter:
            mrLineProperties.moLineJoint = getToken( nElement );
        break;

        case NMSP_DRAWINGML|XML_headEnd:    // CT_LineEndProperties
        case NMSP_DRAWINGML|XML_tailEnd:    // CT_LineEndProperties
        {                                   // ST_LineEndType
            bool bTailEnd = nElement == (NMSP_DRAWINGML|XML_tailEnd);
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
