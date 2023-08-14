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
 */

#pragma once

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/util/XBinaryDataContainer.hpp>

#include <utility>
#include <vcl/BinaryDataContainer.hxx>

class UnoBinaryDataContainer final : public cppu::WeakImplHelper<css::util::XBinaryDataContainer>
{
private:
    BinaryDataContainer maBinaryDataContainer;

public:
    UnoBinaryDataContainer(BinaryDataContainer aBinaryDataContainer)
        : maBinaryDataContainer(std::move(aBinaryDataContainer))
    {
    }

    BinaryDataContainer const& getBinaryDataContainer() const { return maBinaryDataContainer; }

    // XBinaryDataContainer
    css::uno::Sequence<sal_Int8> SAL_CALL getCopyAsByteSequence() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
