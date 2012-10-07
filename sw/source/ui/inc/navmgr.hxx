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
#ifndef SW_NAVMGR_HXX
#define SW_NAVMGR_HXX

#include <vector>

#include <boost/shared_ptr.hpp>

#include "swtypes.hxx"

class   SwWrtShell;
struct  SwPosition;
class SwUnoCrsr;


class SwNavigationMgr
{
private:
    /*
     * List of entries in the navigation history
     * Entries are SwUnoCrsr because thos gets corrected automatically
     * when nodes are deleted.
     *
     * The navigation history behaves as a stack, to which items are added when we jump to a new position
     * (e.g. click a link, or double click an entry from the navigator).
     * Every use of the back/forward buttons results in moving the stack pointer within the navigation history
     */
    typedef ::std::vector< ::boost::shared_ptr<SwUnoCrsr> > Stack_t;
    Stack_t m_entries;
    Stack_t::size_type m_nCurrent; /* Current position within the navigation history */
    SwWrtShell & m_rMyShell; /* The active shell within which the navigation occurs */

    void GotoSwPosition(const SwPosition &rPos);

public:
    /* Constructor that initializes the shell to the current shell */
    SwNavigationMgr( SwWrtShell & rShell );
    /* Can we go back in the history ? */
    bool backEnabled() ;
    /* Can we go forward in the history ? */
    bool forwardEnabled();
    /* The method that is called when we click the back button */
    void goBack() ;
    /* The method that is called when we click the forward button */
    void goForward() ;
    /* The method that adds the position pPos to the navigation history */
    bool addEntry(const SwPosition& rPos);
};
#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
