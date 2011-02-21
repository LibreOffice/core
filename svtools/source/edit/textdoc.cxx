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
#include "precompiled_svtools.hxx"

#include <textdoc.hxx>

#include <stdlib.h>

SV_IMPL_PTRARR( TextCharAttribs, TextCharAttribPtr );



// Vergleichmethode wird von QuickSort gerufen...

EXTERN_C
#if defined( PM2 )
int _stdcall
#else
#ifdef WNT
#if _MSC_VER >= 1200
int __cdecl
#else
int _cdecl
#endif
#else
int
#endif
#endif

CompareStart( const void* pFirst, const void* pSecond )
{
    if ( (*((TextCharAttrib**)pFirst))->GetStart() < (*((TextCharAttrib**)pSecond))->GetStart() )
        return (-1);
    else if ( (*((TextCharAttrib**)pFirst))->GetStart() > (*((TextCharAttrib**)pSecond))->GetStart() )
        return (1);
    return 0;
}


// -------------------------------------------------------------------------
// (+) class TextCharAttrib
// -------------------------------------------------------------------------
TextCharAttrib::TextCharAttrib( const TextAttrib& rAttr, USHORT nStart, USHORT nEnd )
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
    mbHasEmptyAttribs = FALSE;
}

TextCharAttribList::~TextCharAttribList()
{
    // PTRARR_DEL
}

void TextCharAttribList::Clear( BOOL bDestroyAttribs )
{
    if ( bDestroyAttribs )
        TextCharAttribs::DeleteAndDestroy( 0, Count() );
    else
        TextCharAttribs::Remove( 0, Count() );
}


void TextCharAttribList::InsertAttrib( TextCharAttrib* pAttrib )
{
    if ( pAttrib->IsEmpty() )
        mbHasEmptyAttribs = TRUE;

    const USHORT nCount = Count();
    const USHORT nStart = pAttrib->GetStart(); // vielleicht besser fuer Comp.Opt.
    BOOL bInserted = FALSE;
    for ( USHORT x = 0; x < nCount; x++ )
    {
        TextCharAttrib* pCurAttrib = GetObject( x );
        if ( pCurAttrib->GetStart() > nStart )
        {
            Insert( pAttrib, x );
            bInserted = TRUE;
            break;
        }
    }
    if ( !bInserted )
        Insert( pAttrib, nCount );
}

void TextCharAttribList::ResortAttribs()
{
    if ( Count() )
        qsort( (void*)GetData(), Count(), sizeof( TextCharAttrib* ), CompareStart );
}

TextCharAttrib* TextCharAttribList::FindAttrib( USHORT nWhich, USHORT nPos )
{
    // Rueckwaerts, falls eins dort endet, das naechste startet.
    // => Das startende gilt...

    for ( USHORT nAttr = Count(); nAttr; )
    {
        TextCharAttrib* pAttr = GetObject( --nAttr );

        if ( pAttr->GetEnd() < nPos )
            return 0;

        if ( ( pAttr->Which() == nWhich ) && pAttr->IsIn(nPos) )
            return pAttr;
    }
    return NULL;
}

TextCharAttrib* TextCharAttribList::FindNextAttrib( USHORT nWhich, USHORT nFromPos, USHORT nMaxPos ) const
{
    DBG_ASSERT( nWhich, "FindNextAttrib: Which?" );
    const USHORT nAttribs = Count();
    for ( USHORT nAttr = 0; nAttr < nAttribs; nAttr++ )
    {
        TextCharAttrib* pAttr = GetObject( nAttr );
        if ( ( pAttr->GetStart() >= nFromPos ) &&
             ( pAttr->GetEnd() <= nMaxPos ) &&
             ( pAttr->Which() == nWhich ) )
            return pAttr;
    }
    return NULL;
}

BOOL TextCharAttribList::HasAttrib( USHORT nWhich ) const
{
    for ( USHORT nAttr = Count(); nAttr; )
    {
        const TextCharAttrib* pAttr = GetObject( --nAttr );
        if ( pAttr->Which() == nWhich )
            return TRUE;
    }
    return FALSE;
}

BOOL TextCharAttribList::HasBoundingAttrib( USHORT nBound )
{
    // Rueckwaerts, falls eins dort endet, das naechste startet.
    // => Das startende gilt...
    for ( USHORT nAttr = Count(); nAttr; )
    {
        TextCharAttrib* pAttr = GetObject( --nAttr );

        if ( pAttr->GetEnd() < nBound )
            return FALSE;

        if ( ( pAttr->GetStart() == nBound ) || ( pAttr->GetEnd() == nBound ) )
            return TRUE;
    }
    return FALSE;
}

TextCharAttrib* TextCharAttribList::FindEmptyAttrib( USHORT nWhich, USHORT nPos )
{
    if ( !mbHasEmptyAttribs )
        return 0;

    const USHORT nAttribs = Count();
    for ( USHORT nAttr = 0; nAttr < nAttribs; nAttr++ )
    {
        TextCharAttrib* pAttr = GetObject( nAttr );
        if ( pAttr->GetStart() > nPos )
            return 0;

        if ( ( pAttr->GetStart() == nPos ) && ( pAttr->GetEnd() == nPos ) && ( pAttr->Which() == nWhich ) )
            return pAttr;
    }
    return 0;
}

void TextCharAttribList::DeleteEmptyAttribs()
{
    for ( USHORT nAttr = 0; nAttr < Count(); nAttr++ )
    {
        TextCharAttrib* pAttr = GetObject( nAttr );
        if ( pAttr->IsEmpty() )
        {
            Remove( nAttr );
            delete pAttr;
            nAttr--;
        }
    }
    mbHasEmptyAttribs = FALSE;
}

#ifdef  DBG_UTIL
BOOL TextCharAttribList::DbgCheckAttribs()
{
    BOOL bOK = TRUE;
    for ( USHORT nAttr = 0; nAttr < Count(); nAttr++ )
    {
        TextCharAttrib* pAttr = GetObject( nAttr );
        if ( pAttr->GetStart() > pAttr->GetEnd() )
        {
            bOK = FALSE;
            DBG_ERROR( "Attr verdreht" );
        }
    }
    return bOK;
}
#endif

// -------------------------------------------------------------------------
// (+) class TextNode
// -------------------------------------------------------------------------

TextNode::TextNode( const String& rText ) :
    maText( rText )
{
}

void TextNode::ExpandAttribs( USHORT nIndex, USHORT nNew )
{
    if ( !nNew )
        return;

    BOOL bResort = FALSE;
    USHORT nAttribs = maCharAttribs.Count();
    for ( USHORT nAttr = 0; nAttr < nAttribs; nAttr++ )
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
                    bResort = TRUE;
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
//                  bResort = TRUE;     // es gibt ja keine Features mehr...
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

#ifdef EDITDEBUG
    DBG_ASSERT( CheckOrderedList( (TextCharAttribs*)&maCharAttribs ), "Expand: Start-Liste verdreht" );
#endif
}

void TextNode::CollapsAttribs( USHORT nIndex, USHORT nDeleted )
{
    if ( !nDeleted )
        return;

    BOOL bResort = FALSE;
    USHORT nEndChanges = nIndex+nDeleted;

    for ( USHORT nAttr = 0; nAttr < maCharAttribs.Count(); nAttr++ )
    {
        TextCharAttrib* pAttrib = maCharAttribs.GetAttrib( nAttr );
        BOOL bDelAttr = FALSE;
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
                    bDelAttr = TRUE;
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
            bResort = TRUE;
            maCharAttribs.RemoveAttrib( nAttr );
            delete pAttrib;
            nAttr--;
        }
        else if ( pAttrib->IsEmpty() )
            maCharAttribs.HasEmptyAttribs() = TRUE;
    }

    if ( bResort )
        maCharAttribs.ResortAttribs();

#ifdef EDITDEBUG
    DBG_ASSERT( CheckOrderedList( (TextCharAttribs)&maCharAttribs ), "Collaps: Start-Liste verdreht" );
#endif
}

void TextNode::InsertText( USHORT nPos, const String& rText )
{
    maText.Insert( rText, nPos );
    ExpandAttribs( nPos, rText.Len() );
}

void TextNode::InsertText( USHORT nPos, sal_Unicode c )
{
    maText.Insert( c, nPos );
    ExpandAttribs( nPos, 1 );
}

void TextNode::RemoveText( USHORT nPos, USHORT nChars )
{
    maText.Erase( nPos, nChars );
    CollapsAttribs( nPos, nChars );
}

TextNode* TextNode::Split( USHORT nPos, BOOL bKeepEndingAttribs )
{
    String aNewText;
    if ( nPos < maText.Len() )
    {
        aNewText = maText.Copy( nPos );
        maText.Erase( nPos );
    }
    TextNode* pNew = new TextNode( aNewText );

    for ( USHORT nAttr = 0; nAttr < maCharAttribs.Count(); nAttr++ )
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
    USHORT nOldLen = maText.Len();

    maText += rNode.GetText();

#ifdef EDITDEBUG
    DBG_ASSERT( maCharAttribs.DbgCheckAttribs(), "Attribute VOR AppendAttribs kaputt" );
#endif

    const USHORT nAttribs = rNode.GetCharAttribs().Count();
    for ( USHORT nAttr = 0; nAttr < nAttribs; nAttr++ )
    {
        TextCharAttrib* pAttrib = rNode.GetCharAttribs().GetAttrib( nAttr );
        BOOL bMelted = FALSE;
        if ( pAttrib->GetStart() == 0 )
        {
            // Evtl koennen Attribute zusammengefasst werden:
            USHORT nTmpAttribs = maCharAttribs.Count();
            for ( USHORT nTmpAttr = 0; nTmpAttr < nTmpAttribs; nTmpAttr++ )
            {
                TextCharAttrib* pTmpAttrib = maCharAttribs.GetAttrib( nTmpAttr );

                if ( pTmpAttrib->GetEnd() == nOldLen )
                {
                    if ( ( pTmpAttrib->Which() == pAttrib->Which() ) &&
                         ( pTmpAttrib->GetAttr() == pAttrib->GetAttr() ) )
                    {
                        pTmpAttrib->GetEnd() =
                            pTmpAttrib->GetEnd() + pAttrib->GetLen();
                        bMelted = TRUE;
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

#ifdef EDITDEBUG
    DBG_ASSERT( maCharAttribs.DbgCheckAttribs(), "Attribute NACH AppendAttribs kaputt" );
#endif
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
    for ( ULONG nNode = 0; nNode < maTextNodes.Count(); nNode++ )
        delete maTextNodes.GetObject( nNode );
    maTextNodes.clear();
}

String TextDoc::GetText( const sal_Unicode* pSep ) const
{
    ULONG nLen = GetTextLen( pSep );
    ULONG nNodes = maTextNodes.Count();

    if ( nLen > STRING_MAXLEN )
    {
        DBG_ERROR( "Text zu gross fuer String" );
        return String();
    }

    String aASCIIText;
    ULONG nLastNode = nNodes-1;
    for ( ULONG nNode = 0; nNode < nNodes; nNode++ )
    {
        TextNode* pNode = maTextNodes.GetObject( nNode );
        String aTmp( pNode->GetText() );
        aASCIIText += aTmp;
        if ( pSep && ( nNode != nLastNode ) )
            aASCIIText += pSep;
    }

    return aASCIIText;
}

XubString TextDoc::GetText( ULONG nPara ) const
{
    XubString aText;
    TextNode* pNode = ( nPara < maTextNodes.Count() ) ? maTextNodes.GetObject( nPara ) : 0;
    if ( pNode )
        aText = pNode->GetText();

    return aText;
}


ULONG TextDoc::GetTextLen( const xub_Unicode* pSep, const TextSelection* pSel ) const
{
    ULONG nLen = 0;
    ULONG nNodes = maTextNodes.Count();
    if ( nNodes )
    {
        ULONG nStartNode = 0;
        ULONG nEndNode = nNodes-1;
        if ( pSel )
        {
            nStartNode = pSel->GetStart().GetPara();
            nEndNode = pSel->GetEnd().GetPara();
        }

        for ( ULONG nNode = nStartNode; nNode <= nEndNode; nNode++ )
        {
            TextNode* pNode = maTextNodes.GetObject( nNode );

            USHORT nS = 0;
            ULONG nE = pNode->GetText().Len();
            if ( pSel && ( nNode == pSel->GetStart().GetPara() ) )
                nS = pSel->GetStart().GetIndex();
            if ( pSel && ( nNode == pSel->GetEnd().GetPara() ) )
                nE = pSel->GetEnd().GetIndex();

            nLen += ( nE - nS );
        }

        if ( pSep )
            nLen += (nEndNode-nStartNode) * String( pSep ).Len();
    }

    return nLen;
}

TextPaM TextDoc::InsertText( const TextPaM& rPaM, xub_Unicode c )
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

TextPaM TextDoc::InsertParaBreak( const TextPaM& rPaM, BOOL bKeepEndingAttribs )
{
    TextNode* pNode = maTextNodes.GetObject( rPaM.GetPara() );
    TextNode* pNew = pNode->Split( rPaM.GetIndex(), bKeepEndingAttribs );

    maTextNodes.Insert( pNew, rPaM.GetPara()+1 );

    TextPaM aPaM( rPaM.GetPara()+1, 0 );
    return aPaM;
}

TextPaM TextDoc::ConnectParagraphs( TextNode* pLeft, TextNode* pRight )
{
    USHORT nPrevLen = pLeft->GetText().Len();
    pLeft->Append( *pRight );

    // der rechte verschwindet.
    ULONG nRight = maTextNodes.GetPos( pRight );
    maTextNodes.Remove( nRight );
    delete pRight;

    ULONG nLeft = maTextNodes.GetPos( pLeft );
    TextPaM aPaM( nLeft, nPrevLen );
    return aPaM;
}

TextPaM TextDoc::RemoveChars( const TextPaM& rPaM, USHORT nChars )
{
    TextNode* pNode = maTextNodes.GetObject( rPaM.GetPara() );
    pNode->RemoveText( rPaM.GetIndex(), nChars );

    return rPaM;
}

BOOL TextDoc::IsValidPaM( const TextPaM& rPaM )
{
    if ( rPaM.GetPara() >= maTextNodes.Count() )
    {
        DBG_ERROR( "PaM: Para out of range" );
        return FALSE;
    }
    TextNode * pNode = maTextNodes.GetObject( rPaM.GetPara() );
    if ( rPaM.GetIndex() > pNode->GetText().Len() )
    {
        DBG_ERROR( "PaM: Index out of range" );
        return FALSE;
    }
    return TRUE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
