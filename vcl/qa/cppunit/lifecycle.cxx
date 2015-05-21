/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>

#include <vcl/wrkwin.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/combobox.hxx>
#include <vcl/field.hxx>
#include <vcl/virdev.hxx>
#include <vcl/tabctrl.hxx>

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
    void testFocus();

    CPPUNIT_TEST_SUITE(LifecycleTest);
    CPPUNIT_TEST(testCast);
    CPPUNIT_TEST(testVirtualDevice);
    CPPUNIT_TEST(testMultiDispose);
    CPPUNIT_TEST(testIsolatedWidgets);
    CPPUNIT_TEST(testParentedWidgets);
    CPPUNIT_TEST(testChildDispose);
    CPPUNIT_TEST(testPostDispose);
    CPPUNIT_TEST(testFocus);
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
    VclPtrInstance<VirtualDevice> pVDev4( 1 );
    CPPUNIT_ASSERT(!!pVDev && !!pVDev2 && !!pVDev3 && !!pVDev4);
}

void LifecycleTest::testMultiDispose()
{
    VclPtrInstance<WorkWindow> xWin(nullptr, WB_APP|WB_STDWORK);
    CPPUNIT_ASSERT(xWin.get() != NULL);
    xWin->disposeOnce();
    xWin->disposeOnce();
    xWin->disposeOnce();
    CPPUNIT_ASSERT(xWin->GetWindow(GetWindowType::Parent) == NULL);
    CPPUNIT_ASSERT(xWin->GetChild(0) == NULL);
    CPPUNIT_ASSERT(xWin->GetChildCount() == 0);
}

void LifecycleTest::testWidgets(vcl::Window *pParent)
{
    { ScopedVclPtrInstance< PushButton > aPtr( pParent );   }
    { ScopedVclPtrInstance< OKButton > aPtr( pParent );     }
    { ScopedVclPtrInstance< CancelButton > aPtr( pParent ); }
    { ScopedVclPtrInstance< HelpButton > aPtr( pParent );   }

    // Some widgets really insist on adoption.
    if (pParent)
    {
        { ScopedVclPtrInstance< CheckBox > aPtr( pParent );     }
        { ScopedVclPtrInstance< Edit > aPtr( pParent );         }
        { ScopedVclPtrInstance< ComboBox > aPtr( pParent );     }
        { ScopedVclPtrInstance< RadioButton > aPtr( pParent );  }
    }
}

void LifecycleTest::testIsolatedWidgets()
{
    testWidgets(NULL);
}

void LifecycleTest::testParentedWidgets()
{
    ScopedVclPtrInstance<WorkWindow> xWin(nullptr, WB_APP|WB_STDWORK);
    CPPUNIT_ASSERT(xWin.get() != NULL);
    xWin->Show();
    testWidgets(xWin);
}

class DisposableChild : public vcl::Window
{
public:
    DisposableChild(vcl::Window *pParent) : vcl::Window(pParent) {}
    virtual ~DisposableChild()
    {
        disposeOnce();
    }
};

void LifecycleTest::testChildDispose()
{
    VclPtrInstance<WorkWindow> xWin(nullptr, WB_APP|WB_STDWORK);
    CPPUNIT_ASSERT(xWin.get() != NULL);
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

class FocusCrashPostDispose : public TabControl
{
public:
    FocusCrashPostDispose(vcl::Window *pParent) :
        TabControl(pParent, 0)
    {
    }
    virtual bool PreNotify( NotifyEvent& ) SAL_OVERRIDE
    {
        return false;
    }
    virtual bool Notify( NotifyEvent& ) SAL_OVERRIDE
    {
//        CPPUNIT_FAIL("notify");
        return false;
    }
    virtual void GetFocus() SAL_OVERRIDE
    {
        CPPUNIT_FAIL("get focus");
    }
    virtual void LoseFocus() SAL_OVERRIDE
    {
        CPPUNIT_FAIL("this should never be called");
    }
};

void LifecycleTest::testFocus()
{
    ScopedVclPtrInstance<WorkWindow> xWin(nullptr, WB_APP|WB_STDWORK);
    ScopedVclPtrInstance< FocusCrashPostDispose > xChild(xWin);
    xWin->Show();
    xChild->GrabFocus();
    // process asynchronous ToTop
    Scheduler::ProcessTaskScheduling(false);
    // FIXME: really awful to test focus issues without showing windows.
    // CPPUNIT_ASSERT(xChild->HasFocus());
}

CPPUNIT_TEST_SUITE_REGISTRATION(LifecycleTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
