/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <algorithm>

#include <com/sun/star/animations/TransitionSubType.hpp>
#include <com/sun/star/animations/TransitionType.hpp>

#include <comphelper/kit.hxx>
#include <comphelper/scopeguard.hxx>

#include <TransitionPreset.hxx>

using namespace ::com::sun::star;
using namespace ::cpo;

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
    // Given the transition presets offered in the COOL UI:
    comphelper::COKit::setActive(true);
    comphelper::ScopeGuard g([] { comphelper::COKit::setActive(false); });
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

CPPUNIT_TEST_FIXTURE(Test, testMorphTransitionPresets)
{
    // The COOL UI offers the three morph variants as one Morph transition, the options that a
    // PPTX file has.
    comphelper::COKit::setActive(true);
    comphelper::ScopeGuard aGuard([] { comphelper::COKit::setActive(false); });
    const sd::TransitionPresetList& rPresets = sd::TransitionPreset::getTransitionPresetList();

    struct Variant
    {
        OUString maPresetId;
        sal_Int16 mnSubtype;
        OUString maLabel;
    };
    const Variant aVariants[] = {
        { u"morph-objects"_ustr, animations::TransitionSubType::MORPH, u"Objects"_ustr },
        { u"morph-words"_ustr, animations::TransitionSubType::MORPHBYWORD, u"Words"_ustr },
        { u"morph-characters"_ustr, animations::TransitionSubType::MORPHBYCHARACTER,
          u"Characters"_ustr },
    };

    for (const Variant& rVariant : aVariants)
    {
        auto aFound = std::find_if(rPresets.begin(), rPresets.end(),
                                   [&rVariant](const sd::TransitionPresetPtr& pPreset)
                                   { return pPreset->getPresetId() == rVariant.maPresetId; });

        CPPUNIT_ASSERT_MESSAGE(rVariant.maPresetId.toUtf8().getStr(), aFound != rPresets.end());
        const sd::TransitionPresetPtr& pPreset = *aFound;
        CPPUNIT_ASSERT_EQUAL(animations::TransitionType::FADE, pPreset->getTransition());
        CPPUNIT_ASSERT_EQUAL(rVariant.mnSubtype, pPreset->getSubtype());
        CPPUNIT_ASSERT_EQUAL(u"morph"_ustr, pPreset->getSetId());
        CPPUNIT_ASSERT_EQUAL(u"Morph"_ustr, pPreset->getSetLabel());
        CPPUNIT_ASSERT_EQUAL(rVariant.maLabel, pPreset->getVariantLabel());
    }
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
