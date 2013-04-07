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

static sal_Int32 lcl_GetEventFromName( const OUString& aName )
{
    for (sal_Int32 nEvent=0; nEvent<SC_SHEETEVENT_COUNT; ++nEvent)
        if ( aName == ScSheetEvents::GetEventName(nEvent) )
            return nEvent;

    return -1;      // not found
}

// XNameReplace

void SAL_CALL ScSheetEventsObj::replaceByName( const OUString& aName, const uno::Any& aElement )
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

    OUString aScript;
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
                    OUString aEventType;
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

uno::Any SAL_CALL ScSheetEventsObj::getByName( const OUString& aName )
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_Int32 nEvent = lcl_GetEventFromName(aName);
    if (nEvent < 0)
        throw container::NoSuchElementException();

    const OUString* pScript = NULL;
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
                        OUString("EventType"), -1,
                        uno::makeAny( OUString("Script") ), beans::PropertyState_DIRECT_VALUE );
        aPropSeq[1] = beans::PropertyValue(
                        OUString("Script"), -1,
                        uno::makeAny( *pScript ), beans::PropertyState_DIRECT_VALUE );
        aRet <<= aPropSeq;
    }
    // empty Any if nothing was set
    return aRet;
}

uno::Sequence<OUString> SAL_CALL ScSheetEventsObj::getElementNames() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Sequence<OUString> aNames(SC_SHEETEVENT_COUNT);
    for (sal_Int32 nEvent=0; nEvent<SC_SHEETEVENT_COUNT; ++nEvent)
        aNames[nEvent] = ScSheetEvents::GetEventName(nEvent);
    return aNames;
}

sal_Bool SAL_CALL ScSheetEventsObj::hasByName( const OUString& aName ) throw(uno::RuntimeException)
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
