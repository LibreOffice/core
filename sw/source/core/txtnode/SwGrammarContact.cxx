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

#include <vcl/timer.hxx>
#include <IGrammarContact.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <SwGrammarMarkUp.hxx>
#include <txtfrm.hxx>
#include <svl/listener.hxx>

namespace {

/*
 * This class is responsible for the delayed display of grammar checks when a paragraph is edited
 * It's a client of the paragraph the cursor points to.
 * If the cursor position changes, updateCursorPosition has to be called
 * If the grammar checker wants to set a grammar marker at a paragraph, he has to request
 * the grammar list from this class. If the requested paragraph is not edited, it returns
 * the normal grammar list. But if the paragraph is the active one, a proxy list will be returned and
 * all changes are set in this proxy list. If the cursor leaves the paragraph the proxy list
 * will replace the old list. If the grammar checker has completed the paragraph ('setChecked')
 * then a timer is setup which replaces the old list as well.
 */
class SwGrammarContact : public IGrammarContact, public SvtListener
{
    Timer m_aTimer;
    std::unique_ptr<SwGrammarMarkUp> m_pProxyList;
    bool m_isFinished;
    SwTextNode* m_pTextNode;
    DECL_LINK( TimerRepaint, Timer *, void );

public:
    SwGrammarContact();
    virtual ~SwGrammarContact() override { m_aTimer.Stop(); }

    // (pure) virtual functions of IGrammarContact
    virtual void updateCursorPosition( const SwPosition& rNewPos ) override;
    virtual SwGrammarMarkUp* getGrammarCheck( SwTextNode& rTextNode, bool bCreate ) override;
    virtual void finishGrammarCheck( SwTextNode& rTextNode ) override;
    virtual void Notify( const SfxHint& rHint) override;
};

}

SwGrammarContact::SwGrammarContact() : m_isFinished( false ), m_pTextNode(nullptr)
{
    m_aTimer.SetTimeout( 2000 );  // Repaint of grammar check after 'setChecked'
    m_aTimer.SetInvokeHandler( LINK(this, SwGrammarContact, TimerRepaint) );
    m_aTimer.SetDebugName( "sw::SwGrammarContact TimerRepaint" );
}

IMPL_LINK( SwGrammarContact, TimerRepaint, Timer *, pTimer, void )
{
    if( pTimer )
    {
        pTimer->Stop();
        if( m_pTextNode )
        {   //Replace the old wrong list by the proxy list and repaint all frames
            m_pTextNode->SetGrammarCheck( m_pProxyList.release() );
            SwTextFrame::repaintTextFrames( *m_pTextNode );
        }
    }
}

/* I'm always a client of the current paragraph */
void SwGrammarContact::updateCursorPosition( const SwPosition& rNewPos )
{
    SwTextNode* pTextNode = rNewPos.nNode.GetNode().GetTextNode();
    if( pTextNode == m_pTextNode ) // paragraph has been changed
        return;

    m_aTimer.Stop();
    if( m_pTextNode ) // My last paragraph has been left
    {
        if( m_pProxyList )
        {   // replace old list by the proxy list and repaint
            m_pTextNode->SetGrammarCheck( m_pProxyList.release() );
            SwTextFrame::repaintTextFrames( *m_pTextNode );
        }
        EndListeningAll();
    }
    if( pTextNode )
    {
        m_pTextNode = pTextNode;
        StartListening(pTextNode->GetNotifier()); // welcome new paragraph
    }
}

/* deliver a grammar check list for the given text node */
SwGrammarMarkUp* SwGrammarContact::getGrammarCheck( SwTextNode& rTextNode, bool bCreate )
{
    SwGrammarMarkUp *pRet = nullptr;
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
            rTextNode.SetGrammarCheck( pRet );
            rTextNode.SetGrammarCheckDirty( true );
        }
    }
    return pRet;
}

void SwGrammarContact::Notify(const SfxHint& rHint)
{
    auto pLegacy = dynamic_cast<const sw::LegacyModifyHint*>(&rHint);
    if(!pLegacy || pLegacy->GetWhich() != RES_OBJECTDYING)
        return;
    m_aTimer.Stop();
    EndListeningAll();
    m_pTextNode = nullptr;
    m_pProxyList.reset();
}

void SwGrammarContact::finishGrammarCheck( SwTextNode& rTextNode )
{
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
            m_pTextNode->SetGrammarCheck( nullptr );
            SwTextFrame::repaintTextFrames( *m_pTextNode );
        }
    }
}

IGrammarContact* createGrammarContact()
{
    return new SwGrammarContact();
}

void finishGrammarCheck( SwTextNode& rTextNode )
{
    IGrammarContact* pGrammarContact = getGrammarContact( rTextNode );
    if( pGrammarContact )
        pGrammarContact->finishGrammarCheck( rTextNode );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
