/*************************************************************************
 *
 *  $RCSfile: localhierarchybrowsersvc.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:19 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "localhierarchybrowsersvc.hxx"

#ifndef CONFIGMGR_LOCALBE_LOCALSINGLEBACKEND_HXX_
#include "localsinglebackend.hxx"
#endif

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_NULLPOINTEREXCEPTION_HPP_
#include <com/sun/star/lang/NullPointerException.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

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

AsciiServiceName const aLocalHierarchyBrowserServices[] =
{
    "com.sun.star.configuration.backend.local.HierarchyBrowser",
    "com.sun.star.configuration.backend.LocalHierarchyBrowser",
    0,
    "com.sun.star.configuration.backend.HierarchyBrowser",
    0
};
const ServiceImplementationInfo aLocalHierarchyBrowserSI =
{
    "com.sun.star.comp.configuration.backend.local.LocalHierarchyBrowser",
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
( CreationContext const& rServiceManager )
{
    return * new LocalHierarchyBrowserService( rServiceManager );
}
// -----------------------------------------------------------------------------

LocalHierarchyBrowserService::LocalHierarchyBrowserService(CreationArg _xContext)
: m_xServiceFactory(_xContext->getServiceManager(), uno::UNO_QUERY)
{
    if (!m_xServiceFactory.is())
    {
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration Importer: Unexpected NULL context"));
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

        OUString                    aBaseDataUrl;
        uno::Sequence< OUString >   aExcludeList;
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
            OUString sMessage = OUSTRING("Too many arguments for LocalHierarchyBrowser Job");
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

                OUString aLayerBaseUrl;
                bGood  = (aArguments[i].Value >>= aLayerBaseUrl);

                if (aLayerBaseUrl.getLength())
                {
                    OUString aLocalizedSubDir;
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
                        OUString aComponent;
                        bGood = (aArguments[i].Value >>= aComponent);

                        OSL_ASSERT(bGood);

                        aExcludeList.realloc(nNextIndex + 1);
                        aExcludeList[nNextIndex] = aComponent;
                    }
                    break;

                case uno::TypeClass_SEQUENCE:
                    {
                        uno::Sequence<OUString> aComponentList;
                        bGood = (aArguments[i].Value >>= aComponentList);

                        if (bGood)
                        {
                            sal_Int32 const nCount = aComponentList.getLength();
                            aExcludeList.realloc(nNextIndex + nCount);

                            OUString const * pSrc = aComponentList.getConstArray();
                            std::copy(pSrc,pSrc+nCount,aExcludeList.getArray());
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
                sal_Bool bComponents;

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
    OUString getDataFileExtension(JobDesc::Mode mode)
    {
        switch (mode)
        {
        case JobDesc::findSchemas: return OUSTRING(".xcs");
        case JobDesc::findLayers:  return OUSTRING(".xcu");
        default: OSL_ASSERT(false); return OUString();
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

    typedef uno::Sequence< OUString > (LocalHierarchyBrowserService::*Finder)( OUString const & _aBaseDirectory, OUString const & _aComponentFileExtension, uno::Sequence< OUString > const & aExcludeList);

    OSL_ASSERT(JobDesc::getUrls == aJob.result_type || JobDesc::getNames == aJob.result_type);

    Finder const find = (JobDesc::getUrls == aJob.result_type) ?
        &LocalHierarchyBrowserService::findLocalComponentUrls :
        &LocalHierarchyBrowserService::findLocalComponentNames;

    uno::Sequence< OUString > aComponents = (this->*find)(aJob.aBaseDataUrl,getDataFileExtension(aJob.mode), aJob.aExcludeList);

    return uno::makeAny(aComponents);
}
// -----------------------------------------------------------------------------

// XServiceInfo

OUString SAL_CALL
    LocalHierarchyBrowserService::getImplementationName(  )
        throw (uno::RuntimeException)
{
    return getServiceInfo().getImplementationName( );
}
// -----------------------------------------------------------------------------

sal_Bool SAL_CALL
    LocalHierarchyBrowserService::supportsService( const OUString& ServiceName )
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
#include <osl/process.h>
#include <vector>
// -----------------------------------------------------------------------------

    using rtl::OUString;
    namespace uno = com::sun::star::uno;
    // -----------------------------------------------------------------------------

    #define OSL_VERIFY_RC( expr ) OSL_VERIFY( (expr) == osl::FileBase::E_None )
    //------------------------------------------------------------------------------
    static
    inline
    bool matchesExtension( OUString const & aFileName, OUString const & aExt )
    {
        sal_Int32 const nExtStart = aFileName.getLength() - aExt.getLength();
        return nExtStart > 0 && !!aFileName.copy(nExtStart).equalsIgnoreAsciiCase(aExt);
    }
    //------------------------------------------------------------------------------
    static
    inline
    OUString stripExtension( OUString const & aFileName, OUString const & aExt )
    {
        OSL_PRECOND( matchesExtension(aFileName,aExt), "File name doesn't have expected extension");

        sal_Int32 const nExtStart = aFileName.getLength() - aExt.getLength();
        return aFileName.copy(0,nExtStart);
    }
//------------------------------------------------------------------------------
    static
    inline
    bool matchesExtension( osl::FileStatus const & aFileDescriptor, OUString const & aExt )
    {
        OSL_PRECOND( aFileDescriptor.isValid(FileStatusMask_Type | FileStatusMask_FileName),
                    "Not all required file-status fields available for filter" );

        if (aFileDescriptor.getFileType() != osl::FileStatus::Regular)
            return false;

        return matchesExtension(aFileDescriptor.getFileName(),aExt);
    }
//------------------------------------------------------------------------------

    static
    bool makeAbsoluteURL(OUString & rURL )
    {
        using osl::File;
        OUString aBaseDir; osl_getProcessWorkingDir(&aBaseDir.pData);

        File::RC errcode = osl::File::getAbsoluteFileURL(aBaseDir,rURL,rURL);

        return File::E_None == errcode;
    }
//------------------------------------------------------------------------------
    static
    inline
    bool getNextDirectoryItem(osl::Directory & aDirectory, osl::DirectoryItem & aItem, osl::Directory::RC & errcode)
    {
        using osl::Directory;
        switch (errcode = aDirectory.getNextItem(aItem))
        {
        case Directory::E_None:
            return true;

        case Directory::E_NOENT:
            errcode = Directory::E_None;
            return false;

        default:
            return false;
        }
    }
//------------------------------------------------------------------------------
    static inline bool isExcluded(OUString const & aName, uno::Sequence< OUString > const & aExcludeList)
    {
        for (sal_Int32 i = 0; i<aExcludeList.getLength(); ++i)
        {
            if (aExcludeList[i].equals(aName)) return true;
        }
        return false;
    }
//------------------------------------------------------------------------------
    static
    osl::FileBase::RC findComponents( std::vector<OUString> * componentNames, std::vector<OUString> * componentUrls,
                                        OUString const& aDirectoryPath, OUString const& aComponentExtension,
                                        OUString const& aPackagePrefix, OUString const & aComponentSeparator,
                                        uno::Sequence< OUString > const & aExcludeList)
    {
        using osl::Directory;
        using osl::DirectoryItem;
        using osl::FileStatus;

        static sal_Unicode const chDirSep = '/';
        static OUString    const sDirectorySeparator(&chDirSep,1);

        Directory aDirectory(aDirectoryPath);

        osl::Directory::RC errcode = aDirectory.open();

        if (errcode == Directory::E_None)
        {
            sal_uInt32 n_STATUS_FIELDS =  FileStatusMask_Type | FileStatusMask_FileName;
            if (componentUrls) n_STATUS_FIELDS |= FileStatusMask_FileURL;

            osl::DirectoryItem aItem;
            while( getNextDirectoryItem(aDirectory,aItem,errcode) )
            {

                FileStatus aItemDescriptor( n_STATUS_FIELDS );
                errcode = aItem.getFileStatus(aItemDescriptor);

                if ( errcode != DirectoryItem::E_None )
                {
                    OSL_TRACE("Locating Configuration Components - Error (%u) getting status of directory item - skipping\n", unsigned(errcode));
                    continue;
                }

                OSL_ENSURE( aItemDescriptor.isValid(FileStatusMask_Type), "Could not get type of directory item");

                if (aItemDescriptor.getFileType() == FileStatus::Directory)
                {
                    OSL_ENSURE( aItemDescriptor.isValid(FileStatusMask_FileName), "Could not get Name of subdirectory");

                    OUString const aSubdirName = aItemDescriptor.getFileName();
                    OUString const aSubdirPath = aDirectoryPath + sDirectorySeparator + aSubdirName;
                    OUString const aSubpackagePrefix = aPackagePrefix + aSubdirName + aComponentSeparator;
                    // recurse
                    if (!isExcluded(aSubpackagePrefix,aExcludeList))
                        OSL_VERIFY_RC( findComponents( componentNames,  componentUrls,
                                                        aSubdirPath, aComponentExtension,
                                                        aSubpackagePrefix, aComponentSeparator,
                                                        aExcludeList) );
                }
                else if (matchesExtension(aItemDescriptor,aComponentExtension))
                {
                    OSL_ENSURE( aItemDescriptor.isValid(FileStatusMask_FileName), "Could not get Name of component found");

                    OUString const aComponentName = stripExtension( aItemDescriptor.getFileName(), aComponentExtension );
                    OUString const aFullComponentName = aPackagePrefix + aComponentName;

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

    uno::Sequence< OUString > configmgr::localbe::LocalHierarchyBrowserService::findLocalComponentNames( OUString const & _aBaseDirectory, OUString const & _aComponentFileExtension, uno::Sequence< OUString > const & aExcludeList)
    {
        OUString aBaseDirectory(_aBaseDirectory);
        OSL_VERIFY( makeAbsoluteURL(aBaseDirectory) );

        static const sal_Unicode chPkgSep = '.';

        std::vector< OUString > components;

        osl::Directory::RC errcode = findComponents(&components, NULL,
                                                    aBaseDirectory, _aComponentFileExtension,
                                                    OUString(), OUString(&chPkgSep,1),
                                                    aExcludeList );

        if (errcode != osl::Directory::E_None)
        {
            OSL_TRACE("Locating Configuration Components failed - Error (%u) trying to locate files\n", unsigned(errcode));

            if (errcode != osl::Directory::E_NOENT)
            {
                OUString sMsg = OUSTRING("LocalHierarchyBrowser - IO Error while scanning for components: ") +
                                FileHelper::createOSLErrorString(errcode);

                throw com::sun::star::io::IOException(sMsg,*this);
            }
        }

        return uno::Sequence< OUString >(&components.front(),components.size());
    }
// -----------------------------------------------------------------------------

    uno::Sequence< OUString > configmgr::localbe::LocalHierarchyBrowserService::findLocalComponentUrls( OUString const & _aBaseDirectory, OUString const & _aComponentFileExtension, uno::Sequence< OUString > const & aExcludeList)
    {
        OUString aBaseDirectory(_aBaseDirectory);
        OSL_VERIFY( makeAbsoluteURL(aBaseDirectory) );

        static const sal_Unicode chPkgSep = '.';

        std::vector< OUString > components;

        osl::Directory::RC errcode = findComponents(NULL, &components,
                                                    aBaseDirectory, _aComponentFileExtension,
                                                    OUString(), OUString(&chPkgSep,1),
                                                    aExcludeList );

        if (errcode != osl::Directory::E_None)
        {
            OSL_TRACE("Locating Configuration Components failed - Error (%u) trying to locate files\n", unsigned(errcode));

            if (errcode != osl::Directory::E_NOENT)
            {
                OUString sMsg = OUSTRING("LocalHierarchyBrowser - IO Error while scanning for component files: ") +
                                FileHelper::createOSLErrorString(errcode);

                throw com::sun::star::io::IOException(sMsg,*this);
            }
        }

        return uno::Sequence< OUString >(&components.front(),components.size());
    }
//------------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

