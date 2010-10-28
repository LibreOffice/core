/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sheetdata.hxx,v $
 * $Revision: 1.16.32.2 $
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
    rtl::OUString** mpScriptNames;

    void        Clear();

public:
                ScSheetEvents();
                ScSheetEvents(const ScSheetEvents& rOther);
                ~ScSheetEvents();

    const ScSheetEvents&    operator= (const ScSheetEvents& rOther);

    const rtl::OUString*    GetScript(sal_Int32 nEvent) const;
    void                    SetScript(sal_Int32 nEvent, const rtl::OUString* pNew);

    static rtl::OUString    GetEventName(sal_Int32 nEvent);
    static sal_Int32        GetVbaSheetEventId(sal_Int32 nEvent);
    static sal_Int32        GetVbaDocumentEventId(sal_Int32 nEvent);
};

#endif

