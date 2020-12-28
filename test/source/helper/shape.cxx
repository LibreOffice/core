/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/helper/shape.hxx>
#include <sal/types.h>

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyVetoException.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;

namespace apitest::helper::shape
{
uno::Reference<drawing::XShape>
    OOO_DLLPUBLIC_TEST createEllipse(const uno::Reference<lang::XComponent>& r_xComponent,
                                     const sal_Int32 nX, const sal_Int32 nY, const sal_Int32 nWidth,
                                     const sal_Int32 nHeight)
{
    return createShape(r_xComponent, u"Ellipse", nX, nY, nWidth, nHeight);
}

uno::Reference<drawing::XShape>
    OOO_DLLPUBLIC_TEST createLine(const uno::Reference<lang::XComponent>& r_xComponent,
                                  const sal_Int32 nX, const sal_Int32 nY, const sal_Int32 nWidth,
                                  const sal_Int32 nHeight)
{
    return createShape(r_xComponent, u"Line", nX, nY, nWidth, nHeight);
}

uno::Reference<drawing::XShape>
    OOO_DLLPUBLIC_TEST createRectangle(const uno::Reference<lang::XComponent>& r_xComponent,
                                       const sal_Int32 nX, const sal_Int32 nY,
                                       const sal_Int32 nWidth, const sal_Int32 nHeight)
{
    return createShape(r_xComponent, u"Rectangle", nX, nY, nWidth, nHeight);
}

uno::Reference<drawing::XShape> OOO_DLLPUBLIC_TEST
createShape(const uno::Reference<lang::XComponent>& r_xComponent, std::u16string_view r_aKind,
            const sal_Int32 nX, const sal_Int32 nY, const sal_Int32 nWidth, const sal_Int32 nHeight)
{
    uno::Reference<lang::XMultiServiceFactory> xMSF(r_xComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape(
        xMSF->createInstance(OUString::Concat("com.sun.star.drawing.") + r_aKind + "Shape"),
        uno::UNO_QUERY_THROW);

    try
    {
        xShape->setPosition(awt::Point(nX, nY));
        xShape->setSize(awt::Size(nWidth, nHeight));
    }
    catch (const beans::PropertyVetoException&)
    {
    }

    return xShape;
}

} // namespace apitest::helper::shape

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
