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
#include "ChXChartDrawPage.hxx"
#include "chtmodel.hxx"

// header for class OGuard
// header for class Application
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
namespace binfilter {


using namespace vos;
using namespace ::com::sun::star;

#define CHART_DRAW_PAGE_WIDTH_ID 1
#define CHART_DRAW_PAGE_HEIGHT_ID 2

const SfxItemPropertyMap* ImplGetChartDrawPageMap()
{
    // Propertymap fuer einen Outliner Text
    static const SfxItemPropertyMap aChartDrawPage[] =
    {
        { MAP_CHAR_LEN( "Width" ),	CHART_DRAW_PAGE_WIDTH_ID,	&::getCppuType((const sal_Int32*)0), 0, 0 },
        { MAP_CHAR_LEN( "Height" ),	CHART_DRAW_PAGE_HEIGHT_ID,	&::getCppuType((const sal_Int32*)0), 0, 0 },
        { 0,0,0,0,0 }
    };

    return aChartDrawPage;
}

ChXChartDrawPage::ChXChartDrawPage( ChartModel* pModel ) :
        SvxDrawPage( pModel? pModel->GetPage( 0 ): NULL ),
        mpModel( pModel ),
        maPropSet( ImplGetChartDrawPageMap() )
{
    DBG_ASSERT( pModel != NULL, "ChXChartDrawPage: Invalid model (=> invalid page)" );
}

ChXChartDrawPage::~ChXChartDrawPage() throw()
{}


// XInterface
uno::Any SAL_CALL ChXChartDrawPage::queryInterface( const uno::Type & rType )
    throw( uno::RuntimeException )
{
    uno::Any aAny = SvxDrawPage::queryInterface( rType );
    if( aAny.hasValue() )
        return aAny;

    return ::cppu::queryInterface(
        rType,
        ( beans::XPropertySet* )this );
}

void SAL_CALL ChXChartDrawPage::acquire() throw()
{
    SvxDrawPage::acquire();
}

void SAL_CALL ChXChartDrawPage::release() throw()
{
    SvxDrawPage::release();
}

// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL ChXChartDrawPage::getTypes() throw( uno::RuntimeException )
{
    if( maTypeSequence.getLength() == 0 )
    {
        const uno::Sequence< uno::Type > aBaseTypes( SvxDrawPage::getTypes() );
        const sal_Int32 nBaseTypes = aBaseTypes.getLength();
        const uno::Type* pBaseTypes = aBaseTypes.getConstArray();

        maTypeSequence.realloc( nBaseTypes + 1 );		// Note: Keep the size updated !!
        uno::Type* pTypes = maTypeSequence.getArray();

        *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);

        for( sal_Int32 nType = 0; nType < nBaseTypes; nType++ )
            *pTypes++ = *pBaseTypes++;
    }

    return maTypeSequence;
}

uno::Sequence< sal_Int8 > SAL_CALL ChXChartDrawPage::getImplementationId() throw( uno::RuntimeException )
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL ChXChartDrawPage::getPropertySetInfo()
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex());

    return maPropSet.getPropertySetInfo();
}

void SAL_CALL ChXChartDrawPage::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
    throw( beans::UnknownPropertyException,
           beans::PropertyVetoException,
           lang::IllegalArgumentException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex());

    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry( aPropertyName );
    if( mpModel &&
        pMap && pMap->nWID )
    {
        if( pMap->nFlags & beans::PropertyAttribute::READONLY )
            throw beans::PropertyVetoException();

        switch( pMap->nWID )
        {
            case CHART_DRAW_PAGE_WIDTH_ID:
                {
                    const SdrPage* pPage = mpModel->GetPage( 0 );
                    if( pPage )
                    {
                        sal_Int32 nWidth;
                        aValue >>= nWidth;
                        Size aSize = pPage->GetSize();
                        aSize.setWidth( nWidth );

                        mpModel->ResizePage( aSize );
                    }
                }
                break;
            case CHART_DRAW_PAGE_HEIGHT_ID:
                {
                    const SdrPage* pPage = mpModel->GetPage( 0 );
                    if( pPage )
                    {
                        sal_Int32 nHeight;
                        aValue >>= nHeight;
                        Size aSize = pPage->GetSize();
                        aSize.setHeight( nHeight );

                        mpModel->ResizePage( aSize );
                    }
                }
                break;
        }
    }
}

uno::Any SAL_CALL ChXChartDrawPage::getPropertyValue( const ::rtl::OUString& aPropertyName )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex());

    uno::Any aResult;
    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry( aPropertyName );

    if( mpModel &&
        pMap && pMap->nWID )
    {
        switch( pMap->nWID )
        {
            case CHART_DRAW_PAGE_WIDTH_ID:
                {
                    const SdrPage* pPage = mpModel->GetPage( 0 );
                    if( pPage )
                    {
                        Size aSize = pPage->GetSize();
                        aResult <<= aSize.getWidth();
                    }
                }
                break;
            case CHART_DRAW_PAGE_HEIGHT_ID:
                {
                    const SdrPage* pPage = mpModel->GetPage( 0 );
                    if( pPage )
                    {
                        Size aSize = pPage->GetSize();
                        aResult <<= aSize.getHeight();
                    }
                }
                break;
        }
    }
    return aResult;
}

void SAL_CALL ChXChartDrawPage::addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                                           const uno::Reference< beans::XPropertyChangeListener >& xListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{}

void SAL_CALL ChXChartDrawPage::removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                                              const uno::Reference< beans::XPropertyChangeListener >& aListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{}

void SAL_CALL ChXChartDrawPage::addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                                           const uno::Reference< beans::XVetoableChangeListener >& aListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{}

void SAL_CALL ChXChartDrawPage::removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                                              const uno::Reference< beans::XVetoableChangeListener >& aListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{}
}
