/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

using namespace com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;
using namespace css::lang;


class AccessibilityTools
{
public:
    static css::uno::Reference<css::accessibility::XAccessibleContext> getAccessibleObjectForRole(css::uno::Reference<css::accessibility::XAccessible> xacc, sal_Int16 role, const OUString& name);
    static css::uno::Reference<css::accessibility::XAccessibleContext> getAccessibleObjectForRole(css::uno::Reference<css::accessibility::XAccessible> xacc, sal_Int16 role, const OUString& name, const OUString& implName);
};

OUString getImplName(css::uno::Reference<css::accessibility::XAccessibleContext> aObject){
        OUString res = "Error getting Implementation name";
        css::uno::Reference<lang::XServiceInfo> xSI(aObject, uno::UNO_QUERY_THROW);
        res = xSI->getImplementationName();
        return res;
}


css::uno::Reference<css::accessibility::XAccessibleContext> AccessibilityTools::getAccessibleObjectForRole(css::uno::Reference<css::accessibility::XAccessible> xacc, sal_Int16 role, const OUString& name){
    return getAccessibleObjectForRole(xacc, role, name, "");
}
css::uno::Reference<css::accessibility::XAccessibleContext> AccessibilityTools::getAccessibleObjectForRole(css::uno::Reference<css::accessibility::XAccessible> xacc, sal_Int16 role, const OUString& name, const OUString& implName)
{
    css::uno::Reference<css::accessibility::XAccessibleContext> ac = xacc->getAccessibleContext();
    bool isShowing = ac->getAccessibleStateSet()->contains(AccessibleStateType::SHOWING);
    // hotfix for i91828:
    // if role to search is 0 then ignore the role.
    if ( (role == 0 || ac->getAccessibleRole() == role) &&
        (ac->getAccessibleName().indexOf(name) > -1) &&
        (getImplName(ac).indexOf(implName) > -1) &&
        isShowing)
    {
        css::uno::Reference<css::accessibility::XAccessible> SearchedAccessible(xacc, uno::UNO_QUERY);
        return ac;
    }
    else
    {
        sal_Int32 k = ac->getAccessibleChildCount();

        if (ac->getAccessibleChildCount() > 100)
        {
            k = 50;
        }

        for (int i = 0; i < k; i++)
        {
            css::uno::Reference<css::accessibility::XAccessibleContext> ac1 = getAccessibleObjectForRole( ac->getAccessibleChild(i), role, name, implName);

            if(ac1.is())
            {
                CPPUNIT_ASSERT_MESSAGE("Couldn't get Child", ac1.is());
                return ac1;
            }
        }
    }
    return NULL;
}


class CheckIndeterminateState : public UnoApiTest
{
public:
    CheckIndeterminateState():
        UnoApiTest("")
    {
    }

    void testCheckIndeterminateState();

    CPPUNIT_TEST_SUITE(CheckIndeterminateState);
    CPPUNIT_TEST(testCheckIndeterminateState);
    CPPUNIT_TEST_SUITE_END();
};

void CheckIndeterminateState::testCheckIndeterminateState()
{
    css::uno::Reference<css::lang::XComponent> mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    css::uno::Reference<text::XText> text(textDocument->getText(), uno::UNO_QUERY_THROW);
    css::uno::Reference<css::frame::XModel> aModel( textDocument, uno::UNO_QUERY_THROW );

    css::uno::Reference<css::frame::XController> xController(aModel->getCurrentController());

    text->setString("normal");
    css::uno::Reference<css::text::XTextRange> end( text->getEnd(), uno::UNO_QUERY_THROW);
    end->setString("bold");

    css::uno::Reference< beans::XPropertySet> ended( end, uno::UNO_QUERY_THROW);
    ended-> setPropertyValue( "CharWeight", uno::makeAny(::com::sun::star::awt::FontWeight::BOLD));

    css::uno::Reference<text::XTextViewCursorSupplier> controller(xController, uno::UNO_QUERY_THROW);
    controller->getViewCursor()->gotoRange(text, false);


    uno::Reference< frame::XFrame > xFrame( xController->getFrame(), uno::UNO_SET_THROW);
    uno::Reference< awt::XWindow > xWindow( xFrame->getContainerWindow(), uno::UNO_SET_THROW);

    css::uno::Reference<css::accessibility::XAccessible> xRoot (xWindow, uno::UNO_QUERY_THROW );

    uno::Reference< uno::XInterface > oObj = AccessibilityTools::getAccessibleObjectForRole(xRoot,AccessibleRole::TOGGLE_BUTTON, "Bold");
    CPPUNIT_ASSERT_MESSAGE("Found a TOGGLE_BUTTON", oObj.is());
    css::uno::Reference<css::accessibility::XAccessibleContext> oContext(oObj, uno::UNO_QUERY_THROW);


    css::uno::Reference<css::accessibility::XAccessibleStateSet> oSet = oContext->getAccessibleStateSet();

    CPPUNIT_ASSERT_MESSAGE("The INDETERMINATE state is not set." , oSet->contains(AccessibleStateType::INDETERMINATE));

}

CPPUNIT_TEST_SUITE_REGISTRATION(CheckIndeterminateState);


CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
