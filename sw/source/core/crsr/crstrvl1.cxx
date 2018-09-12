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

#include <com/sun/star/i18n/WordType.hpp>

using namespace ::com::sun::star::i18n;

bool SwCursorShell::IsStartWord( sal_Int16 nWordType ) const
{
    return m_pCurrentCursor->IsStartWordWT(nWordType, GetLayout());
}
bool SwCursorShell::IsEndWord( sal_Int16 nWordType ) const
{
    return m_pCurrentCursor->IsEndWordWT(nWordType, GetLayout());
}

bool SwCursorShell::IsInWord( sal_Int16 nWordType ) const
{
    return m_pCurrentCursor->IsInWordWT(nWordType, GetLayout());
}

bool SwCursorShell::IsStartSentence() const
{
    return m_pCurrentCursor->IsStartEndSentence(false, GetLayout());
}
bool SwCursorShell::IsEndSentence() const
{
    return m_pCurrentCursor->IsStartEndSentence(true, GetLayout());
}

bool SwCursorShell::GoStartWord()
{
    return CallCursorShellFN( &SwCursorShell::GoStartWordImpl );
}
bool SwCursorShell::GoEndWord()
{
    return CallCursorShellFN( &SwCursorShell::GoEndWordImpl );
}

bool SwCursorShell::GoNextWord()
{
    return CallCursorShellFN( &SwCursorShell::GoNextWordImpl );
}
bool SwCursorShell::GoPrevWord()
{
    return CallCursorShellFN( &SwCursorShell::GoPrevWordImpl );
}

bool SwCursorShell::GoNextSentence()
{
    return CallCursorShellFN( &SwCursorShell::GoNextSentenceImpl );
}

bool SwCursorShell::GoEndSentence()
{
    return CallCursorShellFN( &SwCursorShell::GoEndSentenceImpl );
}

bool SwCursorShell::GoStartSentence()
{
    return CallCursorShellFN( &SwCursorShell::GoStartSentenceImpl );
}

bool SwCursorShell::SelectWord( const Point* pPt )
{
    return m_pCurrentCursor->SelectWord( this, pPt );
}

void SwCursorShell::ExpandToSentenceBorders()
{
    m_pCurrentCursor->ExpandToSentenceBorders(GetLayout());
}

bool SwCursorShell::GoStartWordImpl()
{
    return getShellCursor(true)->GoStartWordWT(WordType::ANYWORD_IGNOREWHITESPACES, GetLayout());
}

bool SwCursorShell::GoEndWordImpl()
{
    return getShellCursor(true)->GoEndWordWT(WordType::ANYWORD_IGNOREWHITESPACES, GetLayout());
}

bool SwCursorShell::GoNextWordImpl()
{
    return getShellCursor(true)->GoNextWordWT(WordType::ANYWORD_IGNOREWHITESPACES, GetLayout());
}

bool SwCursorShell::GoPrevWordImpl()
{
    return getShellCursor(true)->GoPrevWordWT(WordType::ANYWORD_IGNOREWHITESPACES, GetLayout());
}

bool SwCursorShell::GoNextSentenceImpl()
{
    return getShellCursor(true)->GoSentence(SwCursor::NEXT_SENT, GetLayout());
}
bool SwCursorShell::GoEndSentenceImpl()
{
    return getShellCursor(true)->GoSentence(SwCursor::END_SENT, GetLayout());
}
bool SwCursorShell::GoStartSentenceImpl()
{
    return getShellCursor(true)->GoSentence(SwCursor::START_SENT, GetLayout());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
