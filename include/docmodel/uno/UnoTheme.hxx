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
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/servicehelper.hxx>

#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/util/XTheme.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <utility>
#include <docmodel/dllapi.h>
#include <docmodel/theme/Theme.hxx>

class DOCMODEL_DLLPUBLIC UnoTheme final : public cppu::WeakImplHelper<css::util::XTheme>
{
private:
    model::Theme maTheme;

public:
    UnoTheme(model::Theme const& rTheme)
        : maTheme(rTheme)
    {
    }

    model::Theme const& getTheme() const { return maTheme; }

    // XTheme
    OUString SAL_CALL getName() override;
    css::uno::Sequence<sal_Int32> SAL_CALL getColorSet() override;
};

namespace model::theme
{
DOCMODEL_DLLPUBLIC css::uno::Reference<css::util::XTheme> createXTheme(model::Theme const& rTheme);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
