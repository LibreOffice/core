/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
    void testInsertCommandTextCaretInPlace();
    void testInsertCommandTextOperatorReplacesPlace();
    void testInsertCommandTextOperatorReplacesPlaceInLine();
    void testInsertCommandTextTextReplacesPlaceInLine();
    void testInsertCommandTextFirstPlaceInText();
    void testPasteKeepsCaretAfterPlace();
    void testSelectPlaceAloneInLine();
    void testSelectPlaceOperatorReplacesIt();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testCopyPaste);
    CPPUNIT_TEST(testCopySelectPaste);
    CPPUNIT_TEST(testCutPaste);
    CPPUNIT_TEST(testCutSelectPaste);
    CPPUNIT_TEST(testSelectSurrogatePairs);
    CPPUNIT_TEST(testInsertCommandTextCaretInPlace);
    CPPUNIT_TEST(testInsertCommandTextOperatorReplacesPlace);
    CPPUNIT_TEST(testInsertCommandTextOperatorReplacesPlaceInLine);
    CPPUNIT_TEST(testInsertCommandTextTextReplacesPlaceInLine);
    CPPUNIT_TEST(testInsertCommandTextFirstPlaceInText);
    CPPUNIT_TEST(testPasteKeepsCaretAfterPlace);
    CPPUNIT_TEST(testSelectPlaceAloneInLine);
    CPPUNIT_TEST(testSelectPlaceOperatorReplacesIt);
    CPPUNIT_TEST_SUITE_END();

private:
    SmDocShellRef xDocShRef;
};

void Test::setUp()
{
    BootstrapFixture::setUp();

    SmGlobals::ensure();

    xDocShRef = new SmDocShell(SfxModelFlags::EMBEDDED_OBJECT);
    xDocShRef->DoInitNew(SfxNewDocumentMacros::Disallow);
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

void Test::testInsertCommandTextCaretInPlace()
{
    auto xTree = SmParser5().Parse(u"a"_ustr);
    xTree->Prepare(xDocShRef->GetFormat(), *xDocShRef, 0);

    SmCursor aCursor(xTree.get(), xDocShRef.get());
    ScopedVclPtrInstance<VirtualDevice> pOutputDevice;

    // go to the right end, insert a template the way the Elements panel does, then type
    aCursor.Move(pOutputDevice, MoveRight);
    aCursor.InsertCommandText(u"<?> over <?>"_ustr, true);
    // The numerator is selected, so it shows which placeholder the next input replaces
    CPPUNIT_ASSERT(aCursor.HasSelection());
    aCursor.InsertText(u"3"_ustr);

    // Without the fix the caret stayed after the inserted fraction, so "3" went after it
    CPPUNIT_ASSERT_EQUAL(u"{ a { 3 over <?> } }"_ustr, xDocShRef->GetText());
}

void Test::testInsertCommandTextOperatorReplacesPlace()
{
    auto xTree = SmParser5().Parse(u"a"_ustr);
    xTree->Prepare(xDocShRef->GetFormat(), *xDocShRef, 0);

    SmCursor aCursor(xTree.get(), xDocShRef.get());
    ScopedVclPtrInstance<VirtualDevice> pOutputDevice;

    aCursor.Move(pOutputDevice, MoveRight);
    aCursor.InsertCommandText(u"<?> over <?>"_ustr, true);
    aCursor.InsertElement(MinusElement);
    aCursor.InsertText(u"1"_ustr);

    // A leading operator replaces the placeholder too, instead of making it the left operand
    CPPUNIT_ASSERT_EQUAL(u"{ a { - 1 over <?> } }"_ustr, xDocShRef->GetText());
}

void Test::testInsertCommandTextOperatorReplacesPlaceInLine()
{
    auto xTree = SmParser5().Parse(u"a +"_ustr);
    xTree->Prepare(xDocShRef->GetFormat(), *xDocShRef, 0);

    SmCursor aCursor(xTree.get(), xDocShRef.get());
    ScopedVclPtrInstance<VirtualDevice> pOutputDevice;

    aCursor.Move(pOutputDevice, MoveRight);
    aCursor.Move(pOutputDevice, MoveRight);
    aCursor.InsertCommandText(u"<?> cdot <?>"_ustr, true);
    aCursor.InsertElement(MinusElement);
    aCursor.InsertText(u"1"_ustr);

    // The placeholder shares its line with the rest of the formula here. Without the fix the
    // caret was only after it, so the operator went behind it and the placeholder stayed.
    CPPUNIT_ASSERT_EQUAL(u"{ a + { - 1 cdot <?> } }"_ustr, xDocShRef->GetText());
}

void Test::testInsertCommandTextTextReplacesPlaceInLine()
{
    auto xTree = SmParser5().Parse(u"a +"_ustr);
    xTree->Prepare(xDocShRef->GetFormat(), *xDocShRef, 0);

    SmCursor aCursor(xTree.get(), xDocShRef.get());
    ScopedVclPtrInstance<VirtualDevice> pOutputDevice;

    aCursor.Move(pOutputDevice, MoveRight);
    aCursor.Move(pOutputDevice, MoveRight);
    aCursor.InsertCommandText(u"<?> cdot <?>"_ustr, true);
    aCursor.InsertText(u"x"_ustr);

    // The selected placeholder is replaced, not only the text typed after it
    CPPUNIT_ASSERT_EQUAL(u"{ a + { x cdot <?> } }"_ustr, xDocShRef->GetText());
}

void Test::testInsertCommandTextFirstPlaceInText()
{
    auto xTree = SmParser5().Parse(u"a"_ustr);
    xTree->Prepare(xDocShRef->GetFormat(), *xDocShRef, 0);

    SmCursor aCursor(xTree.get(), xDocShRef.get());
    ScopedVclPtrInstance<VirtualDevice> pOutputDevice;

    aCursor.Move(pOutputDevice, MoveRight);
    aCursor.InsertCommandText(u"nroot{<?>}{<?>}"_ustr, true);
    aCursor.InsertText(u"3"_ustr);

    // The caret graph has the root body before the index, the command text has the index first
    CPPUNIT_ASSERT_EQUAL(u"{ a nroot 3 <?> }"_ustr, xDocShRef->GetText());
}

void Test::testPasteKeepsCaretAfterPlace()
{
    auto xTree = SmParser5().Parse(u"{<?> over b} + c"_ustr);
    xTree->Prepare(xDocShRef->GetFormat(), *xDocShRef, 0);

    SmCursor aCursor(xTree.get(), xDocShRef.get());
    ScopedVclPtrInstance<VirtualDevice> pOutputDevice;

    // select the fraction and copy it, then paste it at the right end and type
    aCursor.Move(pOutputDevice, MoveRight, false);
    aCursor.Copy();
    for (int i = 0; i < 6; i++)
        aCursor.Move(pOutputDevice, MoveRight);
    aCursor.Paste();
    aCursor.InsertText(u"3"_ustr);

#ifndef _WIN32 // FIXME: on Windows clipboard does not work in tests for some reason
    // Only inserting from the Elements panel moves the caret into a placeholder, not pasting
    CPPUNIT_ASSERT_EQUAL(u"{ { { <?> over b } + c } { <?> over b } 3 }"_ustr, xDocShRef->GetText());
#endif
}

/** Collect the placeholders of a formula in the order of its command text */
void collectPlaces(const SmNode* pNode, std::vector<const SmNode*>& rPlaces)
{
    if (!pNode)
        return;
    if (pNode->GetType() == SmNodeType::Place)
        rPlaces.push_back(pNode);
    for (size_t i = 0; i < pNode->GetNumSubNodes(); ++i)
        collectPlaces(pNode->GetSubNode(i), rPlaces);
}

void Test::testSelectPlaceAloneInLine()
{
    auto xTree = SmParser5().Parse(u"<?> over <?>"_ustr);
    xTree->Prepare(xDocShRef->GetFormat(), *xDocShRef, 0);
    std::vector<const SmNode*> aPlaces;
    collectPlaces(xTree.get(), aPlaces);
    CPPUNIT_ASSERT_EQUAL(size_t(2), aPlaces.size());

    SmCursor aCursor(xTree.get(), xDocShRef.get());

    // Select the denominator, the way a click on it does, then type
    CPPUNIT_ASSERT(aCursor.SelectPlace(aPlaces[1]));
    CPPUNIT_ASSERT(aCursor.HasSelection());
    aCursor.InsertText(u"3"_ustr);

    // Only the denominator is replaced, the fraction stays
    CPPUNIT_ASSERT_EQUAL(u"{ <?> over 3 }"_ustr, xDocShRef->GetText());
}

void Test::testSelectPlaceOperatorReplacesIt()
{
    auto xTree = SmParser5().Parse(u"<?> + <?>"_ustr);
    xTree->Prepare(xDocShRef->GetFormat(), *xDocShRef, 0);
    std::vector<const SmNode*> aPlaces;
    collectPlaces(xTree.get(), aPlaces);
    CPPUNIT_ASSERT_EQUAL(size_t(2), aPlaces.size());

    SmCursor aCursor(xTree.get(), xDocShRef.get());

    CPPUNIT_ASSERT(aCursor.SelectPlace(aPlaces[1]));
    aCursor.InsertElement(MinusElement);
    aCursor.InsertText(u"1"_ustr);

    // The selected placeholder is replaced, also by a leading operator
    CPPUNIT_ASSERT_EQUAL(u"{ <?> + - 1 }"_ustr, xDocShRef->GetText());
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
