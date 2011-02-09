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

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"

#include <o3tl/heap_ptr.hxx>




using o3tl::heap_ptr;


class Help
{
  public:
    explicit            Help(
                            int                 i_n )
                                                : n(i_n) { ++nInstanceCount_; }
                        ~Help()                 { --nInstanceCount_; }
    int                 Value() const           { return n; }
    static int          InstanceCount_()        { return nInstanceCount_; }

  private:
    int                 n;
    static int          nInstanceCount_;
};
int Help::nInstanceCount_ = 0;


class heap_ptr_test : public CppUnit::TestFixture
{
  public:
    void global()
    {
        // Construction
        heap_ptr<Help>
            t_empty;
        const heap_ptr<Help>
            t0( new Help(7000) );
        heap_ptr<Help>
            t1( new Help(10) );
        heap_ptr<Help>
            t2( new Help(20) );

        int nHelpCount = 3;

        CPPUNIT_ASSERT_MESSAGE("ctor1", ! t_empty.is());
        CPPUNIT_ASSERT_MESSAGE("ctor2",   t0.is());
        CPPUNIT_ASSERT_MESSAGE("ctor3",   (*t0).Value() == 7000 );
        CPPUNIT_ASSERT_MESSAGE("ctor4",   t0->Value() == 7000 );
        CPPUNIT_ASSERT_MESSAGE("ctor5",   t0.get() == t0.operator->() );
        CPPUNIT_ASSERT_MESSAGE("ctor6",   t0.get() == &(*t0) );

        CPPUNIT_ASSERT_MESSAGE("ctor7",   t1.is());
        CPPUNIT_ASSERT_MESSAGE("ctor8",   (*t1).Value() == 10 );
        CPPUNIT_ASSERT_MESSAGE("ctor9",   t1->Value() == 10 );
        CPPUNIT_ASSERT_MESSAGE("ctor10",   t1.get() == t1.operator->() );
        CPPUNIT_ASSERT_MESSAGE("ctor11",   t1.get() == &(*t1) );

        CPPUNIT_ASSERT_MESSAGE("ctor12",   t2.operator*().Value() == 20);
        CPPUNIT_ASSERT_MESSAGE("ctor13",   Help::InstanceCount_() == nHelpCount);


        // Operator safe_bool() / bool()
        CPPUNIT_ASSERT_MESSAGE("bool1", ! t_empty);
        CPPUNIT_ASSERT_MESSAGE("bool2", t0);
        CPPUNIT_ASSERT_MESSAGE("bool3", t1.is() == static_cast<bool>(t1));


        // Assignment, reset() and release()
            // Release
        Help * hp = t1.release();
        CPPUNIT_ASSERT_MESSAGE("release1", ! t1.is() );
        CPPUNIT_ASSERT_MESSAGE("release2", t1.get() == 0 );
        CPPUNIT_ASSERT_MESSAGE("release3", t1.operator->() == 0 );
        CPPUNIT_ASSERT_MESSAGE("release4", Help::InstanceCount_() == nHelpCount);

            // operator=()
        t_empty = hp;
        CPPUNIT_ASSERT_MESSAGE("assign1", t_empty.is() );
        CPPUNIT_ASSERT_MESSAGE("assign2", Help::InstanceCount_() == nHelpCount);

        t1 = t_empty.release();
        CPPUNIT_ASSERT_MESSAGE("assign3",     t1.is() );
        CPPUNIT_ASSERT_MESSAGE("assign4",   ! t_empty.is() );
        CPPUNIT_ASSERT_MESSAGE("assign5",   Help::InstanceCount_() == nHelpCount);

            // reset()
        hp = new Help(30);
        ++nHelpCount;

        t_empty.reset(hp);
        CPPUNIT_ASSERT_MESSAGE("reset1",  Help::InstanceCount_() == nHelpCount);
        CPPUNIT_ASSERT_MESSAGE("reset2",  t_empty.is() );
        CPPUNIT_ASSERT_MESSAGE("reset3",  t_empty.get() == hp );

            // Ignore second assignment
        t_empty = hp;
        CPPUNIT_ASSERT_MESSAGE("selfassign1",  Help::InstanceCount_() == nHelpCount);
        CPPUNIT_ASSERT_MESSAGE("selfassign2",  t_empty.is() );
        CPPUNIT_ASSERT_MESSAGE("selfassign3",  t_empty.get() == hp );

        t_empty.reset(0);
        hp = 0;
        --nHelpCount;
        CPPUNIT_ASSERT_MESSAGE("reset4",   ! t_empty.is() );
        CPPUNIT_ASSERT_MESSAGE("reset5",   Help::InstanceCount_() == nHelpCount);


        // swap
        t1.swap(t2);
        CPPUNIT_ASSERT_MESSAGE("swap1",   t1->Value() == 20 );
        CPPUNIT_ASSERT_MESSAGE("swap2",   t2->Value() == 10 );
        CPPUNIT_ASSERT_MESSAGE("swap3",   Help::InstanceCount_() == nHelpCount);

        o3tl::swap(t1,t2);
        CPPUNIT_ASSERT_MESSAGE("swap4",   t1->Value() == 10 );
        CPPUNIT_ASSERT_MESSAGE("swap5",   t2->Value() == 20 );
        CPPUNIT_ASSERT_MESSAGE("swap6",   Help::InstanceCount_() == nHelpCount);

        // RAII
        {
            heap_ptr<Help>
                t_raii( new Help(55) );
            CPPUNIT_ASSERT_MESSAGE("raii1", Help::InstanceCount_() == nHelpCount + 1);
        }
        CPPUNIT_ASSERT_MESSAGE("raii2", Help::InstanceCount_() == nHelpCount);
    }


    // These macros are needed by auto register mechanism.
    CPPUNIT_TEST_SUITE(heap_ptr_test);
    CPPUNIT_TEST(global);
    CPPUNIT_TEST_SUITE_END();
}; // class heap_ptr_test

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION(heap_ptr_test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
