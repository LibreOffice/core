/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VLineProperties.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:52:01 $
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

#include "VLineProperties.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  get line properties from a propertyset
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

VLineProperties::VLineProperties()
{
    this->Color = uno::makeAny( sal_Int32(0x000000) ); //type sal_Int32 UNO_NAME_LINECOLOR
    this->LineStyle = uno::makeAny( drawing::LineStyle_SOLID ); //type drawing::LineStyle for property UNO_NAME_LINESTYLE
    this->Transparence = uno::makeAny( sal_Int16(0) );//type sal_Int16 for property UNO_NAME_LINETRANSPARENCE
    this->Width = uno::makeAny( sal_Int32(1) );//type sal_Int32 for property UNO_NAME_LINEWIDTH
}

void VLineProperties::initFromPropertySet( const uno::Reference< beans::XPropertySet >& xProp, bool bUseSeriesPropertyNames )
{
    if(xProp.is())
    {
        if( bUseSeriesPropertyNames ) try
        {
            this->Color = xProp->getPropertyValue( C2U( "BorderColor" ) );
            this->LineStyle = xProp->getPropertyValue( C2U( "BorderStyle" ) );
            this->Transparence = xProp->getPropertyValue( C2U( "BorderTransparency" ) );
            this->Width = xProp->getPropertyValue( C2U( "BorderWidth" ) );
            this->Dash = xProp->getPropertyValue( C2U( "BorderDash" ) );
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
        else try
        {
            this->Color = xProp->getPropertyValue( C2U( "LineColor" ) );
            this->LineStyle = xProp->getPropertyValue( C2U( "LineStyle" ) );
            this->Transparence = xProp->getPropertyValue( C2U( "LineTransparence" ) );
            this->Width = xProp->getPropertyValue( C2U( "LineWidth" ) );
            this->Dash = xProp->getPropertyValue( C2U( "LineDash" ) );
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
