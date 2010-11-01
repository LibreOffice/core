/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef IGRAMMARCONTACT_HXX_INCLUDED
#define IGRAMMARCONTACT_HXX_INCLUDED

struct SwPosition;
class SwTxtNode;
class SwGrammarMarkUp;
class SwDoc;

/** Organizer of the contact between SwTxtNodes and grammar checker
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
    @returns a markup list (grammar) for the given SwTxtNode
    */
    virtual SwGrammarMarkUp* getGrammarCheck( SwTxtNode& rTxtNode, bool bCreate ) = 0;

    /** finishGrammarCheck() has to be called if a grammar checking has been completed
        for a text node. If this text node has not been hided by the current proxy list
        it will be repainted. Otherwise the proxy list replaces the old list and the
        repaint will be triggered by a timer
    @returns void
    */
    virtual void finishGrammarCheck( SwTxtNode& rTxtNode ) = 0;

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
IGrammarContact* getGrammarContact( const SwTxtNode& );

/** finishGrammarCheck() calls the same function of the grammar contact of the document (for a given textnode)
@returns void
*/
void finishGrammarCheck( SwTxtNode& );

#endif // IGRAMMARCONTACT_HXX_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
