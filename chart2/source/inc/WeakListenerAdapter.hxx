/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#pragma once

#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implbase.hxx>

namespace chart
{

/** Adapter that enables adding listeners as weak UNO references.  Thus, adding
    an object as listener to a broadcaster does not increase its reference
    count.

    <p>The helper class, of course, is held as hard reference at the
    broadcaster, but this should never be a problem as the adapter's life time
    depends on no other object.</p>

    <p>Note that in order to remove an object as listener, you have to remove
    the same wrapper that you added, i.e., you should store the adapter as a
    member in the adapted class for later use.</p>
 */
class WeakSelectionChangeListenerAdapter final :
        public ::cppu::WeakImplHelper< css::view::XSelectionChangeListener >
{
public:
    explicit WeakSelectionChangeListenerAdapter(
        const css::uno::Reference< css::view::XSelectionChangeListener > & xListener )
            : m_xListener( xListener ) {}
    virtual ~WeakSelectionChangeListenerAdapter() override;

protected:
    // ____ XSelectionChangeListener ____
    virtual void SAL_CALL selectionChanged(
        const css::lang::EventObject& aEvent ) override;

    // ____ XEventListener (base of all listeners) ____
    virtual void SAL_CALL disposing(
        const css::lang::EventObject& Source ) override
    {
        css::uno::Reference< css::view::XSelectionChangeListener > xEventListener( m_xListener );
        if( xEventListener.is())
            xEventListener->disposing( Source );
    }

private:
    css::uno::WeakReference< css::view::XSelectionChangeListener > m_xListener;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
