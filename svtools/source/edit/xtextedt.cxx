/*************************************************************************
 *
 *  $RCSfile: xtextedt.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mt $ $Date: 2001-03-09 10:21:41 $
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

#include <xtextedt.hxx>
#include <vcl/svapp.hxx>  // International
#include <unotools/textsearch.hxx>
#ifndef _COM_SUN_STAR_UTIL_SEARCHOPTIONS_HPP_
#include <com/sun/star/util/SearchOptions.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_SEARCHFLAGS_HPP_
#include <com/sun/star/util/SearchFlags.hpp>
#endif

using namespace ::com::sun::star;



// -------------------------------------------------------------------------
// class ExtTextEngine
// -------------------------------------------------------------------------
ExtTextEngine::ExtTextEngine() : maGroupChars( String::CreateFromAscii( "(){}[]", 6 ) )
{
}

ExtTextEngine::~ExtTextEngine()
{
}

TextSelection ExtTextEngine::MatchGroup( const TextPaM& rCursor ) const
{
    TextSelection aSel( rCursor );
    USHORT nPos = rCursor.GetIndex();
    ULONG nPara = rCursor.GetPara();
    ULONG nParas = GetParagraphCount();
    if ( ( nPara < nParas ) && ( nPos < GetTextLen( nPara ) ) )
    {
        USHORT nMatchChar = maGroupChars.Search( GetText( rCursor.GetPara() ).GetChar( nPos ) );
        if ( nMatchChar != STRING_NOTFOUND )
        {
            if ( ( nMatchChar % 2 ) == 0 )
            {
                // Vorwaerts suchen...
                sal_Unicode nSC = maGroupChars.GetChar( nMatchChar );
                sal_Unicode nEC = maGroupChars.GetChar( nMatchChar+1 );

                USHORT nCur = nPos+1;
                USHORT nLevel = 1;
                while ( nLevel && ( nPara < nParas ) )
                {
                    XubString aStr = GetText( nPara );
                    while ( nCur < aStr.Len() )
                    {
                        if ( aStr.GetChar( nCur ) == nSC )
                            nLevel++;
                        else if ( aStr.GetChar( nCur ) == nEC )
                        {
                            nLevel--;
                            if ( !nLevel )
                                break;  // while nCur...
                        }
                        nCur++;
                    }

                    if ( nLevel )
                    {
                        nPara++;
                        nCur = 0;
                    }
                }
                if ( nLevel == 0 )  // gefunden
                {
                    aSel.GetStart() = rCursor;
                    aSel.GetEnd() = TextPaM( nPara, nCur+1 );
                }
            }
            else
            {
                // Rueckwaerts suchen...
                xub_Unicode nEC = maGroupChars.GetChar( nMatchChar );
                xub_Unicode nSC = maGroupChars.GetChar( nMatchChar-1 );

                USHORT nCur = rCursor.GetIndex()-1;
                USHORT nLevel = 1;
                while ( nLevel )
                {
                    if ( GetTextLen( nPara ) )
                    {
                        XubString aStr = GetText( nPara );
                        while ( nCur )
                        {
                            if ( aStr.GetChar( nCur ) == nSC )
                            {
                                nLevel--;
                                if ( !nLevel )
                                    break;  // while nCur...
                            }
                            else if ( aStr.GetChar( nCur ) == nEC )
                                nLevel++;

                            nCur--;
                        }
                    }

                    if ( nLevel )
                    {
                        if ( nPara )
                        {
                            nPara--;
                            nCur = GetTextLen( nPara )-1;   // egal ob negativ, weil if Len()
                        }
                        else
                            break;
                    }
                }

                if ( nLevel == 0 )  // gefunden
                {
                    aSel.GetStart() = rCursor;
                    aSel.GetStart().GetIndex()++;   // hinter das Zeichen
                    aSel.GetEnd() = TextPaM( nPara, nCur );
                }
            }
        }
    }
    return aSel;
}

BOOL ExtTextEngine::Search( TextSelection& rSel, const util::SearchOptions& rSearchOptions, BOOL bForward )
{
    TextSelection aSel( rSel );
    aSel.Justify();

    BOOL bSearchInSelection = rSearchOptions.searchFlag & util::SearchFlags::REG_NOT_BEGINOFLINE;

    TextPaM aStartPaM( aSel.GetEnd() );
    if ( aSel.HasRange() && ( ( bSearchInSelection && bForward ) || ( !bSearchInSelection && !bForward ) ) )
    {
        aStartPaM = aSel.GetStart();
    }

    BOOL bFound = FALSE;
    ULONG nStartNode, nEndNode;

    if ( bSearchInSelection )
        nEndNode = bForward ? aSel.GetEnd().GetPara() : aSel.GetStart().GetPara();
    else
        nEndNode = bForward ? (GetParagraphCount()-1) : 0;

    nStartNode = aStartPaM.GetPara();

    util::SearchOptions aOptions( rSearchOptions );
    aOptions.Locale = Application::GetSettings().GetLocale();
    utl::TextSearch aSearcher( rSearchOptions );

    // ueber die Absaetze iterieren...
    for ( ULONG nNode = nStartNode;
            bForward ?  ( nNode <= nEndNode) : ( nNode >= nEndNode );
            bForward ? nNode++ : nNode-- )
    {
        String aText = GetText( nNode );
        USHORT nStartPos = 0;
        USHORT nEndPos = aText.Len();
        if ( nNode == nStartNode )
        {
            if ( bForward )
                nStartPos = aStartPaM.GetIndex();
            else
                nEndPos = aStartPaM.GetIndex();
        }
        if ( ( nNode == nEndNode ) && bSearchInSelection )
        {
            if ( bForward )
                nEndPos = aSel.GetEnd().GetIndex();
            else
                nStartPos = aSel.GetStart().GetIndex();
        }

        if ( bForward )
            bFound = aSearcher.SearchFrwrd( aText, &nStartPos, &nEndPos );
        else
            bFound = aSearcher.SearchBkwrd( aText, &nEndPos, &nStartPos );

        if ( bFound )
        {
            rSel.GetStart().GetPara() = nNode;
            rSel.GetStart().GetIndex() = bForward ? nStartPos : (nEndPos+1);
            rSel.GetEnd().GetPara() = nNode;
            rSel.GetEnd().GetIndex() = bForward ? nEndPos : (nStartPos+1);
            // Ueber den Absatz selektieren?
            if( nEndPos == (-1) ) // USHORT fuer 0 und -1 !
            {
                if ( (rSel.GetEnd().GetPara()+1) < GetParagraphCount() )
                {
                    rSel.GetEnd().GetPara()++;
                    rSel.GetEnd().GetIndex() = 0;
                }
                else
                {
                    rSel.GetEnd().GetIndex() = nStartPos;
                    bFound = FALSE;
                }
            }

            break;
        }

        if ( !bForward && !nNode )  // Bei rueckwaertsuche, wenn nEndNode = 0:
            break;
    }

    return bFound;
}


// -------------------------------------------------------------------------
// class ExtTextView
// -------------------------------------------------------------------------
ExtTextView::ExtTextView( ExtTextEngine* pEng, Window* pWindow )
    : TextView( pEng, pWindow )
{
}

ExtTextView::~ExtTextView()
{
}

BOOL ExtTextView::MatchGroup()
{
    TextSelection aTmpSel( GetSelection() );
    aTmpSel.Justify();
    if ( ( aTmpSel.GetStart().GetPara() != aTmpSel.GetEnd().GetPara() ) ||
         ( ( aTmpSel.GetEnd().GetIndex() - aTmpSel.GetStart().GetIndex() ) > 1 ) )
    {
        return FALSE;
    }

    TextSelection aMatchSel = ((ExtTextEngine*)GetTextEngine())->MatchGroup( aTmpSel.GetStart() );
    if ( aMatchSel.HasRange() )
        SetSelection( aMatchSel );

    return aMatchSel.HasRange() ? TRUE : FALSE;
}

BOOL ExtTextView::Search( const util::SearchOptions& rSearchOptions, BOOL bForward )
{
    BOOL bFound = FALSE;
    TextSelection aSel( GetSelection() );
    if ( ((ExtTextEngine*)GetTextEngine())->Search( aSel, rSearchOptions, bForward ) )
    {
        bFound = TRUE;
        // Erstmal den Anfang des Wortes als Selektion einstellen,
        // damit das ganze Wort in den sichtbaren Bereich kommt.
        SetSelection( aSel.GetStart() );
        ShowCursor( TRUE, FALSE );
    }
    else
    {
        aSel = GetSelection().GetEnd();
    }

    SetSelection( aSel );
    ShowCursor();

    return bFound;
}

USHORT ExtTextView::Replace( const util::SearchOptions& rSearchOptions, BOOL bAll, BOOL bForward )
{
    USHORT nFound = 0;

    if ( !bAll )
    {
        if ( GetSelection().HasRange() )
        {
            InsertText( rSearchOptions.replaceString );
            nFound = 1;
            Search( rSearchOptions, bForward ); // gleich zum naechsten
        }
        else
        {
            if( Search( rSearchOptions, bForward ) )
                nFound = 1;
        }
    }
    else
    {
        // Der Writer ersetzt alle, vom Anfang bis Ende...

        ExtTextEngine* pTextEngine = (ExtTextEngine*)GetTextEngine();

        // HideSelection();
        TextSelection aSel;
        BOOL bSearchInSelection = rSearchOptions.searchFlag & util::SearchFlags::REG_NOT_BEGINOFLINE;
        if ( bSearchInSelection )
        {
            aSel = GetSelection();
            aSel.Justify();
        }

        TextSelection aSearchSel( aSel );

        BOOL bFound = pTextEngine->Search( aSel, rSearchOptions, TRUE );
        if ( bFound )
            pTextEngine->UndoActionStart( XTEXTUNDO_REPLACEALL );
        while ( bFound )
        {
            nFound++;

            TextPaM aNewStart = pTextEngine->ImpInsertText( aSel, rSearchOptions.replaceString );
            aSel = aSearchSel;
            aSel.GetStart() = aNewStart;
            bFound = pTextEngine->Search( aSel, rSearchOptions, TRUE );
        }
        if ( nFound )
        {
            SetSelection( aSel.GetStart() );
            pTextEngine->FormatAndUpdate( this );
            pTextEngine->UndoActionEnd( XTEXTUNDO_REPLACEALL );
        }
    }
    return nFound;
}

BOOL ExtTextView::ImpIndentBlock( BOOL bRight )
{
    BOOL bDone = FALSE;

    TextSelection aSel = GetSelection();
    aSel.Justify();

    HideSelection();
    GetTextEngine()->UndoActionStart( bRight ? XTEXTUNDO_INDENTBLOCK : XTEXTUNDO_UNINDENTBLOCK );

    ULONG nStartPara = aSel.GetStart().GetPara();
    ULONG nEndPara = aSel.GetEnd().GetPara();
    BOOL bIndentAll = TRUE;
    if ( aSel.HasRange() && !aSel.GetEnd().GetIndex() )
    {
        nEndPara--; // den dann nicht einruecken...
    }

    for ( ULONG nPara = nStartPara; nPara <= nEndPara; nPara++ )
    {
        if ( bRight )
        {
            // Tabs hinzufuegen
            GetTextEngine()->ImpInsertText( TextPaM( nPara, 0 ), '\t' );
            bDone = TRUE;
        }
        else
        {
            // Tabs/Blanks entfernen
            String aText = GetTextEngine()->GetText( nPara );
            if ( aText.Len() && (
                    ( aText.GetChar( 0 ) == '\t' ) ||
                    ( aText.GetChar( 0 ) == ' ' ) ) )
            {
                GetTextEngine()->ImpDeleteText( TextSelection( TextPaM( nPara, 0 ), TextPaM( nPara, 1 ) ) );
                bDone = TRUE;
            }
        }
    }

    GetTextEngine()->UndoActionEnd( bRight ? XTEXTUNDO_INDENTBLOCK : XTEXTUNDO_UNINDENTBLOCK );

    BOOL bRange = aSel.HasRange();
    if ( bRight )
    {
        aSel.GetStart().GetIndex()++;
        if ( bRange && ( aSel.GetEnd().GetPara() == nEndPara ) )
            aSel.GetEnd().GetIndex()++;
    }
    else
    {
        if ( aSel.GetStart().GetIndex() )
            aSel.GetStart().GetIndex()--;
        if ( bRange && aSel.GetEnd().GetIndex() )
            aSel.GetEnd().GetIndex()--;
    }

    maSelection = aSel;
    GetTextEngine()->FormatAndUpdate( this );

    return bDone;
}

BOOL ExtTextView::IndentBlock()
{
    return ImpIndentBlock( TRUE );
}

BOOL ExtTextView::UnindentBlock()
{
    return ImpIndentBlock( FALSE );
}

