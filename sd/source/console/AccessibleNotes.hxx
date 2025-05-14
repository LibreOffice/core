/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include "AccessibleObject.hxx"

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ref.hxx>

namespace sdext::presenter
{
class PresenterTextView;

class AccessibleNotes : public AccessibleObject
{
public:
    AccessibleNotes(const OUString& rsName);

    static rtl::Reference<AccessibleObject>
    Create(const css::uno::Reference<awt::XWindow>& rxContentWindow,
           const css::uno::Reference<awt::XWindow>& rxBorderWindow,
           const std::shared_ptr<PresenterTextView>& rpTextView);

    void SetTextView(const std::shared_ptr<PresenterTextView>& rpTextView);

    virtual void SetWindow(const css::uno::Reference<css::awt::XWindow>& rxContentWindow,
                           const css::uno::Reference<css::awt::XWindow>& rxBorderWindow) override;

private:
    std::shared_ptr<PresenterTextView> mpTextView;

    void NotifyCaretChange(const sal_Int32 nOldParagraphIndex, const sal_Int32 nOldCharacterIndex,
                           const sal_Int32 nNewParagraphIndex, const sal_Int32 nNewCharacterIndex);
};

} // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
