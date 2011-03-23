/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_sw.hxx"


#include <crsrsh.hxx>
#include <viscrs.hxx>
#include <callnk.hxx>

sal_Bool SwCrsrShell::IsStartWord( sal_Int16 nWordType ) const
{
    return pCurCrsr->IsStartWord( nWordType );
}

sal_Bool SwCrsrShell::IsEndWord( sal_Int16 nWordType ) const
{
    return pCurCrsr->IsEndWord( nWordType );
}

sal_Bool SwCrsrShell::IsInWord( sal_Int16 nWordType ) const
{
    return pCurCrsr->IsInWord( nWordType );
}

sal_Bool SwCrsrShell::IsStartSentence() const
{
    return pCurCrsr->IsStartEndSentence( false );
}

sal_Bool SwCrsrShell::IsEndSentence() const
{
    return pCurCrsr->IsStartEndSentence( true );
}


sal_Bool SwCrsrShell::GoStartWord()
{
    return CallCrsrFN( &SwCursor::GoStartWord );
}
sal_Bool SwCrsrShell::GoEndWord()
{
    return CallCrsrFN( &SwCursor::GoEndWord );
}
sal_Bool SwCrsrShell::GoNextWord()
{
    return CallCrsrFN( &SwCursor::GoNextWord );
}
sal_Bool SwCrsrShell::GoPrevWord()
{
    return CallCrsrFN( &SwCursor::GoPrevWord );
}
sal_Bool SwCrsrShell::GoNextSentence()
{
    return CallCrsrFN( &SwCursor::GoNextSentence );
}
sal_Bool SwCrsrShell::GoEndSentence()
{
    return CallCrsrFN( &SwCursor::GoEndSentence );
}

sal_Bool SwCrsrShell::GoPrevSentence()
{
    return CallCrsrFN( &SwCursor::GoPrevSentence );
}
sal_Bool SwCrsrShell::GoStartSentence()
{
    return CallCrsrFN( &SwCursor::GoStartSentence );
}

sal_Bool SwCrsrShell::SelectWord( const Point* pPt )
{
    return pCurCrsr->SelectWord( this, pPt );
}

sal_Bool SwCrsrShell::ExpandToSentenceBorders()
{
    return pCurCrsr->ExpandToSentenceBorders();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
