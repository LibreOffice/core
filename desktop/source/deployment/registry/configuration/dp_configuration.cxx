/*************************************************************************
 *
 *  $RCSfile: dp_configuration.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-04-13 12:08:51 $
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
#include "dp_persmap.h"
#include "dp_ucb.h"
#include "dp_xml.h"
#include "rtl/string.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/uri.hxx"
#include "rtl/memory.h"
#include "osl/file.hxx"
#include "ucbhelper/content.hxx"
#include "xmlscript/xml_helper.hxx"
#include "com/sun/star/ucb/NameClash.hpp"
#include "com/sun/star/io/XActiveDataSink.hpp"
#include "com/sun/star/configuration/backend/XLayerImporter.hpp"
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
namespace configuration
{

//==============================================================================
class BackendImpl : public ::dp_registry::backend::PackageRegistryBackend
{
    Reference< ::com::sun::star::configuration::backend::XLayerImporter >
    m_xMergeImporter;
    OUString m_config_layer;

protected:
    // PackageRegistryBackend
    virtual Reference< deployment::XPackage > createPackage(
        OUString const & url,
        OUString const & mediaType, OUString const & subType,
        INetContentTypeParameterList const & params,
        Reference< XCommandEnvironment > const & xCmdEnv );

public:
    void xcu_merge_in( OUString const & url,
                       Reference< XCommandEnvironment > const & xCmdEnv );
    void xcs_merge_in( OUString const & url,
                       Reference< XCommandEnvironment > const & xCmdEnv );
    ::std::auto_ptr< PersistentMap > m_registeredPackages;
    OUString getConfigLayer();

    inline BackendImpl(
        Reference< XComponentContext > const & xComponentContext,
        OUString const & implName,
        Sequence< OUString > const & supported_media_types )
        : PackageRegistryBackend(
            xComponentContext, implName, supported_media_types )
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
    t_Registered m_registered;
    bool m_isSchema;

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
    inline PackageImpl(
        ::rtl::Reference< PackageRegistryBackend > const & myBackend,
        OUString const & url, OUString const & mediaType,
        OUString const & name,
        bool isSchema )
        : Package( myBackend, url, mediaType, name, name /* display-name */,
                   isSchema
                   ? OUSTR("Configuration Schema File")
                   : OUSTR("Configuration Data File") ),
          m_registered( REG_VOID ),
          m_isSchema( isSchema )
        {}
};

//==============================================================================
OUString SAL_CALL getImplementationName()
{
    return OUSTR(
        "com.sun.star.comp.deployment.configuration.PackageRegistryBackend");
}

//==============================================================================
Reference< XInterface > SAL_CALL create(
    Reference< XComponentContext > const & xComponentContext )
    SAL_THROW( (Exception) )
{
    OUString const media_types [] = {
        OUSTR("application/vnd.sun.star.configuration-data"),
        OUSTR("application/vnd.sun.star.configuration-schema")
    };
    return static_cast< ::cppu::OWeakObject * >(
        new BackendImpl(
            xComponentContext, getImplementationName(),
            Sequence< OUString >( media_types, ARLEN(media_types) ) ) );
}

//______________________________________________________________________________
void BackendImpl::initialize( Sequence< Any > const & args )
    throw (RuntimeException)
{
    PackageRegistryBackend::initialize( args );

    if (! transientMode())
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
                RTL_CONSTASCII_STRINGPARAM(".xcu") ))
            return OUSTR("application/vnd.sun.star.configuration-data");
        if (title.endsWithIgnoreAsciiCaseAsciiL(
                RTL_CONSTASCII_STRINGPARAM(".xcs") ))
            return OUSTR("application/vnd.sun.star.configuration-schema");
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

    if (subType.matchIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.configuration-data") ))
    {
        return new PackageImpl(
            this, url, mediaType, name, false /* data file */ );
    }
    if (subType.matchIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.configuration-schema") ))
    {
        return new PackageImpl(
            this, url, mediaType, name, true /* schema file */ );
    }
    throw lang::IllegalArgumentException(
        OUSTR("invalid media-type given: ") + mediaType,
        static_cast< OWeakObject * >(this),
        static_cast< sal_Int16 >(-1 /* not known */) );
}

// Package
//______________________________________________________________________________
PackageImpl::t_Registered PackageImpl::getRegStatus(
    ::osl::ResettableMutexGuard & guard,
    Reference< XCommandEnvironment > const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    that->ensure_no_running_office();
    that->ensure_persistentMode();
    return that->m_registeredPackages->has( m_url )
        ? REG_REGISTERED : REG_NOT_REGISTERED;
}

//##############################################################################

//______________________________________________________________________________
OUString BackendImpl::getConfigLayer()
{
    if (m_config_layer.getLength() == 0)
    {
        OUString layer( expand_reg_url(
                            make_url( getCachePath(), OUSTR("registry") ),
                            getComponentContext() ) );
        ::osl::FileBase::RC rc = ::osl::File::getAbsoluteFileURL(
            OUString(), layer, m_config_layer );
        if (rc != ::osl::FileBase::E_None)
            throw RuntimeException(
                OUSTR("makeing file URL absolute failed: ") + layer,
                static_cast< OWeakObject * >(this) );
        create_folder( 0, m_config_layer );
    }
    return m_config_layer;
}

//==============================================================================
class SchemaFileRoot : public ::dp_misc::XmlRootElement
{
public:
    OUString m_name;
    OUString m_package;

    inline SchemaFileRoot()
        : XmlRootElement( OUSTR("http://openoffice.org/2001/registry"),
                          OUSTR("component-schema") )
        {}

    // XRoot
    virtual Reference< xml::input::XElement > SAL_CALL startRootElement(
        sal_Int32 uid, OUString const & localname,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
};

//______________________________________________________________________________
Reference< xml::input::XElement > SchemaFileRoot::startRootElement(
    sal_Int32 uid, OUString const & localname,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // check root element:
    XmlRootElement::startRootElement( uid, localname, xAttributes );

    // "name" attribute
    m_name = xAttributes->getValueByUidName( getUid(), OUSTR("name") );
    if (m_name.getLength() == 0)
    {
        throw xml::sax::SAXException(
            OUSTR("missing schema name attribute!"),
            static_cast< OWeakObject * >(this), Any() );
    }
    // "package" attribute
    m_package = xAttributes->getValueByUidName( getUid(), OUSTR("package") );
    if (m_package.getLength() == 0)
    {
        throw xml::sax::SAXException(
            OUSTR("missing schema package attribute!"),
            static_cast< OWeakObject * >(this), Any() );
    }

    // don't go deeper...
    return Reference< xml::input::XElement >();
}

//______________________________________________________________________________
void BackendImpl::xcs_merge_in(
    OUString const & url, Reference< XCommandEnvironment > const & xCmdEnv )
{
    // parse out schema package:
    SchemaFileRoot * root = new SchemaFileRoot;
    Reference< xml::input::XRoot > xRoot( root );
    ::ucb::Content ucb_content( url, xCmdEnv );
    xml_parse( xRoot, ucb_content, getComponentContext() );

    OUString dest_folder(
        make_url( getConfigLayer(), OUSTR("schema/") + ::rtl::Uri::encode(
                      root->m_package, rtl_UriCharClassPchar,
                      rtl_UriEncodeIgnoreEscapes,
                      RTL_TEXTENCODING_UTF8 ).replace( '.', '/' ) ) );
    OUString title( root->m_name + OUSTR(".xcs") );
    OUString dest_url( make_url( dest_folder, ::rtl::Uri::encode(
                                     title, rtl_UriCharClassPchar,
                                     rtl_UriEncodeIgnoreEscapes,
                                     RTL_TEXTENCODING_UTF8 ) ) );
    ProgressLevel progress( xCmdEnv );
    if (create_ucb_content( 0, dest_url, xCmdEnv, false /* not throw */ ))
    {
        progress.update(
            OUSTR("schema file already exists in layer: ") + dest_url );
    }
    else
    {
        progress.update( OUSTR("copying schema file ") + url );
        // assure folder is existing:
        ::ucb::Content ucb_dest_folder;
        create_folder( &ucb_dest_folder, dest_folder );
        // ought to give no error, because destination file does not exist
        if (! ucb_dest_folder.transferContent(
                ::ucb::Content( url, xCmdEnv ),
                ::ucb::InsertOperation_COPY,
                title, NameClash::OVERWRITE ))
            throw RuntimeException(
                OUSTR("::ucb::Content::transferContent() failed!"), 0 );
    }
}

//##############################################################################

//------------------------------------------------------------------------------
static OUString encodeForXml( OUString const & text )
{
    // encode conforming xml:
    sal_Int32 len = text.getLength();
    ::rtl::OUStringBuffer buf;
    for ( sal_Int32 pos = 0; pos < len; ++pos )
    {
        sal_Unicode c = text[ pos ];
        switch (c)
        {
        case '<':
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("&lt;") );
            break;
        case '>':
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("&gt;") );
            break;
        case '&':
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("&amp;") );
            break;
        case '\'':
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("&apos;") );
            break;
        case '\"':
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("&quot;") );
            break;
        default:
            buf.append( c );
            break;
        }
    }
    return buf.makeStringAndClear();
}

//______________________________________________________________________________
void BackendImpl::xcu_merge_in(
    OUString const & url, Reference< XCommandEnvironment > const & xCmdEnv )
{
    ProgressLevel progress( xCmdEnv,
                            OUSTR("merging in configuration-data ") + url );

    // looking for %origin%:
    ::ucb::Content ucb_content( url, xCmdEnv );
    ::rtl::ByteSequence bytes( readFile( ucb_content ) );
    ::rtl::ByteSequence filtered( bytes.getLength() * 2,
                                  ::rtl::BYTESEQ_NODEFAULT );
    bool use_filtered = false;
    ::rtl::OString origin;
    sal_Char const * pBytes = reinterpret_cast< sal_Char const * >(
        bytes.getConstArray());
    sal_Size nBytes = bytes.getLength();
    sal_Int32 write_pos = 0;
    while (nBytes > 0)
    {
        sal_Int32 index = rtl_str_indexOfChar_WithLength( pBytes, nBytes, '%' );
        if (index < 0)
        {
            if (! use_filtered) // opt
                break;
            index = nBytes;
        }

        if ((write_pos + index) > filtered.getLength())
            filtered.realloc( (filtered.getLength() + index) * 2 );
        rtl_copyMemory( filtered.getArray() + write_pos, pBytes, index );
        write_pos += index;
        pBytes += index;
        nBytes -= index;
        if (nBytes == 0)
            break;

        // consume %:
        ++pBytes;
        --nBytes;
        sal_Char const * pAdd = "%";
        sal_Int32 nAdd = 1;
        if (nBytes > 1 && pBytes[ 0 ] == '%')
        {
            // %% => %
            ++pBytes;
            --nBytes;
            use_filtered = true;
        }
        else if (rtl_str_shortenedCompare_WithLength(
                     pBytes, nBytes,
                     RTL_CONSTASCII_STRINGPARAM("origin%"),
                     sizeof ("origin%") -1 ) == 0)
        {
            if (origin.getLength() == 0) // encode only once
            {
                origin = ::rtl::OUStringToOString(
                    encodeForXml( url.copy( 0, url.lastIndexOf( '/' ) ) ),
                    // xxx todo: encode always for UTF-8? => lookup doc-header?
                    RTL_TEXTENCODING_UTF8 );
            }
            pAdd = origin.getStr();
            nAdd = origin.getLength();
            pBytes += (sizeof ("origin%") -1);
            nBytes -= (sizeof ("origin%") -1);
            use_filtered = true;
        }
        if ((write_pos + nAdd) > filtered.getLength())
            filtered.realloc( (filtered.getLength() + nAdd) * 2 );
        rtl_copyMemory( filtered.getArray() + write_pos, pAdd, nAdd );
        write_pos += nAdd;
    }
    if (use_filtered && write_pos < filtered.getLength())
        filtered.realloc( write_pos );

    Reference< XComponentContext > const & xContext = getComponentContext();
    if (! m_xMergeImporter.is())
    {
        m_xMergeImporter.set(
            xContext->getServiceManager()->createInstanceWithContext(
                OUSTR("com.sun.star.configuration.backend.MergeImporter"),
                xContext ), UNO_QUERY_THROW );
    }

    Reference< ::com::sun::star::configuration::backend::XLayer > xLayer(
        xContext->getServiceManager()->createInstanceWithContext(
            OUSTR("com.sun.star.configuration.backend.xml.LayerParser"),
            xContext ), UNO_QUERY_THROW );

    Reference< io::XActiveDataSink > xActiveDataSink( xLayer, UNO_QUERY_THROW );
    if (use_filtered)
        xActiveDataSink->setInputStream( ::xmlscript::createInputStream(
                                             filtered ) );
    else
        ucb_content.openStream( xActiveDataSink );
    m_xMergeImporter->importLayerForEntity( xLayer, getConfigLayer() );
}

//______________________________________________________________________________
bool PackageImpl::processPackage(
    bool register_package,
    ::osl::ResettableMutexGuard & guard,
    Reference< XCommandEnvironment > const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    that->ensure_no_running_office();
    that->ensure_persistentMode();

    m_registered = REG_VOID;
    if (register_package)
    {
        if (m_isSchema)
        {
            that->xcs_merge_in( m_url, xCmdEnv );
            OUString insertion = OUSTR("vnd.sun.star.configuration-schema");
            that->m_registeredPackages->put(
                m_url, insertion );
        }
        else
        {
            that->xcu_merge_in( m_url, xCmdEnv );
            OUString insertion = OUSTR("vnd.sun.star.configuration-data");
            that->m_registeredPackages->put(
                m_url, insertion );
        }
        m_registered = REG_REGISTERED;
    }
    else
    {
        // xxx todo: guarding
//         ::osl::MutexGuard guard( that->getMutex() );
        OSL_ASSERT( that->m_registeredPackages->has( m_url ) );
        if (that->m_registeredPackages->erase( m_url ))
        {
            t_string2string_map entries(
                that->m_registeredPackages->getEntries() );
            t_string2string_map::const_iterator iPos( entries.begin() );
            t_string2string_map::const_iterator const iEnd( entries.end() );
            if (m_isSchema)
            {
                // reset config schema layer:
                erase_path( make_url( that->getConfigLayer(), OUSTR("schema") ),
                            xCmdEnv );
                for ( ; iPos != iEnd; ++iPos )
                {
                    if (iPos->second.equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM(
                                "vnd.sun.star.configuration-schema") ))
                        that->xcs_merge_in( iPos->first, xCmdEnv );
                }
            }
            else
            {
                // reset config data layer:
                erase_path( make_url( that->getConfigLayer(), OUSTR("data") ),
                            xCmdEnv );
                for ( ; iPos != iEnd; ++iPos )
                {
                    if (iPos->second.equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM(
                                "vnd.sun.star.configuration-data") ))
                        that->xcu_merge_in( iPos->first, xCmdEnv );
                }
            }
            // xxx todo: error handling in case of thrown exceptions...
            // => inconsistent register_packages.db!
        }
        m_registered = REG_NOT_REGISTERED;
    }

    return true;
}

} // namespace configuration
} // namespace backend
} // namespace dp_registry

