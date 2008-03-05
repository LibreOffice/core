/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: linepropertiescontext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:24:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "oox/drawingml/linepropertiescontext.hxx"

#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include "oox/helper/propertymap.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/drawingml/colorchoicecontext.hxx"
#include "oox/drawingml/drawingmltypes.hxx"

#include "tokens.hxx"

using ::rtl::OUString;
using ::com::sun::star::beans::NamedValue;
using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::drawing;
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
    // ST_LineWidth
    if( xAttribs->hasAttribute( XML_w ) )
        mrLineProperties.getLineWidth() = boost::optional< sal_Int32 >( GetCoordinate( xAttribs->getOptionalValue( XML_w ) ) );

    // "ST_LineCap"
    if( xAttribs->hasAttribute( XML_cap ) )
        mrLineProperties.getLineCap() = boost::optional< sal_Int32 >( xAttribs->getOptionalValueToken( XML_cap, 0 ) );

    // if ( xAttribs->hasAttribute( XML_cmpd ) )    ST_CompoundLine
    // if ( xAttribs->hasAttribute( XML_algn ) )    ST_PenAlignment
}

LinePropertiesContext::~LinePropertiesContext()
{
}

Reference< XFastContextHandler > LinePropertiesContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    const rtl::OUString sLineStyle( RTL_CONSTASCII_USTRINGPARAM( "LineStyle" ) );
    switch( aElementToken )
    {
        // LineFillPropertiesGroup, line fillings currently unsuported
        case NMSP_DRAWINGML|XML_noFill:
            mrLineProperties.getLinePropertyMap()[ sLineStyle ] <<= LineStyle_NONE;
        break;
        case NMSP_DRAWINGML|XML_solidFill:
        {
            mrLineProperties.getLinePropertyMap()[ sLineStyle ] <<= LineStyle_SOLID;
            xRet = new colorChoiceContext( *this, *mrLineProperties.getLineColor() );
        }
        break;
        case NMSP_DRAWINGML|XML_gradFill:
        case NMSP_DRAWINGML|XML_pattFill:
            mrLineProperties.getLinePropertyMap()[ sLineStyle ] <<= LineStyle_SOLID;
        break;

        // LineDashPropertiesGroup
        case NMSP_DRAWINGML|XML_prstDash:   // CT_PresetLineDashProperties
            mrLineProperties.getPresetDash() = boost::optional< sal_Int32 >( xAttribs->getOptionalValueToken( XML_val, XML_solid ) );
        break;
        case NMSP_DRAWINGML|XML_custDash:   // CT_DashStopList
        break;

        // LineJoinPropertiesGroup
        case NMSP_DRAWINGML|XML_round:
        case NMSP_DRAWINGML|XML_bevel:
        case NMSP_DRAWINGML|XML_miter:
        {
            LineJoint eJoint = (aElementToken == (NMSP_DRAWINGML|XML_round)) ? LineJoint_ROUND :
                                (aElementToken == (NMSP_DRAWINGML|XML_bevel)) ? LineJoint_BEVEL :
                                    LineJoint_MITER;
            static const OUString sLineJoint( RTL_CONSTASCII_USTRINGPARAM( "LineJoint" ) );
            mrLineProperties.getLinePropertyMap()[ sLineJoint ] <<= eJoint;
        }
        break;

        case NMSP_DRAWINGML|XML_headEnd:    // CT_LineEndProperties
        case NMSP_DRAWINGML|XML_tailEnd:    // CT_LineEndProperties
        {                                   // ST_LineEndType
            if( xAttribs->hasAttribute( XML_type ) )
            {
                sal_Int32 nType = xAttribs->getOptionalValueToken( XML_type, 0 );
                if ( aElementToken == ( NMSP_DRAWINGML|XML_tailEnd ) )
                    mrLineProperties.getStartArrow() = boost::optional< sal_Int32 >( nType );
                else
                    mrLineProperties.getEndArrow() = boost::optional< sal_Int32 >( nType );
            }
            if ( xAttribs->hasAttribute( XML_w ) )
            {
                sal_Int32 nW = xAttribs->getOptionalValueToken( XML_w, 0 );
                if ( aElementToken == ( NMSP_DRAWINGML|XML_tailEnd ) )
                    mrLineProperties.getStartArrowWidth() = boost::optional< sal_Int32 >( nW );
                else
                    mrLineProperties.getEndArrowWidth() = boost::optional< sal_Int32 >( nW );
            }
            if ( xAttribs->hasAttribute( XML_len ) )
            {
                sal_Int32 nLen = xAttribs->getOptionalValueToken( XML_len, 0 );
                if ( aElementToken == ( NMSP_DRAWINGML|XML_tailEnd ) )
                    mrLineProperties.getStartArrowLength() = boost::optional< sal_Int32 >( nLen );
                else
                    mrLineProperties.getEndArrowLength() = boost::optional< sal_Int32 >( nLen );
            }
        }
        break;
    }
    if ( !xRet.is() )
        xRet.set( this );
    return xRet;
}

} }
