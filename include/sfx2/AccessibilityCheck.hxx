/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <sfx2/dllapi.h>
#include <sfx2/AccessibilityIssue.hxx>

namespace sfx
{
class SFX2_DLLPUBLIC AccessibilityCheck
{
protected:
    AccessibilityIssueCollection m_aIssueCollection;

public:
    AccessibilityCheck() = default;
    virtual ~AccessibilityCheck();

    virtual void check() = 0;

    AccessibilityIssueCollection& getIssueCollection();
};

} // end sfx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
