/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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

/* SwGrammarContact
    This class is responsible for the delayed display of grammar checks when a paragraph is edited
    It's a client of the paragraph the cursor points to.
    If the cursor position changes, updateCursorPosition has to be called
    If the grammar checker wants to set a grammar marker at a paragraph, he has to request
    the grammar list from this class. If the requested paragraph is not edited, it returns
    the normal grammar list. But if the paragraph is the active one, a proxy list will be returned and
    all changes are set in this proxy list. If the cursor leaves the paragraph the proxy list
    will replace the old list. If the grammar checker has completed the paragraph ('setChecked')
    then a timer is setup which replaces the old list as well.
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
    ~SwGrammarContact() { aTimer.Stop(); delete mpProxyList; }

    
    virtual void updateCursorPosition( const SwPosition& rNewPos );
    virtual SwGrammarMarkUp* getGrammarCheck( SwTxtNode& rTxtNode, bool bCreate );
    virtual void finishGrammarCheck( SwTxtNode& rTxtNode );
protected:
    
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);
};

SwGrammarContact::SwGrammarContact() : mpProxyList(0), mbFinished( false )
{
    aTimer.SetTimeout( 2000 );  
    aTimer.SetTimeoutHdl( LINK(this, SwGrammarContact, TimerRepaint) );
}

IMPL_LINK( SwGrammarContact, TimerRepaint, Timer *, pTimer )
{
    if( pTimer )
    {
        pTimer->Stop();
        if( GetRegisteredIn() )
        {   
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
    if( pTxtNode != GetRegisteredIn() ) 
    {
        aTimer.Stop();
        if( GetRegisteredIn() ) 
        {
            if( mpProxyList )
            {   
                getMyTxtNode()->SetGrammarCheck( mpProxyList, true );
                SwTxtFrm::repaintTextFrames( *getMyTxtNode() );
            }
            GetRegisteredInNonConst()->Remove( this ); 
            mpProxyList = 0;
        }
        if( pTxtNode )
            pTxtNode->Add( this ); 
    }
}

/* deliver a grammar check list for the given text node */
SwGrammarMarkUp* SwGrammarContact::getGrammarCheck( SwTxtNode& rTxtNode, bool bCreate )
{
    SwGrammarMarkUp *pRet = 0;
    if( GetRegisteredIn() == &rTxtNode ) 
    {   
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
        pRet = rTxtNode.GetGrammarCheck(); 
        if( bCreate && !pRet ) 
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
    {    
        aTimer.Stop();
        GetRegisteredInNonConst()->Remove( this );
        delete mpProxyList;
        mpProxyList = 0;
    }
}

void SwGrammarContact::finishGrammarCheck( SwTxtNode& rTxtNode )
{
    if( &rTxtNode != GetRegisteredIn() ) 
        SwTxtFrm::repaintTextFrames( rTxtNode ); 
    else
    {
        if( mpProxyList )
        {
            mbFinished = true;
            aTimer.Start(); 
        }
        else if( getMyTxtNode()->GetGrammarCheck() )
        {   
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
