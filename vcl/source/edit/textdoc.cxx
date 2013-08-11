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

#include <textdoc.hxx>

#include <stdlib.h>


// compare function called by QuickSort
static bool CompareStart( const TextCharAttrib* pFirst, const TextCharAttrib* pSecond )
{
    return pFirst->GetStart() < pSecond->GetStart();
}

TextCharAttrib::TextCharAttrib( const TextAttrib& rAttr, sal_uInt16 nStart, sal_uInt16 nEnd )
{
    mpAttr = rAttr.Clone();
    mnStart = nStart,
    mnEnd = nEnd;
}

TextCharAttrib::TextCharAttrib( const TextCharAttrib& rTextCharAttrib )
{
    mpAttr = rTextCharAttrib.GetAttr().Clone();
    mnStart = rTextCharAttrib.mnStart;
    mnEnd = rTextCharAttrib.mnEnd;
}

TextCharAttrib::~TextCharAttrib()
{
    delete mpAttr;
}

TextCharAttribList::TextCharAttribList()
{
    mbHasEmptyAttribs = sal_False;
}

TextCharAttribList::~TextCharAttribList()
{
    // PTRARR_DEL
}

void TextCharAttribList::Clear( sal_Bool bDestroyAttribs )
{
    if ( bDestroyAttribs )
        for(iterator it = begin(); it != end(); ++it)
            delete *it;
    TextCharAttribs::clear();
}


void TextCharAttribList::InsertAttrib( TextCharAttrib* pAttrib )
{
    if ( pAttrib->IsEmpty() )
        mbHasEmptyAttribs = sal_True;

    const sal_uInt16 nCount = size();
    const sal_uInt16 nStart = pAttrib->GetStart(); // maybe better for Comp.Opt.
    bool bInserted = false;
    for ( sal_uInt16 x = 0; x < nCount; x++ )
    {
        TextCharAttrib* pCurAttrib = GetAttrib( x );
        if ( pCurAttrib->GetStart() > nStart )
        {
            insert( begin() + x, pAttrib );
            bInserted = true;
            break;
        }
    }
    if ( !bInserted )
        push_back( pAttrib );
}

void TextCharAttribList::ResortAttribs()
{
    if ( !empty() )
        std::sort( begin(), end(), CompareStart );
}

TextCharAttrib* TextCharAttribList::FindAttrib( sal_uInt16 nWhich, sal_uInt16 nPos )
{
    // backwards; if one ends there and the next starts there
    // ==> the starting one counts
    for ( sal_uInt16 nAttr = size(); nAttr; )
    {
        TextCharAttrib* pAttr = GetAttrib( --nAttr );

        if ( pAttr->GetEnd() < nPos )
            return 0;

        if ( ( pAttr->Which() == nWhich ) && pAttr->IsIn(nPos) )
            return pAttr;
    }
    return NULL;
}

TextCharAttrib* TextCharAttribList::FindNextAttrib( sal_uInt16 nWhich, sal_uInt16 nFromPos, sal_uInt16 nMaxPos ) const
{
    DBG_ASSERT( nWhich, "FindNextAttrib: Which?" );
    const sal_uInt16 nAttribs = size();
    for ( sal_uInt16 nAttr = 0; nAttr < nAttribs; nAttr++ )
    {
        TextCharAttrib* pAttr = GetAttrib( nAttr );
        if ( ( pAttr->GetStart() >= nFromPos ) &&
             ( pAttr->GetEnd() <= nMaxPos ) &&
             ( pAttr->Which() == nWhich ) )
            return pAttr;
    }
    return NULL;
}

sal_Bool TextCharAttribList::HasAttrib( sal_uInt16 nWhich ) const
{
    for ( sal_uInt16 nAttr = size(); nAttr; )
    {
        const TextCharAttrib* pAttr = GetAttrib( --nAttr );
        if ( pAttr->Which() == nWhich )
            return sal_True;
    }
    return sal_False;
}

sal_Bool TextCharAttribList::HasBoundingAttrib( sal_uInt16 nBound )
{
    // backwards; if one ends there and the next starts there
    // ==> the starting one counts
    for ( sal_uInt16 nAttr = size(); nAttr; )
    {
        TextCharAttrib* pAttr = GetAttrib( --nAttr );

        if ( pAttr->GetEnd() < nBound )
            return sal_False;

        if ( ( pAttr->GetStart() == nBound ) || ( pAttr->GetEnd() == nBound ) )
            return sal_True;
    }
    return sal_False;
}

TextCharAttrib* TextCharAttribList::FindEmptyAttrib( sal_uInt16 nWhich, sal_uInt16 nPos )
{
    if ( !mbHasEmptyAttribs )
        return 0;

    const sal_uInt16 nAttribs = size();
    for ( sal_uInt16 nAttr = 0; nAttr < nAttribs; nAttr++ )
    {
        TextCharAttrib* pAttr = GetAttrib( nAttr );
        if ( pAttr->GetStart() > nPos )
            return 0;

        if ( ( pAttr->GetStart() == nPos ) && ( pAttr->GetEnd() == nPos ) && ( pAttr->Which() == nWhich ) )
            return pAttr;
    }
    return 0;
}

void TextCharAttribList::DeleteEmptyAttribs()
{
    for ( sal_uInt16 nAttr = 0; nAttr < size(); nAttr++ )
    {
        TextCharAttrib* pAttr = GetAttrib( nAttr );
        if ( pAttr->IsEmpty() )
        {
            erase( begin() + nAttr );
            delete pAttr;
            nAttr--;
        }
    }
    mbHasEmptyAttribs = sal_False;
}

TextNode::TextNode( const String& rText ) :
    maText( rText )
{
}

void TextNode::ExpandAttribs( sal_uInt16 nIndex, sal_uInt16 nNew )
{
    if ( !nNew )
        return;

    bool bResort = false;
    sal_uInt16 nAttribs = maCharAttribs.Count();
    for ( sal_uInt16 nAttr = 0; nAttr < nAttribs; nAttr++ )
    {
        TextCharAttrib* pAttrib = maCharAttribs.GetAttrib( nAttr );
        if ( pAttrib->GetEnd() >= nIndex )
        {
            // move all attributes that are behind the cursor
            if ( pAttrib->GetStart() > nIndex )
            {
                pAttrib->MoveForward( nNew );
            }
            // 0: expand empty attribute, if at cursor
            else if ( pAttrib->IsEmpty() )
            {
                // Do not check the index; empty one may only be here.
                // If checking later anyway, special case:
                // Start == 0; AbsLen == 1, nNew = 1 => Expand due to new paragraph!
                // Start <= nIndex, End >= nIndex => Start=End=nIndex!
                pAttrib->Expand( nNew );
            }
            // 1: attribute starts before and reaches up to index
            else if ( pAttrib->GetEnd() == nIndex ) // start must be before
            {
                // Only expand if no feature and not in Exclude list!
                // Otherwise e.g. an UL would go until the new ULDB, thus expand both.
                if ( !maCharAttribs.FindEmptyAttrib( pAttrib->Which(), nIndex ) )
                {
                    pAttrib->Expand( nNew );
                }
                else
                    bResort = true;
            }
            // 2: attribute starts before and reaches past the index
            else if ( ( pAttrib->GetStart() < nIndex ) && ( pAttrib->GetEnd() > nIndex ) )
            {
                pAttrib->Expand( nNew );
            }
            // 3: attribute starts at Index
            else if ( pAttrib->GetStart() == nIndex )
            {
                if ( nIndex == 0 )
                {
                    pAttrib->Expand( nNew );
                }
                else
                    pAttrib->MoveForward( nNew );
            }
        }

        DBG_ASSERT( pAttrib->GetStart() <= pAttrib->GetEnd(), "Expand: Attribut verdreht!" );
        DBG_ASSERT( ( pAttrib->GetEnd() <= maText.Len() ), "Expand: Attrib groesser als Absatz!" );
        DBG_ASSERT( !pAttrib->IsEmpty(), "Leeres Attribut nach ExpandAttribs?" );
    }

    if ( bResort )
        maCharAttribs.ResortAttribs();
}

void TextNode::CollapsAttribs( sal_uInt16 nIndex, sal_uInt16 nDeleted )
{
    if ( !nDeleted )
        return;

    bool bResort = false;
    sal_uInt16 nEndChanges = nIndex+nDeleted;

    for ( sal_uInt16 nAttr = 0; nAttr < maCharAttribs.Count(); nAttr++ )
    {
        TextCharAttrib* pAttrib = maCharAttribs.GetAttrib( nAttr );
        bool bDelAttr = false;
        if ( pAttrib->GetEnd() >= nIndex )
        {
            // move all attributes that are behind the cursor
            if ( pAttrib->GetStart() >= nEndChanges )
            {
                pAttrib->MoveBackward( nDeleted );
            }
            // 1. delete inner attributes
            else if ( ( pAttrib->GetStart() >= nIndex ) && ( pAttrib->GetEnd() <= nEndChanges ) )
            {
                // special case: attribute covers the region exactly
                // => keep as an empty attribute
                if ( ( pAttrib->GetStart() == nIndex ) && ( pAttrib->GetEnd() == nEndChanges ) )
                    pAttrib->GetEnd() = nIndex; // empty
                else
                    bDelAttr = true;
            }
            // 2. attribute starts before, ends inside or after
            else if ( ( pAttrib->GetStart() <= nIndex ) && ( pAttrib->GetEnd() > nIndex ) )
            {
                if ( pAttrib->GetEnd() <= nEndChanges ) // ends inside
                    pAttrib->GetEnd() = nIndex;
                else
                    pAttrib->Collaps( nDeleted );       // ends after
            }
            // 3. attribute starts inside, ends after
            else if ( ( pAttrib->GetStart() >= nIndex ) && ( pAttrib->GetEnd() > nEndChanges ) )
            {
                // features are not allowed to expand!
                pAttrib->GetStart() = nEndChanges;
                pAttrib->MoveBackward( nDeleted );
            }
        }

        DBG_ASSERT( pAttrib->GetStart() <= pAttrib->GetEnd(), "Collaps: Attribut verdreht!" );
        DBG_ASSERT( ( pAttrib->GetEnd() <= maText.Len()) || bDelAttr, "Collaps: Attrib groesser als Absatz!" );
        if ( bDelAttr /* || pAttrib->IsEmpty() */ )
        {
            bResort = true;
            maCharAttribs.RemoveAttrib( nAttr );
            delete pAttrib;
            nAttr--;
        }
        else if ( pAttrib->IsEmpty() )
            maCharAttribs.HasEmptyAttribs() = sal_True;
    }

    if ( bResort )
        maCharAttribs.ResortAttribs();
}

void TextNode::InsertText( sal_uInt16 nPos, const String& rText )
{
    maText.Insert( rText, nPos );
    ExpandAttribs( nPos, rText.Len() );
}

void TextNode::InsertText( sal_uInt16 nPos, sal_Unicode c )
{
    maText.Insert( c, nPos );
    ExpandAttribs( nPos, 1 );
}

void TextNode::RemoveText( sal_uInt16 nPos, sal_uInt16 nChars )
{
    maText.Erase( nPos, nChars );
    CollapsAttribs( nPos, nChars );
}

TextNode* TextNode::Split( sal_uInt16 nPos, sal_Bool bKeepEndingAttribs )
{
    String aNewText;
    if ( nPos < maText.Len() )
    {
        aNewText = maText.Copy( nPos );
        maText.Erase( nPos );
    }
    TextNode* pNew = new TextNode( aNewText );

    for ( sal_uInt16 nAttr = 0; nAttr < maCharAttribs.Count(); nAttr++ )
    {
        TextCharAttrib* pAttrib = maCharAttribs.GetAttrib( nAttr );
        if ( pAttrib->GetEnd() < nPos )
        {
            // no change
            ;
        }
        else if ( pAttrib->GetEnd() == nPos )
        {
            // must be copied as an empty attribute
            // !FindAttrib only sensible if traversing backwards through the list!
            if ( bKeepEndingAttribs && !pNew->maCharAttribs.FindAttrib( pAttrib->Which(), 0 ) )
            {
                TextCharAttrib* pNewAttrib = new TextCharAttrib( *pAttrib );
                pNewAttrib->GetStart() = 0;
                pNewAttrib->GetEnd() = 0;
                pNew->maCharAttribs.InsertAttrib( pNewAttrib );
            }
        }
        else if ( pAttrib->IsInside( nPos ) || ( !nPos && !pAttrib->GetStart() ) )
        {
            // If cutting at the very beginning, the attribute has to be
            // copied and changed
            TextCharAttrib* pNewAttrib = new TextCharAttrib( *pAttrib );
            pNewAttrib->GetStart() = 0;
            pNewAttrib->GetEnd() = pAttrib->GetEnd()-nPos;
            pNew->maCharAttribs.InsertAttrib( pNewAttrib );
            // trim
            pAttrib->GetEnd() = nPos;
        }
        else
        {
            DBG_ASSERT( pAttrib->GetStart() >= nPos, "Start < nPos!" );
            DBG_ASSERT( pAttrib->GetEnd() >= nPos, "End < nPos!" );
            // move all into the new node (this)
            maCharAttribs.RemoveAttrib( nAttr );
            pNew->maCharAttribs.InsertAttrib( pAttrib );
            pAttrib->GetStart() = pAttrib->GetStart() - nPos;
            pAttrib->GetEnd() = pAttrib->GetEnd() - nPos;
            nAttr--;
        }
    }
    return pNew;
}

void TextNode::Append( const TextNode& rNode )
{
    sal_uInt16 nOldLen = maText.Len();

    maText += rNode.GetText();

    const sal_uInt16 nAttribs = rNode.GetCharAttribs().Count();
    for ( sal_uInt16 nAttr = 0; nAttr < nAttribs; nAttr++ )
    {
        TextCharAttrib* pAttrib = rNode.GetCharAttribs().GetAttrib( nAttr );
        bool bMelted = false;
        if ( pAttrib->GetStart() == 0 )
        {
            // potentially merge attributes
            sal_uInt16 nTmpAttribs = maCharAttribs.Count();
            for ( sal_uInt16 nTmpAttr = 0; nTmpAttr < nTmpAttribs; nTmpAttr++ )
            {
                TextCharAttrib* pTmpAttrib = maCharAttribs.GetAttrib( nTmpAttr );

                if ( pTmpAttrib->GetEnd() == nOldLen )
                {
                    if ( ( pTmpAttrib->Which() == pAttrib->Which() ) &&
                         ( pTmpAttrib->GetAttr() == pAttrib->GetAttr() ) )
                    {
                        pTmpAttrib->GetEnd() =
                            pTmpAttrib->GetEnd() + pAttrib->GetLen();
                        bMelted = true;
                        break;  // there can be only one of this type at this position
                    }
                }
            }
        }

        if ( !bMelted )
        {
            TextCharAttrib* pNewAttrib = new TextCharAttrib( *pAttrib );
            pNewAttrib->GetStart() = pNewAttrib->GetStart() + nOldLen;
            pNewAttrib->GetEnd() = pNewAttrib->GetEnd() + nOldLen;
            maCharAttribs.InsertAttrib( pNewAttrib );
        }
    }
}

TextDoc::TextDoc()
{
    mnLeftMargin = 0;
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
    for ( sal_uLong nNode = 0; nNode < maTextNodes.Count(); nNode++ )
        delete maTextNodes.GetObject( nNode );
    maTextNodes.clear();
}

OUString TextDoc::GetText( const sal_Unicode* pSep ) const
{
    sal_uLong nLen = GetTextLen( pSep );
    sal_uLong nNodes = maTextNodes.Count();

    if ( nLen > STRING_MAXLEN )
    {
        OSL_FAIL( "Text zu gross fuer String" );
        return String();
    }

    String aASCIIText;
    sal_uLong nLastNode = nNodes-1;
    for ( sal_uLong nNode = 0; nNode < nNodes; nNode++ )
    {
        TextNode* pNode = maTextNodes.GetObject( nNode );
        String aTmp( pNode->GetText() );
        aASCIIText += aTmp;
        if ( pSep && ( nNode != nLastNode ) )
            aASCIIText += pSep;
    }

    return aASCIIText;
}

OUString TextDoc::GetText( sal_uLong nPara ) const
{
    OUString aText;

    TextNode* pNode = ( nPara < maTextNodes.Count() ) ? maTextNodes.GetObject( nPara ) : 0;
    if ( pNode )
        aText = pNode->GetText();

    return aText;
}


sal_uLong TextDoc::GetTextLen( const sal_Unicode* pSep, const TextSelection* pSel ) const
{
    sal_uLong nLen = 0;
    sal_uLong nNodes = maTextNodes.Count();
    if ( nNodes )
    {
        sal_uLong nStartNode = 0;
        sal_uLong nEndNode = nNodes-1;
        if ( pSel )
        {
            nStartNode = pSel->GetStart().GetPara();
            nEndNode = pSel->GetEnd().GetPara();
        }

        for ( sal_uLong nNode = nStartNode; nNode <= nEndNode; nNode++ )
        {
            TextNode* pNode = maTextNodes.GetObject( nNode );

            sal_uInt16 nS = 0;
            sal_uLong nE = pNode->GetText().Len();
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
    DBG_ASSERT( c != 0x0A, "TextDoc::InsertText: Zeilentrenner in Absatz nicht erlaubt!" );
    DBG_ASSERT( c != 0x0D, "TextDoc::InsertText: Zeilentrenner in Absatz nicht erlaubt!" );

    TextNode* pNode = maTextNodes.GetObject( rPaM.GetPara() );
    pNode->InsertText( rPaM.GetIndex(), c );

    TextPaM aPaM( rPaM.GetPara(), rPaM.GetIndex()+1 );
    return aPaM;
}

TextPaM TextDoc::InsertText( const TextPaM& rPaM, const OUString& rStr )
{
    DBG_ASSERT( rStr.indexOf( 0x0A ) == -1, "TextDoc::InsertText: Zeilentrenner in Absatz nicht erlaubt!" );
    DBG_ASSERT( rStr.indexOf( 0x0D ) == -1, "TextDoc::InsertText: Zeilentrenner in Absatz nicht erlaubt!" );

    TextNode* pNode = maTextNodes.GetObject( rPaM.GetPara() );
    pNode->InsertText( rPaM.GetIndex(), rStr );

    TextPaM aPaM( rPaM.GetPara(), rPaM.GetIndex()+rStr.getLength() );
    return aPaM;
}

TextPaM TextDoc::InsertParaBreak( const TextPaM& rPaM, sal_Bool bKeepEndingAttribs )
{
    TextNode* pNode = maTextNodes.GetObject( rPaM.GetPara() );
    TextNode* pNew = pNode->Split( rPaM.GetIndex(), bKeepEndingAttribs );

    maTextNodes.Insert( pNew, rPaM.GetPara()+1 );

    TextPaM aPaM( rPaM.GetPara()+1, 0 );
    return aPaM;
}

TextPaM TextDoc::ConnectParagraphs( TextNode* pLeft, TextNode* pRight )
{
    sal_uInt16 nPrevLen = pLeft->GetText().Len();
    pLeft->Append( *pRight );

    // the paragraph on the right vanishes
    sal_uLong nRight = maTextNodes.GetPos( pRight );
    maTextNodes.Remove( nRight );
    delete pRight;

    sal_uLong nLeft = maTextNodes.GetPos( pLeft );
    TextPaM aPaM( nLeft, nPrevLen );
    return aPaM;
}

TextPaM TextDoc::RemoveChars( const TextPaM& rPaM, sal_uInt16 nChars )
{
    TextNode* pNode = maTextNodes.GetObject( rPaM.GetPara() );
    pNode->RemoveText( rPaM.GetIndex(), nChars );

    return rPaM;
}

sal_Bool TextDoc::IsValidPaM( const TextPaM& rPaM )
{
    if ( rPaM.GetPara() >= maTextNodes.Count() )
    {
        OSL_FAIL( "PaM: Para out of range" );
        return sal_False;
    }
    TextNode * pNode = maTextNodes.GetObject( rPaM.GetPara() );
    if ( rPaM.GetIndex() > pNode->GetText().Len() )
    {
        OSL_FAIL( "PaM: Index out of range" );
        return sal_False;
    }
    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
