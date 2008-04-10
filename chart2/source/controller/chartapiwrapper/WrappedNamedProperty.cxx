/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: WrappedNamedProperty.cxx,v $
 * $Revision: 1.3 $
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

#include "WrappedNamedProperty.hxx"
#include "macros.hxx"

using ::com::sun::star::uno::Any;
using ::rtl::OUString;


//.............................................................................
namespace chart
{
namespace wrapper
{

WrappedNamedProperty::WrappedNamedProperty(
                                  const OUString& rOuterName
                                , const OUString& rInnerName
                                , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
                           : WrappedProperty( rOuterName, rInnerName )
                           , m_spChart2ModelContact( spChart2ModelContact )
{
}
WrappedNamedProperty::~WrappedNamedProperty()
{
}

//static
void WrappedNamedProperty::addWrappedProperties( std::vector< WrappedProperty* >& rList, ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact  )
{
    addWrappedFillProperties( rList, spChart2ModelContact  );
    addWrappedLineProperties( rList, spChart2ModelContact  );
}

//static
void WrappedNamedProperty::addWrappedFillProperties( std::vector< WrappedProperty* >& rList, ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact  )
{
    rList.push_back( new WrappedFillGradientNameProperty( spChart2ModelContact ) );
    rList.push_back( new WrappedFillHatchNameProperty( spChart2ModelContact ) );
    rList.push_back( new WrappedFillBitmapNameProperty( spChart2ModelContact ) );
    rList.push_back( new WrappedFillTransparenceGradientNameProperty( spChart2ModelContact ) );
}

//static
void WrappedNamedProperty::addWrappedLineProperties( std::vector< WrappedProperty* >& rList, ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact  )
{
    rList.push_back( new WrappedLineDashNameProperty( spChart2ModelContact ) );
}

::com::sun::star::uno::Any WrappedNamedProperty::convertInnerToOuterValue( const Any& rInnerValue ) const
{
    Any aRet;
    ::rtl::OUString aName = m_spChart2ModelContact->GetNameOfListElement( rInnerValue, m_aOuterName );
    aRet <<= aName;
    return aRet;

}
Any WrappedNamedProperty::convertOuterToInnerValue( const Any& rOuterValue ) const
{
    Any aRet;
    OUString aName;
    if( rOuterValue >>= aName )
    {
        aRet = m_spChart2ModelContact->GetListElementByName( aName, m_aOuterName );
    }
    return aRet;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

WrappedFillGradientNameProperty::WrappedFillGradientNameProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
        : WrappedNamedProperty( C2U( "FillGradientName" ), C2U( "FillGradient" ), spChart2ModelContact )
{
}
WrappedFillGradientNameProperty::~WrappedFillGradientNameProperty()
{
}

//-----------------------------------------------------------------------------

WrappedFillHatchNameProperty::WrappedFillHatchNameProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
        : WrappedNamedProperty( C2U( "FillHatchName" ), C2U( "FillHatch" ), spChart2ModelContact )
{
}
WrappedFillHatchNameProperty::~WrappedFillHatchNameProperty()
{
}

//-----------------------------------------------------------------------------

WrappedFillBitmapNameProperty::WrappedFillBitmapNameProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
        : WrappedNamedProperty( C2U( "FillBitmapName" ), C2U( "FillBitmap" ), spChart2ModelContact )
{
}
WrappedFillBitmapNameProperty::~WrappedFillBitmapNameProperty()
{
}

//-----------------------------------------------------------------------------

WrappedFillTransparenceGradientNameProperty::WrappedFillTransparenceGradientNameProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
        : WrappedNamedProperty( C2U( "FillTransparenceGradientName" ), C2U( "FillTransparenceGradient" ), spChart2ModelContact )
{
}
WrappedFillTransparenceGradientNameProperty::~WrappedFillTransparenceGradientNameProperty()
{
}

//-----------------------------------------------------------------------------

WrappedLineDashNameProperty::WrappedLineDashNameProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
        : WrappedNamedProperty( C2U( "LineDashName" ), C2U( "LineDash" ), spChart2ModelContact )
{
}
WrappedLineDashNameProperty::~WrappedLineDashNameProperty()
{
}

} //namespace wrapper
} //namespace chart
//.............................................................................
