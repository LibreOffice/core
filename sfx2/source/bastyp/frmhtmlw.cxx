/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: frmhtmlw.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:54:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _INETDEF_HXX
#include <svtools/inetdef.hxx>
#endif

//!(dv) #include <chaos2/cntapi.hxx>
#ifndef GCC
#pragma hdrstop
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#include <unotools/configmgr.hxx>
#include "svtools/urihelper.hxx"

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

using namespace com::sun::star;

extern sal_Char const sHTML_META_classification[];

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

void SfxFrameHTMLWriter::Out_DocInfo( SvStream& rStrm, const String& rBaseURL,
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
                sContent += String(
                    URIHelper::simpleNormalizedMakeRelative(
                        rBaseURL, rReloadURL));
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
    SvStream& rOut, const String& rBaseURL, const uno::Reference < beans::XPropertySet >& xSet,
    rtl_TextEncoding eDestEnc, String *pNonConvertableChars )
{
    try
    {
        ByteString sOut;
        ::rtl::OUString aStr;
        uno::Any aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameURL") );
        if ( (aAny >>= aStr) && aStr.getLength() )
        {
            String aURL = INetURLObject( aStr ).GetMainURL( INetURLObject::DECODE_TO_IURI );
            if( aURL.Len() )
            {
                aURL = URIHelper::simpleNormalizedMakeRelative(
                    rBaseURL, aURL );
                ((sOut += ' ') += sHTML_O_src) += "=\"";
                rOut << sOut.GetBuffer();
                HTMLOutFuncs::Out_String( rOut, aURL, eDestEnc, pNonConvertableChars );
                sOut = '\"';
            }
        }

        aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameName") );
        if ( (aAny >>= aStr) && aStr.getLength() )
        {
            ((sOut += ' ') += sHTML_O_name) += "=\"";
            rOut << sOut.GetBuffer();
            HTMLOutFuncs::Out_String( rOut, aStr, eDestEnc, pNonConvertableChars );
            sOut = '\"';
        }

        sal_Int32 nVal = SIZE_NOT_SET;
        aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameMarginWidth") );
        if ( (aAny >>= nVal) && nVal != SIZE_NOT_SET )
            (((sOut += ' ') += sHTML_O_marginwidth) += '=') += ByteString::CreateFromInt32( nVal );
        aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameMarginHeight") );
        if ( (aAny >>= nVal) && nVal != SIZE_NOT_SET )
            (((sOut += ' ') += sHTML_O_marginheight) += '=') += ByteString::CreateFromInt32( nVal );

        sal_Bool bVal = sal_True;
        aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameIsAutoScroll") );
        if ( (aAny >>= bVal) && !bVal )
        {
            aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameIsScrollingMode") );
            if ( aAny >>= bVal )
            {
                const sal_Char *pStr = bVal ? sHTML_SC_yes : sHTML_SC_no;
                (((sOut += ' ') += sHTML_O_scrolling) += '=') += pStr;
            }
        }

        // frame border (MS+Netscape-Erweiterung)
        aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameIsAutoBorder") );
        if ( (aAny >>= bVal) && !bVal )
        {
            aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameIsBorder") );
            if ( aAny >>= bVal )
            {
                const char* pStr = bVal ? sHTML_SC_yes : sHTML_SC_no;
                (((sOut += ' ') += sHTML_O_frameborder) += '=') += pStr;
            }
        }

        // TODO/LATER: currently not supported attributes
        // resize
        //if( !pFrame->IsResizable() )
        //    (sOut += ' ') += sHTML_O_noresize;
        //
        //if ( pFrame->GetWallpaper() )
        //{
        //    ((sOut += ' ') += sHTML_O_bordercolor) += '=';
        //    rOut << sOut.GetBuffer();
        //    HTMLOutFuncs::Out_Color( rOut, pFrame->GetWallpaper()->GetColor(), eDestEnc );
        //}
        //else
            rOut << sOut.GetBuffer();
    }
    catch ( uno::Exception& )
    {
    }
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


