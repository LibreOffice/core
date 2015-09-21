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

#ifndef INCLUDED_SD_SOURCE_UI_PRESENTER_PRESENTERTEXTVIEW_HXX
#define INCLUDED_SD_SOURCE_UI_PRESENTER_PRESENTERTEXTVIEW_HXX

#include "tools/PropertySet.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/implbase.hxx>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

namespace sd { namespace presenter {

namespace {
    typedef ::cppu::ImplInheritanceHelper <
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
    virtual ~PresenterTextView();

    // XInitialization

    virtual void SAL_CALL initialize (const css::uno::Sequence<css::uno::Any>& rArguments)
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

protected:
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    virtual css::uno::Any GetPropertyValue (
        const OUString& rsPropertyName) SAL_OVERRIDE;
    virtual css::uno::Any SetPropertyValue (
        const OUString& rsPropertyName,
        const css::uno::Any& rValue) SAL_OVERRIDE;

private:
    class Implementation;
    std::unique_ptr<Implementation> mpImplementation;

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed() throw (css::lang::DisposedException);
};

} } // end of namespace ::sd::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
