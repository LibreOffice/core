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

#ifndef _SVTOOLS_JAVACONTEXT_HXX_
#define _SVTOOLS_JAVACONTEXT_HXX_

#include "bf_svtools/svtdllapi.h"
#include <osl/mutex.hxx>

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/uno/XCurrentContext.hpp>


namespace binfilter
{
// We cannot derive from  cppu::WeakImplHelper because we would export the inline
//generated class. This conflicts with other libraries if they use the same inline
//class.
    class  JavaContext :        
        public com::sun::star::uno::XCurrentContext
    {
        
    public:
        JavaContext( const com::sun::star::uno::Reference<
                     com::sun::star::uno::XCurrentContext> & ctx);

        /** The parameter bShowErrorsOnce controls whether a message box is
            only displayed once for a reocurring Java error. That is only
            the first time JavaInteractionHandler.handle is called with a
            particular Request then the message box is shown. Afterwards
            nothing happens.
         */
        JavaContext( const com::sun::star::uno::Reference<
                     com::sun::star::uno::XCurrentContext> & ctx,
                     bool bReportErrorOnce);
        virtual ~JavaContext();

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
            const ::com::sun::star::uno::Type& aType )
            throw (::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL acquire() throw ();

        virtual void SAL_CALL release() throw ();

        // XCurrentContext
        virtual com::sun::star::uno::Any SAL_CALL getValueByName( const rtl::OUString& Name )
            throw (com::sun::star::uno::RuntimeException);

    private:
        JavaContext(); //not implemented
        JavaContext(JavaContext&); //not implemented
        JavaContext& operator = (JavaContext&); //not implemented
        
        oslInterlockedCount	m_aRefCount;

        com::sun::star::uno::Reference<
            com::sun::star::uno::XCurrentContext > m_xNextContext;
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionHandler> m_xHandler;
        bool m_bShowErrorsOnce;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
