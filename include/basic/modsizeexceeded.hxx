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

#include <com/sun/star/task/XInteractionRequest.hpp>
#include <cppuhelper/implbase.hxx>
#include <basic/basicdllapi.h>
#include <vector>

namespace com::sun::star::task { class XInteractionContinuation; }

class BASIC_DLLPUBLIC ModuleSizeExceeded : public cppu::WeakImplHelper< css::task::XInteractionRequest >
{
// C++ interface
public:
    ModuleSizeExceeded( const std::vector<OUString>& sModules );

    bool isAbort() const;
    bool isApprove() const;

// UNO interface
public:
    virtual css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > SAL_CALL getContinuations() override { return m_lContinuations; }
    css::uno::Any SAL_CALL getRequest() override
    {
        return m_aRequest;
    }

// member
private:
    css::uno::Any m_aRequest;
    css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > m_lContinuations;
    css::uno::Reference< css::task::XInteractionContinuation > m_xAbort;
    css::uno::Reference< css::task::XInteractionContinuation> m_xApprove;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
