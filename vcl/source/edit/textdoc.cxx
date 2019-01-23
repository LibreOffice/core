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

#include <memory>
#include "textdoc.hxx"
#include <stdlib.h>
#include <osl/diagnose.h>
#include <sal/log.hxx>

// compare function called by QuickSort
static bool CompareStart( const std::unique_ptr<TextCharAttrib>& pFirst, const std::unique_ptr<TextCharAttrib>& pSecond )
{
    return pFirst->GetStart() < pSecond->GetStart();
}

TextCharAttrib::TextCharAttrib( const TextAttrib& rAttr, sal_Int32 nStart, sal_Int32 nEnd )
    : mpAttr(rAttr.Clone())
    , mnStart(nStart)
    , mnEnd(nEnd)
{
}

TextCharAttrib::TextCharAttrib( const TextCharAttrib& rTextCharAttrib )
    : mpAttr(rTextCharAttrib.mpAttr->Clone())
    , mnStart(rTextCharAttrib.mnStart)
    , mnEnd(rTextCharAttrib.mnEnd)
{
}

TextCharAttribList::TextCharAttribList()
    : mbHasEmptyAttribs(false)
{
}

TextCharAttribList::~TextCharAttribList()
{
    // PTRARR_DEL
}

void TextCharAttribList::Clear()
{
    maAttribs.clear();
}

void TextCharAttribList::InsertAttrib( std::unique_ptr<TextCharAttrib> pAttrib )
{
    if ( pAttrib->IsEmpty() )
        mbHasEmptyAttribs = true;

    const sal_Int32 nStart = pAttrib->GetStart(); // maybe better for Comp.Opt.
    bool bInserted = false;
    auto it = std::find_if(maAttribs.begin(), maAttribs.end(),
        [nStart](std::unique_ptr<TextCharAttrib>& rAttrib) { return rAttrib->GetStart() > nStart; });
    if (it != maAttribs.end())
    {
        maAttribs.insert( it, std::move(pAttrib) );
        bInserted = true;
    }
    if ( !bInserted )
        maAttribs.push_back( std::move(pAttrib) );
}

void TextCharAttribList::ResortAttribs()
{
    std::sort( maAttribs.begin(), maAttribs.end(), CompareStart );
}

TextCharAttrib* TextCharAttribList::FindAttrib( sal_uInt16 nWhich, sal_Int32 nPos )
{
    for (std::vector<std::unique_ptr<TextCharAttrib> >::reverse_iterator it = maAttribs.rbegin(); it != maAttribs.rend(); ++it)
    {
        if ( (*it)->GetEnd() < nPos )
            return nullptr;

        if ( ( (*it)->Which() == nWhich ) && (*it)->IsIn(nPos) )
            return it->get();
    }
    return nullptr;
}

bool TextCharAttribList::HasBoundingAttrib( sal_Int32 nBound )
{
    for (std::vector<std::unique_ptr<TextCharAttrib> >::reverse_iterator it = maAttribs.rbegin(); it != maAttribs.rend(); ++it)
    {
        if ( (*it)->GetEnd() < nBound )
            return false;

        if ( ( (*it)->GetStart() == nBound ) || ( (*it)->GetEnd() == nBound ) )
            return true;
    }
    return false;
}

TextCharAttrib* TextCharAttribList::FindEmptyAttrib( sal_uInt16 nWhich, sal_Int32 nPos )
{
    if ( !mbHasEmptyAttribs )
        return nullptr;

    for (auto const& attrib : maAttribs)
    {
        if ( attrib->GetStart() > nPos )
            return nullptr;

        if ( ( attrib->GetStart() == nPos ) && ( attrib->GetEnd() == nPos ) && ( attrib->Which() == nWhich ) )
            return attrib.get();
    }
    return nullptr;
}

void TextCharAttribList::DeleteEmptyAttribs()
{
    maAttribs.erase(
        std::remove_if( maAttribs.begin(), maAttribs.end(),
            [] (const std::unique_ptr<TextCharAttrib>& rAttrib) { return rAttrib->IsEmpty(); } ),
        maAttribs.end() );
    mbHasEmptyAttribs = false;
}

TextNode::TextNode( const OUString& rText ) :
    maText( rText )
{
}

void TextNode::ExpandAttribs( sal_Int32 nIndex, sal_Int32 nNew )
{
    if ( !nNew )
        return;

    bool bResort = false;
    sal_uInt16 nAttribs = maCharAttribs.Count();
    for ( sal_uInt16 nAttr = 0; nAttr < nAttribs; nAttr++ )
    {
        TextCharAttrib& rAttrib = maCharAttribs.GetAttrib( nAttr );
        if ( rAttrib.GetEnd() >= nIndex )
        {
            // move all attributes that are behind the cursor
            if ( rAttrib.GetStart() > nIndex )
            {
                rAttrib.MoveForward( nNew );
            }
            // 0: expand empty attribute, if at cursor
            else if ( rAttrib.IsEmpty() )
            {
                // Do not check the index; empty one may only be here.
                // If checking later anyway, special case:
                // Start == 0; AbsLen == 1, nNew = 1 => Expand due to new paragraph!
                // Start <= nIndex, End >= nIndex => Start=End=nIndex!
                rAttrib.Expand( nNew );
            }
            // 1: attribute starts before and reaches up to index
            else if ( rAttrib.GetEnd() == nIndex ) // start must be before
            {
                // Only expand if no feature and not in Exclude list!
                // Otherwise e.g. an UL would go until the new ULDB, thus expand both.
                if ( !maCharAttribs.FindEmptyAttrib( rAttrib.Which(), nIndex ) )
                {
                    rAttrib.Expand( nNew );
                }
                else
                    bResort = true;
            }
            // 2: attribute starts before and reaches past the index
            else if ( ( rAttrib.GetStart() < nIndex ) && ( rAttrib.GetEnd() > nIndex ) )
            {
                rAttrib.Expand( nNew );
            }
            // 3: attribute starts at Index
            else if ( rAttrib.GetStart() == nIndex )
            {
                if ( nIndex == 0 )
                {
                    rAttrib.Expand( nNew );
                }
                else
                    rAttrib.MoveForward( nNew );
            }
        }

        SAL_WARN_IF( rAttrib.GetStart() > rAttrib.GetEnd(), "vcl", "Expand: attribute twisted!" );
        SAL_WARN_IF( ( rAttrib.GetEnd() > maText.getLength() ), "vcl", "Expand: attribute greater than paragraph!" );
        SAL_WARN_IF( rAttrib.IsEmpty(), "vcl", "Empty attribute after ExpandAttribs?" );
    }

    if ( bResort )
        maCharAttribs.ResortAttribs();
}

void TextNode::CollapseAttribs( sal_Int32 nIndex, sal_Int32 nDeleted )
{
    if ( !nDeleted )
        return;

    bool bResort = false;
    const sal_Int32 nEndChanges = nIndex+nDeleted;

    for ( sal_uInt16 nAttr = 0; nAttr < maCharAttribs.Count(); nAttr++ )
    {
        TextCharAttrib& rAttrib = maCharAttribs.GetAttrib( nAttr );
        bool bDelAttr = false;
        if ( rAttrib.GetEnd() >= nIndex )
        {
            // move all attributes that are behind the cursor
            if ( rAttrib.GetStart() >= nEndChanges )
            {
                rAttrib.MoveBackward( nDeleted );
            }
            // 1. delete inner attributes
            else if ( ( rAttrib.GetStart() >= nIndex ) && ( rAttrib.GetEnd() <= nEndChanges ) )
            {
                // special case: attribute covers the region exactly
                // => keep as an empty attribute
                if ( ( rAttrib.GetStart() == nIndex ) && ( rAttrib.GetEnd() == nEndChanges ) )
                    rAttrib.SetEnd(nIndex); // empty
                else
                    bDelAttr = true;
            }
            // 2. attribute starts before, ends inside or after
            else if ( ( rAttrib.GetStart() <= nIndex ) && ( rAttrib.GetEnd() > nIndex ) )
            {
                if ( rAttrib.GetEnd() <= nEndChanges ) // ends inside
                    rAttrib.SetEnd(nIndex);
                else
                    rAttrib.Collaps( nDeleted );       // ends after
            }
            // 3. attribute starts inside, ends after
            else if ( ( rAttrib.GetStart() >= nIndex ) && ( rAttrib.GetEnd() > nEndChanges ) )
            {
                // features are not allowed to expand!
                rAttrib.SetStart(nEndChanges);
                rAttrib.MoveBackward( nDeleted );
            }
        }

        SAL_WARN_IF( rAttrib.GetStart() > rAttrib.GetEnd(), "vcl", "Collaps: attribute twisted!" );
        SAL_WARN_IF( ( rAttrib.GetEnd() > maText.getLength()) && !bDelAttr, "vcl", "Collaps: attribute greater than paragraph!" );
        if ( bDelAttr /* || rAttrib.IsEmpty() */ )
        {
            bResort = true;
            maCharAttribs.RemoveAttrib( nAttr );
            nAttr--;
        }
        else if ( rAttrib.IsEmpty() )
            maCharAttribs.HasEmptyAttribs() = true;
    }

    if ( bResort )
        maCharAttribs.ResortAttribs();
}

void TextNode::InsertText( sal_Int32 nPos, const OUString& rText )
{
    maText = maText.replaceAt( nPos, 0, rText );
    ExpandAttribs( nPos, rText.getLength() );
}

void TextNode::InsertText( sal_Int32 nPos, sal_Unicode c )
{
    maText = maText.replaceAt( nPos, 0, OUString(c) );
    ExpandAttribs( nPos, 1 );
}

void TextNode::RemoveText( sal_Int32 nPos, sal_Int32 nChars )
{
    maText = maText.replaceAt( nPos, nChars, "" );
    CollapseAttribs( nPos, nChars );
}

std::unique_ptr<TextNode> TextNode::Split( sal_Int32 nPos )
{
    OUString aNewText;
    if ( nPos < maText.getLength() )
    {
        aNewText = maText.copy( nPos );
        maText = maText.copy(0, nPos);
    }
    std::unique_ptr<TextNode> pNew(new TextNode( aNewText ));

    for ( sal_uInt16 nAttr = 0; nAttr < maCharAttribs.Count(); nAttr++ )
    {
        TextCharAttrib& rAttrib = maCharAttribs.GetAttrib( nAttr );
        if ( rAttrib.GetEnd() < nPos )
        {
            // no change
            ;
        }
        else if ( rAttrib.GetEnd() == nPos )
        {
            // must be copied as an empty attribute
            // !FindAttrib only sensible if traversing backwards through the list!
            if ( !pNew->maCharAttribs.FindAttrib( rAttrib.Which(), 0 ) )
            {
                std::unique_ptr<TextCharAttrib> pNewAttrib(new TextCharAttrib( rAttrib ));
                pNewAttrib->SetStart(0);
                pNewAttrib->SetEnd(0);
                pNew->maCharAttribs.InsertAttrib( std::move(pNewAttrib) );
            }
        }
        else if ( rAttrib.IsInside( nPos ) || ( !nPos && !rAttrib.GetStart() ) )
        {
            // If cutting at the very beginning, the attribute has to be
            // copied and changed
            std::unique_ptr<TextCharAttrib> pNewAttrib(new TextCharAttrib( rAttrib ));
            pNewAttrib->SetStart(0);
            pNewAttrib->SetEnd(rAttrib.GetEnd()-nPos);
            pNew->maCharAttribs.InsertAttrib( std::move(pNewAttrib) );
            // trim
            rAttrib.SetEnd(nPos);
        }
        else
        {
            SAL_WARN_IF( rAttrib.GetStart() < nPos, "vcl", "Start < nPos!" );
            SAL_WARN_IF( rAttrib.GetEnd() < nPos, "vcl", "End < nPos!" );
            // move all into the new node (this)
            pNew->maCharAttribs.InsertAttrib(maCharAttribs.RemoveAttrib(nAttr));
            rAttrib.SetStart( rAttrib.GetStart() - nPos );
            rAttrib.SetEnd( rAttrib.GetEnd() - nPos );
            nAttr--;
        }
    }
    return pNew;
}

void TextNode::Append( const TextNode& rNode )
{
    sal_Int32 nOldLen = maText.getLength();

    maText += rNode.GetText();

    const sal_uInt16 nAttribs = rNode.GetCharAttribs().Count();
    for ( sal_uInt16 nAttr = 0; nAttr < nAttribs; nAttr++ )
    {
        const TextCharAttrib& rAttrib = rNode.GetCharAttrib( nAttr );
        bool bMelted = false;
        if ( rAttrib.GetStart() == 0 )
        {
            // potentially merge attributes
            sal_uInt16 nTmpAttribs = maCharAttribs.Count();
            for ( sal_uInt16 nTmpAttr = 0; nTmpAttr < nTmpAttribs; nTmpAttr++ )
            {
                TextCharAttrib& rTmpAttrib = maCharAttribs.GetAttrib( nTmpAttr );

                if ( rTmpAttrib.GetEnd() == nOldLen )
                {
                    if ( ( rTmpAttrib.Which() == rAttrib.Which() ) &&
                         ( rTmpAttrib.GetAttr() == rAttrib.GetAttr() ) )
                    {
                        rTmpAttrib.SetEnd( rTmpAttrib.GetEnd() + rAttrib.GetLen() );
                        bMelted = true;
                        break;  // there can be only one of this type at this position
                    }
                }
            }
        }

        if ( !bMelted )
        {
            std::unique_ptr<TextCharAttrib> pNewAttrib(new TextCharAttrib( rAttrib ));
            pNewAttrib->SetStart( pNewAttrib->GetStart() + nOldLen );
            pNewAttrib->SetEnd( pNewAttrib->GetEnd() + nOldLen );
            maCharAttribs.InsertAttrib( std::move(pNewAttrib) );
        }
    }
}

TextDoc::TextDoc()
    : mnLeftMargin(0)
{
};

TextDoc::~TextDoc()
{
    DestroyTextNodes();
}

void TextDoc::Clear()
{
    DestroyTextNodes();
}

void TextDoc::DestroyTextNodes()
{
    maTextNodes.clear();
}

OUString TextDoc::GetText( const sal_Unicode* pSep ) const
{
    sal_uInt32 nNodes = static_cast<sal_uInt32>(maTextNodes.size());

    OUStringBuffer aASCIIText;
    const sal_uInt32 nLastNode = nNodes-1;
    for ( sal_uInt32 nNode = 0; nNode < nNodes; ++nNode )
    {
        TextNode* pNode = maTextNodes[ nNode ].get();
        aASCIIText.append(pNode->GetText());
        if ( pSep && ( nNode != nLastNode ) )
            aASCIIText.append(pSep);
    }

    return aASCIIText.makeStringAndClear();
}

OUString TextDoc::GetText( sal_uInt32 nPara ) const
{
    TextNode* pNode = ( nPara < maTextNodes.size() ) ? maTextNodes[ nPara ].get() : nullptr;
    if ( pNode )
        return pNode->GetText();

    return OUString();
}

sal_Int32 TextDoc::GetTextLen( const sal_Unicode* pSep, const TextSelection* pSel ) const
{
    sal_Int32 nLen = 0;
    sal_uInt32 nNodes = static_cast<sal_uInt32>(maTextNodes.size());
    if ( nNodes )
    {
        sal_uInt32 nStartNode = 0;
        sal_uInt32 nEndNode = nNodes-1;
        if ( pSel )
        {
            nStartNode = pSel->GetStart().GetPara();
            nEndNode = pSel->GetEnd().GetPara();
        }

        for ( sal_uInt32 nNode = nStartNode; nNode <= nEndNode; ++nNode )
        {
            TextNode* pNode = maTextNodes[ nNode ].get();

            sal_Int32 nS = 0;
            sal_Int32 nE = pNode->GetText().getLength();
            if ( pSel && ( nNode == pSel->GetStart().GetPara() ) )
                nS = pSel->GetStart().GetIndex();
            if ( pSel && ( nNode == pSel->GetEnd().GetPara() ) )
                nE = pSel->GetEnd().GetIndex();

            nLen += ( nE - nS );
        }

        if ( pSep )
            nLen += (nEndNode-nStartNode) * rtl_ustr_getLength(pSep);
    }

    return nLen;
}

TextPaM TextDoc::InsertText( const TextPaM& rPaM, sal_Unicode c )
{
    SAL_WARN_IF( c == 0x0A, "vcl", "TextDoc::InsertText: Line separator in paragraph not allowed!" );
    SAL_WARN_IF( c == 0x0D, "vcl", "TextDoc::InsertText: Line separator in paragraph not allowed!" );

    TextNode* pNode = maTextNodes[ rPaM.GetPara() ].get();
    pNode->InsertText( rPaM.GetIndex(), c );

    TextPaM aPaM( rPaM.GetPara(), rPaM.GetIndex()+1 );
    return aPaM;
}

TextPaM TextDoc::InsertText( const TextPaM& rPaM, const OUString& rStr )
{
    SAL_WARN_IF( rStr.indexOf( 0x0A ) != -1, "vcl", "TextDoc::InsertText: Line separator in paragraph not allowed!" );
    SAL_WARN_IF( rStr.indexOf( 0x0D ) != -1, "vcl", "TextDoc::InsertText: Line separator in paragraph not allowed!" );

    TextNode* pNode = maTextNodes[ rPaM.GetPara() ].get();
    pNode->InsertText( rPaM.GetIndex(), rStr );

    TextPaM aPaM( rPaM.GetPara(), rPaM.GetIndex()+rStr.getLength() );
    return aPaM;
}

TextPaM TextDoc::InsertParaBreak( const TextPaM& rPaM )
{
    TextNode* pNode = maTextNodes[ rPaM.GetPara() ].get();
    std::unique_ptr<TextNode> pNew = pNode->Split( rPaM.GetIndex() );

    SAL_WARN_IF( maTextNodes.size()>=SAL_MAX_UINT32, "vcl", "InsertParaBreak: more than 4Gi paragraphs!" );
    maTextNodes.insert( maTextNodes.begin() + rPaM.GetPara() + 1, std::move(pNew) );

    TextPaM aPaM( rPaM.GetPara()+1, 0 );
    return aPaM;
}

TextPaM TextDoc::ConnectParagraphs( TextNode* pLeft, const TextNode* pRight )
{
    sal_Int32 nPrevLen = pLeft->GetText().getLength();
    pLeft->Append( *pRight );

    // the paragraph on the right vanishes
    maTextNodes.erase( std::find_if( maTextNodes.begin(), maTextNodes.end(),
                                     [&] (std::unique_ptr<TextNode> const & p) { return p.get() == pRight; } ) );

    sal_uLong nLeft = ::std::find_if( maTextNodes.begin(), maTextNodes.end(),
                                      [&] (std::unique_ptr<TextNode> const & p) { return p.get() == pLeft; } )
                        - maTextNodes.begin();
    TextPaM aPaM( nLeft, nPrevLen );
    return aPaM;
}

void TextDoc::RemoveChars( const TextPaM& rPaM, sal_Int32 nChars )
{
    TextNode* pNode = maTextNodes[ rPaM.GetPara() ].get();
    pNode->RemoveText( rPaM.GetIndex(), nChars );
}

bool TextDoc::IsValidPaM( const TextPaM& rPaM )
{
    if ( rPaM.GetPara() >= maTextNodes.size() )
    {
        OSL_FAIL( "PaM: Para out of range" );
        return false;
    }
    TextNode * pNode = maTextNodes[ rPaM.GetPara() ].get();
    if ( rPaM.GetIndex() > pNode->GetText().getLength() )
    {
        OSL_FAIL( "PaM: Index out of range" );
        return false;
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
