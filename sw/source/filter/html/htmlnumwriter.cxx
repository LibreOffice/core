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

#include <com/sun/star/style/NumberingType.hpp>
#include <hintids.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <svtools/htmlout.hxx>
#include <svl/urihelper.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/lrspitem.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <numrule.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <poolfmt.hxx>
#include <ndtxt.hxx>
#include <paratr.hxx>

#include "htmlnum.hxx"
#include "wrthtml.hxx"

#include <SwNodeNum.hxx>
#include <rtl/strbuf.hxx>

using namespace css;


void SwHTMLWriter::FillNextNumInfo()
{
    m_pNextNumRuleInfo = nullptr;

    sal_uLong nPos = pCurPam->GetPoint()->nNode.GetIndex() + 1;

    bool bTable = false;
    do
    {
        const SwNode* pNd = pDoc->GetNodes()[nPos];
        if( pNd->IsTextNode() )
        {
            // Der naechste wird als naechstes ausgegeben.
            m_pNextNumRuleInfo = new SwHTMLNumRuleInfo( *pNd->GetTextNode() );

            // Vor einer Tabelle behalten wir erst einmal die alte Ebene bei,
            // wenn die gleiche Numerierung hinter der Tabelle
            // fortgesetzt wird und dort nicht von vorne numeriert
            // wird. Die Tabelle wird ann beim Import so weit eingeruckt,
            // wie es der Num-Ebene entspricht.
            if( bTable &&
                m_pNextNumRuleInfo->GetNumRule()==GetNumInfo().GetNumRule() &&
                !m_pNextNumRuleInfo->IsRestart() )
            {
                m_pNextNumRuleInfo->SetDepth( GetNumInfo().GetDepth() );
            }
        }
        else if( pNd->IsTableNode() )
        {
            // Eine Tabelle wird uebersprungen, also den Node
            // hinter der Tabelle betrachten.
            nPos = pNd->EndOfSectionIndex() + 1;
            bTable = true;
        }
        else
        {
            // In allen anderen Faellen ist die Numerierung erstmal
            // zu Ende.
            m_pNextNumRuleInfo = new SwHTMLNumRuleInfo;
        }
    }
    while( !m_pNextNumRuleInfo );
}

void SwHTMLWriter::ClearNextNumInfo()
{
    delete m_pNextNumRuleInfo;
    m_pNextNumRuleInfo = nullptr;
}

Writer& OutHTML_NumBulListStart( SwHTMLWriter& rWrt,
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
                        rWrt.pCurPam->GetPoint()->nNode.GetIndex() + 1;
                    do
                    {
                        const SwNode* pNd = rWrt.pDoc->GetNodes()[nPos];
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
                                // node is numbered, but level is lower
                                // => check completed
                                bStartValue = false;
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

    OSL_ENSURE( rWrt.m_nLastParaToken == 0,
                "<PRE> wurde nicht vor <OL> beendet." );
    sal_uInt16 nPrevDepth =
        (bSameRule && !rInfo.IsRestart()) ? rPrevInfo.GetDepth() : 0;

    for( sal_uInt16 i=nPrevDepth; i<rInfo.GetDepth(); i++ )
    {
        rWrt.OutNewLine(); // <OL>/<UL> in eine neue Zeile

        rWrt.m_aBulletGrfs[i].clear();
        OString sOut = "<";
        const SwNumFormat& rNumFormat = rInfo.GetNumRule()->Get( i );
        sal_Int16 eType = rNumFormat.GetNumberingType();
        if( SVX_NUM_CHAR_SPECIAL == eType )
        {
            // Aufzaehlungs-Liste: <OL>
            sOut += OString(OOO_STRING_SVTOOLS_HTML_unorderlist);

            // den Typ ueber das Bullet-Zeichen bestimmen
            const sal_Char *pStr = nullptr;
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
                sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_type) + "=\"" + OString(pStr) + "\"";
            }
        }
        else if( SVX_NUM_BITMAP == eType )
        {
            // Unordered list: <UL>
            sOut += OString(OOO_STRING_SVTOOLS_HTML_unorderlist);
            rWrt.Strm().WriteOString( sOut );
            OutHTML_BulletImage( rWrt,
                                    nullptr,
                                    rNumFormat.GetBrush() );
        }
        else
        {
            // Ordered list: <OL>
            sOut += OString(OOO_STRING_SVTOOLS_HTML_orderlist);

            // den Typ ueber das Format bestimmen
            sal_Char cType = 0;
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
                sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_type) + "=\"" + OString(cType) + "\"";
            }

            sal_uInt16 nStartVal = rNumFormat.GetStart();
            if( bStartValue && 1 == nStartVal && i == rInfo.GetDepth()-1 )
            {
                // #i51089 - TUNING#
                if ( rWrt.pCurPam->GetNode().GetTextNode()->GetNum() )
                {
                    nStartVal = static_cast< sal_uInt16 >( rWrt.pCurPam->GetNode()
                                .GetTextNode()->GetNumberVector()[i] );
                }
                else
                {
                    OSL_FAIL( "<OutHTML_NumBulListStart(..) - text node has no number." );
                }
            }
            if( nStartVal != 1 )
            {
                sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_start) + "=\"" + OString::number(static_cast<sal_Int32>(nStartVal)) + "\"";
            }
        }

        if (!sOut.isEmpty())
            rWrt.Strm().WriteOString( sOut );

        if( rWrt.m_bCfgOutStyles )
            OutCSS1_NumBulListStyleOpt( rWrt, *rInfo.GetNumRule(), (sal_uInt8)i );

        rWrt.Strm().WriteChar( '>' );

        rWrt.IncIndentLevel(); // Inhalt von <OL> einruecken
    }

    return rWrt;
}

Writer& OutHTML_NumBulListEnd( SwHTMLWriter& rWrt,
                               const SwHTMLNumRuleInfo& rNextInfo )
{
    SwHTMLNumRuleInfo& rInfo = rWrt.GetNumInfo();
    bool bSameRule = rNextInfo.GetNumRule() == rInfo.GetNumRule();
    if( bSameRule && rNextInfo.GetDepth() >= rInfo.GetDepth() &&
        !rNextInfo.IsRestart() )
    {
        return rWrt;
    }

    OSL_ENSURE( rWrt.m_nLastParaToken == 0,
                "<PRE> wurde nicht vor </OL> beendet." );
    sal_uInt16 nNextDepth =
        (bSameRule && !rNextInfo.IsRestart()) ? rNextInfo.GetDepth() : 0;

    // MIB 23.7.97: Die Schleife muss doch rueckwaerts durchlaufen
    // werden, weil die Reihenfolge von </OL>/</UL> stimmen muss
    for( sal_uInt16 i=rInfo.GetDepth(); i>nNextDepth; i-- )
    {
        rWrt.DecIndentLevel(); // Inhalt von <OL> einruecken
        if( rWrt.m_bLFPossible )
            rWrt.OutNewLine(); // </OL>/</UL> in eine neue Zeile

        // es wird also eine Liste angefangen oder beendet:
        sal_Int16 eType = rInfo.GetNumRule()->Get( i-1 ).GetNumberingType();
        const sal_Char *pStr;
        if( SVX_NUM_CHAR_SPECIAL == eType || SVX_NUM_BITMAP == eType)
            pStr = OOO_STRING_SVTOOLS_HTML_unorderlist;
        else
            pStr = OOO_STRING_SVTOOLS_HTML_orderlist;
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), pStr, false );
        rWrt.m_bLFPossible = true;
    }

    return rWrt;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
