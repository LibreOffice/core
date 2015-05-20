/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"

#include <o3tl/enumarray.hxx>


enum class EHint { NONE=0, ONE=1, TWO=2, LAST = TWO };

class enumarray_test : public CppUnit::TestFixture
{
public:
    void testBasics()
    {
        // test initializer syntax

        typedef struct
        {
            const char*  pBaseName;
            int          aHotSpot;
        } curs_ent;

        // call the initializer_list constructor
        const o3tl::enumarray<EHint, curs_ent> aTest1
        {{
            curs_ent{ NULL, 1 },
            { "nullptr", 1 },
            { "hourglass", 1 }
        }};

        const o3tl::enumarray<EHint, int> aTest2 {{1,2,3}};

        // call the default constructor
        o3tl::enumarray<EHint, curs_ent> aTest3;

    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(enumarray_test);
    CPPUNIT_TEST(testBasics);
    CPPUNIT_TEST_SUITE_END();
};


CPPUNIT_TEST_SUITE_REGISTRATION(enumarray_test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
