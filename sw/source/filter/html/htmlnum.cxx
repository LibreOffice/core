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

#include "htmlnum.hxx"
#include <ndtxt.hxx>
#include <doc.hxx>

void SwHTMLNumRuleInfo::Set(const SwTextNode& rTextNd)
{
    const SwNumRule* pTextNdNumRule(rTextNd.GetNumRule());
    if (pTextNdNumRule && pTextNdNumRule != rTextNd.GetDoc().GetOutlineNumRule())
    {
        m_pNumRule = const_cast<SwNumRule*>(pTextNdNumRule);
        m_nDeep = o3tl::narrowing<sal_uInt16>(m_pNumRule ? rTextNd.GetActualListLevel() + 1 : 0);
        m_bNumbered = rTextNd.IsCountedInList();
        // #i57919# - correction of refactoring done by cws swnumtree:
        // <bRestart> has to be set to <true>, if numbering is restarted at this
        // text node and the start value equals <USHRT_MAX>.
        // Start value <USHRT_MAX> indicates, that at this text node the numbering
        // is restarted with the value given at the corresponding level.
        m_bRestart = rTextNd.IsListRestart() && !rTextNd.HasAttrListRestartValue();
    }
    else
    {
        m_pNumRule = nullptr;
        m_nDeep = 0;
        m_bNumbered = m_bRestart = false;
    }
}

// Restart flag is only effective when this level is not below the previous
bool SwHTMLNumRuleInfo::IsRestart(const SwHTMLNumRuleInfo& rPrev) const
{
    // calling this, when the rules are different, makes no sense
    assert(rPrev.GetNumRule() == GetNumRule());

    // An example ODF when the restart flag is set, but has no effect:
    //   <text:list text:style-name="L1">
    //    <text:list-item>
    //     <text:p>l1</text:p>
    //     <text:list>
    //      <text:list-item>
    //       <text:p>l2</text:p>
    //      </text:list-item>
    //      <text:list-item>
    //       <text:p>l2</text:p>
    //      </text:list-item>
    //     </text:list>
    //     <text:list>
    //      <text:list-item>
    //       <text:list>
    //        <text:list-item>
    //         <text:p>l3</text:p>
    //        </text:list-item>
    //       </text:list>
    //      </text:list-item>
    //     </text:list>
    //    </text:list-item>
    //   </text:list>
    // In this case, "l3" is in a separate sublist than "l2", and so the "l3" node gets the
    // "list restart" property. But the document rendering would be
    //   1. l1
    //        1.1. l2
    //        1.2. l2
    //             1.2.1. l3
    // and the second-level numbering will not actually restart at the "l3" node.
    //
    // TODO/LATER: note that restarting may happen at different levels. In the code using this
    // function, the level is reset to 0 whenever a restart is detected. And also, there is no
    // code to actually descend to that new level (close corresponding li/ul/ol elements).

    if (rPrev.GetDepth() < GetDepth())
        return false; // No matter if the restart flag is set, it is not effective for subitems
    return m_bRestart;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
