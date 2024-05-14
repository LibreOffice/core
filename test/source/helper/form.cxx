/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/helper/form.hxx>
#include <sal/types.h>

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

using namespace css;

namespace apitest::helper::form
{
uno::Reference<drawing::XControlShape>
    OOO_DLLPUBLIC_TEST createCommandButton(const uno::Reference<lang::XComponent>& r_xComponent,
                                           const sal_Int32 nX, const sal_Int32 nY,
                                           const sal_Int32 nHeight, const sal_Int32 nWidth)
{
    return createControlShape(r_xComponent, u"CommandButton", nX, nY, nHeight, nWidth);
}

uno::Reference<drawing::XControlShape> OOO_DLLPUBLIC_TEST createControlShape(
    const uno::Reference<lang::XComponent>& r_xComponent, std::u16string_view r_aKind,
    const sal_Int32 nX, const sal_Int32 nY, const sal_Int32 nHeight, const sal_Int32 nWidth)
{
    uno::Reference<lang::XMultiServiceFactory> xMSF(r_xComponent, uno::UNO_QUERY_THROW);

    uno::Reference<drawing::XControlShape> xControlShape(
        xMSF->createInstance(u"com.sun.star.drawing.ControlShape"_ustr), uno::UNO_QUERY_THROW);

    uno::Reference<uno::XInterface> aComponent(
        xMSF->createInstance(OUString::Concat("com.sun.star.form.component.") + r_aKind),
        uno::UNO_SET_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(aComponent, uno::UNO_QUERY_THROW);
    xPropertySet->setPropertyValue(
        u"DefaultControl"_ustr, uno::Any(OUString::Concat("com.sun.star.form.control.") + r_aKind));
    uno::Reference<awt::XControlModel> xControlModel(aComponent, uno::UNO_QUERY_THROW);

    xControlShape->setSize(awt::Size(nHeight, nWidth));
    xControlShape->setPosition(awt::Point(nX, nY));

    xControlShape->setControl(xControlModel);

    return xControlShape;
}

} // namespace apitest::helper::form

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
