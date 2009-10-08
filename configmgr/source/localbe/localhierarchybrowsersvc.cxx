/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: localhierarchybrowsersvc.cxx,v $
 * $Revision: 1.12 $
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

#include "localhierarchybrowsersvc.hxx"
#include "localsinglebackend.hxx"

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif
#include <com/sun/star/lang/NullPointerException.hpp>
#include <rtl/ustrbuf.hxx>

#include <algorithm>
// -----------------------------------------------------------------------------

#define OUSTRING( constascii ) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(constascii))
// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace localbe
    {
// -----------------------------------------------------------------------------

sal_Char const * const aLocalHierarchyBrowserServices[] =
{
    "com.sun.star.configuration.backend.LocalHierarchyBrowser",
    0,
    "com.sun.star.configuration.backend.HierarchyBrowser",
    0
};
const ServiceImplementationInfo aLocalHierarchyBrowserSI =
{
    "com.sun.star.comp.configuration.backend.LocalHierarchyBrowser",
    aLocalHierarchyBrowserServices,
    aLocalHierarchyBrowserServices + 3
};
// -----------------------------------------------------------------------------

const ServiceRegistrationInfo* getLocalHierarchyBrowserServiceInfo()
{ return getRegistrationInfo(& aLocalHierarchyBrowserSI); }
// -----------------------------------------------------------------------------

inline
ServiceInfoHelper LocalHierarchyBrowserService::getServiceInfo()
{
    return & aLocalHierarchyBrowserSI;
}
// -----------------------------------------------------------------------------

uno::Reference< uno::XInterface > SAL_CALL instantiateLocalHierarchyBrowser
( uno::Reference< uno::XComponentContext > const& rServiceManager )
{
    return * new LocalHierarchyBrowserService( rServiceManager );
}
// -----------------------------------------------------------------------------

LocalHierarchyBrowserService::LocalHierarchyBrowserService(uno::Reference< uno::XComponentContext > const & _xContext)
: m_xServiceFactory(_xContext->getServiceManager(), uno::UNO_QUERY)
{
    if (!m_xServiceFactory.is())
    {
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration Importer: Unexpected NULL context"));
        throw lang::NullPointerException(sMessage,NULL);
    }
}
// -----------------------------------------------------------------------------

LocalHierarchyBrowserService::~LocalHierarchyBrowserService()
{}
// -----------------------------------------------------------------------------

namespace
{
    struct JobDesc
    {
        explicit JobDesc(task::XJob * pJob, const uno::Sequence< beans::NamedValue >& aArguments);

        enum Mode   { findNone, findSchemas, findLayers };
        enum Result { getDefault,getUrls, getNames };

        rtl::OUString                    aBaseDataUrl;
        uno::Sequence< rtl::OUString >   aExcludeList;
        Mode                        mode;
        Result                      result_type;
    };

    JobDesc::JobDesc(task::XJob * pJob, const uno::Sequence< beans::NamedValue >& aArguments)
    : aBaseDataUrl()
    , aExcludeList()
    , mode(findNone)
    , result_type(getDefault)
    {
        sal_Int16 const nCount = static_cast<sal_Int16>(aArguments.getLength());

        if (sal_Int32(nCount) != aArguments.getLength())
        {
            rtl::OUString sMessage = OUSTRING("Too many arguments for LocalHierarchyBrowser Job");
            throw lang::IllegalArgumentException(sMessage,pJob,0);
        }

        for (sal_Int16 i=0; i < nCount; ++i)
        {
            sal_Bool bKnown = false;
            sal_Bool bGood  = false;

            if (aArguments[i].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("SchemaDataUrl")))
            {
                bKnown = true;
                bGood  = (aArguments[i].Value >>= aBaseDataUrl);
                mode = (bGood && aBaseDataUrl.getLength()) ? findSchemas : findNone;
            }
            else if (aArguments[i].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("LayerDataUrl")))
            {
                bKnown = true;

                rtl::OUString aLayerBaseUrl;
                bGood  = (aArguments[i].Value >>= aLayerBaseUrl);

                if (aLayerBaseUrl.getLength())
                {
                    rtl::OUString aLocalizedSubDir;
                    LocalSingleBackend::getLayerSubDirectories(aLayerBaseUrl,this->aBaseDataUrl,aLocalizedSubDir);

                    mode = findLayers;
                }
                else
                {
                    mode = findNone;
                }
            }
            else if (aArguments[i].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ExcludeComponents")))
            {
                bKnown = true;

                sal_Int32 const nNextIndex = aExcludeList.getLength();

                switch (aArguments[i].Value.getValueTypeClass())
                {
                case uno::TypeClass_STRING:
                    {
                        rtl::OUString aComponent;
                        bGood = (aArguments[i].Value >>= aComponent);

                        OSL_ASSERT(bGood);

                        aExcludeList.realloc(nNextIndex + 1);
                        aExcludeList[nNextIndex] = aComponent;
                    }
                    break;

                case uno::TypeClass_SEQUENCE:
                    {
                        uno::Sequence<rtl::OUString> aComponentList;
                        bGood = (aArguments[i].Value >>= aComponentList);

                        if (bGood)
                        {
                            sal_Int32 const nCompListCount = aComponentList.getLength();
                            aExcludeList.realloc(nNextIndex + nCompListCount);

                            rtl::OUString const * pSrc = aComponentList.getConstArray();
                            std::copy(pSrc,pSrc+nCompListCount,aExcludeList.getArray());
                        }
                    }
                    break;

                default:
                    OSL_ASSERT(!bGood);
                    break;
                }
            }
            else if (aArguments[i].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("FetchComponentNames")))
            {
                sal_Bool bComponents = sal_False;

                bKnown = true;
                if (aArguments[i].Value.hasValue())
                {
                    bGood  = (aArguments[i].Value >>= bComponents);
                    if (bGood) result_type = bComponents ? getNames : getUrls;
                }
                else
                {
                    bGood = true;
                    result_type = getDefault;
                }
            }

            if (!bGood)
            {
                rtl::OUStringBuffer sMsg;
                sMsg.appendAscii("LocalHierarchyBrowser - Illegal argument: ");
                if (bKnown)
                    sMsg.appendAscii("Wrong value type for argument '");
                else
                    sMsg.appendAscii("Unknown argument '");

                sMsg.append(aArguments[i].Name).appendAscii("'.");

                throw lang::IllegalArgumentException(sMsg.makeStringAndClear(),pJob,i+1);
            }
        }
        if (findNone == mode)
        {
            rtl::OUStringBuffer sMsg;
            sMsg.appendAscii("LocalHierarchyBrowser - Missing argument: ");
            sMsg.appendAscii("No data URL available");
            throw lang::IllegalArgumentException(sMsg.makeStringAndClear(),pJob,0);
        }
        if (getDefault == result_type)
            result_type = (mode == findSchemas) ? getNames : getUrls;

    }

    static
    inline
    rtl::OUString getDataFileExtension(JobDesc::Mode mode)
    {
        switch (mode)
        {
        case JobDesc::findSchemas: return OUSTRING(".xcs");
        case JobDesc::findLayers:  return OUSTRING(".xcu");
        default: OSL_ASSERT(false); return rtl::OUString();
        }
    }
}
// -----------------------------------------------------------------------------

// XJob

uno::Any SAL_CALL
    LocalHierarchyBrowserService::execute( const uno::Sequence< beans::NamedValue >& Arguments )
        throw (lang::IllegalArgumentException, uno::Exception, uno::RuntimeException)
{
    JobDesc const aJob(this,Arguments);

    OSL_ASSERT(JobDesc::getUrls == aJob.result_type || JobDesc::getNames == aJob.result_type);

    uno::Sequence< rtl::OUString > (LocalHierarchyBrowserService::* const find)( rtl::OUString const & _aBaseDirectory, rtl::OUString const & _aComponentFileExtension, uno::Sequence< rtl::OUString > const & aExcludeList) = (JobDesc::getUrls == aJob.result_type) ?
        &LocalHierarchyBrowserService::findLocalComponentUrls :
        &LocalHierarchyBrowserService::findLocalComponentNames;

    uno::Sequence< rtl::OUString > aComponents = (this->*find)(aJob.aBaseDataUrl,getDataFileExtension(aJob.mode), aJob.aExcludeList);

    return uno::makeAny(aComponents);
}
// -----------------------------------------------------------------------------

// XServiceInfo

rtl::OUString SAL_CALL
    LocalHierarchyBrowserService::getImplementationName(  )
        throw (uno::RuntimeException)
{
    return getServiceInfo().getImplementationName( );
}
// -----------------------------------------------------------------------------

sal_Bool SAL_CALL
    LocalHierarchyBrowserService::supportsService( const rtl::OUString& ServiceName )
        throw (uno::RuntimeException)
{
    return getServiceInfo().supportsService( ServiceName );
}
// -----------------------------------------------------------------------------


uno::Sequence< ::rtl::OUString > SAL_CALL
    LocalHierarchyBrowserService::getSupportedServiceNames(  )
        throw (uno::RuntimeException)
{
    return getServiceInfo().getSupportedServiceNames( );
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace

// -----------------------------------------------------------------------------
} // namespace

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#include "filehelper.hxx"
#include <osl/file.hxx>
#include "tools/getprocessworkingdir.hxx"
#include <vector>
// -----------------------------------------------------------------------------

    namespace uno = com::sun::star::uno;
    // -----------------------------------------------------------------------------

    #define OSL_VERIFY_RC( expr ) OSL_VERIFY( (expr) == osl::FileBase::E_None )
    //------------------------------------------------------------------------------
    static
    inline
    bool matchesExtension( rtl::OUString const & aFileName, rtl::OUString const & aExt )
    {
        sal_Int32 const nExtStart = aFileName.getLength() - aExt.getLength();
        return nExtStart > 0 && !!aFileName.copy(nExtStart).equalsIgnoreAsciiCase(aExt);
    }
    //------------------------------------------------------------------------------
    static
    inline
    rtl::OUString stripExtension( rtl::OUString const & aFileName, rtl::OUString const & aExt )
    {
        OSL_PRECOND( matchesExtension(aFileName,aExt), "File name doesn't have expected extension");

        sal_Int32 const nExtStart = aFileName.getLength() - aExt.getLength();
        return aFileName.copy(0,nExtStart);
    }
//------------------------------------------------------------------------------
    static
    inline
    bool matchesExtension( osl::FileStatus const & aFileDescriptor, rtl::OUString const & aExt )
    {
        OSL_PRECOND( aFileDescriptor.isValid(FileStatusMask_Type | FileStatusMask_FileName),
                    "Not all required file-status fields available for filter" );

        if (aFileDescriptor.getFileType() != osl::FileStatus::Regular)
            return false;

        return matchesExtension(aFileDescriptor.getFileName(),aExt);
    }
//------------------------------------------------------------------------------

    static
    bool makeAbsoluteURL(rtl::OUString & rURL )
    {
        rtl::OUString aBaseDir; tools::getProcessWorkingDir(&aBaseDir);

        osl::File::RC errcode = osl::File::getAbsoluteFileURL(aBaseDir,rURL,rURL);

        return osl::File::E_None == errcode;
    }
//------------------------------------------------------------------------------
    static
    inline
    bool getNextDirectoryItem(osl::Directory & aDirectory, osl::DirectoryItem & aItem, osl::Directory::RC & errcode)
    {
        switch (errcode = aDirectory.getNextItem(aItem))
        {
        case osl::Directory::E_None:
            return true;

        case osl::Directory::E_NOENT:
            errcode = osl::Directory::E_None;
            return false;

        default:
            return false;
        }
    }
//------------------------------------------------------------------------------
    static inline bool isExcluded(rtl::OUString const & aName, uno::Sequence< rtl::OUString > const & aExcludeList)
    {
        for (sal_Int32 i = 0; i<aExcludeList.getLength(); ++i)
        {
            if (aExcludeList[i].equals(aName)) return true;
        }
        return false;
    }
//------------------------------------------------------------------------------
    static
    osl::FileBase::RC findComponents( std::vector<rtl::OUString> * componentNames, std::vector<rtl::OUString> * componentUrls,
                                        rtl::OUString const& aDirectoryPath, rtl::OUString const& aComponentExtension,
                                        rtl::OUString const& aPackagePrefix, rtl::OUString const & aComponentSeparator,
                                        uno::Sequence< rtl::OUString > const & aExcludeList)
    {
        static sal_Unicode const chDirSep = '/';
        static rtl::OUString    const sDirectorySeparator(&chDirSep,1);

        osl::Directory aDirectory(aDirectoryPath);

        osl::Directory::RC errcode = aDirectory.open();

        if (errcode == osl::Directory::E_None)
        {
            sal_uInt32 n_STATUS_FIELDS =  FileStatusMask_Type | FileStatusMask_FileName;
            if (componentUrls) n_STATUS_FIELDS |= FileStatusMask_FileURL;

            osl::DirectoryItem aItem;
            while( getNextDirectoryItem(aDirectory,aItem,errcode) )
            {

                osl::FileStatus aItemDescriptor( n_STATUS_FIELDS );
                errcode = aItem.getFileStatus(aItemDescriptor);

                if ( errcode != osl::DirectoryItem::E_None )
                {
                    OSL_TRACE("Locating Configuration Components - Error (%u) getting status of directory item - skipping\n", unsigned(errcode));
                    continue;
                }

                OSL_ENSURE( aItemDescriptor.isValid(FileStatusMask_Type), "Could not get type of directory item");

                if (aItemDescriptor.getFileType() == osl::FileStatus::Directory)
                {
                    OSL_ENSURE( aItemDescriptor.isValid(FileStatusMask_FileName), "Could not get name of subdirectory");

                    rtl::OUString const aSubdirName = aItemDescriptor.getFileName();
                    rtl::OUString const aSubdirPath = aDirectoryPath + sDirectorySeparator + aSubdirName;
                    rtl::OUString const aSubpackagePrefix = aPackagePrefix + aSubdirName + aComponentSeparator;
                    // recurse
                    if (!isExcluded(aSubpackagePrefix,aExcludeList))
                        OSL_VERIFY_RC( findComponents( componentNames,  componentUrls,
                                                        aSubdirPath, aComponentExtension,
                                                        aSubpackagePrefix, aComponentSeparator,
                                                        aExcludeList) );
                }
                else if (matchesExtension(aItemDescriptor,aComponentExtension))
                {
                    OSL_ENSURE( aItemDescriptor.isValid(FileStatusMask_FileName), "Could not get name of component found");

                    rtl::OUString const aComponentName = stripExtension( aItemDescriptor.getFileName(), aComponentExtension );
                    rtl::OUString const aFullComponentName = aPackagePrefix + aComponentName;

                    if (!isExcluded(aFullComponentName,aExcludeList))
                    {
                        if (componentNames)
                        {
                            componentNames->push_back(aFullComponentName);
                        }

                        if (componentUrls)
                        {
                            OSL_ENSURE( aItemDescriptor.isValid(FileStatusMask_FileURL), "Could not get URL of component found");

                            componentUrls->push_back(aItemDescriptor.getFileURL());
                        }
                    }
                }
            }
            aDirectory.close();
        }
        return errcode;
    }
// -----------------------------------------------------------------------------

    uno::Sequence< rtl::OUString > configmgr::localbe::LocalHierarchyBrowserService::findLocalComponentNames( rtl::OUString const & _aBaseDirectory, rtl::OUString const & _aComponentFileExtension, uno::Sequence< rtl::OUString > const & aExcludeList)
    {
        rtl::OUString aBaseDirectory(_aBaseDirectory);
        OSL_VERIFY( makeAbsoluteURL(aBaseDirectory) );

        static const sal_Unicode chPkgSep = '.';

        std::vector< rtl::OUString > components;

        osl::Directory::RC errcode = findComponents(&components, NULL,
                                                    aBaseDirectory, _aComponentFileExtension,
                                                    rtl::OUString(), rtl::OUString(&chPkgSep,1),
                                                    aExcludeList );

        if (errcode != osl::Directory::E_None)
        {
            OSL_TRACE("Locating Configuration Components failed - Error (%u) trying to locate files\n", unsigned(errcode));

            if (errcode != osl::Directory::E_NOENT)
            {
                rtl::OUString sMsg = OUSTRING("LocalHierarchyBrowser - IO Error while scanning for components: ") +
                                FileHelper::createOSLErrorString(errcode);

                throw com::sun::star::io::IOException(sMsg,*this);
            }
        }

        return uno::Sequence< rtl::OUString >(&components.front(),components.size());
    }
// -----------------------------------------------------------------------------

    uno::Sequence< rtl::OUString > configmgr::localbe::LocalHierarchyBrowserService::findLocalComponentUrls( rtl::OUString const & _aBaseDirectory, rtl::OUString const & _aComponentFileExtension, uno::Sequence< rtl::OUString > const & aExcludeList)
    {
        rtl::OUString aBaseDirectory(_aBaseDirectory);
        OSL_VERIFY( makeAbsoluteURL(aBaseDirectory) );

        static const sal_Unicode chPkgSep = '.';

        std::vector< rtl::OUString > components;

        osl::Directory::RC errcode = findComponents(NULL, &components,
                                                    aBaseDirectory, _aComponentFileExtension,
                                                    rtl::OUString(), rtl::OUString(&chPkgSep,1),
                                                    aExcludeList );

        if (errcode != osl::Directory::E_None)
        {
            OSL_TRACE("Locating Configuration Components failed - Error (%u) trying to locate files\n", unsigned(errcode));

            if (errcode != osl::Directory::E_NOENT)
            {
                rtl::OUString sMsg = OUSTRING("LocalHierarchyBrowser - IO Error while scanning for component files: ") +
                                FileHelper::createOSLErrorString(errcode);

                throw com::sun::star::io::IOException(sMsg,*this);
            }
        }

        return uno::Sequence< rtl::OUString >(&components.front(),components.size());
    }
//------------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

