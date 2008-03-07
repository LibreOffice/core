/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_executable.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 11:05:26 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "dp_backend.h"
#include "dp_ucb.h"
#include "dp_interact.h"
#include "rtl/string.hxx"
#include "osl/file.hxx"
#include "ucbhelper/content.hxx"
#include "comphelper/servicedecl.hxx"
#include "svtools/inettype.hxx"
#include "cppuhelper/implbase1.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace dp_registry {
namespace backend {
namespace executable {
namespace {

class BackendImpl : public ::dp_registry::backend::PackageRegistryBackend
{
    class ExecutablePackageImpl : public ::dp_registry::backend::Package
    {
        BackendImpl * getMyBackend() const {
            return static_cast<BackendImpl *>(m_myBackend.get());
        }

        // Package
        virtual beans::Optional< beans::Ambiguous<sal_Bool> > isRegistered_(
            ::osl::ResettableMutexGuard & guard,
            ::rtl::Reference<dp_misc::AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv );
        virtual void processPackage_(
            ::osl::ResettableMutexGuard & guard,
            bool registerPackage,
            ::rtl::Reference<dp_misc::AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv );

        bool getFileAttributes(sal_uInt64& out_Attributes);
        bool isUrlTargetInExtension();

    public:
        inline ExecutablePackageImpl(
            ::rtl::Reference<PackageRegistryBackend> const & myBackend,
            OUString const & url, OUString const & name,
            Reference<deployment::XPackageTypeInfo> const & xPackageType)
            : Package( myBackend, url, name, name /* display-name */,
                       xPackageType ) //,
            {}
    };
    friend class ExecutablePackageImpl;

    typedef ::std::hash_map< OUString, Reference<XInterface>,
                             ::rtl::OUStringHash > t_string2object;

    // PackageRegistryBackend
    virtual Reference<deployment::XPackage> bindPackage_(
        OUString const & url, OUString const & mediaType,
        Reference<XCommandEnvironment> const & xCmdEnv );

    Reference<deployment::XPackageTypeInfo> m_xExecutableTypeInfo;

public:
    BackendImpl( Sequence<Any> const & args,
                 Reference<XComponentContext> const & xComponentContext );

    // XPackageRegistry
    virtual Sequence< Reference<deployment::XPackageTypeInfo> > SAL_CALL
    getSupportedPackageTypes() throw (RuntimeException);

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
                                RID_IMG_COMPONENT,
                                RID_IMG_COMPONENT_HC ) )
{

}

// XPackageRegistry
Sequence< Reference<deployment::XPackageTypeInfo> >
BackendImpl::getSupportedPackageTypes() throw (RuntimeException)
{
    return Sequence<Reference<deployment::XPackageTypeInfo> >(
        & m_xExecutableTypeInfo, 1);
}

// PackageRegistryBackend
Reference<deployment::XPackage> BackendImpl::bindPackage_(
    OUString const & url, OUString const & mediaType,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    if (mediaType.getLength() == 0)
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
            ::ucbhelper::Content ucbContent( url, xCmdEnv );
            const OUString name( ucbContent.getPropertyValue(
                dp_misc::StrTitle::get() ).get<OUString>() );
            if (subType.EqualsIgnoreCaseAscii("vnd.sun.star.executable"))
            {
                return new BackendImpl::ExecutablePackageImpl(
                    this, url, name,  m_xExecutableTypeInfo);
            }
        }
    }
    return Reference<deployment::XPackage>();
}

//##############################################################################


// Package
beans::Optional< beans::Ambiguous<sal_Bool> >
BackendImpl::ExecutablePackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard &,
    ::rtl::Reference<dp_misc::AbortChannel> const &,
    Reference<XCommandEnvironment> const & )
{
    //We must return Optional.isPresent = true, otherwise
    //processPackage is not called.
    //The user shall not be able to enable/disable the executable. This is not needed since
    //the executable does not affect the office. The best thing is to show no
    //status at all. See also BackendImpl::PackageImpl::isRegistered_ (dp_package.cxx)
    //On Windows there is no executable file attribute. One has to use security API for this.
    //However, on Windows we do not have the problem, that after unzipping the file cannot be
    //executed.
    return beans::Optional< beans::Ambiguous<sal_Bool> >(
            sal_True /* IsPresent */,
                beans::Ambiguous<sal_Bool>(
                   sal_True, sal_True /* IsAmbiguous */ ) );
}

void BackendImpl::ExecutablePackageImpl::processPackage_(
    ::osl::ResettableMutexGuard &,
    bool doRegisterPackage,
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
            else
                OSL_ASSERT(0);

            //This won't have affect on Windows
            if (osl::File::E_None != osl::File::setAttributes(
                    dp_misc::expandUnoRcUrl(m_url), attributes))
                OSL_ENSURE(0, "Extension Manager: Could not set executable file attribute.");
        }
    }
}

//We currently cannot check if this XPackage represents a content of a particular exension
//But we can check if we are within $UNO_USER_PACKAGES_CACHE etc.
bool BackendImpl::ExecutablePackageImpl::isUrlTargetInExtension()
{
    bool bSuccess = false;
    OUString sExtensionDir;
    if(getMyBackend()->m_context.equals(OUSTR("user")))
        sExtensionDir = dp_misc::expandUnoRcTerm(OUSTR("$UNO_USER_PACKAGES_CACHE"));
    else if (getMyBackend()->m_context.equals(OUSTR("shared")))
        sExtensionDir = dp_misc::expandUnoRcTerm(OUSTR("$UNO_SHARED_PACKAGES_CACHE"));
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

//##############################################################################


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


