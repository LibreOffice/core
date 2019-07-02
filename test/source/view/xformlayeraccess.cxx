/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/view/xformlayeraccess.hxx>

#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/runtime/XFormController.hpp>
#include <com/sun/star/view/XFormLayerAccess.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;

namespace apitest
{
void XFormLayerAccess::testGetFormController()
{
    uno::Reference<view::XFormLayerAccess> xFLA(init(), uno::UNO_QUERY_THROW);

    const bool bCurrentMode = xFLA->isFormDesignMode();

    // delibritly don't use UNO_QUERY_THROW, so we can use
    // uno::XInterface::is() in CPPUNIT_ASSERT()
    xFLA->setFormDesignMode(false);
    uno::Reference<form::runtime::XFormController> xFC = xFLA->getFormController(m_xForm);
    CPPUNIT_ASSERT(xFC.is());

    xFLA->setFormDesignMode(bCurrentMode);
}

void XFormLayerAccess::testIsFormDesignMode() { testSetFormDesignMode(); }

void XFormLayerAccess::testSetFormDesignMode()
{
    uno::Reference<view::XFormLayerAccess> xFLA(init(), uno::UNO_QUERY_THROW);

    const bool bCurrentMode = xFLA->isFormDesignMode();
    xFLA->setFormDesignMode(!bCurrentMode);

    const bool bNewMode = xFLA->isFormDesignMode();
    CPPUNIT_ASSERT_EQUAL(!bCurrentMode, bNewMode);

    xFLA->setFormDesignMode(bCurrentMode);
    CPPUNIT_ASSERT_EQUAL(bCurrentMode, static_cast<bool>(xFLA->isFormDesignMode()));
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
