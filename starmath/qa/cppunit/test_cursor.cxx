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

#include <memory>

typedef tools::SvRef<SmDocShell> SmDocShellRef;

using namespace ::com::sun::star;

namespace {

class Test : public test::BootstrapFixture {

public:
    // init
    virtual void setUp() override;
    virtual void tearDown() override;

    // tests
    void testCopyPaste();
    void testCopySelectPaste();
    void testCutPaste();
    void testCutSelectPaste();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testCopyPaste);
    CPPUNIT_TEST(testCopySelectPaste);
    CPPUNIT_TEST(testCutPaste);
    CPPUNIT_TEST(testCutSelectPaste);
    CPPUNIT_TEST_SUITE_END();

private:
    SmDocShellRef xDocShRef;
};

void Test::setUp()
{
    BootstrapFixture::setUp();

    SmGlobals::ensure();

    xDocShRef = new SmDocShell(SfxModelFlags::EMBEDDED_OBJECT);
}

void Test::tearDown()
{
    xDocShRef.clear();
    BootstrapFixture::tearDown();
}

void Test::testCopyPaste()
{
    OUString const sInput("a * b + c");
    auto xTree = SmParser().Parse(sInput);
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

    CPPUNIT_ASSERT_EQUAL(OUString(" { a * b + c * b } "), xDocShRef->GetText());
}

void Test::testCopySelectPaste()
{
    OUString const sInput("a * b + c");
    auto xTree = SmParser().Parse(sInput);
    xTree->Prepare(xDocShRef->GetFormat(), *xDocShRef, 0);

    SmCursor aCursor(xTree.get(), xDocShRef.get());
    ScopedVclPtrInstance<VirtualDevice> pOutputDevice;

    // go to the right end
    for (int i=0;i<5;i++)
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

    CPPUNIT_ASSERT_EQUAL(OUString(" { b + c * b + c } "), xDocShRef->GetText());
}

void Test::testCutPaste()
{
    OUString const sInput("a * b + c");
    auto xTree = SmParser().Parse(sInput);
    xTree->Prepare(xDocShRef->GetFormat(), *xDocShRef, 0);

    SmCursor aCursor(xTree.get(), xDocShRef.get());
    ScopedVclPtrInstance<VirtualDevice> pOutputDevice;

    // go to the position at "*"
    aCursor.Move(pOutputDevice, MoveRight);
    // select "* b" and then cut
    aCursor.Move(pOutputDevice, MoveRight, false);
    aCursor.Move(pOutputDevice, MoveRight, false);
    aCursor.Cut();
    // go to the left end and then paste
    aCursor.Move(pOutputDevice, MoveRight);
    aCursor.Move(pOutputDevice, MoveRight);
    aCursor.Paste();

    CPPUNIT_ASSERT_EQUAL(OUString(" { a + c * b } "), xDocShRef->GetText());
}

void Test::testCutSelectPaste()
{
    OUString const sInput("a * b + c");
    auto xTree = SmParser().Parse(sInput);
    xTree->Prepare(xDocShRef->GetFormat(), *xDocShRef, 0);

    SmCursor aCursor(xTree.get(), xDocShRef.get());
    ScopedVclPtrInstance<VirtualDevice> pOutputDevice;

    // go to the right end
    for (int i=0;i<5;i++)
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

    CPPUNIT_ASSERT_EQUAL(OUString(" { b + c * } "), xDocShRef->GetText());
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
