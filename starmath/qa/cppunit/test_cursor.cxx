/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>

#include <vcl/virdev.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <smdll.hxx>

#include <document.hxx>
#include <node.hxx>
#include <cursor.hxx>
#include <parse5.hxx>

#include <memory>

typedef rtl::Reference<SmDocShell> SmDocShellRef;

namespace
{
class Test : public test::BootstrapFixture
{
public:
    // init
    virtual void setUp() override;
    virtual void tearDown() override;

    // tests
    void testCopyPaste();
    void testCopySelectPaste();
    void testCutPaste();
    void testCutSelectPaste();
    void testSelectSurrogatePairs();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testCopyPaste);
    CPPUNIT_TEST(testCopySelectPaste);
    CPPUNIT_TEST(testCutPaste);
    CPPUNIT_TEST(testCutSelectPaste);
    CPPUNIT_TEST(testSelectSurrogatePairs);
    CPPUNIT_TEST_SUITE_END();

private:
    SmDocShellRef xDocShRef;
};

void Test::setUp()
{
    BootstrapFixture::setUp();

    SmGlobals::ensure();

    xDocShRef = new SmDocShell(SfxModelFlags::EMBEDDED_OBJECT);
    xDocShRef->DoInitNew();
}

void Test::tearDown()
{
    xDocShRef->DoClose();
    xDocShRef.clear();
    BootstrapFixture::tearDown();
}

void Test::testCopyPaste()
{
    auto xTree = SmParser5().Parse(u"a * b + c"_ustr);
    xTree->Prepare(xDocShRef->GetFormat(), *xDocShRef, 0);

    SmCursor aCursor(xTree.get(), xDocShRef.get());
    ScopedVclPtrInstance<VirtualDevice> pOutputDevice;

    // go to the position at "*"
    aCursor.Move(pOutputDevice, MoveRight);
    // select "* b" and then copy
    aCursor.Move(pOutputDevice, MoveRight, false);
    aCursor.Move(pOutputDevice, MoveRight, false);
    aCursor.Copy();
    // go to the right end and then paste
    aCursor.Move(pOutputDevice, MoveRight);
    aCursor.Move(pOutputDevice, MoveRight);
    aCursor.Paste();

#ifndef _WIN32 // FIXME: on Windows clipboard does not work in tests for some reason
    CPPUNIT_ASSERT_EQUAL(u"{ { a * b } + { c * b } }"_ustr, xDocShRef->GetText());
#endif
}

void Test::testCopySelectPaste()
{
    auto xTree = SmParser5().Parse(u"a * b + c"_ustr);
    xTree->Prepare(xDocShRef->GetFormat(), *xDocShRef, 0);

    SmCursor aCursor(xTree.get(), xDocShRef.get());
    ScopedVclPtrInstance<VirtualDevice> pOutputDevice;

    // go to the right end
    for (int i = 0; i < 5; i++)
        aCursor.Move(pOutputDevice, MoveRight);
    // select "b + c" and then copy
    aCursor.Move(pOutputDevice, MoveLeft, false);
    aCursor.Move(pOutputDevice, MoveLeft, false);
    aCursor.Move(pOutputDevice, MoveLeft, false);
    aCursor.Copy();
    // go to the left end
    aCursor.Move(pOutputDevice, MoveLeft);
    aCursor.Move(pOutputDevice, MoveLeft);
    // select "a" and then paste
    aCursor.Move(pOutputDevice, MoveRight, false);
    aCursor.Paste();

#ifndef _WIN32 // FIXME: on Windows clipboard does not work in tests for some reason
    CPPUNIT_ASSERT_EQUAL(u"{ { b + { c * b } } + c }"_ustr, xDocShRef->GetText());
#endif
}

void Test::testCutPaste()
{
    auto xTree = SmParser5().Parse(u"a * b + c"_ustr);
    xTree->Prepare(xDocShRef->GetFormat(), *xDocShRef, 0);

    SmCursor aCursor(xTree.get(), xDocShRef.get());
    ScopedVclPtrInstance<VirtualDevice> pOutputDevice;

    // go to the position at "*"
    aCursor.Move(pOutputDevice, MoveRight);
    // select "* b" and then cut
    aCursor.Move(pOutputDevice, MoveRight, false);
    aCursor.Move(pOutputDevice, MoveRight, false);
    aCursor.Cut();
    // go to the right end and then paste
    aCursor.Move(pOutputDevice, MoveRight);
    aCursor.Move(pOutputDevice, MoveRight);
    aCursor.Paste();

#ifndef _WIN32 // FIXME: on Windows clipboard does not work in tests for some reason
    CPPUNIT_ASSERT_EQUAL(u"{ a + { c * b } }"_ustr, xDocShRef->GetText());
#endif
}

void Test::testCutSelectPaste()
{
    auto xTree = SmParser5().Parse(u"a * b + c"_ustr);
    xTree->Prepare(xDocShRef->GetFormat(), *xDocShRef, 0);

    SmCursor aCursor(xTree.get(), xDocShRef.get());
    ScopedVclPtrInstance<VirtualDevice> pOutputDevice;

    // go to the right end
    for (int i = 0; i < 5; i++)
        aCursor.Move(pOutputDevice, MoveRight);
    // select "b + c" and then cut
    aCursor.Move(pOutputDevice, MoveLeft, false);
    aCursor.Move(pOutputDevice, MoveLeft, false);
    aCursor.Move(pOutputDevice, MoveLeft, false);
    aCursor.Cut();
    // go to the left end
    aCursor.Move(pOutputDevice, MoveLeft);
    aCursor.Move(pOutputDevice, MoveLeft);
    // select "a" and then paste
    aCursor.Move(pOutputDevice, MoveRight, false);
    aCursor.Paste();

#ifndef _WIN32 // FIXME: on Windows clipboard does not work in tests for some reason
    CPPUNIT_ASSERT_EQUAL(u"{ b + { c * {} } }"_ustr, xDocShRef->GetText());
#endif
}

void Test::testSelectSurrogatePairs()
{
    auto xTree = SmParser5().Parse(u"\U0001EE4E"_ustr);
    xTree->Prepare(xDocShRef->GetFormat(), *xDocShRef, 0);

    SmCursor aCursor(xTree.get(), xDocShRef.get());
    ScopedVclPtrInstance<VirtualDevice> pOutputDevice;

    // select the first character, cut, then paste
    aCursor.Move(pOutputDevice, MoveRight, false);
    aCursor.Cut();
    aCursor.Paste();

#ifndef _WIN32 // FIXME: on Windows clipboard does not work in tests for some reason
    CPPUNIT_ASSERT_EQUAL(u"\U0001EE4E"_ustr, xDocShRef->GetText());
#endif
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
