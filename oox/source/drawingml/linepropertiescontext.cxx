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

#include <drawingml/linepropertiescontext.hxx>
#include <drawingml/misccontexts.hxx>
#include <drawingml/lineproperties.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

// CT_LineProperties

namespace oox::drawingml {

LinePropertiesContext::LinePropertiesContext( ContextHandler2Helper const & rParent, const AttributeList& rAttribs,
    LineProperties& rLineProperties ) noexcept
: ContextHandler2( rParent )
, mrLineProperties( rLineProperties )
{
    mrLineProperties.moLineWidth = rAttribs.getInteger( XML_w );
    mrLineProperties.moLineCompound = rAttribs.getToken( XML_cmpd );
    mrLineProperties.moLineCap = rAttribs.getToken( XML_cap );
}

LinePropertiesContext::~LinePropertiesContext()
{
}

ContextHandlerRef LinePropertiesContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        // LineFillPropertiesGroup
        case A_TOKEN( noFill ):
        case A_TOKEN( solidFill ):
        case A_TOKEN( gradFill ):
        case A_TOKEN( pattFill ):
            return FillPropertiesContext::createFillContext( *this, nElement, rAttribs, mrLineProperties.maLineFill );
        break;

        // LineDashPropertiesGroup
        case A_TOKEN( prstDash ):  // CT_PresetLineDashProperties
            mrLineProperties.moPresetDash = rAttribs.getToken( XML_val );
        break;
        case A_TOKEN( custDash ):  // CT_DashStopList
            return this;
        case A_TOKEN( ds ):
        {
            // 'a:ds' has 2 attributes : 'd' and 'sp'
            // both are of type 'a:ST_PositivePercentage'
            // according to the specs Office will read percentages formatted with a trailing percent sign
            // or formatted as 1000th of a percent without a trailing percent sign, but only write percentages
            // as 1000th's of a percent without a trailing percent sign.
            // The code below takes care of both scenarios by converting to '1000th of a percent' always
            OUString aStr;
            sal_Int32 nDash = 0;
            aStr = rAttribs.getString( XML_d, "" );
            if ( aStr.endsWith("%") )
            {
                // Ends with a '%'
                aStr = aStr.copy(0, aStr.getLength() - 1);
                aStr = aStr.trim();
                nDash = aStr.toInt32();

                // Convert to 1000th of a percent
                nDash *= 1000;
            }
            else
            {
                nDash = rAttribs.getInteger( XML_d, 0 );
            }

            sal_Int32 nSp = 0;
            aStr = rAttribs.getString( XML_sp, "" );
            if ( aStr.endsWith("%") )
            {
                // Ends with a '%'
                aStr = aStr.copy(0, aStr.getLength() - 1);
                aStr = aStr.trim();
                nSp = aStr.toInt32();

                // Convert to 1000th of a percent
                nSp *= 1000;
            }
            else
            {
                nSp = rAttribs.getInteger( XML_sp, 0 );
            }

            mrLineProperties.maCustomDash.emplace_back( nDash, nSp );
        }
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
            rArrowProps.moArrowType = rAttribs.getToken( XML_type );
            rArrowProps.moArrowWidth = rAttribs.getToken( XML_w );
            rArrowProps.moArrowLength = rAttribs.getToken( XML_len );
        }
        break;
    }
    return nullptr;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
