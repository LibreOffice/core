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


#include <paralist.hxx>

#include <editeng/outliner.hxx>
#include <editeng/numdef.hxx>

#include <osl/diagnose.h>
#include <tools/debug.hxx>

ParagraphData::ParagraphData()
: nDepth( -1 )
, mnNumberingStartValue( -1 )
, mbParaIsNumberingRestart( false )
{
}

ParagraphData::ParagraphData( const ParagraphData& r )
: nDepth( r.nDepth )
, mnNumberingStartValue( r.mnNumberingStartValue )
, mbParaIsNumberingRestart( r.mbParaIsNumberingRestart )
{
}

ParagraphData& ParagraphData::operator=( const ParagraphData& r)
{
    nDepth = r.nDepth;
    mnNumberingStartValue = r.mnNumberingStartValue;
    mbParaIsNumberingRestart = r.mbParaIsNumberingRestart;
    return *this;
}

bool ParagraphData::operator==(const ParagraphData& rCandidate) const
{
    return (nDepth == rCandidate.nDepth
        && mnNumberingStartValue == rCandidate.mnNumberingStartValue
        && mbParaIsNumberingRestart == rCandidate.mbParaIsNumberingRestart);
}

Paragraph::Paragraph( sal_Int16 nDDepth )
: aBulSize( -1, -1)
{

    DBG_ASSERT(  ( nDDepth >= -1 ) && ( nDDepth < SVX_MAX_NUM ), "Paragraph-CTOR: nDepth invalid!" );

    nDepth = nDDepth;
    nFlags = ParaFlag::NONE;
    bVisible = true;
}

Paragraph::Paragraph( const ParagraphData& rData )
: nFlags( ParaFlag::NONE )
, aBulSize( -1, -1)
, bVisible( true )
{
    nDepth = rData.nDepth;
    mnNumberingStartValue = rData.mnNumberingStartValue;
    mbParaIsNumberingRestart = rData.mbParaIsNumberingRestart;
}

Paragraph::~Paragraph()
{
}

void Paragraph::SetNumberingStartValue( sal_Int16 nNumberingStartValue )
{
    mnNumberingStartValue = nNumberingStartValue;
    if( mnNumberingStartValue != -1 )
        mbParaIsNumberingRestart = true;
}

void Paragraph::SetParaIsNumberingRestart( bool bParaIsNumberingRestart )
{
    mbParaIsNumberingRestart = bParaIsNumberingRestart;
    if( !mbParaIsNumberingRestart )
        mnNumberingStartValue = -1;
}

void ParagraphList::Clear( bool bDestroyParagraphs )
{
    if ( bDestroyParagraphs )
    {
        std::vector<Paragraph*>::iterator iter;
        for (iter = maEntries.begin(); iter != maEntries.end(); ++iter)
            delete *iter;
    }

    maEntries.clear();
}

void ParagraphList::Append( Paragraph* pPara)
{
    SAL_WARN_IF( maEntries.size() >= EE_PARA_MAX_COUNT, "editeng", "ParagraphList::Append - overflow");
    maEntries.push_back(pPara);
}

void ParagraphList::Insert( Paragraph* pPara, sal_Int32 nAbsPos)
{
    SAL_WARN_IF( nAbsPos < 0 || (maEntries.size() < static_cast<size_t>(nAbsPos) && nAbsPos != EE_PARA_APPEND),
            "editeng", "ParagraphList::Insert - bad insert position " << nAbsPos);
    SAL_WARN_IF( maEntries.size() >= EE_PARA_MAX_COUNT, "editeng", "ParagraphList::Insert - overflow");

    if (nAbsPos < 0 || maEntries.size() <= static_cast<size_t>(nAbsPos))
        Append( pPara);
    else
        maEntries.insert(maEntries.begin()+nAbsPos,pPara);
}

void ParagraphList::Remove( sal_Int32 nPara )
{
    if (nPara < 0 || maEntries.size() <= static_cast<size_t>(nPara))
    {
        SAL_WARN( "editeng", "ParagraphList::Remove - out of bounds " << nPara);
        return;
    }

    maEntries.erase(maEntries.begin() + nPara );
}

void ParagraphList::MoveParagraphs( sal_Int32 nStart, sal_Int32 nDest, sal_Int32 _nCount )
{
    OSL_ASSERT(static_cast<size_t>(nStart) < maEntries.size() && static_cast<size_t>(nDest) < maEntries.size());

    if ( (( nDest < nStart ) || ( nDest >= ( nStart + _nCount ) )) && nStart >= 0 && nDest >= 0 && _nCount >= 0 )
    {
        std::vector<Paragraph*> aParas;
        std::vector<Paragraph*>::iterator iterBeg = maEntries.begin() + nStart;
        std::vector<Paragraph*>::iterator iterEnd = iterBeg + _nCount;

        std::copy(iterBeg,iterEnd,std::back_inserter(aParas));

        maEntries.erase(iterBeg,iterEnd);

        if ( nDest > nStart )
            nDest -= _nCount;

        std::vector<Paragraph*>::iterator iterIns = maEntries.begin() + nDest;

        std::copy(aParas.begin(),aParas.end(),std::inserter(maEntries,iterIns));
    }
    else
    {
        OSL_FAIL( "MoveParagraphs: Invalid Parameters" );
    }
}

bool ParagraphList::HasChildren( Paragraph* pParagraph ) const
{
    sal_Int32 n = GetAbsPos( pParagraph );
    Paragraph* pNext = GetParagraph( ++n );
    return pNext && ( pNext->GetDepth() > pParagraph->GetDepth() );
}

bool ParagraphList::HasHiddenChildren( Paragraph* pParagraph ) const
{
    sal_Int32 n = GetAbsPos( pParagraph );
    Paragraph* pNext = GetParagraph( ++n );
    return pNext && ( pNext->GetDepth() > pParagraph->GetDepth() ) && !pNext->IsVisible();
}

bool ParagraphList::HasVisibleChildren( Paragraph* pParagraph ) const
{
    sal_Int32 n = GetAbsPos( pParagraph );
    Paragraph* pNext = GetParagraph( ++n );
    return pNext && ( pNext->GetDepth() > pParagraph->GetDepth() ) && pNext->IsVisible();
}

sal_Int32 ParagraphList::GetChildCount( Paragraph* pParent ) const
{
    sal_Int32 nChildCount = 0;
    sal_Int32 n = GetAbsPos( pParent );
    Paragraph* pPara = GetParagraph( ++n );
    while ( pPara && ( pPara->GetDepth() > pParent->GetDepth() ) )
    {
        nChildCount++;
        pPara = GetParagraph( ++n );
    }
    return nChildCount;
}

Paragraph* ParagraphList::GetParent( Paragraph* pParagraph /*, sal_uInt16& rRelPos */ ) const
{
    /* rRelPos = 0 */;
    sal_Int32 n = GetAbsPos( pParagraph );
    Paragraph* pPrev = GetParagraph( --n );
    while ( pPrev && ( pPrev->GetDepth() >= pParagraph->GetDepth() ) )
    {
//      if ( pPrev->GetDepth() == pParagraph->GetDepth() )
//          rRelPos++;
        pPrev = GetParagraph( --n );
    }

    return pPrev;
}

void ParagraphList::Expand( Paragraph* pParent )
{
    sal_Int32 nChildCount = GetChildCount( pParent );
    sal_Int32 nPos = GetAbsPos( pParent );

    for ( sal_Int32 n = 1; n <= nChildCount; n++  )
    {
        Paragraph* pPara = GetParagraph( nPos+n );
        if ( !( pPara->IsVisible() ) )
        {
            pPara->bVisible = true;
            aVisibleStateChangedHdl.Call( *pPara );
        }
    }
}

void ParagraphList::Collapse( Paragraph* pParent )
{
    sal_Int32 nChildCount = GetChildCount( pParent );
    sal_Int32 nPos = GetAbsPos( pParent );

    for ( sal_Int32 n = 1; n <= nChildCount; n++  )
    {
        Paragraph* pPara = GetParagraph( nPos+n );
        if ( pPara->IsVisible() )
        {
            pPara->bVisible = false;
            aVisibleStateChangedHdl.Call( *pPara );
        }
    }
}

sal_Int32 ParagraphList::GetAbsPos( Paragraph* pParent ) const
{
    sal_Int32 pos = 0;
    std::vector<Paragraph*>::const_iterator iter;
    for (iter = maEntries.begin(); iter != maEntries.end(); ++iter, ++pos)
    {
        if (*iter == pParent)
            return pos;
    }

    return EE_PARA_NOT_FOUND;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
