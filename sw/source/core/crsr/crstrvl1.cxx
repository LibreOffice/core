/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: crstrvl1.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:29:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#ifndef _CRSRSH_HXX
#include <crsrsh.hxx>
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif
#ifndef _CALLNK_HXX
#include <callnk.hxx>
#endif

BOOL SwCrsrShell::IsStartWord() const
{
    return pCurCrsr->IsStartWord();
}
BOOL SwCrsrShell::IsEndWord() const
{
    return pCurCrsr->IsEndWord();
}
BOOL SwCrsrShell::IsStartSentence() const
{
    return pCurCrsr->IsStartEndSentence( false );
}
BOOL SwCrsrShell::IsEndSentence() const
{
    return pCurCrsr->IsStartEndSentence( true );
}
BOOL SwCrsrShell::IsInWord() const
{
    return pCurCrsr->IsInWord();
}


BOOL SwCrsrShell::GoStartWord()
{
    return CallCrsrFN( &SwCursor::GoStartWord );
}
BOOL SwCrsrShell::GoEndWord()
{
    return CallCrsrFN( &SwCursor::GoEndWord );
}
BOOL SwCrsrShell::GoNextWord()
{
    return CallCrsrFN( &SwCursor::GoNextWord );
}
BOOL SwCrsrShell::GoPrevWord()
{
    return CallCrsrFN( &SwCursor::GoPrevWord );
}
BOOL SwCrsrShell::GoNextSentence()
{
    return CallCrsrFN( &SwCursor::GoNextSentence );
}
BOOL SwCrsrShell::GoEndSentence()
{
    return CallCrsrFN( &SwCursor::GoEndSentence );
}

BOOL SwCrsrShell::GoPrevSentence()
{
    return CallCrsrFN( &SwCursor::GoPrevSentence );
}
BOOL SwCrsrShell::GoStartSentence()
{
    return CallCrsrFN( &SwCursor::GoStartSentence );
}

BOOL SwCrsrShell::SelectWord( const Point* pPt )
{
    return pCurCrsr->SelectWord( pPt );
}


