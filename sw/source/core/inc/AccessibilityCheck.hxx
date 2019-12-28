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

#include <svx/AccessibilityCheck.hxx>
#include <doc.hxx>

namespace sw
{
class AccessibilityCheck final : public svx::AccessibilityCheck
{
private:
    SwDoc* m_pDoc;

public:
    AccessibilityCheck(SwDoc* pDoc)
        : m_pDoc(pDoc)
    {
    }

    void check() override;
    void checkObject(SdrObject* pObject);
};

} // end sw namespace


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
