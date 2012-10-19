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

#include <textdoc.hxx>

#include <stdlib.h>



// Vergleichmethode wird von QuickSort gerufen...

static bool CompareStart( const TextCharAttrib* pFirst, const TextCharAttrib* pSecond )
{
    return pFirst->GetStart() < pSecond->GetStart();
}

// -------------------------------------------------------------------------
// (+) class TextCharAttrib
// -------------------------------------------------------------------------
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

// -------------------------------------------------------------------------
// (+) class TextCharAttribList
// -------------------------------------------------------------------------

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
    const sal_uInt16 nStart = pAttrib->GetStart(); // vielleicht besser fuer Comp.Opt.
    sal_Bool bInserted = sal_False;
    for ( sal_uInt16 x = 0; x < nCount; x++ )
    {
        TextCharAttrib* pCurAttrib = GetAttrib( x );
        if ( pCurAttrib->GetStart() > nStart )
        {
            insert( begin() + x, pAttrib );
            bInserted = sal_True;
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
    // Rueckwaerts, falls eins dort endet, das naechste startet.
    // => Das startende gilt...

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
    // Rueckwaerts, falls eins dort endet, das naechste startet.
    // => Das startende gilt...
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

// -------------------------------------------------------------------------
// (+) class TextNode
// -------------------------------------------------------------------------

TextNode::TextNode( const String& rText ) :
    maText( rText )
{
}

void TextNode::ExpandAttribs( sal_uInt16 nIndex, sal_uInt16 nNew )
{
    if ( !nNew )
        return;

    sal_Bool bResort = sal_False;
    sal_uInt16 nAttribs = maCharAttribs.Count();
    for ( sal_uInt16 nAttr = 0; nAttr < nAttribs; nAttr++ )
    {
        TextCharAttrib* pAttrib = maCharAttribs.GetAttrib( nAttr );
        if ( pAttrib->GetEnd() >= nIndex )
        {
            // Alle Attribute hinter der Einfuegeposition verschieben...
            if ( pAttrib->GetStart() > nIndex )
            {
                pAttrib->MoveForward( nNew );
            }
            // 0: Leeres Attribut expandieren, wenn an Einfuegestelle
            else if ( pAttrib->IsEmpty() )
            {
                // Index nicht pruefen, leeres durfte nur dort liegen.
                // Wenn spaeter doch Ueberpruefung:
                //   Spezialfall: Start == 0; AbsLen == 1, nNew = 1 => Expand, weil durch Absatzumbruch!
                // Start <= nIndex, End >= nIndex => Start=End=nIndex!
//              if ( pAttrib->GetStart() == nIndex )
                    pAttrib->Expand( nNew );
            }
            // 1: Attribut startet davor, geht bis Index...
            else if ( pAttrib->GetEnd() == nIndex ) // Start muss davor liegen
            {
                // Nur expandieren, wenn kein Feature,
                // und wenn nicht in ExcludeListe!
                // Sonst geht z.B. ein UL bis zum neuen ULDB, beide expandieren
                if ( !maCharAttribs.FindEmptyAttrib( pAttrib->Which(), nIndex ) )
                {
                    pAttrib->Expand( nNew );
                }
                else
                    bResort = sal_True;
            }
            // 2: Attribut startet davor, geht hinter Index...
            else if ( ( pAttrib->GetStart() < nIndex ) && ( pAttrib->GetEnd() > nIndex ) )
            {
                pAttrib->Expand( nNew );
            }
            // 3: Attribut startet auf Index...
            else if ( pAttrib->GetStart() == nIndex )
            {
                if ( nIndex == 0 )
                {
                    pAttrib->Expand( nNew );
//                  bResort = sal_True;     // es gibt ja keine Features mehr...
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

    sal_Bool bResort = sal_False;
    sal_uInt16 nEndChanges = nIndex+nDeleted;

    for ( sal_uInt16 nAttr = 0; nAttr < maCharAttribs.Count(); nAttr++ )
    {
        TextCharAttrib* pAttrib = maCharAttribs.GetAttrib( nAttr );
        sal_Bool bDelAttr = sal_False;
        if ( pAttrib->GetEnd() >= nIndex )
        {
            // Alles Attribute hinter der Einfuegeposition verschieben...
            if ( pAttrib->GetStart() >= nEndChanges )
            {
                pAttrib->MoveBackward( nDeleted );
            }
            // 1. Innenliegende Attribute loeschen...
            else if ( ( pAttrib->GetStart() >= nIndex ) && ( pAttrib->GetEnd() <= nEndChanges ) )
            {
                // Spezialfall: Attrubt deckt genau den Bereich ab
                // => als leeres Attribut behalten.
                if ( ( pAttrib->GetStart() == nIndex ) && ( pAttrib->GetEnd() == nEndChanges ) )
                    pAttrib->GetEnd() = nIndex; // leer
                else
                    bDelAttr = sal_True;
            }
            // 2. Attribut beginnt davor, endet drinnen oder dahinter...
            else if ( ( pAttrib->GetStart() <= nIndex ) && ( pAttrib->GetEnd() > nIndex ) )
            {
                if ( pAttrib->GetEnd() <= nEndChanges ) // endet drinnen
                    pAttrib->GetEnd() = nIndex;
                else
                    pAttrib->Collaps( nDeleted );       // endet dahinter
            }
            // 3. Attribut beginnt drinnen, endet dahinter...
            else if ( ( pAttrib->GetStart() >= nIndex ) && ( pAttrib->GetEnd() > nEndChanges ) )
            {
                // Features duerfen nicht expandieren!
                pAttrib->GetStart() = nEndChanges;
                pAttrib->MoveBackward( nDeleted );
            }
        }

        DBG_ASSERT( pAttrib->GetStart() <= pAttrib->GetEnd(), "Collaps: Attribut verdreht!" );
        DBG_ASSERT( ( pAttrib->GetEnd() <= maText.Len()) || bDelAttr, "Collaps: Attrib groesser als Absatz!" );
        if ( bDelAttr /* || pAttrib->IsEmpty() */ )
        {
            bResort = sal_True;
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
            // bleiben unveraendert....
            ;
        }
        else if ( pAttrib->GetEnd() == nPos )
        {
            // muessen als leeres Attribut kopiert werden.
            // !FindAttrib nur sinnvoll, wenn Rueckwaerts durch Liste!
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
            // Wenn ganz vorne gecuttet wird, muss das Attribut erhalten bleiben!
            // muessen kopiert und geaendert werden
            TextCharAttrib* pNewAttrib = new TextCharAttrib( *pAttrib );
            pNewAttrib->GetStart() = 0;
            pNewAttrib->GetEnd() = pAttrib->GetEnd()-nPos;
            pNew->maCharAttribs.InsertAttrib( pNewAttrib );
            // stutzen:
            pAttrib->GetEnd() = nPos;
        }
        else
        {
            DBG_ASSERT( pAttrib->GetStart() >= nPos, "Start < nPos!" );
            DBG_ASSERT( pAttrib->GetEnd() >= nPos, "End < nPos!" );
            // alle dahinter verschieben in den neuen Node (this)
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
        sal_Bool bMelted = sal_False;
        if ( pAttrib->GetStart() == 0 )
        {
            // Evtl koennen Attribute zusammengefasst werden:
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
                        bMelted = sal_True;
                        break;  // es kann nur eins von der Sorte an der Stelle geben
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

// -------------------------------------------------------------------------
// (+) class TextDoc
// -------------------------------------------------------------------------

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

String TextDoc::GetText( const sal_Unicode* pSep ) const
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

XubString TextDoc::GetText( sal_uLong nPara ) const
{
    XubString aText;
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

TextPaM TextDoc::InsertText( const TextPaM& rPaM, const XubString& rStr )
{
    DBG_ASSERT( rStr.Search( 0x0A ) == STRING_NOTFOUND, "TextDoc::InsertText: Zeilentrenner in Absatz nicht erlaubt!" );
    DBG_ASSERT( rStr.Search( 0x0D ) == STRING_NOTFOUND, "TextDoc::InsertText: Zeilentrenner in Absatz nicht erlaubt!" );

    TextNode* pNode = maTextNodes.GetObject( rPaM.GetPara() );
    pNode->InsertText( rPaM.GetIndex(), rStr );

    TextPaM aPaM( rPaM.GetPara(), rPaM.GetIndex()+rStr.Len() );
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

    // der rechte verschwindet.
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
