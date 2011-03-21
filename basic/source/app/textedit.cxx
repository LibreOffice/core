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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"
#include <tools/stream.hxx>
#include <svtools/texteng.hxx>
#include <svtools/textview.hxx>
#include <svtools/txtattr.hxx>
#include <basic/sbxmeth.hxx>
#include <basic/ttstrhlp.hxx>

#include "basic.hrc"
#include "textedit.hxx"
#include "appedit.hxx"
#include "brkpnts.hxx"
#include <basic/testtool.hxx>  // defines for syntax highlighting

TextEditImp::TextEditImp( AppEdit* pParent, const WinBits& aBits )
: Window( pParent, aBits )
, pAppEdit( pParent )
, bHighlightning( sal_False )
, bDoSyntaxHighlight( sal_False )
, bDelayHighlight( sal_True )
, nTipId( 0 )
, bViewMoved( sal_False )
{
    pTextEngine = new TextEngine();
    pTextEngine->SetMaxTextLen( STRING_MAXLEN );
    pTextEngine->EnableUndo( sal_True );

    pTextView = new TextView( pTextEngine, this );
    pTextEngine->InsertView( pTextView );
    pTextEngine->SetModified( sal_False );

    aSyntaxIdleTimer.SetTimeout( 200 );
    aSyntaxIdleTimer.SetTimeoutHdl( LINK( this, TextEditImp, SyntaxTimerHdl ) );

    aImplSyntaxIdleTimer.SetTimeout( 1 );
    aImplSyntaxIdleTimer.SetTimeoutHdl( LINK( this, TextEditImp, SyntaxTimerHdl ) );

    StartListening( *pTextEngine );

    HideTipTimer.SetTimeout( 5000 );    // 5 seconds
    ShowTipTimer.SetTimeout( 500 );     // 1/2 seconds
    HideTipTimer.SetTimeoutHdl( LINK( this, TextEditImp, HideVarContents ) );
    ShowTipTimer.SetTimeoutHdl( LINK( this, TextEditImp, ShowVarContents ) );
}

TextEditImp::~TextEditImp()
{
    pTextEngine->RemoveView( pTextView );
    delete pTextView;
    delete pTextEngine;
}

sal_Bool TextEditImp::ViewMoved()
{
    sal_Bool bOld = bViewMoved;
    bViewMoved = sal_False;
    return bOld;
}

void TextEditImp::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    (void) rBC; /* avoid warning about unused parameter */
    if ( rHint.ISA( TextHint ) )
    {
        const TextHint& rTextHint = (const TextHint&)rHint;
        if( rTextHint.GetId() == TEXT_HINT_VIEWSCROLLED )
        {
            pAppEdit->pHScroll->SetThumbPos( pTextView->GetStartDocPos().X() );
            pAppEdit->pVScroll->SetThumbPos( pTextView->GetStartDocPos().Y() );
            if ( ((TextEdit*)(pAppEdit->pDataEdit))->GetBreakpointWindow() )
                ((TextEdit*)(pAppEdit->pDataEdit))->GetBreakpointWindow()->Scroll( 0, ((TextEdit*)(pAppEdit->pDataEdit))->GetBreakpointWindow()->GetCurYOffset() - pTextView->GetStartDocPos().Y() );
            bViewMoved = sal_True;
        }
        else if( rTextHint.GetId() == TEXT_HINT_TEXTHEIGHTCHANGED )
        {
            if ( pTextView->GetStartDocPos().Y() )
            {
                long nOutHeight = GetOutputSizePixel().Height();
                long nTextHeight = pTextEngine->GetTextHeight();
                if ( nTextHeight < nOutHeight )
                    pTextView->Scroll( 0, pTextView->GetStartDocPos().Y() );
            }

            pAppEdit->SetScrollBarRanges();
        }
        else if( rTextHint.GetId() == TEXT_HINT_TEXTFORMATTED )
        {
            sal_uIntPtr nWidth = pTextEngine->CalcTextWidth();
            if ( (sal_uIntPtr)nWidth != pAppEdit->nCurTextWidth )
            {
                pAppEdit->nCurTextWidth = nWidth;
                if ( pAppEdit->pHScroll )
                {   // Initialization finished?
                    pAppEdit->pHScroll->SetRange( Range( 0, (long)nWidth) );
                    pAppEdit->pHScroll->SetThumbPos( pTextView->GetStartDocPos().X() );
                }
            }
        }
        else if( rTextHint.GetId() == TEXT_HINT_PARAINSERTED )
        {
            if ( ((TextEdit*)(pAppEdit->pDataEdit))->GetBreakpointWindow() )
                ((TextEdit*)(pAppEdit->pDataEdit))->GetBreakpointWindow()->AdjustBreakpoints( rTextHint.GetValue()+1, sal_True );
        }
        else if( rTextHint.GetId() == TEXT_HINT_PARAREMOVED )
        {
            if ( ((TextEdit*)(pAppEdit->pDataEdit))->GetBreakpointWindow() )
                ((TextEdit*)(pAppEdit->pDataEdit))->GetBreakpointWindow()->AdjustBreakpoints( rTextHint.GetValue()+1, sal_False );

            // Itchy adaption for two signs at line ends instead of one (hard coded default)
            pTextEngine->SetMaxTextLen( STRING_MAXLEN - pTextEngine->GetParagraphCount() );
        }
        else if( rTextHint.GetId() == TEXT_HINT_FORMATPARA )
        {
            DoDelayedSyntaxHighlight(
                sal::static_int_cast< xub_StrLen >( rTextHint.GetValue() ) );
            if ( pTextView->GetTextEngine()->IsModified() )
                ModifyHdl.Call( NULL );
        }
    }
}

#define TEXTATTR_SPECHIAL   55
class TextAttribSpechial : public TextAttrib
{
private:
    FontWeight  maFontWeight;

public:
    TextAttribSpechial( const FontWeight& rFontWeight );
    TextAttribSpechial( const TextAttribSpechial& rAttr );
    ~TextAttribSpechial() {;}

    virtual void            SetFont( Font& rFont ) const;
    virtual TextAttrib*     Clone() const;
    virtual int             operator==( const TextAttrib& rAttr ) const;
};

TextAttribSpechial::TextAttribSpechial( const FontWeight& rFontWeight )
    : TextAttrib( TEXTATTR_SPECHIAL ), maFontWeight( rFontWeight )
{}

TextAttribSpechial::TextAttribSpechial( const TextAttribSpechial& rAttr )
    : TextAttrib( rAttr ), maFontWeight( rAttr.maFontWeight )
{}

void TextAttribSpechial::SetFont( Font& rFont ) const
{
    rFont.SetWeight( maFontWeight );
}

TextAttrib* TextAttribSpechial::Clone() const
{
    return new TextAttribSpechial( *this );
}

int TextAttribSpechial::operator==( const TextAttrib& rAttr ) const
{
    return ( ( TextAttrib::operator==(rAttr ) ) &&
                ( maFontWeight == ((const TextAttribSpechial&)rAttr).maFontWeight ) );
}

void TextEditImp::ImpDoHighlight( const String& rSource, sal_uIntPtr nLineOff )
{
    SbTextPortions aPortionList;
    pAppEdit->GetBasicFrame()->Basic().Highlight( rSource, aPortionList );

    sal_uInt16 nCount = aPortionList.Count();
    if ( !nCount )
        return;

    SbTextPortion& rLast = aPortionList[nCount-1];
    if ( rLast.nStart > rLast.nEnd )    // Only up to the bug of MD repaired
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_FAIL( "MD-Bug is not repaired!" );
#endif
        nCount--;
        aPortionList.Remove( nCount);
        if ( !nCount )
            return;
    }

    // here is the postprocessing for types for the TestTool
    sal_uInt16 i;
    sal_Bool bWasTTControl = sal_False;
    for ( i = 0; i < aPortionList.Count(); i++ )
    {
        SbTextPortion& r = aPortionList[i];
        if ( r.nStart > r.nEnd )    // Only up to the bug of MD repaired
            continue;

        SbTextType eType = r.eType;
        Color aColor;
        switch ( eType )
        {
            case SB_SYMBOL:
                {
                     String aSymbol = rSource.Copy( r.nStart, r.nEnd - r.nStart +1 );
                    r.eType = pAppEdit->GetBasicFrame()->Basic().GetSymbolType( aSymbol, bWasTTControl );

                    if ( r.eType == TT_CONTROL )
                        bWasTTControl = sal_True;
                    else
                        bWasTTControl = sal_False;
                }
                break;
            case SB_PUNCTUATION:
                {
                     String aPunctuation = rSource.Copy( r.nStart, r.nEnd - r.nStart +1 );
                    if ( aPunctuation.CompareToAscii( "." ) != COMPARE_EQUAL )
                        bWasTTControl = sal_False;
                }
                break;
            default:
                bWasTTControl = sal_False;
        }
    }

        // Only the blanks and tabs had to be attributed.
        // If there are two equal attributes one after another,
        // they are optimized by the EditEngine.
        xub_StrLen nLastEnd = 0;
#ifdef DBG_UTIL
        xub_StrLen nLine1 = aPortionList[0].nLine;
#endif
        for ( i = 0; i < nCount; i++ )
        {
            SbTextPortion& r = aPortionList[i];
            DBG_ASSERT( r.nLine == nLine1, "doch mehrere Zeilen ?" );
            DBG_ASSERT( r.nStart <= r.nEnd, "Highlight: Start > End?" );
            if ( r.nStart > r.nEnd )    // Nur bis Bug von MD behoben
                continue;

            if ( r.nStart > nLastEnd )
            {
                // can I trust that all of them except
                // blank und tab will be highlighted?!
                r.nStart = nLastEnd;
            }
            nLastEnd = r.nEnd+1;
            if ( ( i == (nCount-1) ) && ( r.nEnd < rSource.Len() ) )
                r.nEnd = rSource.Len()-1;
        }

    sal_Bool bWasModified = pTextEngine->IsModified();
    for ( i = 0; i < aPortionList.Count(); i++ )
    {
        SbTextPortion& r = aPortionList[i];
        if ( r.nStart > r.nEnd )    // Only up to the bug of MD repaired
            continue;

        SbTextType eCol = r.eType;
        Color aColor;
        sal_uIntPtr nLine = nLineOff+r.nLine-1; // -1, because BASIC starts with 1
        switch ( +eCol )
        {
            case SB_KEYWORD:
                aColor = Color( COL_BLUE );
                break;
            case SB_SYMBOL:
                aColor = Color( RGB_COLORDATA( 0x00, 0x60, 0x00 ) );
                break;
            case SB_STRING:
                aColor = Color( COL_RED );
                break;
            case SB_NUMBER:
                aColor = Color( COL_MAGENTA );
                break;
            case SB_PUNCTUATION:
                aColor = Color( COL_BLACK );
                break;
            case SB_COMMENT:
                aColor = Color( COL_GRAY );
                break;
            case TT_KEYWORD:
            case TT_LOCALCMD:
                aColor = Color( COL_LIGHTBLUE );
                break;
            case TT_REMOTECMD:
                aColor = Color( RGB_COLORDATA( 0x00, 0xB0, 0xB0 ) );
                break;
            case TT_CONTROL:
            case TT_SLOT:
                aColor = Color( RGB_COLORDATA( 0x00, 0xB0, 0x00 ) );
                break;
            case TT_METHOD:
                aColor = Color( RGB_COLORDATA( 0x00, 0xB0, 0x00 ) );
                break;
            case TT_NOMETHOD:
                {
                    aColor = Color( COL_RED );
                    pTextEngine->SetAttrib( TextAttribSpechial( WEIGHT_BOLD ), nLine, r.nStart, r.nEnd+1 );
                }
                break;
            default:
                {
                    aColor = Color( RGB_COLORDATA( 0xff, 0x80, 0x80 ) );
                    OSL_FAIL( "Unknown syntax color" );
                }
        }
        pTextEngine->SetAttrib( TextAttribFontColor( aColor ), nLine, r.nStart, r.nEnd+1 );
    }
    // Highlighting should not modify the text
    pTextEngine->SetModified( bWasModified );
}

void TextEditImp::DoSyntaxHighlight( sal_uIntPtr nPara )
{
    // Due to delayed syntax highlight it can happen that the
    // paragraph does no longer exist
    if ( nPara < pTextEngine->GetParagraphCount() )
    {
        // unfortunatly I don't know if exact this line Modified() ...
        pTextEngine->RemoveAttribs( nPara );
        String aSource( pTextEngine->GetText( nPara ) );
        ImpDoHighlight( aSource, nPara );
    }
}

void TextEditImp::DoDelayedSyntaxHighlight( xub_StrLen nPara )
{
    // Paragraph is added to 'List', processed in TimerHdl.
    // => Do not manipulate paragraphs while EditEngine is formatting

    if ( !bHighlightning && bDoSyntaxHighlight )
    {
        if ( bDelayHighlight )
        {
            aSyntaxLineTable.Insert( nPara, (void*)(sal_uIntPtr)1 );
            aSyntaxIdleTimer.Start();
        }
        else
            DoSyntaxHighlight( nPara );
    }
}

IMPL_LINK( TextEditImp, SyntaxTimerHdl, Timer *, EMPTYARG )
{
    DBG_ASSERT( pTextView, "Not yet a View but Syntax-Highlight ?!" );
    pTextEngine->SetUpdateMode( sal_False );

    bHighlightning = sal_True;
    sal_uInt16 nLine;
    while ( aSyntaxLineTable.First() && !Application::AnyInput( INPUT_MOUSEANDKEYBOARD ) )
    {
        nLine = (sal_uInt16)aSyntaxLineTable.GetCurKey();
        DoSyntaxHighlight( nLine );
        aSyntaxLineTable.Remove( nLine );
    }

    sal_Bool bWasModified = pTextEngine->IsModified();
    if ( aSyntaxLineTable.Count() > 3 )                 // Without VDev
    {
        pTextEngine->SetUpdateMode( sal_True );
        pTextView->ShowCursor( sal_True, sal_True );
    }
    else
        pTextEngine->SetUpdateMode( sal_True );             // ! With VDev

    // SetUpdateMode( sal_True ) soll kein Modify setzen
    pTextEngine->SetModified( bWasModified );

    // SyntaxTimerHdl wird gerufen, wenn Text-Aenderung
    // => gute Gelegenheit, Textbreite zu ermitteln!
    bHighlightning = sal_False;

    if ( aSyntaxLineTable.First() )
        aImplSyntaxIdleTimer.Start();

    return 0;
}

void TextEditImp::InvalidateSyntaxHighlight()
{
    for ( xub_StrLen i = 0; i < pTextEngine->GetParagraphCount(); i++ )
        DoDelayedSyntaxHighlight( i );
}

void TextEditImp::SyntaxHighlight( sal_Bool bNew )
{
    if ( ( bNew && bDoSyntaxHighlight ) || ( !bNew && !bDoSyntaxHighlight ) )
        return;

    bDoSyntaxHighlight = bNew;
    if ( !pTextEngine )
        return;


    if ( bDoSyntaxHighlight )
    {
        InvalidateSyntaxHighlight();
    }
    else
    {
        aSyntaxIdleTimer.Stop();
        pTextEngine->SetUpdateMode( sal_False );
        for ( sal_uIntPtr i = 0; i < pTextEngine->GetParagraphCount(); i++ )
            pTextEngine->RemoveAttribs( i );

        pTextEngine->SetUpdateMode( sal_True );
        pTextView->ShowCursor(sal_True, sal_True );
    }
}


void TextEditImp::SetFont( const Font& rNewFont )
{
    pTextEngine->SetFont(rNewFont);
}

sal_Bool TextEditImp::IsModified()
{
    return pTextEngine->IsModified();
}

void TextEditImp::KeyInput( const KeyEvent& rKeyEvent )
{
    sal_Bool bWasModified = pTextView->GetTextEngine()->IsModified();
    pTextView->GetTextEngine()->SetModified( sal_False );

    if ( !pTextView->KeyInput( rKeyEvent ) )
        Window::KeyInput( rKeyEvent );

    if ( pTextView->GetTextEngine()->IsModified() )
        ModifyHdl.Call( NULL );
    else
        pTextView->GetTextEngine()->SetModified( bWasModified );
}

void TextEditImp::Paint( const Rectangle& rRect )
{
    pTextView->Paint( rRect );
}

void TextEditImp::MouseButtonUp( const MouseEvent& rMouseEvent )
{
    pTextView->MouseButtonUp( rMouseEvent );
}


void TextEditImp::Command( const CommandEvent& rCEvt )
{
    switch( rCEvt.GetCommand() ) {
        case COMMAND_CONTEXTMENU:
        case COMMAND_WHEEL:
            GetParent()->Command( rCEvt );
            break;
        default:
            pTextView->Command( rCEvt );
    }
}


void TextEditImp::MouseButtonDown( const MouseEvent& rMouseEvent )
{
    pTextView->MouseButtonDown( rMouseEvent );
    HideVarContents( NULL );
    ShowTipTimer.Stop();
}


void TextEditImp::MouseMove( const MouseEvent& rMEvt )
{
    pTextView->MouseMove( rMEvt );
    HideVarContents( NULL );
    if ( rMEvt.GetButtons() == 0 )
        ShowTipTimer.Start();
    if ( rMEvt.IsLeaveWindow() )
        ShowTipTimer.Stop();
}


IMPL_LINK( TextEditImp, HideVarContents, void*, EMPTYARG )
{
    if ( nTipId )
    {
        Help::HideTip( nTipId );
        nTipId = 0;
        aTipWord = String();
    }
    return 0;
}

static const char cSuffixes[] = "%&!#@$";


SbxBase* TextEditImp::GetSbxAtMousePos( String &aWord )
{
    Point aPos = GetPointerPosPixel();
    Point aDocPos = pTextView->GetDocPos( aPos );
    aWord = pTextEngine->GetWord( pTextEngine->GetPaM( aDocPos ) );

    if ( aWord.Len() )
    {
        xub_StrLen nLastChar = aWord.Len()-1;
        String aSuffixes = CUniString( cSuffixes );
        if ( aSuffixes.Search( aWord.GetChar(nLastChar) ) != STRING_NOTFOUND )
            aWord.Erase( nLastChar, 1 );
        // because perhaps TestTools throws an error
        sal_Bool bWasError = SbxBase::IsError();
        pAppEdit->GetBasicFrame()->Basic().DebugFindNoErrors( sal_True );
        SbxBase* pSBX = StarBASIC::FindSBXInCurrentScope( aWord );
        pAppEdit->GetBasicFrame()->Basic().DebugFindNoErrors( sal_False );
        DBG_ASSERT( !( !bWasError && SbxBase::IsError()), "Error generated while retrieving Variable data for viewing" );
        if ( !bWasError && SbxBase::IsError() )
            SbxBase::ResetError();

        return pSBX;
    }
    return NULL;
}


IMPL_LINK( TextEditImp, ShowVarContents, void*, EMPTYARG )
{
    String aWord;
    SbxBase* pSBX = GetSbxAtMousePos( aWord );
    String aHelpText;
    Point aPos = GetPointerPosPixel();

    if ( pSBX && pSBX->ISA( SbxVariable ) && !pSBX->ISA( SbxMethod ) )
    {
        SbxVariable* pVar = (SbxVariable*)pSBX;
        SbxDataType eType = pVar->GetType();
        if ( eType == SbxOBJECT )
        {
        // Can cause a crash: Type == Object does not mean pVar == Object
            if ( pVar->GetObject() && pVar->GetObject()->ISA( SbxObject ) )
                aHelpText = ((SbxObject*)(pVar->GetObject()))->GetClassName();
            else
                aHelpText = CUniString("Object");
        }
        else if ( eType & SbxARRAY )
            aHelpText = CUniString("{...}");
        else if ( eType != SbxEMPTY )
        {
            aHelpText = pVar->GetName();
            if ( !aHelpText.Len() ) // Name is not copied in arguments
                aHelpText = aWord;
            aHelpText += '=';
            aHelpText += pVar->GetString();
        }
    }


    if ( aHelpText.Len() && aTipPos != aPos && aTipWord != aWord )
    {
        if ( nTipId )
            Help::HideTip( nTipId );
        nTipId = Help::ShowTip( this, Rectangle(), aHelpText );

        HideTipTimer.Start();
        aTipWord = aWord;
        aTipPos = aPos;
    }
    if ( nTipId && aTipPos != aPos )
    {
        Help::HideTip( nTipId );
        nTipId = 0;
        aTipWord = String();
    }

    return 0;
}


void TextEditImp::BuildKontextMenu( PopupMenu *&pMenu )
{
    String aWord;
    SbxBase* pSBX = GetSbxAtMousePos( aWord );
    if ( pSBX && pSBX->ISA( SbxVariable ) && !pSBX->ISA( SbxMethod ) )
    {
        SbxVariable* pVar = (SbxVariable*)pSBX;
        SbxDataType eType = pVar->GetType();

        if ( ( eType & ( SbxVECTOR | SbxARRAY | SbxBYREF )) == 0 )
        {

/*
Boolean
Currency
Date
Double
Integer
Long
Object
Single
String
Variant(Empty)
*/
            switch ( eType )
            {
                case SbxBOOL:
                case SbxDOUBLE:
                case SbxINTEGER:
                case SbxLONG:
                case SbxSINGLE:
                case SbxSTRING:

                case SbxVARIANT:    // does not occure, instead SbxEMPTY
                case SbxEMPTY:
                    {
                        pAppEdit->GetBasicFrame()->SetEditVar( pVar );
                        if ( !pMenu )
                            pMenu = new PopupMenu();
                        else
                            pMenu->InsertSeparator();

                        pMenu->InsertItem( RID_POPUPEDITVAR, ((BasicFrame*)GetpApp()->GetAppWindow())->GenRealString( GEN_RES_STR1( IDS_EDIT_VAR, aWord ) ) );
                    }
                    break;
                default:
                    ;
            }
        }
    }
}




DBG_NAME(TextEdit)

TextEdit::TextEdit( AppEdit* pParent, const WinBits& aBits )
: pBreakpointWindow( NULL )
, bFileWasUTF8( sal_False )
, bSaveAsUTF8( sal_False )
, aEdit( pParent, aBits | WB_NOHIDESELECTION )
{
DBG_CTOR(TextEdit,0);
}

TextEdit::~TextEdit()
{DBG_DTOR(TextEdit,0);}

void TextEdit::Highlight( sal_uIntPtr nLine, xub_StrLen nCol1, xub_StrLen nCol2 )
{
    if ( nLine )    // Should not occure but at 'Sub expected' in first line
        nLine--;

    String s = aEdit.pTextEngine->GetText( nLine );

    if( nCol1 == STRING_NOTFOUND )
    {
        // No column given
        nCol1 = 0;
        nCol2 = STRING_NOTFOUND;
    }
    if( nCol2 == STRING_NOTFOUND )
    {
        nCol2 = s.Len();
    }
    // Adaption to the Precompiler | EditText != Compilied Text
    if ( nCol2 > s.Len() )
        nCol2 = s.Len();
    if ( nCol1 >= nCol2 )
        nCol1 = 0;

    // Because nCol2 *may* point after the current statement
    // (because the next one starts there) there are space
    // that must be removed
    sal_Bool bColon = sal_False;

    while ( s.GetChar( nCol2 ) == ' ' && nCol2 > nCol1 && !bColon )
    {
        nCol2--;
        if ( s.GetChar( nCol2 ) == ':' )
        {
            nCol2--;
            bColon = sal_True;
        }
    }

    aEdit.ViewMoved();
    aEdit.pTextView->SetSelection( TextSelection(TextPaM(nLine,nCol2+1), TextPaM(nLine,nCol1)) );
    if ( aEdit.ViewMoved() )
    {
        aEdit.pTextView->SetSelection( TextSelection(TextPaM(TEXT_PARA_ALL,1)) );
        aEdit.pTextView->SetSelection( TextSelection(TextPaM((nLine>=2?nLine-2:0),nCol2+1)) );
        aEdit.pTextView->SetSelection( TextSelection(TextPaM(nLine,nCol2+1), TextPaM(nLine,nCol1)) );
    }
}


void TextEdit::Delete(){ aEdit.pTextView->KeyInput( KeyEvent( 0, KeyCode( KEYFUNC_DELETE ) )); }
void TextEdit::Cut(){ aEdit.pTextView->Cut(); }
void TextEdit::Copy(){ aEdit.pTextView->Copy(); }
void TextEdit::Paste(){ aEdit.pTextView->Paste(); }
void TextEdit::Undo(){ aEdit.pTextView->Undo(); }
void TextEdit::Redo(){ aEdit.pTextView->Redo(); }
String TextEdit::GetSelected(){ return aEdit.pTextView->GetSelected(); }
TextSelection TextEdit::GetSelection() const{ return aEdit.pTextView->GetSelection(); }
void TextEdit::SetSelection( const TextSelection& rSelection ){ aEdit.pTextView->SetSelection( rSelection ); }

sal_uInt16 TextEdit::GetLineNr() const
{
    return sal::static_int_cast< sal_uInt16 >(
        aEdit.pTextView->GetSelection().GetEnd().GetPara()+1);
}

void TextEdit::ReplaceSelected( const String& rStr ){ aEdit.pTextView->InsertText(rStr); }
sal_Bool TextEdit::IsModified(){ return aEdit.IsModified(); }

String TextEdit::GetText() const
{
    return aEdit.pTextEngine->GetText( GetSystemLineEnd() );
}

void TextEdit::SetText( const String& rStr ){ aEdit.pTextEngine->SetText(rStr); aEdit.pTextEngine->SetModified( sal_False ); }
void TextEdit::SetModifyHdl( Link l ){ aEdit.SetModifyHdl(l); }
sal_Bool TextEdit::HasText() const { return aEdit.pTextEngine->GetTextLen() > 0; }

// Search from the beginning or at mark + 1
sal_Bool TextEdit::Find( const String& s )
{
    DBG_CHKTHIS(TextEdit,0);

    TextSelection aSelection = aEdit.pTextView->GetSelection();
    sal_uIntPtr nPara = aSelection.GetStart().GetPara();
    xub_StrLen nIndex = aSelection.GetStart().GetIndex();

    if ( aSelection.HasRange() )
        nIndex ++;

    while ( nPara <= aEdit.pTextEngine->GetParagraphCount() )
    {
        String aText = aEdit.pTextEngine->GetText( nPara );

        nIndex = aText.Search( s, nIndex );
        if( nIndex != STRING_NOTFOUND )
        {
            aEdit.pTextView->SetSelection( TextSelection( TextPaM( nPara, nIndex ), TextPaM( nPara, nIndex + s.Len() ) ) );
            return sal_True;
        }
        nIndex = 0;
        nPara++;
    }
    return sal_False;
}

sal_Bool TextEdit::Load( const String& aName )
{
DBG_CHKTHIS(TextEdit,0);
    sal_Bool bOk = sal_True;
    SvFileStream aStrm( aName, STREAM_STD_READ );
    if( aStrm.IsOpen() )
    {
        String aText, aLine, aLineBreak;
        sal_Bool bIsFirstLine = sal_True;
        aLineBreak += '\n';
        aLineBreak.ConvertLineEnd();
        rtl_TextEncoding aFileEncoding = RTL_TEXTENCODING_IBM_850;
        while( !aStrm.IsEof() && bOk )
        {
            aStrm.ReadByteStringLine( aLine, aFileEncoding );
            if ( bIsFirstLine && IsTTSignatureForUnicodeTextfile( aLine ) )
            {
                aFileEncoding = RTL_TEXTENCODING_UTF8;
                bFileWasUTF8 = sal_True;
            }
            else
            {
                if ( !bIsFirstLine )
                    aText += aLineBreak;
                aText += aLine;
                bIsFirstLine = sal_False;
            }
            if( aStrm.GetError() != SVSTREAM_OK )
                bOk = sal_False;
        }
        SetText( aText );
    }
    else
        bOk = sal_False;
    return bOk;
}

sal_Bool TextEdit::Save( const String& aName )
{
DBG_CHKTHIS(TextEdit,0);
    sal_Bool bOk = sal_True;
    SvFileStream aStrm( aName, STREAM_STD_WRITE | STREAM_TRUNC );
    rtl_TextEncoding aFileEncoding = RTL_TEXTENCODING_IBM_850;
    if( aStrm.IsOpen() )
    {
        if ( bFileWasUTF8 || bSaveAsUTF8 )
        {
            aStrm << TT_SIGNATURE_FOR_UNICODE_TEXTFILES;
            aStrm << sal_Char(_LF);
            aFileEncoding = RTL_TEXTENCODING_UTF8;
        }
        String aSave = GetText();
        aSave.ConvertLineEnd(LINEEND_LF);
        aStrm << ByteString( aSave, aFileEncoding ).GetBuffer();
        if( aStrm.GetError() != SVSTREAM_OK )
            bOk = sal_False;
        else
            aEdit.pTextEngine->SetModified(sal_False);
    } else bOk = sal_False;
    return bOk;
}


void TextEdit::BuildKontextMenu( PopupMenu *&pMenu )
{
    DataEdit::BuildKontextMenu( pMenu );
    aEdit.BuildKontextMenu( pMenu );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
