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

#include "oox/drawingml/linepropertiescontext.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "oox/helper/attributelist.hxx"

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
            mrLineProperties.moLineJoint = getBaseToken( nElement );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
