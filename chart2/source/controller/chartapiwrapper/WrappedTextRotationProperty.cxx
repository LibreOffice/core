/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WrappedTextRotationProperty.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 12:34:18 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "WrappedTextRotationProperty.hxx"
#include "macros.hxx"

using ::com::sun::star::uno::Any;
using ::rtl::OUString;


//.............................................................................
namespace chart
{
//.............................................................................

WrappedTextRotationProperty::WrappedTextRotationProperty()
    : ::chart::WrappedProperty( C2U( "TextRotation" ), C2U( "TextRotation" ) )
{
}
WrappedTextRotationProperty::~WrappedTextRotationProperty()
{
}
Any WrappedTextRotationProperty::convertInnerToOuterValue( const Any& rInnerValue ) const
{
    Any aRet;
    double fVal = 0;
    if( rInnerValue >>= fVal )
    {
        sal_Int32 n100thDegrees = static_cast< sal_Int32 >( fVal * 100.0 );
        aRet <<= n100thDegrees;
    }
    return aRet;
}
Any WrappedTextRotationProperty::convertOuterToInnerValue( const Any& rOuterValue ) const
{
    Any aRet;
    sal_Int32 nVal = 0;
    if( rOuterValue >>= nVal )
    {
        double fDoubleDegrees = ( static_cast< double >( nVal ) / 100.0 );
        aRet <<= fDoubleDegrees;
    }
    return aRet;
}

//.............................................................................
} //namespace chart
//.............................................................................
