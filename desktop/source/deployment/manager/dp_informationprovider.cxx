/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_informationprovider.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-06-12 06:19:11 $
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
#include "com/sun/star/deployment/XPackage.hpp"
#include "com/sun/star/deployment/XPackageInformationProvider.hpp"
#include "com/sun/star/deployment/XPackageManager.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"
#include "com/sun/star/task/XAbortChannel.hpp"
#include "com/sun/star/ucb/CommandFailedException.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"

#include "com/sun/star/uno/Reference.hxx"
#include "rtl/ustring.hxx"
#include "ucbhelper/content.hxx"

namespace beans      = com::sun::star::beans ;
namespace deployment = com::sun::star::deployment ;
namespace lang       = com::sun::star::lang ;
namespace registry   = com::sun::star::registry ;
namespace task       = com::sun::star::task ;
namespace css_ucb    = com::sun::star::ucb ;
namespace uno        = com::sun::star::uno ;

/*
namespace container = com::sun::star::container ;
namespace io = com::sun::star::io ;
namespace xml = com::sun::star::xml ;
namespace sdbc = com::sun::star::sdbc ;
*/

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

namespace dp_info {

class PackageInformationProvider :
    public ::cppu::WeakImplHelper3< deployment::XPackageInformationProvider,
                                    css_ucb::XCommandEnvironment,
                                    task::XInteractionHandler >
{
    uno::Reference< uno::XComponentContext> mxContext;

    rtl::OUString getPackageLocation( const uno::Reference< deployment::XPackageManager > _xManager,
                                      const rtl::OUString& _sExtensionId );

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
};

//------------------------------------------------------------------------------

PackageInformationProvider::PackageInformationProvider( uno::Reference< uno::XComponentContext > const& xContext) :
    mxContext(xContext)
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
        ::ucb::Content aContent( aLocationURL, NULL );
        aLocationURL = aContent.getURL();
    }

    return aLocationURL;
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


