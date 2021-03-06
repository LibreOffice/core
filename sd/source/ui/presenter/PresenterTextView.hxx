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

#include <tools/PropertySet.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/implbase.hxx>
#include <memory>

namespace sd::presenter {

typedef ::cppu::ImplInheritanceHelper <
    tools::PropertySet,
    css::lang::XInitialization
> PresenterTextViewInterfaceBase;

/** Render text into bitmaps.  An edit engine is used to render the text.
    This service is used by the presenter screen to render the notes view.
*/
class PresenterTextView
    : public PresenterTextViewInterfaceBase
{
public:
    PresenterTextView ();
    virtual ~PresenterTextView() override;
    PresenterTextView(const PresenterTextView&) = delete;
    PresenterTextView& operator=(const PresenterTextView&) = delete;

    // XInitialization

    virtual void SAL_CALL initialize (const css::uno::Sequence<css::uno::Any>& rArguments) override;

protected:
    virtual void SAL_CALL disposing() override;

    virtual css::uno::Any GetPropertyValue (
        const OUString& rsPropertyName) override;
    virtual css::uno::Any SetPropertyValue (
        const OUString& rsPropertyName,
        const css::uno::Any& rValue) override;

private:
    class Implementation;
    std::unique_ptr<Implementation> mpImplementation;

    /** @throws css::lang::DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed();
};

} // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
