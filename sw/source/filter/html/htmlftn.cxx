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

#include <svtools/htmlout.hxx>
#include <svtools/htmlkywd.hxx>
#include <rtl/strbuf.hxx>
#include <ndindex.hxx>
#include <fmtftn.hxx>
#include <txtftn.hxx>
#include <ftninfo.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <charfmt.hxx>

#include "swhtml.hxx"
#include "wrthtml.hxx"

struct SwHTMLFootEndNote_Impl
{
    SwHTMLTxtFtns aTxtFtns;
    std::vector<OUString> aNames;

    OUString sName;
    OUString sContent;            // Infos fuer die letzte Fussnote
    sal_Bool bEndNote;
    sal_Bool bFixed;
};


sal_Int32 lcl_html_getNextPart( OUString& rPart, const OUString& rContent,
                             sal_Int32 nPos )
{
    rPart = aEmptyStr;
    sal_Int32 nLen = rContent.getLength();
    if( nPos >= nLen )
    {
        nPos = -1;
    }
    else
    {
        sal_Bool bQuoted = sal_False, bDone = sal_False;
        for( ; nPos < nLen && !bDone; nPos++ )
        {
            sal_Unicode c = rContent[nPos];
            switch( c )
            {
            case '\\':
                if( bQuoted )
                    rPart += OUString( c );
                bQuoted = !bQuoted;
                break;

            case ';':
                if( bQuoted )
                    rPart += OUString( c );
                else
                    bDone = sal_True;
                bQuoted = sal_False;
                break;

            default:
                rPart += OUString( c );
                bQuoted = sal_False;
                break;
            }
        }
    }

    return nPos;
}

sal_Int32 lcl_html_getEndNoteInfo( SwEndNoteInfo& rInfo,
                                    const OUString& rContent,
                                    sal_Bool bEndNote )
{
    sal_Int32 nStrPos = 0;
    for( sal_uInt16 nPart = 0; nPart < 4; nPart++ )
    {
        OUString aPart;
        if( -1 != nStrPos )
            nStrPos = lcl_html_getNextPart( aPart, rContent, nStrPos );

        switch( nPart )
        {
        case 0:
            rInfo.aFmt.SetNumberingType( static_cast< sal_Int16 >(bEndNote ? SVX_NUM_ROMAN_LOWER : SVX_NUM_ARABIC));
            if( !aPart.isEmpty() )
                rInfo.aFmt.SetNumberingType(SwHTMLParser::GetNumType( aPart,
                                                             rInfo.aFmt.GetNumberingType() ));
            break;

        case 1:
            rInfo.nFtnOffset = aPart.isEmpty() ? 0 : (sal_uInt16)aPart.toInt32();
            break;

        case 2:
            rInfo.SetPrefix( aPart );
            break;

        case 3:
            rInfo.SetSuffix( aPart );
            break;
        }
    }

    return nStrPos;
}

void SwHTMLParser::FillEndNoteInfo( const OUString& rContent )
{
    SwEndNoteInfo aInfo( pDoc->GetEndNoteInfo() );
    lcl_html_getEndNoteInfo( aInfo, rContent, sal_True );
    pDoc->SetEndNoteInfo( aInfo );
}

void SwHTMLParser::FillFootNoteInfo( const OUString& rContent )
{
    SwFtnInfo aInfo( pDoc->GetFtnInfo() );

    sal_Int32 nStrPos = lcl_html_getEndNoteInfo( aInfo, rContent, sal_False );

    for( sal_uInt16 nPart = 4; nPart < 8; nPart++ )
    {
        OUString aPart;
        if( -1 != nStrPos )
            nStrPos = lcl_html_getNextPart( aPart, rContent, nStrPos );

        switch( nPart )
        {
        case 4:
            aInfo.eNum = FTNNUM_DOC;
            if( !aPart.isEmpty() )
            {
                switch( aPart[0] )
                {
                case 'D': aInfo.eNum = FTNNUM_DOC; break;
                case 'C': aInfo.eNum = FTNNUM_CHAPTER; break;
                case 'P': aInfo.eNum = FTNNUM_PAGE; break;
                }
            }
            break;

        case 5:
            aInfo.ePos = FTNPOS_PAGE;
            if( !aPart.isEmpty() )
            {
                switch( aPart[0] )
                {
                case 'C': aInfo.ePos = FTNPOS_CHAPTER; break;
                case 'P': aInfo.ePos = FTNPOS_PAGE; break;
                }
            }
            break;

        case 6:
            aInfo.aQuoVadis = aPart;
            break;

        case 7:
            aInfo.aErgoSum = aPart;
            break;
        }
    }

    pDoc->SetFtnInfo( aInfo );
}

void SwHTMLParser::InsertFootEndNote( const OUString& rName, sal_Bool bEndNote,
                                      sal_Bool bFixed )
{
    if( !pFootEndNoteImpl )
        pFootEndNoteImpl = new SwHTMLFootEndNote_Impl;

    pFootEndNoteImpl->sName = rName;
    if( pFootEndNoteImpl->sName.getLength() > 3 )
        pFootEndNoteImpl->sName = pFootEndNoteImpl->sName.copy( 0, pFootEndNoteImpl->sName.getLength() - 3 );
    pFootEndNoteImpl->sName = pFootEndNoteImpl->sName.toAsciiUpperCase();
    pFootEndNoteImpl->bEndNote = bEndNote;
    pFootEndNoteImpl->bFixed = bFixed;
    pFootEndNoteImpl->sContent = aEmptyStr;
}

void SwHTMLParser::FinishFootEndNote()
{
    if( !pFootEndNoteImpl )
        return;

    SwFmtFtn aFtn( pFootEndNoteImpl->bEndNote );
    if( pFootEndNoteImpl->bFixed )
        aFtn.SetNumStr( pFootEndNoteImpl->sContent );

    pDoc->InsertPoolItem( *pPam, aFtn, 0 );
    SwTxtFtn * const pTxtFtn = static_cast<SwTxtFtn *>(
        pPam->GetNode()->GetTxtNode()->GetTxtAttrForCharAt(
            pPam->GetPoint()->nContent.GetIndex() - 1, RES_TXTATR_FTN ) );
    // In Kopf- und Fusszeilen duerfen keine Fussnoten eingefuegt werden.
    if( pTxtFtn )
    {
        pFootEndNoteImpl->aTxtFtns.push_back( pTxtFtn );
        pFootEndNoteImpl->aNames.push_back(pFootEndNoteImpl->sName);
    }
    pFootEndNoteImpl->sName = aEmptyStr;
    pFootEndNoteImpl->sContent = aEmptyStr;
    pFootEndNoteImpl->bFixed = sal_False;
}

void SwHTMLParser::InsertFootEndNoteText()
{
    if( pFootEndNoteImpl && pFootEndNoteImpl->bFixed )
        pFootEndNoteImpl->sContent += aToken;
}

void SwHTMLParser::DeleteFootEndNoteImpl()
{
    delete pFootEndNoteImpl;
    pFootEndNoteImpl = 0;
}

SwNodeIndex *SwHTMLParser::GetFootEndNoteSection( const OUString& rName )
{
    SwNodeIndex *pStartNodeIdx = 0;

    if( pFootEndNoteImpl )
    {
        OUString aName(rName.toAsciiUpperCase());

        size_t nCount = pFootEndNoteImpl->aNames.size();
        for(size_t i = 0; i < nCount; ++i)
        {
            if(pFootEndNoteImpl->aNames[i] == aName)
            {
                pStartNodeIdx = pFootEndNoteImpl->aTxtFtns[i]->GetStartNode();
                pFootEndNoteImpl->aNames.erase(pFootEndNoteImpl->aNames.begin() + i);
                pFootEndNoteImpl->aTxtFtns.erase( pFootEndNoteImpl->aTxtFtns.begin() + i );
                if(pFootEndNoteImpl->aNames.empty())
                {
                    delete pFootEndNoteImpl;
                    pFootEndNoteImpl = 0;
                }

                break;
            }
        }
    }

    return pStartNodeIdx;
}

Writer& OutHTML_SwFmtFtn( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    SwFmtFtn& rFmtFtn = (SwFmtFtn&)rHt;
    SwTxtFtn *pTxtFtn = rFmtFtn.GetTxtFtn();
    if( !pTxtFtn )
        return rWrt;

    OUString sFtnName, sClass;
    sal_uInt16 nPos;
    if( rFmtFtn.IsEndNote() )
    {
        nPos = rHTMLWrt.pFootEndNotes ? rHTMLWrt.pFootEndNotes->size() : 0;
        OSL_ENSURE( nPos == rHTMLWrt.nFootNote + rHTMLWrt.nEndNote,
                "OutHTML_SwFmtFtn: Position falsch" );
        sClass = OOO_STRING_SVTOOLS_HTML_sdendnote_anc;
        sFtnName = OOO_STRING_SVTOOLS_HTML_sdendnote + OUString::number( (sal_Int32)(++rHTMLWrt.nEndNote) );
    }
    else
    {
        nPos = rHTMLWrt.nFootNote;
        sClass = OOO_STRING_SVTOOLS_HTML_sdfootnote_anc;
        sFtnName = OOO_STRING_SVTOOLS_HTML_sdfootnote + OUString::number( (sal_Int32)(++rHTMLWrt.nFootNote));
    }

    if( !rHTMLWrt.pFootEndNotes )
        rHTMLWrt.pFootEndNotes = new SwHTMLTxtFtns;
    rHTMLWrt.pFootEndNotes->insert( rHTMLWrt.pFootEndNotes->begin() + nPos, pTxtFtn );

    OStringBuffer sOut;
    sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_anchor).append(' ')
        .append(OOO_STRING_SVTOOLS_HTML_O_class).append("=\"");
    rWrt.Strm() << sOut.makeStringAndClear().getStr();
    HTMLOutFuncs::Out_String( rWrt.Strm(), sClass, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
    sOut.append("\" ").append(OOO_STRING_SVTOOLS_HTML_O_name).append("=\"");
    rWrt.Strm() << sOut.makeStringAndClear().getStr();
    HTMLOutFuncs::Out_String( rWrt.Strm(), sFtnName, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
    sOut.append(OOO_STRING_SVTOOLS_HTML_FTN_anchor).append("\" ")
        .append(OOO_STRING_SVTOOLS_HTML_O_href).append("=\"#");
    rWrt.Strm() << sOut.makeStringAndClear().getStr();
    HTMLOutFuncs::Out_String( rWrt.Strm(), sFtnName, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
    sOut.append(OOO_STRING_SVTOOLS_HTML_FTN_symbol).append('\"');
    if( !rFmtFtn.GetNumStr().isEmpty() )
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_sdfixed);
    sOut.append('>');
    rWrt.Strm() << sOut.makeStringAndClear().getStr();
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_superscript, sal_True );

    HTMLOutFuncs::Out_String( rWrt.Strm(), rFmtFtn.GetViewNumStr(*rWrt.pDoc),
                                 rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_superscript, sal_False );
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_anchor, sal_False );

    return rWrt;
}

void SwHTMLWriter::OutFootEndNotes()
{
    OSL_ENSURE( pFootEndNotes,
            "SwHTMLWriter::OutFootEndNotes(): unnoetiger Aufruf" );
    if( !pFootEndNotes )
        return;

#if OSL_DEBUG_LEVEL > 0
    sal_uInt16 nFtn = nFootNote, nEn = nEndNote;
#endif
    nFootNote = 0, nEndNote = 0;

    for( sal_uInt16 i=0; i<pFootEndNotes->size(); i++ )
    {
        SwTxtFtn *pTxtFtn = (*pFootEndNotes)[i];
        pFmtFtn = &pTxtFtn->GetFtn();

        String sFtnName, sClass;
        if( pFmtFtn->IsEndNote() )
        {
            sClass.AssignAscii( OOO_STRING_SVTOOLS_HTML_sdendnote );
            sFtnName.AssignAscii( OOO_STRING_SVTOOLS_HTML_sdendnote );
            sFtnName.Append( OUString::number((sal_Int32)(++nEndNote)) );
        }
        else
        {
            sClass.AssignAscii( OOO_STRING_SVTOOLS_HTML_sdfootnote );
            sFtnName.AssignAscii( OOO_STRING_SVTOOLS_HTML_sdfootnote );
            sFtnName.Append( OUString::number((sal_Int32)(++nFootNote)));
        }

        if( bLFPossible )
            OutNewLine();
        OStringBuffer sOut;
        sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_division)
            .append(' ').append(OOO_STRING_SVTOOLS_HTML_O_id).append("=\"");
        Strm() << sOut.makeStringAndClear().getStr();
        HTMLOutFuncs::Out_String( Strm(), sFtnName, eDestEnc, &aNonConvertableCharacters );
        Strm() << "\">";

        bLFPossible = sal_True;
        IncIndentLevel();   // Inhalt von <DIV> einruecken

        OSL_ENSURE( pTxtFtn, "SwHTMLWriter::OutFootEndNotes: SwTxtFtn fehlt" );
        SwNodeIndex *pSttNdIdx = pTxtFtn->GetStartNode();
        OSL_ENSURE( pSttNdIdx,
                "SwHTMLWriter::OutFootEndNotes: StartNode-Index fehlt" );
        if( pSttNdIdx )
        {
            HTMLSaveData aSaveData( *this, pSttNdIdx->GetIndex()+1,
                pSttNdIdx->GetNode().EndOfSectionIndex(), sal_False );
            Out_SwDoc( pCurPam );
        }

        DecIndentLevel();   // Inhalt von <DIV> einruecken
        if( bLFPossible )
            OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_division, sal_False );
        bLFPossible = sal_True;

        OSL_ENSURE( !pFmtFtn,
                "SwHTMLWriter::OutFootEndNotes: Ftn wurde nicht ausgegeben" );
        if( pFmtFtn )
        {
            if( pFmtFtn->IsEndNote() )
                nEndNote++;
            else
                nFootNote++;

            pFmtFtn = 0;
        }
    }

#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE( nFtn == nFootNote,
            "SwHTMLWriter::OutFootEndNotes: Anzahl Fussnoten stimmt nicht" );
    OSL_ENSURE( nEn == nEndNote,
            "SwHTMLWriter::OutFootEndNotes: Anzahl Endnoten stimmt nicht" );
#endif

    delete pFootEndNotes;
    pFootEndNotes = 0;
    nFootNote = nEndNote = 0;
}

String SwHTMLWriter::GetFootEndNoteSym( const SwFmtFtn& rFmtFtn )
{
    const SwEndNoteInfo * pInfo = 0;
    if( rFmtFtn.GetNumStr().isEmpty() )
        pInfo = rFmtFtn.IsEndNote() ? &pDoc->GetEndNoteInfo()
                                    : &pDoc->GetFtnInfo();

    String sRet;
    if( pInfo )
        sRet = pInfo->GetPrefix();
    sRet += rFmtFtn.GetViewNumStr( *pDoc );
    if( pInfo )
        sRet += pInfo->GetSuffix();

    return sRet;
}

void SwHTMLWriter::OutFootEndNoteSym( const SwFmtFtn& rFmtFtn,
                                         const String& rNum,
                                         sal_uInt16 nScript )
{
    const SwEndNoteInfo *pInfo;

    String sFtnName, sClass;
    if( rFmtFtn.IsEndNote() )
    {
        sClass.AssignAscii( OOO_STRING_SVTOOLS_HTML_sdendnote_sym );
        sFtnName.AssignAscii( OOO_STRING_SVTOOLS_HTML_sdendnote );
        sFtnName.Append( OUString::number((sal_Int32)nEndNote) );
        pInfo = &pDoc->GetEndNoteInfo();
    }
    else
    {
        sClass.AssignAscii( OOO_STRING_SVTOOLS_HTML_sdfootnote_sym );
        sFtnName.AssignAscii( OOO_STRING_SVTOOLS_HTML_sdfootnote );
        sFtnName.Append( OUString::number((sal_Int32)nFootNote));
        pInfo = &pDoc->GetFtnInfo();
    }

    const SwCharFmt *pSymCharFmt = pInfo->GetCharFmt( *pDoc );
    if( pSymCharFmt && 0 != aScriptTextStyles.count( pSymCharFmt->GetName() ) )
    {
        switch( nScript )
        {
        case CSS1_OUTMODE_WESTERN:
            sClass.AppendAscii( RTL_CONSTASCII_STRINGPARAM("-western") );
            break;
        case CSS1_OUTMODE_CJK:
            sClass.AppendAscii( RTL_CONSTASCII_STRINGPARAM("-cjk") );
            break;
        case CSS1_OUTMODE_CTL:
            sClass.AppendAscii( RTL_CONSTASCII_STRINGPARAM("-ctl") );
            break;
        }
    }

    OStringBuffer sOut;
    sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_anchor).append(' ')
        .append(OOO_STRING_SVTOOLS_HTML_O_class).append("=\"");
    Strm() << sOut.makeStringAndClear().getStr();
    HTMLOutFuncs::Out_String( Strm(), sClass, eDestEnc, &aNonConvertableCharacters );
    sOut.append("\" ").append(OOO_STRING_SVTOOLS_HTML_O_name).append("=\"");
    Strm() << sOut.makeStringAndClear().getStr();
    HTMLOutFuncs::Out_String( Strm(), sFtnName, eDestEnc, &aNonConvertableCharacters );
    sOut.append(OOO_STRING_SVTOOLS_HTML_FTN_symbol).append("\" ")
        .append(OOO_STRING_SVTOOLS_HTML_O_href).append("=\"#");
    Strm() << sOut.makeStringAndClear().getStr();
    HTMLOutFuncs::Out_String( Strm(), sFtnName, eDestEnc, &aNonConvertableCharacters );
    sOut.append(OOO_STRING_SVTOOLS_HTML_FTN_anchor).append("\">");
    Strm() << sOut.makeStringAndClear().getStr();

    HTMLOutFuncs::Out_String( Strm(), rNum, eDestEnc, &aNonConvertableCharacters );
    HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_anchor, sal_False );
}

static sal_uInt16 lcl_html_fillEndNoteInfo( const SwEndNoteInfo& rInfo,
                                 OUString *pParts,
                                 sal_Bool bEndNote  )
{
    sal_uInt16 nParts = 0;
    sal_Int16 eFmt = rInfo.aFmt.GetNumberingType();
    if( (bEndNote ? SVX_NUM_ROMAN_LOWER : SVX_NUM_ARABIC) != eFmt )
    {
        const sal_Char *pStr = SwHTMLWriter::GetNumFormat( eFmt );
        if( pStr )
        {
            pParts[0] = OUString::createFromAscii( pStr );
            nParts = 1;
        }
    }
    if( rInfo.nFtnOffset > 0 )
    {
        pParts[1] = OUString::number(rInfo.nFtnOffset);
        nParts = 2;
    }
    if( !rInfo.GetPrefix().isEmpty() )
    {
        pParts[2] = rInfo.GetPrefix();
        nParts = 3;
    }
    if( !rInfo.GetSuffix().isEmpty() )
    {
        pParts[3] = rInfo.GetSuffix();
        nParts = 4;
    }

    return nParts;
}

static void lcl_html_outFootEndNoteInfo( Writer& rWrt, OUString *pParts,
                                  sal_uInt16 nParts, const sal_Char *pName )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    OUString aContent;
    for( sal_uInt16 i=0; i<nParts; i++ )
    {
        OUString aTmp( pParts[i] );
        aTmp = aTmp.replaceAll( "\\", "\\\\" );
        aTmp = aTmp.replaceAll( ";", "\\;" );
        if( i > 0 )
            aContent += ";";
        aContent += aTmp;
    }

    rHTMLWrt.OutNewLine();
    OStringBuffer sOut;
    sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_meta).append(' ')
        .append(OOO_STRING_SVTOOLS_HTML_O_name).append("=\"").append(pName)
        .append("\" ").append(OOO_STRING_SVTOOLS_HTML_O_content).append("=\"");
    rWrt.Strm() << sOut.makeStringAndClear().getStr();
    HTMLOutFuncs::Out_String( rWrt.Strm(), aContent, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
    rWrt.Strm() << "\">";
}

void SwHTMLWriter::OutFootEndNoteInfo()
{
    // Nummerntyp (1 bzw. i)
    // Offset (0)
    // Davor
    // Dahinter
    // Dok/Seite/Kap (D)
    // Position (S)
    // Folgeseite
    // Beginn

    {
        const SwFtnInfo& rInfo = pDoc->GetFtnInfo();
        OUString aParts[8];
        sal_uInt16 nParts = lcl_html_fillEndNoteInfo( rInfo, aParts, sal_False );
        if( rInfo.eNum != FTNNUM_DOC )
        {
            aParts[4] = rInfo.eNum == FTNNUM_CHAPTER ? OUString( "C" ) : OUString( "P" );
            nParts = 5;
        }
        if( rInfo.ePos != FTNPOS_PAGE)
        {
            aParts[5] = "C";
            nParts = 6;
        }
        if( rInfo.aQuoVadis.Len() > 0 )
        {
            aParts[6] = rInfo.aQuoVadis;
            nParts = 7;
        }
        if( rInfo.aErgoSum.Len() > 0 )
        {
            aParts[7] = rInfo.aErgoSum;
            nParts = 8;
        }
        if( nParts > 0 )
            lcl_html_outFootEndNoteInfo( *this, aParts, nParts,
                                         OOO_STRING_SVTOOLS_HTML_META_sdfootnote );
    }

    {
        const SwEndNoteInfo& rInfo = pDoc->GetEndNoteInfo();
        OUString aParts[4];
        sal_uInt16 nParts = lcl_html_fillEndNoteInfo( rInfo, aParts, sal_True );
        if( nParts > 0 )
            lcl_html_outFootEndNoteInfo( *this, aParts, nParts,
                                         OOO_STRING_SVTOOLS_HTML_META_sdendnote );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
