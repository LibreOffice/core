/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_NAVMGR_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_NAVMGR_HXX

#include <vector>
#include <memory>

#include "swtypes.hxx"
#include "calbck.hxx"
#include "unocrsr.hxx"
#include "vcl/svapp.hxx"

class   SwWrtShell;
struct  SwPosition;
class SwUnoCursor;

class SwNavigationMgr
{
private:
    /*
     * List of entries in the navigation history
     * Entries are SwUnoCursor because these gets corrected automatically
     * when nodes are deleted.
     *
     * The navigation history behaves as a stack, to which items are added when we jump to a new position
     * (e.g. click a link, or double click an entry from the navigator).
     * Every use of the back/forward buttons results in moving the stack pointer within the navigation history
     */
    typedef ::std::vector< sw::UnoCursorPointer > Stack_t;
    Stack_t m_entries;
    Stack_t::size_type m_nCurrent; /* Current position within the navigation history */
    SwWrtShell & m_rMyShell; /* The active shell within which the navigation occurs */

    void GotoSwPosition(const SwPosition &rPos);

public:
    /* Constructor that initializes the shell to the current shell */
    SwNavigationMgr( SwWrtShell & rShell );
    virtual ~SwNavigationMgr()
    {
        SolarMutexGuard g;
        m_entries.clear();
    }
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
