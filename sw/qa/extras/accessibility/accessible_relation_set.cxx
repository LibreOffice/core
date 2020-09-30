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

#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>

using namespace com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;
using namespace css::lang;

namespace
{
class AccessibilityTools
{
public:
    static css::uno::Reference<css::accessibility::XAccessibleContext>
    getAccessibleObjectForRole(const css::uno::Reference<css::accessibility::XAccessible>& xacc,
                               sal_Int16 role);
};
}

css::uno::Reference<css::accessibility::XAccessibleContext>
AccessibilityTools::getAccessibleObjectForRole(
    const css::uno::Reference<css::accessibility::XAccessible>& xacc, sal_Int16 role)
{
    css::uno::Reference<css::accessibility::XAccessibleContext> ac = xacc->getAccessibleContext();
    bool isShowing = ac->getAccessibleStateSet()->contains(AccessibleStateType::SHOWING);

    if ((ac->getAccessibleRole() == role) && isShowing)
    {
        return ac;
    }
    else
    {
        int k = ac->getAccessibleChildCount();
        if (ac->getAccessibleChildCount() > 100)
        {
            k = 50;
        }

        for (int i = 0; i < k; i++)
        {
            css::uno::Reference<css::accessibility::XAccessibleContext> ac2
                = AccessibilityTools::getAccessibleObjectForRole(ac->getAccessibleChild(i), role);
            if (ac2.is())
            {
                return ac2;
            }
        }
    }
    return nullptr;
}

class AccessibleRelationSet : public UnoApiTest
{
public:
    AccessibleRelationSet()
        : UnoApiTest("")
    {
    }

    uno::Reference<lang::XComponent> init(uno::Reference<css::accessibility::XAccessible>& para1,
                                          uno::Reference<css::accessibility::XAccessible>& para2,
                                          uno::Reference<css::accessibility::XAccessible>& para3);
    void contents_flows_to_and_from();

    CPPUNIT_TEST_SUITE(AccessibleRelationSet);
    CPPUNIT_TEST(contents_flows_to_and_from);
    CPPUNIT_TEST_SUITE_END();
};

uno::Reference<lang::XComponent>
AccessibleRelationSet::init(uno::Reference<css::accessibility::XAccessible>& para1,
                            uno::Reference<css::accessibility::XAccessible>& para2,
                            uno::Reference<css::accessibility::XAccessible>& para3)
{
    uno::Reference<css::lang::XComponent> xComponent
        = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    uno::Reference<text::XTextDocument> xTextDoc(xComponent, uno::UNO_QUERY);

    css::uno::Reference<text::XText> oText(xTextDoc->getText(), uno::UNO_SET_THROW);

    css::uno::Reference<text::XTextCursor> oCursor = oText->createTextCursor();

    for (int i = 0; i < 5; i++)
    {
        oText->insertString(oCursor, "Paragraph Number: " + OUString::number(i), false);
        oText->insertControlCharacter(oCursor, text::ControlCharacter::PARAGRAPH_BREAK, false);
    }

    css::uno::Reference<css::frame::XModel> aModel(xTextDoc, uno::UNO_QUERY_THROW);
    css::uno::Reference<css::frame::XController> xController(aModel->getCurrentController());

    css::uno::Reference<css::accessibility::XAccessibleContext> ctx;
    uno::Reference<frame::XFrame> xFrame(xController->getFrame(), uno::UNO_SET_THROW);
    uno::Reference<awt::XWindow> xWindow(xFrame->getComponentWindow(), uno::UNO_SET_THROW);

    css::uno::Reference<css::accessibility::XAccessible> xRoot(xWindow, uno::UNO_QUERY_THROW);
    ctx = AccessibilityTools::getAccessibleObjectForRole(xRoot, AccessibleRole::DOCUMENT_TEXT);
    CPPUNIT_ASSERT_MESSAGE("Couldn't get AccessibleRole.DOCUMENT_TEXT object", ctx.is());
    para1 = ctx->getAccessibleChild(0);
    para2 = ctx->getAccessibleChild(1);
    para3 = ctx->getAccessibleChild(2);
    return xComponent;
}

void AccessibleRelationSet::contents_flows_to_and_from()
{
    //contents_flows_to
    std::vector<OUString> types{ "INVALID",       "CONTENT_FLOWS_FROM", "CONTENT_FLOWS_TO",
                                 "CONTROLLED_BY", "CONTROLLER_FOR",     "LABEL_FOR",
                                 "LABELED_BY",    "MEMBER_OF",          "SUB_WINDOW_OF" };

    css::uno::Reference<css::accessibility::XAccessible> para1;
    css::uno::Reference<css::accessibility::XAccessible> para2;
    css::uno::Reference<css::accessibility::XAccessible> para3;

    uno::Reference<lang::XComponent> xComponent(init(para1, para2, para3));
    css::uno::Reference<css::accessibility::XAccessibleContext> oObj(para1, uno::UNO_QUERY_THROW);
    css::uno::Reference<css::accessibility::XAccessibleRelationSet> set
        = oObj->getAccessibleRelationSet();

    css::uno::Reference<css::accessibility::XAccessibleText> atarget;

    if (set.is())
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("didn't gain correct count of relations", sal_Int32(1),
                                     set->getRelationCount());
        sal_Int16 firstrelation = set->getRelation(0).RelationType;
        css::uno::Reference<css::accessibility::XAccessibleText> adummy(
            set->getRelation(0).TargetSet[0], uno::UNO_QUERY_THROW);
        atarget = adummy;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("didn't gain correct relation type for paragraph 0", types[2],
                                     types[firstrelation]);

        css::uno::Reference<css::accessibility::XAccessibleText> paraText2(para2,
                                                                           uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("didn't gain correct target paragraph", atarget->getText(),
                                     paraText2->getText());
    }

    //contents_flows_from()
    css::uno::Reference<css::accessibility::XAccessibleContext> oObj2(para2, uno::UNO_QUERY_THROW);
    css::uno::Reference<css::accessibility::XAccessibleRelationSet> set2
        = oObj2->getAccessibleRelationSet();

    sal_Int16 relationtypes[2] = { -1, -1 };
    css::uno::Reference<css::accessibility::XAccessibleText> atargets[2];

    if (set2.is())
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("didn't gain correct count of relations", sal_Int32(2),
                                     set2->getRelationCount());
        sal_Int16 tmprelation = set2->getRelation(0).RelationType;
        if (tmprelation == 1)
        {
            css::uno::Reference<css::accessibility::XAccessibleText> adummy(
                set2->getRelation(0).TargetSet[0], uno::UNO_QUERY_THROW);
            atargets[0] = adummy;
            relationtypes[0] = tmprelation;
        }
        else if (tmprelation == 2)
        {
            css::uno::Reference<css::accessibility::XAccessibleText> adummy(
                set2->getRelation(0).TargetSet[0], uno::UNO_QUERY_THROW);
            atargets[1] = adummy;
            relationtypes[1] = tmprelation;
        }
        else
        {
            CPPUNIT_FAIL("didn't gain correct relation type");
        }
        tmprelation = set2->getRelation(1).RelationType;
        if (tmprelation == 1)
        {
            css::uno::Reference<css::accessibility::XAccessibleText> adummy(
                set2->getRelation(1).TargetSet[0], uno::UNO_QUERY_THROW);
            atargets[0] = adummy;
            relationtypes[0] = tmprelation;
        }
        else if (tmprelation == 2)
        {
            css::uno::Reference<css::accessibility::XAccessibleText> adummy(
                set2->getRelation(1).TargetSet[0], uno::UNO_QUERY_THROW);
            atargets[1] = adummy;
            relationtypes[1] = tmprelation;
        }
        else
        {
            CPPUNIT_FAIL("didn't gain correct relation type");
        }
    }

    CPPUNIT_ASSERT_EQUAL_MESSAGE("didn't gain correct relation type for paragraph 1", types[1],
                                 types[relationtypes[0]]);

    css::uno::Reference<css::accessibility::XAccessibleText> paraText1(para1, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("didn't gain correct target paragraph", atargets[0]->getText(),
                                 paraText1->getText());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("didn't gain correct relation type for paragraph 3", types[2],
                                 types[relationtypes[1]]);

    css::uno::Reference<css::accessibility::XAccessibleText> paraText3(para3, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("didn't gain correct target paragraph", atargets[1]->getText(),
                                 paraText3->getText());

    closeDocument(xComponent);
}

CPPUNIT_TEST_SUITE_REGISTRATION(AccessibleRelationSet);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
