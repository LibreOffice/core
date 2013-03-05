/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef EXTENSIONS_ABP_UNODIALOG_HXX
#define EXTENSIONS_ABP_UNODIALOG_HXX

#include <svtools/genericunodialog.hxx>
#include <comphelper/proparrhlp.hxx>
#include "componentmodule.hxx"
#include <com/sun/star/task/XJob.hpp>
#include <cppuhelper/implbase1.hxx>

//.........................................................................
namespace abp
{
//.........................................................................

    //=====================================================================
    //= OABSPilotUno
    //=====================================================================
    class OABSPilotUno;
    typedef ::svt::OGenericUnoDialog                                OABSPilotUno_DBase;
    typedef ::cppu::ImplHelper1< ::com::sun::star::task::XJob >     OABSPilotUno_JBase;
    typedef ::comphelper::OPropertyArrayUsageHelper< OABSPilotUno > OABSPilotUno_PBase;
    /// the UNO wrapper for the address book source pilot
    class OABSPilotUno
            :public OModuleResourceClient
            ,public OABSPilotUno_DBase
            ,public OABSPilotUno_JBase
            ,public OABSPilotUno_PBase
    {
        ::rtl::OUString m_sDataSourceName;
        OABSPilotUno(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);

    public:
        // XInterface (disambiguation)
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire(  ) throw ();
        virtual void SAL_CALL release(  ) throw ();

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
        virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo - static methods
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::rtl::OUString getImplementationName_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >&);

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

        // XJob
        virtual ::com::sun::star::uno::Any SAL_CALL execute( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& lArgs ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XInitialisation
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        using OABSPilotUno_DBase::execute;
    protected:
    // OGenericUnoDialog overridables
        virtual Dialog* createDialog(Window* _pParent);
        virtual void executedDialog(sal_Int16 _nExecutionResult);
    };

//.........................................................................
}   // namespace abp
//.........................................................................

#endif // EXTENSIONS_ABP_UNODIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
