/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fsfactory.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 14:56:33 $
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
#include "precompiled_svtools.hxx"

#include "fsfactory.hxx"

#ifndef _CPPUHELPER_FACTORY_HXX_
#include "cppuhelper/factory.hxx"
#endif

#ifndef _COM_SUN_STAR_UCB_XSIMPLEFILEACCESS_HPP_
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif


#include <ucbhelper/fileidentifierconverter.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/content.hxx>

#include <unotools/tempfile.hxx>
#include <unotools/ucbhelper.hxx>

#include "fsstorage.hxx"


using namespace ::com::sun::star;

//-------------------------------------------------------------------------
// Checks whether it is a file URL that represents a folder
sal_Bool isLocalNotFile_Impl( ::rtl::OUString aURL )
{
    sal_Bool bResult = sal_False;

    ::rtl::OUString aSystemPath;
    ::ucb::ContentBroker* pBroker = ::ucb::ContentBroker::get();
    if ( !pBroker )
        throw uno::RuntimeException();

    uno::Reference< ::com::sun::star::ucb::XContentProviderManager > xManager =
            pBroker->getContentProviderManagerInterface();
    try
    {
           aSystemPath = ::ucb::getSystemPathFromFileURL( xManager, aURL );
    }
    catch ( uno::Exception& )
    {
    }

    if ( aSystemPath.getLength() != 0 )
    {
        // it is a local file URL, check that it is not a file
        try
        {
            uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > xDummyEnv;
            ::ucb::Content aContent( aURL, xDummyEnv );
            bResult = aContent.isFolder();
        }
        catch( uno::Exception& )
        {
            bResult = sal_True;
        }
    }

    return bResult;
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL FSStorageFactory::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = ::rtl::OUString::createFromAscii("com.sun.star.embed.FileSystemStorageFactory");
    aRet[1] = ::rtl::OUString::createFromAscii("com.sun.star.comp.embed.FileSystemStorageFactory");
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL FSStorageFactory::impl_staticGetImplementationName()
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.embed.FileSystemStorageFactory");
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL FSStorageFactory::impl_staticCreateSelfInstance(
            const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return uno::Reference< uno::XInterface >( *new FSStorageFactory( xServiceManager ) );
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL FSStorageFactory::createInstance()
    throw ( uno::Exception,
            uno::RuntimeException )
{
    ::rtl::OUString aTempURL;

    aTempURL = ::utl::TempFile( NULL, sal_True ).GetURL();

    if ( !aTempURL.getLength() )
        throw uno::RuntimeException(); // TODO: can not create tempfile

    ::ucb::Content aResultContent( aTempURL, uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );

    return uno::Reference< uno::XInterface >(
                static_cast< OWeakObject* >( new FSStorage( aResultContent,
                                                            embed::ElementModes::READWRITE,
                                                            uno::Sequence< beans::PropertyValue >(),
                                                            m_xFactory ) ),
                uno::UNO_QUERY );
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL FSStorageFactory::createInstanceWithArguments(
            const uno::Sequence< uno::Any >& aArguments )
    throw ( uno::Exception,
            uno::RuntimeException )
{
    // The request for storage can be done with up to three arguments

    // The first argument specifies a source for the storage
    // it must be URL.
    // The second value is a mode the storage should be open in.
    // And the third value is a media descriptor.

    sal_Int32 nArgNum = aArguments.getLength();
    OSL_ENSURE( nArgNum < 4, "Wrong parameter number" );

    if ( !nArgNum )
        return createInstance();

    // first try to retrieve storage open mode if any
    // by default the storage will be open in readonly mode
    sal_Int32 nStorageMode = embed::ElementModes::READ;
    if ( nArgNum >= 2 )
    {
        if( !( aArguments[1] >>= nStorageMode ) )
        {
            OSL_ENSURE( sal_False, "Wrong second argument!\n" );
            throw uno::Exception(); // TODO: Illegal argument
        }
        // it's allways possible to read written storage in this implementation
        nStorageMode |= embed::ElementModes::READ;
    }

    // retrieve storage source URL
    ::rtl::OUString aURL;

    if ( aArguments[0] >>= aURL )
    {
        if ( !aURL.getLength() )
        {
            OSL_ENSURE( sal_False, "Empty URL is provided!\n" );
            throw uno::Exception(); // TODO: illegal argument
        }
    }
    else
    {
        OSL_ENSURE( sal_False, "Wrong first argument!\n" );
        throw uno::Exception(); // TODO: Illegal argument
    }

    // retrieve mediadescriptor and set storage properties
    uno::Sequence< beans::PropertyValue > aDescr;
    uno::Sequence< beans::PropertyValue > aPropsToSet;

    if ( nArgNum >= 3 )
    {
        if( aArguments[2] >>= aDescr )
        {
            aPropsToSet.realloc(1);
            aPropsToSet[0].Name = ::rtl::OUString::createFromAscii( "URL" );
            aPropsToSet[0].Value <<= aURL;

            for ( sal_Int32 nInd = 0, nNumArgs = 1; nInd < aDescr.getLength(); nInd++ )
            {
                if ( aDescr[nInd].Name.equalsAscii( "InteractionHandler" ) )
                {
                    aPropsToSet.realloc( ++nNumArgs );
                    aPropsToSet[nNumArgs-1].Name = aDescr[nInd].Name;
                    aPropsToSet[nNumArgs-1].Value = aDescr[nInd].Value;
                    break;
                }
                else
                    OSL_ENSURE( sal_False, "Unacceptable property, will be ignored!\n" );
            }
        }
        else
        {
            OSL_ENSURE( sal_False, "Wrong third argument!\n" );
            throw uno::Exception(); // TODO: Illegal argument
        }
    }

    // allow to use other ucp's
    // if ( !isLocalNotFile_Impl( aURL ) )
    if ( aURL.equalsIgnoreAsciiCaseAsciiL( "vnd.sun.star.pkg", 16 )
      || aURL.equalsIgnoreAsciiCaseAsciiL( "vnd.sun.star.zip", 16 )
      || ::utl::UCBContentHelper::IsDocument( aURL ) )
    {
        OSL_ENSURE( sal_False, "File system storages can be based only on file URLs!\n" ); // ???
        throw uno::Exception(); // TODO: illegal argument
    }

    if ( ( nStorageMode & embed::ElementModes::WRITE ) && !( nStorageMode & embed::ElementModes::NOCREATE ) )
        FSStorage::MakeFolderNoUI( aURL, sal_False );
    else if ( !::utl::UCBContentHelper::IsFolder( aURL ) )
        throw io::IOException(); // there is no such folder

    ::ucb::Content aResultContent( aURL, uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );

    // create storage based on source
    return uno::Reference< uno::XInterface >(
                static_cast< OWeakObject* >( new FSStorage( aResultContent, nStorageMode, aPropsToSet, m_xFactory ) ),
                uno::UNO_QUERY );
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL FSStorageFactory::getImplementationName()
    throw ( uno::RuntimeException )
{
    return impl_staticGetImplementationName();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL FSStorageFactory::supportsService( const ::rtl::OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq = impl_staticGetSupportedServiceNames();

    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( ServiceName.compareTo( aSeq[nInd] ) == 0 )
            return sal_True;

    return sal_False;
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL FSStorageFactory::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return impl_staticGetSupportedServiceNames();
}

//-------------------------------------------------------------------------

extern "C"
{
SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment (
    const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo (
    void * /* pServiceManager */, void * pRegistryKey)
{
    if (pRegistryKey)
    {
        uno::Reference< registry::XRegistryKey > xRegistryKey (
            reinterpret_cast< registry::XRegistryKey*>(pRegistryKey));

        uno::Reference< registry::XRegistryKey > xNewKey;
        xNewKey = xRegistryKey->createKey(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
            FSStorageFactory::impl_staticGetImplementationName() +
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES")));

        const uno::Sequence< ::rtl::OUString > aServices (
            FSStorageFactory::impl_staticGetSupportedServiceNames());
        for( sal_Int32 i = 0; i < aServices.getLength(); i++ )
            xNewKey->createKey( aServices.getConstArray()[i] );

        return sal_True;
    }
    return sal_False;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory (
    const sal_Char * pImplementationName, void * pServiceManager, void * /* pRegistryKey */)
{
    void * pResult = 0;
    if (pServiceManager)
    {
        uno::Reference< lang::XSingleServiceFactory > xFactory;
        if (FSStorageFactory::impl_staticGetImplementationName().compareToAscii (pImplementationName) == 0)
        {
            xFactory = cppu::createOneInstanceFactory (
                reinterpret_cast< lang::XMultiServiceFactory* >(pServiceManager),
                FSStorageFactory::impl_staticGetImplementationName(),
                FSStorageFactory::impl_staticCreateSelfInstance,
                FSStorageFactory::impl_staticGetSupportedServiceNames() );
        }
        if (xFactory.is())
        {
            xFactory->acquire();
            pResult = xFactory.get();
        }
    }
    return pResult;
}

} // extern "C"

