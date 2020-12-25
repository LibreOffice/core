/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <graphic/UnoBinaryDataContainer.hxx>

#include <cppuhelper/queryinterface.hxx>

using namespace css;

// css::lang::XUnoTunnel
UNO3_GETIMPLEMENTATION_IMPL(UnoBinaryDataContainer);

css::uno::Sequence<sal_Int8> SAL_CALL UnoBinaryDataContainer::getCopyAsByteSequence()
{
    if (maBinaryDataContainer.isEmpty())
        return css::uno::Sequence<sal_Int8>();

    size_t nSize = maBinaryDataContainer.getSize();

    css::uno::Sequence<sal_Int8> aData(nSize);

    std::copy(maBinaryDataContainer.cbegin(), maBinaryDataContainer.cend(), aData.getArray());

    return aData;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
