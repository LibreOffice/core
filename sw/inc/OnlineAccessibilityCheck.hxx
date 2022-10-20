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

#include "ndindex.hxx"
#include "ndtxt.hxx"
#include <svl/listener.hxx>
#include <vcl/timer.hxx>
#include <AccessibilityCheck.hxx>

struct SwPosition;
class SwTextNode;

namespace sw
{
class OnlineAccessibilityCheck : public SvtListener
{
private:
    SwDoc& m_rDocument;
    sw::AccessibilityCheck m_aAccessibilityCheck;
    SwTextNode* m_pCurrentTextNode;
    SwNodeOffset m_aCurrentNodeIndex;
    sal_Int32 m_nAccessibilityIssues;

    void runCheck(SwTextNode* pTextNode);

public:
    OnlineAccessibilityCheck(SwDoc& rDocument);
    void update(const SwPosition& rNewPos);
    sal_Int32 getNumberOfAccessibilityIssues() { return m_nAccessibilityIssues; }
};

} // end sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
