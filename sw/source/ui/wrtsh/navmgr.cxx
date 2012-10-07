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

#include "navmgr.hxx"
#include "wrtsh.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <cmdid.h>
#include <view.hxx>
#include <doc.hxx>
#include <unocrsr.hxx>

#include <com/sun/star/frame/XLayoutManager.hpp>

namespace css = ::com::sun::star;

/**
 * If SMART is defined, the navigation history has recency with temporal ordering enhancement,
 * as described on http://zing.ncsl.nist.gov/hfweb/proceedings/greenberg/
 */
#define SMART 1

/*
 *  This method positions the cursor to the position rPos
 */
void SwNavigationMgr::GotoSwPosition(const SwPosition &rPos) {
    /* EnterStdMode() prevents the cursor to 'block' the current shell when it should move from the image back to the normal shell */
    m_rMyShell.EnterStdMode();
    m_rMyShell.StartAllAction();
    /*
     *    cursor consists of two SwPositions: Point and Mark.
     *  Such a pair is called a PaM. SwPaM is derived from SwRing.
     *  The Ring contains the single regions of a multi-selection.
     */
    SwPaM* pPaM = m_rMyShell.GetCrsr();

    if(pPaM->HasMark())
        pPaM->DeleteMark();      // If there was a selection, get rid of it
    *pPaM->GetPoint() = rPos;    // Position Cursor

    m_rMyShell.EndAllAction();
}
/*
 * Ctor for the SwNavigationMgr class
 * Sets the shell to the current shell
 * and the index of the current position to 0
 */
SwNavigationMgr::SwNavigationMgr(SwWrtShell & rShell)
    : m_nCurrent(0), m_rMyShell(rShell)
{
}

/*
 * This method is used by the navigation shell - defined in sw/source/ui/inc/navsh.hxx
 * and implemented in sw/source/ui/shells/navsh.cxx
 * It is called when we want to check if the back button should be enabled or not.
 * The back button should be enabled only if there are some entries in the navigation history
 */
bool SwNavigationMgr::backEnabled() {
    return (m_nCurrent > 0);
}
/*
 * Similar to backEnabled() method.
 * The forward button should be enabled if we ever clicked back
 * Due to the implementation of the navigation class, this is when the
 * current position within the navigation history entries in not the last one
 * i.e. when the m_nCurrent index is not at the end of the m_entries vector
 */
bool SwNavigationMgr::forwardEnabled() {
    return m_nCurrent+1 < m_entries.size();
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
        /* Trying to get the current cursor */
        SwPaM* pPaM = m_rMyShell.GetCrsr();
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
                m_nCurrent--;
            }
        }
        m_nCurrent--;
        /* Position cursor to appropriate navigation history entry */
        GotoSwPosition(*m_entries[m_nCurrent]->GetPoint());
        /* Refresh the buttons */
        if (bForwardWasDisabled)
            m_rMyShell.GetView().GetViewFrame()->GetBindings().Invalidate(FN_NAVIGATION_FORWARD);
        if (!backEnabled())
            m_rMyShell.GetView().GetViewFrame()->GetBindings().Invalidate(FN_NAVIGATION_BACK);
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
        m_nCurrent++;
        GotoSwPosition(*m_entries[m_nCurrent]->GetPoint());
        /* Refresh the buttons */
        if (bBackWasDisabled)
            m_rMyShell.GetView().GetViewFrame()->GetBindings().Invalidate(FN_NAVIGATION_BACK);
        if (!forwardEnabled())
            m_rMyShell.GetView().GetViewFrame()->GetBindings().Invalidate(FN_NAVIGATION_FORWARD);
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
                       // Indicates whether the index should be decremented before jumping back or not
#if SMART
    /* If any forward history exists, twist the tail of the list from the current position to the end */
    if (bForwardWasEnabled) {

        size_t number_ofm_entries = m_entries.size(); /* To avoid calling m_entries.size() multiple times */
        int curr = m_nCurrent; /* Index from which we'll twist the tail. */
        int n = (number_ofm_entries - curr) / 2; /* Number of entries that will swap places */
        for (int i = 0; i < n; i++) {
            ::std::swap(m_entries[curr + i], m_entries[number_ofm_entries -1 - i]);
        }

        if (*m_entries.back()->GetPoint() != rPos)
        {
            SwUnoCrsr *const pCursor = m_rMyShell.GetDoc()->CreateUnoCrsr(rPos);
            m_entries.push_back(::boost::shared_ptr<SwUnoCrsr>(pCursor));
        }
        bRet = true;
    }
    else {
        if ( (!m_entries.empty() && *m_entries.back()->GetPoint() != rPos) || m_entries.empty() ) {
            SwUnoCrsr *const pCursor = m_rMyShell.GetDoc()->CreateUnoCrsr(rPos);
            m_entries.push_back(::boost::shared_ptr<SwUnoCrsr>(pCursor));
            bRet = true;
        }
        if (m_entries.size() > 1 && *m_entries.back()->GetPoint() == rPos)
            bRet = true;
        if (m_entries.size() == 1 && *m_entries.back()->GetPoint() == rPos)
            bRet = false;
    }
#else
    m_entries.erase(m_entries.begin() + m_nCurrent, m_entries.end());
    SwUnoCrsr *const pCursor = m_rMyShell.GetDoc()->CreateUnoCrsr(rPos);
    m_entries.push_back(::boost::shared_ptr<SwUnoCrsr>(pCursor));
    bRet = true;
#endif
    m_nCurrent = m_entries.size();

    /* Refresh buttons */
    if (bBackWasDisabled)
        m_rMyShell.GetView().GetViewFrame()->GetBindings().Invalidate(FN_NAVIGATION_BACK);
    if (bForwardWasEnabled)
        m_rMyShell.GetView().GetViewFrame()->GetBindings().Invalidate(FN_NAVIGATION_FORWARD);

    /* show the Navigation toolbar */
    css::uno::Reference< css::frame::XFrame > xFrame =
        m_rMyShell.GetView().GetViewFrame()->GetFrame().GetFrameInterface();
    if (xFrame.is())
    {
        css::uno::Reference< css::beans::XPropertySet > xPropSet(xFrame, css::uno::UNO_QUERY);
        if (xPropSet.is())
        {
            css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
            css::uno::Any aValue = xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" ) ) );

            aValue >>= xLayoutManager;
            if (xLayoutManager.is())
            {
                const ::rtl::OUString sResourceURL( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/navigationobjectbar" ) );
                css::uno::Reference< css::ui::XUIElement > xUIElement = xLayoutManager->getElement(sResourceURL);
                if (!xUIElement.is())
                {
                    xLayoutManager->createElement( sResourceURL );
                    xLayoutManager->showElement( sResourceURL );
                }
            }
        }
    }

    return bRet;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
