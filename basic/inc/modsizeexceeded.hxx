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

#ifndef _BASIC_MODSIZEEXCEEDED_HXX
#define _BASIC_MODSIZEEXCEEDED_HXX

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <cppuhelper/implbase1.hxx>

class ModuleSizeExceeded : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionRequest >
{
    // C++ interface
    public:
    ModuleSizeExceeded( const com::sun::star::uno::Sequence< ::rtl::OUString>& sModules );

    sal_Bool isAbort() const;
    sal_Bool isApprove() const;

    // UNO interface
    public:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< com::sun::star::task::XInteractionContinuation > > SAL_CALL getContinuations() throw( ::com::sun::star::uno::RuntimeException ) { return m_lContinuations; }
    com::sun::star::uno::Any SAL_CALL getRequest() throw( com::sun::star::uno::RuntimeException )
    {
        return m_aRequest;
    }

    // member
    private:
    rtl::OUString m_sMods;
    com::sun::star::uno::Any m_aRequest;
    com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::task::XInteractionContinuation > > m_lContinuations;
    com::sun::star::uno::Reference< com::sun::star::task::XInteractionContinuation > m_xAbort;
    com::sun::star::uno::Reference< com::sun::star::task::XInteractionContinuation> m_xApprove;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
