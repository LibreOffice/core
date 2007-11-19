/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_informationprovider.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-19 16:53:40 $
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
#include "precompiled_desktop.hxx"

#include <cppuhelper/implbase3.hxx>

#include "comphelper/servicedecl.hxx"

#include "com/sun/star/deployment/thePackageManagerFactory.hpp"
#include "com/sun/star/deployment/UpdateInformationProvider.hpp"
#include "com/sun/star/deployment/XPackage.hpp"
#include "com/sun/star/deployment/XPackageInformationProvider.hpp"
#include "com/sun/star/deployment/XPackageManager.hpp"
#include "com/sun/star/deployment/XUpdateInformationProvider.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"
#include "com/sun/star/task/XAbortChannel.hpp"
#include "com/sun/star/ucb/CommandFailedException.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/xml/dom/XElement.hpp"
#include "com/sun/star/xml/dom/XNode.hpp"

#include "com/sun/star/uno/Reference.hxx"
#include "rtl/ustring.hxx"
#include "ucbhelper/content.hxx"

#include "dp_descriptioninfoset.hxx"
#include "dp_identifier.hxx"
#include "dp_version.hxx"
#include "dp_misc.h"

namespace beans      = com::sun::star::beans ;
namespace deployment = com::sun::star::deployment ;
namespace lang       = com::sun::star::lang ;
namespace registry   = com::sun::star::registry ;
namespace task       = com::sun::star::task ;
namespace css_ucb    = com::sun::star::ucb ;
namespace uno        = com::sun::star::uno ;
namespace xml = com::sun::star::xml ;

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

namespace dp_info {

class PackageInformationProvider :
    public ::cppu::WeakImplHelper3< deployment::XPackageInformationProvider,
                                    css_ucb::XCommandEnvironment,
                                    task::XInteractionHandler >
{
    public:
                 PackageInformationProvider( uno::Reference< uno::XComponentContext >const& xContext);
    virtual     ~PackageInformationProvider();

    static uno::Sequence< rtl::OUString > getServiceNames();
    static rtl::OUString getImplName();

    // XInteractionHandler
    virtual void SAL_CALL handle( const uno::Reference< task::XInteractionRequest >& Request )
                                throw( uno::RuntimeException );
    // XCommandEnvironment
    virtual uno::Reference< task::XInteractionHandler > SAL_CALL getInteractionHandler()
        throw ( uno::RuntimeException ) { return static_cast<task::XInteractionHandler*>(this); };

    virtual uno::Reference< css_ucb::XProgressHandler > SAL_CALL getProgressHandler()
        throw ( uno::RuntimeException ) { return uno::Reference< css_ucb::XProgressHandler >(); };

    // XPackageInformationProvider
    virtual rtl::OUString SAL_CALL getPackageLocation( const rtl::OUString& extensionId )
        throw ( uno::RuntimeException );
    virtual uno::Sequence< uno::Sequence< rtl::OUString > > SAL_CALL isUpdateAvailable( const rtl::OUString& extensionId )
        throw ( uno::RuntimeException );
    virtual uno::Sequence< uno::Sequence< rtl::OUString > > SAL_CALL getExtensionList()
        throw ( uno::RuntimeException );
//---------
private:

    uno::Reference< uno::XComponentContext> mxContext;

    rtl::OUString getPackageLocation( const uno::Reference< deployment::XPackageManager > _xManager,
                                      const rtl::OUString& _sExtensionId );

    uno::Reference< deployment::XUpdateInformationProvider > mxUpdateInformation;

    uno::Sequence< uno::Reference< xml::dom::XElement > >
            getUpdateInformation( uno::Sequence< rtl::OUString > const & urls,
                                  rtl::OUString const & identifier ) const;
    uno::Sequence< uno::Reference< deployment::XPackage > >
            getPackages( const uno::Reference< deployment::XPackageManager > _xManager );
    uno::Sequence< uno::Sequence< rtl::OUString > > isUpdateAvailable( const uno::Reference< deployment::XPackageManager > _xManager,
                            const rtl::OUString& _sExtensionId );
    uno::Sequence< uno::Sequence< rtl::OUString > > getExtensionList( const uno::Reference< deployment::XPackageManager > _xManager );
    uno::Sequence< uno::Sequence< rtl::OUString > > concatLists( uno::Sequence< uno::Sequence< rtl::OUString > > aFirst,
                                                                 uno::Sequence< uno::Sequence< rtl::OUString > > aSecond );
};

//------------------------------------------------------------------------------

PackageInformationProvider::PackageInformationProvider( uno::Reference< uno::XComponentContext > const& xContext) :
    mxContext( xContext ),
    mxUpdateInformation( deployment::UpdateInformationProvider::create( xContext ) )
{
}

//------------------------------------------------------------------------------

PackageInformationProvider::~PackageInformationProvider()
{
}

//------------------------------------------------------------------------------
void SAL_CALL PackageInformationProvider::handle( uno::Reference< task::XInteractionRequest > const & rRequest)
    throw (uno::RuntimeException)
{
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > xContinuations = rRequest->getContinuations();
    if ( xContinuations.getLength() == 1 )
    {
        xContinuations[0]->select();
    }
}

//------------------------------------------------------------------------------
rtl::OUString PackageInformationProvider::getPackageLocation(
        const uno::Reference< deployment::XPackageManager > _xManager,
        const rtl::OUString& _rExtensionId )
{
    rtl::OUString aLocationURL;

    if ( _xManager.is() )
    {
        const uno::Sequence< uno::Reference< deployment::XPackage > > packages(
                _xManager->getDeployedPackages(
                    uno::Reference< task::XAbortChannel >(),
                    static_cast < XCommandEnvironment *> (this) ) );

        for ( int pos = packages.getLength(); pos--; )
        {
            try
            {
                const rtl::OUString aName = packages[ pos ]->getName();
                const beans::Optional< rtl::OUString > aID = packages[ pos ]->getIdentifier();
                if ( aID.IsPresent && aID.Value.compareTo( _rExtensionId ) == 0 )
                {
                    aLocationURL = packages[ pos ]->getURL();
                    break;
                }
            }
            catch ( uno::RuntimeException & ) {}
        }
    }

    return aLocationURL;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL
PackageInformationProvider::getPackageLocation( const rtl::OUString& _sExtensionId )
    throw ( uno::RuntimeException )
{
    uno::Reference< deployment::XPackageManager > xManager;
    try {
        xManager = deployment::thePackageManagerFactory::get( mxContext )->getPackageManager( UNISTRING("user") );
    }
    catch ( css_ucb::CommandFailedException & ){}
    catch ( uno::RuntimeException & ) {}

    rtl::OUString aLocationURL = getPackageLocation( xManager, _sExtensionId );

    if ( aLocationURL.getLength() == 0 )
    {
        try {
            xManager = deployment::thePackageManagerFactory::get( mxContext )->getPackageManager( UNISTRING("shared") );
        }
        catch ( css_ucb::CommandFailedException & ){}
        catch ( uno::RuntimeException & ) {}

        aLocationURL = getPackageLocation( xManager, _sExtensionId );
    }

    if ( aLocationURL.getLength() )
    {
        ::ucbhelper::Content aContent( aLocationURL, NULL );
        aLocationURL = aContent.getURL();
    }

    return aLocationURL;
}

//------------------------------------------------------------------------------

uno::Sequence< uno::Sequence< rtl::OUString > > SAL_CALL
PackageInformationProvider::isUpdateAvailable( const rtl::OUString& _sExtensionId )
    throw ( uno::RuntimeException )
{
    uno::Sequence< uno::Sequence< rtl::OUString > > aUpdateListUser;

    uno::Reference< deployment::XPackageManager > xManager;
    try {
        xManager = deployment::thePackageManagerFactory::get( mxContext )->getPackageManager( UNISTRING("user") );
    }
    catch ( css_ucb::CommandFailedException & ){}
    catch ( uno::RuntimeException & ) {}

    aUpdateListUser = isUpdateAvailable( xManager, _sExtensionId );

    uno::Sequence< uno::Sequence< rtl::OUString > > aUpdateListShared;
    try {
        xManager = deployment::thePackageManagerFactory::get( mxContext )->getPackageManager( UNISTRING("shared") );
    }
    catch ( css_ucb::CommandFailedException & ){}
    catch ( uno::RuntimeException & ) {}

    aUpdateListShared = isUpdateAvailable( xManager, _sExtensionId );

    if ( !aUpdateListUser.hasElements() )
        return aUpdateListShared;
    else if ( !aUpdateListShared.hasElements() )
        return aUpdateListUser;
    else
        return concatLists( aUpdateListUser, aUpdateListShared );
}

//------------------------------------------------------------------------------
uno::Sequence< uno::Sequence< rtl::OUString > > SAL_CALL PackageInformationProvider::getExtensionList()
    throw ( uno::RuntimeException )
{
    uno::Sequence< uno::Sequence< rtl::OUString > > aListUser;

    uno::Reference< deployment::XPackageManager > xManager;
    try {
        xManager = deployment::thePackageManagerFactory::get( mxContext )->getPackageManager( UNISTRING("user") );
    }
    catch ( css_ucb::CommandFailedException & ){}
    catch ( uno::RuntimeException & ) {}

    aListUser = getExtensionList( xManager );

    uno::Sequence< uno::Sequence< rtl::OUString > > aListShared;
    try {
        xManager = deployment::thePackageManagerFactory::get( mxContext )->getPackageManager( UNISTRING("shared") );
    }
    catch ( css_ucb::CommandFailedException & ){}
    catch ( uno::RuntimeException & ) {}

    aListShared = getExtensionList( xManager );

    if ( !aListUser.hasElements() )
        return aListShared;
    else if ( !aListShared.hasElements() )
        return aListUser;
    else
        return concatLists( aListUser, aListShared );
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
uno::Sequence< uno::Reference< deployment::XPackage > >
    PackageInformationProvider::getPackages( const uno::Reference< deployment::XPackageManager > _xMgr )
{
    uno::Sequence< uno::Reference< deployment::XPackage > > packages;
    try {
        packages = _xMgr->getDeployedPackages( uno::Reference< task::XAbortChannel >(),
                                               static_cast < XCommandEnvironment *> (this) );
    }
    catch ( deployment::DeploymentException & )
    {}
    catch ( css_ucb::CommandFailedException & )
    {}
    catch ( css_ucb::CommandAbortedException & )
    {}
    catch ( lang::IllegalArgumentException & e )
    {
        throw uno::RuntimeException(e.Message, e.Context);
    }

    return packages;
}

//------------------------------------------------------------------------------
uno::Sequence< uno::Reference< xml::dom::XElement > >
    PackageInformationProvider::getUpdateInformation( uno::Sequence< rtl::OUString > const & urls,
                                                      rtl::OUString const & identifier ) const
{
    try
    {
        return mxUpdateInformation->getUpdateInformation( urls, identifier );
    }
    catch ( uno::RuntimeException & ) {
        throw;
    }
    catch ( css_ucb::CommandFailedException & ) {}
    catch ( css_ucb::CommandAbortedException & ) {}
    catch ( uno::Exception & ) {}

    return uno::Sequence< uno::Reference< xml::dom::XElement > >();
}

//------------------------------------------------------------------------------
uno::Sequence< uno::Sequence< rtl::OUString > >
    PackageInformationProvider::isUpdateAvailable(
                    const uno::Reference< deployment::XPackageManager > _xManager,
                    const rtl::OUString& _sExtensionId )
{
    uno::Sequence< uno::Sequence< rtl::OUString > > aList;
    sal_Int32 nCount = 0;
    bool bPackageFound = false;

    // If the package manager is readonly then the user cannot modify anything anyway
    // so we can abort the search here
    if ( _xManager.is() && ! _xManager->isReadOnly() )
    {
        uno::Sequence< uno::Reference< deployment::XPackage > > packages( getPackages( _xManager ) );
        uno::Sequence< uno::Reference< xml::dom::XElement > > defaultInfos;

        for ( int pos = packages.getLength(); pos-- && !bPackageFound; )
        {
            uno::Reference< deployment::XPackage > package( packages[ pos ] );
            uno::Sequence< rtl::OUString > urls( package->getUpdateInformationURLs());
            uno::Sequence< uno::Reference< xml::dom::XElement > > infos;
            rtl::OUString id( dp_misc::getIdentifier( package ) );

            if ( _sExtensionId.getLength() )
            {
                if ( _sExtensionId == id )
                    bPackageFound = true;
                else /* we have an ID and the IDs don't match, continue with next package */
                    continue;
            }

            if ( urls.getLength() != 0)
            {
                infos = getUpdateInformation( urls, id );
            }
            else
            {
                if ( defaultInfos.getLength() == 0 )
                {
                    const rtl::OUString defaultURL( dp_misc::getExtensionDefaultUpdateURL() );
                    if ( defaultURL.getLength() )
                        defaultInfos = getUpdateInformation( uno::Sequence< rtl::OUString >( &defaultURL, 1 ),
                                                             rtl::OUString() );
                }
                infos = defaultInfos;
            }
            rtl::OUString latestVersion( package->getVersion() );
            sal_Int32 latestIndex = -1;
            for ( sal_Int32 i = 0; i < infos.getLength(); ++i )
            {
                dp_misc::DescriptionInfoset infoset( mxContext,
                                    uno::Reference< xml::dom::XNode >( infos[i], uno::UNO_QUERY_THROW));
                boost::optional< rtl::OUString > id2( infoset.getIdentifier() );

                if (!id2)
                    continue;

                if (*id2 == id)
                {
                    rtl::OUString v( infoset.getVersion() );
                    if ( dp_misc::compareVersions( v, latestVersion ) == dp_misc::GREATER )
                    {
                        latestVersion = v;
                        latestIndex = i;
                    }
                }
            }
            if ( latestIndex != -1 )
            {
                rtl::OUString aNewEntry[2];
                aNewEntry[0] = id;
                aNewEntry[1] = latestVersion;
                aList.realloc( ++nCount );
                aList[ nCount-1 ] = ::uno::Sequence< rtl::OUString >( aNewEntry, 2 );
            }
        }
    }
    return aList;
}

//------------------------------------------------------------------------------
uno::Sequence< uno::Sequence< rtl::OUString > >
    PackageInformationProvider::getExtensionList(
                    const uno::Reference< deployment::XPackageManager > _xManager )
{
    uno::Sequence< uno::Sequence< rtl::OUString > > aList;

    if ( _xManager.is() )
    {
        uno::Sequence< uno::Reference< deployment::XPackage > > packages( getPackages( _xManager ) );

        aList.realloc( packages.getLength() );

        for ( int pos = packages.getLength(); pos--; )
        {
            uno::Reference< deployment::XPackage > package( packages[ pos ] );
            rtl::OUString aNewEntry[2];

            aNewEntry[0] = dp_misc::getIdentifier( package );
            aNewEntry[1] = package->getVersion();
            aList[ pos ] = ::uno::Sequence< rtl::OUString >( aNewEntry, 2 );
        }
    }
    return aList;
}

//------------------------------------------------------------------------------
uno::Sequence< uno::Sequence< rtl::OUString > > PackageInformationProvider::concatLists(
            uno::Sequence< uno::Sequence< rtl::OUString > > aFirst,
            uno::Sequence< uno::Sequence< rtl::OUString > > aSecond )
{
    sal_Int32 nFirstCount = aFirst.getLength();
    sal_Int32 nSecondCount = aSecond.getLength();
    sal_Int32 nIndex = nFirstCount;

    for ( sal_Int32 i=0; i < nSecondCount; i++ )
    {
        bool bDuplicateEntry = false;
        for ( sal_Int32 j=0; j < nFirstCount; j++ )
        {
            if ( aFirst[ j ][0] == aSecond[ i ][0] )
            {
                bDuplicateEntry = true;
                break;
            }
        }
        if ( !bDuplicateEntry )
        {
            nIndex += 1;
            aFirst.realloc( nIndex );
            aFirst[ nIndex - 1 ] = aSecond[ i ];
        }
    }
    return aFirst;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

namespace sdecl = comphelper::service_decl;
sdecl::class_<PackageInformationProvider> servicePIP;
extern sdecl::ServiceDecl const serviceDecl(
    servicePIP,
    // a private one:
    "com.sun.star.comp.deployment.PackageInformationProvider",
    "com.sun.star.comp.deployment.PackageInformationProvider" );

//------------------------------------------------------------------------------
bool singleton_entries(
    uno::Reference< registry::XRegistryKey > const & xRegistryKey )
{
    try {
        uno::Reference< registry::XRegistryKey > xKey(
            xRegistryKey->createKey(
                serviceDecl.getImplementationName() +
                // xxx todo: use future generated function to get singleton name
                UNISTRING("/UNO/SINGLETONS/"
                      "com.sun.star.deployment.PackageInformationProvider") ) );
        xKey->setStringValue( serviceDecl.getSupportedServiceNames()[0] );
        return true;
    }
    catch (registry::InvalidRegistryException & exc) {
        (void) exc; // avoid warnings
        OSL_ENSURE( 0, ::rtl::OUStringToOString(
                        exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
        return false;
    }
}

} // namespace dp_info


