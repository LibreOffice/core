/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx"
#include <docsh.hxx>
#include <svx/svdpage.hxx>

using namespace css;

class DocumentThemesTest : public ScUcalcTestBase
{
};

namespace
{
CPPUNIT_TEST_FIXTURE(DocumentThemesTest, testThemes)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, "Test");
    ScDrawLayer* pDrawLayer = m_pDoc->GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);
    const SdrPage* pPage(pDrawLayer->GetPage(0));
    CPPUNIT_ASSERT(pPage);
    auto const& pTheme = pPage->getSdrPageProperties().GetTheme();
    CPPUNIT_ASSERT(pTheme);
}

} // end anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
