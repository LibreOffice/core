/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <vcl/wrkwin.hxx>
#include <vcl/toolkit/edit.hxx>
#include <vcl/toolkit/button.hxx>
#include <vcl/toolkit/combobox.hxx>
#include <vcl/toolkit/dialog.hxx>
#include <vcl/toolkit/field.hxx>
#include <vcl/virdev.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/layout.hxx>
#include <vcl/scheduler.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/lang/XComponent.hpp>

class LifecycleTest : public test::BootstrapFixture
{
    void testWidgets(vcl::Window *pParent);

public:
    LifecycleTest() : BootstrapFixture(true, false) {}

    void testCast();
    void testVirtualDevice();
    void testMultiDispose();
    void testIsolatedWidgets();
    void testParentedWidgets();
    void testChildDispose();
    void testPostDispose();
    void testLeakage();
    void testToolkit();

    CPPUNIT_TEST_SUITE(LifecycleTest);
    CPPUNIT_TEST(testCast);
    CPPUNIT_TEST(testVirtualDevice);
    CPPUNIT_TEST(testMultiDispose);
    CPPUNIT_TEST(testIsolatedWidgets);
    CPPUNIT_TEST(testParentedWidgets);
    CPPUNIT_TEST(testChildDispose);
    CPPUNIT_TEST(testPostDispose);
    CPPUNIT_TEST(testLeakage);
    CPPUNIT_TEST(testToolkit);
    CPPUNIT_TEST_SUITE_END();
};

// A compile time sanity check
void LifecycleTest::testCast()
{
    ScopedVclPtrInstance< PushButton > xButton( nullptr, 0 );
    ScopedVclPtr<vcl::Window> xWindow(xButton);

    ScopedVclPtrInstance< MetricField > xField( nullptr, 0 );
    ScopedVclPtr<SpinField> xSpin(xField);
    ScopedVclPtr<Edit> xEdit(xField);

// the following line should NOT compile
//    VclPtr<PushButton> xButton2(xWindow);
}

void LifecycleTest::testVirtualDevice()
{
    VclPtr<VirtualDevice> pVDev = VclPtr< VirtualDevice >::Create();
    ScopedVclPtrInstance< VirtualDevice > pVDev2;
    VclPtrInstance<VirtualDevice> pVDev3;
    VclPtrInstance<VirtualDevice> pVDev4(DeviceFormat::DEFAULT);
    CPPUNIT_ASSERT(!!pVDev);
    CPPUNIT_ASSERT(!!pVDev2);
    CPPUNIT_ASSERT(!!pVDev3);
    CPPUNIT_ASSERT(!!pVDev4);
    pVDev.disposeAndClear();
    pVDev4.disposeAndClear();
}

void LifecycleTest::testMultiDispose()
{
    VclPtrInstance<WorkWindow> xWin(nullptr, WB_APP|WB_STDWORK);
    CPPUNIT_ASSERT(xWin);
    xWin->disposeOnce();
    xWin->disposeOnce();
    xWin->disposeOnce();
    CPPUNIT_ASSERT(!xWin->GetWindow(GetWindowType::Parent));
    CPPUNIT_ASSERT(!xWin->GetChild(0));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(0), xWin->GetChildCount());
}

void LifecycleTest::testWidgets(vcl::Window *pParent)
{
    {
        ScopedVclPtrInstance< PushButton > aPtr( pParent );
        (void)aPtr; // silence unused variable warning
    }
    {
        ScopedVclPtrInstance< OKButton > aPtr( pParent );
        (void)aPtr; // silence unused variable warning
    }
    {
        ScopedVclPtrInstance< CancelButton > aPtr( pParent );
        (void)aPtr; // silence unused variable warning
    }
    {
        ScopedVclPtrInstance< HelpButton > aPtr( pParent );
        (void)aPtr; // silence unused variable warning
    }

    // Some widgets really insist on adoption.
    if (pParent)
    {
        {
            ScopedVclPtrInstance< CheckBox > aPtr( pParent );
            (void)aPtr; // silence unused variable warning
        }
        {
            ScopedVclPtrInstance< Edit > aPtr( pParent );
            (void)aPtr; // silence unused variable warning
        }
        {
            ScopedVclPtrInstance< ComboBox > aPtr( pParent );
            (void)aPtr; // silence unused variable warning
        }
        {
            ScopedVclPtrInstance< RadioButton > aPtr( pParent, true, 0 );
            (void)aPtr; // silence unused variable warning
        }
    }
}

void LifecycleTest::testIsolatedWidgets()
{
    testWidgets(nullptr);
}

void LifecycleTest::testParentedWidgets()
{
    ScopedVclPtrInstance<WorkWindow> xWin(nullptr, WB_APP|WB_STDWORK);
    CPPUNIT_ASSERT(xWin);
    xWin->Show();
    testWidgets(xWin);
}

namespace {

class DisposableChild : public vcl::Window
{
public:
    explicit DisposableChild(vcl::Window *pParent) : vcl::Window(pParent) {}
    virtual ~DisposableChild() override
    {
        disposeOnce();
    }
};

}

void LifecycleTest::testChildDispose()
{
    VclPtrInstance<WorkWindow> xWin(nullptr, WB_APP|WB_STDWORK);
    CPPUNIT_ASSERT(xWin);
    VclPtrInstance< DisposableChild > xChild( xWin.get() );
    xWin->Show();
    xChild->disposeOnce();
    xWin->disposeOnce();
}

void LifecycleTest::testPostDispose()
{
    VclPtrInstance<WorkWindow> xWin(nullptr, WB_APP|WB_STDWORK);
    xWin->disposeOnce();

    // check selected methods continue to work post-dispose
    CPPUNIT_ASSERT(!xWin->GetParent());
    xWin->Show();
    CPPUNIT_ASSERT(!xWin->IsReallyShown());
    CPPUNIT_ASSERT(!xWin->IsEnabled());
    CPPUNIT_ASSERT(!xWin->IsInputEnabled());
    CPPUNIT_ASSERT(!xWin->GetChild(0));
    CPPUNIT_ASSERT(!xWin->GetWindow(GetWindowType::Parent));
}

namespace {

template <class vcl_type>
class LeakTestClass : public vcl_type
{
    bool &mrDeleted;
public:
    template<typename... Arg>
        LeakTestClass(bool &bDeleted, Arg &&... arg) :
            vcl_type(std::forward<Arg>(arg)...),
            mrDeleted(bDeleted)
    {
        mrDeleted = false;
    }
    ~LeakTestClass()
    {
        mrDeleted = true;
    }
};

class LeakTestObject
{
    bool                mbDeleted;
    VclPtr<vcl::Window> mxRef;
    void               *mpRef;
    LeakTestObject()
        : mbDeleted(false)
        , mpRef(nullptr)
    {
    }
public:
    template<typename vcl_type, typename... Arg> static LeakTestObject *
        Create(Arg &&... arg)
    {
        LeakTestObject *pNew = new LeakTestObject();
        pNew->mxRef = VclPtr< LeakTestClass< vcl_type > >::Create( pNew->mbDeleted,
                                                                   std::forward<Arg>(arg)...);
        pNew->mpRef = static_cast<void *>(static_cast<vcl::Window *>(pNew->mxRef));
        return pNew;
    }
    const VclPtr<vcl::Window>& getRef() const { return mxRef; }
    void disposeAndClear()
    {
        mxRef.disposeAndClear();
    }
    void assertDeleted()
    {
        if (!mbDeleted)
        {
            OUStringBuffer aMsg = "Type '";
            vcl::Window *pWin = static_cast<vcl::Window *>(mpRef);
            aMsg.appendAscii(typeid(*pWin).name());
            aMsg.append("' not freed after dispose");
            CPPUNIT_FAIL(OUStringToOString(aMsg.makeStringAndClear(),
                                           RTL_TEXTENCODING_UTF8).getStr());
        }
    }
};

}

void LifecycleTest::testLeakage()
{
    std::vector<LeakTestObject *> aObjects;

    // Create objects
    aObjects.push_back(LeakTestObject::Create<WorkWindow>(nullptr, WB_APP|WB_STDWORK));
    VclPtr<vcl::Window> xParent = aObjects.back()->getRef();

    aObjects.push_back(LeakTestObject::Create<PushButton>(xParent));
    aObjects.push_back(LeakTestObject::Create<CheckBox>(xParent));
    aObjects.push_back(LeakTestObject::Create<Edit>(xParent));
    aObjects.push_back(LeakTestObject::Create<ComboBox>(xParent));
    aObjects.push_back(LeakTestObject::Create<RadioButton>(xParent, true, 0));

    { // something that looks like a dialog
        aObjects.push_back(LeakTestObject::Create<Dialog>(xParent,WB_CLIPCHILDREN|WB_MOVEABLE|WB_3DLOOK|WB_CLOSEABLE|WB_SIZEABLE));
        VclPtr<vcl::Window> xDlgParent = aObjects.back()->getRef();
        aObjects.push_back(LeakTestObject::Create<VclVBox>(xDlgParent));
        VclPtr<vcl::Window> xVBox = aObjects.back()->getRef();
        aObjects.push_back(LeakTestObject::Create<VclVButtonBox>(xVBox));
    }

    aObjects.push_back(LeakTestObject::Create<Dialog>(xParent, u"PrintProgressDialog", "vcl/ui/printprogressdialog.ui"));
    xParent.clear();

    for (auto i = aObjects.rbegin(); i != aObjects.rend(); ++i)
        (*i)->getRef()->Show();

    for (auto i = aObjects.rbegin(); i != aObjects.rend(); ++i)
        (*i)->disposeAndClear();

    for (auto i = aObjects.begin(); i != aObjects.end(); ++i)
        (*i)->assertDeleted();

    for (auto i = aObjects.begin(); i != aObjects.end(); ++i)
        delete *i;
}

void LifecycleTest::testToolkit()
{
    LeakTestObject *pVclWin = LeakTestObject::Create<WorkWindow>(nullptr, WB_APP|WB_STDWORK);
    css::uno::Reference<css::awt::XWindow> xWindow(pVclWin->getRef()->GetComponentInterface(), css::uno::UNO_QUERY);
    CPPUNIT_ASSERT(xWindow.is());

    // test UNO dispose
    css::uno::Reference<css::lang::XComponent> xWinComponent = xWindow;
    CPPUNIT_ASSERT(xWinComponent.is());
    CPPUNIT_ASSERT(!pVclWin->getRef()->IsDisposed());
    xWinComponent->dispose();
    CPPUNIT_ASSERT(pVclWin->getRef()->IsDisposed());

    // test UNO cleanup
    xWinComponent.clear();
    xWindow.clear();
    pVclWin->disposeAndClear();
    pVclWin->assertDeleted();

    delete pVclWin;
}

CPPUNIT_TEST_SUITE_REGISTRATION(LifecycleTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
