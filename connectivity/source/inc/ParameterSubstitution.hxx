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
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <cppuhelper/implbase3.hxx>

namespace connectivity
{
    typedef ::cppu::WeakImplHelper3< ::com::sun::star::util::XStringSubstitution
                                    ,::com::sun::star::lang::XServiceInfo
                                    ,::com::sun::star::lang::XInitialization > ParameterSubstitution_BASE;
    class ParameterSubstitution : public ParameterSubstitution_BASE
    {
        ::osl::Mutex                                                                    m_aMutex;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >    m_xContext;
        ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XConnection >     m_xConnection;

        ParameterSubstitution( const ParameterSubstitution& );
        ParameterSubstitution& operator=( const ParameterSubstitution& );
    public:

        static OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static(  ) throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > create( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >  & xContext);
    protected:
        ParameterSubstitution(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext );
        virtual ~ParameterSubstitution(){}

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);
        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XStringSubstitution
        virtual OUString SAL_CALL substituteVariables( const OUString& aText, ::sal_Bool bSubstRequired ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL reSubstituteVariables( const OUString& aText ) throw (::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL getSubstituteVariableValue( const OUString& variable ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    };
// ==================================
} // connectivity
// ==================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
