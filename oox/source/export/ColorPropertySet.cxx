/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ColorPropertySet.cxx,v $
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

#include "ColorPropertySet.hxx"

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/drawing/FillStyle.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::com::sun::star::uno::RuntimeException;

// ================================================================================

namespace
{
class lcl_ColorPropertySetInfo : public ::cppu::WeakImplHelper1<
        XPropertySetInfo  >
{
public:
    lcl_ColorPropertySetInfo( bool bFillColor );

protected:
    // ____ XPropertySetInfo ____
    virtual Sequence< Property > SAL_CALL getProperties()                throw (RuntimeException);
    virtual Property SAL_CALL getPropertyByName( const OUString& aName ) throw (UnknownPropertyException, RuntimeException);
    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name )  throw (RuntimeException);

private:
    bool m_bIsFillColor;
    OUString m_aColorPropName;
    Property m_aColorProp;
};

lcl_ColorPropertySetInfo::lcl_ColorPropertySetInfo( bool bFillColor ) :
        m_bIsFillColor( bFillColor ),
        // note: length of FillColor and LineColor is 9
        m_aColorPropName( (bFillColor ? "FillColor" : "LineColor"), 9, RTL_TEXTENCODING_ASCII_US ),
        m_aColorProp( m_aColorPropName, -1,
                      ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)), 0)
{}

Sequence< Property > SAL_CALL lcl_ColorPropertySetInfo::getProperties()
    throw (RuntimeException)
{

    return Sequence< Property >( & m_aColorProp, 1 );
}

Property SAL_CALL lcl_ColorPropertySetInfo::getPropertyByName( const OUString& aName )
    throw (UnknownPropertyException, RuntimeException)
{
    if( aName.equals( m_aColorPropName ))
        return m_aColorProp;
    throw UnknownPropertyException( m_aColorPropName, static_cast< uno::XWeak * >( this ));
}

sal_Bool SAL_CALL lcl_ColorPropertySetInfo::hasPropertyByName( const OUString& Name )
    throw (RuntimeException)
{
    return Name.equals( m_aColorPropName );
}

} // anonymous namespace

// ================================================================================

namespace oox
{
namespace drawingml
{

ColorPropertySet::ColorPropertySet( sal_Int32 nColor, bool bFillColor /* = true */ ) :
        // note: length of FillColor and LineColor is 9
        m_aColorPropName( (bFillColor ? "FillColor" : "LineColor"), 9, RTL_TEXTENCODING_ASCII_US ),
        m_nColor( nColor ),
        m_bIsFillColor( bFillColor ),
        m_nDefaultColor( 0x0099ccff )  // blue 8
{}

ColorPropertySet::~ColorPropertySet()
{}

void ColorPropertySet::setColor( sal_Int32 nColor )
{
    m_nColor = nColor;
}

sal_Int32 ColorPropertySet::getColor()
{
    return m_nColor;
}

// ____ XPropertySet ____

Reference< XPropertySetInfo > SAL_CALL ColorPropertySet::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    if( ! m_xInfo.is())
        m_xInfo.set( new lcl_ColorPropertySetInfo( m_bIsFillColor ));

    return m_xInfo;
}

void SAL_CALL ColorPropertySet::setPropertyValue( const OUString& /* aPropertyName */, const uno::Any& aValue )
    throw (UnknownPropertyException,
           PropertyVetoException,
           lang::IllegalArgumentException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    aValue >>= m_nColor;
}

uno::Any SAL_CALL ColorPropertySet::getPropertyValue( const OUString& aPropertyName )
    throw (UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    if( aPropertyName.equalsAscii("FillStyle") && m_bIsFillColor )
    {
        ::com::sun::star::drawing::FillStyle aFillStyle = ::com::sun::star::drawing::FillStyle_SOLID;
        return uno::makeAny(aFillStyle);
    }
    return uno::makeAny( m_nColor );
}

void SAL_CALL ColorPropertySet::addPropertyChangeListener( const OUString& /* aPropertyName */, const Reference< XPropertyChangeListener >& /* xListener */ )
    throw (UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    OSL_ENSURE( false, "Not Implemented" );
    return;
}

void SAL_CALL ColorPropertySet::removePropertyChangeListener( const OUString& /* aPropertyName */, const Reference< XPropertyChangeListener >& /* aListener */ )
    throw (UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    OSL_ENSURE( false, "Not Implemented" );
    return;
}

void SAL_CALL ColorPropertySet::addVetoableChangeListener( const OUString& /* PropertyName */, const Reference< XVetoableChangeListener >& /* aListener */ )
    throw (UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    OSL_ENSURE( false, "Not Implemented" );
    return;
}

void SAL_CALL ColorPropertySet::removeVetoableChangeListener( const OUString& /* PropertyName */, const Reference< XVetoableChangeListener >& /* aListener */ )
    throw (UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    OSL_ENSURE( false, "Not Implemented" );
    return;
}

// ____ XPropertyState ____

PropertyState SAL_CALL ColorPropertySet::getPropertyState( const OUString& /* PropertyName */ )
    throw (UnknownPropertyException,
           uno::RuntimeException)
{
    return PropertyState_DIRECT_VALUE;
}

Sequence< PropertyState > SAL_CALL ColorPropertySet::getPropertyStates( const Sequence< OUString >& /* aPropertyName */ )
    throw (UnknownPropertyException,
           uno::RuntimeException)
{
    PropertyState aState = PropertyState_DIRECT_VALUE;
    return Sequence< PropertyState >( & aState, 1 );
}

void SAL_CALL ColorPropertySet::setPropertyToDefault( const OUString& PropertyName )
    throw (UnknownPropertyException,
           uno::RuntimeException)
{
    if( PropertyName.equals( m_aColorPropName ))
        m_nColor = m_nDefaultColor;
}

uno::Any SAL_CALL ColorPropertySet::getPropertyDefault( const OUString& aPropertyName )
    throw (UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    if( aPropertyName.equals( m_aColorPropName ))
        return uno::makeAny( m_nDefaultColor );
    return uno::Any();
}

} //  namespace chart
} //  namespace xmloff
