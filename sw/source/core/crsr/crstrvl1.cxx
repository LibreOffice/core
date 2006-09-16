/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: crstrvl1.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 20:45:36 $
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

FASTBOOL SwCrsrShell::IsStartWord() const
{
    return pCurCrsr->IsStartWord();
}
FASTBOOL SwCrsrShell::IsEndWord() const
{
    return pCurCrsr->IsEndWord();
}
FASTBOOL SwCrsrShell::IsStartSentence() const
{
    return pCurCrsr->IsStartEndSentence( false );
}
FASTBOOL SwCrsrShell::IsEndSentence() const
{
    return pCurCrsr->IsStartEndSentence( true );
}
FASTBOOL SwCrsrShell::IsInWord() const
{
    return pCurCrsr->IsInWord();
}


FASTBOOL SwCrsrShell::GoStartWord()
{
    return CallCrsrFN( &SwCursor::GoStartWord );
}
FASTBOOL SwCrsrShell::GoEndWord()
{
    return CallCrsrFN( &SwCursor::GoEndWord );
}
FASTBOOL SwCrsrShell::GoNextWord()
{
    return CallCrsrFN( &SwCursor::GoNextWord );
}
FASTBOOL SwCrsrShell::GoPrevWord()
{
    return CallCrsrFN( &SwCursor::GoPrevWord );
}
FASTBOOL SwCrsrShell::GoNextSentence()
{
    return CallCrsrFN( &SwCursor::GoNextSentence );
}
FASTBOOL SwCrsrShell::GoEndSentence()
{
    return CallCrsrFN( &SwCursor::GoEndSentence );
}

FASTBOOL SwCrsrShell::GoPrevSentence()
{
    return CallCrsrFN( &SwCursor::GoPrevSentence );
}
FASTBOOL SwCrsrShell::GoStartSentence()
{
    return CallCrsrFN( &SwCursor::GoStartSentence );
}

FASTBOOL SwCrsrShell::SelectWord( const Point* pPt )
{
    return pCurCrsr->SelectWord( pPt );
}


