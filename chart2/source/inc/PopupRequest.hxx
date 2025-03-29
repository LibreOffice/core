/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <config_options.h>

#include <comphelper/compbase.hxx>
#include <com/sun/star/awt/XRequestCallback.hpp>

namespace chart
{
namespace impl
{
typedef comphelper::WeakComponentImplHelper<css::awt::XRequestCallback> PopupRequest_Base;
}

class PopupRequest final : public impl::PopupRequest_Base
{
public:
    explicit PopupRequest();
    virtual ~PopupRequest() override;

    css::uno::Reference<css::awt::XCallback> const& getCallback() const { return m_xCallback; }

    // ____ XRequestCallback ____
    virtual void SAL_CALL addCallback(const css::uno::Reference<::css::awt::XCallback>& xCallback,
                                      const css::uno::Any& aData) override;

private:
    css::uno::Reference<css::awt::XCallback> m_xCallback;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
