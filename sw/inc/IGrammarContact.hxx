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

#ifndef INCLUDED_SW_INC_IGRAMMARCONTACT_HXX
#define INCLUDED_SW_INC_IGRAMMARCONTACT_HXX

struct SwPosition;
class SwTextNode;
class SwGrammarMarkUp;
class SwDoc;

/** Organizer of the contact between SwTextNodes and grammar checker
*/
class IGrammarContact
{
public:

    /** Update cursor position reacts to a change of the current input cursor
        As long as the cursor in inside a paragraph, the grammar checking does
        not show new grammar faults. When the cursor leaves the paragraph, these
        faults are shown.
    @returns void
    */
    virtual void updateCursorPosition( const SwPosition& rNewPos ) = 0;

    /** getGrammarCheck checks if the given text node is blocked by the current cursor
        if not, the normal markup list is returned
        if blocked, it will return a markup list "proxy"
    @returns a markup list (grammar) for the given SwTextNode
    */
    virtual SwGrammarMarkUp* getGrammarCheck( SwTextNode& rTextNode, bool bCreate ) = 0;

    /** finishGrammarCheck() has to be called if a grammar checking has been completed
        for a text node. If this text node has not been hided by the current proxy list
        it will be repainted. Otherwise the proxy list replaces the old list and the
        repaint will be triggered by a timer
    @returns void
    */
    virtual void finishGrammarCheck( SwTextNode& rTextNode ) = 0;

public:
    virtual ~IGrammarContact() {}
};

/** Factory for a grammar contact
@returns a new created grammar contact object
*/
IGrammarContact* createGrammarContact();

/* Helper functions */

/** getGrammarContact() delivers the grammar contact of the document (for a given textnode)
@returns grammar contact
*/
IGrammarContact* getGrammarContact( const SwTextNode& );

/** finishGrammarCheck() calls the same function of the grammar contact of the document (for a given textnode)
@returns void
*/
void finishGrammarCheck( SwTextNode& );

#endif // INCLUDED_SW_INC_IGRAMMARCONTACT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
