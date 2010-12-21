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
#include "precompiled_svx.hxx"

#include <algorithm>

#include "unogaltheme.hxx"
#include "unogalitem.hxx"
#include "svx/galtheme.hxx"
#include "svx/gallery1.hxx"
#include "svx/galmisc.hxx"
#include <svx/fmmodel.hxx>
#include <rtl/uuid.h>
#include <vos/mutex.hxx>
#ifndef _SV_SVAPP_HXX_
#include <vcl/svapp.hxx>
#endif
#include <unotools/pathoptions.hxx>

using namespace ::com::sun::star;

namespace unogallery {

// -----------------
// - GalleryTheme -
// -----------------

GalleryTheme::GalleryTheme( const ::rtl::OUString& rThemeName )
{
    mpGallery = ::Gallery::GetGalleryInstance();
    mpTheme = ( mpGallery ? mpGallery->AcquireTheme( rThemeName, *this ) : NULL );

    if( mpGallery )
        StartListening( *mpGallery );
}

// ------------------------------------------------------------------------------

GalleryTheme::~GalleryTheme()
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );

    DBG_ASSERT( !mpTheme || mpGallery, "Theme is living without Gallery" );

    implReleaseItems( NULL );

    if( mpGallery )
    {
        EndListening( *mpGallery );

        if( mpTheme )
            mpGallery->ReleaseTheme( mpTheme, *this );
    }
}

// ------------------------------------------------------------------------------

::rtl::OUString GalleryTheme::getImplementationName_Static()
    throw()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.gallery.GalleryTheme" ) );
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > GalleryTheme::getSupportedServiceNames_Static()
    throw()
{
    uno::Sequence< ::rtl::OUString > aSeq( 1 );

    aSeq.getArray()[ 0 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.gallery.GalleryTheme" ) );

    return aSeq;
}

// ------------------------------------------------------------------------------

::rtl::OUString SAL_CALL GalleryTheme::getImplementationName()
    throw( uno::RuntimeException )
{
    return getImplementationName_Static();
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL GalleryTheme::supportsService( const ::rtl::OUString& ServiceName )
    throw( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString >    aSNL( getSupportedServiceNames() );
    const ::rtl::OUString*              pArray = aSNL.getConstArray();

    for( int i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return true;

    return false;
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL GalleryTheme::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

// ------------------------------------------------------------------------------

uno::Sequence< uno::Type > SAL_CALL GalleryTheme::getTypes()
    throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type >  aTypes( 5 );
    uno::Type*                  pTypes = aTypes.getArray();

    *pTypes++ = ::getCppuType((const uno::Reference< lang::XServiceInfo>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< lang::XTypeProvider>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< container::XElementAccess>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< container::XIndexAccess>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< gallery::XGalleryTheme>*)0);

    return aTypes;
}

// ------------------------------------------------------------------------------

uno::Sequence< sal_Int8 > SAL_CALL GalleryTheme::getImplementationId()
    throw(uno::RuntimeException)
{
    const vos::OGuard                   aGuard( Application::GetSolarMutex() );
    static uno::Sequence< sal_Int8 >    aId;

    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( reinterpret_cast< sal_uInt8* >( aId.getArray() ), 0, sal_True );
    }

    return aId;
}

// ------------------------------------------------------------------------------

uno::Type SAL_CALL GalleryTheme::getElementType()
    throw (uno::RuntimeException)
{
    return ::getCppuType( (const uno::Reference< gallery::XGalleryItem >*) 0);
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL GalleryTheme::hasElements()
    throw (uno::RuntimeException)
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );

    return( ( mpTheme != NULL ) && ( mpTheme->GetObjectCount() > 0 ) );
}

// ------------------------------------------------------------------------------

sal_Int32 SAL_CALL GalleryTheme::getCount()
    throw (uno::RuntimeException)
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );

    return( mpTheme ? mpTheme->GetObjectCount() : 0 );
}

// ------------------------------------------------------------------------------

uno::Any SAL_CALL GalleryTheme::getByIndex( ::sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );
    uno::Any            aRet;

    if( mpTheme )
    {
        if( ( nIndex < 0 ) || ( nIndex >= getCount() ) )
        {
            throw lang::IndexOutOfBoundsException();
        }
        else
        {
            const GalleryObject* pObj = mpTheme->ImplGetGalleryObject( nIndex );

            if( pObj )
                aRet = uno::makeAny( uno::Reference< gallery::XGalleryItem >( new GalleryItem( *this, *pObj ) ) );
        }
    }

    return aRet;
}

// ------------------------------------------------------------------------------

::rtl::OUString SAL_CALL GalleryTheme::getName(  )
    throw (uno::RuntimeException)
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );
    ::rtl::OUString     aRet;

    if( mpTheme )
        aRet = mpTheme->GetName();

    return aRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL GalleryTheme::update(  )
    throw (uno::RuntimeException)
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );

    if( mpTheme )
    {
        const Link aDummyLink;
        mpTheme->Actualize( aDummyLink );
    }
}

// ------------------------------------------------------------------------------

::sal_Int32 SAL_CALL GalleryTheme::insertURLByIndex(
    const ::rtl::OUString& rURL, ::sal_Int32 nIndex )
    throw (lang::WrappedTargetException, uno::RuntimeException)
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );
    sal_Int32           nRet = -1;

    if( mpTheme )
    {
        try
        {
            const INetURLObject aURL( rURL );

            nIndex = ::std::max( ::std::min( nIndex, getCount() ), sal_Int32( 0 ) );

            if( ( aURL.GetProtocol() != INET_PROT_NOT_VALID ) && mpTheme->InsertURL( aURL, nIndex ) )
            {
                const GalleryObject* pObj = mpTheme->ImplGetGalleryObject( aURL );

                if( pObj )
                    nRet = mpTheme->ImplGetGalleryObjectPos( pObj );
            }
        }
        catch( ... )
        {
        }
    }

    return nRet;
}

// ------------------------------------------------------------------------------

::sal_Int32 SAL_CALL GalleryTheme::insertGraphicByIndex(
    const uno::Reference< graphic::XGraphic >& rxGraphic, sal_Int32 nIndex )
    throw (lang::WrappedTargetException, uno::RuntimeException)
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );
    sal_Int32           nRet = -1;

    if( mpTheme )
    {
        try
        {
            const Graphic aGraphic( rxGraphic );

            nIndex = ::std::max( ::std::min( nIndex, getCount() ), sal_Int32( 0 ) );

            if( mpTheme->InsertGraphic( aGraphic, nIndex ) )
                nRet = nIndex;
        }
        catch( ... )
        {
        }
    }

    return nRet;
}

// ------------------------------------------------------------------------------

::sal_Int32 SAL_CALL GalleryTheme::insertDrawingByIndex(
    const uno::Reference< lang::XComponent >& Drawing, sal_Int32 nIndex )
    throw (lang::WrappedTargetException, uno::RuntimeException)
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );
    sal_Int32           nRet = -1;

    if( mpTheme )
    {
        GalleryDrawingModel* pModel = GalleryDrawingModel::getImplementation( Drawing );

        if( pModel && pModel->GetDoc() && pModel->GetDoc()->ISA( FmFormModel ) )
        {
            nIndex = ::std::max( ::std::min( nIndex, getCount() ), sal_Int32( 0 ) );

            if( mpTheme->InsertModel( *static_cast< FmFormModel* >( pModel->GetDoc() ), nIndex ) )
                nRet = nIndex;
        }
    }

    return nRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL GalleryTheme::removeByIndex( sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );

    if( mpTheme )
    {
        if( ( nIndex < 0 ) || ( nIndex >= getCount() ) )
            throw lang::IndexOutOfBoundsException();
        else
            mpTheme->RemoveObject( nIndex );
    }
}

// ------------------------------------------------------------------------------

void GalleryTheme::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );
    const GalleryHint&  rGalleryHint = static_cast< const GalleryHint& >( rHint );

    switch( rGalleryHint.GetType() )
    {
        case( GALLERY_HINT_CLOSE_THEME ):
        {
            DBG_ASSERT( !mpTheme || mpGallery, "Theme is living without Gallery" );

            implReleaseItems( NULL );

            if( mpGallery && mpTheme )
            {
                mpGallery->ReleaseTheme( mpTheme, *this );
                mpTheme = NULL;
            }
        }
        break;

        case( GALLERY_HINT_CLOSE_OBJECT ):
        {
            GalleryObject* pObj = reinterpret_cast< GalleryObject* >( rGalleryHint.GetData1() );

            if( pObj )
                implReleaseItems( pObj );
        }
        break;

        default:
        break;
    }
}

// ------------------------------------------------------------------------------

void GalleryTheme::implReleaseItems( GalleryObject* pObj )
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );

    for( GalleryItemList::iterator aIter = maItemList.begin(); aIter != maItemList.end();  )
    {
        if( !pObj || ( (*aIter)->implGetObject() == pObj ) )
        {
            (*aIter)->implSetInvalid();
            aIter = maItemList.erase( aIter );
        }
        else
            ++aIter;
    }
}

// ------------------------------------------------------------------------------

::GalleryTheme* GalleryTheme::implGetTheme() const
{
    return mpTheme;
}

// ------------------------------------------------------------------------------

void GalleryTheme::implRegisterGalleryItem( ::unogallery::GalleryItem& rItem )
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );

//  DBG_ASSERT( maItemList.find( &rItem ) == maItemList.end(), "Item already registered" );
    maItemList.push_back( &rItem );
}

// ------------------------------------------------------------------------------

void GalleryTheme::implDeregisterGalleryItem( ::unogallery::GalleryItem& rItem )
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );

//  DBG_ASSERT( maItemList.find( &rItem ) != maItemList.end(), "Item is not registered" );
    maItemList.remove( &rItem );
}

}
