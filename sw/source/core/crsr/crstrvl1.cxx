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

bool SwCrsrShell::IsStartWord( sal_Int16 nWordType ) const
{
    return m_pCurCrsr->IsStartWord( nWordType );
}
bool SwCrsrShell::IsEndWord( sal_Int16 nWordType ) const
{
    return m_pCurCrsr->IsEndWord( nWordType );
}

bool SwCrsrShell::IsInWord( sal_Int16 nWordType ) const
{
    return m_pCurCrsr->IsInWord( nWordType );
}

bool SwCrsrShell::IsStartSentence() const
{
    return m_pCurCrsr->IsStartEndSentence( false );
}
bool SwCrsrShell::IsEndSentence() const
{
    return m_pCurCrsr->IsStartEndSentence( true );
}

bool SwCrsrShell::GoStartWord()
{
    return CallCrsrFN( &SwCursor::GoStartWord );
}
bool SwCrsrShell::GoEndWord()
{
    return CallCrsrFN( &SwCursor::GoEndWord );
}

bool SwCrsrShell::GoNextWord()
{
    return CallCrsrFN( &SwCursor::GoNextWord );
}
bool SwCrsrShell::GoPrevWord()
{
    return CallCrsrFN( &SwCursor::GoPrevWord );
}

bool SwCrsrShell::GoNextSentence()
{
    return CallCrsrFN( &SwCursor::GoNextSentence );
}

bool SwCrsrShell::GoEndSentence()
{
    return CallCrsrFN( &SwCursor::GoEndSentence );
}
bool SwCrsrShell::GoStartSentence()
{
    return CallCrsrFN( &SwCursor::GoStartSentence );
}

bool SwCrsrShell::SelectWord( const Point* pPt )
{
    return m_pCurCrsr->SelectWord( this, pPt );
}

void SwCrsrShell::SelectCell()
{
    UpdateCrsr(SwCrsrShell::SELECTCELL);
}

bool SwCrsrShell::ExpandToSentenceBorders()
{
    return m_pCurCrsr->ExpandToSentenceBorders();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
