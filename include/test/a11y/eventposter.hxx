/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <test/testdllapi.hxx>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <rtl/ustring.hxx>
#include <vcl/window.hxx>

namespace test
{
/**
 * @brief Base helper class to send events to a window.
 *
 * Implementations of this helper will usually just wrap an implementation of post*Event*() calls.
 * This class is mostly useful to encapsulate the calls when getting the target window is not
 * trivial or is only relevant to sending events, and to have a generic event poster interface.
 *
 * Additionally, this class provides simplified helpers to send event pairs, like key down/up, or
 * text+commit, to make it easier on the common case for callers.
 */
class OOO_DLLPUBLIC_TEST EventPosterHelperBase
{
public:
    virtual ~EventPosterHelperBase(){};

    /** @see SfxLokHelper::postKeyEventAsync */
    virtual void postKeyEventAsync(int nType, int nCharCode, int nKeyCode) const = 0;

    /** Posts a full key down/up cycle */
    void postKeyEventAsync(int nCharCode, int nKeyCode) const
    {
        postKeyEventAsync(LOK_KEYEVENT_KEYINPUT, nCharCode, nKeyCode);
        postKeyEventAsync(LOK_KEYEVENT_KEYUP, nCharCode, nKeyCode);
    }

    /** @see SfxLokHelper::postExtTextEventAsync */
    virtual void postExtTextEventAsync(int nType, const OUString& rText) const = 0;

    /** Posts a full text input + commit sequence */
    void postExtTextEventAsync(const OUString& rText) const
    {
        postExtTextEventAsync(LOK_EXT_TEXTINPUT, rText);
        postExtTextEventAsync(LOK_EXT_TEXTINPUT_END, rText);
    }
};

/**
 * @brief Helper to send events to a window.
 *
 * This helper basically just wraps SfxLokHelper::post*EventAsync() calls to hold the target window
 * reference in the class.
 */
class OOO_DLLPUBLIC_TEST EventPosterHelper : public EventPosterHelperBase
{
protected:
    VclPtr<vcl::Window> mxWindow;

public:
    EventPosterHelper(void)
        : mxWindow(nullptr)
    {
    }
    EventPosterHelper(VclPtr<vcl::Window> xWindow)
        : mxWindow(xWindow)
    {
    }
    EventPosterHelper(vcl::Window* pWindow)
        : mxWindow(pWindow)
    {
    }

    vcl::Window* getWindow() const { return mxWindow; }

    void setWindow(VclPtr<vcl::Window> xWindow) { mxWindow = xWindow; }
    void setWindow(vcl::Window* pWindow) { mxWindow = pWindow; }

    explicit operator bool() const { return mxWindow && !mxWindow->isDisposed(); }
    bool operator!() const { return !bool(*this); }

    using EventPosterHelperBase::postKeyEventAsync;
    using EventPosterHelperBase::postExtTextEventAsync;

    /** @see SfxLokHelper::postKeyEventAsync */
    virtual void postKeyEventAsync(int nType, int nCharCode, int nKeyCode) const override;
    /** @see SfxLokHelper::postExtTextEventAsync */
    virtual void postExtTextEventAsync(int nType, const OUString& rText) const override;
};

/**
 * @brief Accessibility-specialized helper to send events to a window.
 *
 * This augments @c test::EventPosterHelper to simplify usage in accessibility tests.
 */
class OOO_DLLPUBLIC_TEST AccessibleEventPosterHelper : public EventPosterHelper
{
public:
    AccessibleEventPosterHelper(void)
        : EventPosterHelper()
    {
    }
    AccessibleEventPosterHelper(const css::uno::Reference<css::accessibility::XAccessible>& xAcc)
    {
        setWindow(xAcc);
    }

    /**
     * @brief Sets the window on which post events based on an accessible object inside it.
     * @param xAcc An accessible object inside a toplevel.
     *
     * This method tries and find the top level window containing @p xAcc to use it to post events.
     *
     * This currently relies on a toplevel accessible being a @c VCLXWindow, and requires that
     * window's output device to be set (@see VCLXWindow::GetWindow()).
     */
    void setWindow(css::uno::Reference<css::accessibility::XAccessible> xAcc);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
