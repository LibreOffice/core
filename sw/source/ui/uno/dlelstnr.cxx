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


#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/linguistic2/DictionaryListEventFlags.hpp>
#include <com/sun/star/linguistic2/XDictionaryList.hpp>
#include <com/sun/star/linguistic2/LinguServiceManager.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>
#include <com/sun/star/linguistic2/ProofreadingIterator.hpp>
#include <com/sun/star/linguistic2/LinguServiceEventFlags.hpp>

#include <unotools/lingucfg.hxx>

#include <com/sun/star/uno/Reference.h>
#include <comphelper/processfactory.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <tools/shl.hxx>
#include "dlelstnr.hxx"
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <view.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::linguistic2::LinguServiceEventFlags;

SwLinguServiceEventListener::SwLinguServiceEventListener()
{
    Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );
    try
    {
        xDesktop = frame::Desktop::create(xContext);
        xDesktop->addTerminateListener( this );

        xLngSvcMgr = LinguServiceManager::create(xContext);
        xLngSvcMgr->addLinguServiceManagerListener( (XLinguServiceEventListener *) this );

        if (SvtLinguConfig().HasGrammarChecker())
        {
            xGCIterator = ProofreadingIterator::create(xContext);
            Reference< XLinguServiceEventBroadcaster > xBC( xGCIterator, UNO_QUERY );
            if (xBC.is())
                xBC->addLinguServiceEventListener( (XLinguServiceEventListener *) this );
        }
    }
    catch (const uno::Exception&)
    {
        OSL_FAIL("exception caught in SwLinguServiceEventListener c-tor" );
    }
}

SwLinguServiceEventListener::~SwLinguServiceEventListener()
{
}

void SwLinguServiceEventListener::processDictionaryListEvent(
            const DictionaryListEvent& rDicListEvent)
        throw( RuntimeException )
{
    SolarMutexGuard aGuard;

    sal_Int16 nEvt = rDicListEvent.nCondensedEvent;

    sal_Int16 nSpellWrongFlags =
            DictionaryListEventFlags::ADD_POS_ENTRY     |
            DictionaryListEventFlags::DEL_NEG_ENTRY     |
            DictionaryListEventFlags::ACTIVATE_POS_DIC  |
            DictionaryListEventFlags::DEACTIVATE_NEG_DIC;
    bool bIsSpellWrong  =  0 != (nEvt & nSpellWrongFlags);
    sal_Int16 nSpellAllFlags =
            DictionaryListEventFlags::ADD_NEG_ENTRY     |
            DictionaryListEventFlags::DEL_POS_ENTRY     |
            DictionaryListEventFlags::ACTIVATE_NEG_DIC  |
            DictionaryListEventFlags::DEACTIVATE_POS_DIC;
    bool bIsSpellAll    =  0 != (nEvt & nSpellAllFlags);

    if (bIsSpellWrong || bIsSpellAll)
        SW_MOD()->CheckSpellChanges( false, bIsSpellWrong, bIsSpellAll, false );
}

void SAL_CALL SwLinguServiceEventListener::processLinguServiceEvent(
            const LinguServiceEvent& rLngSvcEvent )
        throw(RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    bool bIsSpellWrong = 0 != (rLngSvcEvent.nEvent & SPELL_WRONG_WORDS_AGAIN);
    bool bIsSpellAll   = 0 != (rLngSvcEvent.nEvent & SPELL_CORRECT_WORDS_AGAIN);
    if (0 != (rLngSvcEvent.nEvent & PROOFREAD_AGAIN))
        bIsSpellWrong = bIsSpellAll = true;     // have all spelling and grammar checked...
    if (bIsSpellWrong || bIsSpellAll)
    {
        SW_MOD()->CheckSpellChanges( false, bIsSpellWrong, bIsSpellAll, false );
    }
    if (rLngSvcEvent.nEvent & HYPHENATE_AGAIN)
    {
        SwView *pSwView = SW_MOD()->GetFirstView();

        //!! since this function may be called within the ctor of
        //!! SwView (during formatting) where the WrtShell is not yet
        //!! created, we have to check for the WrtShellPtr to see
        //!! if it is already availbale
        while (pSwView && pSwView->GetWrtShellPtr())
        {
            pSwView->GetWrtShell().ChgHyphenation();
            pSwView = SW_MOD()->GetNextView( pSwView );
        }
    }
}

void SAL_CALL SwLinguServiceEventListener::disposing(
            const EventObject& rEventObj )
        throw(RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if (xLngSvcMgr.is() && rEventObj.Source == xLngSvcMgr)
        xLngSvcMgr = 0;
    if (xLngSvcMgr.is() && rEventObj.Source == xGCIterator)
        xGCIterator = 0;
}

void SAL_CALL SwLinguServiceEventListener::queryTermination(
            const EventObject& /*rEventObj*/ )
        throw(TerminationVetoException, RuntimeException, std::exception)
{
}

void SAL_CALL SwLinguServiceEventListener::notifyTermination(
            const EventObject& rEventObj )
        throw(RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if (xDesktop.is()  &&  rEventObj.Source == xDesktop)
    {
        if (xLngSvcMgr.is())
            xLngSvcMgr = 0;
        if (xGCIterator.is())
            xGCIterator = 0;
        xDesktop = NULL;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
