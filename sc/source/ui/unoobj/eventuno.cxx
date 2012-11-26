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
#include "precompiled_sc.hxx"

#include "eventuno.hxx"
#include "miscuno.hxx"
#include "unoguard.hxx"
#include "docsh.hxx"
#include "sheetevents.hxx"
#include "unonames.hxx"

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
    const SfxSimpleHint* pSfxSimpleHint = dynamic_cast< const SfxSimpleHint* >(&rHint);

    if(pSfxSimpleHint && SFX_HINT_DYING == pSfxSimpleHint->GetId())
    {
        mpDocShell = NULL;
    }
}

sal_Int32 lcl_GetEventFromName( const rtl::OUString& aName )
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
    ScUnoGuard aGuard;
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
    if (aScript.getLength())
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
    ScUnoGuard aGuard;
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
                        rtl::OUString::createFromAscii("EventType"), -1,
                        uno::makeAny( rtl::OUString::createFromAscii("Script") ), beans::PropertyState_DIRECT_VALUE );
        aPropSeq[1] = beans::PropertyValue(
                        rtl::OUString::createFromAscii("Script"), -1,
                        uno::makeAny( *pScript ), beans::PropertyState_DIRECT_VALUE );
        aRet <<= aPropSeq;
    }
    // empty Any if nothing was set
    return aRet;
}

uno::Sequence<rtl::OUString> SAL_CALL ScSheetEventsObj::getElementNames() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Sequence<rtl::OUString> aNames(SC_SHEETEVENT_COUNT);
    for (sal_Int32 nEvent=0; nEvent<SC_SHEETEVENT_COUNT; ++nEvent)
        aNames[nEvent] = ScSheetEvents::GetEventName(nEvent);
    return aNames;
}

sal_Bool SAL_CALL ScSheetEventsObj::hasByName( const ::rtl::OUString& aName ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    sal_Int32 nEvent = lcl_GetEventFromName(aName);
    return (nEvent >= 0);
}

// XElementAccess

uno::Type SAL_CALL ScSheetEventsObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Sequence<beans::PropertyValue>*)0);
}

sal_Bool SAL_CALL ScSheetEventsObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (mpDocShell)
        return sal_True;
    return sal_False;
}



