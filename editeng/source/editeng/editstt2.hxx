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

#ifndef _EDITSTT2_HXX
#define _EDITSTT2_HXX

#include <editeng/editstat.hxx>

class InternalEditStatus : public EditStatus
{

public:
            InternalEditStatus() { ; }

    void    TurnOnFlags( sal_uLong nFlags )
                { nControlBits |= nFlags; }

    void    TurnOffFlags( sal_uLong nFlags )
                { nControlBits &= ~nFlags; }

    void    TurnOnStatusBits( sal_uLong nBits )
                { nStatusBits |= nBits; }

    void    TurnOffStatusBits( sal_uLong nBits )
                { nStatusBits &= ~nBits; }


    sal_Bool    UseCharAttribs() const
                { return ( ( nControlBits & EE_CNTRL_USECHARATTRIBS ) != 0 ); }

    sal_Bool    NotifyCursorMovements() const
                { return ( ( nControlBits & EE_CNTRL_CRSRLEFTPARA ) != 0 ); }

    sal_Bool    UseIdleFormatter() const
                { return ( ( nControlBits & EE_CNTRL_DOIDLEFORMAT) != 0 ); }

    sal_Bool    AllowPasteSpecial() const
                { return ( ( nControlBits & EE_CNTRL_PASTESPECIAL ) != 0 ); }

    sal_Bool    DoAutoIndenting() const
                { return ( ( nControlBits & EE_CNTRL_AUTOINDENTING ) != 0 ); }

    sal_Bool    DoUndoAttribs() const
                { return ( ( nControlBits & EE_CNTRL_UNDOATTRIBS ) != 0 ); }

    sal_Bool    OneCharPerLine() const
                { return ( ( nControlBits & EE_CNTRL_ONECHARPERLINE ) != 0 ); }

    sal_Bool    IsOutliner() const
                { return ( ( nControlBits & EE_CNTRL_OUTLINER ) != 0 ); }

    sal_Bool    IsOutliner2() const
                { return ( ( nControlBits & EE_CNTRL_OUTLINER2 ) != 0 ); }

    sal_Bool    IsAnyOutliner() const
                { return IsOutliner() || IsOutliner2(); }

    sal_Bool    DoNotUseColors() const
                { return ( ( nControlBits & EE_CNTRL_NOCOLORS ) != 0 ); }

    sal_Bool    AllowBigObjects() const
                { return ( ( nControlBits & EE_CNTRL_ALLOWBIGOBJS ) != 0 ); }

    sal_Bool    DoOnlineSpelling() const
                { return ( ( nControlBits & EE_CNTRL_ONLINESPELLING ) != 0 ); }

    sal_Bool    DoStretch() const
                { return ( ( nControlBits & EE_CNTRL_STRETCHING ) != 0 ); }

    sal_Bool    AutoPageSize() const
                { return ( ( nControlBits & EE_CNTRL_AUTOPAGESIZE ) != 0 ); }
    sal_Bool    AutoPageWidth() const
                { return ( ( nControlBits & EE_CNTRL_AUTOPAGESIZEX ) != 0 ); }
    sal_Bool    AutoPageHeight() const
                { return ( ( nControlBits & EE_CNTRL_AUTOPAGESIZEY ) != 0 ); }

    sal_Bool    MarkFields() const
                { return ( ( nControlBits & EE_CNTRL_MARKFIELDS ) != 0 ); }

    sal_Bool    DoRestoreFont() const
                { return ( ( nControlBits & EE_CNTRL_RESTOREFONT ) != 0 ); }

    sal_Bool    DoImportRTFStyleSheets() const
                { return ( ( nControlBits & EE_CNTRL_RTFSTYLESHEETS ) != 0 ); }

    sal_Bool    DoAutoCorrect() const
                { return ( ( nControlBits & EE_CNTRL_AUTOCORRECT ) != 0 ); }

    sal_Bool    DoAutoComplete() const
                { return ( ( nControlBits & EE_CNTRL_AUTOCOMPLETE ) != 0 ); }

    sal_Bool    DoTabIndenting() const
                { return ( ( nControlBits & EE_CNTRL_TABINDENTING ) != 0 ); }

    sal_Bool    DoFormat100() const
                { return ( ( nControlBits & EE_CNTRL_FORMAT100 ) != 0 ); }

    sal_Bool    ULSpaceSummation() const
                { return ( ( nControlBits & EE_CNTRL_ULSPACESUMMATION ) != 0 ); }

    sal_Bool    ULSpaceFirstParagraph() const
                { return ( ( nControlBits & EE_CNTRL_ULSPACEFIRSTPARA ) != 0 ); }
};

#endif // _EDITSTT2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
