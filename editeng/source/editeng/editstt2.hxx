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

#ifndef INCLUDED_EDITENG_SOURCE_EDITENG_EDITSTT2_HXX
#define INCLUDED_EDITENG_SOURCE_EDITENG_EDITSTT2_HXX

#include <editeng/editstat.hxx>

class InternalEditStatus : public EditStatus
{

public:
    void    TurnOnFlags( EEControlBits nFlags )
                { nControlBits |= nFlags; }

    void    TurnOffFlags( EEControlBits nFlags )
                { nControlBits &= ~nFlags; }

    bool    UseCharAttribs() const
                { return bool( nControlBits & EEControlBits::USECHARATTRIBS ); }

    bool    NotifyCursorMovements() const
                { return bool( nControlBits & EEControlBits::CRSRLEFTPARA ); }

    bool    UseIdleFormatter() const
                { return bool( nControlBits & EEControlBits::DOIDLEFORMAT); }

    bool    AllowPasteSpecial() const
                { return bool( nControlBits & EEControlBits::PASTESPECIAL ); }

    bool    DoAutoIndenting() const
                { return bool( nControlBits & EEControlBits::AUTOINDENTING ); }

    bool    DoUndoAttribs() const
                { return bool( nControlBits & EEControlBits::UNDOATTRIBS ); }

    bool    OneCharPerLine() const
                { return bool( nControlBits & EEControlBits::ONECHARPERLINE ); }

    bool    IsOutliner() const
                { return bool( nControlBits & EEControlBits::OUTLINER ); }

    bool    IsOutliner2() const
                { return bool( nControlBits & EEControlBits::OUTLINER2 ); }

    bool    IsAnyOutliner() const
                { return IsOutliner() || IsOutliner2(); }

    bool    DoNotUseColors() const
                { return bool( nControlBits & EEControlBits::NOCOLORS ); }

    bool    AllowBigObjects() const
                { return bool( nControlBits & EEControlBits::ALLOWBIGOBJS ); }

    bool    DoOnlineSpelling() const
                { return bool( nControlBits & EEControlBits::ONLINESPELLING ); }

    bool    DoStretch() const
                { return bool( nControlBits & EEControlBits::STRETCHING ); }

    bool    AutoPageSize() const
                { return bool( nControlBits & EEControlBits::AUTOPAGESIZE ); }
    bool    AutoPageWidth() const
                { return bool( nControlBits & EEControlBits::AUTOPAGESIZEX ); }
    bool    AutoPageHeight() const
                { return bool( nControlBits & EEControlBits::AUTOPAGESIZEY ); }

    bool    MarkFields() const
                { return bool( nControlBits & EEControlBits::MARKFIELDS ); }

    bool    DoRestoreFont() const
                { return bool( nControlBits & EEControlBits::RESTOREFONT ); }

    bool    DoImportRTFStyleSheets() const
                { return bool( nControlBits & EEControlBits::RTFSTYLESHEETS ); }

    bool    DoAutoCorrect() const
                { return bool( nControlBits & EEControlBits::AUTOCORRECT ); }

    bool    DoAutoComplete() const
                { return bool( nControlBits & EEControlBits::AUTOCOMPLETE ); }

    bool    DoTabIndenting() const
                { return bool( nControlBits & EEControlBits::TABINDENTING ); }

    bool    DoFormat100() const
                { return bool( nControlBits & EEControlBits::FORMAT100 ); }

    bool    ULSpaceSummation() const
                { return bool( nControlBits & EEControlBits::ULSPACESUMMATION ); }

    bool    ULSpaceFirstParagraph() const
                { return bool( nControlBits & EEControlBits::ULSPACEFIRSTPARA ); }
};

#endif // INCLUDED_EDITENG_SOURCE_EDITENG_EDITSTT2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
