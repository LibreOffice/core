/*************************************************************************
 *
 *  $RCSfile: dlelstnr.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: tl $ $Date: 2000-11-02 13:35:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop


#ifndef _COM_SUN_STAR_LINGUISTIC2_DICTIONARYLISTEVENTFLAGS_HPP_
#include <com/sun/star/linguistic2/DictionaryListEventFlags.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC_XDICTIONARYLIST_HPP_
#include <com/sun/star/linguistic2/XDictionaryList.hpp>
#endif

#include <com/sun/star/uno/Reference.h>

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _DLELSTNR_HXX_
#include "dlelstnr.hxx"
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;

/* -----------------------------17.03.00 09:07--------------------------------

 ---------------------------------------------------------------------------*/
SwDicListEvtListener::SwDicListEvtListener(
            const Reference< XDictionaryList >  &rxDicList )
{
    xDicList = rxDicList;
    if (xDicList.is())
    {
        xDicList->addDictionaryListEventListener( this, sal_False );
    }

    Reference< XMultiServiceFactory > xMgr( utl::getProcessServiceFactory() );
    if (xMgr.is())
    {
        OUString aSvcName( OUString::createFromAscii(
                "com.sun.star.frame.Desktop" ) );
        xDesktop = Reference< frame::XDesktop >(
                xMgr->createInstance( aSvcName ), UNO_QUERY );
        if (xDesktop.is())
            xDesktop->addTerminateListener( this );
    }
}
/* -----------------------------17.03.00 09:07--------------------------------

 ---------------------------------------------------------------------------*/
SwDicListEvtListener::~SwDicListEvtListener()
{
}

/* -----------------------------17.03.00 09:06--------------------------------

 ---------------------------------------------------------------------------*/

void SwDicListEvtListener::processDictionaryListEvent(
            const DictionaryListEvent& rDicListEvent)
        throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    sal_Int16 nEvt = rDicListEvent.nCondensedEvent;

    sal_Bool bIsSpellWrong  =  ( nEvt & DictionaryListEventFlags::ADD_POS_ENTRY )
                            || ( nEvt & DictionaryListEventFlags::DEL_NEG_ENTRY )
                            || ( nEvt & DictionaryListEventFlags::ACTIVATE_POS_DIC )
                            || ( nEvt & DictionaryListEventFlags::DEACTIVATE_NEG_DIC );
    sal_Bool bIsSpellAll    =  ( nEvt & DictionaryListEventFlags::ADD_NEG_ENTRY )
                            || ( nEvt & DictionaryListEventFlags::DEL_POS_ENTRY )
                            || ( nEvt & DictionaryListEventFlags::ACTIVATE_NEG_DIC )
                            || ( nEvt & DictionaryListEventFlags::DEACTIVATE_POS_DIC );
    SW_MOD()->CheckSpellChanges( sal_False, bIsSpellWrong, bIsSpellAll );
}


void SAL_CALL SwDicListEvtListener::disposing(
            const EventObject& rEventObj )
        throw(RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (xDicList.is()  &&  rEventObj.Source == xDicList)
    {
        xDicList = 0;
    }
}


void SAL_CALL SwDicListEvtListener::queryTermination(
            const EventObject& rEventObj )
        throw(TerminationVetoException, RuntimeException)
{
    //vos::OGuard aGuard(Application::GetSolarMutex());
}


void SAL_CALL SwDicListEvtListener::notifyTermination(
            const EventObject& rEventObj )
        throw(RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (xDesktop.is()  &&  rEventObj.Source == xDesktop)
    {
        if (xDicList.is())
        {
            xDicList->removeDictionaryListEventListener( this );
            xDicList = NULL;
        }
        xDesktop = NULL;
    }
}

