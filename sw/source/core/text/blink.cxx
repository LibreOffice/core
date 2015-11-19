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

#include "viewsh.hxx"
#include "rootfrm.hxx"
#include "txtfrm.hxx"
#include "blink.hxx"
#include "porlin.hxx"
#include "porlay.hxx"

// Visible time
#define BLINK_ON_TIME       2400L
// Invisible time
#define BLINK_OFF_TIME      800L

/**
 * pBlink points to the instance where blinking portions need to register.
 * If necessary, it needs to be created by SwBlink.
 * They are then triggered rhythimcally for a repaint. They can query
 * for being visible or invisible with IsVisible().
 */
SwBlink *pBlink = nullptr;

SwBlink::SwBlink()
{
    bVisible = true;
    // Prepare the timer
    aTimer.SetTimeout( BLINK_ON_TIME );
    aTimer.SetTimeoutHdl( LINK(this, SwBlink, Blinker) );
}

SwBlink::~SwBlink( )
{
    aTimer.Stop();
}

/**
 * SwBlink::Blinker (timer):
 * Toggle visibility flag
 * Determine the repaint rectangle and invalidate them in their OleShells.
 */
IMPL_LINK_NOARG_TYPED(SwBlink, Blinker, Timer *, void)
{
    bVisible = !bVisible;
    if( bVisible )
        aTimer.SetTimeout( BLINK_ON_TIME );
    else
        aTimer.SetTimeout( BLINK_OFF_TIME );
    if (!m_List.empty())
    {

        for (SwBlinkSet::iterator it = m_List.begin(); it != m_List.end(); )
        {
            const SwBlinkPortion* pTmp = it->get();
            if( pTmp->GetRootFrame() &&
                pTmp->GetRootFrame()->GetCurrShell() )
            {
                ++it;

                Point aPos = pTmp->GetPos();
                long nWidth, nHeight;
                switch ( pTmp->GetDirection() )
                {
                    case 900:
                        aPos.X() -= pTmp->GetPortion()->GetAscent();
                        aPos.Y() -= pTmp->GetPortion()->Width();
                        nWidth = pTmp->GetPortion()->SvLSize().Height();
                        nHeight = pTmp->GetPortion()->SvLSize().Width();
                        break;
                    case 1800:
                        aPos.Y() -= pTmp->GetPortion()->Height() -
                                    pTmp->GetPortion()->GetAscent();
                        aPos.X() -= pTmp->GetPortion()->Width();
                        nWidth = pTmp->GetPortion()->SvLSize().Width();
                        nHeight = pTmp->GetPortion()->SvLSize().Height();
                        break;
                    case 2700:
                        aPos.X() -= pTmp->GetPortion()->Height() -
                                    pTmp->GetPortion()->GetAscent();
                        nWidth = pTmp->GetPortion()->SvLSize().Height();
                        nHeight = pTmp->GetPortion()->SvLSize().Width();
                        break;
                    default:
                        aPos.Y() -= pTmp->GetPortion()->GetAscent();
                        nWidth = pTmp->GetPortion()->SvLSize().Width();
                        nHeight = pTmp->GetPortion()->SvLSize().Height();
                }

                Rectangle aRefresh( aPos, Size( nWidth, nHeight ) );
                aRefresh.Right() += ( aRefresh.Bottom()- aRefresh.Top() ) / 8;
                pTmp->GetRootFrame()
                    ->GetCurrShell()->InvalidateWindows( aRefresh );
            }
            else // Portions without a shell can be removed from the list
                it = m_List.erase(it);
        }
    }
    else // If the list is empty, the timer can be stopped
        aTimer.Stop();
}

void SwBlink::Insert( const Point& rPoint, const SwLinePortion* pPor,
                      const SwTextFrame *pTextFrame, sal_uInt16 nDir )
{
    std::unique_ptr<SwBlinkPortion> pBlinkPor(new SwBlinkPortion(pPor, nDir));

    SwBlinkSet::iterator it = m_List.find( pBlinkPor );
    if (it != m_List.end())
    {
        (*it)->SetPos( rPoint );
    }
    else
    {
        pBlinkPor->SetPos( rPoint );
        pBlinkPor->SetRootFrame( pTextFrame->getRootFrame() );
        m_List.insert(std::move(pBlinkPor));
        pTextFrame->SetBlinkPor();
        if( pPor->IsLayPortion() || pPor->IsParaPortion() )
            const_cast<SwLineLayout*>(static_cast<const SwLineLayout*>(pPor))->SetBlinking();

        if( !aTimer.IsActive() )
            aTimer.Start();
    }
}

void SwBlink::Replace( const SwLinePortion* pOld, const SwLinePortion* pNew )
{
    // setting direction to 0 because direction does not matter
    // for this operation
    std::unique_ptr<SwBlinkPortion> pBlinkPortion(new SwBlinkPortion(pOld, 0));
    SwBlinkSet::iterator it = m_List.find( pBlinkPortion );
    if (it != m_List.end())
    {
        std::unique_ptr<SwBlinkPortion> pTmp(new SwBlinkPortion(it->get(), pNew));
        m_List.erase( it );
        m_List.insert(std::move(pTmp));
    }
}

void SwBlink::Delete( const SwLinePortion* pPor )
{
    // setting direction to 0 because direction does not matter
    // for this operation
    std::unique_ptr<SwBlinkPortion> pBlinkPortion(new SwBlinkPortion(pPor, 0));
    m_List.erase( pBlinkPortion );
}

void SwBlink::FrameDelete( const SwRootFrame* pRoot )
{
    for (SwBlinkSet::iterator it = m_List.begin(); it != m_List.end(); )
    {
        if (pRoot == (*it)->GetRootFrame())
            m_List.erase( it++ );
        else
            ++it;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
