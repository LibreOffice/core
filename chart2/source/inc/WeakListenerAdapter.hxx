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
#include <com/sun/star/util/XModifyListener.hpp>
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
template< class Listener >
    class WeakListenerAdapter : public
    ::cppu::WeakImplHelper< Listener >
{
public:
    explicit WeakListenerAdapter( const css::uno::Reference< Listener > & xListener ) :
            m_xListener( xListener )
    {}
    explicit WeakListenerAdapter( const css::uno::WeakReference< Listener > & xListener ) :
            m_xListener( xListener )
    {}

protected:
    // ____ XEventListener (base of all listeners) ____
    virtual void SAL_CALL disposing(
        const css::lang::EventObject& Source ) override
    {
        css::uno::Reference< css::lang::XEventListener > xEventListener(
                  css::uno::Reference< Listener >( m_xListener), css::uno::UNO_QUERY );
        if( xEventListener.is())
            xEventListener->disposing( Source );
    }

    css::uno::Reference< Listener > getListener() const
    {
        return m_xListener;
    }

private:
    css::uno::WeakReference< Listener > m_xListener;
};

class WeakModifyListenerAdapter :
        public WeakListenerAdapter< css::util::XModifyListener >
{
public:
    explicit WeakModifyListenerAdapter(
        const css::uno::WeakReference< css::util::XModifyListener > & xListener );
    virtual ~WeakModifyListenerAdapter() override;

protected:
    // ____ XModifyListener ____
    virtual void SAL_CALL modified( const css::lang::EventObject& aEvent ) override;
};

class WeakSelectionChangeListenerAdapter :
        public WeakListenerAdapter< css::view::XSelectionChangeListener >
{
public:
    explicit WeakSelectionChangeListenerAdapter(
        const css::uno::Reference< css::view::XSelectionChangeListener > & xListener );
    virtual ~WeakSelectionChangeListenerAdapter() override;

protected:
    // ____ XSelectionChangeListener ____
    virtual void SAL_CALL selectionChanged(
        const css::lang::EventObject& aEvent ) override;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
