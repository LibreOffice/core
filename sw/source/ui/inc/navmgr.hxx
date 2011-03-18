/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       [ Maja Djordjevic < ovcica@gmail.com > ]
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Cédric Bosdonnat <cbosdonnat@novell.com>
 *                 Caolan McNamara <caolanm@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#ifndef _NAVMGR_HXX
#define _NAVMGR_HXX

#include "swtypes.hxx"
#include "pam.hxx"
#include "swdllapi.h"

class   SwWrtShell;
struct  SwPosition;

class SW_DLLPUBLIC SwNavigationMgr
{
private:
    /*
     * List of entries in the navigation history
     * Each entry is a SwPosition, which represents a position within the document
     * SwPosition is given by a node index (SwNodeIndex) which usually represents the paragraph the position is in
     * and an index (SwIndex), which represents the position inside this paragraph.
     * You can find more on SwPositions at http://wiki.services.openoffice.org/wiki/Writer_Core_And_Layout
     *
     * The navigation history behaves as a stack, to which items are added when we jump to a new position
     * (e.g. click a link, or double click an entry from the navigator).
     * Every use of the back/forward buttons results in moving the stack pointer within the navigation history
     */
    std::vector<SwPosition> _entries;
    std::vector<SwPosition>::size_type _nCurrent; /* Current position within the navigation history */
    SwWrtShell* _pMyShell; /* The active shell within which the navigation occurs */

    void GotoSwPosition(const SwPosition &rPos);

public:
    /* Constructor that initializes the shell to the current shell */
    SwNavigationMgr( SwWrtShell* pShell );
    /* Can we go back in the history ? */
    sal_Bool backEnabled() ;
    /* Can we go forward in the history ? */
    sal_Bool forwardEnabled();
    /* The method that is called when we click the back button */
    void goBack() ;
    /* The method that is called when we click the forward button */
    void goForward() ;
    /* The method that adds the position pPos to the navigation history */
    bool addEntry(const SwPosition& rPos);
};
#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
