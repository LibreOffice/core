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


#include <vcl/xtextedt.hxx>
#include <vcl/svapp.hxx>  // International
#include <unotools/textsearch.hxx>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchFlags.hpp>

using namespace ::com::sun::star;



// -------------------------------------------------------------------------
// class ExtTextEngine
// -------------------------------------------------------------------------
ExtTextEngine::ExtTextEngine() : maGroupChars(rtl::OUString("(){}[]"))
{
}

ExtTextEngine::~ExtTextEngine()
{
}

TextSelection ExtTextEngine::MatchGroup( const TextPaM& rCursor ) const
{
    TextSelection aSel( rCursor );
    sal_uInt16 nPos = rCursor.GetIndex();
    sal_uLong nPara = rCursor.GetPara();
    sal_uLong nParas = GetParagraphCount();
    if ( ( nPara < nParas ) && ( nPos < GetTextLen( nPara ) ) )
    {
        sal_uInt16 nMatchChar = maGroupChars.Search( GetText( rCursor.GetPara() ).GetChar( nPos ) );
        if ( nMatchChar != STRING_NOTFOUND )
        {
            if ( ( nMatchChar % 2 ) == 0 )
            {
                // Vorwaerts suchen...
                sal_Unicode nSC = maGroupChars.GetChar( nMatchChar );
                sal_Unicode nEC = maGroupChars.GetChar( nMatchChar+1 );

                sal_uInt16 nCur = nPos+1;
                sal_uInt16 nLevel = 1;
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

                sal_uInt16 nCur = rCursor.GetIndex()-1;
                sal_uInt16 nLevel = 1;
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

sal_Bool ExtTextEngine::Search( TextSelection& rSel, const util::SearchOptions& rSearchOptions, sal_Bool bForward )
{
    TextSelection aSel( rSel );
    aSel.Justify();

    sal_Bool bSearchInSelection = (0 != (rSearchOptions.searchFlag & util::SearchFlags::REG_NOT_BEGINOFLINE) );

    TextPaM aStartPaM( aSel.GetEnd() );
    if ( aSel.HasRange() && ( ( bSearchInSelection && bForward ) || ( !bSearchInSelection && !bForward ) ) )
    {
        aStartPaM = aSel.GetStart();
    }

    bool bFound = false;
    sal_uLong nStartNode, nEndNode;

    if ( bSearchInSelection )
        nEndNode = bForward ? aSel.GetEnd().GetPara() : aSel.GetStart().GetPara();
    else
        nEndNode = bForward ? (GetParagraphCount()-1) : 0;

    nStartNode = aStartPaM.GetPara();

    util::SearchOptions aOptions( rSearchOptions );
    aOptions.Locale = Application::GetSettings().GetLocale();
    utl::TextSearch aSearcher( rSearchOptions );

    // ueber die Absaetze iterieren...
    for ( sal_uLong nNode = nStartNode;
            bForward ?  ( nNode <= nEndNode) : ( nNode >= nEndNode );
            bForward ? nNode++ : nNode-- )
    {
        String aText = GetText( nNode );
        sal_uInt16 nStartPos = 0;
        sal_uInt16 nEndPos = aText.Len();
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
            rSel.GetStart().GetIndex() = nStartPos;
            rSel.GetEnd().GetPara() = nNode;
            rSel.GetEnd().GetIndex() = nEndPos;
            // Ueber den Absatz selektieren?
            // Select over the paragraph?
            // FIXME  This should be max long...
            if( nEndPos == sal::static_int_cast<sal_uInt16>(-1) ) // sal_uInt16 for 0 and -1 !
            {
                if ( (rSel.GetEnd().GetPara()+1) < GetParagraphCount() )
                {
                    rSel.GetEnd().GetPara()++;
                    rSel.GetEnd().GetIndex() = 0;
                }
                else
                {
                    rSel.GetEnd().GetIndex() = nStartPos;
                    bFound = false;
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

sal_Bool ExtTextView::MatchGroup()
{
    TextSelection aTmpSel( GetSelection() );
    aTmpSel.Justify();
    if ( ( aTmpSel.GetStart().GetPara() != aTmpSel.GetEnd().GetPara() ) ||
         ( ( aTmpSel.GetEnd().GetIndex() - aTmpSel.GetStart().GetIndex() ) > 1 ) )
    {
        return sal_False;
    }

    TextSelection aMatchSel = ((ExtTextEngine*)GetTextEngine())->MatchGroup( aTmpSel.GetStart() );
    if ( aMatchSel.HasRange() )
        SetSelection( aMatchSel );

    return aMatchSel.HasRange() ? sal_True : sal_False;
}

sal_Bool ExtTextView::Search( const util::SearchOptions& rSearchOptions, sal_Bool bForward )
{
    sal_Bool bFound = sal_False;
    TextSelection aSel( GetSelection() );
    if ( ((ExtTextEngine*)GetTextEngine())->Search( aSel, rSearchOptions, bForward ) )
    {
        bFound = sal_True;
        // Erstmal den Anfang des Wortes als Selektion einstellen,
        // damit das ganze Wort in den sichtbaren Bereich kommt.
        SetSelection( aSel.GetStart() );
        ShowCursor( sal_True, sal_False );
    }
    else
    {
        aSel = GetSelection().GetEnd();
    }

    SetSelection( aSel );
    ShowCursor();

    return bFound;
}

sal_uInt16 ExtTextView::Replace( const util::SearchOptions& rSearchOptions, sal_Bool bAll, sal_Bool bForward )
{
    sal_uInt16 nFound = 0;

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

        sal_Bool bSearchInSelection = (0 != (rSearchOptions.searchFlag & util::SearchFlags::REG_NOT_BEGINOFLINE) );
        if ( bSearchInSelection )
        {
            aSel = GetSelection();
            aSel.Justify();
        }

        TextSelection aSearchSel( aSel );

        sal_Bool bFound = pTextEngine->Search( aSel, rSearchOptions, sal_True );
        if ( bFound )
            pTextEngine->UndoActionStart();
        while ( bFound )
        {
            nFound++;

            TextPaM aNewStart = pTextEngine->ImpInsertText( aSel, rSearchOptions.replaceString );
            aSel = aSearchSel;
            aSel.GetStart() = aNewStart;
            bFound = pTextEngine->Search( aSel, rSearchOptions, sal_True );
        }
        if ( nFound )
        {
            SetSelection( aSel.GetStart() );
            pTextEngine->FormatAndUpdate( this );
            pTextEngine->UndoActionEnd();
        }
    }
    return nFound;
}

sal_Bool ExtTextView::ImpIndentBlock( sal_Bool bRight )
{
    sal_Bool bDone = sal_False;

    TextSelection aSel = GetSelection();
    aSel.Justify();

    HideSelection();
    GetTextEngine()->UndoActionStart();

    sal_uLong nStartPara = aSel.GetStart().GetPara();
    sal_uLong nEndPara = aSel.GetEnd().GetPara();
    if ( aSel.HasRange() && !aSel.GetEnd().GetIndex() )
    {
        nEndPara--; // den dann nicht einruecken...
    }

    for ( sal_uLong nPara = nStartPara; nPara <= nEndPara; nPara++ )
    {
        if ( bRight )
        {
            // Tabs hinzufuegen
            GetTextEngine()->ImpInsertText( TextPaM( nPara, 0 ), '\t' );
            bDone = sal_True;
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
                bDone = sal_True;
            }
        }
    }

    GetTextEngine()->UndoActionEnd();

    sal_Bool bRange = aSel.HasRange();
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

    ImpSetSelection( aSel );
    GetTextEngine()->FormatAndUpdate( this );

    return bDone;
}

sal_Bool ExtTextView::IndentBlock()
{
    return ImpIndentBlock( sal_True );
}

sal_Bool ExtTextView::UnindentBlock()
{
    return ImpIndentBlock( sal_False );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
