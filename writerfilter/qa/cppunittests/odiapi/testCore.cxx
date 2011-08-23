/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <testshl/simpleheader.hxx>
#include <odiapi/core/Node.hxx>
#include <odiapi/props/Properties.hxx>

using namespace odiapi::core;
using namespace odiapi::props;
using namespace writerfilter;
using namespace std;

class TestCore : public CppUnit::TestFixture
{
public:
    void testCreateCore()
    {
        PropertyPool::Pointer_t pool = createPropertyPool();
        PropertyBag_Pointer_t pb = createPropertyBag();

        pb->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));
        PropertyPoolHandle_Pointer_t ph = pool->insert(pb);

        Node::Pointer_t node = createNode(NS_style::LN_char, ph, "NS_style::LN_char");

        CPPUNIT_ASSERT_MESSAGE("Create node failed wrong node id", node->getId() == NS_style::LN_char);
        CPPUNIT_ASSERT_MESSAGE("Create node failed wrong text", node->getText() == "NS_style::LN_char");
        CPPUNIT_ASSERT_MESSAGE("Create node failed wrong pool handle", node->getProperties() == ph);
    }

    void testInsertSibling()
    {
        PropertyPool::Pointer_t pool = createPropertyPool();
        PropertyBag_Pointer_t pb = createPropertyBag();

        pb->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));
        PropertyPoolHandle_Pointer_t ph = pool->insert(pb);

        Node::Pointer_t node1 = createNode(NS_style::LN_char, ph, "Text");
        Node::Pointer_t node2 = createNode(NS_style::LN_char, ph, "\\par");

        node1->insertSibling(node2);

        string postfixSeq = node1->getText();
        const Node* n = node1.get();
        while (n->hasNext())
        {
            n = &n->getNext();
            postfixSeq = postfixSeq + n->getText();
        }

        CPPUNIT_ASSERT_MESSAGE("Insert sibling failed", postfixSeq == "Text\\par");

        Node::Pointer_t node3 = createNode(NS_style::LN_char, ph, "\\span");

        node1->insertSibling(node3);

        postfixSeq = node1->getText();
        n = node1.get();
        while (n->hasNext())
        {
            n = &n->getNext();
            postfixSeq = postfixSeq + n->getText();
        }

        CPPUNIT_ASSERT_MESSAGE("Insert sibling failed", postfixSeq == "Text\\span\\par");
    }

    void testAppendChildren()
    {
        PropertyPool::Pointer_t pool = createPropertyPool();
        PropertyBag_Pointer_t pb = createPropertyBag();

        pb->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));
        PropertyPoolHandle_Pointer_t ph = pool->insert(pb);

        Node::Pointer_t node1 = createNode(NS_style::LN_char, ph, "Text");
        const Node* pn1 = node1.get();

        Node::Pointer_t node2 = createNode(NS_style::LN_char, ph, "\\par");

        node2->appendChildren(node1);

        CPPUNIT_ASSERT_MESSAGE("Append children failed", &node2->getFirstChild() == pn1);

        const Node* n = &node2->getFirstChild();
        string postfixSeq = n->getText() + n->getNext().getText();

        CPPUNIT_ASSERT_MESSAGE("Append children failed", postfixSeq == "Text\\par");
    }

    void testCore()
    {
        PropertyPool::Pointer_t pool = createPropertyPool();
        PropertyBag_Pointer_t pb = createPropertyBag();

        pb->insert(createIntegerProperty(NS_fo::LN_font_weight, 12));
        PropertyPoolHandle_Pointer_t ph = pool->insert(pb);

        Node::Pointer_t node1 = createNode(NS_style::LN_char, ph, "A");
        Node::Pointer_t node2 = createNode(NS_style::LN_char, ph, "\\span");

        node2->appendChildren(node1);

        Node::Pointer_t node3 = createNode(NS_style::LN_char, ph, "B");
        Node::Pointer_t node4 = createNode(NS_style::LN_char, ph, "\\span");

        node4->appendChildren(node3);

        node2->insertSibling(node4);

        Node::Pointer_t node5 = createNode(NS_style::LN_char, ph, "\\par");

        node5->appendChildren(node2);

        Node::Pointer_t node6 = createNode(NS_style::LN_char, ph, "C");
        Node::Pointer_t node7 = createNode(NS_style::LN_char, ph, "\\span");

        node7->appendChildren(node6);

        node5->appendChildren(node7);


        string postfixSeq = node5->getText();
        const Node* n = node5.get();
        while (n->hasPrevious())
        {
            n = &n->getPrevious();
            postfixSeq = postfixSeq + n->getText();
        }

        CPPUNIT_ASSERT_MESSAGE("Insert sibling failed", postfixSeq == "\\par\\spanC\\spanB\\spanA");
    }

    CPPUNIT_TEST_SUITE(TestCore);
    CPPUNIT_TEST(testCreateCore);
    CPPUNIT_TEST(testInsertSibling);
    CPPUNIT_TEST(testAppendChildren);
    CPPUNIT_TEST(testCore);
    CPPUNIT_TEST_SUITE_END();
};

//#####################################
// register test suites
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestCore, "TestCore");

//NOADDITIONAL;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
