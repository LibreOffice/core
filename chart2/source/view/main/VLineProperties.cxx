/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
