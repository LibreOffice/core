/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <algorithm>

#include <com/sun/star/animations/TransitionSubType.hpp>
#include <com/sun/star/animations/TransitionType.hpp>

#include <TransitionPreset.hxx>

using namespace ::com::sun::star;

namespace
{
/// Covers sd/source/core/ fixes.
class Test : public UnoApiTest
{
public:
    Test();
};

Test::Test()
    : UnoApiTest(u"/sd/qa/core/data/"_ustr)
{
}

CPPUNIT_TEST_FIXTURE(Test, testAirplaneTransitionPreset)
{
    const sd::TransitionPresetList& rPresets = sd::TransitionPreset::getTransitionPresetList();

    // When looking for the airplane transition:
    auto it = std::find_if(rPresets.begin(), rPresets.end(),
                           [](const sd::TransitionPresetPtr& pPreset) {
                               return pPreset->getPresetId() == "wipe-airplane";
                           });

    // Then make sure it's offered as a variant of the Wipe transition:
    // Without the accompanying fix in place, this test would have failed, the airplane transition
    // could be only imported from PPTX, not selected on the UI.
    CPPUNIT_ASSERT(it != rPresets.end());
    const sd::TransitionPresetPtr& pPreset = *it;
    CPPUNIT_ASSERT_EQUAL(animations::TransitionType::MISCSHAPEWIPE, pPreset->getTransition());
    CPPUNIT_ASSERT_EQUAL(animations::TransitionSubType::AIRPLANE, pPreset->getSubtype());
    CPPUNIT_ASSERT_EQUAL(u"wipe"_ustr, pPreset->getSetId());
    CPPUNIT_ASSERT_EQUAL(u"Airplane"_ustr, pPreset->getVariantLabel());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
