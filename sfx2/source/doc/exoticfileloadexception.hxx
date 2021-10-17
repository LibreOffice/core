/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_EXOTICFILELOADEXCEPTION_HXX
#define INCLUDED_SFX2_EXOTICFILELOADEXCEPTION_HXX

#include <com/sun/star/task/XInteractionContinuation.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <cppuhelper/implbase.hxx>

class ExoticFileLoadException : public cppu::WeakImplHelper<css::task::XInteractionRequest>
{
    // C++ interface
public:
    ExoticFileLoadException(const OUString& rURL, const OUString& rFilterUIName);
    bool isApprove() const;

    // UNO interface
public:
    virtual css::uno::Sequence<css::uno::Reference<css::task::XInteractionContinuation>>
        SAL_CALL getContinuations() override
    {
        return m_lContinuations;
    }
    css::uno::Any SAL_CALL getRequest() override { return m_aRequest; }

    // member
private:
    css::uno::Any m_aRequest;
    css::uno::Reference<css::task::XInteractionContinuation> m_xAbort;
    css::uno::Reference<css::task::XInteractionContinuation> m_xApprove;
    css::uno::Sequence<css::uno::Reference<css::task::XInteractionContinuation>> m_lContinuations;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
