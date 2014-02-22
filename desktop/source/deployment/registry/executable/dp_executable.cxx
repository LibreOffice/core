/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "dp_misc.h"
#include "dp_backend.h"
#include "dp_ucb.h"
#include "dp_interact.h"
#include "rtl/string.hxx"
#include "osl/file.hxx"
#include "ucbhelper/content.hxx"
#include "comphelper/servicedecl.hxx"
#include "svl/inettype.hxx"
#include "cppuhelper/implbase1.hxx"
#include "dp_executablebackenddb.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace dp_misc;

namespace dp_registry {
namespace backend {
namespace executable {
namespace {

class BackendImpl : public ::dp_registry::backend::PackageRegistryBackend
{
    class ExecutablePackageImpl : public ::dp_registry::backend::Package
    {
        BackendImpl * getMyBackend() const;

        
        virtual beans::Optional< beans::Ambiguous<sal_Bool> > isRegistered_(
            ::osl::ResettableMutexGuard & guard,
            ::rtl::Reference<dp_misc::AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv );
        virtual void processPackage_(
            ::osl::ResettableMutexGuard & guard,
            bool registerPackage,
            bool startup,
            ::rtl::Reference<dp_misc::AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv );

        bool getFileAttributes(sal_uInt64& out_Attributes);
        bool isUrlTargetInExtension();

    public:
        inline ExecutablePackageImpl(
            ::rtl::Reference<PackageRegistryBackend> const & myBackend,
            OUString const & url, OUString const & name,
            Reference<deployment::XPackageTypeInfo> const & xPackageType,
            bool bRemoved, OUString const & identifier)
            : Package( myBackend, url, name, name /* display-name */,
                       xPackageType, bRemoved, identifier)
            {}
    };
    friend class ExecutablePackageImpl;

    typedef ::boost::unordered_map< OUString, Reference<XInterface>,
                             OUStringHash > t_string2object;

    
    virtual Reference<deployment::XPackage> bindPackage_(
        OUString const & url, OUString const & mediaType, sal_Bool bRemoved,
        OUString const & identifier, Reference<XCommandEnvironment> const & xCmdEnv );

    void addDataToDb(OUString const & url);
    bool hasActiveEntry(OUString const & url);
    void revokeEntryFromDb(OUString const & url);

    Reference<deployment::XPackageTypeInfo> m_xExecutableTypeInfo;
    std::auto_ptr<ExecutableBackendDb> m_backendDb;
public:
    BackendImpl( Sequence<Any> const & args,
                 Reference<XComponentContext> const & xComponentContext );

    
    virtual Sequence< Reference<deployment::XPackageTypeInfo> > SAL_CALL
    getSupportedPackageTypes() throw (RuntimeException);
    virtual void SAL_CALL packageRemoved(OUString const & url, OUString const & mediaType)
        throw (deployment::DeploymentException,
               uno::RuntimeException);

    using PackageRegistryBackend::disposing;
};


BackendImpl::BackendImpl(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xComponentContext )
    : PackageRegistryBackend( args, xComponentContext ),
      m_xExecutableTypeInfo(new Package::TypeInfo(
                                "application/vnd.sun.star.executable",
                                "", "Executable", RID_IMG_COMPONENT ) )
{
    if (!transientMode())
    {
        OUString dbFile = makeURL(getCachePath(), "backenddb.xml");
        m_backendDb.reset(
            new ExecutableBackendDb(getComponentContext(), dbFile));
   }
}

void BackendImpl::addDataToDb(OUString const & url)
{
    if (m_backendDb.get())
        m_backendDb->addEntry(url);
}

void BackendImpl::revokeEntryFromDb(OUString const & url)
{
    if (m_backendDb.get())
        m_backendDb->revokeEntry(url);
}

bool BackendImpl::hasActiveEntry(OUString const & url)
{
    if (m_backendDb.get())
        return m_backendDb->hasActiveEntry(url);
    return false;
}



Sequence< Reference<deployment::XPackageTypeInfo> >
BackendImpl::getSupportedPackageTypes() throw (RuntimeException)
{
    return Sequence<Reference<deployment::XPackageTypeInfo> >(
        & m_xExecutableTypeInfo, 1);
}

void BackendImpl::packageRemoved(OUString const & url, OUString const & /*mediaType*/)
        throw (deployment::DeploymentException,
               uno::RuntimeException)
{
    if (m_backendDb.get())
        m_backendDb->removeEntry(url);
}


Reference<deployment::XPackage> BackendImpl::bindPackage_(
    OUString const & url, OUString const & mediaType, sal_Bool bRemoved,
    OUString const & identifier, Reference<XCommandEnvironment> const & xCmdEnv )
{
    if (mediaType.isEmpty())
    {
        throw lang::IllegalArgumentException(
            StrCannotDetectMediaType::get() + url,
            static_cast<OWeakObject *>(this), static_cast<sal_Int16>(-1) );
    }

    OUString type, subType;
    INetContentTypeParameterList params;
    if (INetContentTypes::parse( mediaType, type, subType, &params ))
    {
        if (type.equalsIgnoreAsciiCase("application"))
        {
            OUString name;
            if (!bRemoved)
            {
                ::ucbhelper::Content ucbContent(
                    url, xCmdEnv, getComponentContext() );
                name = StrTitle::getTitle( ucbContent );
            }
            if (subType.equalsIgnoreAsciiCase("vnd.sun.star.executable"))
            {
                return new BackendImpl::ExecutablePackageImpl(
                    this, url, name,  m_xExecutableTypeInfo, bRemoved,
                    identifier);
            }
        }
    }
    return Reference<deployment::XPackage>();
}




BackendImpl * BackendImpl::ExecutablePackageImpl::getMyBackend() const
{
    BackendImpl * pBackend = static_cast<BackendImpl *>(m_myBackend.get());
    if (NULL == pBackend)
    {
        
        check();
        
        throw RuntimeException( "Failed to get the BackendImpl",
            static_cast<OWeakObject*>(const_cast<ExecutablePackageImpl *>(this)));
    }
    return pBackend;
}

beans::Optional< beans::Ambiguous<sal_Bool> >
BackendImpl::ExecutablePackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard &,
    ::rtl::Reference<dp_misc::AbortChannel> const &,
    Reference<XCommandEnvironment> const & )
{
    bool registered = getMyBackend()->hasActiveEntry(getURL());
    return beans::Optional< beans::Ambiguous<sal_Bool> >(
            sal_True /* IsPresent */,
                beans::Ambiguous<sal_Bool>(
                    registered, sal_False /* IsAmbiguous */ ) );
}

void BackendImpl::ExecutablePackageImpl::processPackage_(
    ::osl::ResettableMutexGuard &,
    bool doRegisterPackage,
    bool /*startup*/,
    ::rtl::Reference<dp_misc::AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & /*xCmdEnv*/ )
{
    checkAborted(abortChannel);
    if (doRegisterPackage)
    {
        if (!isUrlTargetInExtension())
        {
            OSL_ASSERT(false);
            return;
        }
        sal_uInt64 attributes = 0;
        
        if (getFileAttributes(attributes))
        {
            if(getMyBackend()->m_context == "user")
                attributes |= osl_File_Attribute_OwnExe;
            else if (getMyBackend()->m_context == "shared")
                attributes |= (osl_File_Attribute_OwnExe | osl_File_Attribute_GrpExe
                               | osl_File_Attribute_OthExe);
            else if (!(getMyBackend()->m_context == "bundled"))
                
                
                OSL_ASSERT(false);

            
            osl::File::setAttributes(
                    dp_misc::expandUnoRcUrl(m_url), attributes);
        }
        getMyBackend()->addDataToDb(getURL());
    }
    else
    {
        getMyBackend()->revokeEntryFromDb(getURL());
    }
}





bool BackendImpl::ExecutablePackageImpl::isUrlTargetInExtension()
{
    bool bSuccess = false;
    OUString sExtensionDir;
    if(getMyBackend()->m_context == "user")
        sExtensionDir = dp_misc::expandUnoRcTerm("$UNO_USER_PACKAGES_CACHE");
    else if (getMyBackend()->m_context == "shared")
        sExtensionDir = dp_misc::expandUnoRcTerm("$UNO_SHARED_PACKAGES_CACHE");
    else if (getMyBackend()->m_context == "bundled")
        sExtensionDir = dp_misc::expandUnoRcTerm("$BUNDLED_EXTENSIONS");
    else
        OSL_ASSERT(false);
    
    if (osl::File::E_None == osl::File::getAbsoluteFileURL(OUString(), sExtensionDir, sExtensionDir))
    {
        OUString sFile;
        if (osl::File::E_None == osl::File::getAbsoluteFileURL(
            OUString(), dp_misc::expandUnoRcUrl(m_url), sFile))
        {
            if (sFile.match(sExtensionDir, 0))
                bSuccess = true;
        }
    }
    return bSuccess;
}

bool BackendImpl::ExecutablePackageImpl::getFileAttributes(sal_uInt64& out_Attributes)
{
    bool bSuccess = false;
    const OUString url(dp_misc::expandUnoRcUrl(m_url));
    osl::DirectoryItem item;
    if (osl::FileBase::E_None == osl::DirectoryItem::get(url, item))
    {
        osl::FileStatus aStatus(osl_FileStatus_Mask_Attributes);
        if( osl::FileBase::E_None == item.getFileStatus(aStatus))
        {
            out_Attributes = aStatus.getAttributes();
            bSuccess = true;
        }
    }
    return bSuccess;
}



} 

namespace sdecl = comphelper::service_decl;
sdecl::class_<BackendImpl, sdecl::with_args<true> > serviceBI;
extern sdecl::ServiceDecl const serviceDecl(
    serviceBI,
    "com.sun.star.comp.deployment.executable.PackageRegistryBackend",
    BACKEND_SERVICE_NAME );

} 
} 
} 


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
