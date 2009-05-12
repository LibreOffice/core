/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: binarycache.cxx,v $
 * $Revision: 1.10 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "binarycache.hxx"

#include "binaryreadhandler.hxx"
#include "binarywritehandler.hxx"

#include "mergedcomponentdata.hxx"
#include "filehelper.hxx"
#include "typeconverter.hxx"

#ifndef _CONFIGMGR_BOOTSTRAP_HXX
#include "bootstrap.hxx"
#endif
#include <osl/file.hxx>
#include "tools/getprocessworkingdir.hxx"
#include <rtl/ustrbuf.hxx>
#include <rtl/logfile.hxx>

#define RTL_LOGFILE_OU2A(rtlOUString)   (::rtl::OUStringToOString((rtlOUString), RTL_TEXTENCODING_ASCII_US).getStr())

namespace configmgr
{
    // -----------------------------------------------------------------------------
    namespace backend
    {
        const rtl::OUString aSettingName(
                RTL_CONSTASCII_USTRINGPARAM( CONTEXT_ITEM_PREFIX_ "CacheUrl"));
        // ---------------------------------------------------------------------------------------
        static inline bool isValidFileURL (rtl::OUString const& _sFileURL)
        {
            rtl::OUString sSystemPath;
            return _sFileURL.getLength() && (osl::File::E_None == osl::File::getSystemPathFromFileURL(_sFileURL, sSystemPath));
        }
        // -----------------------------------------------------------------------------
        // ---------------------------------------------------------------------------------------
        static
        bool implEnsureAbsoluteURL(rtl::OUString & _rsURL) // also strips embedded dots etc.
        {
            if (!_rsURL.getLength())
                return false;

            if (!isValidFileURL(_rsURL))
            {
                OSL_TRACE("Binary cache: File URL %s is invalid.",
                            rtl::OUStringToOString(_rsURL,RTL_TEXTENCODING_ASCII_US).getStr());
                return false;
            }

            rtl::OUString sBasePath;
            OSL_VERIFY(tools::getProcessWorkingDir(&sBasePath));

            rtl::OUString sAbsolute;
            if ( osl::File::E_None == osl::File::getAbsoluteFileURL(sBasePath, _rsURL, sAbsolute))
            {
                _rsURL = sAbsolute;
                return isValidFileURL(_rsURL);
            }
            else
            {
                OSL_ENSURE(!isValidFileURL(_rsURL), "Could not get absolute file URL for valid URL");
                return false;
            }
        }
        // ---------------------------------------------------------------------------------------
        static const sal_Unicode kComponentSeparator = '.' ;
        static const sal_Unicode kPathSeparator = '/' ;
        static const char kBinarySuffix[] = ".dat" ;

        rtl::OUString BinaryCache::getCacheFileURL(const rtl::OUString& aComponent) const
        {
            rtl::OUStringBuffer retCode (mBaseURL);
            retCode.append(kPathSeparator) ;
        //  retCode.append(aComponent.replace(kComponentSeparator, kPathSeparator)) ;
            retCode.append(aComponent) ;
            retCode.appendAscii(RTL_CONSTASCII_STRINGPARAM(kBinarySuffix));

            rtl::OUString aResult = retCode.makeStringAndClear() ;

            if (isValidFileURL(aResult))
            {
                return aResult;
            }
            else
            {
                OSL_ENSURE(false, "Component File URL is invalid");
                return rtl::OUString();
            }
        }
        // -----------------------------------------------------------------------------
        BinaryCache::BinaryCache(const uno::Reference<uno::XComponentContext>& xContext )
        : mBaseURL()
        , mOwnerEntity()
        , mbCacheEnabled(false)
        {

            //initialise the base URL
            ContextReader aReader(xContext);

            rtl::OUString sCacheUrl;
            if (!aReader.isAdminService())
            {
                        mbCacheEnabled = (aReader.getBestContext()->getValueByName(aSettingName) >>= sCacheUrl)
                                        && implEnsureAbsoluteURL(sCacheUrl);
                    }

                    if (mbCacheEnabled)
                    {
                        mBaseURL = sCacheUrl;
                if (!FileHelper::dirExists(sCacheUrl))
                {
                    osl::File::RC errorCode = FileHelper::mkdirs(sCacheUrl);
                    if (errorCode)
                                {
#if (OSL_DEBUG_LEVEL > 0)
                                    rtl::OString sURL = rtl::OUStringToOString(sCacheUrl,RTL_TEXTENCODING_ASCII_US);
                        rtl::OString sErr = rtl::OUStringToOString(FileHelper::createOSLErrorString(errorCode),RTL_TEXTENCODING_ASCII_US);
                                        ::osl_trace("Configuration: Cannot create cache directory \"%s\". "
                                   "Error is %s [%d]",sURL.getStr(),sErr.getStr(),int(errorCode)) ;
#endif
                                    mbCacheEnabled = false;
                    }
                }
                }
        }
        // -----------------------------------------------------------------------------

        void BinaryCache::setOwnerEntity(const rtl::OUString & aOwnerEntity)
        {
            OSL_PRECOND(mOwnerEntity.getLength() == 0, "Owner entity of cache already set");
            mOwnerEntity = aOwnerEntity;
        }
        // -----------------------------------------------------------------------------

        void BinaryCache::disableCache()
        {
            mbCacheEnabled = false;
        }
        // -----------------------------------------------------------------------------

        bool BinaryCache::isCacheEnabled(rtl::OUString const & aEntity) const
        {
            if (!mbCacheEnabled) return false;

            // default entity is empty
            if (aEntity.getLength() == 0) return true;

            return aEntity.equals(mOwnerEntity);
        }
        // -----------------------------------------------------------------------------
        bool BinaryCache::readComponentData(MergedComponentData & aComponentData,
                                uno::Reference< lang::XMultiServiceFactory > const & aFactory,
                                rtl::OUString const & aComponent,
                                rtl::OUString const & aSchemaVersion,
                                rtl::OUString const & aEntity,
                                com::sun::star::lang::Locale const & aRequestedLocale,
                                std::vector< com::sun::star::lang::Locale > & outKnownLocales,
                                const uno::Reference<backenduno::XLayer> * pLayers,
                                sal_Int32 nNumLayers,
                                bool bIncludeTemplates)
        {
            if (isCacheEnabled(aEntity))
            try
            {
                RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::BinaryCache", "jb99855", "configmgr: BinaryCache::readComponentData() - enabled");
                BinaryReadHandler aCacheReader(getCacheFileURL(aComponent),aComponent,aFactory);

                // #i49148# Invalidate cache when schema version changes - using former 'owner' parameter for version
                if(aCacheReader.validateHeader(pLayers, nNumLayers, aSchemaVersion, aRequestedLocale, outKnownLocales))
                {
                    RTL_LOGFILE_CONTEXT_AUTHOR(aLog1, "configmgr::backend::BinaryCache", "jb99855", "configmgr: BinaryCache::readComponentData() - cache hit");
                    aComponentData.setSchemaRoot( aCacheReader.readComponentTree() );
                    if (bIncludeTemplates)
                        aComponentData.setTemplatesTree( aCacheReader.readTemplatesTree() );
                    return true;
                }
            }
            catch (uno::Exception & e)
            {
                OSL_TRACE("Binary Cache read failed - exception: %s", rtl::OUStringToOString(e.Message,RTL_TEXTENCODING_ASCII_US).getStr());
            }
            return false;
        }
        // -----------------------------------------------------------------------------

        bool BinaryCache::writeComponentData(MergedComponentData const & aComponentData,
                                uno::Reference< lang::XMultiServiceFactory > const & aFactory,
                                rtl::OUString const & aComponent,
                                rtl::OUString const & aSchemaVersion,
                                rtl::OUString const & aEntity,
                                std::vector< com::sun::star::lang::Locale > const & aKnownLocales,
                                const uno::Reference<backenduno::XLayer> * pLayers,
                                sal_Int32 nNumLayers)
        {
            if (isCacheEnabled(aEntity))
            try
            {
                RTL_LOGFILE_CONTEXT_AUTHOR(aLog3, "configmgr::backend::BinaryCache", "jb99855", "configmgr: BinaryCache::writeComponentData() - enabled");
                BinaryWriteHandler aCacheWriter(getCacheFileURL(aComponent),aComponent, aFactory);

                //write data to cache
                // #i49148# Invalidate cache when schema version changes - using former 'owner' parameter for schema
                if (aCacheWriter.generateHeader(pLayers, nNumLayers, aSchemaVersion, aKnownLocales))
                {
                    aCacheWriter.writeComponentTree(aComponentData.getSchemaTree());
                    aCacheWriter.writeTemplatesTree(aComponentData.getTemplatesTree());
                    return true;
                }
            }
            catch (uno::Exception & e)
            {
                OSL_TRACE("Configuration: Cache write failed - exception: %s", rtl::OUStringToOString(e.Message,RTL_TEXTENCODING_ASCII_US).getStr());
            }
            return false;
        }
        // -----------------------------------------------------------------------------

    }
// -----------------------------------------------------------------------------
}
