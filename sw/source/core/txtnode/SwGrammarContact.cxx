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
#include <hints.hxx>
#include <IGrammarContact.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <SwGrammarMarkUp.hxx>
#include <txtfrm.hxx>
#include <rootfrm.hxx>
#include <viewsh.hxx>

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
class SwGrammarContact : public IGrammarContact, public SwClient
{
    Timer aTimer;
    SwGrammarMarkUp *mpProxyList;
    bool mbFinished;
    SwTxtNode* getMyTxtNode() { return (SwTxtNode*)GetRegisteredIn(); }
      DECL_LINK( TimerRepaint, Timer * );

public:
    SwGrammarContact();
    virtual ~SwGrammarContact() { aTimer.Stop(); delete mpProxyList; }

    // (pure) virtual functions of IGrammarContact
    virtual void updateCursorPosition( const SwPosition& rNewPos ) SAL_OVERRIDE;
    virtual SwGrammarMarkUp* getGrammarCheck( SwTxtNode& rTxtNode, bool bCreate ) SAL_OVERRIDE;
    virtual void finishGrammarCheck( SwTxtNode& rTxtNode ) SAL_OVERRIDE;
protected:
    // virtual function of SwClient
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) SAL_OVERRIDE;
};

SwGrammarContact::SwGrammarContact() : mpProxyList(0), mbFinished( false )
{
    aTimer.SetTimeout( 2000 );  // Repaint of grammar check after 'setChecked'
    aTimer.SetTimeoutHdl( LINK(this, SwGrammarContact, TimerRepaint) );
}

IMPL_LINK( SwGrammarContact, TimerRepaint, Timer *, pTimer )
{
    if( pTimer )
    {
        pTimer->Stop();
        if( GetRegisteredIn() )
        {   //Replace the old wrong list by the proxy list and repaint all frames
            getMyTxtNode()->SetGrammarCheck( mpProxyList, true );
            mpProxyList = 0;
            SwTxtFrm::repaintTextFrames( *getMyTxtNode() );
        }
    }
    return 0;
}

/* I'm always a client of the current paragraph */
void SwGrammarContact::updateCursorPosition( const SwPosition& rNewPos )
{
    SwTxtNode* pTxtNode = rNewPos.nNode.GetNode().GetTxtNode();
    if( pTxtNode != GetRegisteredIn() ) // paragraph has been changed
    {
        aTimer.Stop();
        if( GetRegisteredIn() ) // My last paragraph has been left
        {
            if( mpProxyList )
            {   // replace old list by the proxy list and repaint
                getMyTxtNode()->SetGrammarCheck( mpProxyList, true );
                SwTxtFrm::repaintTextFrames( *getMyTxtNode() );
            }
            GetRegisteredInNonConst()->Remove( this ); // good bye old paragraph
            mpProxyList = 0;
        }
        if( pTxtNode )
            pTxtNode->Add( this ); // welcome new paragraph
    }
}

/* deliver a grammar check list for the given text node */
SwGrammarMarkUp* SwGrammarContact::getGrammarCheck( SwTxtNode& rTxtNode, bool bCreate )
{
    SwGrammarMarkUp *pRet = 0;
    if( GetRegisteredIn() == &rTxtNode ) // hey, that's my current paragraph!
    {   // so you will get a proxy list...
        if( bCreate )
        {
            if( mbFinished )
            {
                delete mpProxyList;
                mpProxyList = 0;
            }
            if( !mpProxyList )
            {
                if( rTxtNode.GetGrammarCheck() )
                    mpProxyList = (SwGrammarMarkUp*)rTxtNode.GetGrammarCheck()->Clone();
                else
                {
                    mpProxyList = new SwGrammarMarkUp();
                    mpProxyList->SetInvalid( 0, COMPLETE_STRING );
                }
            }
           mbFinished = false;
        }
        pRet = mpProxyList;
    }
    else
    {
        pRet = rTxtNode.GetGrammarCheck(); // do you have already a list?
        if( bCreate && !pRet ) // do you want to create a list?
        {
            pRet = new SwGrammarMarkUp();
            pRet->SetInvalid( 0, COMPLETE_STRING );
            rTxtNode.SetGrammarCheck( pRet );
            rTxtNode.SetGrammarCheckDirty( true );
        }
    }
    return pRet;
}

void SwGrammarContact::Modify( const SfxPoolItem* pOld, const SfxPoolItem * )
{
    if( !pOld || pOld->Which() != RES_OBJECTDYING )
        return;

    SwPtrMsgPoolItem *pDead = (SwPtrMsgPoolItem *)pOld;
    if( pDead->pObject == GetRegisteredIn() )
    {    // if my current paragraph dies, I throw the proxy list away
        aTimer.Stop();
        GetRegisteredInNonConst()->Remove( this );
        delete mpProxyList;
        mpProxyList = 0;
    }
}

void SwGrammarContact::finishGrammarCheck( SwTxtNode& rTxtNode )
{
    if( &rTxtNode != GetRegisteredIn() ) // not my paragraph
        SwTxtFrm::repaintTextFrames( rTxtNode ); // can be repainted directly
    else
    {
        if( mpProxyList )
        {
            mbFinished = true;
            aTimer.Start(); // will replace old list and repaint with delay
        }
        else if( getMyTxtNode()->GetGrammarCheck() )
        {   // all grammar problems seems to be gone, no delay needed
            getMyTxtNode()->SetGrammarCheck( 0, true );
            SwTxtFrm::repaintTextFrames( *getMyTxtNode() );
        }
    }
}

IGrammarContact* createGrammarContact()
{
    return new SwGrammarContact();
}

void finishGrammarCheck( SwTxtNode& rTxtNode )
{
    IGrammarContact* pGrammarContact = getGrammarContact( rTxtNode );
    if( pGrammarContact )
        pGrammarContact->finishGrammarCheck( rTxtNode );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
