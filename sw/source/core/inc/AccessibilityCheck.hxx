/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SW_SOURCE_CORE_ACCESSIBILITYCHECK_HXX
#define INCLUDED_SW_SOURCE_CORE_ACCESSIBILITYCHECK_HXX

#include <doc.hxx>
#include <svx/AccessibilityIssue.hxx>
#include <node.hxx>

class AccessibilityCheck
{
    SwDoc* m_pDoc;
    std::vector<svx::AccessibilityIssue> m_aAccessibilityIssueCollection;

public:
    AccessibilityCheck(SwDoc* pDoc)
        : m_pDoc(pDoc)
    {
    }

    std::vector<svx::AccessibilityIssue> const& getIssueCollecton()
    {
        return m_aAccessibilityIssueCollection;
    }

    void check();
    void checkNoTextNode(SwNoTextNode* pNoTextNode);
    void checkObject(SdrObject* pObject);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
