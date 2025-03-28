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

#include <SwGrammarMarkUp.hxx>
#include <svl/listener.hxx>
#include <vcl/timer.hxx>

struct SwPosition;
class SwTextNode;

namespace sw
{
/**
 * This class is responsible for the delayed display of grammar checks when a paragraph is edited
 * It's a client of the paragraph the cursor points to.
 * If the cursor position changes, updateCursorPosition has to be called
 * If the grammar checker wants to set a grammar marker at a paragraph, he has to request
 * the grammar list from this class. If the requested paragraph is not edited, it returns
 * the normal grammar list. But if the paragraph is the active one, a proxy list will be returned and
 * all changes are set in this proxy list. If the cursor leaves the paragraph the proxy list
 * will replace the old list. If the grammar checker has completed the paragraph ('setChecked')
 * then a timer is setup which replaces the old list as well.
 */
class GrammarContact final : public SvtListener
{
    Timer m_aTimer;
    std::unique_ptr<SwGrammarMarkUp> m_pProxyList;
    bool m_isFinished;
    SwTextNode* m_pTextNode;
    DECL_LINK(TimerRepaint, Timer*, void);

public:
    GrammarContact();
    ~GrammarContact() { m_aTimer.Stop(); }

    /** Update cursor position reacts to a change of the current input cursor
        As long as the cursor in inside a paragraph, the grammar checking does
        not show new grammar faults. When the cursor leaves the paragraph, these
        faults are shown.
    @returns void
    */
    void updateCursorPosition(const SwPosition& rNewPos);

    /** getGrammarCheck checks if the given text node is blocked by the current cursor
        if not, the normal markup list is returned
        if blocked, it will return a markup list "proxy"
    @returns a markup list (grammar) for the given SwTextNode
    */
    SwGrammarMarkUp* getGrammarCheck(SwTextNode& rTextNode, bool bCreate);

    /** finishGrammarCheck() has to be called if a grammar checking has been completed
        for a text node. If this text node has not been hidden by the current proxy list
        it will be repainted. Otherwise the proxy list replaces the old list and the
        repaint will be triggered by a timer
    @returns void
    */
    void finishGrammarCheck(const SwTextNode& rTextNode);

    void CheckBroadcaster();
};

/* Helper functions */

/** getGrammarContact() delivers the grammar contact of the document (for a given textnode)
@returns grammar contact
*/
GrammarContact* getGrammarContactFor(const SwTextNode&);

/** finishGrammarCheck() calls the same function of the grammar contact of the document (for a given textnode)
@returns void
*/
void finishGrammarCheckFor(const SwTextNode&);

} // end sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
