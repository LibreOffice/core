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
    return maBinaryDataContainer.getAsSequence();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
