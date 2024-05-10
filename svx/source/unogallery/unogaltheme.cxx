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

#include <algorithm>

#include "unogaltheme.hxx"
#include "unogalitem.hxx"
#include <svx/galtheme.hxx>
#include <svx/gallery1.hxx>
#include <svx/galmisc.hxx>
#include <svx/fmmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/unopage.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star;

namespace unogallery {


GalleryTheme::GalleryTheme( std::u16string_view rThemeName )
{
    mpGallery = ::Gallery::GetGalleryInstance();
    mpTheme = ( mpGallery ? mpGallery->AcquireTheme( rThemeName, *this ) : nullptr );

    if( mpGallery )
        StartListening( *mpGallery );
}


GalleryTheme::~GalleryTheme()
{
    const SolarMutexGuard aGuard;

    DBG_ASSERT( !mpTheme || mpGallery, "Theme is living without Gallery" );

    implReleaseItems( nullptr );

    if( mpGallery )
    {
        EndListening( *mpGallery );

        if( mpTheme )
            mpGallery->ReleaseTheme( mpTheme, *this );
    }
}


OUString SAL_CALL GalleryTheme::getImplementationName()
{
    return u"com.sun.star.comp.gallery.GalleryTheme"_ustr;
}

sal_Bool SAL_CALL GalleryTheme::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence< OUString > SAL_CALL GalleryTheme::getSupportedServiceNames()
{
    return { u"com.sun.star.gallery.GalleryTheme"_ustr };
}

uno::Sequence< uno::Type > SAL_CALL GalleryTheme::getTypes()
{
    static const uno::Sequence aTypes {
        cppu::UnoType<lang::XServiceInfo>::get(),
        cppu::UnoType<lang::XTypeProvider>::get(),
        cppu::UnoType<container::XElementAccess>::get(),
        cppu::UnoType<container::XIndexAccess>::get(),
        cppu::UnoType<gallery::XGalleryTheme>::get(),
    };
    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL GalleryTheme::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}


uno::Type SAL_CALL GalleryTheme::getElementType()
{
    return cppu::UnoType<gallery::XGalleryItem>::get();
}


sal_Bool SAL_CALL GalleryTheme::hasElements()
{
    const SolarMutexGuard aGuard;

    return( ( mpTheme != nullptr ) && ( mpTheme->GetObjectCount() > 0 ) );
}


sal_Int32 SAL_CALL GalleryTheme::getCount()
{
    const SolarMutexGuard aGuard;

    return( mpTheme ? mpTheme->GetObjectCount() : 0 );
}


uno::Any SAL_CALL GalleryTheme::getByIndex( ::sal_Int32 nIndex )
{
    const SolarMutexGuard aGuard;
    uno::Any            aRet;

    if( mpTheme )
    {
        if( ( nIndex < 0 ) || ( nIndex >= getCount() ) )
        {
            throw lang::IndexOutOfBoundsException();
        }
        const GalleryObject* pObj = mpTheme->maGalleryObjectCollection.getForPosition( nIndex );

        if( pObj )
            aRet <<= uno::Reference< gallery::XGalleryItem >( new GalleryItem( *this, *pObj ) );
    }

    return aRet;
}


OUString SAL_CALL GalleryTheme::getName(  )
{
    const SolarMutexGuard aGuard;
    OUString     aRet;

    if( mpTheme )
        aRet = mpTheme->GetName();

    return aRet;
}


void SAL_CALL GalleryTheme::update(  )
{
    const SolarMutexGuard aGuard;

    if( mpTheme )
    {
        const Link<const INetURLObject&, void> aDummyLink;
        mpTheme->Actualize( aDummyLink );
    }
}


::sal_Int32 SAL_CALL GalleryTheme::insertURLByIndex(
    const OUString& rURL, ::sal_Int32 nIndex )
{
    const SolarMutexGuard aGuard;
    sal_Int32           nRet = -1;

    if( mpTheme )
    {
        try
        {
            const INetURLObject aURL( rURL );

            nIndex = std::clamp( nIndex, sal_Int32(0), getCount() );

            if( ( aURL.GetProtocol() != INetProtocol::NotValid ) && mpTheme->InsertURL( aURL, nIndex ) )
            {
                const GalleryObject* pObj = mpTheme->maGalleryObjectCollection.searchObjectWithURL( aURL );

                if( pObj )
                    nRet = mpTheme->maGalleryObjectCollection.searchPosWithObject( pObj );
            }
        }
        catch( ... )
        {
        }
    }

    return nRet;
}


::sal_Int32 SAL_CALL GalleryTheme::insertGraphicByIndex(
    const uno::Reference< graphic::XGraphic >& rxGraphic, sal_Int32 nIndex )
{
    const SolarMutexGuard aGuard;
    sal_Int32           nRet = -1;

    if( mpTheme )
    {
        try
        {
            const Graphic aGraphic( rxGraphic );

            nIndex = std::clamp( nIndex, sal_Int32(0), getCount() );

            if( mpTheme->InsertGraphic( aGraphic, nIndex ) )
                nRet = nIndex;
        }
        catch( ... )
        {
        }
    }

    return nRet;
}


::sal_Int32 SAL_CALL GalleryTheme::insertDrawingByIndex(
    const uno::Reference< lang::XComponent >& Drawing, sal_Int32 nIndex )
{
    const SolarMutexGuard aGuard;
    sal_Int32 nRet = -1;

    if( mpTheme )
    {
        GalleryDrawingModel* pModel = comphelper::getFromUnoTunnel<GalleryDrawingModel>( Drawing );

        if( pModel && dynamic_cast<const FmFormModel*>(pModel->GetDoc()) )
        {
            // Here we're inserting something that's already a gallery theme drawing
            nIndex = std::clamp( nIndex, sal_Int32(0), getCount() );

            if( mpTheme->InsertModel( *static_cast< FmFormModel* >( pModel->GetDoc() ), nIndex ) )
                nRet = nIndex;
        }
        else if (!pModel)
        {
            // #i80184# Try to do the right thing and make a Gallery drawing out
            // of an ordinary Drawing if possible.
            try
            {
                uno::Reference< drawing::XDrawPagesSupplier > xDrawPagesSupplier( Drawing, uno::UNO_QUERY_THROW );
                uno::Reference< drawing::XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), uno::UNO_SET_THROW );
                uno::Reference< drawing::XDrawPage > xPage( xDrawPages->getByIndex( 0 ), uno::UNO_QUERY_THROW );
                SvxDrawPage* pUnoPage = xPage.is() ? comphelper::getFromUnoTunnel<SvxDrawPage>( xPage ) : nullptr;
                SdrModel* pOrigModel = pUnoPage ? &pUnoPage->GetSdrPage()->getSdrModelFromSdrPage() : nullptr;
                SdrPage* pOrigPage = pUnoPage ? pUnoPage->GetSdrPage() : nullptr;

                if (pOrigPage && pOrigModel)
                {
                    FmFormModel* pTmpModel = new FmFormModel(&pOrigModel->GetItemPool());
                    // Clone to new target SdrModel
                    rtl::Reference<SdrPage> pNewPage = pOrigPage->CloneSdrPage(*pTmpModel);
                    pTmpModel->InsertPage(pNewPage.get(), 0);

                    rtl::Reference< GalleryDrawingModel > xDrawing( new GalleryDrawingModel( pTmpModel ) );
                    pTmpModel->setUnoModel( xDrawing );

                    nRet = insertDrawingByIndex( xDrawing, nIndex );
                    return nRet;
                }
            }
            catch (...)
            {
            }
        }
    }

    return nRet;
}


void SAL_CALL GalleryTheme::removeByIndex( sal_Int32 nIndex )
{
    const SolarMutexGuard aGuard;

    if( mpTheme )
    {
        if( ( nIndex < 0 ) || ( nIndex >= getCount() ) )
            throw lang::IndexOutOfBoundsException();
        mpTheme->RemoveObject( nIndex );
    }
}


void GalleryTheme::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const SolarMutexGuard aGuard;
    const GalleryHint&  rGalleryHint = static_cast< const GalleryHint& >( rHint );

    switch( rGalleryHint.GetType() )
    {
        case GalleryHintType::CLOSE_THEME:
        {
            DBG_ASSERT( !mpTheme || mpGallery, "Theme is living without Gallery" );

            implReleaseItems( nullptr );

            if( mpGallery && mpTheme )
            {
                mpGallery->ReleaseTheme( mpTheme, *this );
                mpTheme = nullptr;
            }
        }
        break;

        case GalleryHintType::CLOSE_OBJECT:
        {
            GalleryObject* pObj = static_cast< GalleryObject* >( rGalleryHint.GetData1() );

            if( pObj )
                implReleaseItems( pObj );
        }
        break;

        default:
        break;
    }
}


void GalleryTheme::implReleaseItems( GalleryObject const * pObj )
{
    const SolarMutexGuard aGuard;

    for( GalleryItemVector::iterator aIter = maItemVector.begin(); aIter != maItemVector.end();  )
    {
        if( !pObj || ( (*aIter)->implGetObject() == pObj ) )
        {
            (*aIter)->implSetInvalid();
            aIter = maItemVector.erase( aIter );
        }
        else
            ++aIter;
    }
}


void GalleryTheme::implRegisterGalleryItem( ::unogallery::GalleryItem& rItem )
{
    const SolarMutexGuard aGuard;

    maItemVector.push_back( &rItem );
}


void GalleryTheme::implDeregisterGalleryItem( ::unogallery::GalleryItem& rItem )
{
    const SolarMutexGuard aGuard;

    std::erase(maItemVector, &rItem);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
