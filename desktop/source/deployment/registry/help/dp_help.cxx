/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "dp_help.hrc"
#include "dp_backend.h"
#include "dp_helpbackenddb.hxx"
#include "dp_ucb.h"
#include "rtl/uri.hxx"
#include "osl/file.hxx"
#include "rtl/bootstrap.hxx"
#include "ucbhelper/content.hxx"
#include "comphelper/servicedecl.hxx"
#include "svl/inettype.hxx"
#include "uno/current_context.hxx"
#include "unotools/pathoptions.hxx"

#if !defined(ANDROID) && !defined(IOS)
#include <helpcompiler/compilehelp.hxx>
#include <helpcompiler/HelpIndexer.hxx>
#endif
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess2.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarExpandUrl.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include "boost/optional.hpp"

using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace dp_registry {
namespace backend {
namespace help {
namespace {

//==============================================================================
class BackendImpl : public ::dp_registry::backend::PackageRegistryBackend
{
    class PackageImpl : public ::dp_registry::backend::Package
    {
        BackendImpl * getMyBackend() const;

        // Package
        virtual beans::Optional< beans::Ambiguous<sal_Bool> > isRegistered_(
            ::osl::ResettableMutexGuard & guard,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv );
        virtual void processPackage_(
            ::osl::ResettableMutexGuard & guard,
            bool registerPackage,
            bool startup,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv );


    public:
        PackageImpl(
            ::rtl::Reference<PackageRegistryBackend> const & myBackend,
            OUString const & url, OUString const & name,
            Reference<deployment::XPackageTypeInfo> const & xPackageType,
            bool bRemoved, OUString const & identifier);

        bool extensionContainsCompiledHelp();

        //XPackage
        virtual css::beans::Optional< ::rtl::OUString > SAL_CALL getRegistrationDataURL()
            throw (deployment::ExtensionRemovedException, css::uno::RuntimeException);
    };
    friend class PackageImpl;

    // PackageRegistryBackend
    virtual Reference<deployment::XPackage> bindPackage_(
        OUString const & url, OUString const & mediaType,
        sal_Bool bRemoved, OUString const & identifier,
        Reference<XCommandEnvironment> const & xCmdEnv );

    void implProcessHelp( PackageImpl * package, bool doRegisterPackage,
                          Reference<ucb::XCommandEnvironment> const & xCmdEnv);
    void implCollectXhpFiles( const rtl::OUString& aDir,
        std::vector< rtl::OUString >& o_rXhpFileVector );

    void addDataToDb(OUString const & url, HelpBackendDb::Data const & data);
    ::boost::optional<HelpBackendDb::Data> readDataFromDb(OUString const & url);
    bool hasActiveEntry(OUString const & url);
    void revokeEntryFromDb(OUString const & url);
    bool activateEntry(OUString const & url);

    Reference< ucb::XSimpleFileAccess2 > getFileAccess( void );
    Reference< ucb::XSimpleFileAccess2 > m_xSFA;

    const Reference<deployment::XPackageTypeInfo> m_xHelpTypeInfo;
    Sequence< Reference<deployment::XPackageTypeInfo> > m_typeInfos;
    std::auto_ptr<HelpBackendDb> m_backendDb;

public:
    BackendImpl( Sequence<Any> const & args,
                 Reference<XComponentContext> const & xComponentContext );

    // XPackageRegistry
    virtual Sequence< Reference<deployment::XPackageTypeInfo> > SAL_CALL
        getSupportedPackageTypes() throw (RuntimeException);
    virtual void SAL_CALL packageRemoved(OUString const & url, OUString const & mediaType)
        throw (deployment::DeploymentException,
               uno::RuntimeException);

};

//______________________________________________________________________________
BackendImpl::BackendImpl(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xComponentContext )
    : PackageRegistryBackend( args, xComponentContext ),
      m_xHelpTypeInfo( new Package::TypeInfo(
                               OUSTR("application/vnd.sun.star.help"),
                               rtl::OUString(),
                               getResourceString(RID_STR_HELP),
                               RID_IMG_HELP ) ),
      m_typeInfos( 1 )
{
    m_typeInfos[ 0 ] = m_xHelpTypeInfo;
    if (!transientMode())
    {
        OUString dbFile = makeURL(getCachePath(), OUSTR("backenddb.xml"));
        m_backendDb.reset(
            new HelpBackendDb(getComponentContext(), dbFile));

        //clean up data folders which are no longer used.
        //This must not be done in the same process where the help files
        //are still registers. Only after revoking and restarting OOo the folders
        //can be removed. This works now, because the extension manager is a singleton
        //and the backends are only create once per process.
        ::std::list<OUString> folders = m_backendDb->getAllDataUrls();
        deleteUnusedFolders(OUString(), folders);
   }
}

// XPackageRegistry
//______________________________________________________________________________
Sequence< Reference<deployment::XPackageTypeInfo> >
BackendImpl::getSupportedPackageTypes() throw (RuntimeException)
{
    return m_typeInfos;
}

void BackendImpl::packageRemoved(OUString const & url, OUString const & /*mediaType*/)
        throw (deployment::DeploymentException,
               uno::RuntimeException)
{
    if (m_backendDb.get())
        m_backendDb->removeEntry(url);
}

// PackageRegistryBackend
//______________________________________________________________________________
Reference<deployment::XPackage> BackendImpl::bindPackage_(
    OUString const & url, OUString const & mediaType_,
    sal_Bool bRemoved, OUString const & identifier,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    // we don't support auto detection:
    if (mediaType_.isEmpty())
        throw lang::IllegalArgumentException(
            StrCannotDetectMediaType::get() + url,
            static_cast<OWeakObject *>(this), static_cast<sal_Int16>(-1) );

    String type, subType;
    INetContentTypeParameterList params;
    if (INetContentTypes::parse( mediaType_, type, subType, &params ))
    {
        if (type.EqualsIgnoreCaseAscii("application"))
        {
            OUString name;
            if (!bRemoved)
            {
                ::ucbhelper::Content ucbContent(
                    url, xCmdEnv, getComponentContext() );
                name = StrTitle::getTitle( ucbContent );
            }

            if (subType.EqualsIgnoreCaseAscii(
                    "vnd.sun.star.help"))
            {
                return new PackageImpl(
                    this, url, name, m_xHelpTypeInfo, bRemoved,
                    identifier);
            }
        }
    }
    throw lang::IllegalArgumentException(
        StrUnsupportedMediaType::get() + mediaType_,
        static_cast<OWeakObject *>(this),
        static_cast<sal_Int16>(-1) );
}

void BackendImpl::addDataToDb(
    OUString const & url, HelpBackendDb::Data const & data)
{
    if (m_backendDb.get())
        m_backendDb->addEntry(url, data);
}

::boost::optional<HelpBackendDb::Data> BackendImpl::readDataFromDb(
    OUString const & url)
{
    ::boost::optional<HelpBackendDb::Data> data;
    if (m_backendDb.get())
        data = m_backendDb->getEntry(url);
    return data;
}

bool BackendImpl::hasActiveEntry(OUString const & url)
{
    if (m_backendDb.get())
        return m_backendDb->hasActiveEntry(url);
    return false;
}

void BackendImpl::revokeEntryFromDb(OUString const & url)
{
    if (m_backendDb.get())
        m_backendDb->revokeEntry(url);
}

bool BackendImpl::activateEntry(OUString const & url)
{
    if (m_backendDb.get())
        return m_backendDb->activateEntry(url);
    return false;
}


BackendImpl::PackageImpl::PackageImpl(
    ::rtl::Reference<PackageRegistryBackend> const & myBackend,
    OUString const & url, OUString const & name,
    Reference<deployment::XPackageTypeInfo> const & xPackageType,
    bool bRemoved, OUString const & identifier)
    : Package( myBackend, url, name, name, xPackageType, bRemoved,
               identifier)
{
}

// Package
BackendImpl * BackendImpl::PackageImpl::getMyBackend() const
{
    BackendImpl * pBackend = static_cast<BackendImpl *>(m_myBackend.get());
    if (NULL == pBackend)
    {
        //May throw a DisposedException
        check();
        //We should never get here...
        throw RuntimeException(
            OUSTR("Failed to get the BackendImpl"),
            static_cast<OWeakObject*>(const_cast<PackageImpl *>(this)));
    }
    return pBackend;
}

bool BackendImpl::PackageImpl::extensionContainsCompiledHelp()
{
    bool bCompiled = true;
    rtl::OUString aExpandedHelpURL = dp_misc::expandUnoRcUrl(getURL());

    ::osl::Directory helpFolder(aExpandedHelpURL);
    if ( helpFolder.open() == ::osl::File::E_None)
    {
        //iterate over the contents of the help folder
        //We assume that all folders withing the help folder contain language specific
        //help files. If just one of them does not contain compiled help then this
        //function returns false.
        ::osl::DirectoryItem item;
        ::osl::File::RC errorNext = ::osl::File::E_None;
        while ((errorNext = helpFolder.getNextItem(item)) == ::osl::File::E_None)
        {
            //No find the language folders
            ::osl::FileStatus stat(osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileName |osl_FileStatus_Mask_FileURL);
            if (item.getFileStatus(stat) == ::osl::File::E_None)
            {
                if (stat.getFileType() != ::osl::FileStatus::Directory)
                    continue;

                //look if there is the folder help.idxl in the language folder
                OUString compUrl(stat.getFileURL() + OUSTR("/help.idxl"));
                ::osl::Directory compiledFolder(compUrl);
                if (compiledFolder.open() != ::osl::File::E_None)
                {
                    bCompiled = false;
                    break;
                }
            }
            else
            {
                //Error
                OSL_ASSERT(0);
                bCompiled = false;
                break;
            }
        }
        if (errorNext != ::osl::File::E_NOENT
            && errorNext != ::osl::File::E_None)
        {
            //Error
            OSL_ASSERT(0);
            bCompiled = false;
        }
    }
    return bCompiled;
}

//______________________________________________________________________________
beans::Optional< beans::Ambiguous<sal_Bool> >
BackendImpl::PackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard &,
    ::rtl::Reference<AbortChannel> const &,
    Reference<XCommandEnvironment> const & )
{
    BackendImpl * that = getMyBackend();

    bool bReg = false;
    if (that->hasActiveEntry(getURL()))
        bReg = true;

    return beans::Optional< beans::Ambiguous<sal_Bool> >( true, beans::Ambiguous<sal_Bool>( bReg, false ) );
}

//______________________________________________________________________________
void BackendImpl::PackageImpl::processPackage_(
    ::osl::ResettableMutexGuard &,
    bool doRegisterPackage,
    bool /* startup */,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    (void)doRegisterPackage;
    (void)abortChannel;
    (void)xCmdEnv;

    BackendImpl* that = getMyBackend();
    that->implProcessHelp( this, doRegisterPackage, xCmdEnv);
}

beans::Optional< OUString > BackendImpl::PackageImpl::getRegistrationDataURL()
    throw (deployment::ExtensionRemovedException,
           css::uno::RuntimeException)
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();

    ::boost::optional<HelpBackendDb::Data> data =
          getMyBackend()->readDataFromDb(getURL());

    if (data && getMyBackend()->hasActiveEntry(getURL()))
        return beans::Optional<OUString>(true, data->dataUrl);

    return beans::Optional<OUString>(true, OUString());
}

void BackendImpl::implProcessHelp(
    PackageImpl * package, bool doRegisterPackage,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv)
{
    Reference< deployment::XPackage > xPackage(package);
    OSL_ASSERT(xPackage.is());
    if (doRegisterPackage)
    {
        //revive already processed help if possible
        if ( !activateEntry(xPackage->getURL()))
        {
            HelpBackendDb::Data data;
            data.dataUrl = xPackage->getURL();
            if (!package->extensionContainsCompiledHelp())
            {
#if !defined(ANDROID) && !defined(IOS)
                const OUString sHelpFolder = createFolder(OUString(), xCmdEnv);
                data.dataUrl = sHelpFolder;

                Reference< ucb::XSimpleFileAccess2 > xSFA = getFileAccess();
                rtl::OUString aHelpURL = xPackage->getURL();
                rtl::OUString aExpandedHelpURL = dp_misc::expandUnoRcUrl( aHelpURL );
                rtl::OUString aName = xPackage->getName();
                if( !xSFA->isFolder( aExpandedHelpURL ) )
                {
                    rtl::OUString aErrStr = getResourceString( RID_STR_HELPPROCESSING_GENERAL_ERROR );
                    aErrStr += rtl::OUString("No help folder" );
                    OWeakObject* oWeakThis = static_cast<OWeakObject *>(this);
                    throw deployment::DeploymentException( rtl::OUString(), oWeakThis,
                                                           makeAny( uno::Exception( aErrStr, oWeakThis ) ) );
                }

                // Scan languages
                Sequence< rtl::OUString > aLanguageFolderSeq = xSFA->getFolderContents( aExpandedHelpURL, true );
                sal_Int32 nLangCount = aLanguageFolderSeq.getLength();
                const rtl::OUString* pSeq = aLanguageFolderSeq.getConstArray();
                for( sal_Int32 iLang = 0 ; iLang < nLangCount ; ++iLang )
                {
                    rtl::OUString aLangURL = pSeq[iLang];
                    if( xSFA->isFolder( aLangURL ) )
                    {
                        std::vector< rtl::OUString > aXhpFileVector;

                        // calculate jar file URL
                        sal_Int32 indexStartSegment = aLangURL.lastIndexOf('/');
                        // for example "/en"
                        OUString langFolderURLSegment(
                            aLangURL.copy(
                                indexStartSegment + 1, aLangURL.getLength() - indexStartSegment - 1));

                        //create the folder in the "temporary folder"
                        ::ucbhelper::Content langFolderContent;
                        const OUString langFolderDest = makeURL(sHelpFolder, langFolderURLSegment);
                        const OUString langFolderDestExpanded = ::dp_misc::expandUnoRcUrl(langFolderDest);
                        ::dp_misc::create_folder(
                            &langFolderContent,
                            langFolderDest, xCmdEnv);

                        const OUString aHelpStr("help");
                        const OUString aSlash("/");

                        rtl::OUString aJarFile(
                            makeURL(sHelpFolder, langFolderURLSegment + aSlash + aHelpStr +
                                    OUSTR(".jar")));
                        aJarFile = ::dp_misc::expandUnoRcUrl(aJarFile);

                        rtl::OUString aEncodedJarFilePath = rtl::Uri::encode(
                            aJarFile, rtl_UriCharClassPchar,
                            rtl_UriEncodeIgnoreEscapes,
                            RTL_TEXTENCODING_UTF8 );
                        rtl::OUString aDestBasePath = rtl::OUString("vnd.sun.star.zip://" );
                        aDestBasePath += aEncodedJarFilePath;
                        aDestBasePath += rtl::OUString("/" );

                        sal_Int32 nLenLangFolderURL = aLangURL.getLength() + 1;

                        Sequence< rtl::OUString > aSubLangSeq = xSFA->getFolderContents( aLangURL, true );
                        sal_Int32 nSubLangCount = aSubLangSeq.getLength();
                        const rtl::OUString* pSubLangSeq = aSubLangSeq.getConstArray();
                        for( sal_Int32 iSubLang = 0 ; iSubLang < nSubLangCount ; ++iSubLang )
                        {
                            rtl::OUString aSubFolderURL = pSubLangSeq[iSubLang];
                            if( !xSFA->isFolder( aSubFolderURL ) )
                                continue;

                            implCollectXhpFiles( aSubFolderURL, aXhpFileVector );

                            // Copy to package (later: move?)
                            rtl::OUString aDestPath = aDestBasePath;
                            rtl::OUString aPureFolderName = aSubFolderURL.copy( nLenLangFolderURL );
                            aDestPath += aPureFolderName;
                            xSFA->copy( aSubFolderURL, aDestPath );
                        }

                        // Call compiler
                        sal_Int32 nXhpFileCount = aXhpFileVector.size();
                        rtl::OUString* pXhpFiles = new rtl::OUString[nXhpFileCount];
                        for( sal_Int32 iXhp = 0 ; iXhp < nXhpFileCount ; ++iXhp )
                        {
                            rtl::OUString aXhpFile = aXhpFileVector[iXhp];
                            rtl::OUString aXhpRelFile = aXhpFile.copy( nLenLangFolderURL );
                            pXhpFiles[iXhp] = aXhpRelFile;
                        }

                        rtl::OUString aOfficeHelpPath( SvtPathOptions().GetHelpPath() );
                        rtl::OUString aOfficeHelpPathFileURL;
                        ::osl::File::getFileURLFromSystemPath( aOfficeHelpPath, aOfficeHelpPathFileURL );

                        HelpProcessingErrorInfo aErrorInfo;
                        bool bSuccess = compileExtensionHelp(
                            aOfficeHelpPathFileURL, aHelpStr, aLangURL,
                            nXhpFileCount, pXhpFiles,
                            langFolderDestExpanded, aErrorInfo );

                        if( bSuccess )
                        {
                            rtl::OUString aLang;
                            sal_Int32 nLastSlash = aLangURL.lastIndexOf( '/' );
                            if( nLastSlash != -1 )
                                aLang = aLangURL.copy( nLastSlash + 1 );
                            else
                                aLang = rtl::OUString("en" );

                            rtl::OUString aMod("help");

                            HelpIndexer aIndexer(aLang, aMod, langFolderDestExpanded, langFolderDestExpanded);
                            aIndexer.indexDocuments();
                        }

                        if( !bSuccess )
                        {
                            sal_uInt16 nErrStrId = 0;
                            switch( aErrorInfo.m_eErrorClass )
                            {
                            case HELPPROCESSING_GENERAL_ERROR:
                            case HELPPROCESSING_INTERNAL_ERROR:     nErrStrId = RID_STR_HELPPROCESSING_GENERAL_ERROR; break;
                            case HELPPROCESSING_XMLPARSING_ERROR:   nErrStrId = RID_STR_HELPPROCESSING_XMLPARSING_ERROR; break;
                            default: ;
                            };

                            rtl::OUString aErrStr;
                            if( nErrStrId != 0 )
                            {
                                aErrStr = getResourceString( nErrStrId );

                                // Remoce CR/LF
                                rtl::OUString aErrMsg( aErrorInfo.m_aErrorMsg );
                                sal_Unicode nCR = 13, nLF = 10;
                                sal_Int32 nSearchCR = aErrMsg.indexOf( nCR );
                                sal_Int32 nSearchLF = aErrMsg.indexOf( nLF );
                                sal_Int32 nCopy;
                                if( nSearchCR != -1 || nSearchLF != -1 )
                                {
                                    if( nSearchCR == -1 )
                                        nCopy = nSearchLF;
                                    else if( nSearchLF == -1 )
                                        nCopy = nSearchCR;
                                    else
                                        nCopy = ( nSearchCR < nSearchLF ) ? nSearchCR : nSearchLF;

                                    aErrMsg = aErrMsg.copy( 0, nCopy );
                                }
                                aErrStr += aErrMsg;
                                if( nErrStrId == RID_STR_HELPPROCESSING_XMLPARSING_ERROR && !aErrorInfo.m_aXMLParsingFile.isEmpty() )
                                {
                                    aErrStr += rtl::OUString(" in " );

                                    rtl::OUString aDecodedFile = rtl::Uri::decode( aErrorInfo.m_aXMLParsingFile,
                                                                                   rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
                                    aErrStr += aDecodedFile;
                                    if( aErrorInfo.m_nXMLParsingLine != -1 )
                                    {
                                        aErrStr += rtl::OUString(", line " );
                                        aErrStr += ::rtl::OUString::valueOf( aErrorInfo.m_nXMLParsingLine );
                                    }
                                }
                            }

                            OWeakObject* oWeakThis = static_cast<OWeakObject *>(this);
                            throw deployment::DeploymentException( rtl::OUString(), oWeakThis,
                                                                   makeAny( uno::Exception( aErrStr, oWeakThis ) ) );
                        }
                    }
                }
#else
                (void) xCmdEnv;
#endif
            }
                //Writing the data entry replaces writing the flag file. If we got to this
                //point the registration was successful.
            addDataToDb(xPackage->getURL(), data);
        }
    } //if (doRegisterPackage)
    else
    {
        revokeEntryFromDb(xPackage->getURL());
    }
}

void BackendImpl::implCollectXhpFiles( const rtl::OUString& aDir,
    std::vector< rtl::OUString >& o_rXhpFileVector )
{
    Reference< ucb::XSimpleFileAccess2 > xSFA = getFileAccess();

    // Scan xhp files recursively
    Sequence< rtl::OUString > aSeq = xSFA->getFolderContents( aDir, true );
    sal_Int32 nCount = aSeq.getLength();
    const rtl::OUString* pSeq = aSeq.getConstArray();
    for( sal_Int32 i = 0 ; i < nCount ; ++i )
    {
        rtl::OUString aURL = pSeq[i];
        if( xSFA->isFolder( aURL ) )
        {
            implCollectXhpFiles( aURL, o_rXhpFileVector );
        }
        else
        {
            sal_Int32 nLastDot = aURL.lastIndexOf( '.' );
            if( nLastDot != -1 )
            {
                rtl::OUString aExt = aURL.copy( nLastDot + 1 );
                if( aExt.equalsIgnoreAsciiCase( rtl::OUString("xhp" ) ) )
                    o_rXhpFileVector.push_back( aURL );
            }
        }
    }
}

Reference< ucb::XSimpleFileAccess2 > BackendImpl::getFileAccess( void )
{
    if( !m_xSFA.is() )
    {
        Reference<XComponentContext> const & xContext = getComponentContext();
        if( xContext.is() )
        {
            m_xSFA = ucb::SimpleFileAccess::create(xContext);
        }
        if( !m_xSFA.is() )
        {
            throw RuntimeException(
                ::rtl::OUString(
                "dp_registry::backend::help::BackendImpl::getFileAccess(), "
                "could not instatiate SimpleFileAccess." ),
                Reference< XInterface >() );
        }
    }
    return m_xSFA;
}

} // anon namespace

namespace sdecl = comphelper::service_decl;
sdecl::class_<BackendImpl, sdecl::with_args<true> > serviceBI;
extern sdecl::ServiceDecl const serviceDecl(
    serviceBI,
    "com.sun.star.comp.deployment.help.PackageRegistryBackend",
    BACKEND_SERVICE_NAME );

} // namespace help
} // namespace backend
} // namespace dp_registry

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
