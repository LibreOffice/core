/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/view/xcontrolaccess.hxx>
#include <test/helper/form.hxx>

#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/view/XControlAccess.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace css;

namespace apitest
{
void XControlAccess::testGetControl()
{
    uno::Reference<view::XControlAccess> xCA(init(), uno::UNO_QUERY_THROW);

    uno::Reference<lang::XComponent> xComponent(getXComponent(), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPagesSupplier> xDPS(xComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xDP(xDPS->getDrawPages()->getByIndex(0),
                                           uno::UNO_QUERY_THROW);

    uno::Reference<drawing::XShapes> xShapes(xDP, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape(
        helper::form::createControlShape(xComponent, u"CommandButton", 10000, 50000, 100, 100),
        uno::UNO_QUERY_THROW);

    xShapes->add(xShape);

    uno::Reference<drawing::XControlShape> xCS(xShape, uno::UNO_QUERY_THROW);
    uno::Reference<awt::XControlModel> xCM(xCS->getControl(), uno::UNO_SET_THROW);
    uno::Reference<awt::XControl> xControl(xCA->getControl(xCM), uno::UNO_SET_THROW);

    CPPUNIT_ASSERT(xControl.is());
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
