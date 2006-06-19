/*************************************************************************
*
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: binarycache.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:18:22 $
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

#include "binarycache.hxx"

#include "binaryreadhandler.hxx"
#include "binarywritehandler.hxx"

#include "mergedcomponentdata.hxx"

#ifndef _CONFIGMGR_FILEHELPER_HXX_
#include "filehelper.hxx"
#endif

#ifndef CONFIGMGR_TYPECONVERTER_HXX
#include "typeconverter.hxx"
#endif

#ifndef _CONFIGMGR_BOOTSTRAP_HXX
#include "bootstrap.hxx"
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif // _RTL_USTRBUF_HXX_

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#define RTL_LOGFILE_OU2A(rtlOUString)   (::rtl::OUStringToOString((rtlOUString), RTL_TEXTENCODING_ASCII_US).getStr())

namespace configmgr
{
    // -----------------------------------------------------------------------------
    namespace backend
    {

        using ::rtl::OUString;

        const OUString aSettingName(
                RTL_CONSTASCII_USTRINGPARAM( CONTEXT_ITEM_PREFIX_ "CacheUrl"));
        // ---------------------------------------------------------------------------------------
        static inline bool isValidFileURL (OUString const& _sFileURL)
        {
            using osl::File;

            OUString sSystemPath;
            return _sFileURL.getLength() && (File::E_None == File::getSystemPathFromFileURL(_sFileURL, sSystemPath));
        }
        // -----------------------------------------------------------------------------
        // ---------------------------------------------------------------------------------------
        static
        bool implEnsureAbsoluteURL(rtl::OUString & _rsURL) // also strips embedded dots etc.
        {
            using osl::File;
            if (!_rsURL.getLength())
                return false;

            if (!isValidFileURL(_rsURL))
            {
                OSL_TRACE("Binary cache: File URL %s is invalid.",
                            rtl::OUStringToOString(_rsURL,RTL_TEXTENCODING_ASCII_US).getStr());
                return false;
            }

            rtl::OUString sBasePath = _rsURL;
            OSL_VERIFY(osl_Process_E_None == osl_getProcessWorkingDir(&sBasePath.pData));

            rtl::OUString sAbsolute;
            if ( File::E_None == File::getAbsoluteFileURL(sBasePath, _rsURL, sAbsolute))
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

        OUString BinaryCache::getCacheFileURL(const OUString& aComponent) const
        {
            rtl::OUStringBuffer retCode (mBaseURL);
            retCode.append(kPathSeparator) ;
        //  retCode.append(aComponent.replace(kComponentSeparator, kPathSeparator)) ;
            retCode.append(aComponent) ;
            retCode.appendAscii(RTL_CONSTASCII_STRINGPARAM(kBinarySuffix));

            OUString aResult = retCode.makeStringAndClear() ;

            if (isValidFileURL(aResult))
            {
                return aResult;
            }
            else
            {
                OSL_ENSURE(false, "Component File URL is invalid");
                return OUString();
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

            OUString sCacheUrl;
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

        void BinaryCache::setOwnerEntity(const OUString & aOwnerEntity)
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
                                MultiServiceFactory const & aFactory,
                                OUString const & aComponent,
                                OUString const & aSchemaVersion,
                                OUString const & aEntity,
                                localehelper::Locale const & aRequestedLocale,
                                localehelper::LocaleSequence & outKnownLocales,
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
                                MultiServiceFactory const & aFactory,
                                OUString const & aComponent,
                                OUString const & aSchemaVersion,
                                OUString const & aEntity,
                                localehelper::LocaleSequence const & aKnownLocales,
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
