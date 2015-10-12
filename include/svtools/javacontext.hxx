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

#ifndef INCLUDED_SVTOOLS_JAVACONTEXT_HXX
#define INCLUDED_SVTOOLS_JAVACONTEXT_HXX

#include <svtools/svtdllapi.h>
#include <osl/mutex.hxx>

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/uno/XCurrentContext.hpp>


namespace svt
{
// We cannot derive from  cppu::WeakImplHelper because we would export the inline
//generated class. This conflicts with other libraries if they use the same inline
//class.
    class SVT_DLLPUBLIC JavaContext :
        public com::sun::star::uno::XCurrentContext
    {

    public:
        /** The parameter bShowErrorsOnce controls whether a message box is
            only displayed once for a reocurring Java error. That is only
            the first time JavaInteractionHandler.handle is called with a
            particular Request then the message box is shown. Afterwards
            nothing happens.
         */
        JavaContext( const com::sun::star::uno::Reference<
                     com::sun::star::uno::XCurrentContext> & ctx,
                     bool bReportErrorOnce = true );
        virtual ~JavaContext();

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
            const ::com::sun::star::uno::Type& aType )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual void SAL_CALL acquire() throw () SAL_OVERRIDE;

        virtual void SAL_CALL release() throw () SAL_OVERRIDE;

        // XCurrentContext
        virtual com::sun::star::uno::Any SAL_CALL getValueByName( const OUString& Name )
            throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    private:
        JavaContext(JavaContext&) = delete;
        JavaContext& operator = (JavaContext&) = delete;

        oslInterlockedCount m_aRefCount;

        com::sun::star::uno::Reference<
            com::sun::star::uno::XCurrentContext > m_xNextContext;
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionHandler> m_xHandler;
        bool m_bShowErrorsOnce;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
