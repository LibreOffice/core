/*************************************************************************
 *
 *  $RCSfile: dp_sfwk.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:12:08 $
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
#include "dp_sfwk.hrc"
#include "dp_backend.h"
#include "dp_ucb.h"
#include "dp_parceldesc.hxx"
#include "rtl/uri.hxx"
#include "ucbhelper/content.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "svtools/inettype.hxx"
#include <com/sun/star/container/XNameContainer.hpp>
#include <drafts/com/sun/star/script/provider/XScriptProviderFactory.hpp>
#include <memory>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::drafts::com::sun::star::script;

using ::rtl::OUString;
namespace css = ::com::sun::star;

namespace dp_registry
{
namespace backend
{
namespace sfwk
{

//==============================================================================
class BackendImpl : public ::dp_registry::backend::PackageRegistryBackend
{
private:
    OUString m_sCtx; //TODO needs an accessor or needs to be passed as part of create for Package
protected:
    // PackageRegistryBackend
    virtual Reference<deployment::XPackage> bindPackage_(
        OUString const & url, OUString const & mediaType,
        Reference<XCommandEnvironment> const & xCmdEnv );

public:
    BackendImpl(
        Sequence<Any> const & args,
        Reference<XComponentContext> const & xComponentContext,
        OUString const & implName,
        Sequence<OUString> const & supportedMediaTypes );

    OUString getCtx() { return m_sCtx; }
};

//==============================================================================
class PackageImpl : public ::dp_registry::backend::Package
{
protected:
    Reference< container::XNameContainer > m_xNameCntrPkgHandler;

    inline BackendImpl * getMyBackend() const
        { return static_cast<BackendImpl *>(m_myBackend.get()); }
    void initPackageHandler();
    // Package
    virtual beans::Optional< beans::Ambiguous<sal_Bool> > isRegistered_(
        ::osl::ResettableMutexGuard & guard,
        ::rtl::Reference<AbortChannel> const & abortChannel,
        Reference<XCommandEnvironment> const & xCmdEnv );
    virtual void processPackage_(
        ::osl::ResettableMutexGuard & guard,
        bool registerPackage,
        ::rtl::Reference<AbortChannel> const & abortChannel,
        Reference<XCommandEnvironment> const & xCmdEnv );

    inline PackageImpl(
        ::rtl::Reference<BackendImpl> const & myBackend,
        OUString const & url, OUString const & libType,
        OUString const & mediaType )
        : Package( myBackend.get(), url, mediaType,
                   OUString(), OUString(), // will be late-initialized
           OUString( libType ) )
        { initPackageHandler(); }

public:
    static PackageImpl * create(
        ::rtl::Reference<BackendImpl> const & myBackend,
        OUString const & url,
        OUString const & mediaType,
        Reference<XCommandEnvironment> const &xCmdEnv, OUString const & libType );

    // XPackage
    virtual Any SAL_CALL getIcon( sal_Bool highContrast, sal_Bool smallIcon )
        throw (RuntimeException);
};

//______________________________________________________________________________
PackageImpl * PackageImpl::create(
    ::rtl::Reference<BackendImpl> const & myBackend,
    OUString const & url,
    OUString const & mediaType,
    Reference<XCommandEnvironment> const &xCmdEnv,
    OUString const & libType  )
{
    ::std::auto_ptr<PackageImpl> ret(
        new PackageImpl( myBackend, url, libType, mediaType ) );

    // name, displayName:
    // name and display name default the same
    ret->m_displayName = url.copy( url.lastIndexOf( '/' ) + 1 );
    ret->m_name = ret->m_displayName;

    OSL_TRACE("PakageImpl displayName is %s",
        ::rtl::OUStringToOString( ret->m_displayName , RTL_TEXTENCODING_ASCII_US ).pData->buffer );

    return ret.release();
}

//______________________________________________________________________________
BackendImpl::BackendImpl(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xComponentContext,
    OUString const & implName,
    Sequence<OUString> const & supportedMediaTypes )
    : PackageRegistryBackend(
        args, xComponentContext, implName, supportedMediaTypes )
{
    if (! transientMode())
    {
/*
        if (office_is_running())
        {
            Reference<XComponentContext> xContext( getComponentContext() );
            m_xScriptLibs.set(
                xContext->getServiceManager()->createInstanceWithContext(
                    OUSTR("com.sun.star."
                          "script.ApplicationScriptLibraryContainer"),
                    xContext ), UNO_QUERY_THROW );
            m_xDialogLibs.set(
                xContext->getServiceManager()->createInstanceWithContext(
                    OUSTR("com.sun.star."
                          "script.ApplicationDialogLibraryContainer"),
                    xContext ), UNO_QUERY_THROW );
        }
        else
        {
            OUString basic_path(
                m_eContext == CONTEXT_USER
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
*/
        OUString ctx(
            m_eContext == CONTEXT_USER
                ? OUSTR("user") : OUSTR("share") );
        m_sCtx = ctx;
    }
}

//==============================================================================
OUString SAL_CALL getImplementationName()
{
    return OUSTR("com.sun.star.comp.deployment.sfwk.PackageRegistryBackend");
}

//==============================================================================
Reference<XInterface> SAL_CALL create(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xComponentContext )
    SAL_THROW( (Exception) )
{
    OUString const mediaTypes [] = {
        OUSTR("application/vnd.sun.star.framework-script")
    };
    return static_cast< ::cppu::OWeakObject * >(
        new BackendImpl(
            args, xComponentContext, getImplementationName(),
            Sequence<OUString >( mediaTypes, ARLEN(mediaTypes) ) ) );
}

// PackageRegistryBackend
//______________________________________________________________________________
Reference<deployment::XPackage> BackendImpl::bindPackage_(
    OUString const & url, OUString const & mediaType_,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    OUString mediaType( mediaType_ );
    if (mediaType.getLength() == 0)
    {
        // detect media-type:
        ::ucb::Content ucbContent;
        if (create_ucb_content( &ucbContent, url, xCmdEnv ) &&
            ucbContent.isFolder())
        {
            // probe for script.xlb:
            if (create_ucb_content(
                    0, make_url( url, OUSTR("parcel-descriptor.xml") ),
                    xCmdEnv, false /* no throw */ ))
        {
                mediaType = OUSTR("application/vnd.sun.star.framework-script");

        }
        }
        if (mediaType.getLength() == 0)
            throw lang::IllegalArgumentException(
                m_strCannotDetectMediaType + url,
                static_cast<OWeakObject *>(this), static_cast<sal_Int16>(-1) );
    }

    String type, subType;
    INetContentTypeParameterList params;
    if (INetContentTypes::parse( mediaType, type, subType, &params ))
    {
        if (type.EqualsIgnoreCaseAscii("application"))
        {
            if (subType.EqualsIgnoreCaseAscii("vnd.sun.star.framework-script"))
            {
                OUString sParcelDescURL = make_url( url, OUSTR("parcel-descriptor.xml") );
                ::ucb::Content ucb_content( sParcelDescURL, xCmdEnv );
                ParcelDescDocHandler* pHandler =
                    new ParcelDescDocHandler();
                Reference< xml::sax::XDocumentHandler > xDocHandler =  pHandler;
                Reference<XComponentContext> xContext( getComponentContext() );
                Reference< xml::sax::XParser > xParser(
                    xContext->getServiceManager()->createInstanceWithContext(
                        OUSTR("com.sun.star.xml.sax.Parser"), xContext ),
                            UNO_QUERY_THROW );
                xParser->setDocumentHandler( xDocHandler );
                xml::sax::InputSource source;
                source.aInputStream = ucb_content.openStream();
                source.sSystemId = ucb_content.getURL();
                xParser->parseStream( source );
                OUString lang;

                if ( !pHandler->isParsed() )
                {
                    throw lang::IllegalArgumentException(
                        m_strCannotDetectMediaType + url,
                        static_cast<OWeakObject *>(this), static_cast<sal_Int16>(-1) );
                }
                lang =  pHandler->getParcelLanguage();
                OUString sfwkLibType = getResourceString( RID_STR_SFWK_LIB );
                // replace %MACRONAME placeholder with language name
                OUString MACRONAME( OUSTR("%MACROLANG" ) );
                sal_Int32 startOfReplace = sfwkLibType.indexOf( MACRONAME );
                sal_Int32 charsToReplace = MACRONAME.getLength();
                sfwkLibType = sfwkLibType.replaceAt( startOfReplace, charsToReplace, lang );
                OSL_TRACE("******************************");
                OSL_TRACE(" BackEnd detected lang = %s  ",
                     OUStringToOString( lang, RTL_TEXTENCODING_ASCII_US ).getStr() );
                OSL_TRACE(" for url %s",
                     OUStringToOString( source.sSystemId, RTL_TEXTENCODING_ASCII_US ).getStr() );
                OSL_TRACE("******************************");
                return PackageImpl::create(
                    this, url, mediaType, xCmdEnv, sfwkLibType );
            }
        }
    }
    throw lang::IllegalArgumentException(
        m_strUnsupportedMediaType + mediaType,
        static_cast<OWeakObject *>(this),
        static_cast<sal_Int16>(-1) );
}

//##############################################################################

// XPackage
//______________________________________________________________________________

void PackageImpl:: initPackageHandler()
{
    BackendImpl * that = getMyBackend();
    Any aContext;

    if ( that->getCtx().equals(  OUSTR("user") ) )
    {
        aContext  <<= OUSTR("user");

    }
    else if ( that->getCtx().equals( OUSTR( "share" ) ) )
    {
        aContext  <<= OUSTR("share");
    }
    else
    {
        // NOT supported at the momemtn // TODO
    }

    Reference< provider::XScriptProviderFactory > xFac(
        that->getComponentContext()->getValueByName(
            OUSTR( "/singletons/drafts.com.sun.star.script.provider.theMasterScriptProviderFactory") ), UNO_QUERY );

    if ( xFac.is() )
    {
        Reference< container::XNameContainer > xName( xFac->createScriptProvider( aContext ), UNO_QUERY );
        if ( xName.is() )
        {
            m_xNameCntrPkgHandler.set( xName );
        }
    }
    // TODO what happens if above fails??
}

Any PackageImpl::getIcon( sal_Bool highContrast, sal_Bool smallIcon )
    throw (RuntimeException)
{
    OSL_ASSERT( smallIcon );
    if (smallIcon)
    {
        sal_uInt16 ret = highContrast ? RID_IMG_SCRIPTLIB_HC : RID_IMG_SCRIPTLIB;
        return makeAny(ret);
    }
    return Package::getIcon( highContrast, smallIcon );
}

// Package
//______________________________________________________________________________
beans::Optional< beans::Ambiguous<sal_Bool> > PackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard & guard,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    return beans::Optional< beans::Ambiguous<sal_Bool> >(
        true /* IsPresent */,
        beans::Ambiguous<sal_Bool>(
            m_xNameCntrPkgHandler.is() && m_xNameCntrPkgHandler->hasByName(
                m_url ),
            false /* IsAmbiguous */ ) );
}

//______________________________________________________________________________
void PackageImpl::processPackage_(
    ::osl::ResettableMutexGuard & guard,
    bool registerPackage,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();

    if ( !m_xNameCntrPkgHandler.is() )
    {
        OSL_TRACE("no package handler!!!!");
        throw RuntimeException( OUSTR("No package Handler " ),
            Reference< XInterface >() );
    }

    if (registerPackage)
    {
        // will throw if it fails
        m_xNameCntrPkgHandler->insertByName( m_url, makeAny( Reference< XPackage >(this) ) );

    }
    else // revokePackage()
    {
        m_xNameCntrPkgHandler->removeByName( m_url );
    }
}

} // namespace sfwk
} // namespace backend
} // namespace dp_registry

