/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

