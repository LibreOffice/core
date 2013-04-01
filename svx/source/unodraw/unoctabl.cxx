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

#include <unotools/pathoptions.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <cppuhelper/implbase2.hxx>

#include "../customshapes/EnhancedCustomShapeEngine.hxx"

#include <svx/xtable.hxx>
#include "svx/unoshcol.hxx"
#include "recoveryui.hxx"
#include "svx/xmlgrhlp.hxx"
#include "tbunocontroller.hxx"
#include "tbunosearchcontrollers.hxx"

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::cppu;

class SvxUnoColorTable : public WeakImplHelper2< container::XNameContainer, lang::XServiceInfo >
{
private:
    XColorListRef pList;

public:
    SvxUnoColorTable() throw();
    virtual ~SvxUnoColorTable() throw();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const  OUString& ServiceName ) throw( uno::RuntimeException);
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames() throw( uno::RuntimeException);

    static OUString getImplementationName_Static() throw()
    {
        return OUString("com.sun.star.drawing.SvxUnoColorTable");
    }

    static uno::Sequence< OUString >  getSupportedServiceNames_Static(void) throw();

    // XNameContainer
    virtual void SAL_CALL insertByName( const  OUString& aName, const  uno::Any& aElement ) throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException);
    virtual void SAL_CALL removeByName( const  OUString& Name ) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const  OUString& aName, const  uno::Any& aElement ) throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

    // XNameAccess
    virtual uno::Any SAL_CALL getByName( const  OUString& aName ) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

    virtual uno::Sequence<  OUString > SAL_CALL getElementNames() throw( uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasByName( const  OUString& aName ) throw( uno::RuntimeException);

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw( uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw( uno::RuntimeException);
};

SvxUnoColorTable::SvxUnoColorTable() throw()
{
    pList = XPropertyList::CreatePropertyList( XCOLOR_LIST, SvtPathOptions().GetPalettePath() )->AsColorList();
}

SvxUnoColorTable::~SvxUnoColorTable() throw()
{
}

sal_Bool SAL_CALL SvxUnoColorTable::supportsService( const  OUString& ServiceName ) throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSNL( getSupportedServiceNames() );
    const OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;

    return sal_False;
}

OUString SAL_CALL SvxUnoColorTable::getImplementationName() throw( uno::RuntimeException )
{
    return OUString("SvxUnoColorTable");
}

uno::Sequence< OUString > SAL_CALL SvxUnoColorTable::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

uno::Sequence< OUString > SvxUnoColorTable::getSupportedServiceNames_Static(void) throw()
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString("com.sun.star.drawing.ColorTable" );
    return aSNS;
}

// XNameContainer
void SAL_CALL SvxUnoColorTable::insertByName( const OUString& aName, const uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException )
{
    if( hasByName( aName ) )
        throw container::ElementExistException();

    sal_Int32 nColor = 0;
    if( !(aElement >>= nColor) )
        throw lang::IllegalArgumentException();

    if( pList.is() )
    {
        XColorEntry* pEntry = new XColorEntry( Color( (ColorData)nColor ), aName  );
        pList->Insert( pEntry, pList->Count() );
    }
}

void SAL_CALL SvxUnoColorTable::removeByName( const OUString& Name )
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    long nIndex = pList.is() ? pList->Get( Name ) : -1;
    if( nIndex == -1 )
        throw container::NoSuchElementException();

    pList->Remove( nIndex );
}

// XNameReplace
void SAL_CALL SvxUnoColorTable::replaceByName( const OUString& aName, const uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    sal_Int32 nColor = 0;
    if( !(aElement >>= nColor) )
        throw lang::IllegalArgumentException();

    long nIndex = pList.is() ? pList->Get( aName ) : -1;
    if( nIndex == -1  )
        throw container::NoSuchElementException();

    XColorEntry* pEntry = new XColorEntry( Color( (ColorData)nColor ), aName );
    delete pList->Replace( nIndex, pEntry );
}

// XNameAccess
uno::Any SAL_CALL SvxUnoColorTable::getByName( const OUString& aName )
    throw( container::NoSuchElementException,  lang::WrappedTargetException, uno::RuntimeException)
{
    long nIndex = pList.is() ? pList->Get( aName ) : -1;
    if( nIndex == -1 )
        throw container::NoSuchElementException();

    XColorEntry* pEntry = pList->GetColor( nIndex );
    return uno::Any( (sal_Int32) pEntry->GetColor().GetRGBColor() );
}

uno::Sequence< OUString > SAL_CALL SvxUnoColorTable::getElementNames()
    throw( uno::RuntimeException )
{
    const long nCount = pList.is() ? pList->Count() : 0;

    uno::Sequence< OUString > aSeq( nCount );
    OUString* pStrings = aSeq.getArray();

    for( long nIndex = 0; nIndex < nCount; nIndex++ )
    {
        XColorEntry* pEntry = pList->GetColor( (long)nIndex );
        pStrings[nIndex] = pEntry->GetName();
    }

    return aSeq;
}

sal_Bool SAL_CALL SvxUnoColorTable::hasByName( const OUString& aName )
    throw( uno::RuntimeException )
{
    long nIndex = pList.is() ? pList->Get( aName ) : -1;
    return nIndex != -1;
}

// XElementAccess
uno::Type SAL_CALL SvxUnoColorTable::getElementType()
    throw( uno::RuntimeException )
{
    return ::getCppuType((const sal_Int32*)0);
}

sal_Bool SAL_CALL SvxUnoColorTable::hasElements()
    throw( uno::RuntimeException )
{
    return pList.is() && pList->Count() != 0;
}

/**
 * Create a colortable
 */
uno::Reference< uno::XInterface > SAL_CALL SvxUnoColorTable_createInstance(const uno::Reference< lang::XMultiServiceFactory > & ) throw(uno::Exception)
{
    return *new SvxUnoColorTable();
}
uno::Reference< uno::XInterface > SAL_CALL create_EnhancedCustomShapeEngine( const uno::Reference< lang::XMultiServiceFactory >& rxFact ) throw(uno::Exception)
{
    return *new EnhancedCustomShapeEngine( rxFact );
}

//
// export this service
//

#include "UnoGraphicExporter.hxx"
#include "unogalthemeprovider.hxx"
#include <com/sun/star/registry/XRegistryKey.hpp>
#include "sal/types.h"
#include "osl/diagnose.h"
#include "cppuhelper/factory.hxx"
#include "uno/lbnames.h"
#include <svx/sdr/primitive2d/primitiveFactory2d.hxx>

extern "C"
{

SAL_DLLPUBLIC_EXPORT void * SAL_CALL svx_component_getFactory (
    const sal_Char * pImplName, void * pServiceManager, void *  )
{
    void * pRet = 0;
    if( pServiceManager  )
    {
        uno::Reference< lang::XSingleServiceFactory > xFactory;

        if( rtl_str_compare( pImplName, "com.sun.star.drawing.SvxUnoColorTable" ) == 0 )
        {
            xFactory = createSingleFactory( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                SvxUnoColorTable::getImplementationName_Static(),
                SvxUnoColorTable_createInstance,
                SvxUnoColorTable::getSupportedServiceNames_Static() );
        }
        else if ( rtl_str_compare( pImplName, "com.sun.star.drawing.EnhancedCustomShapeEngine" ) == 0 )
        {
            xFactory = createSingleFactory( reinterpret_cast< lang::XMultiServiceFactory* >( pServiceManager ),
                EnhancedCustomShapeEngine_getImplementationName(),
                create_EnhancedCustomShapeEngine,
                EnhancedCustomShapeEngine_getSupportedServiceNames() );
        }
        else if( rtl_str_compare( pImplName, "com.sun.star.drawing.SvxShapeCollection" ) == 0 )
        {
            xFactory = createSingleFactory( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                SvxShapeCollection::getImplementationName_Static(),
                SvxShapeCollection_createInstance,
                SvxShapeCollection::getSupportedServiceNames_Static() );
        }
        else if( svx::RecoveryUI::st_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = ::cppu::createSingleFactory( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                svx::RecoveryUI::st_getImplementationName(),
                svx::RecoveryUI::st_createInstance,
                svx::RecoveryUI::st_getSupportedServiceNames() );
        }
        else if( svx::GraphicExporter_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = ::cppu::createSingleFactory( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                svx::GraphicExporter_getImplementationName(),
                svx::GraphicExporter_createInstance,
                svx::GraphicExporter_getSupportedServiceNames() );
        }
        else if ( svx::FontHeightToolBoxControl::getImplementationName_Static().equalsAscii( pImplName ) )
        {
            xFactory = createSingleFactory( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                svx::FontHeightToolBoxControl::getImplementationName_Static(),
                svx::FontHeightToolBoxControl_createInstance,
                svx::FontHeightToolBoxControl::getSupportedServiceNames_Static() );
        }
        else if ( svx::FindTextToolbarController::getImplementationName_Static().equalsAscii( pImplName ) )
        {
            xFactory = createSingleFactory( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                svx::FindTextToolbarController::getImplementationName_Static(),
                svx::FindTextToolbarController_createInstance,
                svx::FindTextToolbarController::getSupportedServiceNames_Static() );
        }
        else if ( svx::UpDownSearchToolboxController::getImplementationName_Static( svx::UpDownSearchToolboxController::DOWN ).equalsAscii( pImplName ) )
        {
            xFactory = createSingleFactory( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                svx::UpDownSearchToolboxController::getImplementationName_Static( svx::UpDownSearchToolboxController::DOWN ),
                svx::DownSearchToolboxController_createInstance,
                svx::UpDownSearchToolboxController::getSupportedServiceNames_Static() );
        }
        else if ( svx::UpDownSearchToolboxController::getImplementationName_Static( svx::UpDownSearchToolboxController::UP ).equalsAscii( pImplName ) )
        {
            xFactory = createSingleFactory( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                svx::UpDownSearchToolboxController::getImplementationName_Static( svx::UpDownSearchToolboxController::UP ),
                svx::UpSearchToolboxController_createInstance,
                svx::UpDownSearchToolboxController::getSupportedServiceNames_Static() );
        }
        else if ( svx::ExitSearchToolboxController::getImplementationName_Static().equalsAscii( pImplName ) )
        {
            xFactory = createSingleFactory( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                svx::ExitSearchToolboxController::getImplementationName_Static(),
                svx::ExitFindbarToolboxController_createInstance,
                svx::ExitSearchToolboxController::getSupportedServiceNames_Static() );
        }
        else if ( svx::FindbarDispatcher::getImplementationName_Static().equalsAscii( pImplName ) )
        {
            xFactory = createSingleFactory( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                svx::FindbarDispatcher::getImplementationName_Static(),
                svx::FindbarDispatcher_createInstance,
                svx::FindbarDispatcher::getSupportedServiceNames_Static() );
        }
        else if( ::unogallery::GalleryThemeProvider_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = ::cppu::createSingleFactory( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                ::unogallery::GalleryThemeProvider_getImplementationName(),
                ::unogallery::GalleryThemeProvider_createInstance,
                ::unogallery::GalleryThemeProvider_getSupportedServiceNames() );
        }
        else if( drawinglayer::primitive2d::PrimitiveFactory2D::getImplementationName_Static().equalsAscii( pImplName ) )
        {
            // XPrimitiveFactory2D
            xFactory = ::cppu::createSingleFactory( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                drawinglayer::primitive2d::PrimitiveFactory2D::getImplementationName_Static(),
                drawinglayer::primitive2d::XPrimitiveFactory2DProvider_createInstance,
                drawinglayer::primitive2d::PrimitiveFactory2D::getSupportedServiceNames_Static() );
        }
        else if( ::svx::SvXMLGraphicImportHelper_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = ::cppu::createSingleFactory(
                reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                ::svx::SvXMLGraphicImportHelper_getImplementationName(),
                ::svx::SvXMLGraphicImportHelper_createInstance,
                ::svx::SvXMLGraphicImportHelper_getSupportedServiceNames() );
        }
        else if( ::svx::SvXMLGraphicExportHelper_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = ::cppu::createSingleFactory(
                reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                ::svx::SvXMLGraphicExportHelper_getImplementationName(),
                ::svx::SvXMLGraphicExportHelper_createInstance,
                ::svx::SvXMLGraphicExportHelper_getSupportedServiceNames() );
        }
        if( xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
