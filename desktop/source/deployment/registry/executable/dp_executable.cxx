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
using ::rtl::OUString;

namespace dp_registry {
namespace backend {
namespace executable {
namespace {

class BackendImpl : public ::dp_registry::backend::PackageRegistryBackend
{
    class ExecutablePackageImpl : public ::dp_registry::backend::Package
    {
        BackendImpl * getMyBackend() const;

        // Package
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
                             ::rtl::OUStringHash > t_string2object;

    // PackageRegistryBackend
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

    // XPackageRegistry
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
                                OUSTR("application/vnd.sun.star.executable"),
                                OUSTR(""),
                                OUSTR("Executable"),
                                RID_IMG_COMPONENT ) )
{
    if (!transientMode())
    {
        OUString dbFile = makeURL(getCachePath(), OUSTR("backenddb.xml"));
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


// XPackageRegistry
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

// PackageRegistryBackend
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

    String type, subType;
    INetContentTypeParameterList params;
    if (INetContentTypes::parse( mediaType, type, subType, &params ))
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
            if (subType.EqualsIgnoreCaseAscii("vnd.sun.star.executable"))
            {
                return new BackendImpl::ExecutablePackageImpl(
                    this, url, name,  m_xExecutableTypeInfo, bRemoved,
                    identifier);
            }
        }
    }
    return Reference<deployment::XPackage>();
}



// Package
BackendImpl * BackendImpl::ExecutablePackageImpl::getMyBackend() const
{
    BackendImpl * pBackend = static_cast<BackendImpl *>(m_myBackend.get());
    if (NULL == pBackend)
    {
        //May throw a DisposedException
        check();
        //We should never get here...
        throw RuntimeException(
            OUSTR("Failed to get the BackendImpl"),
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
            OSL_ASSERT(0);
            return;
        }
        sal_uInt64 attributes = 0;
        //Setting the executable attribut does not affect executables on Windows
        if (getFileAttributes(attributes))
        {
            if(getMyBackend()->m_context.equals(OUSTR("user")))
                attributes |= osl_File_Attribute_OwnExe;
            else if (getMyBackend()->m_context.equals(OUSTR("shared")))
                attributes |= (osl_File_Attribute_OwnExe | osl_File_Attribute_GrpExe
                               | osl_File_Attribute_OthExe);
            else if (!getMyBackend()->m_context.equals(OUSTR("bundled")))
                //Bundled extension are required to be in the properly
                //installed. That is an executable must have the right flags
                OSL_ASSERT(0);

            //This won't have affect on Windows
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

//We currently cannot check if this XPackage represents a content of a particular extension
//But we can check if we are within $UNO_USER_PACKAGES_CACHE etc.
//Done for security reasons. For example an extension manifest could contain a path to
//an executable outside the extension.
bool BackendImpl::ExecutablePackageImpl::isUrlTargetInExtension()
{
    bool bSuccess = false;
    OUString sExtensionDir;
    if(getMyBackend()->m_context.equals(OUSTR("user")))
        sExtensionDir = dp_misc::expandUnoRcTerm(OUSTR("$UNO_USER_PACKAGES_CACHE"));
    else if (getMyBackend()->m_context.equals(OUSTR("shared")))
        sExtensionDir = dp_misc::expandUnoRcTerm(OUSTR("$UNO_SHARED_PACKAGES_CACHE"));
    else if (getMyBackend()->m_context.equals(OUSTR("bundled")))
        sExtensionDir = dp_misc::expandUnoRcTerm(OUSTR("$BUNDLED_EXTENSIONS"));
    else
        OSL_ASSERT(0);
    //remove file ellipses
    if (osl::File::E_None == osl::File::getAbsoluteFileURL(OUString(), sExtensionDir, sExtensionDir))
    {
        OUString sFile;
        if (osl::File::E_None == osl::File::getAbsoluteFileURL(
            OUString(), dp_misc::expandUnoRcUrl(m_url), sFile))
        {
            if (sal_True == sFile.match(sExtensionDir, 0))
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



} // anon namespace

namespace sdecl = comphelper::service_decl;
sdecl::class_<BackendImpl, sdecl::with_args<true> > serviceBI;
extern sdecl::ServiceDecl const serviceDecl(
    serviceBI,
    "com.sun.star.comp.deployment.executable.PackageRegistryBackend",
    BACKEND_SERVICE_NAME );

} // namespace component
} // namespace backend
} // namespace dp_registry


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
