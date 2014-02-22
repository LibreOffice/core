/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
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

/**
 *  If SMART is defined, the navigation history has recency with temporal ordering enhancement,
 *  as described on http:
 */

#define SMART 1



void SwNavigationMgr::GotoSwPosition(const SwPosition &rPos) {
    
    
    m_rMyShell.EnterStdMode();
    m_rMyShell.StartAllAction();
    
    
    
    SwPaM* pPaM = m_rMyShell.GetCrsr();

    if(pPaM->HasMark())
        pPaM->DeleteMark();      
    *pPaM->GetPoint() = rPos;    

    m_rMyShell.EndAllAction();
}





SwNavigationMgr::SwNavigationMgr(SwWrtShell & rShell)
    : m_nCurrent(0), m_rMyShell(rShell)
{
}






bool SwNavigationMgr::backEnabled() {
    return (m_nCurrent > 0);
}







bool SwNavigationMgr::forwardEnabled() {
    return m_nCurrent+1 < m_entries.size();
}





void SwNavigationMgr::goBack()  {

    
    
    

    if (backEnabled()) {
        /* Trying to get the current cursor */
        SwPaM* pPaM = m_rMyShell.GetCrsr();
        if (!pPaM) {
            return;
        }
        

        bool bForwardWasDisabled = !forwardEnabled();

        
        
        

        if (bForwardWasDisabled) {

            
            
            

            
            

            if (addEntry(*pPaM->GetPoint()) ) {
                m_nCurrent--;
            }
        }
        m_nCurrent--;
        
        GotoSwPosition(*m_entries[m_nCurrent]->GetPoint());
        
        if (bForwardWasDisabled)
            m_rMyShell.GetView().GetViewFrame()->GetBindings().Invalidate(FN_NAVIGATION_FORWARD);
        if (!backEnabled())
            m_rMyShell.GetView().GetViewFrame()->GetBindings().Invalidate(FN_NAVIGATION_BACK);
    }
}



void SwNavigationMgr::goForward() {

    
    
    

    if (forwardEnabled()) {
        
        bool bBackWasDisabled = !backEnabled();
        
        
        m_nCurrent++;
        GotoSwPosition(*m_entries[m_nCurrent]->GetPoint());
        
        if (bBackWasDisabled)
            m_rMyShell.GetView().GetViewFrame()->GetBindings().Invalidate(FN_NAVIGATION_BACK);
        if (!forwardEnabled())
            m_rMyShell.GetView().GetViewFrame()->GetBindings().Invalidate(FN_NAVIGATION_FORWARD);
    }
}




bool SwNavigationMgr::addEntry(const SwPosition& rPos) {
    
    bool bBackWasDisabled = !backEnabled();
    bool bForwardWasEnabled = forwardEnabled();

    bool bRet = false; 
                       
                       
#if SMART
    
    
    if (bForwardWasEnabled) {

        size_t number_ofm_entries = m_entries.size(); 
        int curr = m_nCurrent; 
        int n = (number_ofm_entries - curr) / 2; 
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

    
    if (bBackWasDisabled)
        m_rMyShell.GetView().GetViewFrame()->GetBindings().Invalidate(FN_NAVIGATION_BACK);
    if (bForwardWasEnabled)
        m_rMyShell.GetView().GetViewFrame()->GetBindings().Invalidate(FN_NAVIGATION_FORWARD);

    
    css::uno::Reference< css::frame::XFrame > xFrame =
        m_rMyShell.GetView().GetViewFrame()->GetFrame().GetFrameInterface();
    if (xFrame.is())
    {
        css::uno::Reference< css::beans::XPropertySet > xPropSet(xFrame, css::uno::UNO_QUERY);
        if (xPropSet.is())
        {
            css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
            css::uno::Any aValue = xPropSet->getPropertyValue("LayoutManager");

            aValue >>= xLayoutManager;
            if (xLayoutManager.is())
            {
                const OUString sResourceURL( "private:resource/toolbar/navigationobjectbar" );
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
