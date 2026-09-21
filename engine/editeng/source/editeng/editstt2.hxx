/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <editeng/editstat.hxx>

class InternalEditStatus : public EditStatus
{

public:
    void    TurnOnFlags( EEControlBits nFlags )
                { m_nControlBits |= nFlags; }

    void    TurnOffFlags( EEControlBits nFlags )
                { m_nControlBits &= ~nFlags; }

    bool    UseCharAttribs() const
                { return bool( m_nControlBits & EEControlBits::USECHARATTRIBS ); }

    bool    UseIdleFormatter() const
                { return bool( m_nControlBits & EEControlBits::DOIDLEFORMAT); }

    bool    AllowPasteSpecial() const
                { return bool( m_nControlBits & EEControlBits::PASTESPECIAL ); }

    bool    DoAutoIndenting() const
                { return bool( m_nControlBits & EEControlBits::AUTOINDENTING ); }

    bool    DoUndoAttribs() const
                { return bool( m_nControlBits & EEControlBits::UNDOATTRIBS ); }

    bool    OneCharPerLine() const
    {
        return bool(m_nControlBits & (EEControlBits::ONECHARPERLINE | EEControlBits::STACKED));
    }

    bool    IsOutliner() const
                { return bool( m_nControlBits & EEControlBits::OUTLINER ); }

    bool    DoNotUseColors() const
                { return bool( m_nControlBits & EEControlBits::NOCOLORS ); }

    bool    AllowBigObjects() const
                { return bool( m_nControlBits & EEControlBits::ALLOWBIGOBJS ); }

    bool    DoOnlineSpelling() const
                { return bool( m_nControlBits & EEControlBits::ONLINESPELLING ); }

    bool    DoStretch() const
                { return bool( m_nControlBits & EEControlBits::STRETCHING ); }

    bool    AutoPageSize() const
                { return bool( m_nControlBits & EEControlBits::AUTOPAGESIZE ); }
    bool    AutoPageWidth() const
                { return bool( m_nControlBits & EEControlBits::AUTOPAGESIZEX ); }
    bool    AutoPageHeight() const
                { return bool( m_nControlBits & EEControlBits::AUTOPAGESIZEY ); }

    bool    MarkNonUrlFields() const
                { return bool( m_nControlBits & EEControlBits::MARKNONURLFIELDS ); }

    bool    MarkUrlFields() const
                { return bool( m_nControlBits & EEControlBits::MARKURLFIELDS ); }

    bool    DoImportRTFStyleSheets() const
                { return bool( m_nControlBits & EEControlBits::RTFSTYLESHEETS ); }

    bool    DoAutoCorrect() const
                { return bool( m_nControlBits & EEControlBits::AUTOCORRECT ); }

    bool    DoAutoComplete() const
                { return bool( m_nControlBits & EEControlBits::AUTOCOMPLETE ); }

    bool    ULSpaceSummation() const
                { return bool( m_nControlBits & EEControlBits::ULSPACESUMMATION ); }

    bool    IsSingleLine() const
                { return bool( m_nControlBits & EEControlBits::SINGLELINE ); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
