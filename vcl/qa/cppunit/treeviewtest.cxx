/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <salinst.hxx>
#include <svdata.hxx>

#include <cppuhelper/implbase.hxx>
#include <test/bootstrapfixture.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/TreeView.hxx>

class TreeViewTest : public test::BootstrapFixture
{
public:
    TreeViewTest()
        : BootstrapFixture(true, false)
    {
    }

    void setUp() override;
    void tearDown() override;

    void testIterPrevious();

    CPPUNIT_TEST_SUITE(TreeViewTest);
    CPPUNIT_TEST(testIterPrevious);
    CPPUNIT_TEST_SUITE_END();

private:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::TreeView> m_xTreeView;
};

void TreeViewTest::setUp()
{
    test::BootstrapFixture::setUp();

    OUString sDataDir = m_directories.getURLFromSrc(u"vcl/qa/cppunit/data/");

    SalInstance* pSalInstance = GetSalInstance();
    m_xBuilder = pSalInstance->CreateBuilder(nullptr, sDataDir, u"treeviewtest.ui"_ustr);
    m_xTreeView = m_xBuilder->weld_tree_view(u"treeview"_ustr);
}

void TreeViewTest::tearDown()
{
    m_xTreeView.reset();
    m_xBuilder.reset();

    test::BootstrapFixture::tearDown();
}

// Test that iter_previous works when the previous node has children on demand. See tdf#172670
void TreeViewTest::testIterPrevious()
{
    // Create the first node with children on demand
    m_xTreeView->append_text(u"first"_ustr);
    std::unique_ptr<weld::TreeIter> xIter = m_xTreeView->make_iterator();
    m_xTreeView->get_iter_first(*xIter);
    m_xTreeView->set_children_on_demand(*xIter, true);

    // Add a regular second node
    m_xTreeView->append_text(u"second"_ustr);

    // Point the iterator to the second node
    m_xTreeView->iter_next(*xIter);
    CPPUNIT_ASSERT_EQUAL(u"second"_ustr, m_xTreeView->get_text(*xIter));

    // Move back to the first node. Without the fix this will do an infinite recursion with stack
    // overflow.
    m_xTreeView->iter_previous(*xIter);
    CPPUNIT_ASSERT_EQUAL(u"first"_ustr, m_xTreeView->get_text(*xIter));
}

CPPUNIT_TEST_SUITE_REGISTRATION(TreeViewTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
