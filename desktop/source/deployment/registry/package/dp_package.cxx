/*************************************************************************
 *
 *  $RCSfile: dp_package.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-04-13 12:09:21 $
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

#include "dp_backend.h"
#include "dp_ucb.h"
#include "dp_platform.h"
#include "dp_persmap.h"
#include "rtl/uri.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "ucbhelper/content.hxx"
#include "comphelper/anytostring.hxx"
#include "com/sun/star/ucb/XContentAccess.hpp"
#include "com/sun/star/sdbc/XResultSet.hpp"
#include "com/sun/star/sdbc/XRow.hpp"
#include <vector>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace dp_registry
{
namespace backend
{
namespace package
{

//==============================================================================
class BackendImpl : public PackageRegistryBackend
{
protected:
    virtual void SAL_CALL disposing();

    // PackageRegistryBackend
    virtual Reference< deployment::XPackage > createPackage(
        OUString const & url,
        OUString const & mediaType, OUString const & subType,
        INetContentTypeParameterList const & params,
        Reference< XCommandEnvironment > const & xCmdEnv );

public:
    ::std::auto_ptr< ::dp_misc::PersistentMap > m_registeredPackages;

    inline BackendImpl(
        Reference< XComponentContext > const & xComponentContext,
        OUString const & implName,
        Sequence< OUString > const & supported_media_types )
        : PackageRegistryBackend(
            xComponentContext, implName, supported_media_types )
        {}

    // XInitialization
    void SAL_CALL initialize( Sequence< Any > const & args )
        throw (Exception);

    // XPackageRegistry
    virtual OUString SAL_CALL detectMediaType( OUString const & url )
        throw (RuntimeException);
};

//==============================================================================
class PackageImpl : public ::dp_registry::backend::Package
{
    OUString m_name;
    OUString m_scan_url;
    bool m_legacy_package;
    t_Registered m_registered;

    Sequence< Reference< deployment::XPackage > > m_composition;
    bool m_composition_init_success;
    bool m_composition_init;
    bool bindComposition(
        Sequence< Reference< deployment::XPackage > > * pComp,
        Reference< XCommandEnvironment > const & xCmdEnv );
    bool bindComposition_(
        ::std::vector< Reference< deployment::XPackage > > & comp,
        OUString const & url, Reference< XCommandEnvironment > const & xCmdEnv,
        bool skip_registration = false );

protected:
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

    virtual void SAL_CALL disposing();

    inline BackendImpl * getMyBackend() const
        { return static_cast< BackendImpl * >(m_myBackend.get()); }
public:
    PackageImpl(
        ::rtl::Reference< PackageRegistryBackend > const & myBackend,
        OUString const & url,
        OUString const & mediaType,
        OUString const & name,
        bool legacy_package );
};

//==============================================================================
OUString SAL_CALL getImplementationName()
{
    return OUSTR(
        "com.sun.star.comp.deployment.package.PackageRegistryBackend");
}

//==============================================================================
Reference< XInterface > SAL_CALL create(
    Reference< XComponentContext > const & xComponentContext )
    SAL_THROW( (Exception) )
{
    OUString const media_types [] = {
        OUSTR("application/vnd.sun.star.uno-package"),
        OUSTR("application/vnd.sun.star.uno-legacy-package")
    };
    return static_cast< ::cppu::OWeakObject * >(
        new BackendImpl(
            xComponentContext, getImplementationName(),
            Sequence< OUString >( media_types, ARLEN(media_types) ) ) );
}

//______________________________________________________________________________
void BackendImpl::disposing()
{
    // close db:
    m_registeredPackages.reset(0);
    PackageRegistryBackend::disposing();
}

// XInitialization
//______________________________________________________________________________
void BackendImpl::initialize( Sequence< Any > const & args )
    throw (Exception)
{
    PackageRegistryBackend::initialize( args );
    if (getCachePath().getLength() > 0)
    {
        m_registeredPackages.reset(
            new PersistentMap(
                make_url( getCachePath(), OUSTR("registered_packages.db") ),
                getComponentContext() ) );
    }
}

// XPackageRegistry
//______________________________________________________________________________
OUString BackendImpl::detectMediaType( OUString const & url )
    throw (RuntimeException)
{
    ::ucb::Content ucb_content;
    if (::ucb::Content::create( url, Reference< XCommandEnvironment >(),
                                ucb_content ))
    {
        OUString title( extract_throw< OUString >(
                            ucb_content.getPropertyValue( OUSTR("Title") ) ) );
        if (title.endsWithIgnoreAsciiCaseAsciiL(
                RTL_CONSTASCII_STRINGPARAM(".uno.pkg") ))
            return OUSTR("application/vnd.sun.star.uno-package");
        if (title.endsWithIgnoreAsciiCaseAsciiL(
                RTL_CONSTASCII_STRINGPARAM(".zip") ))
            return OUSTR("application/vnd.sun.star.uno-legacy-package");
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
    ::ucb::Content ucb_content( url, xCmdEnv );
    OUString name( extract_throw< OUString >(
                       ucb_content.getPropertyValue( OUSTR("Title") ) ) );

    bool legacy_package;
    if (subType.matchIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.uno-package") ))
    {
        legacy_package = false;
    }
    else if (subType.matchIgnoreAsciiCaseAsciiL(
                 RTL_CONSTASCII_STRINGPARAM(
                     "vnd.sun.star.uno-legacy-package") ))
    {
        legacy_package = true;
    }
    else
    {
        throw lang::IllegalArgumentException(
            OUSTR("invalid sub-type given: ") + subType,
            static_cast< OWeakObject * >(this),
            static_cast< sal_Int16 >(-1 /* not known */) );
    }
    return new PackageImpl(
        this, url, mediaType, name, legacy_package );
}

//______________________________________________________________________________
PackageImpl::PackageImpl(
    ::rtl::Reference< PackageRegistryBackend > const & myBackend,
    OUString const & url,
    OUString const & mediaType,
    OUString const & name,
    bool legacy_package )
    : Package( myBackend, url, mediaType, name, name /* display-name */,
               // Description:
               legacy_package
               ? OUSTR("Legacy UNO Package Composition")
               : OUSTR("UNO Package Composition"),
               true /* has package composition property */ ),
      m_legacy_package( legacy_package ),
      m_registered( REG_VOID ),
      m_composition_init( false ),
      m_composition_init_success( false )
{
    ::rtl::OUStringBuffer buf;
    buf.appendAscii(
        RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.pkg://") );
    buf.append( ::rtl::Uri::encode(
                    url, rtl_UriCharClassRegName,
                    rtl_UriEncodeIgnoreEscapes,
                    RTL_TEXTENCODING_UTF8 ) );
    buf.append( static_cast< sal_Unicode >('/') );
    m_scan_url = buf.makeStringAndClear();
}

// OPropertySetHelper
//______________________________________________________________________________
void PackageImpl::getFastPropertyValue( Any & rValue, sal_Int32 nHandle ) const
{
    // xxx todo: upon disposing() getPropertyValue( URL )...
//     check();
    switch (nHandle)
    {
    case COMPOSITION:
    {
        try
        {
            Sequence< Reference< deployment::XPackage > > comp;
            if (const_cast< PackageImpl * >(this)->bindComposition(
                    &comp, Reference< XCommandEnvironment >() ))
            {
                rValue <<= comp;
            }
            else
            {
                rValue.clear();
            }
        }
        catch (RuntimeException &)
        {
            throw;
        }
        catch (Exception &)
        {
            Any exc( ::cppu::getCaughtException() );
            throw lang::WrappedTargetException(
                OUSTR("caught exception while binding package composition!"),
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

//______________________________________________________________________________
void PackageImpl::disposing()
{
    sal_Int32 len = m_composition.getLength();
    Reference< deployment::XPackage > const * p =
        m_composition.getConstArray();
    for ( sal_Int32 pos = 0; pos < len; ++pos )
        try_dispose( p[ pos ] );
    m_composition.realloc( 0 );

    Package::disposing();
}

// Package
//______________________________________________________________________________
PackageImpl::t_Registered PackageImpl::getRegStatus(
    ::osl::ResettableMutexGuard & guard,
    Reference< XCommandEnvironment > const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    if (m_registered == REG_VOID)
    {
        if (that->m_registeredPackages.get() == 0)
            return REG_VOID;
        m_registered = REG_NOT_REGISTERED;
        if (that->m_registeredPackages->has( m_url ))
            m_registered = REG_REGISTERED;
    }
    return m_registered;
}

//______________________________________________________________________________
bool PackageImpl::processPackage(
    bool register_package,
    ::osl::ResettableMutexGuard & guard,
    Reference< XCommandEnvironment > const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    ProgressLevel progress( xCmdEnv );
    Sequence< Reference< deployment::XPackage > > comp;
    bool success = bindComposition( &comp, xCmdEnv );
    if (success)
    {
        m_registered = REG_VOID;
        if (register_package)
        {
            progress.update( m_url + OUSTR(": registering composition...") );
            sal_Int32 len = comp.getLength();
            for ( sal_Int32 pos = 0; pos < len; ++pos )
            {
                try
                {
                    success &= (comp[ pos ]->registerPackage( xCmdEnv )
                                != sal_False);
                }
                catch (deployment::DeploymentException &)
                {
                    progress.update( OUSTR("starting rollback...") );
                    success = true;
                    while (pos--) // reverse order
                    {
                        try
                        {
                            comp[ pos ]->revokePackage( xCmdEnv );
                        }
                        catch (deployment::DeploymentException & exc)
                        {
                            OSL_ENSURE(
                                0, ::rtl::OUStringToOString(
                                    ::comphelper::anyToString( makeAny(exc) ),
                                    RTL_TEXTENCODING_UTF8 ) );
                            success = false;
                        }
                        progress.update(
                            success
                            ? OUSTR("rollback finished.")
                            : OUSTR("rollback failed!") );
                    }
                    throw;
                }
            }
            progress.update(
                m_url + OUSTR(": finished registering composition.") );
            if (success)
                m_registered = REG_REGISTERED;
        }
        else
        {
            progress.update( m_url + OUSTR(": revoking composition...") );
            // revoke in reverse order:
            for ( sal_Int32 pos = comp.getLength(); pos--; )
            {
                try
                {
                    success &= (comp[ pos ]->revokePackage( xCmdEnv )
                                != sal_False);
                }
                catch (...)
                {
                    throw;
                }
            }
            progress.update(
                m_url + OUSTR(": finished revoking composition.") );
            if (success)
                m_registered = REG_NOT_REGISTERED;
        }
    }

    // update persistence:
    if (that->m_registeredPackages.get() != 0)
    {
        if (m_registered == REG_REGISTERED)
            that->m_registeredPackages->put( m_url, m_mediaType );
        else if (m_registered == REG_NOT_REGISTERED)
            that->m_registeredPackages->erase( m_url );
    }

    return success;
}

//______________________________________________________________________________
bool PackageImpl::bindComposition(
    Sequence< Reference< deployment::XPackage > > * pComp,
    Reference< XCommandEnvironment > const & xCmdEnv )
{
    if (! m_composition_init)
    {
        ProgressLevel progress(
            xCmdEnv, OUSTR("binding packages composition ") + m_url );

        typedef ::std::vector< Reference< deployment::XPackage > > t_packagevec;
        t_packagevec comp;
        bool success;
        try
        {
            success = bindComposition_( comp, m_scan_url, xCmdEnv );
        }
        catch (...) // finally
        {
            {
            ::osl::MutexGuard guard( getMutex() );
            if (! m_composition_init)
            {
                m_composition_init_success = false;
                m_composition_init = true;
            }
            }
            throw;
        }

        // sort: schema before config data, typelibs before components:
        Sequence< Reference< deployment::XPackage > > ret(
            static_cast< sal_Int32 >(comp.size()) );
        Reference< deployment::XPackage > * pret = ret.getArray();
        sal_Int32 lower_end = 0;
        sal_Int32 upper_end = ret.getLength();
        t_packagevec::const_iterator iPos( comp.begin() );
        t_packagevec::const_iterator const iEnd( comp.end() );
        OUString str_MediaType = OUSTR("MediaType");
        for ( ; iPos != iEnd; ++iPos )
        {
            Reference< beans::XPropertySet > xProps( *iPos, UNO_QUERY_THROW );
            OUString mediaType( extract_throw< OUString >(
                                    xProps->getPropertyValue(str_MediaType) ) );
            // xxx todo: normalize media-types?
            if (mediaType.matchIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM(
                        "application/vnd.sun.star.uno-component;") ) ||
                mediaType.matchIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM(
                        "application/vnd.sun.star.configuration-data") ))
            {
                --upper_end;
                pret[ upper_end ] = *iPos;
            }
            else
            {
                pret[ lower_end ] = *iPos;
                ++lower_end;
            }
        }
        OSL_ASSERT( lower_end == upper_end );

        ::osl::MutexGuard guard( getMutex() );
        if (! m_composition_init)
        {
            m_composition = ret;
            m_composition_init_success = success;
            m_composition_init = true;
        }
    }
    *pComp = m_composition;
    return m_composition_init_success;
}

//______________________________________________________________________________
bool PackageImpl::bindComposition_(
    ::std::vector< Reference< deployment::XPackage > > & comp,
    OUString const & url,
    Reference< XCommandEnvironment > const & xCmdEnv,
    bool skip_registration )
{
    ProgressLevel progress( xCmdEnv, OUSTR("scanning folder ") + url );
    ::ucb::Content ucb_content( url, xCmdEnv );

    if (m_legacy_package)
    {
        // check for platform pathes:
        OUString title( extract_throw< OUString >(
                            ucb_content.getPropertyValue( OUSTR("Title") ) ) );
        if (title.endsWithIgnoreAsciiCaseAsciiL(
                RTL_CONSTASCII_STRINGPARAM(".plt") ))
        {
            if (! platform_fits( title.copy( 0, title.getLength() - 4 ) ))
            {
                progress.update( OUSTR("ignoring path ") + url );
                return true;
            }
        }

        if (title.endsWithIgnoreAsciiCaseAsciiL(
                RTL_CONSTASCII_STRINGPARAM("skip_registration") ))
        {
            skip_registration = true;
            progress.update(
                OUSTR("recognized skip_registration folder: ") + url );
        }
    }

    OUString ar [] = { OUSTR("Title"), OUSTR("IsFolder") };
    Reference< sdbc::XResultSet > xResultSet(
        ucb_content.createCursor( Sequence< OUString >( ar, ARLEN(ar) ),
                                  ::ucb::INCLUDE_FOLDERS_AND_DOCUMENTS ) );
    bool success = true;
    while (xResultSet->next())
    {
        Reference< sdbc::XRow > xRow( xResultSet, UNO_QUERY_THROW );
        OUString title( xRow->getString( 1 /* Title */ ) );
        OUString path( make_url( url, ::rtl::Uri::encode(
                                     title, rtl_UriCharClassPchar,
                                     rtl_UriEncodeIgnoreEscapes,
                                     RTL_TEXTENCODING_UTF8 ) ) );
        OUString mediaType;
        if (m_legacy_package)
        {
            mediaType =
                m_myBackend->getPackageRegistry()->detectMediaType( path );
        }
        else // ! m_legacy_package
        {
            Reference< XContentAccess > xContentAccess(
                xRow, UNO_QUERY_THROW );
            ::ucb::Content item( xContentAccess->queryContent(), xCmdEnv );
            try
            {
                item.getPropertyValue( OUSTR("MediaType") ) >>= mediaType;
            }
            catch (beans::UnknownPropertyException &)
            {
            }
        }

        if (mediaType.getLength() == 0)
        {
            if (xRow->getBoolean( 2 /* IsFolder */ ))
            {
                // recurse into folder:
                success &= bindComposition_(
                    comp, path, xCmdEnv, skip_registration );
            }
            // xxx todo: discuss whether it is sensible to ignore files
            // within .uno.pkg packages that don't have a media-type entry
            else
            {
                // ignore this item:
                ProgressLevel progress( xCmdEnv );
                progress.update(
                    OUSTR("WARNING: ignoring package item ") + path );
            }
            continue;
        }

        if (skip_registration &&
            // xxx todo: additional media-type parsing?
            mediaType.matchIgnoreAsciiCaseAsciiL(
                RTL_CONSTASCII_STRINGPARAM(
                    "application/vnd.sun.star.uno-component") ))
            continue; // no components

        Reference< deployment::XPackage > xPackage;
        bool ret = m_myBackend->getPackageRegistry()->bindPackage(
            xPackage, path, mediaType, xCmdEnv );
        success &= ret;
        if (ret && xPackage.is())
            comp.push_back( xPackage );
    }
    return success;
}

} // namespace package
} // namespace backend
} // namespace dp_registry

