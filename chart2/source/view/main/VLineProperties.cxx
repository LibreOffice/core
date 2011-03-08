/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "VLineProperties.hxx"
#include "macros.hxx"
#include <com/sun/star/drawing/LineStyle.hpp>

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
    this->Width = uno::makeAny( sal_Int32(0) );//type sal_Int32 for property UNO_NAME_LINEWIDTH
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
            this->DashName = xProp->getPropertyValue( C2U( "BorderDashName" ) );
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
            this->DashName = xProp->getPropertyValue( C2U( "LineDashName" ) );
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
    else
        this->LineStyle = uno::makeAny( drawing::LineStyle_NONE );
}

bool VLineProperties::isLineVisible() const
{
    bool bRet = false;

    drawing::LineStyle aLineStyle(drawing::LineStyle_SOLID);
    this->LineStyle >>= aLineStyle;
    if( aLineStyle != drawing::LineStyle_NONE )
    {
        sal_Int16 nLineTransparence=0;
        this->Transparence >>= nLineTransparence;
        if(100!=nLineTransparence)
        {
            bRet = true;
        }
    }

    return bRet;
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
