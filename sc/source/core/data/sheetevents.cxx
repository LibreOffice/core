/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sheetdata.cxx,v $
 * $Revision: 1.69.32.3 $
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

