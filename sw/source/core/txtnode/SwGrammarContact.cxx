/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <vcl/timer.hxx>
#include <hints.hxx>
#include <IGrammarContact.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <SwGrammarMarkUp.hxx>
#include <txtfrm.hxx>
#include <rootfrm.hxx>
#include <viewsh.hxx>

extern void repaintTextFrames( SwModify& rModify );


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
    SwTxtNode* getMyTxtNode() { return (SwTxtNode*)pRegisteredIn; }
      DECL_LINK( TimerRepaint, Timer * );

public:
    SwGrammarContact();
    ~SwGrammarContact() { aTimer.Stop(); delete mpProxyList; }

    // (pure) virtual functions of IGrammarContact
    virtual void updateCursorPosition( const SwPosition& rNewPos );
    virtual SwGrammarMarkUp* getGrammarCheck( SwTxtNode& rTxtNode, bool bCreate );
    virtual void finishGrammarCheck( SwTxtNode& rTxtNode );

    // virtual function of SwClient
    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);
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
        if( pRegisteredIn )
        {   //Replace the old wrong list by the proxy list and repaint all frames
            getMyTxtNode()->SetGrammarCheck( mpProxyList, true );
            mpProxyList = 0;
            repaintTextFrames( *pRegisteredIn );
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
                repaintTextFrames( *pRegisteredIn );
            }
            pRegisteredIn->Remove( this ); // good bye old paragraph
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
                    mpProxyList->SetInvalid( 0, STRING_LEN );
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
            pRet->SetInvalid( 0, STRING_LEN );
            rTxtNode.SetGrammarCheck( pRet );
            rTxtNode.SetGrammarCheckDirty( true );
        }
    }
    return pRet;
}

void SwGrammarContact::Modify( SfxPoolItem *pOld, SfxPoolItem * )
{
    if( !pOld || pOld->Which() != RES_OBJECTDYING )
        return;

    SwPtrMsgPoolItem *pDead = (SwPtrMsgPoolItem *)pOld;
    if( pDead->pObject == pRegisteredIn )
    {    // if my current paragraph dies, I throw the proxy list away
        aTimer.Stop();
        pRegisteredIn->Remove( this );
        delete mpProxyList;
        mpProxyList = 0;
    }
}

void SwGrammarContact::finishGrammarCheck( SwTxtNode& rTxtNode )
{
    if( &rTxtNode != pRegisteredIn ) // not my paragraph
        repaintTextFrames( rTxtNode ); // can be repainted directly
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
            repaintTextFrames( *pRegisteredIn );
        }
    }
}

IGrammarContact* createGrammarContact()
{
    return new SwGrammarContact();
}

/* repaint all text frames of the given text node */
void repaintTextFrames( SwModify& rModify )
{
    SwClientIter aIter( rModify );
    for( const SwTxtFrm *pFrm = (const SwTxtFrm*)aIter.First( TYPE(SwTxtFrm) );
         pFrm; pFrm = (const SwTxtFrm*)aIter.Next() )
    {
        SwRect aRec( pFrm->PaintArea() );
        const SwRootFrm *pRootFrm = pFrm->FindRootFrm();
        ViewShell *pCurShell = pRootFrm ? pRootFrm->GetCurrShell() : NULL;
        if( pCurShell )
            pCurShell->InvalidateWindows( aRec );
    }
}

void finishGrammarCheck( SwTxtNode& rTxtNode )
{
    IGrammarContact* pGrammarContact = getGrammarContact( rTxtNode );
    if( pGrammarContact )
        pGrammarContact->finishGrammarCheck( rTxtNode );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
