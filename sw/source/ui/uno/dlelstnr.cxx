/*************************************************************************
 *
 *  $RCSfile: dlelstnr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:49 $
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


#ifndef _COM_SUN_STAR_LINGUISTIC_DICTIONARYLISTEVENTFLAGS_HPP_
#include <com/sun/star/linguistic/DictionaryListEventFlags.hpp>
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

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic;

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
}
/* -----------------------------17.03.00 09:07--------------------------------

 ---------------------------------------------------------------------------*/
SwDicListEvtListener::~SwDicListEvtListener()
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());

    if (xDicList.is())
    {
        xDicList->removeDictionaryListEventListener( this );
    }
}

/* -----------------------------17.03.00 09:06--------------------------------

 ---------------------------------------------------------------------------*/
void SwDicListEvtListener::processDictionaryListEvent(
            const DictionaryListEvent& aDicListEvent)
        throw( ::com::sun::star::uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    sal_Int16 nEvt = aDicListEvent.nCondensedEvent;

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

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.6  2000/09/18 16:06:15  willem.vandorp
    OpenOffice header added.

    Revision 1.5  2000/05/11 12:51:32  tl
    if[n]def ONE_LINGU entfernt, namespace's verwendet

    Revision 1.4  2000/03/21 15:39:43  os
    UNOIII

    Revision 1.3  2000/02/11 15:00:05  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.2  2000/01/11 19:38:42  tl
    #70735# evaluate DictionaryListEvents

    Revision 1.1  2000/01/11 10:49:11  tl
    #70735# initial revision


------------------------------------------------------------------------*/

