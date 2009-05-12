/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: extensionmigration.hxx,v $
 * $Revision: 1.2 $
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

#ifndef _DESKTOP_EXTENSIONMIGRATION_HXX_
#define _DESKTOP_EXTENSIONMIGRATION_HXX_

#include "misc.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/implbase3.hxx>
#include <osl/mutex.hxx>
#include <osl/file.hxx>
#include "xmlscript/xmllib_imexp.hxx"

namespace com { namespace sun { namespace star {
    namespace uno {
        class XComponentContext;
    }
    namespace deployment {
        class XPackage;
    }
}}}



class INetURLObject;


//.........................................................................
namespace migration
{
//.........................................................................

    ::rtl::OUString SAL_CALL ExtensionMigration_getImplementationName();
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL ExtensionMigration_getSupportedServiceNames();
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ExtensionMigration_create(
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext )
        SAL_THROW( (::com::sun::star::uno::Exception) );


    // =============================================================================
    // class ExtensionMigration
    // =============================================================================

    typedef ::cppu::WeakImplHelper3<
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::lang::XInitialization,
        ::com::sun::star::task::XJob > ExtensionMigration_BASE;

    class ExtensionMigration : public ExtensionMigration_BASE
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_ctx;
        ::osl::Mutex            m_aMutex;
        ::rtl::OUString         m_sSourceDir;
        ::rtl::OUString         m_sTargetDir;

        ::xmlscript::LibDescriptorArray m_scriptElements;
        ::xmlscript::LibDescriptorArray m_dialogElements;

        ::osl::FileBase::RC     checkAndCreateDirectory( INetURLObject& rDirURL );
        void                    copyConfig( const ::rtl::OUString& sSourceDir, const ::rtl::OUString& sTargetDir );
        bool                    isCompatibleBerkleyDb(const ::rtl::OUString& sSourceDir);
        bool                    copy( const ::rtl::OUString& sSourceDir, const ::rtl::OUString& sTargetDir );
        bool                    processExtensions( const ::rtl::OUString& sSourceDir,
                                    const ::rtl::OUString& sTargetDir );
        /* fills m_scriptElements and  m_dialogElements
        */
        void prepareBasicLibs();
        void prepareBasicLibs(const ::rtl::OUString & sURL,
            ::xmlscript::LibDescriptorArray & out_elements);
        bool isBasicPackageEnabled( const ::com::sun::star::uno::Reference<
            ::com::sun::star::deployment::XPackage > & xPkg);
        void registerBasicPackage(
            const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > & xPkg);
        void registerConfigurationPackage(
            const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > & xPkg);

    public:
        ExtensionMigration(::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > const & ctx);
        virtual ~ExtensionMigration();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName()
            throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& rServiceName )
            throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
            throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XJob
        virtual ::com::sun::star::uno::Any SAL_CALL execute(
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Arguments )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException);
    };

//.........................................................................
}   // namespace migration
//.........................................................................

#endif // _DESKTOP_AUTOCORRMIGRATION_HXX_
