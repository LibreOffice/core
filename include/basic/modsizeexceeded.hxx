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

#ifndef INCLUDED_BASIC_MODSIZEEXCEEDED_HXX
#define INCLUDED_BASIC_MODSIZEEXCEEDED_HXX

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <cppuhelper/implbase1.hxx>
#include <basic/basicdllapi.h>

class BASIC_DLLPUBLIC ModuleSizeExceeded : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionRequest >
{
    // C++ interface
    public:
    ModuleSizeExceeded( const com::sun::star::uno::Sequence<OUString>& sModules );

    bool isAbort() const;
    bool isApprove() const;

    // UNO interface
    public:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< com::sun::star::task::XInteractionContinuation > > SAL_CALL getContinuations() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override { return m_lContinuations; }
    com::sun::star::uno::Any SAL_CALL getRequest() throw( com::sun::star::uno::RuntimeException, std::exception ) override
    {
        return m_aRequest;
    }

    // member
    private:
    com::sun::star::uno::Any m_aRequest;
    com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::task::XInteractionContinuation > > m_lContinuations;
    com::sun::star::uno::Reference< com::sun::star::task::XInteractionContinuation > m_xAbort;
    com::sun::star::uno::Reference< com::sun::star::task::XInteractionContinuation> m_xApprove;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
