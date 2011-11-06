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

// INCLUDE ---------------------------------------------------------------

#include "sheetevents.hxx"
#include <com/sun/star/script/vba/VBAEventId.hpp>
#include <tools/debug.hxx>

// -----------------------------------------------------------------------

// static
rtl::OUString ScSheetEvents::GetEventName(sal_Int32 nEvent)
{
    if (nEvent<0 || nEvent>=SC_SHEETEVENT_COUNT)
    {
        DBG_ERRORFILE("invalid event number");
        return rtl::OUString();
    }

    static const sal_Char* aEventNames[] =
    {
        "OnFocus",                  // SC_SHEETEVENT_FOCUS
        "OnUnfocus",                // SC_SHEETEVENT_UNFOCUS
        "OnSelect",                 // SC_SHEETEVENT_SELECT
        "OnDoubleClick",            // SC_SHEETEVENT_DOUBLECLICK
        "OnRightClick",             // SC_SHEETEVENT_RIGHTCLICK
        "OnChange",                 // SC_SHEETEVENT_CHANGE
        "OnCalculate"               // SC_SHEETEVENT_CALCULATE
    };
    return rtl::OUString::createFromAscii(aEventNames[nEvent]);
}

// static
sal_Int32 ScSheetEvents::GetVbaSheetEventId(sal_Int32 nEvent)
{
    using namespace ::com::sun::star::script::vba::VBAEventId;
    if (nEvent<0 || nEvent>=SC_SHEETEVENT_COUNT)
    {
        DBG_ERRORFILE("invalid event number");
        return NO_EVENT;
    }

    static const sal_Int32 nVbaEventIds[] =
    {
        WORKSHEET_ACTIVATE,             // SC_SHEETEVENT_FOCUS
        WORKSHEET_DEACTIVATE,           // SC_SHEETEVENT_UNFOCUS
        WORKSHEET_SELECTIONCHANGE,      // SC_SHEETEVENT_SELECT
        WORKSHEET_BEFOREDOUBLECLICK,    // SC_SHEETEVENT_DOUBLECLICK
        WORKSHEET_BEFORERIGHTCLICK,     // SC_SHEETEVENT_RIGHTCLICK
        WORKSHEET_CHANGE,               // SC_SHEETEVENT_CHANGE
        WORKSHEET_CALCULATE             // SC_SHEETEVENT_CALCULATE
    };
    return nVbaEventIds[nEvent];
}

// static
sal_Int32 ScSheetEvents::GetVbaDocumentEventId(sal_Int32 nEvent)
{
    using namespace ::com::sun::star::script::vba::VBAEventId;
    sal_Int32 nSheetEventId = GetVbaSheetEventId(nEvent);
    return (nSheetEventId != NO_EVENT) ? (nSheetEventId + USERDEFINED_START) : NO_EVENT;
}

// -----------------------------------------------------------------------

ScSheetEvents::ScSheetEvents() :
    mpScriptNames(NULL)
{
}

ScSheetEvents::~ScSheetEvents()
{
    Clear();
}

void ScSheetEvents::Clear()
{
    if (mpScriptNames)
    {
        for (sal_Int32 nEvent=0; nEvent<SC_SHEETEVENT_COUNT; ++nEvent)
            delete mpScriptNames[nEvent];
        delete[] mpScriptNames;
        mpScriptNames = NULL;
    }
}

ScSheetEvents::ScSheetEvents(const ScSheetEvents& rOther) :
    mpScriptNames(NULL)
{
    *this = rOther;
}

const ScSheetEvents& ScSheetEvents::operator=(const ScSheetEvents& rOther)
{
    Clear();
    if (rOther.mpScriptNames)
    {
        mpScriptNames = new rtl::OUString*[SC_SHEETEVENT_COUNT];
        for (sal_Int32 nEvent=0; nEvent<SC_SHEETEVENT_COUNT; ++nEvent)
            if (rOther.mpScriptNames[nEvent])
                mpScriptNames[nEvent] = new rtl::OUString(*rOther.mpScriptNames[nEvent]);
            else
                mpScriptNames[nEvent] = NULL;
    }
    return *this;
}

const rtl::OUString* ScSheetEvents::GetScript(sal_Int32 nEvent) const
{
    if (nEvent<0 || nEvent>=SC_SHEETEVENT_COUNT)
    {
        DBG_ERRORFILE("invalid event number");
        return NULL;
    }

    if (mpScriptNames)
        return mpScriptNames[nEvent];
    return NULL;
}

void ScSheetEvents::SetScript(sal_Int32 nEvent, const rtl::OUString* pNew)
{
    if (nEvent<0 || nEvent>=SC_SHEETEVENT_COUNT)
    {
        DBG_ERRORFILE("invalid event number");
        return;
    }

    if (!mpScriptNames)
    {
        mpScriptNames = new rtl::OUString*[SC_SHEETEVENT_COUNT];
        for (sal_Int32 nEventIdx=0; nEventIdx<SC_SHEETEVENT_COUNT; ++nEventIdx)
            mpScriptNames[nEventIdx] = NULL;
    }
    delete mpScriptNames[nEvent];
    if (pNew)
        mpScriptNames[nEvent] = new rtl::OUString(*pNew);
    else
        mpScriptNames[nEvent] = NULL;
}

