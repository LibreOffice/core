/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_LANG_XCOMPONENT_HXX
#define INCLUDED_TEST_LANG_XCOMPONENT_HXX

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>

#include <test/testdllapi.hxx>

namespace apitest
{
class OOO_DLLPUBLIC_TEST XComponent
{
public:
    virtual css::uno::Reference<css::uno::XInterface> init() = 0;
    virtual void triggerDesktopTerminate() = 0;

    void testAddEventListener();
    void testRemoveEventListener();
    void testDisposedByDesktopTerminate();

protected:
    ~XComponent() {}
};

} // namespace apitest

#endif // INCLUDED_TEST_LANG_XCOMPONENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
