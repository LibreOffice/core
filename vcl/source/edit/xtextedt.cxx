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

#include <i18nutil/searchopt.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/xtextedt.hxx>
#include <vcl/textview.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <unotools/textsearch.hxx>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchFlags.hpp>

using namespace ::com::sun::star;

static const std::wstring gaGroupChars = L"(){}[]";

ExtTextEngine::ExtTextEngine()
{
}

ExtTextEngine::~ExtTextEngine()
{
}

TextSelection ExtTextEngine::MatchGroup( const TextPaM& rCursor ) const
{
    TextSelection aSel( rCursor );
    const sal_Int32 nPos = rCursor.GetIndex();
    sal_uInt32 nPara = rCursor.GetPara();
    const sal_uInt32 nParas = GetParagraphCount();
    if ( ( nPara < nParas ) && ( nPos < GetTextLen( nPara ) ) )
    {
        size_t nMatchIndex = gaGroupChars.find( GetText( rCursor.GetPara() )[ nPos ] );
        if ( nMatchIndex != std::wstring::npos )
        {
            if ( ( nMatchIndex % 2 ) == 0 )
            {
                // search forwards
                sal_Unicode nSC = gaGroupChars[ nMatchIndex ];
                sal_Unicode nEC = gaGroupChars[ nMatchIndex+1 ];

                sal_Int32 nCur = nPos+1;
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
                sal_Unicode nEC = gaGroupChars[ nMatchIndex ];
                sal_Unicode nSC = gaGroupChars[ nMatchIndex-1 ];

                sal_Int32 nCur = rCursor.GetIndex()-1;
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
                            nCur = GetTextLen( nPara )-1;   // no matter if negative, as if Len()
                        }
                        else
                            break;
                    }
                }

                if ( nLevel == 0 )  // found
                {
                    aSel.GetStart() = rCursor;
                    ++aSel.GetStart().GetIndex();   // behind the char
                    aSel.GetEnd() = TextPaM( nPara, nCur );
                }
            }
        }
    }
    return aSel;
}

bool ExtTextEngine::Search( TextSelection& rSel, const i18nutil::SearchOptions& rSearchOptions, bool bForward )
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
    sal_uInt32 nEndNode;

    if ( bSearchInSelection )
        nEndNode = bForward ? aSel.GetEnd().GetPara() : aSel.GetStart().GetPara();
    else
        nEndNode = bForward ? (GetParagraphCount()-1) : 0;

    const sal_uInt32 nStartNode = aStartPaM.GetPara();

    i18nutil::SearchOptions aOptions( rSearchOptions );
    aOptions.Locale = Application::GetSettings().GetLanguageTag().getLocale();
    utl::TextSearch aSearcher( utl::TextSearch::UpgradeToSearchOptions2(aOptions) );

    // iterate over the paragraphs
    for ( sal_uInt32 nNode = nStartNode;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
