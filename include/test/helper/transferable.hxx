/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <rtl/string.hxx>
#include <test/testdllapi.hxx>

namespace apitest
{
namespace helper
{
namespace transferable
{
OString OOO_DLLPUBLIC_TEST getTextSelection(
    const css::uno::Reference<css::datatransfer::XTransferable>& xTransferable, OString mimeType);
} // namespace transferable
} // namespace helper
} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
