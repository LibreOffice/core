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

#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <svtools/htmlout.hxx>
#include <numrule.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>

#include "htmlnum.hxx"
#include "wrthtml.hxx"

#include <osl/diagnose.h>

using namespace css;


void SwHTMLWriter::FillNextNumInfo()
{
    m_pNextNumRuleInfo = nullptr;

    sal_uLong nPos = m_pCurrentPam->GetPoint()->nNode.GetIndex() + 1;

    bool bTable = false;
    do
    {
        const SwNode* pNd = m_pDoc->GetNodes()[nPos];
        if( pNd->IsTextNode() )
        {
            m_pNextNumRuleInfo.reset( new SwHTMLNumRuleInfo( *pNd->GetTextNode() ) );

            // Before a table we keep the old level if the same numbering is
            // continued after the table and no new numbering is started.
            // The table will get the indentation that corresponds to its
            // numbering level during import.
            if( bTable &&
                m_pNextNumRuleInfo->GetNumRule()==GetNumInfo().GetNumRule() &&
                !m_pNextNumRuleInfo->IsRestart() )
            {
                m_pNextNumRuleInfo->SetDepth( GetNumInfo().GetDepth() );
            }
        }
        else if( pNd->IsTableNode() )
        {
            // A table is skipped so the node after table is viewed.
            nPos = pNd->EndOfSectionIndex() + 1;
            bTable = true;
        }
        else
        {
            // In all other case the numbering is over.
            m_pNextNumRuleInfo.reset(new SwHTMLNumRuleInfo);
        }
    }
    while( !m_pNextNumRuleInfo );
}

void SwHTMLWriter::ClearNextNumInfo()
{
    m_pNextNumRuleInfo.reset();
}

void SwHTMLWriter::SetNextNumInfo( std::unique_ptr<SwHTMLNumRuleInfo> pNxt )
{
    m_pNextNumRuleInfo = std::move(pNxt);
}

Writer& OutHTML_NumberBulletListStart( SwHTMLWriter& rWrt,
                                 const SwHTMLNumRuleInfo& rInfo )
{
    SwHTMLNumRuleInfo& rPrevInfo = rWrt.GetNumInfo();
    bool bSameRule = rPrevInfo.GetNumRule() == rInfo.GetNumRule();
    if( bSameRule && rPrevInfo.GetDepth() >= rInfo.GetDepth() &&
        !rInfo.IsRestart() )
    {
        return rWrt;
    }

    bool bStartValue = false;
    if( !bSameRule && rInfo.GetDepth() )
    {
        OUString aName( rInfo.GetNumRule()->GetName() );
        if( 0 != rWrt.m_aNumRuleNames.count( aName ) )
        {
            // The rule has been applied before
            sal_Int16 eType = rInfo.GetNumRule()
                ->Get( rInfo.GetDepth()-1 ).GetNumberingType();
            if( SVX_NUM_CHAR_SPECIAL != eType && SVX_NUM_BITMAP != eType )
            {
                // If it's a numbering rule, the current number should be
                // exported as start value, but only if there are no nodes
                // within the numbering that have a lower level
                bStartValue = true;
                if( rInfo.GetDepth() > 1 )
                {
                    sal_uLong nPos =
                        rWrt.m_pCurrentPam->GetPoint()->nNode.GetIndex() + 1;
                    do
                    {
                        const SwNode* pNd = rWrt.m_pDoc->GetNodes()[nPos];
                        if( pNd->IsTextNode() )
                        {
                            const SwTextNode *pTextNd = pNd->GetTextNode();
                            if( !pTextNd->GetNumRule() )
                            {
                                // node isn't numbered => check completed
                                break;
                            }

                            OSL_ENSURE(! pTextNd->IsOutline(),
                                   "outline not expected");

                            if( pTextNd->GetActualListLevel() + 1 <
                                rInfo.GetDepth() )
                            {
                                if (rPrevInfo.GetDepth() == 0)
                                    // previous node had no numbering => write start value
                                    bStartValue = true;
                                else
                                    // node is numbered, but level is lower
                                    bStartValue = false;
                                // => check completed
                                break;
                            }
                            nPos++;
                        }
                        else if( pNd->IsTableNode() )
                        {
                            // skip table
                            nPos = pNd->EndOfSectionIndex() + 1;
                        }
                        else
                        {
                            // end node or sections start node -> check
                            // completed
                            break;
                        }
                    }
                    while( true );
                }
            }
        }
        else
        {
            rWrt.m_aNumRuleNames.insert( aName );
        }
    }

    OSL_ENSURE( rWrt.m_nLastParaToken == HtmlTokenId::NONE,
                "<PRE> was not closed before <OL>." );
    sal_uInt16 nPrevDepth =
        (bSameRule && !rInfo.IsRestart()) ? rPrevInfo.GetDepth() : 0;

    for( sal_uInt16 i=nPrevDepth; i<rInfo.GetDepth(); i++ )
    {
        rWrt.OutNewLine(); // <OL>/<UL> in a new row

        rWrt.m_aBulletGrfs[i].clear();
        OString sOut = "<" + rWrt.GetNamespace();
        const SwNumFormat& rNumFormat = rInfo.GetNumRule()->Get( i );
        sal_Int16 eType = rNumFormat.GetNumberingType();
        if( SVX_NUM_CHAR_SPECIAL == eType )
        {
            // ordered list: <OL>
            sOut += OString(OOO_STRING_SVTOOLS_HTML_unorderlist);

            // determine the type by the bullet character
            const char *pStr = nullptr;
            switch( rNumFormat.GetBulletChar() )
            {
            case HTML_BULLETCHAR_DISC:
                pStr = OOO_STRING_SVTOOLS_HTML_ULTYPE_disc;
                break;
            case HTML_BULLETCHAR_CIRCLE:
                pStr = OOO_STRING_SVTOOLS_HTML_ULTYPE_circle;
                break;
            case HTML_BULLETCHAR_SQUARE:
                pStr = OOO_STRING_SVTOOLS_HTML_ULTYPE_square;
                break;
            }

            if( pStr )
            {
                sOut += OString::Concat(" " OOO_STRING_SVTOOLS_HTML_O_type "=\"") + pStr + "\"";
            }
        }
        else if( SVX_NUM_BITMAP == eType )
        {
            // Unordered list: <UL>
            sOut += OString(OOO_STRING_SVTOOLS_HTML_unorderlist);
            rWrt.Strm().WriteOString( sOut );
            OutHTML_BulletImage( rWrt,
                                    nullptr,
                                    rNumFormat.GetBrush(),
                                    rWrt.m_aBulletGrfs[i]);
        }
        else
        {
            // Ordered list: <OL>
            sOut += OString(OOO_STRING_SVTOOLS_HTML_orderlist);

            // determine the type by the format
            char cType = 0;
            switch( eType )
            {
                case SVX_NUM_CHARS_UPPER_LETTER:
                case SVX_NUM_CHARS_UPPER_LETTER_N:
                    cType = 'A';
                    break;
                case SVX_NUM_CHARS_LOWER_LETTER:
                case SVX_NUM_CHARS_LOWER_LETTER_N:
                    cType = 'a';
                    break;
                case SVX_NUM_ROMAN_UPPER:
                    cType = 'I';
                    break;
                case SVX_NUM_ROMAN_LOWER:
                    cType = 'i';
                    break;
            }
            if( cType )
            {
                sOut += " " OOO_STRING_SVTOOLS_HTML_O_type "=\"" + OStringChar(cType) + "\"";
            }

            sal_uInt16 nStartVal = rNumFormat.GetStart();
            if( bStartValue && 1 == nStartVal && i == rInfo.GetDepth()-1 )
            {
                if ( rWrt.m_pCurrentPam->GetNode().GetTextNode()->GetNum() )
                {
                    nStartVal = static_cast< sal_uInt16 >( rWrt.m_pCurrentPam->GetNode()
                                .GetTextNode()->GetNumberVector()[i] );
                }
                else
                {
                    OSL_FAIL( "<OutHTML_NumberBulletListStart(..) - text node has no number." );
                }
            }
            if( nStartVal != 1 )
            {
                sOut += " " OOO_STRING_SVTOOLS_HTML_O_start "=\"" + OString::number(static_cast<sal_Int32>(nStartVal)) + "\"";
            }
        }

        if (!sOut.isEmpty() && SVX_NUM_BITMAP != eType)  // second condition to avoid adding extra ul, already done before.
            rWrt.Strm().WriteOString( sOut );

        if( rWrt.m_bCfgOutStyles )
            OutCSS1_NumberBulletListStyleOpt( rWrt, *rInfo.GetNumRule(), static_cast<sal_uInt8>(i) );

        rWrt.Strm().WriteChar( '>' );

        rWrt.IncIndentLevel(); // indent content of <OL>
    }

    return rWrt;
}

Writer& OutHTML_NumberBulletListEnd( SwHTMLWriter& rWrt,
                               const SwHTMLNumRuleInfo& rNextInfo )
{
    SwHTMLNumRuleInfo& rInfo = rWrt.GetNumInfo();
    bool bSameRule = rNextInfo.GetNumRule() == rInfo.GetNumRule();
    bool bListEnd = !bSameRule || rNextInfo.GetDepth() < rInfo.GetDepth() || rNextInfo.IsRestart();

    if (rWrt.mbXHTML)
    {
        if (bListEnd || (!bListEnd && rNextInfo.IsNumbered()))
        {
            HTMLOutFuncs::Out_AsciiTag(rWrt.Strm(),
                                       rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_li, false);
        }
    }

    if (!bListEnd)
    {
        return rWrt;
    }

    OSL_ENSURE( rWrt.m_nLastParaToken == HtmlTokenId::NONE,
                "<PRE> was not closed before </OL>." );
    sal_uInt16 nNextDepth =
        (bSameRule && !rNextInfo.IsRestart()) ? rNextInfo.GetDepth() : 0;

    // MIB 23.7.97: We must loop backwards, to get the right order of </OL>/</UL>
    for( sal_uInt16 i=rInfo.GetDepth(); i>nNextDepth; i-- )
    {
        rWrt.DecIndentLevel(); // indent content of <OL>
        if( rWrt.m_bLFPossible )
            rWrt.OutNewLine(); // </OL>/</UL> in a new line

        // a list is started or ended:
        sal_Int16 eType = rInfo.GetNumRule()->Get( i-1 ).GetNumberingType();
        OString aTag;
        if( SVX_NUM_CHAR_SPECIAL == eType || SVX_NUM_BITMAP == eType)
            aTag = OOO_STRING_SVTOOLS_HTML_unorderlist;
        else
            aTag = OOO_STRING_SVTOOLS_HTML_orderlist;
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rWrt.GetNamespace() + aTag, false );
        rWrt.m_bLFPossible = true;
    }

    return rWrt;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
