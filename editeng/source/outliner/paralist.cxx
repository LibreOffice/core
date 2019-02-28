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


#include "paralist.hxx"

#include <editeng/outliner.hxx>
#include <editeng/numdef.hxx>

#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <libxml/xmlwriter.h>

#include <iterator>

ParagraphData::ParagraphData()
: nDepth( -1 )
, mnNumberingStartValue( -1 )
, mbParaIsNumberingRestart( false )
{
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

void Paragraph::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("Paragraph"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("nDepth"), "%" SAL_PRIdINT32, static_cast<sal_Int32>(nDepth));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("mnNumberingStartValue"), "%" SAL_PRIdINT32, static_cast<sal_Int32>(mnNumberingStartValue));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("mbParaIsNumberingRestart"), "%" SAL_PRIdINT32, static_cast<sal_Int32>(mbParaIsNumberingRestart));
    xmlTextWriterEndElement(pWriter);
}

void ParagraphList::Clear()
{
    maEntries.clear();
}

void ParagraphList::Append( std::unique_ptr<Paragraph> pPara)
{
    SAL_WARN_IF( maEntries.size() >= EE_PARA_MAX_COUNT, "editeng", "ParagraphList::Append - overflow");
    maEntries.push_back(std::move(pPara));
}

void ParagraphList::Insert( std::unique_ptr<Paragraph> pPara, sal_Int32 nAbsPos)
{
    SAL_WARN_IF( nAbsPos < 0 || (maEntries.size() < static_cast<size_t>(nAbsPos) && nAbsPos != EE_PARA_APPEND),
            "editeng", "ParagraphList::Insert - bad insert position " << nAbsPos);
    SAL_WARN_IF( maEntries.size() >= EE_PARA_MAX_COUNT, "editeng", "ParagraphList::Insert - overflow");

    if (nAbsPos < 0 || maEntries.size() <= static_cast<size_t>(nAbsPos))
        Append( std::move(pPara) );
    else
        maEntries.insert(maEntries.begin()+nAbsPos, std::move(pPara));
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
        std::vector<std::unique_ptr<Paragraph>> aParas;
        auto iterBeg = maEntries.begin() + nStart;
        auto iterEnd = iterBeg + _nCount;

        for (auto it = iterBeg; it != iterEnd; ++it)
            aParas.push_back(std::move(*it));

        maEntries.erase(iterBeg,iterEnd);

        if ( nDest > nStart )
            nDest -= _nCount;

        for (auto & i : aParas)
        {
            maEntries.insert(maEntries.begin() + nDest, std::move(i));
            ++nDest;
        }
    }
    else
    {
        OSL_FAIL( "MoveParagraphs: Invalid Parameters" );
    }
}

bool ParagraphList::HasChildren( Paragraph const * pParagraph ) const
{
    sal_Int32 n = GetAbsPos( pParagraph );
    Paragraph* pNext = GetParagraph( ++n );
    return pNext && ( pNext->GetDepth() > pParagraph->GetDepth() );
}

bool ParagraphList::HasHiddenChildren( Paragraph const * pParagraph ) const
{
    sal_Int32 n = GetAbsPos( pParagraph );
    Paragraph* pNext = GetParagraph( ++n );
    return pNext && ( pNext->GetDepth() > pParagraph->GetDepth() ) && !pNext->IsVisible();
}

bool ParagraphList::HasVisibleChildren( Paragraph const * pParagraph ) const
{
    sal_Int32 n = GetAbsPos( pParagraph );
    Paragraph* pNext = GetParagraph( ++n );
    return pNext && ( pNext->GetDepth() > pParagraph->GetDepth() ) && pNext->IsVisible();
}

sal_Int32 ParagraphList::GetChildCount( Paragraph const * pParent ) const
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

Paragraph* ParagraphList::GetParent( Paragraph const * pParagraph ) const
{
    sal_Int32 n = GetAbsPos( pParagraph );
    Paragraph* pPrev = GetParagraph( --n );
    while ( pPrev && ( pPrev->GetDepth() >= pParagraph->GetDepth() ) )
    {
        pPrev = GetParagraph( --n );
    }

    return pPrev;
}

void ParagraphList::Expand( Paragraph const * pParent )
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

void ParagraphList::Collapse( Paragraph const * pParent )
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

sal_Int32 ParagraphList::GetAbsPos( Paragraph const * pParent ) const
{
    sal_Int32 pos = 0;
    for (auto const& entry : maEntries)
    {
        if (entry.get() == pParent)
            return pos;
        ++pos;
    }

    return EE_PARA_NOT_FOUND;
}

void ParagraphList::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("ParagraphList"));
    for (auto const & pParagraph : maEntries)
        pParagraph->dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
