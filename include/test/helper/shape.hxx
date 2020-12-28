/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_HELPER_SHAPE_HXX
#define INCLUDED_TEST_HELPER_SHAPE_HXX

#include <sal/config.h>

#include <string_view>

#include <sal/types.h>
#include <test/testdllapi.hxx>

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <com/sun/star/uno/Reference.hxx>

namespace apitest
{
namespace helper
{
namespace shape
{
/** @brief Create a XShape of kind "Ellipse".
 *
 * @param r_xComponent  The document
 * @param nX            The x-position of the shape.
 * @param nY            The y-position of the shape.
 * @param nHeight       The height of the shape.
 * @param nWidth        The width of the shape.
 * @return  The created XShape.
 */
css::uno::Reference<css::drawing::XShape>
    OOO_DLLPUBLIC_TEST createEllipse(const css::uno::Reference<css::lang::XComponent>& r_xComponent,
                                     const sal_Int32 nX, const sal_Int32 nY, const sal_Int32 nWidth,
                                     const sal_Int32 nHeight);

/** @brief Create a XShape of kind "Line".
 *
 * @param r_xComponent  The document
 * @param nX            The x-position of the shape.
 * @param nY            The y-position of the shape.
 * @param nHeight       The height of the shape.
 * @param nWidth        The width of the shape.
 * @return  The created XShape.
 */
css::uno::Reference<css::drawing::XShape>
    OOO_DLLPUBLIC_TEST createLine(const css::uno::Reference<css::lang::XComponent>& r_xComponent,
                                  const sal_Int32 nX, const sal_Int32 nY, const sal_Int32 nWidth,
                                  const sal_Int32 nHeight);

/** @brief Create a XShape of kind "Rectangle".
 *
 * @param r_xComponent  The document
 * @param nX            The x-position of the shape.
 * @param nY            The y-position of the shape.
 * @param nHeight       The height of the shape.
 * @param nWidth        The width of the shape.
 * @return  The created XShape.
 */
css::uno::Reference<css::drawing::XShape> OOO_DLLPUBLIC_TEST
createRectangle(const css::uno::Reference<css::lang::XComponent>& r_xComponent, const sal_Int32 nX,
                const sal_Int32 nY, const sal_Int32 nWidth, const sal_Int32 nHeight);

/** @brief Create a XShape.
 *
 * @param r_xComponent  The document
 * @param r_aKind       The kind of shape.
 * @param nX            The x-position of the shape.
 * @param nY            The y-position of the shape.
 * @param nHeight       The height of the shape.
 * @param nWidth        The width of the shape.
 * @return  The created XShape.
 */
css::uno::Reference<css::drawing::XShape> OOO_DLLPUBLIC_TEST createShape(
    const css::uno::Reference<css::lang::XComponent>& r_xComponent, std::u16string_view r_aKind,
    const sal_Int32 nX, const sal_Int32 nY, const sal_Int32 nWidth, const sal_Int32 nHeight);

} // namespace shape
} // namespace helper
} // namespace apitest

#endif // INCLUDED_TEST_HELPER_SHAPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
