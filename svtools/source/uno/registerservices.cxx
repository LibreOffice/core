/*************************************************************************
 *
 *  $RCSfile: registerservices.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:11:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _SVT_UNOIFACE_HXX
#include "unoiface.hxx"
#endif
#ifndef _PRODUCE_HXX
#include "imgprod.hxx"
#endif

#ifndef _TOOLKIT_HELPER_MACROS_HXX_
#include <toolkit/helper/macros.hxx>
#endif

#ifndef SVTOOLS_PRODUCTREGISTRATION_HXX
#include "productregistration.hxx"
#endif

#include <algorithm>
#include <functional>

#define IMPL_CREATEINSTANCE( ImplName ) \
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ImplName##_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& ) \
    { return ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface >( ( ::cppu::OWeakObject* ) new ImplName ); }

#define IMPL_CREATEINSTANCE2( ImplName ) \
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ImplName##_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rSMgr) \
    { return ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface >( ( ::cppu::OWeakObject* ) new ImplName( rSMgr ) ); }

// for CreateInstance functions implemented elsewhere
#define DECLARE_CREATEINSTANCE( ImplName ) \
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ImplName##_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );

// for CreateInstance functions implemented elsewhere, while the function is within a namespace
#define DECLARE_CREATEINSTANCE_NAMESPACE( nmspe, ImplName ) \
    namespace nmspe {   \
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ImplName##_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );    \
    }

// -------------------------------------------------------------------------------------

DECLARE_CREATEINSTANCE( SvNumberFormatterServiceObj )
DECLARE_CREATEINSTANCE( SvNumberFormatsSupplierServiceObject )
DECLARE_CREATEINSTANCE( ImageProducer )
DECLARE_CREATEINSTANCE_NAMESPACE( svt, OAddressBookSourceDialogUno )
DECLARE_CREATEINSTANCE( SvFilterOptionsDialog )
DECLARE_CREATEINSTANCE_NAMESPACE( svt, SvtFolderPicker )
DECLARE_CREATEINSTANCE_NAMESPACE( svt, SvtFilePicker )
DECLARE_CREATEINSTANCE_NAMESPACE( svt, SvtOfficeFolderPicker )
DECLARE_CREATEINSTANCE_NAMESPACE( svt, SvtOfficeFilePicker )

IMPL_CREATEINSTANCE( ImageProducer );

namespace svt
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::registry;
    using namespace ::com::sun::star::lang;

    //--------------------------------------------------------------------
    struct CreateSubKey : public ::std::unary_function< ::rtl::OUString, void >
    {
        const Reference< XRegistryKey >& m_rxParentKey;
        CreateSubKey( const Reference< XRegistryKey >& _rxParentKey ) : m_rxParentKey( _rxParentKey ) { }

        void operator()( const ::rtl::OUString& _rSubKeyName )
        {
            m_rxParentKey->createKey( _rSubKeyName );
        }
    };

    //--------------------------------------------------------------------
    void RegisterImplementation(
            const Reference< XRegistryKey >& _rxLocation,
            const ::rtl::OUString& _rImplName,
            const Sequence< ::rtl::OUString >& _rSupportedServices
        )
    {
        try
        {
            ::rtl::OUString sImplKeyName( '/' );
            sImplKeyName += _rImplName;
            sImplKeyName += ::rtl::OUString::createFromAscii( "/UNO/SERVICES" );
            Reference< XRegistryKey > xNewKey = _rxLocation->createKey( sImplKeyName );

            OSL_ENSURE( xNewKey.is(), "::RegisterImplementation: invalid new impl key!" );
            if ( xNewKey.is() )
            {
                // create sub keys for all services supported by this implementation
                ::std::for_each(
                    _rSupportedServices.getConstArray(),
                    _rSupportedServices.getConstArray() + _rSupportedServices.getLength(),
                    CreateSubKey( xNewKey )
                );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "::RegisterImplementation: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    /** helper class for implementing component_getFactory
    */
    class FactoryCreator
    {
    private:
        Reference< XMultiServiceFactory >   m_xORB;
        ::rtl::OUString                     m_sRequestedComponent;

        Reference< XSingleServiceFactory >  m_xCreatedFactory;

    public:
        // ctor
        FactoryCreator(
            const Reference< XMultiServiceFactory >& _rxORB,
            const ::rtl::OUString& _rRequestedImplName
        );

        // get the return value, which is the last factory created in one of the tryXXX methods
        const Reference< XSingleServiceFactory >&
                    getCreatedFactory( ) const { return m_xCreatedFactory; }

        /** try the given service name, create a single service factory for it if it matches the impl name given
            in the ctor
        */
        sal_Bool    trySingleServiceFactory(
                        const ::rtl::OUString& _rImplName,
                        const Sequence< ::rtl::OUString >& _rSupportedServices,
                        ::cppu::ComponentInstantiation _pComponentCreator
                    );
    };

    //--------------------------------------------------------------------
    FactoryCreator::FactoryCreator( const Reference< XMultiServiceFactory >& _rxORB, const ::rtl::OUString& _rRequestedImplName )
        :m_xORB( _rxORB )
        ,m_sRequestedComponent( _rRequestedImplName )
    {
    }

    //--------------------------------------------------------------------
    sal_Bool FactoryCreator::trySingleServiceFactory( const ::rtl::OUString& _rImplName, const Sequence< ::rtl::OUString >& _rSupportedServices,
        ::cppu::ComponentInstantiation _pComponentCreator )
    {
        OSL_ENSURE( !getCreatedFactory( ).is(), "FactoryCreator::trySingleServiceFactory: already have a factory!" );
            // aprevious tryXXX returned true if this here fails ....
        if ( getCreatedFactory( ).is() )
            return sal_False;

        if ( _rImplName.equals( m_sRequestedComponent ) )
        {
            m_xCreatedFactory = ::cppu::createSingleFactory(
                m_xORB, m_sRequestedComponent, _pComponentCreator, _rSupportedServices
            );
        }

        return m_xCreatedFactory.is();
    }
}

extern "C"
{

void SAL_CALL component_getImplementationEnvironment( const sal_Char** ppEnvTypeName, uno_Environment** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo( void* _pServiceManager, void* _pRegistryKey )
{
    if (_pRegistryKey)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey > xRegistryKey;
        xRegistryKey = reinterpret_cast< ::com::sun::star::registry::XRegistryKey* >( _pRegistryKey );
        ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey > xNewKey;

        xNewKey = xRegistryKey->createKey( ::rtl::OUString::createFromAscii( "/stardiv.svtools.SvtTextLoader/UNO/SERVICES" ) );
        xNewKey->createKey( ::rtl::OUString::createFromAscii( "stardiv.one.frame.FrameLoader" ) );
        xNewKey->createKey( ::rtl::OUString::createFromAscii( "stardiv.one.frame.TextLoader" ) );
        xNewKey = xRegistryKey->createKey( ::rtl::OUString::createFromAscii( "/stardiv.svtools.SvtTextLoader/UNO/Loader" ) );
        ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >  xLoaderKey = xRegistryKey->createKey( ::rtl::OUString::createFromAscii( "/stardiv.svtools.SvtTextLoader/Loader" ) );
        xNewKey = xLoaderKey->createKey( ::rtl::OUString::createFromAscii( "Pattern" ) );
        xNewKey->setAsciiValue( ::rtl::OUString::createFromAscii( ".component:Text*" ) );

        xNewKey = xRegistryKey->createKey( ::rtl::OUString::createFromAscii( "/com.sun.star.uno.util.numbers.SvNumberFormatsSupplierServiceObject/UNO/SERVICES" ) );
        xNewKey->createKey( ::rtl::OUString::createFromAscii( "com.sun.star.util.NumberFormatsSupplier" ) );

        xNewKey = xRegistryKey->createKey( ::rtl::OUString::createFromAscii( "/com.sun.star.uno.util.numbers.SvNumberFormatterServiceObject/UNO/SERVICES" ) );
        xNewKey->createKey( ::rtl::OUString::createFromAscii( "com.sun.star.util.NumberFormatter" ) );

        xNewKey = xRegistryKey->createKey( ::rtl::OUString::createFromAscii( "/org.openoffice.comp.svt.OAddressBookSourceDialogUno/UNO/SERVICES" ) );
        xNewKey->createKey( ::rtl::OUString::createFromAscii( "com.sun.star.ui.AddressBookSourceDialog" ) );

        xNewKey = xRegistryKey->createKey( ::rtl::OUString::createFromAscii( "/com.sun.star.awt.ImageProducer/UNO/SERVICES" ) );
        xNewKey->createKey( ::rtl::OUString::createFromAscii( "com.sun.star.awt.ImageProducer" ) );

        xNewKey = xRegistryKey->createKey( ::rtl::OUString::createFromAscii( "/com.sun.star.svtools.FilePicker/UNO/SERVICES" ) );
        xNewKey->createKey( ::rtl::OUString::createFromAscii( "com.sun.star.ui.dialogs.FilePicker" ) );

        xNewKey = xRegistryKey->createKey( ::rtl::OUString::createFromAscii( "/com.sun.star.svtools.FolderPicker/UNO/SERVICES" ) );
        xNewKey->createKey( ::rtl::OUString::createFromAscii( "com.sun.star.ui.dialogs.FolderPicker" ) );

        xNewKey = xRegistryKey->createKey( ::rtl::OUString::createFromAscii( "/com.sun.star.svtools.OfficeFilePicker/UNO/SERVICES" ) );
        xNewKey->createKey( ::rtl::OUString::createFromAscii( "com.sun.star.ui.dialogs.OfficeFilePicker" ) );

        xNewKey = xRegistryKey->createKey( ::rtl::OUString::createFromAscii( "/com.sun.star.svtools.OfficeFolderPicker/UNO/SERVICES" ) );
        xNewKey->createKey( ::rtl::OUString::createFromAscii( "com.sun.star.ui.dialogs.OfficeFolderPicker" ) );

        xNewKey = xRegistryKey->createKey( ::rtl::OUString::createFromAscii( "/com.sun.star.svtools.SvFilterOptionsDialog/UNO/SERVICES" ) );
        xNewKey->createKey( ::rtl::OUString::createFromAscii( "com.sun.star.ui.dialogs.FilterOptionsDialog" ) );


        // the product registration component
        ::svt::RegisterImplementation(
            xRegistryKey,
            ::svt::OProductRegistration::getImplementationName_Static( ),
            ::svt::OProductRegistration::getSupportedServiceNames_Static( )
        );
    }

    return sal_True;
}

void* SAL_CALL component_getFactory( const sal_Char* sImplementationName, void* _pServiceManager, void* _pRegistryKey )
{
    void* pRet = NULL;

    if ( _pServiceManager )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceManager;
        xServiceManager = reinterpret_cast< ::com::sun::star::lang::XMultiServiceFactory* >( _pServiceManager );
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > xFactory;

        if ( rtl_str_compare( sImplementationName, "com.sun.star.awt.ImageProducer") == 0 )
        {
            ::com::sun::star::uno::Sequence< ::rtl::OUString > aServiceNames(1);
            aServiceNames.getArray()[0] = ::rtl::OUString::createFromAscii( "com.sun.star.awt.ImageProducer" );
            xFactory = ::cppu::createSingleFactory( xServiceManager, ::rtl::OUString::createFromAscii( sImplementationName ), ImageProducer_CreateInstance, aServiceNames );
        }
        else if ( rtl_str_compare( sImplementationName, "com.sun.star.uno.util.numbers.SvNumberFormatsSupplierServiceObject") == 0 )
        {
            ::com::sun::star::uno::Sequence< ::rtl::OUString > aServiceNames(1);
            aServiceNames.getArray()[0] = ::rtl::OUString::createFromAscii( "com.sun.star.util.NumberFormatsSupplier" );
            xFactory = ::cppu::createSingleFactory( xServiceManager, ::rtl::OUString::createFromAscii( sImplementationName ), SvNumberFormatsSupplierServiceObject_CreateInstance, aServiceNames );
        }
        else if ( rtl_str_compare( sImplementationName, "com.sun.star.uno.util.numbers.SvNumberFormatterServiceObject") == 0 )
        {
            ::com::sun::star::uno::Sequence< ::rtl::OUString > aServiceNames(1);
            aServiceNames.getArray()[0] = ::rtl::OUString::createFromAscii( "com.sun.star.util.NumberFormatter" );
            xFactory = ::cppu::createSingleFactory( xServiceManager, ::rtl::OUString::createFromAscii( sImplementationName ), SvNumberFormatterServiceObj_CreateInstance, aServiceNames );
        }
        else if ( rtl_str_compare( sImplementationName, "org.openoffice.comp.svt.OAddressBookSourceDialogUno") == 0 )
        {
            ::com::sun::star::uno::Sequence< ::rtl::OUString > aServiceNames(1);
            aServiceNames.getArray()[0] = ::rtl::OUString::createFromAscii( "com.sun.star.ui.AddressBookSourceDialog" );
            xFactory = ::cppu::createSingleFactory( xServiceManager, ::rtl::OUString::createFromAscii( sImplementationName ), svt::OAddressBookSourceDialogUno_CreateInstance, aServiceNames );
        }
        else if ( rtl_str_compare( sImplementationName, "com.sun.star.svtools.FilePicker") == 0 )
        {
            ::com::sun::star::uno::Sequence< ::rtl::OUString > aServiceNames(1);
            aServiceNames.getArray()[0] = ::rtl::OUString::createFromAscii( "com.sun.star.ui.dialogs.FilePicker" );
            xFactory = ::cppu::createSingleFactory( xServiceManager,
                                                    ::rtl::OUString::createFromAscii( sImplementationName ),
                                                    ::svt::SvtFilePicker_CreateInstance,
                                                    aServiceNames );
        }
        else if ( rtl_str_compare( sImplementationName, "com.sun.star.svtools.FolderPicker") == 0 )
        {
            ::com::sun::star::uno::Sequence< ::rtl::OUString > aServiceNames(1);
            aServiceNames.getArray()[0] = ::rtl::OUString::createFromAscii( "com.sun.star.ui.dialogs.FolderPicker" );
            xFactory = ::cppu::createSingleFactory( xServiceManager,
                                                    ::rtl::OUString::createFromAscii( sImplementationName ),
                                                    ::svt::SvtFolderPicker_CreateInstance,
                                                    aServiceNames );
        }
        else if ( rtl_str_compare( sImplementationName, "com.sun.star.svtools.OfficeFilePicker") == 0 )
        {
            ::com::sun::star::uno::Sequence< ::rtl::OUString > aServiceNames(1);
            aServiceNames.getArray()[0] = ::rtl::OUString::createFromAscii( "com.sun.star.ui.dialogs.OfficeFilePicker" );
            xFactory = ::cppu::createSingleFactory( xServiceManager,
                                                    ::rtl::OUString::createFromAscii( sImplementationName ),
                                                    ::svt::SvtOfficeFilePicker_CreateInstance,
                                                    aServiceNames );
        }
        else if ( rtl_str_compare( sImplementationName, "com.sun.star.svtools.OfficeFolderPicker") == 0 )
        {
            ::com::sun::star::uno::Sequence< ::rtl::OUString > aServiceNames(1);
            aServiceNames.getArray()[0] = ::rtl::OUString::createFromAscii( "com.sun.star.ui.dialogs.OfficeFolderPicker" );
            xFactory = ::cppu::createSingleFactory( xServiceManager,
                                                    ::rtl::OUString::createFromAscii( sImplementationName ),
                                                    ::svt::SvtOfficeFolderPicker_CreateInstance,
                                                    aServiceNames );
        }
        else if ( rtl_str_compare( sImplementationName, "com.sun.star.svtools.SvFilterOptionsDialog") == 0 )
        {
            ::com::sun::star::uno::Sequence< ::rtl::OUString > aServiceNames(1);
            aServiceNames.getArray()[0] = ::rtl::OUString::createFromAscii( "com.sun.star.ui.dialogs.FilterOptionsDialog" );
            xFactory = ::cppu::createSingleFactory( xServiceManager, ::rtl::OUString::createFromAscii( sImplementationName ), SvFilterOptionsDialog_CreateInstance, aServiceNames );
        }
        else
        {
            // a helper class which could ease the creation and make the following more readable and maintanable
            // (if it would be used for all services)
            ::svt::FactoryCreator aCreatorHelper( xServiceManager, ::rtl::OUString::createFromAscii( sImplementationName ) );

            aCreatorHelper.trySingleServiceFactory(
                ::svt::OProductRegistration::getImplementationName_Static( ),
                ::svt::OProductRegistration::getSupportedServiceNames_Static( ),
                ::svt::OProductRegistration::Create
            );

            xFactory = aCreatorHelper.getCreatedFactory();
        }

        if ( xFactory.is() )
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }
    return pRet;
}

}   // "C"

