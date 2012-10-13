/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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


#include "eventuno.hxx"
#include "miscuno.hxx"
#include "docsh.hxx"
#include "sheetevents.hxx"
#include "unonames.hxx"
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;

//------------------------------------------------------------------------

SC_SIMPLE_SERVICE_INFO( ScSheetEventsObj, "ScSheetEventsObj", "com.sun.star.document.Events" )

//------------------------------------------------------------------------

ScSheetEventsObj::ScSheetEventsObj(ScDocShell* pDocSh, SCTAB nT) :
    mpDocShell( pDocSh ),
    mnTab( nT )
{
    mpDocShell->GetDocument()->AddUnoObject(*this);
}

ScSheetEventsObj::~ScSheetEventsObj()
{
    if (mpDocShell)
        mpDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScSheetEventsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //! reference update
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        mpDocShell = NULL;
    }
}

static sal_Int32 lcl_GetEventFromName( const rtl::OUString& aName )
{
    for (sal_Int32 nEvent=0; nEvent<SC_SHEETEVENT_COUNT; ++nEvent)
        if ( aName == ScSheetEvents::GetEventName(nEvent) )
            return nEvent;

    return -1;      // not found
}

// XNameReplace

void SAL_CALL ScSheetEventsObj::replaceByName( const rtl::OUString& aName, const uno::Any& aElement )
    throw(lang::IllegalArgumentException, container::NoSuchElementException,
          lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!mpDocShell)
        throw uno::RuntimeException();

    sal_Int32 nEvent = lcl_GetEventFromName(aName);
    if (nEvent < 0)
        throw container::NoSuchElementException();

    ScSheetEvents aNewEvents;
    const ScSheetEvents* pOldEvents = mpDocShell->GetDocument()->GetSheetEvents(mnTab);
    if (pOldEvents)
        aNewEvents = *pOldEvents;

    rtl::OUString aScript;
    if ( aElement.hasValue() )      // empty Any -> reset event
    {
        uno::Sequence<beans::PropertyValue> aPropSeq;
        if ( aElement >>= aPropSeq )
        {
            sal_Int32 nPropCount = aPropSeq.getLength();
            for (sal_Int32 nPos=0; nPos<nPropCount; ++nPos)
            {
                const beans::PropertyValue& rProp = aPropSeq[nPos];
                if ( rProp.Name.compareToAscii( SC_UNO_EVENTTYPE ) == 0 )
                {
                    rtl::OUString aEventType;
                    if ( rProp.Value >>= aEventType )
                    {
                        // only "Script" is supported
                        if ( aEventType.compareToAscii( SC_UNO_SCRIPT ) != 0 )
                            throw lang::IllegalArgumentException();
                    }
                }
                else if ( rProp.Name.compareToAscii( SC_UNO_SCRIPT ) == 0 )
                    rProp.Value >>= aScript;
            }
        }
    }
    if (!aScript.isEmpty())
        aNewEvents.SetScript( nEvent, &aScript );
    else
        aNewEvents.SetScript( nEvent, NULL );       // reset

    mpDocShell->GetDocument()->SetSheetEvents( mnTab, &aNewEvents );
    mpDocShell->SetDocumentModified();
}

// XNameAccess

uno::Any SAL_CALL ScSheetEventsObj::getByName( const rtl::OUString& aName )
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_Int32 nEvent = lcl_GetEventFromName(aName);
    if (nEvent < 0)
        throw container::NoSuchElementException();

    const rtl::OUString* pScript = NULL;
    if (mpDocShell)
    {
        const ScSheetEvents* pEvents = mpDocShell->GetDocument()->GetSheetEvents(mnTab);
        if (pEvents)
            pScript = pEvents->GetScript(nEvent);
    }

    uno::Any aRet;
    if (pScript)
    {
        uno::Sequence<beans::PropertyValue> aPropSeq( 2 );
        aPropSeq[0] = beans::PropertyValue(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("EventType")), -1,
                        uno::makeAny( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Script")) ), beans::PropertyState_DIRECT_VALUE );
        aPropSeq[1] = beans::PropertyValue(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Script")), -1,
                        uno::makeAny( *pScript ), beans::PropertyState_DIRECT_VALUE );
        aRet <<= aPropSeq;
    }
    // empty Any if nothing was set
    return aRet;
}

uno::Sequence<rtl::OUString> SAL_CALL ScSheetEventsObj::getElementNames() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Sequence<rtl::OUString> aNames(SC_SHEETEVENT_COUNT);
    for (sal_Int32 nEvent=0; nEvent<SC_SHEETEVENT_COUNT; ++nEvent)
        aNames[nEvent] = ScSheetEvents::GetEventName(nEvent);
    return aNames;
}

sal_Bool SAL_CALL ScSheetEventsObj::hasByName( const ::rtl::OUString& aName ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_Int32 nEvent = lcl_GetEventFromName(aName);
    return (nEvent >= 0);
}

// XElementAccess

uno::Type SAL_CALL ScSheetEventsObj::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCppuType((uno::Sequence<beans::PropertyValue>*)0);
}

sal_Bool SAL_CALL ScSheetEventsObj::hasElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (mpDocShell)
        return sal_True;
    return false;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
