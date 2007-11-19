/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoctabl.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-19 17:22:03 $
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
#include "precompiled_svx.hxx"

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#include "../customshapes/EnhancedCustomShapeEngine.hxx"

#include <svx/xtable.hxx>
#include "unoshcol.hxx"
#include "recoveryui.hxx"
#include "xmlgrhlp.hxx"
#include "tbunocontroller.hxx"

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

    for( INT32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return TRUE;

    return FALSE;
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

    INT32 nColor = 0;
    if( aElement >>= nColor )
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
    INT32 nColor = 0;
    if( aElement >>= nColor )
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

#ifndef SVX_LIGHT
#include "UnoGraphicExporter.hxx"
#endif
#include "unogalthemeprovider.hxx"

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include "osl/diagnose.h"
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include "cppuhelper/factory.hxx"
#endif

#ifndef _UNO_LBNAMES_H_
#include "uno/lbnames.h"
#endif

extern "C"
{

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment (
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

static void writeInfo (
    registry::XRegistryKey * pRegistryKey,
    const OUString& rImplementationName,
    const uno::Sequence< OUString >& rServices)
{
    uno::Reference< registry::XRegistryKey > xNewKey(
        pRegistryKey->createKey(
            OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) + rImplementationName + OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") ) ) );

    for( sal_Int32 i = 0; i < rServices.getLength(); i++ )
        xNewKey->createKey( rServices.getConstArray()[i]);
}

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo (
    void * , void * pRegistryKey)
{
    if( pRegistryKey )
    {
        try
        {
            registry::XRegistryKey *pKey = reinterpret_cast< registry::XRegistryKey * >( pRegistryKey );

            writeInfo( pKey, SvxShapeCollection::getImplementationName_Static(), SvxShapeCollection::getSupportedServiceNames_Static() );
            writeInfo( pKey, SvxUnoColorTable::getImplementationName_Static(), SvxUnoColorTable::getSupportedServiceNames_Static() );
            writeInfo( pKey, EnhancedCustomShapeEngine_getImplementationName(), EnhancedCustomShapeEngine_getSupportedServiceNames() );
            writeInfo( pKey, svx::RecoveryUI::st_getImplementationName(), svx::RecoveryUI::st_getSupportedServiceNames() );
#ifndef SVX_LIGHT
            writeInfo( pKey, svx::GraphicExporter_getImplementationName(), svx::GraphicExporter_getSupportedServiceNames() );
#endif
            writeInfo( pKey, svx::FontHeightToolBoxControl::getImplementationName_Static(), svx::FontHeightToolBoxControl::getSupportedServiceNames_Static() );
            writeInfo( pKey, ::unogallery::GalleryThemeProvider_getImplementationName(),::unogallery::GalleryThemeProvider_getSupportedServiceNames() );
            writeInfo( pKey, ::svx::SvXMLGraphicImportHelper_getImplementationName(),::svx::SvXMLGraphicImportHelper_getSupportedServiceNames() );
            writeInfo( pKey, ::svx::SvXMLGraphicExportHelper_getImplementationName(),::svx::SvXMLGraphicExportHelper_getSupportedServiceNames() );
        }
        catch (registry::InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }

    return sal_True;
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
#ifndef SVX_LIGHT
        else if( svx::GraphicExporter_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = ::cppu::createSingleFactory( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                svx::GraphicExporter_getImplementationName(),
                svx::GraphicExporter_createInstance,
                svx::GraphicExporter_getSupportedServiceNames() );
        }
#endif
        else if ( svx::FontHeightToolBoxControl::getImplementationName_Static().equalsAscii( pImplName ) )
        {
            xFactory = createSingleFactory( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                svx::FontHeightToolBoxControl::getImplementationName_Static(),
                svx::FontHeightToolBoxControl_createInstance,
                svx::FontHeightToolBoxControl::getSupportedServiceNames_Static() );
        }
        else if( ::unogallery::GalleryThemeProvider_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = ::cppu::createSingleFactory( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                ::unogallery::GalleryThemeProvider_getImplementationName(),
                ::unogallery::GalleryThemeProvider_createInstance,
                ::unogallery::GalleryThemeProvider_getSupportedServiceNames() );
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
