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

#ifndef SC_SHEETEVENTS_HXX
#define SC_SHEETEVENTS_HXX

#include <rtl/ustring.hxx>

#define SC_SHEETEVENT_FOCUS         0
#define SC_SHEETEVENT_UNFOCUS       1
#define SC_SHEETEVENT_SELECT        2
#define SC_SHEETEVENT_DOUBLECLICK   3
#define SC_SHEETEVENT_RIGHTCLICK    4
#define SC_SHEETEVENT_CHANGE        5
#define SC_SHEETEVENT_CALCULATE     6
#define SC_SHEETEVENT_COUNT         7

class ScSheetEvents
{
    OUString** mpScriptNames;

    void        Clear();

public:
                ScSheetEvents();
                ScSheetEvents(const ScSheetEvents& rOther);
                ~ScSheetEvents();

    const ScSheetEvents&    operator= (const ScSheetEvents& rOther);

    const OUString*    GetScript(sal_Int32 nEvent) const;
    void                    SetScript(sal_Int32 nEvent, const OUString* pNew);

    static OUString    GetEventName(sal_Int32 nEvent);
    static sal_Int32        GetVbaSheetEventId(sal_Int32 nEvent);
    static sal_Int32        GetVbaDocumentEventId(sal_Int32 nEvent);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
