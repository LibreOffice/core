/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <cppuhelper/implbase.hxx>
#include <sal/config.h>
#include <scriptinterop/XPresentation.hpp>

#include "presentation.hxx"

namespace
{
class PresentationImpl : public cppu::WeakImplHelper<scriptinterop::XPresentation>
{
public:
    explicit PresentationImpl(css::uno::Reference<css::frame::XModel> const& model)
        : model_(model)
    {
    }

    css::uno::Reference<css::uno::XInterface> SAL_CALL getuno() override { return model_; }

private:
    css::uno::Reference<css::frame::XModel> model_;
};
}

namespace scriptinterop::detail
{
css::uno::Reference<scriptinterop::XPresentation>
createPresentation(css::uno::Reference<css::frame::XModel> const& model)
{
    return new PresentationImpl(model);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
