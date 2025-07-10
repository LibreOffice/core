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

#include <sheetevents.hxx>
#include <com/sun/star/script/vba/VBAEventId.hpp>
#include <optional>

OUString ScSheetEvents::GetEventName(ScSheetEventId nEvent)
{
    static const char* const aEventNames[] =
    {
        "OnFocus",                  // SC_SHEETEVENT_FOCUS
        "OnUnfocus",                // SC_SHEETEVENT_UNFOCUS
        "OnSelect",                 // SC_SHEETEVENT_SELECT
        "OnDoubleClick",            // SC_SHEETEVENT_DOUBLECLICK
        "OnRightClick",             // SC_SHEETEVENT_RIGHTCLICK
        "OnChange",                 // SC_SHEETEVENT_CHANGE
        "OnCalculate"               // SC_SHEETEVENT_CALCULATE
    };
    return OUString::createFromAscii(aEventNames[static_cast<int>(nEvent)]);
}

sal_Int32 ScSheetEvents::GetVbaSheetEventId(ScSheetEventId nEvent)
{
    using namespace ::com::sun::star::script::vba::VBAEventId;

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
    return nVbaEventIds[static_cast<int>(nEvent)];
}

const int COUNT = static_cast<int>(ScSheetEventId::COUNT);

sal_Int32 ScSheetEvents::GetVbaDocumentEventId(ScSheetEventId nEvent)
{
    using namespace ::com::sun::star::script::vba::VBAEventId;
    sal_Int32 nSheetEventId = GetVbaSheetEventId(nEvent);
    return (nSheetEventId != NO_EVENT) ? (nSheetEventId + USERDEFINED_START) : NO_EVENT;
}

ScSheetEvents::ScSheetEvents()
{
}

ScSheetEvents::~ScSheetEvents()
{
    Clear();
}

void ScSheetEvents::Clear()
{
    mpScriptNames.reset();
}

ScSheetEvents::ScSheetEvents(const ScSheetEvents& rOther)
{
    *this = rOther;
}

ScSheetEvents& ScSheetEvents::operator=(const ScSheetEvents& rOther)
{
    if (this != &rOther)
    {
        Clear();
        if (rOther.mpScriptNames)
        {
            mpScriptNames.reset( new std::optional<OUString>[COUNT] );
            for (sal_Int32 nEvent=0; nEvent<COUNT; ++nEvent)
                mpScriptNames[nEvent] = rOther.mpScriptNames[nEvent];
        }
    }
    return *this;
}

const OUString* ScSheetEvents::GetScript(ScSheetEventId nEvent) const
{
    if (mpScriptNames)
    {
        std::optional<OUString> const & r = mpScriptNames[static_cast<int>(nEvent)];
        if (r)
            return &*r;
    }
    return nullptr;
}

void ScSheetEvents::SetScript(ScSheetEventId eEvent, const OUString* pNew)
{
    int nEvent = static_cast<int>(eEvent);
    if (!mpScriptNames)
    {
        mpScriptNames.reset( new std::optional<OUString>[COUNT] );
    }
    if (pNew)
        mpScriptNames[nEvent] = *pNew;
    else
        mpScriptNames[nEvent].reset();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
