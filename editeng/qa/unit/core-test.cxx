/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2012 Kohei Yoshida <kohei.yoshida@suse.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "sal/config.h"
#include "sal/precppunit.hxx"

#include "test/bootstrapfixture.hxx"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "svl/itempool.hxx"
#include "editeng/eerdll.hxx"
#include "editeng/eerdll2.hxx"
#include "editeng/editeng.hxx"
#include "editeng/eeitem.hxx"
#include "editeng/editids.hrc"

namespace {

const SfxItemInfo aItemInfos[] = {
    { SID_ATTR_FRAMEDIRECTION, SFX_ITEM_POOLABLE },         // EE_PARA_WRITINGDIR
    { 0, SFX_ITEM_POOLABLE },                               // EE_PARA_XMLATTRIBS
    { SID_ATTR_PARA_HANGPUNCTUATION, SFX_ITEM_POOLABLE },   // EE_PARA_HANGINGPUNCTUATION
    { SID_ATTR_PARA_FORBIDDEN_RULES, SFX_ITEM_POOLABLE },
    { SID_ATTR_PARA_SCRIPTSPACE, SFX_ITEM_POOLABLE },       // EE_PARA_ASIANCJKSPACING
    { SID_ATTR_NUMBERING_RULE, SFX_ITEM_POOLABLE },         // EE_PARA_NUMBULL
    { 0, SFX_ITEM_POOLABLE },                               // EE_PARA_HYPHENATE
    { 0, SFX_ITEM_POOLABLE },                               // EE_PARA_BULLETSTATE
    { 0, SFX_ITEM_POOLABLE },                               // EE_PARA_OUTLLRSPACE
    { SID_ATTR_PARA_OUTLLEVEL, SFX_ITEM_POOLABLE },         // EE_PARA_OUTLLEVEL
    { SID_ATTR_PARA_BULLET, SFX_ITEM_POOLABLE },            // EE_PARA_BULLET
    { SID_ATTR_LRSPACE, SFX_ITEM_POOLABLE },                // EE_PARA_LRSPACE
    { SID_ATTR_ULSPACE, SFX_ITEM_POOLABLE },                // EE_PARA_ULSPACE
    { SID_ATTR_PARA_LINESPACE, SFX_ITEM_POOLABLE },         // EE_PARA_SBL
    { SID_ATTR_PARA_ADJUST, SFX_ITEM_POOLABLE },            // EE_PARA_JUST
    { SID_ATTR_TABSTOP, SFX_ITEM_POOLABLE },                // EE_PARA_TABS
    { SID_ATTR_ALIGN_HOR_JUSTIFY_METHOD, SFX_ITEM_POOLABLE }, // EE_PARA_JUST_METHOD
    { SID_ATTR_ALIGN_VER_JUSTIFY, SFX_ITEM_POOLABLE },      // EE_PARA_VER_JUST
    { SID_ATTR_CHAR_COLOR, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_FONT, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_FONTHEIGHT, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_SCALEWIDTH, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_WEIGHT, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_UNDERLINE, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_STRIKEOUT, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_POSTURE, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_CONTOUR, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_SHADOWED, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_ESCAPEMENT, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_AUTOKERN, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_KERNING, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_WORDLINEMODE, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_LANGUAGE, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_CJK_LANGUAGE, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_CTL_LANGUAGE, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_CJK_FONT, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_CTL_FONT, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_CJK_FONTHEIGHT, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_CTL_FONTHEIGHT, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_CJK_WEIGHT, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_CTL_WEIGHT, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_CJK_POSTURE, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_CTL_POSTURE, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_EMPHASISMARK, SFX_ITEM_POOLABLE },
    { SID_ATTR_CHAR_RELIEF, SFX_ITEM_POOLABLE },
    { 0, SFX_ITEM_POOLABLE },                           // EE_CHAR_RUBI_DUMMY
    { 0, SFX_ITEM_POOLABLE },                           // EE_CHAR_XMLATTRIBS
    { SID_ATTR_CHAR_OVERLINE, SFX_ITEM_POOLABLE },
    { 0, SFX_ITEM_POOLABLE },                           // EE_FEATURE_TAB
    { 0, SFX_ITEM_POOLABLE },                           // EE_FEATURE_LINEBR
    { SID_ATTR_CHAR_CHARSETCOLOR, SFX_ITEM_POOLABLE },  // EE_FEATURE_NOTCONV
    { SID_FIELD, SFX_ITEM_POOLABLE }
};

class TestPool : public SfxItemPool
{
public:
    TestPool() : SfxItemPool("TestPool", EE_ITEMS_START, EE_ITEMS_END, aItemInfos, NULL, true)
    {
        SfxPoolItem** ppDefItems = EditDLL::Get().GetGlobalData()->GetDefItems();
        SetDefaults(ppDefItems);
    }
    virtual ~TestPool() {}
};

class Test : public test::BootstrapFixture
{
public:
    Test();

    void testConstruction();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testConstruction);
    CPPUNIT_TEST_SUITE_END();
};

Test::Test() {}

void Test::testConstruction()
{
    TestPool aPool;

    EditEngine aEngine(&aPool);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
