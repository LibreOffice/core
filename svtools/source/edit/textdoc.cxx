/*************************************************************************
 *
 *  $RCSfile: textdoc.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <textdoc.hxx>

#include <stdlib.h>

SV_IMPL_PTRARR( TextCharAttribs, TextCharAttribPtr );



// Vergleichmethode wird von QuickSort gerufen...

EXTERN_C
#if defined( PM2 ) && (!defined( CSET ) && !defined ( MTW ) && !defined( WTC ))
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
    String aNewText = maText.Copy( nPos );
    maText.Erase( nPos );
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
            pAttrib->GetStart() -= nPos;
            pAttrib->GetEnd() -= nPos;
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
                        pTmpAttrib->GetEnd() += pAttrib->GetLen();
                        bMelted = TRUE;
                        break;  // es kann nur eins von der Sorte an der Stelle geben
                    }
                }
            }
        }

        if ( !bMelted )
        {
            TextCharAttrib* pNewAttrib = new TextCharAttrib( *pAttrib );
            pNewAttrib->GetStart() += nOldLen;
            pNewAttrib->GetEnd() += nOldLen;
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
    maTextNodes.Clear();
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

/*

void TextDoc::InsertAttribInSelection( TextNode* pNode, USHORT nStart, USHORT nEnd, const SfxPoolItem& rPoolItem )
{
    DBG_ASSERT( pNode, "Wohin mit dem Attribut?" );
    DBG_ASSERT( nEnd <= pNode->Len(), "InsertAttrib: Attribut zu gross!" );

    // fuer Optimierung:
    // dieses endet am Anfang der Selektion => kann erweitert werden
    TextCharAttrib* pEndingAttrib = 0;
    // dieses startet am Ende der Selektion => kann erweitert werden
    TextCharAttrib* pStartingAttrib = 0;

    DBG_ASSERT( nStart <= nEnd, "Kleiner Rechenfehler in InsertAttribInSelection" );

    RemoveAttribs( pNode, nStart, nEnd, pStartingAttrib, pEndingAttrib, rPoolItem.Which() );

    if ( pStartingAttrib && pEndingAttrib &&
         ( *(pStartingAttrib->GetItem()) == rPoolItem ) &&
         ( *(pEndingAttrib->GetItem()) == rPoolItem ) )
    {
        // wird ein groesses Attribut.
        pEndingAttrib->GetEnd() = pStartingAttrib->GetEnd();
        pCurPool->Remove( *(pStartingAttrib->GetItem()) );
        pNode->GetCharAttribs().GetAttribs().Remove( pNode->GetCharAttribs().GetAttribs().GetPos( pStartingAttrib ) );
        delete pStartingAttrib;
    }
    else if ( pStartingAttrib && ( *(pStartingAttrib->GetItem()) == rPoolItem ) )
        pStartingAttrib->GetStart() = nStart;
    else if ( pEndingAttrib && ( *(pEndingAttrib->GetItem()) == rPoolItem ) )
        pEndingAttrib->GetEnd() = nEnd;
    else
        InsertAttrib( rPoolItem, pNode, nStart, nEnd );

    if ( pStartingAttrib )
        pNode->GetCharAttribs().ResortAttribs();
}

BOOL TextDoc::RemoveAttribs( TextNode* pNode, USHORT nStart, USHORT nEnd, USHORT nWhich )
{
    TextCharAttrib* pStarting;
    TextCharAttrib* pEnding;
    return RemoveAttribs( pNode, nStart, nEnd, pStarting, pEnding, nWhich );
}

BOOL TextDoc::RemoveAttribs( TextNode* pNode, USHORT nStart, USHORT nEnd, TextCharAttrib*& rpStarting, TextCharAttrib*& rpEnding, USHORT nWhich )
{
    DBG_ASSERT( pNode, "Wohin mit dem Attribut?" );
    DBG_ASSERT( nEnd <= pNode->Len(), "InsertAttrib: Attribut zu gross!" );

    // dieses endet am Anfang der Selektion => kann erweitert werden
    rpEnding = 0;
    // dieses startet am Ende der Selektion => kann erweitert werden
    rpStarting = 0;

    BOOL bChanged = FALSE;

    DBG_ASSERT( nStart <= nEnd, "Kleiner Rechenfehler in InsertAttribInSelection" );

    // ueber die Attribute iterieren...
    USHORT nAttr = 0;
    TextCharAttrib* pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
    while ( pAttr )
    {
        BOOL bRemoveAttrib = FALSE;
        if ( !nWhich || ( pAttr->Which() == nWhich ) )
        {
            // Attribut beginnt in Selection
            if ( ( pAttr->GetStart() >= nStart ) && ( pAttr->GetStart() <= nEnd ) )
            {
                bChanged = TRUE;
                if ( pAttr->GetEnd() > nEnd )
                {
                    pAttr->GetStart() = nEnd;   // dann faengt es dahinter an
                    rpStarting = pAttr;
                    break;  // es kann kein weiteres Attrib hier liegen
                }
                else if ( !pAttr->IsFeature() || ( pAttr->GetStart() == nStart ) )
                {
                    // Feature nur loeschen, wenn genau an der Stelle
                    bRemoveAttrib = TRUE;
                }
            }

            // Attribut endet in Selection
            else if ( ( pAttr->GetEnd() >= nStart ) && ( pAttr->GetEnd() <= nEnd ) )
            {
                bChanged = TRUE;
                if ( ( pAttr->GetStart() < nStart ) && !pAttr->IsFeature() )
                {
                    pAttr->GetEnd() = nStart;   // dann hoert es hier auf
                    rpEnding = pAttr;
                }
                else if ( !pAttr->IsFeature() || ( pAttr->GetStart() == nStart ) )
                {
                    // Feature nur loeschen, wenn genau an der Stelle
                    bRemoveAttrib = TRUE;
                }
            }
            // Attribut ueberlappt die Selektion
            else if ( ( pAttr->GetStart() <= nStart ) && ( pAttr->GetEnd() >= nEnd ) )
            {
                bChanged = TRUE;
                if ( pAttr->GetStart() == nStart )
                {
                    pAttr->GetStart() = nEnd;
                    rpStarting = pAttr;
                    break;  // es kann weitere Attribute geben!
                }
                else if ( pAttr->GetEnd() == nEnd )
                {
                    pAttr->GetEnd() = nStart;
                    rpEnding = pAttr;
                    break;  // es kann weitere Attribute geben!
                }
                else // Attribut muss gesplittet werden...
                {
                    USHORT nOldEnd = pAttr->GetEnd();
                    pAttr->GetEnd() = nStart;
                    rpEnding = pAttr;
//                  ULONG nSavePos = pNode->GetCharAttribs().GetStartList().GetCurPos();
                    InsertAttrib( *pAttr->GetItem(), pNode, nEnd, nOldEnd );
//                  pNode->GetCharAttribs().GetStartList().Seek( nSavePos );
                    break;  // es kann weitere Attribute geben!
                }
            }
        }
        if ( bRemoveAttrib )
        {
            DBG_ASSERT( ( pAttr != rpStarting ) && ( pAttr != rpEnding ), "Loeschen und behalten des gleichen Attributs ?" );
            pNode->GetCharAttribs().GetAttribs().Remove(nAttr);
            pCurPool->Remove( *pAttr->GetItem() );
            delete pAttr;
            nAttr--;
        }
        nAttr++;
        pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
    }
    return bChanged;
}

#pragma SEG_FUNCDEF(editdoc_3f)

void TextDoc::InsertAttrib( const SfxPoolItem& rPoolItem, TextNode* pNode, USHORT nStart, USHORT nEnd )
{
    // Diese Methode prueft nicht mehr, ob ein entspr. Attribut
    // schon an der Stelle existiert!

    // pruefen, ob neues Attrib oder einfach nur Ende eines Attribs...
//  const SfxPoolItem& rDefItem = pNode->GetContentAttribs().GetItem( rPoolItem.Which() );
//  BOOL bCreateAttrib = ( rDefItem != rPoolItem );

    // Durch den Verlust der Exclude-Liste geht es nicht mehr, dass ich
    // kein neues Attribut benoetige und nur das alte nicht expandiere...
//  if ( !bCreateAttrib )
    {
        // => Wenn schon Default-Item, dann wenigstens nur dann einstellen,
        // wenn davor wirklich ein entsprechendes Attribut.
//      if ( pNode->GetCharAttribs().FindAttrib( rPoolItem.Which(), nStart ) )
//          bCreateAttrib = TRUE;
        // Aber kleiner Trost:
        // Die wenigsten schreiben, aendern das Attr, schreiben, und
        // stellen dann wieder das Default-Attr ein.
    }

    // 22.9.95:
    // Die Uberlegung, einfach das andere Attribut nicht zu expandieren, war
    // sowieso falsch, da das DefAttr aus einer Vorlage kommen kann,
    // die irgendwann verschwindet!
//  if ( bCreateAttrib )
//  {
        TextCharAttrib* pAttrib = MakeCharAttrib( *pCurPool, rPoolItem, nStart, nEnd );
        DBG_ASSERT( pAttrib, "MakeCharAttrib fehlgeschlagen!" );
        pNode->GetCharAttribs().InsertAttrib( pAttrib );
//  }
//  else
//  {
//      TextCharAttrib* pTmpAttrib =
//          pNode->GetCharAttribs().FindAnyAttrib( rPoolItem.Which() );
//      if ( pTmpAttrib )   // sonst benoetige ich es sowieso nicht....
//      {
//          aExcludeList.Insert( pTmpAttrib->GetItem() );
//      }
//  }
}

#pragma SEG_FUNCDEF(editdoc_40)

void TextDoc::InsertAttrib( TextNode* pNode, USHORT nStart, USHORT nEnd, const SfxPoolItem& rPoolItem )
{
    if ( nStart != nEnd )
    {
        InsertAttribInSelection( pNode, nStart, nEnd, rPoolItem );
    }
    else
    {
        // Pruefen, ob schon ein neues Attribut mit der WhichId an der Stelle:
        TextCharAttrib* pAttr = pNode->GetCharAttribs().FindEmptyAttrib( rPoolItem.Which(), nStart );
        if ( pAttr )
        {
            // Attribut entfernen....
            pNode->GetCharAttribs().GetAttribs().Remove(
                pNode->GetCharAttribs().GetAttribs().GetPos( pAttr ) );
        }

        // pruefen, ob ein 'gleiches' Attribut an der Stelle liegt.
        pAttr = pNode->GetCharAttribs().FindAttrib( rPoolItem.Which(), nStart );
        if ( pAttr )
        {
            if ( pAttr->IsInside( nStart ) )    // splitten
            {
                // ???????????????????????????????
                // eigentlich noch pruefen, ob wirklich splittet, oder return !
                // ???????????????????????????????
                USHORT nOldEnd = pAttr->GetEnd();
                pAttr->GetEnd() = nStart;
                pAttr = MakeCharAttrib( *pCurPool, *(pAttr->GetItem()), nStart, nOldEnd );
                pNode->GetCharAttribs().InsertAttrib( pAttr );
            }
            else if ( pAttr->GetEnd() == nStart )
            {
                DBG_ASSERT( !pAttr->IsEmpty(), "Doch noch ein leeres Attribut?" );
                // pruefen, ob genau das gleiche Attribut
                if ( *(pAttr->GetItem()) == rPoolItem )
                    return;
            }
        }
        InsertAttrib( rPoolItem, pNode, nStart, nStart );
    }
}

#pragma SEG_FUNCDEF(editdoc_41)

void TextDoc::FindAttribs( TextNode* pNode, USHORT nStartPos, USHORT nEndPos, SfxItemSet& rCurSet )
{
    DBG_ASSERT( pNode, "Wo soll ich suchen ?" );
    DBG_ASSERT( nStartPos <= nEndPos, "Ungueltiger Bereich!" );

    USHORT nAttr = 0;
    TextCharAttrib* pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
    // keine Selection...
    if ( nStartPos == nEndPos )
    {
        while ( pAttr && ( pAttr->GetStart() <= nEndPos) )
        {
            const SfxPoolItem* pItem = 0;
            // Attribut liegt dadrueber...
            if ( ( pAttr->GetStart() < nStartPos ) && ( pAttr->GetEnd() > nStartPos ) )
                pItem = pAttr->GetItem();
            // Attribut endet hier, ist nicht leer
            else if ( ( pAttr->GetStart() < nStartPos ) && ( pAttr->GetEnd() == nStartPos ) )
            {
                if ( !pNode->GetCharAttribs().FindEmptyAttrib( pAttr->GetItem()->Which(), nStartPos ) )
                    pItem = pAttr->GetItem();
            }
            // Attribut endet hier, ist leer
            else if ( ( pAttr->GetStart() == nStartPos ) && ( pAttr->GetEnd() == nStartPos ) )
            {
//              if ( aExcludeList.FindAttrib( pAttr->GetItem()->Which() ) )
                    pItem = pAttr->GetItem();
//              else if ( pNode->Len() == 0 )   // Sonderfall
//                  pItem = pAttr->GetItem();
            }
            // Attribut beginnt hier
            else if ( ( pAttr->GetStart() == nStartPos ) && ( pAttr->GetEnd() > nStartPos ) )
            {
                if ( nStartPos == 0 )   // Sonderfall
                    pItem = pAttr->GetItem();
            }

            if ( pItem )
            {
                USHORT nWhich = pItem->Which();
                if ( rCurSet.GetItemState( nWhich ) == SFX_ITEM_OFF )
                {
                    rCurSet.Put( *pItem );
                }
                else if ( rCurSet.GetItemState( nWhich ) == SFX_ITEM_ON )
                {
                    const SfxPoolItem& rItem = rCurSet.Get( nWhich );
                    if ( rItem != *pItem )
                    {
                        rCurSet.InvalidateItem( nWhich );
                    }
                }
            }
            nAttr++;
            pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
        }
    }
    else    // Selektion
    {
        while ( pAttr && ( pAttr->GetStart() < nEndPos) )
        {
            const SfxPoolItem* pItem = 0;
            // Attribut liegt dadrueber...
            if ( ( pAttr->GetStart() <= nStartPos ) && ( pAttr->GetEnd() >= nEndPos ) )
                pItem = pAttr->GetItem();
            // Attribut startet mitten drin...
            else if ( pAttr->GetStart() >= nStartPos )
            {
                // !!! pItem = pAttr->GetItem();
                // einfach nur pItem reicht nicht, da ich z.B. bei Shadow
                // niemals ein ungleiches Item finden wuerde, da ein solche
                // seine Anwesenheit durch Abwesenheit repraesentiert!
                // if ( ... )
                // Es muesste geprueft werden, on genau das gleiche Attribut
                // an der Bruchstelle aufsetzt, was recht aufwendig ist.
                // Da ich beim Einfuegen von Attributen aber etwas optimiere
                // tritt der Fall nicht so schnell auf...
                // Also aus Geschwindigkeitsgruenden:
                rCurSet.InvalidateItem( pAttr->GetItem()->Which() );

            }
            // Attribut endet mitten drin...
            else if ( pAttr->GetEnd() > nStartPos )
            {
                // pItem = pAttr->GetItem();
                // s.o.

                // -----------------31.05.95 16:01-------------------
                //  Ist falsch, wenn das gleiche Attribut sofort wieder
                //  eingestellt wird!
                //  => Sollte am besten nicht vorkommen, also gleich beim
                //      Setzen von Attributen richtig machen!
                // --------------------------------------------------
                rCurSet.InvalidateItem( pAttr->GetItem()->Which() );
            }

            if ( pItem )
            {
                USHORT nWhich = pItem->Which();
                if ( rCurSet.GetItemState( nWhich ) == SFX_ITEM_OFF )
                {
                    rCurSet.Put( *pItem );
                }
                else if ( rCurSet.GetItemState( nWhich ) == SFX_ITEM_ON )
                {
                    const SfxPoolItem& rItem = rCurSet.Get( nWhich );
                    if ( rItem != *pItem )
                    {
                        rCurSet.InvalidateItem( nWhich );
                    }
                }
            }
            nAttr++;
            pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
        }
    }
}


*/


