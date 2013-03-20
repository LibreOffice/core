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
#if 1

#include "tools/PropertySet.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/implbase1.hxx>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace sd { namespace presenter {

namespace {
    typedef ::cppu::ImplInheritanceHelper1 <
        tools::PropertySet,
        css::lang::XInitialization
    > PresenterTextViewInterfaceBase;
}


/** Render text into bitmaps.  An edit engine is used to render the text.
    This service is used by the presenter screen to render the notes view.
*/
class PresenterTextView
    : private ::boost::noncopyable,
      public PresenterTextViewInterfaceBase
{
public:
    explicit PresenterTextView (const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~PresenterTextView (void);

    // XInitialization

    virtual void SAL_CALL initialize (const css::uno::Sequence<css::uno::Any>& rArguments)
        throw (css::uno::Exception, css::uno::RuntimeException);


protected:
    virtual void SAL_CALL disposing (void);

    virtual css::uno::Any GetPropertyValue (
        const ::rtl::OUString& rsPropertyName);
    virtual css::uno::Any SetPropertyValue (
        const ::rtl::OUString& rsPropertyName,
        const css::uno::Any& rValue);

private:
    class Implementation;
    ::boost::scoped_ptr<Implementation> mpImplementation;

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed (void) throw (css::lang::DisposedException);
};

} } // end of namespace ::sd::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
