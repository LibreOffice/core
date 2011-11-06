/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"




#include <com/sun/star/linguistic2/DictionaryListEventFlags.hpp>
#include <com/sun/star/linguistic2/XDictionaryList.hpp>
#include <com/sun/star/linguistic2/XLinguServiceManager.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>
#include <com/sun/star/linguistic2/XProofreadingIterator.hpp>
#include <com/sun/star/linguistic2/LinguServiceEventFlags.hpp>

#include <unotools/lingucfg.hxx>

#include <com/sun/star/uno/Reference.h>
#include <comphelper/processfactory.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <tools/shl.hxx>
#include "dlelstnr.hxx"
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <view.hxx>


using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::linguistic2::LinguServiceEventFlags;

#define A2OU(x) OUString::createFromAscii(x)

/* -----------------------------17.03.00 09:07--------------------------------

 ---------------------------------------------------------------------------*/
SwLinguServiceEventListener::SwLinguServiceEventListener()
{
    Reference< XMultiServiceFactory > xMgr( comphelper::getProcessServiceFactory() );
    if (xMgr.is())
    {
        try
        {
            OUString aSvcName( A2OU( "com.sun.star.frame.Desktop" ) );
            xDesktop = Reference< frame::XDesktop >(
                    xMgr->createInstance( aSvcName ), UNO_QUERY );
            if (xDesktop.is())
                xDesktop->addTerminateListener( this );

            aSvcName = A2OU( "com.sun.star.linguistic2.LinguServiceManager" );
            xLngSvcMgr = Reference< XLinguServiceManager >( xMgr->createInstance( aSvcName ), UNO_QUERY );
            if (xLngSvcMgr.is())
                xLngSvcMgr->addLinguServiceManagerListener( (XLinguServiceEventListener *) this );

            if (SvtLinguConfig().HasGrammarChecker())
            {
                aSvcName = A2OU( "com.sun.star.linguistic2.ProofreadingIterator" );
                xGCIterator = Reference< XProofreadingIterator >( xMgr->createInstance( aSvcName ), UNO_QUERY );
                Reference< XLinguServiceEventBroadcaster > xBC( xGCIterator, UNO_QUERY );
                if (xBC.is())
                    xBC->addLinguServiceEventListener( (XLinguServiceEventListener *) this );
            }
        }
        catch (uno::Exception &)
        {
            DBG_ASSERT(0, "exception caught in SwLinguServiceEventListener c-tor" );
        }
    }
}
/* -----------------------------17.03.00 09:07--------------------------------

 ---------------------------------------------------------------------------*/
SwLinguServiceEventListener::~SwLinguServiceEventListener()
{
}

/* -----------------------------17.03.00 09:06--------------------------------

 ---------------------------------------------------------------------------*/

void SwLinguServiceEventListener::processDictionaryListEvent(
            const DictionaryListEvent& rDicListEvent)
        throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    sal_Int16 nEvt = rDicListEvent.nCondensedEvent;

    sal_Int16 nSpellWrongFlags =
            DictionaryListEventFlags::ADD_POS_ENTRY     |
            DictionaryListEventFlags::DEL_NEG_ENTRY     |
            DictionaryListEventFlags::ACTIVATE_POS_DIC  |
            DictionaryListEventFlags::DEACTIVATE_NEG_DIC;
    sal_Bool bIsSpellWrong  =  0 != (nEvt & nSpellWrongFlags);
    sal_Int16 nSpellAllFlags =
            DictionaryListEventFlags::ADD_NEG_ENTRY     |
            DictionaryListEventFlags::DEL_POS_ENTRY     |
            DictionaryListEventFlags::ACTIVATE_NEG_DIC  |
            DictionaryListEventFlags::DEACTIVATE_POS_DIC;
    sal_Bool bIsSpellAll    =  0 != (nEvt & nSpellAllFlags);

    if (bIsSpellWrong || bIsSpellAll)
        SW_MOD()->CheckSpellChanges( sal_False, bIsSpellWrong, bIsSpellAll, sal_False );
}


void SAL_CALL SwLinguServiceEventListener::processLinguServiceEvent(
            const LinguServiceEvent& rLngSvcEvent )
        throw(RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    sal_Bool bIsSpellWrong = 0 != (rLngSvcEvent.nEvent & SPELL_WRONG_WORDS_AGAIN);
    sal_Bool bIsSpellAll   = 0 != (rLngSvcEvent.nEvent & SPELL_CORRECT_WORDS_AGAIN);
    if (0 != (rLngSvcEvent.nEvent & PROOFREAD_AGAIN))
        bIsSpellWrong = bIsSpellAll = sal_True;     // have all spelling and grammar checked...
    if (bIsSpellWrong || bIsSpellAll)
    {
        SW_MOD()->CheckSpellChanges( sal_False, bIsSpellWrong, bIsSpellAll, sal_False );
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
        throw(RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (xLngSvcMgr.is()  &&  rEventObj.Source == xLngSvcMgr)
        xLngSvcMgr = 0;
    if (xLngSvcMgr.is()  &&  rEventObj.Source == xGCIterator)
        xGCIterator = 0;
}


void SAL_CALL SwLinguServiceEventListener::queryTermination(
            const EventObject& /*rEventObj*/ )
        throw(TerminationVetoException, RuntimeException)
{
    //vos::OGuard aGuard(Application::GetSolarMutex());
}


void SAL_CALL SwLinguServiceEventListener::notifyTermination(
            const EventObject& rEventObj )
        throw(RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (xDesktop.is()  &&  rEventObj.Source == xDesktop)
    {
        if (xLngSvcMgr.is())
            xLngSvcMgr = 0;
        if (xGCIterator.is())
            xGCIterator = 0;
        xDesktop = NULL;
    }
}

