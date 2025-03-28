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

#include <GrammarContact.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <txtfrm.hxx>

namespace sw
{

GrammarContact::GrammarContact()
  : m_aTimer( "sw::SwGrammarContact TimerRepaint" ),
    m_isFinished( false ),
    m_pTextNode(nullptr)
{
    m_aTimer.SetTimeout( 2000 );  // Repaint of grammar check after 'setChecked'
    m_aTimer.SetInvokeHandler( LINK(this, GrammarContact, TimerRepaint) );
}

void GrammarContact::CheckBroadcaster()
{
    if (HasBroadcaster())
        return;
    m_pTextNode = nullptr;
    m_pProxyList.reset();
}

IMPL_LINK( GrammarContact, TimerRepaint, Timer *, pTimer, void )
{
    CheckBroadcaster();
    if( pTimer )
    {
        pTimer->Stop();
        if( m_pTextNode )
        {   //Replace the old wrong list by the proxy list and repaint all frames
            m_pTextNode->SetGrammarCheck( std::move(m_pProxyList) );
            SwTextFrame::repaintTextFrames( *m_pTextNode );
        }
    }
}

/* I'm always a client of the current paragraph */
void GrammarContact::updateCursorPosition( const SwPosition& rNewPos )
{
    CheckBroadcaster();
    SwTextNode* pTextNode = rNewPos.GetNode().GetTextNode();
    if( pTextNode == m_pTextNode ) // paragraph has been changed
        return;

    m_aTimer.Stop();
    if( m_pTextNode ) // My last paragraph has been left
    {
        if( m_pProxyList )
        {   // replace old list by the proxy list and repaint
            m_pTextNode->SetGrammarCheck( std::move(m_pProxyList) );
            SwTextFrame::repaintTextFrames( *m_pTextNode );
        }
        EndListeningAll();
    }
    if( pTextNode )
    {
        m_pTextNode = pTextNode;
        EndListeningAll();
        StartListening(pTextNode->GetNotifier()); // welcome new paragraph
    }
}

/* deliver a grammar check list for the given text node */
SwGrammarMarkUp* GrammarContact::getGrammarCheck( SwTextNode& rTextNode, bool bCreate )
{
    SwGrammarMarkUp *pRet = nullptr;
    CheckBroadcaster();
    if( m_pTextNode == &rTextNode ) // hey, that's my current paragraph!
    {   // so you will get a proxy list...
        if( bCreate )
        {
            if( m_isFinished )
            {
                m_pProxyList.reset();
            }
            if( !m_pProxyList )
            {
                if( rTextNode.GetGrammarCheck() )
                    m_pProxyList.reset( static_cast<SwGrammarMarkUp*>(rTextNode.GetGrammarCheck()->Clone()) );
                else
                {
                    m_pProxyList.reset( new SwGrammarMarkUp() );
                    m_pProxyList->SetInvalid( 0, COMPLETE_STRING );
                }
            }
            m_isFinished = false;
        }
        pRet = m_pProxyList.get();
    }
    else
    {
        pRet = rTextNode.GetGrammarCheck(); // do you have already a list?
        if( bCreate && !pRet ) // do you want to create a list?
        {
            pRet = new SwGrammarMarkUp();
            pRet->SetInvalid( 0, COMPLETE_STRING );
            rTextNode.SetGrammarCheck( std::unique_ptr<SwGrammarMarkUp>(pRet) );
            rTextNode.SetGrammarCheckDirty( true );
        }
    }
    return pRet;
}

void GrammarContact::finishGrammarCheck( const SwTextNode& rTextNode )
{
    CheckBroadcaster();
    if( &rTextNode != m_pTextNode ) // not my paragraph
        SwTextFrame::repaintTextFrames( rTextNode ); // can be repainted directly
    else
    {
        if( m_pProxyList )
        {
            m_isFinished = true;
            m_aTimer.Start(); // will replace old list and repaint with delay
        }
        else if( m_pTextNode->GetGrammarCheck() )
        {   // all grammar problems seems to be gone, no delay needed
            m_pTextNode->ClearGrammarCheck();
            SwTextFrame::repaintTextFrames( *m_pTextNode );
        }
    }
}

sw::GrammarContact* getGrammarContactFor(const SwTextNode& rTextNode)
{
    const SwDoc& rDoc = rTextNode.GetDoc();
    if (rDoc.IsInDtor())
        return nullptr;
    return rDoc.getGrammarContact().get();
}

void finishGrammarCheckFor(const SwTextNode& rTextNode)
{
    sw::GrammarContact* pGrammarContact = getGrammarContactFor(rTextNode);
    if (pGrammarContact)
    {
        pGrammarContact->finishGrammarCheck(rTextNode);
    }
}

} // end sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
