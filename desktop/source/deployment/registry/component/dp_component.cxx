/*************************************************************************
 *
 *  $RCSfile: dp_component.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-04-13 12:08:32 $
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
#include "rtl/string.hxx"
#include "rtl/strbuf.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/uri.hxx"
#include "osl/file.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "ucbhelper/content.hxx"
#include "comphelper/anytostring.hxx"
#include "xmlscript/xml_helper.hxx"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "com/sun/star/registry/XSimpleRegistry.hpp"
#include "com/sun/star/registry/XImplementationRegistration.hpp"
#include "com/sun/star/loader/XImplementationLoader.hpp"
#include "com/sun/star/io/XInputStream.hpp"
#include "com/sun/star/sdbc/XResultSet.hpp"
#include "com/sun/star/sdbc/XRow.hpp"
#include "com/sun/star/ucb/NameClash.hpp"
#include "com/sun/star/container/XSet.hpp"
#include <hash_set>
#include <vector>
#include <memory>
#include <algorithm>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;
using ::osl::File;

namespace dp_registry
{
namespace backend
{
namespace component
{

typedef ::std::hash_set<
    ::rtl::OUString, ::rtl::OUStringHash > t_stringset;

//==============================================================================
class BackendImpl : public ::dp_registry::backend::PackageRegistryBackend
{
    t_stringset m_jar_typelibs;
    t_stringset m_rdb_typelibs;
    bool m_unorc_inited;
    bool m_unorc_modified;

    OUString m_inflated_dir, m_inflated_reg_dir;
    ::std::auto_ptr< PersistentMap > m_urlToTemp;

protected:
    // PackageRegistryBackend
    virtual Reference< deployment::XPackage > createPackage(
        OUString const & url,
        OUString const & mediaType, OUString const & subType,
        INetContentTypeParameterList const & params,
        Reference< XCommandEnvironment > const & xCmdEnv );

    virtual void SAL_CALL disposing();

public:
    Reference< registry::XImplementationRegistration > m_xImplReg;
    Reference< registry::XSimpleRegistry > m_xServicesRDB;

    t_stringset & getTypelibs( bool jar )
        { return jar ? m_jar_typelibs : m_rdb_typelibs; }
    void unorc_verify_init( Reference< XCommandEnvironment > const & xCmdEnv );
    void unorc_flush( Reference< XCommandEnvironment > const & xCmdEnv );

    OUString getInflatedCopy(
        OUString const & url,
        Reference< XCommandEnvironment > const & xCmdEnv );
    void removeInflatedCopy(
        OUString const & url,
        Reference< XCommandEnvironment > const & xCmdEnv );

    bool addToUnoRc(
        bool jarFile, OUString const & url,
        Reference< XCommandEnvironment > const & xCmdEnv );
    bool removeFromUnoRc(
        bool jarFile, OUString const & url,
        Reference< XCommandEnvironment > const & xCmdEnv );

    Sequence< Reference< lang::XServiceInfo > > loadFactories(
        OUString const & url, OUString const & loader );

    inline BackendImpl(
        Reference< XComponentContext > const & xComponentContext,
        OUString const & implName,
        Sequence< OUString > const & supported_media_types )
        : PackageRegistryBackend(
            xComponentContext, implName, supported_media_types ),
          m_unorc_inited( false ),
          m_unorc_modified( false ),
          m_xImplReg(
              xComponentContext->getServiceManager()
              ->createInstanceWithContext(
                  OUSTR("com.sun.star.registry.ImplementationRegistration"),
                  xComponentContext ), UNO_QUERY_THROW )
        {}

    // XInitialization
    virtual void SAL_CALL initialize( Sequence< Any > const & args )
        throw (RuntimeException);

    // XPackageRegistry
    virtual OUString SAL_CALL detectMediaType( OUString const & url )
        throw (RuntimeException);
};

//==============================================================================
class ComponentPackageImpl : public ::dp_registry::backend::Package
{
    OUString m_inflated_url;
    OUString m_loader;
    t_Registered m_registered;

    inline BackendImpl * getMyBackend() const
        { return static_cast< BackendImpl * >(m_myBackend.get()); }

    // Package
    virtual t_Registered getRegStatus(
        ::osl::ResettableMutexGuard & guard,
        Reference< XCommandEnvironment > const & xCmdEnv );
    virtual bool processPackage(
        bool register_package,
        ::osl::ResettableMutexGuard & guard,
        Reference< XCommandEnvironment > const & xCmdEnv );

public:
    inline ComponentPackageImpl(
        ::rtl::Reference< PackageRegistryBackend > const & myBackend,
        OUString const & url, OUString const & mediaType,
        OUString const & name, OUString const & description,
        OUString const & loader )
        : Package( myBackend, url, mediaType, name, name /* display-name */,
                   description ),
          m_loader( loader ),
          m_registered( REG_VOID )
        {}
};

//==============================================================================
class TypelibraryPackageImpl : public ::dp_registry::backend::Package
{
    OUString m_inflated_unorc_url;
    bool m_jarFile;
    t_Registered m_registered;
    Reference< container::XHierarchicalNameAccess > m_xTDprov;

    inline BackendImpl * getMyBackend() const
        { return static_cast< BackendImpl * >(m_myBackend.get()); }

    // Package
    virtual t_Registered getRegStatus(
        ::osl::ResettableMutexGuard & guard,
        Reference< XCommandEnvironment > const & xCmdEnv );
    virtual bool processPackage(
        bool register_package,
        ::osl::ResettableMutexGuard & guard,
        Reference< XCommandEnvironment > const & xCmdEnv );

public:
    inline TypelibraryPackageImpl(
        ::rtl::Reference< PackageRegistryBackend > const & myBackend,
        OUString const & url, OUString const & mediaType,
        OUString const & name, OUString const & description, bool jarFile )
        : Package( myBackend, url, mediaType, name, name /* display-name */,
                   description ),
          m_jarFile( jarFile ),
          m_registered( REG_VOID )
        {}
};

//______________________________________________________________________________
void BackendImpl::disposing()
{
    try
    {
        m_xServicesRDB->close();
        m_xServicesRDB.clear();
        m_urlToTemp.reset(0);
        unorc_flush( Reference< XCommandEnvironment >() );

        PackageRegistryBackend::disposing();
    }
    catch (RuntimeException &)
    {
        throw;
    }
    catch (Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException(
            OUSTR("caught unexpected exception while disposing..."),
            static_cast< OWeakObject * >(this), exc );
    }
}

struct predicate_equalsIgnoreAsciiCase
{
    OUString m_str;
    predicate_equalsIgnoreAsciiCase( OUString const & str ) : m_str( str ) {}
    bool operator () ( t_string2string_map::value_type const & v ) const
        { return v.second.equalsIgnoreAsciiCase( m_str ); }
};

//______________________________________________________________________________
void BackendImpl::initialize( Sequence< Any > const & args )
    throw (RuntimeException)
{
    PackageRegistryBackend::initialize( args );

    // services.rdb:
    m_xServicesRDB.set(
        getComponentContext()->getServiceManager()
        ->createInstanceWithContext(
            OUSTR("com.sun.star.registry.SimpleRegistry"),
            getComponentContext() ), UNO_QUERY_THROW );

    if (transientMode())
    {
        // in-mem rdb:
        m_xServicesRDB->open( OUString() /* in-mem */,
                              false /* ! read-only */, true /* create */ );
        // inflated in-mem DB:
        m_urlToTemp.reset( new PersistentMap );
    }
    else
    {
        m_xServicesRDB->open(
            // xxx todo: switch to alternating rdbs:
            expand_reg_url( make_url( getCachePath(), OUSTR("services.rdb") ),
                            getComponentContext() ),
            false /* ! read-only */, true /* create */ );

        // ensure "inflated" folder:
        m_inflated_reg_dir = make_url( getCachePath(), OUSTR("inflated") );
        ::ucb::Content inflated_dir_content;
        create_folder( &inflated_dir_content, m_inflated_reg_dir );
        m_inflated_dir = expand_reg_url(
            m_inflated_reg_dir, getComponentContext() );

        // scan for zombie temp dirs:
        OUString inflated_db_url(
            make_url( getCachePath(), OUSTR("inflated.db") ) );
        m_urlToTemp.reset( new PersistentMap( inflated_db_url,
                                              getComponentContext() ) );
        t_string2string_map urlToTemp( m_urlToTemp->getEntries() );

        OUString str_Title = OUSTR("Title");
        Reference< sdbc::XResultSet > xResultSet(
            inflated_dir_content.createCursor(
                Sequence< OUString >( &str_Title, 1 ),
                ::ucb::INCLUDE_DOCUMENTS_ONLY ) );
        // get all inflated temp directories:
        ::std::vector< OUString > tempEntries;
        while (xResultSet->next())
        {
            OUString title( Reference< sdbc::XRow >(
                                xResultSet, UNO_QUERY_THROW )->getString(
                                    1 /* Title */ ) );
            tempEntries.push_back( ::rtl::Uri::encode(
                                       title, rtl_UriCharClassPchar,
                                       rtl_UriEncodeIgnoreEscapes,
                                       RTL_TEXTENCODING_UTF8 ) );
        }

        for ( ::std::size_t pos = 0; pos < tempEntries.size(); ++pos )
        {
            OUString const & tempEntry = tempEntries[ pos ];
            if (::std::find_if( urlToTemp.begin(), urlToTemp.end(),
                                predicate_equalsIgnoreAsciiCase( tempEntry ) )
                == urlToTemp.end())
            {
                // temp entry not needed anymore:
                OUString url( make_url( m_inflated_reg_dir, tempEntry ) );
                erase_path( url );
                erase_path( url + OUSTR(".inflated") );
            }
            OSL_ASSERT( ! m_urlToTemp->has( tempEntry ) );
        }
    }
}

//==============================================================================
OUString SAL_CALL getImplementationName()
{
    return OUSTR(
        "com.sun.star.comp.deployment.component.PackageRegistryBackend");
}

//==============================================================================
Reference< XInterface > SAL_CALL create(
    Reference< XComponentContext > const & xComponentContext )
    SAL_THROW( (Exception) )
{
    OUString const media_types [] = {
        OUSTR("application/vnd.sun.star.uno-component"),
        OUSTR("application/vnd.sun.star.uno-typelibrary")
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
                                ucb_content ))
    {
        OUString title( extract_throw< OUString >(
                            ucb_content.getPropertyValue( OUSTR("Title") ) ) );
        if (title.endsWithIgnoreAsciiCaseAsciiL(
                RTL_CONSTASCII_STRINGPARAM(SAL_DLLEXTENSION) ))
            return OUSTR("application/vnd.sun.star.uno-component;type=native;"
                         "platform=" THIS_OS "_" THIS_CPU);
        if (title.endsWithIgnoreAsciiCaseAsciiL(
                RTL_CONSTASCII_STRINGPARAM(".jar") ))
        {
            // xxx todo: ait for MAV fix:
//             // read .jar manifest file:
//             ::rtl::OUStringBuffer buf;
//             buf.appendAscii(
//                 RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.pkg://") );
//             buf.append( ::rtl::Uri::encode( url,
//                                             rtl_UriCharClassRegName,
//                                             rtl_UriEncodeIgnoreEscapes,
//                                             RTL_TEXTENCODING_UTF8 ) );
//             buf.appendAscii(
//                 RTL_CONSTASCII_STRINGPARAM("/META-INF/MANIFEST.MF") );
//             ::ucb::Content manifest_content;
//             if (create_ucb_content( &manifest_content, buf.makeStringAndClear(),
//                                     Reference< XCommandEnvironment >(),
//                                     false /* no throw */ ))
//             {
//                 OUString line;
//                 if (readLine( &line, OUSTR("RegistrationClassName"),
//                               manifest_content, RTL_TEXTENCODING_ASCII_US ))
                    return OUSTR(
                        "application/vnd.sun.star.uno-component;type=Java");
//             }
//             return OUSTR("application/vnd.sun.star.uno-typelibrary;type=Java");
        }
        if (title.endsWithIgnoreAsciiCaseAsciiL(
                RTL_CONSTASCII_STRINGPARAM(".py") ))
            return OUSTR("application/vnd.sun.star.uno-component;type=Python");
        if (title.endsWithIgnoreAsciiCaseAsciiL(
                RTL_CONSTASCII_STRINGPARAM(".rdb") ))
            return OUSTR("application/vnd.sun.star.uno-typelibrary;type=RDB");
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

    // xxx todo: probe and evaluate component xml description

    if (subType.matchIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.uno-component") ))
    {
        INetContentTypeParameter const * param = params.find( "type" );
        if (param != 0)
        {
            OUString value(param->m_sValue);
            if (value.equalsIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM("native") ))
            {
                return new ComponentPackageImpl(
                    this, url, mediaType, name,
                    OUSTR("UNO Shared Library Component"),
                    OUSTR("com.sun.star.loader.SharedLibrary") );
            }
            if (value.equalsIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM("Java") ))
            {
                return new ComponentPackageImpl(
                    this, url, mediaType, name,
                    OUSTR("UNO Java Component"),
                    OUSTR("com.sun.star.loader.Java2") );
            }
            if (value.equalsIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM("Python") ))
            {
                return new ComponentPackageImpl(
                    this, url, mediaType, name,
                    OUSTR("UNO Python Component"),
                    OUSTR("com.sun.star.loader.Python") );
            }
            else // fallback: guess loader  xxx todo discuss
            {
                OUString loader( OUSTR("com.sun.star.loader.") + value );
                // check if loader service available:
                if (getComponentContext()->getServiceManager()
                    ->createInstanceWithContext(
                        loader, getComponentContext() ).is())
                {
                    ::rtl::OUStringBuffer buf;
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("UNO ") );
                    buf.append( value );
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" Component") );
                    return new ComponentPackageImpl(
                        this, url, mediaType, name,
                        buf.makeStringAndClear(), loader );
                }
            }
            throw lang::IllegalArgumentException(
                OUSTR("no support for UNO Components of type: ") + value,
                static_cast< OWeakObject * >(this),
                static_cast< sal_Int16 >(-1 /* not known */) );
        }
    }
    else if (subType.matchIgnoreAsciiCaseAsciiL(
                 RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.uno-typelibrary") ))
    {
        INetContentTypeParameter const * param = params.find( "type" );
        if (param != 0)
        {
            OUString value(param->m_sValue);
            if (value.equalsIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM("RDB") ))
            {
                return new TypelibraryPackageImpl(
                    this, url, mediaType, name,
                    OUSTR("Native Type-Library File"), false /* rdb */ );
            }
            if (value.equalsIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM("Java") ))
            {
                return new TypelibraryPackageImpl(
                    this, url, mediaType, name,
                    OUSTR("Java Type-Library File"), true /* jar */ );
            }
            throw lang::IllegalArgumentException(
                OUSTR("no support for type-libraries of type: ") + value,
                static_cast< OWeakObject * >(this),
                static_cast< sal_Int16 >(-1 /* not known */) );
        }
    }
    throw lang::IllegalArgumentException(
        OUSTR("invalid media-type given: ") + mediaType,
        static_cast< OWeakObject * >(this),
        static_cast< sal_Int16 >(-1 /* not known */) );
}

//##############################################################################

//______________________________________________________________________________
void BackendImpl::unorc_verify_init(
    Reference< XCommandEnvironment > const & xCmdEnv )
{
    if (transientMode())
        return;
    ::osl::MutexGuard guard( getMutex() );
    if (! m_unorc_inited)
    {
        ::ucb::Content ucb_content;
        if (create_ucb_content(
                &ucb_content,
                make_url( getCachePath(), OUSTR(SAL_CONFIGFILE("uno")) ),
                xCmdEnv, false /* no throw */ ))
        {
            OUString line;
            if (readLine( &line, OUSTR("UNO_JAVA_CLASSPATH="), ucb_content,
                          RTL_TEXTENCODING_UTF8 ))
            {
                sal_Int32 index = sizeof ("UNO_JAVA_CLASSPATH=") - 1;
                do
                {
                    OUString token( line.getToken( 0, ' ', index ).trim() );
                    if (token.getLength() > 0)
                        m_jar_typelibs.insert( token );
                }
                while (index >= 0);
            }
            if (readLine( &line, OUSTR("UNO_TYPES="), ucb_content,
                          RTL_TEXTENCODING_UTF8 ))
            {
                sal_Int32 index = sizeof ("UNO_TYPES=") - 1;
                do
                {
                    OUString token( line.getToken( 0, ' ', index ).trim() );
                    if (token.getLength() > 0)
                        m_rdb_typelibs.insert( token );
                }
                while (index >= 0);
            }
            m_unorc_modified = false;
            m_unorc_inited = true;
        }
        else
        {
            // file has never been written before: write now
            m_unorc_modified = true;
            m_unorc_inited = true;
            unorc_flush( xCmdEnv );
        }
    }
}

//______________________________________________________________________________
void BackendImpl::unorc_flush(
    Reference< XCommandEnvironment > const & xCmdEnv )
{
    if (transientMode())
        return;
    if (!m_unorc_inited || !m_unorc_modified)
        return;

    ::rtl::OStringBuffer buf;
    if (! m_jar_typelibs.empty())
    {
        t_stringset::const_iterator iPos( m_jar_typelibs.begin() );
        t_stringset::const_iterator const iEnd( m_jar_typelibs.end() );
        buf.append( RTL_CONSTASCII_STRINGPARAM("UNO_JAVA_CLASSPATH=") );
        while (iPos != iEnd)
        {
            // encoded ASCII file-urls:
            ::rtl::OString item(
                ::rtl::OUStringToOString( *iPos, RTL_TEXTENCODING_ASCII_US ) );
            buf.append( item );
            ++iPos;
            if (iPos != iEnd)
                buf.append( ' ' );
        }
        buf.append( '\n' );
    }
    if (! m_jar_typelibs.empty())
    {
        t_stringset::const_iterator iPos( m_rdb_typelibs.begin() );
        t_stringset::const_iterator const iEnd( m_rdb_typelibs.end() );
        buf.append( RTL_CONSTASCII_STRINGPARAM("UNO_TYPES=") );
        while (iPos != iEnd)
        {
            // encoded ASCII file-urls:
            ::rtl::OString item(
                ::rtl::OUStringToOString( *iPos, RTL_TEXTENCODING_ASCII_US ) );
            buf.append( item );
            ++iPos;
            if (iPos != iEnd)
                buf.append( ' ' );
        }
        buf.append( '\n' );
    }
    buf.append(
        RTL_CONSTASCII_STRINGPARAM("UNO_SERVICES=$ORIGIN/services.rdb\n") );

    Reference< io::XInputStream > xData(
        ::xmlscript::createInputStream(
            ::rtl::ByteSequence(
                reinterpret_cast< sal_Int8 const * >(buf.getStr()),
                buf.getLength() ) ) );
    ::ucb::Content ucb_content(
        make_url( getCachePath(), OUSTR(SAL_CONFIGFILE("uno")) ), xCmdEnv );
    ucb_content.writeStream( xData, true /* replace existing */ );
    m_unorc_modified = false;
}

//------------------------------------------------------------------------------
static inline OUString encodeForRcFile( OUString const & str )
{
    // escape $\{} (=> rtl bootstrap files)
    ::rtl::OUStringBuffer buf;
    sal_Int32 pos = 0;
    sal_Int32 len = str.getLength();
    for ( ; pos < len; ++pos )
    {
        sal_Unicode c = str[ pos ];
        switch (c)
        {
        case '$':
        case '\\':
        case '{':
        case '}':
            buf.append( static_cast< sal_Unicode >('\\') );
            break;
        }
        buf.append( c );
    }
    return buf.makeStringAndClear();
}

//______________________________________________________________________________
bool BackendImpl::addToUnoRc(
    bool jarFile, OUString const & rcurl,
    Reference< XCommandEnvironment > const & xCmdEnv )
{
    ::osl::MutexGuard guard( getMutex() );
    unorc_verify_init( xCmdEnv );
    ::std::pair< t_stringset::iterator, bool > insertion(
        getTypelibs(jarFile).insert( rcurl ) );
    m_unorc_modified |= insertion.second;
    // write immediately:
    unorc_flush( xCmdEnv );
    return insertion.second;
}

//______________________________________________________________________________
bool BackendImpl::removeFromUnoRc(
    bool jarFile, OUString const & rcurl,
    Reference< XCommandEnvironment > const & xCmdEnv )
{
    ::osl::MutexGuard guard( getMutex() );
    unorc_verify_init( xCmdEnv );
    ::std::size_t erased = getTypelibs(jarFile).erase( rcurl );
    bool ret = (erased > 0);
    m_unorc_modified |= ret;
    OSL_ASSERT( ret );
    // write immediately:
    unorc_flush( xCmdEnv );
    return ret;
}

//------------------------------------------------------------------------------
void copyDeep( OUString const & source_url, OUString const & dest_url,
               Reference< XCommandEnvironment > const & xCmdEnv )
{
    // prepare destination folder:
    ::ucb::Content ucb_dest_folder;
    create_folder( &ucb_dest_folder,
                   dest_url.copy( 0, dest_url.lastIndexOf( '/' ) ) );

    ::rtl::OUStringBuffer buf;
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("inflating ") );
    buf.append( source_url );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" to ") );
    buf.append( dest_url );
    ProgressLevel progress( xCmdEnv );
    progress.update( buf.makeStringAndClear() );

    ::ucb::Content source_content( source_url, xCmdEnv );
    if (! ucb_dest_folder.transferContent(
            source_content, ::ucb::InsertOperation_COPY,
            OUString(), NameClash::OVERWRITE ))
        throw RuntimeException(
            OUSTR("::ucb::Content::transferContent() failed!"), 0 );

    // copy dependent files:
    if (// xxx todo: wait for MAV fix in package
        dest_url.endsWithIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM(".jar") ))
    {
        // .jar manifest Class-Path dependencies:
        ::rtl::OUStringBuffer buf;
        buf.appendAscii(
            RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.pkg://") );
        buf.append( ::rtl::Uri::encode( /* xxx todo */dest_url,
                                        rtl_UriCharClassRegName,
                                        rtl_UriEncodeIgnoreEscapes,
                                        RTL_TEXTENCODING_UTF8 ) );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("/META-INF/MANIFEST.MF") );
        ::ucb::Content manifest_content;
        if (create_ucb_content(
                &manifest_content,
                buf.makeStringAndClear(), xCmdEnv, false /* no throw */ ))
        {
            OUString line;
            if (readLine( &line, OUSTR("Class-Path"),
                          manifest_content, RTL_TEXTENCODING_ASCII_US ))
            {
                sal_Int32 index = line.indexOf( ':' );
                if (index >= 0)
                {
                    OUString dest_dir(
                        dest_url.copy( 0, dest_url.lastIndexOf( '/' ) ) );
                    ++index;
                    do
                    {
                        OUString rel_path(
                            line.getToken( 0, ' ', index ).trim() );
                        if (rel_path.getLength() > 0)
                        {
                            OUString source(
                                make_url(
                                    source_url.copy(
                                        0, source_url.lastIndexOf( '/' ) ),
                                    rel_path ) );
                            copyDeep( source, make_url( dest_dir, rel_path ),
                                      xCmdEnv );
                        }
                    }
                    while (index >= 0);
                }
            }
        }
    }
//     else if (source_url.endsWithIgnoreAsciiCaseAsciiL(
//                  RTL_CONSTASCII_STRINGPARAM(SAL_DLLEXTENSION) ))
//     {
//         // xxx todo?
//         // check .RPATH dependencies for .so files:
//     }
}

//______________________________________________________________________________
OUString BackendImpl::getInflatedCopy(
    OUString const & url, Reference< XCommandEnvironment > const & xCmdEnv )
{
    ::osl::MutexGuard guard( getMutex() );
    OUString package( url.copy( url.lastIndexOf( '/' ) + 1 ) );
    OUString tempEntry;
    if (m_urlToTemp->get( &tempEntry, url ))
    {
        OUString dir( make_url( m_inflated_reg_dir, tempEntry ) );
        dir += OUSTR(".inflated");
        return make_url( dir, package );
    }

    OUString dir;
    // create new temp dir:
    File::RC rc;
    if (transientMode())
    {
        rc = File::createTempFile( 0, 0, &dir );
        tempEntry = dir.copy( dir.lastIndexOf( '/' ) + 1 );
    }
    else
    {
        rc = File::createTempFile( &m_inflated_dir, 0, &dir );
        OSL_ASSERT( dir.matchIgnoreAsciiCase( m_inflated_dir ) );
        tempEntry = dir.copy( dir.lastIndexOf( '/' ) + 1 );
        dir = make_url( m_inflated_reg_dir, tempEntry );
    }
    if (rc != File::E_None)
        throw RuntimeException(
            OUSTR("::osl::File::createTempFile() failed!"), 0 );
    dir += OUSTR(".inflated");

    OUString dest_url( make_url( dir, package ) );
    copyDeep( url, dest_url, xCmdEnv );

    // add new entry to DB:
    m_urlToTemp->put( url, tempEntry );
    return dest_url;
}

//______________________________________________________________________________
void BackendImpl::removeInflatedCopy(
    OUString const & url, Reference< XCommandEnvironment > const & xCmdEnv )
{
    ::osl::MutexGuard guard( getMutex() );
    bool erased = m_urlToTemp->erase( url );
    OSL_ENSURE( erased, "entry not in inflated DB!?" );
}

//______________________________________________________________________________
Sequence< Reference< lang::XServiceInfo > > BackendImpl::loadFactories(
    OUString const & url, OUString const & loader )
{
    // HACK: highly dependent on stoc/source/servicemanager
    //       and stoc/source/implreg implementation which rely on the same
    //       services.rdb format!
    Reference< XComponentContext > const & xContext = getComponentContext();
    Reference< registry::XSimpleRegistry > xMemReg(
        xContext->getServiceManager()->createInstanceWithContext(
            OUSTR("com.sun.star.registry.SimpleRegistry"), xContext ),
        UNO_QUERY_THROW );
    xMemReg->open( OUString() /* in mem */, false, true );

    Reference< loader::XImplementationLoader > xLoader(
        xContext->getServiceManager()->createInstanceWithContext(
            loader, xContext ), UNO_QUERY_THROW );
    try
    {
        xLoader->writeRegistryInfo( xMemReg->getRootKey(), OUString(), url );
    }
    // ignore if factory cannot be registered:
    catch (registry::CannotRegisterImplementationException &)
    {
    }

    Sequence< Reference< registry::XRegistryKey > > keys(
        xMemReg->getRootKey()->openKeys() );
    ::std::vector< Reference< lang::XServiceInfo > > ret;
    ret.reserve( keys.getLength() );
    Reference< registry::XRegistryKey > const * pkeys = keys.getConstArray();
    for ( sal_Int32 pos = 0; pos < keys.getLength(); ++pos )
    {
        try
        {
            ret.push_back(
                Reference< lang::XServiceInfo >(
                    xLoader->activate( pkeys[ pos ]->getKeyName().copy(
                                           1 /* loading slash */ ),
                                       OUString(),
                                       url,
                                       pkeys[ pos ] ), UNO_QUERY_THROW ) );
        }
        // ignore if factory cannot be activated:
        catch (loader::CannotActivateFactoryException &)
        {
        }
    }
    return Sequence< Reference< lang::XServiceInfo > >( &ret[ 0 ], ret.size() );
}

// Package
//______________________________________________________________________________
ComponentPackageImpl::t_Registered ComponentPackageImpl::getRegStatus(
    ::osl::ResettableMutexGuard & guard,
    Reference< XCommandEnvironment > const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    if (m_registered == REG_VOID)
    {
        m_registered = REG_NOT_REGISTERED;
        // lookup rdb for location URL:
        if (m_inflated_url.getLength() == 0) // get an inflated URL:
            m_inflated_url = that->getInflatedCopy( m_url, xCmdEnv );

        Reference< registry::XRegistryKey > xRootKey(
            that->m_xServicesRDB->getRootKey() );
        Reference< registry::XRegistryKey > xImplKey(
            xRootKey->openKey( OUSTR("IMPLEMENTATIONS") ) );
        Sequence< OUString > implNames;
        if (xImplKey.is())
            implNames = xImplKey->getKeyNames();
        OUString const * pImplNames = implNames.getConstArray();
        for ( sal_Int32 pos = implNames.getLength(); pos--; )
        {
            OUString key( pImplNames[ pos ] + OUSTR("/UNO/LOCATION") );
            Reference< registry::XRegistryKey > xKey( xRootKey->openKey(key) );
            if (xKey.is() && xKey->isValid())
            {
                OUString location( xKey->getAsciiValue() );
                if (location.equalsIgnoreAsciiCase( m_inflated_url ))
                {
                    m_registered = REG_REGISTERED;
                    break;
                }
            }
        }
    }
    return m_registered;
}

//______________________________________________________________________________
bool ComponentPackageImpl::processPackage(
    bool register_package,
    ::osl::ResettableMutexGuard & guard,
    Reference< XCommandEnvironment > const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    that->ensure_no_running_office();

    if (m_inflated_url.getLength() == 0) // get an inflated URL:
        m_inflated_url = that->getInflatedCopy( m_url, xCmdEnv );
    bool java = m_loader.equalsAsciiL(
        RTL_CONSTASCII_STRINGPARAM("com.sun.star.loader.Java2") );

    bool success = true;
    ProgressLevel progress( xCmdEnv );
    that->unorc_verify_init( xCmdEnv ); // ensure written unorc file
    if (register_package)
    {
        // XXX todo: singletons live deployment

        // rdb part:
        progress.update(
            OUSTR("registering UNO component ") + m_inflated_url );
        OSL_ASSERT( that->m_xServicesRDB.is() );
        try
        {
            that->m_xImplReg->registerImplementation(
                m_loader, m_inflated_url /* loaders will expand */,
                that->m_xServicesRDB );
        }
        catch (registry::CannotRegisterImplementationException &)
        {
            success = false;
            // xxx todo: wait for MAV fix
            if (java) // workaround typelibs
                success &= that->addToUnoRc(
                    java, encodeForRcFile(m_inflated_url), xCmdEnv );
            else
                throw;
        }
        m_registered = REG_VOID;

//         // live insertion:
//         Sequence< Reference< lang::XServiceInfo > > factories(
//             that->loadFactories( m_inflated_url, m_loader ) );
//         Reference< container::XSet > xSet(
//             that->getComponentContext()->getServiceManager(), UNO_QUERY_THROW );
//         Reference< lang::XServiceInfo > const * pfactories =
//             factories.getConstArray();
//         for ( sal_Int32 pos = 0; pos < factories.getLength(); ++pos )
//         {
//             try
//             {
//                 xSet->insert( makeAny(pfactories[ pos ]) );
//             }
//             // ignore if factory has already benn inserted:
//             catch (container::ElementExistException &)
//             {
//                 OSL_ENSURE( 0, "### factory already registered?" );
//             }
//         }

        if (java && success) // xxx todo: add to CLASSPATH until we have an
                             // own extendable classloader, the sandbox
                             // classloader is insufficient, because the bridge
                             // and class com.sun.star.uno.Type load classes
                             // using a different classloader, e.g. the
                             // system classloader.
            success &= that->addToUnoRc(
                java, encodeForRcFile(m_inflated_url), xCmdEnv );

        if (success)
            m_registered = REG_REGISTERED;
    }
    else // revokePackage()
    {
        m_registered = REG_VOID;
        // rdb part:
        that->m_xImplReg->revokeImplementation(
            m_inflated_url /* loaders will expand */, that->m_xServicesRDB );

//         // live removal:
//         Sequence< Reference< lang::XServiceInfo > > factories(
//             that->loadFactories( m_inflated_url, m_loader ) );
//         Reference< container::XSet > xSet(
//             that->getComponentContext()->getServiceManager(), UNO_QUERY_THROW );
//         Reference< lang::XServiceInfo > const * pfactories =
//             factories.getConstArray();
//         for ( sal_Int32 pos = 0; pos < factories.getLength(); ++pos )
//         {
//             try
//             {
//                 xSet->remove(
//                     makeAny( pfactories[ pos ]->getImplementationName() ) );
//             }
//             // ignore if factory is not in:
//             catch (container::NoSuchElementException &)
//             {
//                 OSL_ENSURE( 0, "### registered factory not in?" );
//             }
//         }

        if (java) // xxx todo: add to CLASSPATH until we have an
                  // own extendable classloader, the sandbox
                  // classloader is insufficient, because the bridge
                  // and class com.sun.star.uno.Type load classes
                  // using a different classloader, e.g. the
                  // system classloader.
            success &= that->removeFromUnoRc(
                java, encodeForRcFile(m_inflated_url), xCmdEnv );

        that->removeInflatedCopy( m_url, xCmdEnv );

        if (success)
            m_registered = REG_NOT_REGISTERED;
    }

    return success;
}

//##############################################################################

// Package
//______________________________________________________________________________
TypelibraryPackageImpl::t_Registered TypelibraryPackageImpl::getRegStatus(
    ::osl::ResettableMutexGuard & guard,
    Reference< XCommandEnvironment > const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    if (m_inflated_unorc_url.getLength() == 0) // get an inflated URL:
        m_inflated_unorc_url = encodeForRcFile(
            that->getInflatedCopy( m_url, xCmdEnv ) );

    t_stringset const & rSet = that->getTypelibs(m_jarFile);
    return rSet.find(m_inflated_unorc_url) == rSet.end()
        ? REG_NOT_REGISTERED : REG_REGISTERED;
}

//______________________________________________________________________________
bool TypelibraryPackageImpl::processPackage(
    bool register_package,
    ::osl::ResettableMutexGuard & guard,
    Reference< XCommandEnvironment > const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    that->ensure_no_running_office();

    if (m_inflated_unorc_url.getLength() == 0) // get an inflated URL:
        m_inflated_unorc_url = encodeForRcFile(
            that->getInflatedCopy( m_url, xCmdEnv ) );

    bool success = true;
    m_registered = REG_VOID;
    if (register_package)
    {
        // xxx todo: check jar, rdb compatibility to existing types

        if (that->addToUnoRc( m_jarFile, m_inflated_unorc_url, xCmdEnv ))
        {
//             // live insertion:
//             if (m_jarFile)
//             {
//                 // xxx todo add to classpath at runtime:
//                 that->ensure_persistentMode();
//             }
//             else // RDB:
//             {
//                 Reference< XComponentContext > const & xContext =
//                     that->getComponentContext();
//                 if (! m_xTDprov.is())
//                 {
//                     m_xTDprov.set(
//                         xContext->getServiceManager()
//                         ->createInstanceWithArgumentsAndContext(
//                             OUSTR("com.sun.star.comp.stoc."
//                                   "RegistryTypeDescriptionProvider"),
//                             Sequence< Any >( &arg, 1 ), xContext ), UNO_QUERY );
//                     OSL_ASSERT( m_xTDprov.is() );
//                 }
//                 if (m_xTDprov.is())
//                 {
//                     Reference< container::XSet > xSet(
//                         xContext->getValueByName(
//                             OUSTR("/singletons/com.sun.star."
//                                   "reflection.theTypeDescriptionManager") ),
//                         UNO_QUERY_THROW );
//                     xSet->insert( makeAny(m_xTDprov) );
//                 }
//             }
        }
        m_registered = REG_REGISTERED;
    }
    else // revokePackage()
    {
        if (that->removeFromUnoRc( m_jarFile, m_inflated_unorc_url, xCmdEnv ))
        {
//             // revoke live:
//             if (m_jarFile)
//             {
//                 // xxx todo add to classpath at runtime:
//                 that->ensure_persistentMode();
//             }
//             else
//             {
//                 if (m_xTDprov.is())
//                 {
//                     Reference< container::XSet > xSet(
//                         xContext->getValueByName(
//                             OUSTR("/singletons/com.sun.star."
//                                   "reflection.theTypeDescriptionManager") ),
//                         UNO_QUERY_THROW );
//                     xSet->remove( makeAny(m_xTDprov) );
//                 }
//             }
        }
        else
            success = false;
        m_registered = REG_NOT_REGISTERED;
        that->removeInflatedCopy( m_url, xCmdEnv );
    }
    return success;
}

} // namespace component
} // namespace backend
} // namespace dp_registry

