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


#include <vcl/xtextedt.hxx>
#include <vcl/svapp.hxx>  // International
#include <unotools/textsearch.hxx>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchFlags.hpp>

using namespace ::com::sun::star;


ExtTextEngine::ExtTextEngine() : maGroupChars(OUString("(){}[]"))
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
                // search forwards
                sal_Unicode nSC = maGroupChars.GetChar( nMatchChar );
                sal_Unicode nEC = maGroupChars.GetChar( nMatchChar+1 );

                sal_uInt16 nCur = nPos+1;
                sal_uInt16 nLevel = 1;
                while ( nLevel && ( nPara < nParas ) )
                {
                    OUString aStr = GetText( nPara );
                    while ( nCur < aStr.getLength() )
                    {
                        if ( aStr[nCur] == nSC )
                            nLevel++;
                        else if ( aStr[nCur] == nEC )
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
                if ( nLevel == 0 )  // found
                {
                    aSel.GetStart() = rCursor;
                    aSel.GetEnd() = TextPaM( nPara, nCur+1 );
                }
            }
            else
            {
                // search backwards
                sal_Unicode nEC = maGroupChars.GetChar( nMatchChar );
                sal_Unicode nSC = maGroupChars.GetChar( nMatchChar-1 );

                sal_uInt16 nCur = rCursor.GetIndex()-1;
                sal_uInt16 nLevel = 1;
                while ( nLevel )
                {
                    if ( GetTextLen( nPara ) )
                    {
                        OUString aStr = GetText( nPara );
                        while ( nCur )
                        {
                            if ( aStr[nCur] == nSC )
                            {
                                nLevel--;
                                if ( !nLevel )
                                    break;  // while nCur...
                            }
                            else if ( aStr[nCur] == nEC )
                                nLevel++;

                            nCur--;
                        }
                    }

                    if ( nLevel )
                    {
                        if ( nPara )
                        {
                            nPara--;
                            nCur = GetTextLen( nPara )-1;   // no matter if negativ, as if Len()
                        }
                        else
                            break;
                    }
                }

                if ( nLevel == 0 )  // found
                {
                    aSel.GetStart() = rCursor;
                    aSel.GetStart().GetIndex()++;   // behind the char
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

    bool bSearchInSelection = (0 != (rSearchOptions.searchFlag & util::SearchFlags::REG_NOT_BEGINOFLINE) );

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
    aOptions.Locale = Application::GetSettings().GetLanguageTag().getLocale();
    utl::TextSearch aSearcher( rSearchOptions );

    // iterate over the paragraphs
    for ( sal_uLong nNode = nStartNode;
            bForward ?  ( nNode <= nEndNode) : ( nNode >= nEndNode );
            bForward ? nNode++ : nNode-- )
    {
        OUString aText = GetText( nNode );
        sal_Int32 nStartPos = 0;
        sal_Int32 nEndPos = aText.getLength();
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
            bFound = aSearcher.SearchForward( aText, &nStartPos, &nEndPos );
        else
            bFound = aSearcher.SearchBackward( aText, &nEndPos, &nStartPos );

        if ( bFound )
        {
            rSel.GetStart().GetPara() = nNode;
            rSel.GetStart().GetIndex() = nStartPos;
            rSel.GetEnd().GetPara() = nNode;
            rSel.GetEnd().GetIndex() = nEndPos;
            // Select over the paragraph?
            // FIXME  This should be max long...
            if( nEndPos == -1)
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

        if ( !bForward && !nNode )  // if searching backwards, if nEndNode == 0:
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
        // First add the beginning of the word to the selection,
        // so that the whole word is in the visible region.
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
            Search( rSearchOptions, bForward ); // right away to the next
        }
        else
        {
            if( Search( rSearchOptions, bForward ) )
                nFound = 1;
        }
    }
    else
    {
        // the writer replaces all, from beginning to end

        ExtTextEngine* pTextEngine = (ExtTextEngine*)GetTextEngine();

        // HideSelection();
        TextSelection aSel;

        bool bSearchInSelection = (0 != (rSearchOptions.searchFlag & util::SearchFlags::REG_NOT_BEGINOFLINE) );
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
        nEndPara--; // do not indent
    }

    for ( sal_uLong nPara = nStartPara; nPara <= nEndPara; nPara++ )
    {
        if ( bRight )
        {
            // add tabs
            GetTextEngine()->ImpInsertText( TextPaM( nPara, 0 ), '\t' );
            bDone = sal_True;
        }
        else
        {
            // remove Tabs/Blanks
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
