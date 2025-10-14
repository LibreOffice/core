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

#include <sfx2/AccessibilityCheck.hxx>
#include <doc.hxx>
#include <node.hxx>

namespace sw
{
/** Base class for accessibility checks */
class BaseCheck
{
protected:
    sfx::AccessibilityIssueCollection& m_rIssueCollection;

public:
    BaseCheck(sfx::AccessibilityIssueCollection& rIssueCollection)
        : m_rIssueCollection(rIssueCollection)
    {
    }
    virtual ~BaseCheck() {}
};

class SW_DLLPUBLIC AccessibilityCheck final : public sfx::AccessibilityCheck
{
private:
    SwDoc* m_pDoc;

    std::vector<std::shared_ptr<BaseCheck>> m_aDocumentChecks;
    std::vector<std::shared_ptr<BaseCheck>> m_aNodeChecks;

    AccessibilityCheck() = delete;

    void init();

public:
    AccessibilityCheck(SwDoc* pDoc)
        : m_pDoc(pDoc)
    {
    }

    void check() override;
    void checkObject(SwNode* pNode, SwFrameFormat const&);
    void checkNode(SwNode* pNode);
    void checkDocumentProperties();
};

} // end sw namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
