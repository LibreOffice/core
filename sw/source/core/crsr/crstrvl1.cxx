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

#include <crsrsh.hxx>
#include <viscrs.hxx>
#include <callnk.hxx>

bool SwCursorShell::IsStartWord( sal_Int16 nWordType ) const
{
    return m_pCurrentCursor->IsStartWord( nWordType );
}
bool SwCursorShell::IsEndWord( sal_Int16 nWordType ) const
{
    return m_pCurrentCursor->IsEndWord( nWordType );
}

bool SwCursorShell::IsInWord( sal_Int16 nWordType ) const
{
    return m_pCurrentCursor->IsInWord( nWordType );
}

bool SwCursorShell::IsStartSentence() const
{
    return m_pCurrentCursor->IsStartEndSentence( false );
}
bool SwCursorShell::IsEndSentence() const
{
    return m_pCurrentCursor->IsStartEndSentence( true );
}

bool SwCursorShell::GoStartWord()
{
    return CallCursorFN( &SwCursor::GoStartWord );
}
bool SwCursorShell::GoEndWord()
{
    return CallCursorFN( &SwCursor::GoEndWord );
}

bool SwCursorShell::GoNextWord()
{
    return CallCursorFN( &SwCursor::GoNextWord );
}
bool SwCursorShell::GoPrevWord()
{
    return CallCursorFN( &SwCursor::GoPrevWord );
}

bool SwCursorShell::GoNextSentence()
{
    return CallCursorFN( &SwCursor::GoNextSentence );
}

bool SwCursorShell::GoEndSentence()
{
    return CallCursorFN( &SwCursor::GoEndSentence );
}
bool SwCursorShell::GoStartSentence()
{
    return CallCursorFN( &SwCursor::GoStartSentence );
}

bool SwCursorShell::SelectWord( const Point* pPt )
{
    return m_pCurrentCursor->SelectWord( this, pPt );
}

bool SwCursorShell::ExpandToSentenceBorders()
{
    return m_pCurrentCursor->ExpandToSentenceBorders();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
