/*************************************************************************
 *
 *  $RCSfile: dp_script.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-04-13 12:09:59 $
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

#include "dp_lib_container.h"
#include "dp_backend.h"
#include "dp_ucb.h"
#include "rtl/uri.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "ucbhelper/content.hxx"
#include "comphelper/anytostring.hxx"
#include <memory>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace dp_registry
{
namespace backend
{
namespace script
{

//==============================================================================
class BackendImpl : public ::dp_registry::backend::PackageRegistryBackend
{
protected:
    // PackageRegistryBackend
    virtual Reference< deployment::XPackage > createPackage(
        OUString const & url,
        OUString const & mediaType, OUString const & subType,
        INetContentTypeParameterList const & params,
        Reference< XCommandEnvironment > const & xCmdEnv );

public:
    // currently only for library containers:
    enum { LAYER_NONE, LAYER_USER, LAYER_SHARED } m_layer;

    ::std::auto_ptr< LibraryContainer > m_basic_script_libs;
    ::std::auto_ptr< LibraryContainer > m_dialog_libs;

    inline BackendImpl(
        Reference< XComponentContext > const & xComponentContext,
        OUString const & implName,
        Sequence< OUString > const & supported_media_types )
        : PackageRegistryBackend(
            xComponentContext, implName, supported_media_types ),
          m_layer( LAYER_NONE )
        {}

    // XInitialization
    virtual void SAL_CALL initialize( Sequence< Any > const & args )
        throw (RuntimeException);

    // XPackageRegistry
    virtual OUString SAL_CALL detectMediaType( OUString const & url )
        throw (RuntimeException);
};

//==============================================================================
class PackageImpl : public ::dp_registry::backend::Package
{
    bool m_scriptLib;
    OUString m_libURL;
    OUString const & getLibName(
        Reference< XCommandEnvironment > const & xCmdEnv );

    inline BackendImpl * getMyBackend() const
        { return static_cast< BackendImpl * >(m_myBackend.get()); }

    inline LibraryContainer * getContainer() const
    {
        return m_scriptLib ? getMyBackend()->m_basic_script_libs.get()
                           : getMyBackend()->m_dialog_libs.get();
    }

    // OPropertySetHelper
    virtual void SAL_CALL getFastPropertyValue(
        Any & rValue, sal_Int32 nHandle ) const;

    // Package
    virtual t_Registered getRegStatus(
        ::osl::ResettableMutexGuard & guard,
        Reference< XCommandEnvironment > const & xCmdEnv );
    virtual bool processPackage(
        bool register_package,
        ::osl::ResettableMutexGuard & guard,
        Reference< XCommandEnvironment > const & xCmdEnv );

public:
    PackageImpl(
        ::rtl::Reference< PackageRegistryBackend > const & myBackend,
        OUString const & url,
        OUString const & mediaType,
        bool scriptLib )
        : Package( myBackend, url, mediaType,
                   OUString() /* no name yet */,
                   OUString() /* no display-name yet */,
                   scriptLib
                   ? OUSTR("Basic Script Library")
                   : OUSTR("Dialog Library") ),
          m_scriptLib( scriptLib ),
          m_libURL( make_url( url, scriptLib ? OUSTR("script.xlb")
                                             : OUSTR("dialog.xlb") ) )
        {}
};

//______________________________________________________________________________
void BackendImpl::initialize( Sequence< Any > const & args )
    throw (RuntimeException)
{
    PackageRegistryBackend::initialize( args );

    if (args.getLength() > 2) // optional context
    {
        OUString context( extract_throw< OUString >( args[ 2 ] ) );
        if (context.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("user") ))
            m_layer = LAYER_USER;
        else if (context.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("shared") ))
            m_layer = LAYER_SHARED;
    }
    if (m_layer != LAYER_NONE)
    {
        OUString basic_path(
            m_layer == LAYER_USER
            ? OUSTR("vnd.sun.star.expand:${$SYSBINDIR/"
                    SAL_CONFIGFILE("bootstrap")
                    ":UserInstallation}/user/basic")
            : OUSTR("vnd.sun.star.expand:${$SYSBINDIR/"
                    SAL_CONFIGFILE("bootstrap")
                    ":BaseInstallation}/share/basic") );
        m_basic_script_libs.reset(
            new LibraryContainer(
                make_url( basic_path, OUSTR("/script.xlc") ),
                getMutex(),
                getComponentContext() ) );
        m_dialog_libs.reset(
            new LibraryContainer(
                make_url( basic_path, OUSTR("/dialog.xlc") ),
                getMutex(),
                getComponentContext() ) );
    }
}

//==============================================================================
OUString SAL_CALL getImplementationName()
{
    return OUSTR("com.sun.star.comp.deployment.script.PackageRegistryBackend");
}

//==============================================================================
Reference< XInterface > SAL_CALL create(
    Reference< XComponentContext > const & xComponentContext )
    SAL_THROW( (Exception) )
{
    OUString const media_types [] = {
        OUSTR("application/vnd.sun.star.basic-script"),
        OUSTR("application/vnd.sun.star.dialog-library")
    };
    return static_cast< ::cppu::OWeakObject * >(
        new BackendImpl(
            xComponentContext, getImplementationName(),
            Sequence< OUString >( media_types, ARLEN(media_types) ) ) );
}

// XPackageRegistry
//______________________________________________________________________________
OUString BackendImpl::detectMediaType( OUString const & url )
    throw (RuntimeException)
{
    ::ucb::Content ucb_content;
    if (::ucb::Content::create( url, Reference< XCommandEnvironment >(),
                                ucb_content ) &&
        ucb_content.isFolder())
    {
        // probe for script.xlb:
        if (create_ucb_content(
                0, make_url( ucb_content.getURL(), OUSTR("script.xlb") ),
                Reference< XCommandEnvironment >(), false /* no throw */ ))
            return OUSTR("application/vnd.sun.star.basic-script");
        // probe for dialog.xlb:
        if (create_ucb_content(
                0, make_url( ucb_content.getURL(), OUSTR("dialog.xlb") ),
                Reference< XCommandEnvironment >(), false /* no throw */ ))
            return OUSTR("application/vnd.sun.star.dialog-library");
    }
    return OUString();
}

// PackageRegistryBackend
//______________________________________________________________________________
Reference< deployment::XPackage > BackendImpl::createPackage(
    OUString const & url,
    OUString const & mediaType, OUString const & subType,
    INetContentTypeParameterList const & params,
    Reference< XCommandEnvironment > const & xCmdEnv )
{
    if (subType.matchIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.basic-script") ))
        return new PackageImpl( this, url, mediaType, true );
    if (subType.matchIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.dialog-library") ))
        return new PackageImpl( this, url, mediaType, false );
    throw lang::IllegalArgumentException(
        OUSTR("invalid media-type given: ") + mediaType,
        static_cast< OWeakObject * >(this),
        static_cast< sal_Int16 >(-1 /* not known */) );
}

//##############################################################################

//______________________________________________________________________________
OUString const & PackageImpl::getLibName(
    Reference< XCommandEnvironment > const & xCmdEnv )
{
    ::osl::MutexGuard guard( getMutex() );
    if (m_name.getLength() == 0)
    {
        m_name = LibraryContainer::get_libname(
            m_libURL, xCmdEnv, getMyBackend()->getComponentContext() );
    }
    return m_name;
}

// OPropertySetHelper
//______________________________________________________________________________
void PackageImpl::getFastPropertyValue( Any & rValue, sal_Int32 nHandle ) const
{
    // xxx todo: upon disposing() getPropertyValue( URL )...
//     check();
    switch (nHandle)
    {
    case NAME:
    case DISPLAY_NAME:
    {
        try
        {
            rValue <<= const_cast< PackageImpl * >(this)->getLibName( 0 );
        }
        catch (RuntimeException &)
        {
            throw;
        }
        catch (Exception &)
        {
            Any exc( ::cppu::getCaughtException() );
            throw lang::WrappedTargetException(
                OUSTR("caught exception reading Basic library name!"),
                static_cast< OWeakObject * >(
                    const_cast< PackageImpl * >(this) ), exc );
        }
        break;
    }
    default:
        Package::getFastPropertyValue( rValue, nHandle );
        break;
    }
}

// Package
//______________________________________________________________________________
PackageImpl::t_Registered PackageImpl::getRegStatus(
    ::osl::ResettableMutexGuard & guard,
    Reference< XCommandEnvironment > const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    if (that->m_layer == BackendImpl::LAYER_NONE)
        return REG_VOID;
    if (getContainer()->has( getLibName( xCmdEnv ), xCmdEnv ))
        return REG_REGISTERED;
    else
        return REG_NOT_REGISTERED;
}

//______________________________________________________________________________
bool PackageImpl::processPackage(
    bool register_package,
    ::osl::ResettableMutexGuard & guard,
    Reference< XCommandEnvironment > const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    // xxx todo:
    OSL_ASSERT( that->m_layer != BackendImpl::LAYER_NONE );
    that->ensure_no_running_office();

    LibraryContainer * container = getContainer();
    bool success;
    if (register_package)
        success = container->insert( getLibName( xCmdEnv ), m_libURL, xCmdEnv );
    else // revokePackage()
        success = container->remove( getLibName( xCmdEnv ), m_libURL, xCmdEnv );
    success &= container->flush( xCmdEnv );
    return success;
}

} // namespace script
} // namespace backend
} // namespace dp_registry

