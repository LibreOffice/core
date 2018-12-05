/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_INC_HELPER_FORM_HXX
#define INCLUDED_TEST_INC_HELPER_FORM_HXX

#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <rtl/string.hxx>
#include <sal/config.h>
#include <sal/types.h>
#include <test/testdllapi.hxx>

namespace apitest
{
/** @brief Create a XControlShape
 *
 * @param r_xComponent  The document.
 * @param nHeight       The height of the shape.
 * @param nWidth        The width of the shape.
 * @param nX            The x-position of the shape.
 * @param nY            The y-position of the shape.
 * @param r_aKind       The kind of the shape.
 */
css::uno::Reference<css::drawing::XControlShape> OOO_DLLPUBLIC_TEST createControlShape(
    const css::uno::Reference<css::lang::XComponent>& r_xComponent, const sal_Int32 nHeight,
    const sal_Int32 nWidth, const sal_Int32 nX, const sal_Int32 nY, const OUString& r_aKind);

} // namespace apitest

#endif // INCLUDED_TEST_INC_HELPER_FORM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
