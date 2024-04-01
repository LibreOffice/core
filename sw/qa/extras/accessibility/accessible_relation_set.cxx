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
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
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

#include <test/a11y/AccessibilityTools.hxx>

using namespace com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

class AccessibleRelationSet : public UnoApiTest
{
public:
    AccessibleRelationSet()
        : UnoApiTest("")
    {
    }

    void init(uno::Reference<css::accessibility::XAccessible>& para1,
              uno::Reference<css::accessibility::XAccessible>& para2,
              uno::Reference<css::accessibility::XAccessible>& para3);
    void contents_flows_to_and_from();

    CPPUNIT_TEST_SUITE(AccessibleRelationSet);
    CPPUNIT_TEST(contents_flows_to_and_from);
    CPPUNIT_TEST_SUITE_END();
};

void AccessibleRelationSet::init(uno::Reference<css::accessibility::XAccessible>& para1,
                                 uno::Reference<css::accessibility::XAccessible>& para2,
                                 uno::Reference<css::accessibility::XAccessible>& para3)
{
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    uno::Reference<text::XTextDocument> xTextDoc(mxComponent, uno::UNO_QUERY);

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
}

void AccessibleRelationSet::contents_flows_to_and_from()
{
    //contents_flows_to

    css::uno::Reference<css::accessibility::XAccessible> para1;
    css::uno::Reference<css::accessibility::XAccessible> para2;
    css::uno::Reference<css::accessibility::XAccessible> para3;

    init(para1, para2, para3);
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
        CPPUNIT_ASSERT_EQUAL_MESSAGE("didn't gain correct relation type for paragraph 0",
                                     AccessibilityTools::getRelationTypeName(
                                         accessibility::AccessibleRelationType::CONTENT_FLOWS_TO),
                                     AccessibilityTools::getRelationTypeName(firstrelation));

        css::uno::Reference<css::accessibility::XAccessibleText> paraText2(para2,
                                                                           uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("didn't gain correct target paragraph", atarget->getText(),
                                     paraText2->getText());
    }

    //contents_flows_from()
    css::uno::Reference<css::accessibility::XAccessibleContext> oObj2(para2, uno::UNO_QUERY_THROW);
    css::uno::Reference<css::accessibility::XAccessibleRelationSet> set2
        = oObj2->getAccessibleRelationSet();

    sal_Int16 relationtypes[2] = { accessibility::AccessibleRelationType::INVALID,
                                   accessibility::AccessibleRelationType::INVALID };
    css::uno::Reference<css::accessibility::XAccessibleText> atargets[2];

    if (set2.is())
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("didn't gain correct count of relations", sal_Int32(2),
                                     set2->getRelationCount());
        sal_Int16 tmprelation = set2->getRelation(0).RelationType;
        if (tmprelation == accessibility::AccessibleRelationType::CONTENT_FLOWS_FROM)
        {
            css::uno::Reference<css::accessibility::XAccessibleText> adummy(
                set2->getRelation(0).TargetSet[0], uno::UNO_QUERY_THROW);
            atargets[0] = adummy;
            relationtypes[0] = tmprelation;
        }
        else if (tmprelation == accessibility::AccessibleRelationType::CONTENT_FLOWS_TO)
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
        if (tmprelation == accessibility::AccessibleRelationType::CONTENT_FLOWS_FROM)
        {
            css::uno::Reference<css::accessibility::XAccessibleText> adummy(
                set2->getRelation(1).TargetSet[0], uno::UNO_QUERY_THROW);
            atargets[0] = adummy;
            relationtypes[0] = tmprelation;
        }
        else if (tmprelation == accessibility::AccessibleRelationType::CONTENT_FLOWS_TO)
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

    CPPUNIT_ASSERT_EQUAL_MESSAGE("didn't gain correct relation type for paragraph 1",
                                 AccessibilityTools::getRelationTypeName(
                                     accessibility::AccessibleRelationType::CONTENT_FLOWS_FROM),
                                 AccessibilityTools::getRelationTypeName(relationtypes[0]));

    css::uno::Reference<css::accessibility::XAccessibleText> paraText1(para1, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("didn't gain correct target paragraph", atargets[0]->getText(),
                                 paraText1->getText());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("didn't gain correct relation type for paragraph 3",
                                 AccessibilityTools::getRelationTypeName(
                                     accessibility::AccessibleRelationType::CONTENT_FLOWS_TO),
                                 AccessibilityTools::getRelationTypeName(relationtypes[1]));

    css::uno::Reference<css::accessibility::XAccessibleText> paraText3(para3, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("didn't gain correct target paragraph", atargets[1]->getText(),
                                 paraText3->getText());
}

CPPUNIT_TEST_SUITE_REGISTRATION(AccessibleRelationSet);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
