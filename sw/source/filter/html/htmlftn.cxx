/*************************************************************************
 *
 *  $RCSfile: htmlftn.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2001-02-26 07:35:34 $
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


#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HTMLOUT_HXX //autogen
#include <svtools/htmlout.hxx>
#endif
#ifndef _HTMLKYWD_HXX
#include <svtools/htmlkywd.hxx>
#endif

#ifndef _ERRHDL_HXX //autogen
#include <errhdl.hxx>
#endif
#ifndef _NDINDEX_HXX //autogen
#include <ndindex.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FTNINFO_HXX //autogen
#include <ftninfo.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _NDTXT_HXX //autogen
#include <ndtxt.hxx>
#endif

#include "swhtml.hxx"
#include "wrthtml.hxx"

typedef SwTxtFtn *SwTxtFtnPtr;
SV_DECL_PTRARR( SwHTMLTxtFtns, SwTxtFtnPtr, 1, 1 )

struct SwHTMLFootEndNote_Impl
{
    SwHTMLTxtFtns aTxtFtns;
    SvStringsDtor aNames;

    String sName;
    String sContent;            // Infos fuer die letzte Fussnote
    BOOL bEndNote;
    BOOL bFixed;
};


xub_StrLen lcl_html_getNextPart( String& rPart, const String& rContent,
                             xub_StrLen nPos )
{
    rPart = aEmptyStr;
    xub_StrLen nLen = rContent.Len();
    if( nPos >= nLen )
    {
        nPos = STRING_MAXLEN;
    }
    else
    {
        BOOL bQuoted = FALSE, bDone = FALSE;
        for( ; nPos < nLen && !bDone; nPos++ )
        {
            sal_Unicode c = rContent.GetChar( nPos );
            switch( c )
            {
            case '\\':
                if( bQuoted )
                    rPart += c;
                bQuoted = !bQuoted;
                break;

            case ';':
                if( bQuoted )
                    rPart += c;
                else
                    bDone = TRUE;
                bQuoted = FALSE;
                break;

            default:
                rPart += c;
                bQuoted = FALSE;
                break;
            }
        }
    }

    return nPos;
}

xub_StrLen lcl_html_getEndNoteInfo( SwEndNoteInfo& rInfo,
                                    const String& rContent,
                                    BOOL bEndNote )
{
    xub_StrLen nStrPos = 0;
    for( USHORT nPart = 0; nPart < 4; nPart++ )
    {
        String aPart;
        if( STRING_MAXLEN != nStrPos )
            nStrPos = lcl_html_getNextPart( aPart, rContent, nStrPos );

        switch( nPart )
        {
        case 0:
            rInfo.aFmt.SetNumberingType(bEndNote ? SVX_NUM_ROMAN_LOWER : SVX_NUM_ARABIC);
            if( aPart.Len() )
                rInfo.aFmt.SetNumberingType(SwHTMLParser::GetNumType( aPart,
                                                             rInfo.aFmt.GetNumberingType() ));
            break;

        case 1:
            rInfo.nFtnOffset = aPart.Len() == 0 ? 0 : (USHORT)aPart.ToInt32();
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

void SwHTMLParser::FillEndNoteInfo( const String& rContent )
{
    SwEndNoteInfo aInfo( pDoc->GetEndNoteInfo() );
    lcl_html_getEndNoteInfo( aInfo, rContent, TRUE );
    pDoc->SetEndNoteInfo( aInfo );
}

void SwHTMLParser::FillFootNoteInfo( const String& rContent )
{
    SwFtnInfo aInfo( pDoc->GetFtnInfo() );

    xub_StrLen nStrPos = lcl_html_getEndNoteInfo( aInfo, rContent, FALSE );

    for( USHORT nPart = 4; nPart < 8; nPart++ )
    {
        String aPart;
        if( STRING_MAXLEN != nStrPos )
            nStrPos = lcl_html_getNextPart( aPart, rContent, nStrPos );

        switch( nPart )
        {
        case 4:
            aInfo.eNum = FTNNUM_DOC;
            if( aPart.Len() )
            {
                switch( aPart.GetChar(0) )
                {
                case 'D': aInfo.eNum = FTNNUM_DOC; break;
                case 'C': aInfo.eNum = FTNNUM_CHAPTER; break;
                case 'P': aInfo.eNum = FTNNUM_PAGE; break;
                }
            }
            break;

        case 5:
            aInfo.ePos = FTNPOS_PAGE;
            if( aPart.Len() )
            {
                switch( aPart.GetChar(0) )
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

void SwHTMLParser::InsertFootEndNote( const String& rName, BOOL bEndNote,
                                      BOOL bFixed )
{
    if( !pFootEndNoteImpl )
        pFootEndNoteImpl = new SwHTMLFootEndNote_Impl;

    pFootEndNoteImpl->sName = rName;
    if( pFootEndNoteImpl->sName.Len() > 3 )
        pFootEndNoteImpl->sName.Erase( pFootEndNoteImpl->sName.Len() - 3 );
    // TODO: ToUpperAscii???
    pFootEndNoteImpl->sName.ToUpperAscii();

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

    pDoc->Insert( *pPam, aFtn );
    SwTxtFtn *pTxtFtn =
        (SwTxtFtn *)pPam->GetNode()->GetTxtNode()->GetTxtAttr(
                                pPam->GetPoint()->nContent.GetIndex() - 1,
                                RES_TXTATR_FTN );
    // In Kopf- und Fusszeilen duerfen keine Fussnoten eingefuegt werden.
    if( pTxtFtn )
    {
        pFootEndNoteImpl->aTxtFtns.Insert( pTxtFtn,
                                           pFootEndNoteImpl->aTxtFtns.Count() );

        pFootEndNoteImpl->aNames.Insert( new String(pFootEndNoteImpl->sName),
                                         pFootEndNoteImpl->aNames.Count() );
    }
    pFootEndNoteImpl->sName = aEmptyStr;
    pFootEndNoteImpl->sContent = aEmptyStr;
    pFootEndNoteImpl->bFixed = FALSE;
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

SwNodeIndex *SwHTMLParser::GetFootEndNoteSection( const String& rName )
{
    SwNodeIndex *pSttNdIdx = 0;

    if( pFootEndNoteImpl )
    {
        String aName( rName );
        // TODO: ToUpperAscii
        aName.ToUpperAscii();

        USHORT nCount = pFootEndNoteImpl->aNames.Count();
        for( USHORT i=0; i<nCount; i++ )
        {
            if( *pFootEndNoteImpl->aNames[i] == aName )
            {
                pSttNdIdx = pFootEndNoteImpl->aTxtFtns[i]->GetStartNode();
                pFootEndNoteImpl->aNames.DeleteAndDestroy( i, 1 );
                pFootEndNoteImpl->aTxtFtns.Remove( i, 1 );
                if( !pFootEndNoteImpl->aNames.Count() )
                {
                    delete pFootEndNoteImpl;
                    pFootEndNoteImpl = 0;
                }

                break;
            }
        }
    }

    return pSttNdIdx;
}

Writer& OutHTML_SwFmtFtn( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    SwFmtFtn& rFmtFtn = (SwFmtFtn&)rHt;
    SwTxtFtn *pTxtFtn = rFmtFtn.GetTxtFtn();
    if( !pTxtFtn )
        return rWrt;

    String sFtnName, sClass;
    USHORT nPos;
    if( rFmtFtn.IsEndNote() )
    {
        nPos = rHTMLWrt.pFootEndNotes ? rHTMLWrt.pFootEndNotes->Count() : 0;
        ASSERT( nPos == rHTMLWrt.nFootNote + rHTMLWrt.nEndNote,
                "OutHTML_SwFmtFtn: Position falsch" );
        sClass.AssignAscii( sHTML_sdendnote_anc );
        sFtnName.AssignAscii( sHTML_sdendnote );
        sFtnName += String::CreateFromInt32( (sal_Int32)(++rHTMLWrt.nEndNote) );
    }
    else
    {
        nPos = rHTMLWrt.nFootNote;
        sClass.AssignAscii( sHTML_sdfootnote_anc );
        sFtnName.AssignAscii( sHTML_sdfootnote);
        sFtnName += String::CreateFromInt32( (sal_Int32)(++rHTMLWrt.nFootNote));
    }

    if( !rHTMLWrt.pFootEndNotes )
        rHTMLWrt.pFootEndNotes = new SwHTMLTxtFtns;
    rHTMLWrt.pFootEndNotes->Insert( pTxtFtn, nPos );

    ByteString sOut( '<' );
    (((sOut += sHTML_anchor) += ' ') += sHTML_O_class) += "=\"";
    rWrt.Strm() << sOut.GetBuffer();
    HTMLOutFuncs::Out_String( rWrt.Strm(), sClass, rHTMLWrt.eDestEnc );
    ((sOut += "\" ") += sHTML_O_name) += "=\"";
    rWrt.Strm() << sOut.GetBuffer();
    HTMLOutFuncs::Out_String( rWrt.Strm(), sFtnName, rHTMLWrt.eDestEnc );
    (((sOut += sHTML_FTN_anchor) += "\" ") += sHTML_O_href) += "=\"#";
    rWrt.Strm() << sOut.GetBuffer();
    HTMLOutFuncs::Out_String( rWrt.Strm(), sFtnName, rHTMLWrt.eDestEnc );
    (sOut += sHTML_FTN_symbol)+= '\"';
    if( rFmtFtn.GetNumStr().Len() )
        (sOut += ' ') += sHTML_O_sdfixed;
    sOut += '>';
    rWrt.Strm() << sOut.GetBuffer();
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_superscript, TRUE );

    HTMLOutFuncs::Out_String( rWrt.Strm(), rFmtFtn.GetViewNumStr(*rWrt.pDoc),
                                 rHTMLWrt.eDestEnc );
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_superscript, FALSE );
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_anchor, FALSE );

    return rWrt;
}

void SwHTMLWriter::OutFootEndNotes()
{
    ASSERT( pFootEndNotes,
            "SwHTMLWriter::OutFootEndNotes(): unnoetiger Aufruf" );
    if( !pFootEndNotes )
        return;

#ifndef PRODUCT
    USHORT nFtn = nFootNote, nEn = nEndNote;
#endif
    nFootNote = 0, nEndNote = 0;

    for( USHORT i=0; i<pFootEndNotes->Count(); i++ )
    {
        SwTxtFtn *pTxtFtn = (*pFootEndNotes)[i];
        pFmtFtn = &pTxtFtn->GetFtn();

        String sFtnName, sClass;
        if( pFmtFtn->IsEndNote() )
        {
            sClass.AssignAscii( sHTML_sdendnote );
            sFtnName.AssignAscii( sHTML_sdendnote );
            sFtnName.Append( String::CreateFromInt32((sal_Int32)(++nEndNote)) );
        }
        else
        {
            sClass.AssignAscii( sHTML_sdfootnote );
            sFtnName.AssignAscii( sHTML_sdfootnote );
            sFtnName.Append( String::CreateFromInt32((sal_Int32)(++nFootNote)));
        }

        if( bLFPossible )
            OutNewLine();
        ByteString sOut( '<' );
        (((sOut += sHTML_division) += ' ') += sHTML_O_id) += "=\"";
        Strm() << sOut.GetBuffer();
        HTMLOutFuncs::Out_String( Strm(), sFtnName, eDestEnc );
        Strm() << "\">";

        bLFPossible = TRUE;
        IncIndentLevel();   // Inhalt von <DIV> einruecken

        ASSERT( pTxtFtn, "SwHTMLWriter::OutFootEndNotes: SwTxtFtn fehlt" );
        SwNodeIndex *pSttNdIdx = pTxtFtn->GetStartNode();
        ASSERT( pSttNdIdx,
                "SwHTMLWriter::OutFootEndNotes: StartNode-Index fehlt" );
        if( pSttNdIdx )
        {
            HTMLSaveData aSaveData( *this, pSttNdIdx->GetIndex()+1,
                pSttNdIdx->GetNode().EndOfSectionIndex(), GetFlyFrmFmt(),
                FALSE );
            Out_SwDoc( pCurPam );
        }

        DecIndentLevel();   // Inhalt von <DIV> einruecken
        if( bLFPossible )
            OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_division, FALSE );
        bLFPossible = TRUE;

        ASSERT( !pFmtFtn,
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

#ifndef PRODUCT
    ASSERT( nFtn == nFootNote,
            "SwHTMLWriter::OutFootEndNotes: Anzahl Fussnoten stimmt nicht" );
    ASSERT( nEn == nEndNote,
            "SwHTMLWriter::OutFootEndNotes: Anzahl Endnoten stimmt nicht" );
#endif

    delete pFootEndNotes;
    pFootEndNotes = 0;
    nFootNote = nEndNote = 0;
}

xub_StrLen SwHTMLWriter::GetFootEndNoteSymLen( const SwFmtFtn& rFmtFtn )
{
    xub_StrLen nLen = rFmtFtn.GetViewNumStr( *pDoc ).Len();

    if( rFmtFtn.GetNumStr().Len() == 0 )
    {
        const SwEndNoteInfo * pInfo =
            rFmtFtn.IsEndNote() ? &pDoc->GetEndNoteInfo() : &pDoc->GetFtnInfo();

        nLen += pInfo->GetPrefix().Len();
        nLen += pInfo->GetSuffix().Len();
    }

    return nLen;
}

void SwHTMLWriter::OutFootEndNoteSym( const SwFmtFtn& rFmtFtn )
{
    const SwEndNoteInfo *pInfo;

    String sFtnName, sClass, sPrefix, sSuffix;
    if( rFmtFtn.IsEndNote() )
    {
        sClass.AssignAscii( sHTML_sdendnote_sym );
        sFtnName.AssignAscii( sHTML_sdendnote );
        sFtnName.Append( String::CreateFromInt32((sal_Int32)nEndNote) );
        pInfo = &pDoc->GetEndNoteInfo();
    }
    else
    {
        sClass.AssignAscii( sHTML_sdfootnote_sym );
        sFtnName.AssignAscii( sHTML_sdfootnote );
        sFtnName.Append( String::CreateFromInt32((sal_Int32)nFootNote));
        pInfo = &pDoc->GetFtnInfo();
    }

    ByteString sOut( '<' );
    (((sOut += sHTML_anchor) +=  ' ') += sHTML_O_class) += "=\"";
    Strm() << sOut.GetBuffer();
    HTMLOutFuncs::Out_String( Strm(), sClass, eDestEnc );
    ((sOut += "\" ") += sHTML_O_name) += "=\"";
    Strm() << sOut.GetBuffer();
    HTMLOutFuncs::Out_String( Strm(), sFtnName, eDestEnc );
    (((sOut += sHTML_FTN_symbol) +="\" ") += sHTML_O_href) += "=\"#";
    Strm() << sOut.GetBuffer();
    HTMLOutFuncs::Out_String( Strm(), sFtnName, eDestEnc );
    (sOut += sHTML_FTN_anchor) += "\">";
    Strm() << sOut.GetBuffer();

    BOOL bAuto = rFmtFtn.GetNumStr().Len() == 0;
    String sNum;
    if( bAuto )
        sNum += pInfo->GetPrefix();
    sNum += rFmtFtn.GetViewNumStr(*pDoc);
    if( bAuto )
        sNum += pInfo->GetSuffix();
    HTMLOutFuncs::Out_String( Strm(), sNum, eDestEnc );
    HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_anchor, FALSE );
}

USHORT lcl_html_fillEndNoteInfo( const SwEndNoteInfo& rInfo,
                                 String *pParts,
                                 BOOL bEndNote  )
{
    USHORT nParts = 0;
    sal_Int16 eFmt = rInfo.aFmt.GetNumberingType();
    if( (bEndNote ? SVX_NUM_ROMAN_LOWER : SVX_NUM_ARABIC) != eFmt )
    {
        const sal_Char *pStr = SwHTMLWriter::GetNumFormat( eFmt );
        if( pStr )
        {
            pParts[0] = String::CreateFromAscii( pStr );
            nParts = 1;
        }
    }
    if( rInfo.nFtnOffset > 0 )
    {
        pParts[1] = String::CreateFromInt32( (sal_Int32)rInfo.nFtnOffset );
        nParts = 2;
    }
    if( rInfo.GetPrefix().Len() > 0 )
    {
        pParts[2] = rInfo.GetPrefix();
        nParts = 3;
    }
    if( rInfo.GetSuffix().Len() > 0 )
    {
        pParts[3] = rInfo.GetSuffix();
        nParts = 4;
    }

    return nParts;
}

void lcl_html_outFootEndNoteInfo( Writer& rWrt, String *pParts,
                                  USHORT nParts, const sal_Char *pName )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    String aContent;
    for( USHORT i=0; i<nParts; i++ )
    {
        xub_StrLen nPos = 0;
        String aTmp( pParts[i] );
        String aRep( String::CreateFromAscii("\\\\") );
        while( STRING_NOTFOUND != (nPos = aTmp.SearchAndReplaceAscii( "\\",
                                                     aRep, nPos ) ) )
            nPos += 2;
        nPos = 0;
        aRep.AssignAscii( "\\;" );
        while( STRING_NOTFOUND != (nPos = aTmp.SearchAndReplaceAscii( ";",
                                                     aRep, nPos ) ) )
            nPos += 2;
        if( i > 0 )
            aContent += ';';
        aContent += aTmp;
    }

    rHTMLWrt.OutNewLine();
    ByteString sOut( '<' );
    (((((((sOut += sHTML_meta) +=  ' ')
        += sHTML_O_name) += "=\"") += pName) += "\" ")
        += sHTML_O_content) += "=\"";
    rWrt.Strm() << sOut.GetBuffer();
    HTMLOutFuncs::Out_String( rWrt.Strm(), aContent, rHTMLWrt.eDestEnc );
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
        String aParts[8];
        USHORT nParts = lcl_html_fillEndNoteInfo( rInfo, aParts, FALSE );
        if( rInfo.eNum != FTNNUM_DOC )
        {
            aParts[4] = rInfo.eNum == FTNNUM_CHAPTER ? 'C' : 'P';
            nParts = 5;
        }
        if( rInfo.ePos != FTNPOS_PAGE)
        {
            aParts[5] = 'C';
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
                                         sHTML_META_sdfootnote );
    }

    {
        const SwEndNoteInfo& rInfo = pDoc->GetEndNoteInfo();
        String aParts[4];
        USHORT nParts = lcl_html_fillEndNoteInfo( rInfo, aParts, TRUE );
        if( nParts > 0 )
            lcl_html_outFootEndNoteInfo( *this, aParts, nParts,
                                         sHTML_META_sdendnote );
    }
}

