/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/util/XTheme.hpp>

#include <docmodel/dllapi.h>

namespace model
{
class Theme;
}

class DOCMODEL_DLLPUBLIC UnoTheme final : public cppu::WeakImplHelper<css::util::XTheme>
{
private:
    std::shared_ptr<model::Theme> mpTheme;

public:
    UnoTheme(std::shared_ptr<model::Theme> const& pTheme)
        : mpTheme(pTheme)
    {
    }

    std::shared_ptr<model::Theme> const& getTheme() const { return mpTheme; }

    // XTheme
    OUString SAL_CALL getName() override;
    css::uno::Sequence<sal_Int32> SAL_CALL getColorSet() override;
};

namespace model::theme
{
DOCMODEL_DLLPUBLIC css::uno::Reference<css::util::XTheme>
createXTheme(std::shared_ptr<model::Theme> const& pTheme);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
