/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

