/*************************************************************************
 *
 *  $RCSfile: frmhtml.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: as $ $Date: 2000-11-08 14:25:44 $
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

#include <tools/list.hxx>
#include <tools/table.hxx>

#if SUPD<613//MUSTINI
    #ifndef _SFXINIMGR_HXX //autogen
    #include <svtools/iniman.hxx>
    #endif
#endif
#ifndef _HTMLTOKN_H
#include <svtools/htmltokn.h>
#endif
#pragma hdrstop

#include "docinf.hxx"

#define _SVSTDARR_USHORTS
#define _SVSTDARR_ULONGS
#include <svtools/svstdarr.hxx>

#include "sfx.hrc"

#include "app.hxx"
#include "frmhtml.hxx"
#include "fsetobsh.hxx"
#include "docfile.hxx"
#include "viewfrm.hxx"
#include "evntconf.hxx"
#include "request.hxx"
#include "fcontnr.hxx"
#include "sfxtypes.hxx"
#if SUPD<613//MUSTINI
#include "inimgr.hxx"
#endif
#define SFX_HTMLFRMSIZE_REL 0x0001
#define SFX_HTMLFRMSIZE_PERCENT 0x0002

static sal_Char __READONLY_DATA sHTML_SC_yes[] =    "YES";
static sal_Char __READONLY_DATA sHTML_SC_no[] =     "NO";
static sal_Char __READONLY_DATA sHTML_SC_auto[] =   "AUTO";

#define HTML_O_READONLY "READONLY"
#define HTML_O_EDIT     "EDIT"

static HTMLOptionEnum __READONLY_DATA aScollingTable[] =
{
    { sHTML_SC_yes,     ScrollingYes    },
    { sHTML_SC_no,      ScrollingNo     },
    { sHTML_SC_auto,    ScrollingAuto   },
    { 0,                0               }
};

SV_IMPL_PTRARR( _SfxFrameHTMLContexts, _SfxFrameHTMLContext* )

long SfxParserDelete_Impl( void* pObj, void* pArg )
{
    ((SfxFrameHTMLParser*)pArg )->ReleaseRef();
    delete (AsynchronLink*)pObj;
    return 0;
}

/*  */

_SfxFrameHTMLContext::_SfxFrameHTMLContext( const SfxFrameHTMLParser *pParser ) :
    nFrame( pParser->nFrame ),
    pFrameSet( pParser->pFrameSet )
{}

_SfxFrameHTMLContext::~_SfxFrameHTMLContext()
{}

/*  */

void SfxFrameHTMLParser::SaveContext()
{
    _SfxFrameHTMLContext *pContext = new _SfxFrameHTMLContext( this );
    aContextStack.C40_INSERT( _SfxFrameHTMLContext, pContext,
                              aContextStack.Count() );
}

void SfxFrameHTMLParser::InitContext( SfxFrameSetDescriptor *pFSet )
{
    nFrame = 0;
    pFrameSet = pFSet;
}

void SfxFrameHTMLParser::RestoreContext()
{
    DBG_ASSERT( aContextStack.Count(), "keine Kontexte mehr vorhanden!" )
    if( !aContextStack.Count() )
        return;

    _SfxFrameHTMLContext *pContext =
        aContextStack[ aContextStack.Count()-1 ];
    aContextStack.Remove( aContextStack.Count()-1, 1 );

    nFrame = pContext->nFrame;
    pFrameSet = pContext->pFrameSet;

    delete pContext;
}

/*  */

SfxFrameDescriptor *SfxFrameHTMLParser::GetCurrentFrame() const
{
    if( !pFrameSet || nFrame >= pFrameSet->GetFrameCount() )
        return 0;
    return pFrameSet->GetFrame( nFrame );
}

void SfxFrameHTMLParser::IncFramePos()
{
    if( !pFrameSet || nFrame >= pFrameSet->GetFrameCount() )
        return;
    nFrame++;
}

/*  */

void SfxFrameHTMLParser::ParseSizeSpec( const String& rSpec,
                                        SvULongs& rSizes,
                                        SvUShorts& rAttrs ) const
{
    if( !rSpec.Len() )
        return;

    xub_StrLen nPos = 0, nEnd = 0;
    while( STRING_NOTFOUND != nEnd )
    {
        sal_Unicode c;
        while ( ( c = rSpec.GetChar(nPos) ) == ' ' || c == '\t' ||
            c == '\n' || c == '\r' )
        {
            nPos++;
        }

        nEnd = rSpec.Search( ',', nPos );

        String aTmp( rSpec.Copy( nPos, STRING_NOTFOUND==nEnd ? STRING_LEN
                                                             : nEnd-nPos ) );
        sal_Int32 nSz = aTmp.ToInt32();
        if ( nSz == 0L && aTmp.Search('0') == STRING_NOTFOUND )
            nSz = 1L;

        rSizes.Insert( nSz, rSizes.Count() );
        USHORT nAttrs = 0;
        if( STRING_NOTFOUND != aTmp.Search( '*' ) )
            nAttrs |= SFX_HTMLFRMSIZE_REL;
        else if( STRING_NOTFOUND != aTmp.Search( '%' ) )
            nAttrs |= SFX_HTMLFRMSIZE_PERCENT;

        rAttrs.Insert( nAttrs, rAttrs.Count() );

        nPos = nEnd;
        nPos++;
    }
}

void SfxFrameHTMLParser::Continue( int nToken )
{
    String sTmpBaseURL( INetURLObject::GetBaseURL() );
    INetURLObject::SetBaseURL( aBaseURL );

    HTMLParser::Continue( nToken );

    INetURLObject::SetBaseURL( sTmpBaseURL );
}

/*  */

void SfxFrameHTMLParser::NextToken( int nToken )
{
    if( bInNoFrames )
    {
        switch ( nToken )
        {
            case HTML_META:
                return;
                break;
            case HTML_NOFRAMES_OFF:
                bInNoFrames = FALSE;
                break;
            default:
                break;
        }
    }

    switch( nToken )
    {
    case HTML_BODY_ON:
        // Without JavaScript, the cann be ignored.
        break;

    case HTML_META:
        if( pDoc )
        {
            ParseMetaOptions( GetDocInfo(), pDoc->GetHeaderAttributes() );
        }
        break;

    case HTML_TITLE_ON:
        InsertTitle();
        break;

    case HTML_BASE:
        {
            const HTMLOptions *pOptions = GetOptions();
            USHORT nArrLen = pOptions->Count();

            for( USHORT i = pOptions->Count(); i; )
            {
                const HTMLOption *pOption = (*pOptions)[ --i ];
                switch( pOption->GetToken() )
                {
                case HTML_O_HREF:
                    aBaseURL = pOption->GetString();
//                  INetURLObject::SetBaseURL( aBaseURL );
                    break;
                }
            }
        }
        break;

    case HTML_FRAMESET_ON:
        if( pDocFrameSet )
            NewFrameSet();
        else
        {
            // es ist ein Frames Dokument
            bIsFrameDoc = TRUE;
            eState = SVPAR_ACCEPTED;
        }
        break;
    case HTML_FRAMESET_OFF:
        if( pDocFrameSet )
            EndFrameSet();
        break;

    case HTML_FRAME_ON:
        if( pDocFrameSet )
            InsertFrame();
        break;

    case HTML_NOFRAMES_ON:
        bInNoFrames = TRUE;
        break;

    case HTML_APPLET_ON:
    case HTML_IFRAME_ON:
    case HTML_LINEBREAK:
    case HTML_HORZRULE:
    case HTML_IMAGE:
    case HTML_EMBED:
    case HTML_INPUT:
    case HTML_SELECT_ON:
    case HTML_TEXTAREA_ON:
        if( !pDocFrameSet  )
        {
           // es ist kein Frames Dokument (natuerlich gibt es noch mehr Tags,
           // aber dies duerften die haeufigsten am Dok-Anfang sein
           bIsFrameDoc = FALSE;
           eState = SVPAR_ACCEPTED;
        }
        break;

    case HTML_TEXTTOKEN:
        if( !pDocFrameSet && aToken.Len() &&
            (aToken.Len() > 1 || ' ' != aToken.GetChar(0)) )
        {
            // es ist kein Frames Dokument
            bIsFrameDoc = FALSE;
            eState = SVPAR_ACCEPTED;
        }
        break;
    case HTML_SCRIPT_ON:
        NewScript();
        break;

    case HTML_SCRIPT_OFF:
        EndScript();
        break;

    case HTML_RAWDATA:
        if( IsReadScript() && !bIgnoreRawData )
        {
            if( aScriptSource.Len() )
                aScriptSource += '\n';
            aScriptSource += aToken;
        }
        break;

//  case HTML_UNKNOWNCONTROL_ON:
//      if( IsInHeader() && !aUnknownToken.Len() &&
//          sSaveToken.Len() && '!' != sSaveToken.GetChar(0) )
//          aUnknownToken = sSaveToken;
//      break;
    }
}

void SfxFrameHTMLParser::NewScript()
{
    BOOL bFinishDownload = FALSE;
    String aScriptURL;
    String aScriptType;
    String aLib, aModule;
    ParseScriptOptions( aScriptType, eScriptType, aScriptURL,
                        aLib, aModule );

    if( aScriptURL.Len() && HTML_SL_JAVASCRIPT == eScriptType )
    {
        // Den Inhalt des Script-Tags ignorieren
        bIgnoreRawData = TRUE;
    }
}

void SfxFrameHTMLParser::EndScript()
{
    bIgnoreRawData = FALSE;

    // MIB: 3/3/2000: Scripts a ignored currently?
    aScriptSource.Erase();
}

/*  */

void SfxFrameHTMLParser::InsertTitle()
{
    String aTitle;
    int nToken;
    while( HTML_TEXTTOKEN == (nToken = GetNextToken()) && IsParserWorking() )
    {
        aTitle += aToken;
    }

    if( SVPAR_PENDING == GetStatus() )
        return;

    // Wir sollten jetzt eigentlich auf einem </TITLE> stehen, das wir
    // aber ignorieren
    if( HTML_TITLE_OFF != nToken )
        SkipToken( -1 );

    SfxDocumentInfo *pDocInfo = GetDocInfo();
    if( aTitle.Len() && pDocInfo && !pDocInfo->GetTitle().Len() )
    {
        pDocInfo->SetTitle( aTitle );
    }
}

void SfxFrameHTMLParser::NewFrameSet()
{
    String aRowSpec, aColSpec;
    Wallpaper aWallpaper;
    BOOL bBorder = TRUE;
    BOOL bBorderSet = FALSE;
    BOOL bSpacingSet = FALSE;
    BOOL bBrushSet = FALSE;
    long nSpacing = 0L;

    // die Optionen holen
    ScriptType eScriptType = STARBASIC;
    USHORT nEvent;

    const HTMLOptions *pOptions = GetOptions();
    USHORT nArrLen = pOptions->Count();
    for ( USHORT i=0; i<nArrLen; i++ )
    {
        BOOL bSetEvent = FALSE;
        const HTMLOption *pOption = (*pOptions)[i];
        switch( pOption->GetToken() )
        {
        case HTML_O_BORDERCOLOR:
            {
                Color aColor;
                pOption->GetColor( aColor );
                aWallpaper.SetColor( aColor );
                bBrushSet = TRUE;
                break;
            }
        case HTML_O_ROWS:
            aRowSpec = pOption->GetString();
            break;
        case HTML_O_COLS:
            aColSpec = pOption->GetString();
            break;
        case HTML_O_FRAMEBORDER:
        {
            bBorderSet = TRUE;
            String aStr = pOption->GetString();
            bBorder = TRUE;
            if ( aStr.EqualsIgnoreCaseAscii("NO") ||
                 aStr.EqualsIgnoreCaseAscii("0") )
                bBorder = FALSE;
            break;
        }
        case HTML_O_FRAMESPACING:
        case HTML_O_BORDER:
            bSpacingSet = TRUE;
            nSpacing = pOption->GetNumber();
            break;
        case HTML_O_ONLOAD:
            eScriptType = JAVASCRIPT;
        case HTML_O_SDONLOAD:
            nEvent = SFX_EVENT_OPENDOC;
            bSetEvent = TRUE;
            break;

        case HTML_O_ONUNLOAD:
            eScriptType = JAVASCRIPT;
        case HTML_O_SDONUNLOAD:
            nEvent = SFX_EVENT_PREPARECLOSEDOC;
            bSetEvent = TRUE;
            break;

        case HTML_O_ONFOCUS:
            eScriptType = JAVASCRIPT;
        case HTML_O_SDONFOCUS:
            nEvent = SFX_EVENT_ACTIVATEDOC;
            bSetEvent = TRUE;
            break;

        case HTML_O_ONBLUR:
            eScriptType = JAVASCRIPT;
        case HTML_O_SDONBLUR:
            nEvent = SFX_EVENT_DEACTIVATEDOC;
            bSetEvent = TRUE;
            break;

        case HTML_O_ONERROR:
//              if( bAnyStarBasic )
//                  InsertBasicDocEvent( SFX_EVENT_ACTIVATEDOC,
//                                       pOption->GetString() );
            break;
        }

        if ( bSetEvent )
        {
            DBG_ASSERT( pDoc, "Macros im Parser, aber kein Dokument!" );
            String sEmpty;
            SFX_APP()->GetEventConfig()->ConfigureEvent( nEvent,
                SvxMacro( pOption->GetString(), sEmpty, eScriptType ),
                pDoc );
        }

    }

    if ( !aRowSpec.Len() && !aColSpec.Len() )
    {
        // Netscape l"a\st es zu, da\s keine Gr"o\senangabe erfolgt
        aRowSpec.AssignAscii( "100%" );
    }

    // die ROW/COL-Optionen parsen
    SvULongs aRowSizes;
    SvUShorts aRowAttrs;
    ParseSizeSpec( aRowSpec, aRowSizes, aRowAttrs );
    USHORT nRows = aRowSizes.Count();

    SvULongs aColSizes;
    SvUShorts aColAttrs;
    ParseSizeSpec( aColSpec, aColSizes, aColAttrs );
    USHORT nCols = aColSizes.Count();

    // einen FrameSet anlegen (wenn pFrameSet nicht gesetzt ist, ist dies
    // das erste <FRAMESET> und wir muessen den Doc-Frameset benutzen)
    SfxFrameDescriptor *pFrame = GetCurrentFrame();
    if ( pFrameSet )
    {
        while ( !pFrame && aContextStack.Count() )
        {
            EndFrameSet();
            pFrame = GetCurrentFrame();
        }

        if ( !pFrame )
            return;
    }

    SfxFrameSetDescriptor *pNewFrameSet =
        pFrameSet ? new SfxFrameSetDescriptor( pFrame )
                  : pDocFrameSet;

    // und mit Leben fuellen
    if( nRows && nCols )
    {
        SfxFrameSetDescriptor *pFirst = NULL;
        for( USHORT i=0; i<nRows; i++ )
        {
            pNewFrameSet->SetColSet( FALSE );
            SfxFrameDescriptor *pNewFrame =
                new SfxFrameDescriptor( pNewFrameSet );
            SfxFrameSetDescriptor *pFrameSet =
                                new SfxFrameSetDescriptor( pNewFrame );
            if ( !pFirst )
                pFirst = pFrameSet;
            pFrameSet->SetColSet( TRUE );

            if( (aRowAttrs[i] & SFX_HTMLFRMSIZE_REL) != 0 )
                pNewFrame->SetWidthRel( aRowSizes[i] );
            else if( (aRowAttrs[i] & SFX_HTMLFRMSIZE_PERCENT) != 0 )
                pNewFrame->SetWidthPercent( aRowSizes[i] );
            else
                pNewFrame->SetWidthAbs( aRowSizes[i] );

            for( USHORT j=0; j<nCols; j++ )
            {
                SfxFrameDescriptor *pFrame =
                    new SfxFrameDescriptor( pFrameSet );
                if( (aColAttrs[j] & SFX_HTMLFRMSIZE_REL) != 0 )
                    pFrame->SetWidthRel( aColSizes[j] );
                else if( (aColAttrs[j] & SFX_HTMLFRMSIZE_PERCENT) != 0 )
                    pFrame->SetWidthPercent( aColSizes[j] );
                else
                    pFrame->SetWidthAbs( aColSizes[j] );
            }
        }

        // den aktuellen FrameSet-Kontext retten und einen neuen einstellen
        if( pNewFrameSet != pDocFrameSet )
            SaveContext();
        InitContext( pNewFrameSet );
        SaveContext();
        InitContext( pFirst );
    }
    else if( nRows || nCols )
    {
        USHORT nCount = nRows ? nRows : nCols;
        pNewFrameSet->SetColSet( nCols != 0 );

        for( USHORT i=0; i<nCount; i++ )
        {
            SfxFrameDescriptor *pFrame =
                new SfxFrameDescriptor( pNewFrameSet );
            USHORT nAttrs = (nRows ? aRowAttrs[i] : aColAttrs[i] );
            ULONG nSize = (nRows ? aRowSizes[i] : aColSizes[i] );
            if( (nAttrs & SFX_HTMLFRMSIZE_REL) != 0 )
                pFrame->SetWidthRel( nSize );
            else if( (nAttrs & SFX_HTMLFRMSIZE_PERCENT) != 0 )
                pFrame->SetWidthPercent( nSize );
            else
                pFrame->SetWidthAbs( nSize );
        }

        // den aktuellen FrameSet-Kontext retten und einen neuen einstellen
        if( pNewFrameSet != pDocFrameSet )
            SaveContext();
        InitContext( pNewFrameSet );
    }

    if ( bBorderSet )
        pNewFrameSet->SetFrameBorder( bBorder );
    if ( bSpacingSet )
        pNewFrameSet->SetFrameSpacing( nSpacing );
    if ( bBrushSet )
        pNewFrameSet->SetWallpaper( aWallpaper );
}

void SfxFrameHTMLParser::EndFrameSet()
{
    if( aContextStack.Count() )
    {
        // den vorherigen FrameSet-Kontext wieder herstellen
        RestoreContext();

        // und zum naechsten Frame gehen
        IncFramePos();
        SfxFrameDescriptor *pFrame = GetCurrentFrame();
        if ( pFrame && pFrame->GetFrameSet() )
        {
            SaveContext();
            InitContext( pFrame->GetFrameSet() );
        }
    }
    else
    {
        // der Root-Frameset wird beendet
        if( pFrameSet )
        {
            // "Cursor" aus dem Frameset herausbewegen, damit nichsts
            // mehr eingefuegt wird.
            nFrame = pFrameSet->GetFrameCount();
        }
    }
}

void SfxFrameHTMLParser::InsertFrame()
{
    // den aktuellen Frame holen (ist nicht da, falls es noch kein
    // <FRAMESET> gab oder das aktuelle FrameSet schon voll ist
    SfxFrameDescriptor *pFrame = GetCurrentFrame();
    while ( !pFrame && aContextStack.Count() )
    {
        EndFrameSet();
        pFrame = GetCurrentFrame();
    }

    if ( !pFrame )
        return;

    const HTMLOptions *pOptions = GetOptions();
    ParseFrameOptions( pFrame, pOptions );

    // und zum naechsten Frame gehen
    IncFramePos();
}

void SfxFrameHTMLParser::ParseFrameOptions( SfxFrameDescriptor *pFrame,
        const HTMLOptions *pOptions )
{
    // die Optionen holen und setzen
    Size aMargin( pFrame->GetMargin() );

    // MIB 15.7.97: Netscape scheint marginwidth auf 0 zu setzen, sobald
    // marginheight gesetzt wird und umgekehrt. Machen wir jetzt wegen
    // bug #41665# auch so.
    // Netscape l"a\st aber ein direktes Setzen auf 0 nicht zu, IE4.0 schon.
    // Den Bug machen wir nicht mit!
    BOOL bMarginWidth = FALSE, bMarginHeight = FALSE;

    USHORT nArrLen = pOptions->Count();
    for ( USHORT i=0; i<nArrLen; i++ )
    {
        const HTMLOption *pOption = (*pOptions)[i];
        switch( pOption->GetToken() )
        {
        case HTML_O_BORDERCOLOR:
            {
                Color aColor;
                pOption->GetColor( aColor );
                pFrame->SetWallpaper( Wallpaper( aColor ) );
                break;
            }
        case HTML_O_SRC:
            pFrame->SetURL( INetURLObject::RelToAbs(pOption->GetString()) );
            break;
        case HTML_O_NAME:
            pFrame->SetName( pOption->GetString() );
            break;
        case HTML_O_MARGINWIDTH:
            aMargin.Width() = pOption->GetNumber();

//          if( aMargin.Width() < 1 )
//              aMargin.Width() = 1;
            if( !bMarginHeight )
                aMargin.Height() = 0;
            bMarginWidth = TRUE;
            break;
        case HTML_O_MARGINHEIGHT:
            aMargin.Height() = pOption->GetNumber();

//          if( aMargin.Height() < 1 )
//              aMargin.Height() = 1;
            if( !bMarginWidth )
                aMargin.Width() = 0;
            bMarginHeight = TRUE;
            break;
        case HTML_O_SCROLLING:
            pFrame->SetScrollingMode(
                (ScrollingMode)pOption->GetEnum( aScollingTable,
                                                 ScrollingAuto ) );
            break;
        case HTML_O_FRAMEBORDER:
        {
            String aStr = pOption->GetString();
            BOOL bBorder = TRUE;
            if ( aStr.EqualsIgnoreCaseAscii("NO") ||
                 aStr.EqualsIgnoreCaseAscii("0") )
                bBorder = FALSE;
            pFrame->SetFrameBorder( bBorder );
            break;
        }
        case HTML_O_NORESIZE:
            pFrame->SetResizable( FALSE );
            break;
        default:
            if ( pOption->GetTokenString().EqualsIgnoreCaseAscii(
                                                        HTML_O_READONLY ) )
            {
                String aStr = pOption->GetString();
                BOOL bReadonly = TRUE;
                if ( aStr.EqualsIgnoreCaseAscii("FALSE") )
                    bReadonly = FALSE;
                pFrame->SetReadOnly( bReadonly );
            }
            else if ( pOption->GetTokenString().EqualsIgnoreCaseAscii(
                                                        HTML_O_EDIT ) )
            {
                String aStr = pOption->GetString();
                BOOL bEdit = TRUE;
                if ( aStr.EqualsIgnoreCaseAscii("FALSE") )
                    bEdit = FALSE;
                pFrame->SetEditable( bEdit );
            }

            break;
        }
    }

    pFrame->SetMargin( aMargin );
}


SfxFrameHTMLParser::SfxFrameHTMLParser( SfxMedium& rMedium,
            SfxFrameSetObjectShell* _pDoc )
    : SfxHTMLParser( *rMedium.GetInStream(), TRUE, &rMedium ),
    nLine( 0 ),
    nFrame( 0 ),
    pFrameSet( 0 ),
    pDoc( _pDoc ),
    pDocFrameSet( 0 ),
    bIsFrameDoc( FALSE ),
    bInNoFrames( FALSE ),
    bIgnoreRawData( FALSE ),
    eScriptType ( HTML_SL_JAVASCRIPT ),
    pDeleteLink(0),
    aBaseURL( pDoc ? pDoc->GetBaseURL() : INetURLObject::GetBaseURL() )
{
    SvKeyValueIterator *pHeaderAttrs = pDoc->GetHeaderAttributes();
    if( pHeaderAttrs )
        SetEncodingByHTTPHeader( pHeaderAttrs );

    if ( pDoc )
    {
        SfxItemSet* pSet = rMedium.GetItemSet();
        SFX_ITEMSET_ARG(
            pSet, pLoadRef, SfxRefItem, SID_LOADENVIRONMENT, FALSE);
        if( pLoadRef )
        {
            ((SfxLoadEnvironment*)&pLoadRef->GetValue())->
                DocumentDetected( _pDoc, ERRCODE_NONE );
        }
        pDocFrameSet = pDoc->pDescriptor;
    }
}

SfxFrameHTMLParser::SfxFrameHTMLParser( SvStream& rStream,
            SfxFrameSetObjectShell* _pDoc )
    : SfxHTMLParser( rStream, TRUE ),
    nLine( 0 ),
    nFrame( 0 ),
    pFrameSet( 0 ),
    pDoc( _pDoc ),
    pDocFrameSet( 0 ),
    bIsFrameDoc( FALSE ),
    bInNoFrames( FALSE ),
    eScriptType ( HTML_SL_JAVASCRIPT ),
    pDeleteLink(0),
    aBaseURL( pDoc ? pDoc->GetBaseURL() : INetURLObject::GetBaseURL() )
{
    // Ohne Medium kann es auch keinen HTTP-Header und damit auch
    // keine Zeichsatz-Informationen geben!
    if ( pDoc )
    {
        pDocFrameSet = pDoc->pDescriptor;
    }
}

SfxFrameHTMLParser::~SfxFrameHTMLParser()
{
    if ( pDoc && GetMedium() )
    {
        SfxItemSet* pSet = GetMedium()->GetItemSet();
        const SfxPoolItem *pItem;
        SfxItemState eItemState = pSet->GetItemState( SID_LOADENVIRONMENT, FALSE, &pItem );
        SfxLoadEnvironment* pEnv = (SfxLoadEnvironment*)
            &((const SfxRefItem*)pItem)->GetValue();
        pEnv->SetDataAvailableLink( Link() );
        pEnv->DocumentDetected( pDoc, 0 );
        pSet->ClearItem( SID_LOADENVIRONMENT );
        pDoc->SetTitle( pDoc->GetDocInfo().GetTitle() );
        pDoc->ReleaseRef();
        // FinishedLoading ruft die ViewShell!
    }
}

SvParserState SfxFrameHTMLParser::CallParser()
{
    // die Base-URL kann durch <BASE> umgeschossen werden
//  String aBaseURL( INetURLObject::GetBaseURL() );
    if ( pDoc && GetMedium() )
    {
        pDoc->AddRef();
        SfxItemSet* pSet = GetMedium()->GetItemSet();
        const SfxPoolItem *pItem;
        SfxItemState eItemState = pSet->GetItemState( SID_LOADENVIRONMENT, FALSE, &pItem );
        SfxLoadEnvironment* pEnv = (SfxLoadEnvironment*)
            &((const SfxRefItem*)pItem)->GetValue();
        pEnv->SetDataAvailableLink( GetAsynchCallLink() );
    }

    SvParserState eState = HTMLParser::CallParser();

    if ( eState == SVPAR_ACCEPTED && pDoc && GetMedium() )
    {
        // DataAvailableLink zur"ucksetzen, damit den keiner mehr aufruft
        SfxItemSet* pSet = GetMedium()->GetItemSet();
        const SfxPoolItem *pItem;
        SfxItemState eItemState = pSet->GetItemState( SID_LOADENVIRONMENT, FALSE, &pItem );
        SfxLoadEnvironment* pEnv = (SfxLoadEnvironment*)
            &((const SfxRefItem*)pItem)->GetValue();
        pEnv->SetDataAvailableLink( Link() );
    }

//  INetURLObject::SetBaseURL( aBaseURL );

    return eState;
}

SfxDocumentInfo* SfxFrameHTMLParser::GetDocInfo()
{
    return pDoc ? &pDoc->GetDocInfo() : NULL;
}

void SfxFrameHTMLParser::EndParser()
{
    AddRef();
    AsynchronLink* pLink = new AsynchronLink;
    *pLink = Link( pLink, SfxParserDelete_Impl );
    pLink->Call( this );
}


