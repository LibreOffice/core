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
#include "precompiled_sw.hxx"

#include "navmgr.hxx"
#include "wrtsh.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <cmdid.h>
#include <view.hxx>

/**
 * If SMART is defined, the navigation history has recency with temporal ordering enhancement,
 * as described on http://zing.ncsl.nist.gov/hfweb/proceedings/greenberg/
 */
#define SMART 1

/*
 *  This method positions the cursor to the position rPos
 */
void SwNavigationMgr::GotoSwPosition(const SwPosition &rPos) {
    SwWrtShell& rSh = *_pMyShell;
    /* EnterStdMode() prevents the cursor to 'block' the current shell when it should move from the image back to the normal shell */
    rSh.EnterStdMode();
    rSh.StartAllAction();
    /*
     *    cursor consists of two SwPositions: Point and Mark.
     *  Such a pair is called a PaM. SwPaM is derived from SwRing.
     *  The Ring contains the single regions of a multi-selection.
     */
    SwPaM* pPaM = rSh.GetCrsr();

    if(pPaM->HasMark())
        pPaM->DeleteMark();      // If there was a selection, get rid of it
    *pPaM->GetPoint() = rPos;    // Position Cursor

    rSh.EndAllAction();
}
/*
 * Ctor for the SwNavigationMgr class
 * Sets the shell to the current shell
 * and the index of the current position to 0
 */

SwNavigationMgr::SwNavigationMgr(SwWrtShell* pShell)
    : _nCurrent(0), _pMyShell(pShell) {
}
/*
 * This method is used by the navigation shell - defined in sw/source/ui/inc/navsh.hxx
 * and implemented in sw/source/ui/shells/navsh.cxx
 * It is called when we want to check if the back button should be enabled or not.
 * The back button should be enabled only if there are some entries in the navigation history
 */
sal_Bool SwNavigationMgr::backEnabled() {
    return (_nCurrent > 0);
}
/*
 * Similar to backEnabled() method.
 * The forward button should be enabled if we ever clicked back
 * Due to the implementation of the navigation class, this is when the
 * current position within the navigation history entries in not the last one
 * i.e. when the _nCurrent index is not at the end of the _entries vector
 */
sal_Bool SwNavigationMgr::forwardEnabled() {
    return _nCurrent+1 < _entries.size();
}


/*
 * The goBack() method positions the cursor to the previous entry in the navigation history
 * If there was no history to go forward to, it adds the current position of the cursor
 * to the history so we could go forward to where we came from
 */
void SwNavigationMgr::goBack()  {
    /*
     * Although the button should be disabled whenever the backEnabled() returns false,
     * the UI is sometimes not as responsive as we would like it to be :)
     * this check prevents segmentation faults and in this way the class is not relying on the UI
     */
    if (backEnabled()) {
        SwWrtShell& rSh = *_pMyShell;
        /* Trying to get the current cursor */
        SwPaM* pPaM = rSh.GetCrsr();
        if (!pPaM) {
            return;
        }

        /* This flag will be used to manually refresh the buttons */
        bool bForwardWasDisabled = !forwardEnabled();
        /*
         * If we're going backwards in our history, but the current location is not
         * in the history then we need to add *here* to it so that we can "go
         * forward" to here again.
         */

        if (bForwardWasDisabled) {
            /*
             * the cursor consists of two SwPositions: Point and Mark.
             * We are adding the current Point to the navigation history
             * so we could later navigate forward to it
             */
            /* The addEntry() method returns true iff we should decrement the index before navigating back */
            if (addEntry(*pPaM->GetPoint()) ) {
                _nCurrent--;
            }
        }
        _nCurrent--;
        /* Position cursor to appropriate navigation history entry */
        GotoSwPosition(_entries[_nCurrent]);
        /* Refresh the buttons */
        if (bForwardWasDisabled)
            _pMyShell->GetView().GetViewFrame()->GetBindings().Invalidate(FN_NAVIGATION_FORWARD);
        if (!backEnabled())
            _pMyShell->GetView().GetViewFrame()->GetBindings().Invalidate(FN_NAVIGATION_BACK);
    }
}
/*
 * The goForward() method positions the cursor to the next entry in the navigation history
 */

void SwNavigationMgr::goForward() {
    /*
     * Although the button should be disabled whenever the backForward() returns false,
     * the UI is sometimes not as responsive as we would like it to be :)
     * this check prevents segmentation faults and in this way the class is not relying on the UI
     */

    if (forwardEnabled()) {
        /* This flag will be used to manually refresh the buttons */
        bool bBackWasDisabled = !backEnabled();
        /*
         * The current index is positioned at the current entry in the navigation history
         * We have to increment it to go to the next entry
         */
        _nCurrent++;
        GotoSwPosition(_entries[_nCurrent]);
        /* Refresh the buttons */
        if (bBackWasDisabled)
            _pMyShell->GetView().GetViewFrame()->GetBindings().Invalidate(FN_NAVIGATION_BACK);
        if (!forwardEnabled())
            _pMyShell->GetView().GetViewFrame()->GetBindings().Invalidate(FN_NAVIGATION_FORWARD);
    }
}
/*
 * This method adds the SwPosition rPos to the navigation history
 * rPos is usually the current position of the cursor in the document
 */
bool SwNavigationMgr::addEntry(const SwPosition& rPos) {
    /* Flags that will be used for refreshing the buttons */
    bool bBackWasDisabled = !backEnabled();
    bool bForwardWasEnabled = forwardEnabled();

    bool bRet = false; // return value of the function.
                       // Indicates weather the index should be decremented before jumping back or not
#if SMART
    /* If any forward history exists, twist the tail of the list from the current position to the end */
    if (bForwardWasEnabled) {

        size_t number_of_entries = _entries.size(); /* To avoid calling _entries.size() multiple times */
        int curr = _nCurrent; /* Index from which we'll twist the tail. */
        int n = (number_of_entries - curr) / 2; /* Number of entries that will swap places */
        for (int i = 0; i < n; i++) {
            SwPosition temp = _entries[curr + i];
            _entries[curr + i] = _entries[number_of_entries -1 - i];
            _entries[number_of_entries -1 - i] = temp;
        }

           if (_entries.back() != rPos)
           _entries.push_back(rPos);


        bRet = true;
    }
    else {
        if ( (_entries.size() > 0 && _entries.back() != rPos) || (_entries.size() == 0) ) {
            _entries.push_back(rPos);
            bRet = true;
        }
        if (_entries.size() > 1 && _entries.back() == rPos)
            bRet = true;
        if (_entries.size() == 1 && _entries.back() == rPos)
            bRet = false;
    }
#else
    _entries.erase(_entries.begin() + _nCurrent, _entries.end());
    _entries.push_back(rPos);
    bRet = true;
#endif
    _nCurrent = _entries.size();

    /* Refresh buttons */
    if (bBackWasDisabled)
        _pMyShell->GetView().GetViewFrame()->GetBindings().Invalidate(FN_NAVIGATION_BACK);
    if (bForwardWasEnabled)
        _pMyShell->GetView().GetViewFrame()->GetBindings().Invalidate(FN_NAVIGATION_FORWARD);
    return bRet;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
