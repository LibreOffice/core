/*************************************************************************
 *
 *  $RCSfile: frmhtmlw.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 07:58:27 $
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

#include <so3/iface.hxx>
#ifndef _INETDEF_HXX
#include <svtools/inetdef.hxx>
#endif
//!(dv) #include <chaos2/cntapi.hxx>
#pragma hdrstop
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#include <unotools/configmgr.hxx>

#include "docinf.hxx"
#include "frmhtmlw.hxx"
#include "evntconf.hxx"
#include "frame.hxx"
#include "app.hxx"
#include "viewfrm.hxx"
#include "docfile.hxx"
#include "sfxresid.hxx"
#include "objsh.hxx"
#include "sfx.hrc"
#include "bastyp.hrc"

// -----------------------------------------------------------------------

extern sal_Char __FAR_DATA sHTML_META_classification[];

static sal_Char __READONLY_DATA sHTML_SC_yes[] =    "YES";
static sal_Char __READONLY_DATA sHTML_SC_no[] =     "NO";
static sal_Char __READONLY_DATA sHTML_SC_auto[] =   "AUTO";
static sal_Char __READONLY_DATA sHTML_MIME_text_html[] =    "text/html; charset=";

static HTMLOutEvent __FAR_DATA aFrameSetEventTable[] =
{
    { sHTML_O_SDonload,     sHTML_O_onload,     SFX_EVENT_OPENDOC   },
    { sHTML_O_SDonunload,   sHTML_O_onunload,   SFX_EVENT_PREPARECLOSEDOC   },
    { sHTML_O_SDonfocus,    sHTML_O_onfocus,    SFX_EVENT_ACTIVATEDOC   },
    { sHTML_O_SDonblur,     sHTML_O_onblur,     SFX_EVENT_DEACTIVATEDOC },
    { 0,                    0,                  0                   }
};

#if defined(MAC)
const sal_Char SfxFrameHTMLWriter::sNewLine[] = "\015";
#elif defined(UNX)
const sal_Char SfxFrameHTMLWriter::sNewLine[] = "\012";
#else
const sal_Char __FAR_DATA SfxFrameHTMLWriter::sNewLine[] = "\015\012";
#endif

void SfxFrameHTMLWriter::OutMeta( SvStream& rStrm,
                                  const sal_Char *pIndent,
                                  const String& rName,
                                  const String& rContent, BOOL bHTTPEquiv,
                                     rtl_TextEncoding eDestEnc,
                                  String *pNonConvertableChars  )
{
    rStrm << sNewLine;
    if( pIndent )
        rStrm << pIndent;

    ByteString sOut( '<' );
    (((sOut += sHTML_meta) += ' ')
        += (bHTTPEquiv ? sHTML_O_httpequiv : sHTML_O_name)) += "=\"";
    rStrm << sOut.GetBuffer();

    HTMLOutFuncs::Out_String( rStrm, rName, eDestEnc, pNonConvertableChars );

    ((sOut = "\" ") += sHTML_O_content) += "=\"";
    rStrm << sOut.GetBuffer();

    HTMLOutFuncs::Out_String( rStrm, rContent, eDestEnc, pNonConvertableChars ) << "\">";
}

void SfxFrameHTMLWriter::Out_DocInfo( SvStream& rStrm,
                                      const SfxDocumentInfo* pInfo,
                                      const sal_Char *pIndent,
                                         rtl_TextEncoding eDestEnc,
                                  String *pNonConvertableChars  )
{
    const sal_Char *pCharSet =
                rtl_getBestMimeCharsetFromTextEncoding( eDestEnc );

    if( pCharSet )
    {
        String aContentType = String::CreateFromAscii( sHTML_MIME_text_html );
        aContentType.AppendAscii( pCharSet );
        OutMeta( rStrm, pIndent, sHTML_META_content_type, aContentType, TRUE,
                  eDestEnc, pNonConvertableChars );
    }

    // Titel (auch wenn er leer ist)
    rStrm << sNewLine;
    if( pIndent )
        rStrm << pIndent;
    HTMLOutFuncs::Out_AsciiTag( rStrm, sHTML_title );
    if( pInfo  )
    {
        const String& rTitle = pInfo->GetTitle();
        if( rTitle.Len() )
            HTMLOutFuncs::Out_String( rStrm, rTitle, eDestEnc, pNonConvertableChars );
    }
    HTMLOutFuncs::Out_AsciiTag( rStrm, sHTML_title, FALSE );

    // Target-Frame
    if( pInfo )
    {
        const String& rTarget = pInfo->GetDefaultTarget();
        if( rTarget.Len() )
        {
            rStrm << sNewLine;
            if( pIndent )
                rStrm << pIndent;

            ByteString sOut( '<' );
            (((sOut += sHTML_base) += ' ') += sHTML_O_target) += "=\"";
            rStrm << sOut.GetBuffer();
            HTMLOutFuncs::Out_String( rStrm, rTarget, eDestEnc, pNonConvertableChars )
                << "\">";
        }
    }

    // Who we are
    String sGenerator( SfxResId( STR_HTML_GENERATOR ) );
    sGenerator.SearchAndReplaceAscii( "%1", String( DEFINE_CONST_UNICODE( TOOLS_INETDEF_OS ) ) );
    OutMeta( rStrm, pIndent, sHTML_META_generator, sGenerator, FALSE, eDestEnc, pNonConvertableChars );

    if( pInfo )
    {
        // Reload
        if( pInfo->IsReloadEnabled() )
        {
            String sContent = String::CreateFromInt32(
                                (sal_Int32)pInfo->GetReloadDelay() );

            const String &rReloadURL = pInfo->GetReloadURL();
            if( rReloadURL.Len() )
            {
                sContent.AppendAscii( ";URL=" );
                sContent += INetURLObject::AbsToRel(rReloadURL);
            }

            OutMeta( rStrm, pIndent, sHTML_META_refresh, sContent, TRUE,
                      eDestEnc, pNonConvertableChars );
        }

        // Author
        const SfxStamp& rCreated = pInfo->GetCreated();
        const String& rAuthor = rCreated.GetName();
        if( rAuthor.Len() )
            OutMeta( rStrm, pIndent, sHTML_META_author, rAuthor, FALSE,
                      eDestEnc, pNonConvertableChars );

        // created
        const DateTime& rCreatedDT = rCreated.GetTime();
        String sOut(
            String::CreateFromInt32( (sal_Int32)rCreatedDT.GetDate() ) );
        (sOut += ';') +=
            String::CreateFromInt32( (sal_Int32)rCreatedDT.GetTime() );
        OutMeta( rStrm, pIndent, sHTML_META_created, sOut, FALSE, eDestEnc, pNonConvertableChars );

        // changedby
        const SfxStamp& rChanged = pInfo->GetChanged();
        const String& rChangedBy = rChanged.GetName();
        if( rChangedBy.Len() )
            OutMeta( rStrm, pIndent, sHTML_META_changedby, rChangedBy, FALSE,
                      eDestEnc, pNonConvertableChars );

        // changed
        const DateTime& rChangedDT = rChanged.GetTime();
        sOut = String::CreateFromInt32( (sal_Int32)rChangedDT.GetDate() );
        (sOut += ';') +=
            String::CreateFromInt32( (sal_Int32)rChangedDT.GetTime() );
        OutMeta( rStrm, pIndent, sHTML_META_changed, sOut, FALSE, eDestEnc, pNonConvertableChars );

        // Thema
        const String& rTheme = pInfo->GetTheme();
        if( rTheme.Len() )
            OutMeta( rStrm, pIndent, sHTML_META_classification, rTheme, FALSE,
                      eDestEnc, pNonConvertableChars );

        // Beschreibung
        const String& rComment = pInfo->GetComment();
        if( rComment.Len() )
            OutMeta( rStrm, pIndent, sHTML_META_description, rComment, FALSE,
                      eDestEnc, pNonConvertableChars);

        // Keywords
        const String& rKeywords = pInfo->GetKeywords();
        if( rKeywords.Len() )
            OutMeta( rStrm, pIndent, sHTML_META_keywords, rKeywords, FALSE,
                      eDestEnc, pNonConvertableChars);

        // die Benutzer-Eintraege
        USHORT nKeys = pInfo->GetUserKeyCount();

        // Leere Eintraege am Ende werden nicht ausgegeben
        while( nKeys && !pInfo->GetUserKey(nKeys-1).GetWord().Len() )
            nKeys--;

        for( USHORT i=0; i< nKeys; i++ )
        {
            const SfxDocUserKey& rUserKey = pInfo->GetUserKey(i);
            String aWord( rUserKey.GetWord() );
            aWord.EraseTrailingChars();
            if( rUserKey.GetTitle().Len() )
                OutMeta( rStrm, pIndent, rUserKey.GetTitle(), aWord, FALSE,
                         eDestEnc, pNonConvertableChars );
        }
    }
}
/*
void SfxFrameHTMLWriter::OutHeader( rtl_TextEncoding eDestEnc )
{
    // <HTML>
    // <HEAD>
    // <TITLE>Titel</TITLE>
    // </HEAD>
    HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_html ) << sNewLine;
    HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_head );

    Out_DocInfo( Strm(), &pDoc->GetDocInfo(), "\t", eDestEnc );
    Strm() << sNewLine;
    HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_head, FALSE ) << sNewLine;

//! OutScript();            // Hier fehlen noch die Scripten im Header
}
*/

void SfxFrameHTMLWriter::Out_FrameDescriptor(
    SvStream& rOut, const SfxFrameDescriptor *pFrame, BOOL bFlatten,
    SfxFrame* pTopFrame, rtl_TextEncoding eDestEnc, String *pNonConvertableChars )
{
    SfxFrameSetDescriptor *pFSet = pFrame->GetFrameSet();

    ByteString sOut;

    String aURL = bFlatten ?
        pFrame->GetActualURL().GetMainURL( INetURLObject::DECODE_TO_IURI ) :
        pFrame->GetURL().GetMainURL( INetURLObject::DECODE_TO_IURI );
    if( aURL.Len() )
    {
        if( !bFlatten ) aURL = INetURLObject::AbsToRel( aURL );
        ((sOut += ' ') += sHTML_O_src) += "=\"";
        rOut << sOut.GetBuffer();
        HTMLOutFuncs::Out_String( rOut, aURL, eDestEnc, pNonConvertableChars );
        sOut = '\"';
    }

    // der Name (nur wenn der String nicht leer ist)
    const String& rName = pFrame->GetName();
    if( rName.Len() )
    {
        ((sOut += ' ') += sHTML_O_name) += "=\"";
        rOut << sOut.GetBuffer();
        HTMLOutFuncs::Out_String( rOut, rName, eDestEnc, pNonConvertableChars );
        sOut = '\"';
    }

    // margin width und height; -1 ist der Default-Margin
    const Size& rMargin = pFrame->GetMargin();
    if( rMargin.Width() >= 0 )
        (((sOut += ' ') += sHTML_O_marginwidth) += '=')
            += ByteString::CreateFromInt32( (sal_Int32)rMargin.Width() );
    if( rMargin.Height() >= 0 )
        (((sOut += ' ') += sHTML_O_marginheight) += '=')
            += ByteString::CreateFromInt32( (sal_Int32)rMargin.Height() );

    // scroll mode
    const sal_Char *pStr = 0;
    switch( pFrame->GetScrollingMode() )
    {
        case ScrollingYes:  pStr = sHTML_SC_yes;    break;
        case ScrollingNo:   pStr = sHTML_SC_no;     break;
//      case ScrollingAuto: pStr = sHTML_SC_auto;   break;  // Default !!
    }

    if( pStr )
        (((sOut += ' ') += sHTML_O_scrolling) += '=') += pStr;

    // resize
    if( !pFrame->IsResizable() )
        (sOut += ' ') += sHTML_O_noresize;

    // frame border (MS+Netscape-Erweiterung)
    if ( pFrame->IsFrameBorderSet() )
    {
        pStr = pFrame->IsFrameBorderOn() ? sHTML_SC_yes : sHTML_SC_no;
        if( pStr )
            (((sOut += ' ') += sHTML_O_frameborder) += '=') += pStr;
    }

    if ( pFrame->GetWallpaper() )
    {
        ((sOut += ' ') += sHTML_O_bordercolor) += '=';
        rOut << sOut.GetBuffer();
        HTMLOutFuncs::Out_Color( rOut, pFrame->GetWallpaper()->GetColor(), eDestEnc );
    }
    else
        rOut << sOut.GetBuffer();
}

String SfxFrameHTMLWriter::CreateURL( SfxFrame* pFrame )
{
    String aRet;
    SfxObjectShell* pShell = pFrame->GetCurrentDocument();
    if( !aRet.Len() && pShell )
    {
        aRet = pShell->GetMedium()->GetName();
//!(dv)     CntAnchor::ToPresentationURL( aRet );
    }

    return aRet;
}


