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
    XColorTable*    pTable;

public:
    SvxUnoColorTable() throw();
    virtual ~SvxUnoColorTable() throw();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const  OUString& ServiceName ) throw( uno::RuntimeException);
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);

    static OUString getImplementationName_Static() throw()
    {
        return OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.SvxUnoColorTable"));
    }

    static uno::Sequence< OUString >  getSupportedServiceNames_Static(void) throw();

    // XNameContainer
    virtual void SAL_CALL insertByName( const  OUString& aName, const  uno::Any& aElement ) throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException);
    virtual void SAL_CALL removeByName( const  OUString& Name ) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const  OUString& aName, const  uno::Any& aElement ) throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

    // XNameAccess
    virtual uno::Any SAL_CALL getByName( const  OUString& aName ) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

    virtual uno::Sequence<  OUString > SAL_CALL getElementNames(  ) throw( uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasByName( const  OUString& aName ) throw( uno::RuntimeException);

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw( uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw( uno::RuntimeException);
};

SvxUnoColorTable::SvxUnoColorTable() throw()
{
    pTable = new XColorTable( SvtPathOptions().GetPalettePath() );
}

SvxUnoColorTable::~SvxUnoColorTable() throw()
{
    delete pTable;
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
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SvxUnoColorTable") );
}

uno::Sequence< OUString > SAL_CALL SvxUnoColorTable::getSupportedServiceNames(  )
    throw( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

uno::Sequence< OUString > SvxUnoColorTable::getSupportedServiceNames_Static(void) throw()
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ColorTable" ));
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

    if( pTable )
    {
        XColorEntry* pEntry = new XColorEntry( Color( (ColorData)nColor ), aName  );
        pTable->Insert( pTable->Count(), pEntry );
    }
}

void SAL_CALL SvxUnoColorTable::removeByName( const OUString& Name )
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    long nIndex = pTable ? ((XPropertyTable*)pTable)->Get( Name ) : -1;
    if( nIndex == -1 )
        throw container::NoSuchElementException();

    pTable->Remove( nIndex );
}

// XNameReplace
void SAL_CALL SvxUnoColorTable::replaceByName( const OUString& aName, const uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    sal_Int32 nColor = 0;
    if( !(aElement >>= nColor) )
        throw lang::IllegalArgumentException();

    long nIndex = pTable ? ((XPropertyTable*)pTable)->Get( aName ) : -1;
    if( nIndex == -1  )
        throw container::NoSuchElementException();

    XColorEntry* pEntry = new XColorEntry( Color( (ColorData)nColor ), aName );
    delete pTable->Replace( nIndex, pEntry );
}

// XNameAccess
uno::Any SAL_CALL SvxUnoColorTable::getByName( const  OUString& aName )
    throw( container::NoSuchElementException,  lang::WrappedTargetException, uno::RuntimeException)
{
    long nIndex = pTable ? ((XPropertyTable*)pTable)->Get( aName ) : -1;
    if( nIndex == -1 )
        throw container::NoSuchElementException();

    XColorEntry* pEntry = ((XColorTable*)pTable)->GetColor( nIndex );
    return uno::Any( (sal_Int32) pEntry->GetColor().GetRGBColor() );
}

uno::Sequence< OUString > SAL_CALL SvxUnoColorTable::getElementNames(  )
    throw( uno::RuntimeException )
{
    const long nCount = pTable ? pTable->Count() : 0;

    uno::Sequence< OUString > aSeq( nCount );
    OUString* pStrings = aSeq.getArray();

    for( long nIndex = 0; nIndex < nCount; nIndex++ )
    {
        XColorEntry* pEntry = pTable->GetColor( (long)nIndex );
        pStrings[nIndex] = pEntry->GetName();
    }

    return aSeq;
}

sal_Bool SAL_CALL SvxUnoColorTable::hasByName( const OUString& aName )
    throw( uno::RuntimeException )
{
    long nIndex = pTable ? ((XPropertyTable*)pTable)->Get( aName ) : -1;
    return nIndex != -1;
}

// XElementAccess
uno::Type SAL_CALL SvxUnoColorTable::getElementType(  )
    throw( uno::RuntimeException )
{
    return ::getCppuType((const sal_Int32*)0);
}

sal_Bool SAL_CALL SvxUnoColorTable::hasElements(  )
    throw( uno::RuntimeException )
{
    return pTable && pTable->Count() != 0;
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

/*
namespace svx
{
extern OUString SAL_CALL ExtrusionDepthController_getImplementationName();
extern uno::Reference< uno::XInterface > SAL_CALL ExtrusionDepthController_createInstance(const uno::Reference< lang::XMultiServiceFactory > &)  throw( uno::RuntimeException );
extern uno::Sequence< OUString > SAL_CALL ExtrusionDepthController_getSupportedServiceNames() throw( uno::RuntimeException );
}
*/

extern "C"
{

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment (
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory (
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
            xFactory = createSingleFactory( reinterpret_cast< NMSP_LANG::XMultiServiceFactory* >( pServiceManager ),
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
        else if ( svx::DownSearchToolboxController::getImplementationName_Static().equalsAscii( pImplName ) )
        {
            xFactory = createSingleFactory( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                svx::DownSearchToolboxController::getImplementationName_Static(),
                svx::DownSearchToolboxController_createInstance,
                svx::DownSearchToolboxController::getSupportedServiceNames_Static() );
        }
        else if ( svx::UpSearchToolboxController::getImplementationName_Static().equalsAscii( pImplName ) )
        {
            xFactory = createSingleFactory( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                svx::UpSearchToolboxController::getImplementationName_Static(),
                svx::UpSearchToolboxController_createInstance,
                svx::UpSearchToolboxController::getSupportedServiceNames_Static() );
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
/*
        else if( ::svx::ExtrusionDepthController_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = ::cppu::createSingleFactory(
                reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                ::svx::ExtrusionDepthController_getImplementationName(),
                ::svx::ExtrusionDepthController_createInstance,
                ::svx::ExtrusionDepthController_getSupportedServiceNames() );
        }
*/
        if( xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

}
