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

#include <vector>
#include <cppuhelper/weakref.hxx>
#include "sddllapi.h"

class SdPage;

class SD_DLLPUBLIC SdCustomShow final
{
public:
    typedef ::std::vector<const SdPage*> PageVec;

private:
    PageVec maPages;
    OUString aName;

    // this is a weak reference to a possible living api wrapper for this custom show
    css::uno::WeakReference<css::uno::XInterface> mxUnoCustomShow;

public:
    SdCustomShow();
    SdCustomShow(css::uno::Reference<css::uno::XInterface> const& xShow);

    ~SdCustomShow();

    // @@@ copy ctor, but no copy assignment? @@@
    SdCustomShow(const SdCustomShow& rShow);

    SdCustomShow& operator=(const SdCustomShow& rShow) = delete;

    /** Provides a direct access to the collection of the SdPage objects. */
    PageVec& PagesVector() { return maPages; }
    /**
     * Replaces all occurrences of pOldPage with pNewPage.
     * If pNewPage is 0 then removes all occurrences of pOldPage.
     */
    void ReplacePage(const SdPage* pOldPage, const SdPage* pNewPage);

    void SetName(const OUString& rName);
    const OUString& GetName() const { return aName; }

    css::uno::Reference<css::uno::XInterface> getUnoCustomShow();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
