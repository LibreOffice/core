/*************************************************************************
 *
 *  $RCSfile: edit.cxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: mt $ $Date: 2001-08-20 11:09:37 $
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

#define _SV_EDIT_CXX

#ifndef _SV_RC_H
#include <rc.h>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_DECOVIEW_HXX
#include <decoview.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_CURSOR_HXX
#include <cursor.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif
#ifndef _SV_SVIDS_HRC
#include <svids.hrc>
#endif
#ifndef _SV_MENU_HXX
#include <menu.hxx>
#endif
#ifndef _VCL_CMDEVT_H
#include <cmdevt.h>
#endif
#ifndef _SV_SUBEDIT_HXX
#include <subedit.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <edit.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif

#include <vos/mutex.hxx>


#ifndef _COM_SUN_STAR_I18N_XBREAKITERATOR_HPP_
#include <com/sun/star/i18n/XBreakIterator.hpp>
#endif

#ifndef _COM_SUN_STAR_I18N_CHARACTERITERATORMODE_HPP_
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#endif

#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARD_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XFLUSHABLECLIPBOARD_HPP_
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_DNDCONSTANS_HPP_
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDRAGGESTURERECOGNIZER_HPP_
#include <com/sun/star/datatransfer/dnd/XDragGestureRecognizer.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGET_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#endif



#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <rtl/memory.h>

#include <unohelp.hxx>


#pragma hdrstop

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

#define EXTRAOFFSET_X   2


// =======================================================================

class TextDataObject :  public ::com::sun::star::datatransfer::XTransferable,
                        public ::cppu::OWeakObject

{
private:
//    uno::Reference< datatransfer::clipboard::XClipboard >& mxClipboard;
    String          maText;

public:
//                  TextDataObject( uno::Reference< datatransfer::clipboard::XClipboard >& rxClipboard, const String& rText );
                    TextDataObject( const String& rText );
                    ~TextDataObject();

    String&         GetString() { return maText; }

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

    // ::com::sun::star::datatransfer::XTransferable
    ::com::sun::star::uno::Any SAL_CALL getTransferData( const ::com::sun::star::datatransfer::DataFlavor& aFlavor ) throw(::com::sun::star::datatransfer::UnsupportedFlavorException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& aFlavor ) throw(::com::sun::star::uno::RuntimeException);
};

// TextDataObject::TextDataObject( uno::Reference< datatransfer::clipboard::XClipboard >& rxClipboard, const String& rText ) : maText( rText )
TextDataObject::TextDataObject( const String& rText ) : maText( rText )
{
//    mxClipboard = rxClipboard;
}

TextDataObject::~TextDataObject()
{
}

// uno::XInterface
uno::Any TextDataObject::queryInterface( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType, SAL_STATIC_CAST( datatransfer::XTransferable*, this ) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// datatransfer::XTransferable
uno::Any TextDataObject::getTransferData( const datatransfer::DataFlavor& rFlavor ) throw(datatransfer::UnsupportedFlavorException, io::IOException, uno::RuntimeException)
{
    uno::Any aAny;

    ULONG nT = SotExchange::GetFormat( rFlavor );
    if ( nT == SOT_FORMAT_STRING )
    {
        aAny <<= (::rtl::OUString)GetString();
    }
    else
    {
        throw datatransfer::UnsupportedFlavorException();
    }
    return aAny;
}

uno::Sequence< datatransfer::DataFlavor > TextDataObject::getTransferDataFlavors(  ) throw(uno::RuntimeException)
{
    uno::Sequence< datatransfer::DataFlavor > aDataFlavors(1);
    SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aDataFlavors.getArray()[0] );
    return aDataFlavors;
}

sal_Bool TextDataObject::isDataFlavorSupported( const datatransfer::DataFlavor& rFlavor ) throw(uno::RuntimeException)
{
    ULONG nT = SotExchange::GetFormat( rFlavor );
    return ( nT == SOT_FORMAT_STRING );
}







// - Redo
// - Bei Tracking-Cancel DefaultSelection wieder herstellen

// =======================================================================

static FncGetSpecialChars pImplFncGetSpecialChars = NULL;

// =======================================================================

#define EDIT_ALIGN_LEFT             1
#define EDIT_ALIGN_CENTER           2
#define EDIT_ALIGN_RIGHT            3

#define EDIT_DEL_LEFT               1
#define EDIT_DEL_RIGHT              2

#define EDIT_DELMODE_SIMPLE         11
#define EDIT_DELMODE_RESTOFWORD     12
#define EDIT_DELMODE_RESTOFCONTENT  13

// =======================================================================

static uno::Reference < i18n::XBreakIterator > ImplGetBreakIterator()
{
    static uno::Reference < i18n::XBreakIterator > xB;
    if ( !xB.is() )
        xB = vcl::unohelper::CreateBreakIterator();
    return xB;
}

// =======================================================================

struct DDInfo
{
    Cursor          aCursor;
    Selection       aDndStartSel;
    xub_StrLen      nDropPos;
    BOOL            bStarterOfDD;
    BOOL            bDroppedInMe;
    BOOL            bVisCursor;

    DDInfo()
    {
        aCursor.SetStyle( CURSOR_SHADOW );
        nDropPos = 0;
        bStarterOfDD = FALSE;
        bDroppedInMe = FALSE;
        bVisCursor = FALSE;
    }
};

// =======================================================================

struct Impl_IMEInfos
{
    String      aOldTextAfterStartPos;
    USHORT*     pAttribs;
    xub_StrLen  nPos;
    xub_StrLen  nLen;
    BOOL        bCursor;
    BOOL        bWasCursorOverwrite;

                Impl_IMEInfos( xub_StrLen nPos, const String& rOldTextAfterStartPos );
                ~Impl_IMEInfos();

    void        CopyAttribs( const xub_StrLen* pA, xub_StrLen nL );
    void        DestroyAttribs();
};

// -----------------------------------------------------------------------

Impl_IMEInfos::Impl_IMEInfos( xub_StrLen nP, const String& rOldTextAfterStartPos )
 : aOldTextAfterStartPos( rOldTextAfterStartPos )
{
    nPos = nP;
    nLen = 0;
    bCursor = TRUE;
    pAttribs = NULL;
    bWasCursorOverwrite = FALSE;
}

// -----------------------------------------------------------------------

Impl_IMEInfos::~Impl_IMEInfos()
{
    delete pAttribs;
}

// -----------------------------------------------------------------------

void Impl_IMEInfos::CopyAttribs( const xub_StrLen* pA, xub_StrLen nL )
{
    nLen = nL;
    delete pAttribs;
    pAttribs = new USHORT[ nL ];
    rtl_copyMemory( pAttribs, pA, nL*sizeof(USHORT) );
}

// -----------------------------------------------------------------------

void Impl_IMEInfos::DestroyAttribs()
{
    delete pAttribs;
    pAttribs = NULL;
    nLen = 0;
}

// =======================================================================

Edit::Edit( WindowType nType ) :
    Control( nType )
{
    ImplInitData();
}

// -----------------------------------------------------------------------

Edit::Edit( Window* pParent, WinBits nStyle ) :
    Control( WINDOW_EDIT )
{
    ImplInitData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

Edit::Edit( Window* pParent, const ResId& rResId ) :
    Control( WINDOW_EDIT )
{
    ImplInitData();
    rResId.SetRT( RSC_EDIT );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );
    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

Edit::~Edit()
{
    delete mpDDInfo;
    Cursor* pCursor = GetCursor();
    if ( pCursor )
    {
        SetCursor( NULL );
        delete pCursor;
    }

    delete mpIMEInfos;

    if ( mpUpdateDataTimer )
        delete mpUpdateDataTimer;

//    GetDragGestureRecognizer()->removeDragGestureListener( this );
//    GetDropTarget()->removeDropTargetListener( this );
}

// -----------------------------------------------------------------------

void Edit::ImplInitData()
{
    mpSubEdit               = NULL;
    mpUpdateDataTimer       = NULL;
    mnXOffset               = 0;
    mnAlign                 = EDIT_ALIGN_LEFT;
    mnMaxTextLen            = EDIT_NOLIMIT;
    meAutocompleteAction    = AUTOCOMPLETE_KEYINPUT;
    mbModified              = FALSE;
    mbInternModified        = FALSE;
    mbReadOnly              = FALSE;
    mbInsertMode            = TRUE;
    mbClickedInSelection    = FALSE;
    mbActivePopup           = FALSE;
    mbIsSubEdit             = FALSE;
    mbInMBDown              = FALSE;
    mpDDInfo                = NULL;
    mpIMEInfos              = NULL;
    mcEchoChar              = 0;

    vcl::unohelper::DragAndDropWrapper* pDnDWrapper = new vcl::unohelper::DragAndDropWrapper( this );
    mxDnDListener = pDnDWrapper;
}

// -----------------------------------------------------------------------

void Edit::ImplInit( Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( nStyle );
    if ( !(nStyle & (WB_CENTER | WB_RIGHT)) )
        nStyle |= WB_LEFT;

    Control::ImplInit( pParent, nStyle, NULL );

    mbReadOnly = (nStyle & WB_READONLY) != 0;

    mnAlign = EDIT_ALIGN_LEFT;
    if ( nStyle & WB_RIGHT )
        mnAlign = EDIT_ALIGN_RIGHT;
    else if ( nStyle & WB_CENTER )
        mnAlign = EDIT_ALIGN_CENTER;

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    SetBackground( Wallpaper( rStyleSettings.GetFieldColor() ) );
    SetFillColor( rStyleSettings.GetFieldColor() );
    SetCursor( new Cursor );

    SetPointer( Pointer( POINTER_TEXT ) );
    ImplInitSettings( TRUE, TRUE, TRUE );

    uno::Reference< datatransfer::dnd::XDragGestureListener> xDGL( mxDnDListener, uno::UNO_QUERY );
    uno::Reference< datatransfer::dnd::XDragGestureRecognizer > xDGR = GetDragGestureRecognizer();
    if ( xDGR.is() )
    {
        xDGR->addDragGestureListener( xDGL );
        uno::Reference< datatransfer::dnd::XDropTargetListener> xDTL( mxDnDListener, uno::UNO_QUERY );
        GetDropTarget()->addDropTargetListener( xDTL );
        GetDropTarget()->setActive( sal_True );
        GetDropTarget()->setDefaultActions( datatransfer::dnd::DNDConstants::ACTION_COPY_OR_MOVE );
    }
}

// -----------------------------------------------------------------------

WinBits Edit::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOTABSTOP) )
        nStyle |= WB_TABSTOP;
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;

    return nStyle;
}

// -----------------------------------------------------------------------

BOOL Edit::IsCharInput( const KeyEvent& rKeyEvent )
{
    // In the future we must use new Unicode functions for this
    xub_Unicode cCharCode = rKeyEvent.GetCharCode();
    return ((cCharCode >= 32) && (cCharCode != 127) &&
            !rKeyEvent.GetKeyCode().IsControlMod());
}

// -----------------------------------------------------------------------

void Edit::ImplModified()
{
    mbModified = TRUE;
    Modify();
}

// -----------------------------------------------------------------------

void Edit::ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont = rStyleSettings.GetFieldFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetZoomedPointFont( aFont );
    }

    if ( bFont || bForeground )
    {
        Color aTextColor = rStyleSettings.GetFieldTextColor();
        if ( IsControlForeground() )
            aTextColor = GetControlForeground();
        SetTextColor( aTextColor );
    }

    if ( bBackground )
    {
        if( IsControlBackground() )
        {
            SetBackground( GetControlBackground() );
            SetFillColor( GetControlBackground() );
        }
        else
        {
            SetBackground( rStyleSettings.GetFieldColor() );
            SetFillColor( rStyleSettings.GetFieldColor() );
        }
    }
}

// -----------------------------------------------------------------------

XubString Edit::ImplGetText() const
{
    if ( mcEchoChar || (GetStyle() & WB_PASSWORD) )
    {
        XubString   aText;
        xub_Unicode cEchoChar;
        if ( mcEchoChar )
            cEchoChar = mcEchoChar;
        else
            cEchoChar = '*';
        aText.Fill( maText.Len(), cEchoChar );
        return aText;
    }
    else
        return maText;
}

// -----------------------------------------------------------------------

void Edit::ImplRepaint( xub_StrLen nStart, xub_StrLen nEnd )
{
    if ( !IsReallyVisible() )
        return;

    XubString aText = ImplGetText();
    if ( nStart >= aText.Len() )
        return;

    if ( nEnd > aText.Len() )
        nEnd = aText.Len();

    Cursor* pCursor = GetCursor();
    BOOL bVisCursor = pCursor ? pCursor->IsVisible() : FALSE;
    if ( pCursor )
        pCursor->Hide();

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    if ( IsEnabled() )
        ImplInitSettings( FALSE, TRUE, FALSE );
    else
        SetTextColor( rStyleSettings.GetDisableColor() );

    SetTextFillColor( IsControlBackground() ? GetControlBackground() : rStyleSettings.GetFieldColor() );

    // In der Hoehe zentrieren
    long    nH = GetOutputSize().Height();
    long    nTH = GetTextHeight();
    Point   aPos( mnXOffset, (nH-nTH)/2 );

    BOOL bDrawSelection = maSelection.Len() && ( HasFocus() || ( GetStyle() & WB_NOHIDESELECTION ) || mbActivePopup );

    if ( !bDrawSelection && !mpIMEInfos )
    {
        aPos.X() = GetTextWidth( aText, 0, nStart ) + mnXOffset + EXTRAOFFSET_X;
        DrawText( aPos, aText, nStart, nEnd - nStart );
    }
    else
    {
        Selection aTmpSel( maSelection );
        aTmpSel.Justify();

        xub_StrLen nIndex = nStart;
        while ( nIndex < nEnd )
        {
            xub_StrLen nTmpEnd = nEnd;
            USHORT      nAttr = 0;
            if ( mpIMEInfos && mpIMEInfos->pAttribs )
            {
                xub_StrLen nIMEEnd = mpIMEInfos->nPos+mpIMEInfos->nLen;
                if ( (nIndex < mpIMEInfos->nPos) && (nTmpEnd > mpIMEInfos->nPos) )
                {
                    nTmpEnd = mpIMEInfos->nPos;
                }
                else if ( (nIndex >= mpIMEInfos->nPos) && (nIndex < nIMEEnd) )
                {
                    // Attributweise ausgeben...
                    nTmpEnd = nIndex + 1;
                    if ( (nIndex >= mpIMEInfos->nPos) && (nIndex < (mpIMEInfos->nPos+mpIMEInfos->nLen)) )
                    {
                        nAttr = mpIMEInfos->pAttribs[nIndex-mpIMEInfos->nPos];
                        xub_StrLen nMax = mpIMEInfos->nPos+mpIMEInfos->nLen;
                        while ( ( nTmpEnd < nMax ) && ( mpIMEInfos->pAttribs[ nTmpEnd - mpIMEInfos->nPos ] == nAttr ) )
                            nTmpEnd++;
                    }
                }
            }
            else if ( bDrawSelection )
            {
                if ( ( nIndex < aTmpSel.Min() ) && ( nTmpEnd > aTmpSel.Min() ) )
                    nTmpEnd = (xub_StrLen)aTmpSel.Min();
                else if ( ( nIndex >= aTmpSel.Min() ) && ( nIndex < aTmpSel.Max() ) && ( nTmpEnd > aTmpSel.Max() ) )
                    nTmpEnd = (xub_StrLen)aTmpSel.Max();
            }

            ImplInitSettings( mpIMEInfos ? TRUE : FALSE, TRUE, FALSE );
            BOOL bSelected = bDrawSelection && ((xub_StrLen)aTmpSel.Min() <= nIndex ) && ((xub_StrLen)aTmpSel.Max() > nIndex );
            if ( bSelected || ( nAttr & EXTTEXTINPUT_ATTR_HIGHLIGHT) )
            {
                SetTextColor( rStyleSettings.GetHighlightTextColor() );
                SetTextFillColor( rStyleSettings.GetHighlightColor() );
            }
            else
            {
                SetTextFillColor( IsControlBackground() ? GetControlBackground() : rStyleSettings.GetFieldColor() );
            }

            if ( nAttr )
            {
                Font aFont = GetFont();
                if ( nAttr & EXTTEXTINPUT_ATTR_UNDERLINE )
                    aFont.SetUnderline( UNDERLINE_SINGLE );
                else if ( nAttr & EXTTEXTINPUT_ATTR_BOLDUNDERLINE )
                    aFont.SetUnderline( UNDERLINE_BOLD );
                else if ( nAttr & EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE )
                    aFont.SetUnderline( UNDERLINE_DOTTED );
                else if ( nAttr & EXTTEXTINPUT_ATTR_DASHDOTUNDERLINE )
                    aFont.SetUnderline( UNDERLINE_DOTTED );
                else if ( nAttr & EXTTEXTINPUT_ATTR_GRAYWAVELINE )
                {
                    aFont.SetUnderline( UNDERLINE_WAVE );
                    SetTextLineColor( Color( COL_LIGHTGRAY ) );
                }
                SetFont( aFont );

                if ( nAttr & EXTTEXTINPUT_ATTR_REDTEXT )
                    SetTextColor( Color( COL_RED ) );
                else if ( nAttr & EXTTEXTINPUT_ATTR_HALFTONETEXT )
                    SetTextColor( Color( COL_LIGHTGRAY ) );
            }
            aPos.X() = GetTextWidth( aText, 0, nIndex ) + mnXOffset + EXTRAOFFSET_X;
            DrawText( aPos, aText, nIndex, nTmpEnd - nIndex );
            nIndex = nTmpEnd;
        }
    }

    if ( bVisCursor && ( !mpIMEInfos || mpIMEInfos->bCursor ) )
        pCursor->Show();
}

// -----------------------------------------------------------------------

void Edit::ImplDelete( const Selection& rSelection, BYTE nDirection, BYTE nMode )
{
    XubString aText = ImplGetText();

    // loeschen moeglich?
    if ( !rSelection.Len() &&
         (((rSelection.Min() == 0) && (nDirection == EDIT_DEL_LEFT)) ||
          ((rSelection.Max() == aText.Len()) && (nDirection == EDIT_DEL_RIGHT))) )
        return;

    long nOldWidth = GetTextWidth( aText );
    Selection aSelection( rSelection );
    aSelection.Justify();

    if ( !aSelection.Len() )
    {
        uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();
        if ( nDirection == EDIT_DEL_LEFT )
        {
            if ( nMode == EDIT_DELMODE_RESTOFWORD )
            {
                i18n::Boundary aBoundary = xBI->getWordBoundary( maText, aSelection.Min(), GetSettings().GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES, sal_True );
                if ( aBoundary.startPos == aSelection.Min() )
                    aBoundary = xBI->previousWord( maText, aSelection.Min(), GetSettings().GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES );
                aSelection.Min() = aBoundary.startPos;
            }
            else if ( nMode == EDIT_DELMODE_RESTOFCONTENT )
               {
                aSelection.Min() = 0;
            }
            else
            {
                sal_Int32 nCount = 1;
                aSelection.Min() = xBI->previousCharacters( maText, aSelection.Min(), GetSettings().GetLocale(), i18n::CharacterIteratorMode::SKIPCHARACTER, nCount, nCount );
            }
        }
        else
        {
            if ( nMode == EDIT_DELMODE_RESTOFWORD )
            {
                i18n::Boundary aBoundary = xBI->nextWord( maText, aSelection.Max(), GetSettings().GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES );
                aSelection.Max() = aBoundary.startPos;
            }
            else if ( nMode == EDIT_DELMODE_RESTOFCONTENT )
            {
                aSelection.Max() = aText.Len();
            }
            else
            {
                sal_Int32 nCount = 1;
                aSelection.Max() = xBI->nextCharacters( maText, aSelection.Max(), GetSettings().GetLocale(), i18n::CharacterIteratorMode::SKIPCHARACTER, nCount, nCount );;
            }
        }
    }

    maText.Erase( (xub_StrLen)aSelection.Min(), (xub_StrLen)aSelection.Len() );
    maSelection.Min() = aSelection.Min();
    maSelection.Max() = aSelection.Min();
    ImplAlignAndPaint( (xub_StrLen)aSelection.Min(), nOldWidth );
    mbInternModified = TRUE;
}

// -----------------------------------------------------------------------

void Edit::ImplInsertText( const XubString& rStr, const Selection* pNewSel )
{
    Selection aSelection( maSelection );
    aSelection.Justify();

    XubString aNewText( rStr );
    aNewText.EraseAllChars( _LF );
    aNewText.EraseAllChars( _CR );
    aNewText.SearchAndReplaceAll( '\t', ' ' );

    if ( (maText.Len() + aNewText.Len() - aSelection.Len()) > mnMaxTextLen )
        return;

    long nOldWidth = GetTextWidth( ImplGetText() );

    if ( aSelection.Len() )
        maText.Erase( (xub_StrLen)aSelection.Min(), (xub_StrLen)aSelection.Len() );
    else if ( !mbInsertMode && (aSelection.Max() < maText.Len()) )
        maText.Erase( (xub_StrLen)aSelection.Max(), 1 );

    if ( aNewText.Len() )
        maText.Insert( aNewText, (xub_StrLen)aSelection.Min() );

    if ( !pNewSel )
    {
        maSelection.Min() = aSelection.Min() + aNewText.Len();
        maSelection.Max() = maSelection.Min();
    }
    else
    {
        maSelection = *pNewSel;
        if ( maSelection.Min() > maText.Len() )
            maSelection.Min() = maText.Len();
        if ( maSelection.Max() > maText.Len() )
            maSelection.Max() = maText.Len();
    }

    ImplAlignAndPaint( (xub_StrLen)aSelection.Min(), nOldWidth );
    mbInternModified = TRUE;
}

// -----------------------------------------------------------------------

void Edit::ImplSetText( const XubString& rText, const Selection* pNewSelection )
{
    // Der Text wird dadurch geloescht das der alte Text komplett 'selektiert'
    // wird, dann InsertText, damit flackerfrei.
    if ( (rText != maText) || (pNewSelection && (*pNewSelection != maSelection)) )
    {
        maSelection.Min() = 0;
        maSelection.Max() = maText.Len();
        if ( mnXOffset || HasPaintEvent() )
        {
            mnXOffset = 0;
            maText = rText;

            if ( pNewSelection )
                ImplSetSelection( *pNewSelection, FALSE );

            if ( mnXOffset && !pNewSelection )
                maSelection.Max() = 0;

            ImplAlign();
            Invalidate();
        }
        else
            ImplInsertText( rText, pNewSelection );
    }
}

// -----------------------------------------------------------------------

void Edit::ImplClearBackground( long nXStart, long nXEnd )
{
    Point aTmpPoint;
    Rectangle aRect( aTmpPoint, GetOutputSizePixel() );
    aRect.Left() = nXStart;
    aRect.Right() = nXEnd;

    Cursor* pCursor = HasFocus() ? GetCursor() : NULL;

    if ( pCursor )
        pCursor->Hide();

    Erase( aRect );

    if ( pCursor )
        pCursor->Show();
}

// -----------------------------------------------------------------------

void Edit::ImplShowCursor( BOOL bOnlyIfVisible )
{
    if ( !IsUpdateMode() || ( bOnlyIfVisible && !IsReallyVisible() ) )
        return;

    Cursor*     pCursor = GetCursor();
    XubString   aText = ImplGetText();
    long        nTextWidth = GetTextWidth( aText, 0, (xub_StrLen)maSelection.Max() );

    long nCursorWidth = 0;
    if ( !mbInsertMode && !maSelection.Len() && (maSelection.Max() < aText.Len()) )
        nCursorWidth = GetTextWidth( aText, (xub_StrLen)maSelection.Max(), 1 );
    long nCursorPosX = nTextWidth + mnXOffset + EXTRAOFFSET_X;

    // Cursor muss im sichtbaren Bereich landen:
    Size aOutSize = GetOutputSizePixel();
    if ( (nCursorPosX < 0) || (nCursorPosX >= aOutSize.Width()) )
    {
        long nOldXOffset = mnXOffset;

        if ( nCursorPosX < 0 )
        {
            mnXOffset = - nTextWidth;
            long nMaxX = 0;
            mnXOffset += aOutSize.Width() / 5;
            if ( mnXOffset > nMaxX )
                mnXOffset = nMaxX;
        }
        else
        {
            mnXOffset = (aOutSize.Width()-EXTRAOFFSET_X) - nTextWidth;
            // Etwas mehr?
            if ( (aOutSize.Width()-EXTRAOFFSET_X) < nTextWidth )
            {
                long nMaxNegX = (aOutSize.Width()-EXTRAOFFSET_X) - GetTextWidth( aText );
                mnXOffset -= aOutSize.Width() / 5;
                if ( mnXOffset < nMaxNegX )  // beides negativ...
                    mnXOffset = nMaxNegX;
            }
        }

        nCursorPosX = nTextWidth + mnXOffset + EXTRAOFFSET_X;
        if ( nCursorPosX == aOutSize.Width() )  // dann nicht sichtbar...
            nCursorPosX--;

        if ( mnXOffset != nOldXOffset )
        {
            if ( mnXOffset > (-EXTRAOFFSET_X) )
                ImplClearBackground( 0, mnXOffset+EXTRAOFFSET_X );
            ImplRepaint();
        }
    }

    long nTextHeight = GetTextHeight();
    long nCursorPosY = (aOutSize.Height()-nTextHeight) / 2;
    pCursor->SetPos( Point( nCursorPosX, nCursorPosY ) );
    pCursor->SetSize( Size( nCursorWidth, nTextHeight ) );
    pCursor->Show();
}

// -----------------------------------------------------------------------

void Edit::ImplAlign()
{
    long nTextWidth = GetTextWidth( ImplGetText() );
    long nOutWidth = GetOutputSizePixel().Width();

    if ( mnAlign == EDIT_ALIGN_LEFT )
    {
        if( mnXOffset && ( nTextWidth < nOutWidth ) )
            mnXOffset = 0;

    }
    else if ( mnAlign == EDIT_ALIGN_RIGHT )
    {
        long nMinXOffset = nOutWidth - nTextWidth;
        if ( mnXOffset < nMinXOffset )
            mnXOffset = nMinXOffset;
    }
    else if( mnAlign == EDIT_ALIGN_CENTER )
    {
        // Mit Abfrage schoener, wenn gescrollt, dann aber nicht zentriert im gescrollten Zustand...
//      if ( nTextWidth < nOutWidth )
            mnXOffset = (nOutWidth - nTextWidth) / 2;
    }
}


// -----------------------------------------------------------------------

void Edit::ImplAlignAndPaint( xub_StrLen nChangedFrom, long nOldWidth )
{
    long        nNewWidth = GetTextWidth( ImplGetText() );
    xub_StrLen  nPaintStart = nChangedFrom;

    long nOldXOffset = mnXOffset;
    ImplAlign();
    if ( mnAlign == EDIT_ALIGN_LEFT )
    {
        if ( nOldWidth > nNewWidth )
        {
            if ( mnXOffset != nOldXOffset )
            {
                nPaintStart = 0;
                if ( mnXOffset > (-EXTRAOFFSET_X) )
                    ImplClearBackground( 0, mnXOffset+EXTRAOFFSET_X );
            }
            ImplClearBackground( nNewWidth+mnXOffset+EXTRAOFFSET_X, nOldWidth+nOldXOffset+EXTRAOFFSET_X );
        }
    }
    else if ( mnAlign == EDIT_ALIGN_RIGHT )
    {
        nPaintStart = 0;
        ImplClearBackground( GetOutputSizePixel().Width()-Max( nOldWidth, nNewWidth )-1+EXTRAOFFSET_X, mnXOffset+1+EXTRAOFFSET_X );
    }
    else // EDIT_ALIGN_CENTER
    {
        nPaintStart = 0;
        ImplClearBackground( 0, mnXOffset + 1 + EXTRAOFFSET_X );
        ImplClearBackground( mnXOffset+nNewWidth-1, GetOutputSizePixel().Width() + EXTRAOFFSET_X );
    }

    ImplRepaint( nPaintStart, STRING_LEN );
    ImplShowCursor();
}

// -----------------------------------------------------------------------

xub_StrLen Edit::ImplGetCharPos( const Point& rWindowPos )
{
    return GetTextBreak( ImplGetText(), rWindowPos.X() - mnXOffset - EXTRAOFFSET_X );
}

// -----------------------------------------------------------------------

void Edit::ImplSetCursorPos( xub_StrLen nChar, BOOL bSelect )
{
    Selection aSelection( maSelection );
    aSelection.Max() = nChar;
    if ( !bSelect )
        aSelection.Min() = aSelection.Max();
    ImplSetSelection( aSelection );
}

// -----------------------------------------------------------------------

void Edit::ImplLoadRes( const ResId& rResId )
{
    Control::ImplLoadRes( rResId );

    xub_StrLen nTextLength = ReadShortRes();
    if ( nTextLength )
        SetMaxTextLen( nTextLength );
}

// -----------------------------------------------------------------------

void Edit::ImplCopy( uno::Reference< datatransfer::clipboard::XClipboard >& rxClipboard )
{
    if ( rxClipboard.is() )
    {
        TextDataObject* pDataObj = new TextDataObject( GetSelected() );
        const sal_uInt32 nRef = Application::ReleaseSolarMutex();
        rxClipboard->setContents( pDataObj, NULL );

        Reference< datatransfer::clipboard::XFlushableClipboard > xFlushableClipboard( rxClipboard, uno::UNO_QUERY );
        if( xFlushableClipboard.is() )
            xFlushableClipboard->flushClipboard();

        Application::AcquireSolarMutex( nRef );
    }
}

// -----------------------------------------------------------------------

void Edit::ImplPaste( uno::Reference< datatransfer::clipboard::XClipboard >& rxClipboard )
{
    if ( rxClipboard.is() )
    {
        const sal_uInt32 nRef = Application::ReleaseSolarMutex();
        uno::Reference< datatransfer::XTransferable > xDataObj = rxClipboard->getContents();
        Application::AcquireSolarMutex( nRef );
        if ( xDataObj.is() )
        {
            datatransfer::DataFlavor aFlavor;
            SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aFlavor );
            if ( xDataObj->isDataFlavorSupported( aFlavor ) )
            {
                uno::Any aData = xDataObj->getTransferData( aFlavor );
                ::rtl::OUString aText;
                aData >>= aText;
                ReplaceSelected( aText );
            }
        }
    }
}

// -----------------------------------------------------------------------

void Edit::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( mpSubEdit )
    {
        Control::MouseButtonDown( rMEvt );
        return;
    }

    xub_StrLen nChar = ImplGetCharPos( rMEvt.GetPosPixel() );
    Selection aSelection( maSelection );
    aSelection.Justify();

    if ( rMEvt.GetClicks() < 4 )
    {
        mbClickedInSelection = FALSE;
        if ( rMEvt.GetClicks() == 3 )
            ImplSetSelection( Selection( 0, 0xFFFF ) );
        else if ( rMEvt.GetClicks() == 2 )
        {
            uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();
             i18n::Boundary aBoundary = xBI->getWordBoundary( maText, aSelection.Max(), GetSettings().GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES, sal_True );
            ImplSetSelection( Selection( aBoundary.startPos, aBoundary.endPos ) );
        }
        else if ( !rMEvt.IsShift() && HasFocus() && aSelection.IsInside( nChar ) )
            mbClickedInSelection = TRUE;
        else if ( rMEvt.IsLeft() )
            ImplSetCursorPos( nChar, rMEvt.IsShift() );

        if ( !mbClickedInSelection && rMEvt.IsLeft() && ( rMEvt.GetClicks() == 1 ) )
            StartTracking( STARTTRACK_SCROLLREPEAT );
    }

    mbInMBDown = TRUE;  // Dann im GetFocus nicht alles selektieren
    GrabFocus();
    mbInMBDown = FALSE;
}

// -----------------------------------------------------------------------

void Edit::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( mbClickedInSelection && rMEvt.IsLeft() )
    {
        xub_StrLen nChar = ImplGetCharPos( rMEvt.GetPosPixel() );
        ImplSetCursorPos( nChar, FALSE );
        mbClickedInSelection = FALSE;
    }
    else if ( rMEvt.IsMiddle() && !mbReadOnly &&
              ( GetSettings().GetMouseSettings().GetMiddleButtonAction() == MOUSE_MIDDLE_PASTESELECTION ) )
    {
        ImplPaste( Window::GetSelection() );
    }
}

// -----------------------------------------------------------------------

void Edit::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        if ( mbClickedInSelection )
        {
            xub_StrLen nChar = ImplGetCharPos( rTEvt.GetMouseEvent().GetPosPixel() );
            ImplSetCursorPos( nChar, FALSE );
            mbClickedInSelection = FALSE;
        }
        else if ( rTEvt.GetMouseEvent().IsLeft() && GetSelection().Len() )
        {
            ImplCopy( Window::GetSelection() );
        }
    }
    else
    {
        if( !mbClickedInSelection )
        {
            xub_StrLen nChar = ImplGetCharPos( rTEvt.GetMouseEvent().GetPosPixel() );
            ImplSetCursorPos( nChar, TRUE );
        }
    }
}

// -----------------------------------------------------------------------

BOOL Edit::ImplHandleKeyEvent( const KeyEvent& rKEvt )
{
    BOOL        bDone = FALSE;
    USHORT      nCode = rKEvt.GetKeyCode().GetCode();
    KeyFuncType eFunc = rKEvt.GetKeyCode().GetFunction();

    mbInternModified = FALSE;

    if ( eFunc != KEYFUNC_DONTKNOW )
    {
        switch ( eFunc )
        {
            case KEYFUNC_CUT:
            {
                if ( !mbReadOnly && maSelection.Len() && !(GetStyle() & WB_PASSWORD) )
                {
                    Cut();
                    ImplModified();
                    bDone = TRUE;
                }
            }
            break;

            case KEYFUNC_COPY:
            {
                if ( !(GetStyle() & WB_PASSWORD) )
                {
                    Copy();
                    bDone = TRUE;
                }
            }
            break;

            case KEYFUNC_PASTE:
            {
                if ( !mbReadOnly )
                {
                    Paste();
                    bDone = TRUE;
                }
            }
            break;

            case KEYFUNC_UNDO:
            {
                if ( !mbReadOnly )
                {
                    Undo();
                    bDone = TRUE;
                }
            }
            break;

            default: // wird dann evtl. unten bearbeitet.
                eFunc = KEYFUNC_DONTKNOW;
        }
    }

    if ( eFunc == KEYFUNC_DONTKNOW )
    {
        switch ( nCode )
        {
            case KEY_LEFT:
            case KEY_RIGHT:
            case KEY_HOME:
            case KEY_END:
            {
                if ( !rKEvt.GetKeyCode().IsMod2() )
                {
                    uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();

                    Selection aSel( maSelection );
                    BOOL bWord = rKEvt.GetKeyCode().IsMod1();
                    // Range wird in ImplSetSelection geprueft...
                    if ( ( nCode == KEY_LEFT ) && aSel.Max() )
                    {
                        if ( bWord )
                        {
                            i18n::Boundary aBoundary = xBI->getWordBoundary( maText, aSel.Max(), GetSettings().GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES, sal_True );
                            if ( aBoundary.startPos == aSel.Max() )
                                aBoundary = xBI->previousWord( maText, aSel.Max(), GetSettings().GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES );
                            aSel.Max() = aBoundary.startPos;
                        }
                        else
                        {
                            sal_Int32 nCount = 1;
                            aSel.Max() = xBI->previousCharacters( maText, aSel.Max(), GetSettings().GetLocale(), i18n::CharacterIteratorMode::SKIPCHARACTER, nCount, nCount );
                        }
                    }
                    else if ( ( nCode == KEY_RIGHT ) && ( aSel.Max() < maText.Len() ) )
                    {
                        if ( bWord )
                           {
                            i18n::Boundary aBoundary = xBI->nextWord( maText, aSel.Max(), GetSettings().GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES );
                            aSel.Max() = aBoundary.startPos;
                        }
                        else
                        {
                            sal_Int32 nCount = 1;
                            aSel.Max() = xBI->nextCharacters( maText, aSel.Max(), GetSettings().GetLocale(), i18n::CharacterIteratorMode::SKIPCHARACTER, nCount, nCount );
                        }
                    }
                    else if ( nCode == KEY_HOME )
                        aSel.Max() = 0;
                    else if ( nCode == KEY_END )
                        aSel.Max() = 0xFFFF;

                    if ( !rKEvt.GetKeyCode().IsShift() )
                        aSel.Min() = aSel.Max();

                    if ( aSel != GetSelection() )
                    {
                        ImplSetSelection( aSel );
                        if ( aSel.Len() )
                            ImplCopy( Window::GetSelection() );
                    }

                    if ( (nCode == KEY_END) && maAutocompleteHdl.IsSet() && !rKEvt.GetKeyCode().GetModifier() )
                    {
                        if ( (maSelection.Min() == maSelection.Max()) && (maSelection.Min() == maText.Len()) )
                        {
                            meAutocompleteAction = AUTOCOMPLETE_KEYINPUT;
                            maAutocompleteHdl.Call( this );
                        }
                    }

                    bDone = TRUE;
                }
            }
            break;

            case KEY_BACKSPACE:
            case KEY_DELETE:
            {
                if ( !mbReadOnly && !rKEvt.GetKeyCode().IsMod2() )
                {
                    BYTE nDel = (nCode == KEY_DELETE) ? EDIT_DEL_RIGHT : EDIT_DEL_LEFT;
                    BYTE nMode = rKEvt.GetKeyCode().IsMod1() ? EDIT_DELMODE_RESTOFWORD : EDIT_DELMODE_SIMPLE;
                    if ( (nMode == EDIT_DELMODE_RESTOFWORD) && rKEvt.GetKeyCode().IsShift() )
                        nMode = EDIT_DELMODE_RESTOFCONTENT;
                    xub_StrLen nOldLen = maText.Len();
                    ImplDelete( maSelection, nDel, nMode );
                    if ( maText.Len() != nOldLen )
                        ImplModified();
                    bDone = TRUE;
                }
            }
            break;

            case KEY_INSERT:
            {
                if ( !mpIMEInfos && !mbReadOnly && !rKEvt.GetKeyCode().IsMod2() )
                {
                    SetInsertMode( !mbInsertMode );
                    bDone = TRUE;
                }
            }
            break;

            case KEY_TAB:
            {
                if ( !mbReadOnly && maAutocompleteHdl.IsSet() &&
                     maSelection.Min() && (maSelection.Min() == maText.Len()) &&
                     !rKEvt.GetKeyCode().IsMod1() && !rKEvt.GetKeyCode().IsMod2() )
                {
                    // Kein Autocomplete wenn alles Selektiert oder Edit leer, weil dann
                    // keine vernuenftige Tab-Steuerung!
                    if ( rKEvt.GetKeyCode().IsShift() )
                        meAutocompleteAction = AUTOCOMPLETE_TABBACKWARD;
                    else
                        meAutocompleteAction = AUTOCOMPLETE_TABFORWARD;

                    maAutocompleteHdl.Call( this );

                    // Wurde nichts veraendert, dann TAB fuer DialogControl
                    if ( GetSelection().Len() )
                        bDone = TRUE;
                }
            }
            break;

            default:
            {
                if ( IsCharInput( rKEvt ) )
                {
                    bDone = TRUE;   // Auch bei ReadOnly die Zeichen schlucken.
                    if ( !mbReadOnly )
                    {
                        ImplInsertText( rKEvt.GetCharCode() );
                        if ( maAutocompleteHdl.IsSet() )
                        {
                            if ( (maSelection.Min() == maSelection.Max()) && (maSelection.Min() == maText.Len()) )
                            {
                                meAutocompleteAction = AUTOCOMPLETE_KEYINPUT;
                                maAutocompleteHdl.Call( this );
                            }
                        }
                    }
                }
            }
        }
    }

    if ( !bDone && rKEvt.GetKeyCode().IsMod1() )
    {
        if ( nCode == KEY_A )
        {
            ImplSetSelection( Selection( 0, maText.Len() ) );
            bDone = TRUE;
        }
        else if ( rKEvt.GetKeyCode().IsShift() && (nCode == KEY_S) )
        {
            if ( pImplFncGetSpecialChars )
            {
                Selection aSaveSel = GetSelection();    // Falls jemand in Get/LoseFocus die Selektion verbiegt, z.B. URL-Zeile...
                XubString aChars = pImplFncGetSpecialChars( this, GetFont() );
                SetSelection( aSaveSel );
                if ( aChars.Len() )
                {
                    ImplInsertText( aChars );
                    ImplModified();
                }
                bDone = TRUE;
            }
        }
    }

    if ( mbInternModified )
        ImplModified();

    return bDone;
}

// -----------------------------------------------------------------------

void Edit::KeyInput( const KeyEvent& rKEvt )
{
    if ( mpSubEdit || !ImplHandleKeyEvent( rKEvt ) )
        Control::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

void Edit::Paint( const Rectangle& )
{
    if ( !mpSubEdit )
        ImplRepaint();
}

// -----------------------------------------------------------------------

void Edit::Resize()
{
    if ( !mpSubEdit && IsReallyVisible() )
    {
        // Wegen vertikaler Zentrierung...
        mnXOffset = 0;
        ImplAlign();
        Invalidate();
        ImplShowCursor();
    }
}

// -----------------------------------------------------------------------

void Edit::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, ULONG nFlags )
{
    if ( GetSubEdit() )
    {
        GetSubEdit()->Draw( pDev, rPos, rSize, nFlags );
    }

    ImplInitSettings( TRUE, TRUE, TRUE );

    Point aPos = pDev->LogicToPixel( rPos );
    Size aSize = pDev->LogicToPixel( rSize );
    Font aFont = GetDrawPixelFont( pDev );
    OutDevType eOutDevType = pDev->GetOutDevType();

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    pDev->SetTextFillColor();

    // Border/Background
    pDev->SetLineColor();
    pDev->SetFillColor();
    BOOL bBorder = !(nFlags & WINDOW_DRAW_NOBORDER ) && (GetStyle() & WB_BORDER);
    BOOL bBackground = !(nFlags & WINDOW_DRAW_NOBACKGROUND) && IsControlBackground();
    if ( bBorder || bBackground )
    {
        Rectangle aRect( aPos, aSize );
        if ( bBorder )
        {
            DecorationView aDecoView( pDev );
            aRect = aDecoView.DrawFrame( aRect, FRAME_DRAW_DOUBLEIN );
        }
        if ( bBackground )
        {
            pDev->SetFillColor( GetControlBackground() );
            pDev->DrawRect( aRect );
        }
    }

    // Inhalt
    if ( ( nFlags & WINDOW_DRAW_MONO ) || ( eOutDevType == OUTDEV_PRINTER ) )
        pDev->SetTextColor( Color( COL_BLACK ) );
    else
    {
        if ( !(nFlags & WINDOW_DRAW_NODISABLE ) && !IsEnabled() )
        {
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
            pDev->SetTextColor( rStyleSettings.GetDisableColor() );
        }
        else
        {
            pDev->SetTextColor( GetTextColor() );
        }
    }

    XubString   aText = ImplGetText();
    long        nTextHeight = pDev->GetTextHeight();
    long        nTextWidth = pDev->GetTextWidth( aText );
    long        nOnePixel = GetDrawPixel( pDev, 1 );
    long        nOffX = 3*nOnePixel;
    long        nOffY = (aSize.Height() - nTextHeight) / 2;

    if ( GetStyle() & WB_CENTER )
    {
        aPos.X() += (aSize.Width() - nTextWidth) / 2;
        nOffX = 0;
    }
    else if ( GetStyle() & WB_RIGHT )
    {
        aPos.X() += aSize.Width() - nTextWidth;
        nOffX = -nOffX;
    }

    // Clipping?
    if ( (nOffY < 0) ||
         ((nOffY+nTextHeight) > aSize.Height()) ||
         ((nOffX+nTextWidth) > aSize.Width()) )
    {
        Rectangle aClip( aPos, aSize );
        if ( nTextHeight > aSize.Height() )
            aClip.Bottom() += nTextHeight-aSize.Height()+1;  // Damit HP-Drucker nicht 'weg-optimieren'
        pDev->IntersectClipRegion( aClip );
    }

    pDev->DrawText( Point( aPos.X() + nOffX, aPos.Y() + nOffY ), aText );
    pDev->Pop();
}

// -----------------------------------------------------------------------

void Edit::GetFocus()
{
    if ( mpSubEdit )
        mpSubEdit->ImplGrabFocus( GetGetFocusFlags() );
    else if ( !mbActivePopup )
    {
        maUndoText = maText;

        ULONG nSelOptions = GetSettings().GetStyleSettings().GetSelectionOptions();
        if ( !( GetStyle() & (WB_NOHIDESELECTION|WB_READONLY) )
                && ( GetGetFocusFlags() & (GETFOCUS_INIT|GETFOCUS_TAB|GETFOCUS_CURSOR|GETFOCUS_MNEMONIC) ) )
        {
            if ( nSelOptions & SELECTION_OPTION_SHOWFIRST )
            {
                maSelection.Min() = maText.Len();
                maSelection.Max() = 0;
            }
            else
            {
                maSelection.Min() = 0;
                maSelection.Max() = maText.Len();
            }
        }

        ImplShowCursor();

        if ( maSelection.Len() )
        {
            // Selektion malen
            if ( !HasPaintEvent() )
                ImplRepaint();
            else
                Invalidate();
        }

        SetInputContext( InputContext( GetFont(), !IsReadOnly() ? INPUTCONTEXT_TEXT|INPUTCONTEXT_EXTTEXTINPUT : 0 ) );
    }

    Control::GetFocus();
}

// -----------------------------------------------------------------------

void Edit::LoseFocus()
{
    if ( !mpSubEdit )
    {
        if ( !mbActivePopup && !( GetStyle() & WB_NOHIDESELECTION ) && maSelection.Len() )
            ImplRepaint();    // Selektion malen
    }

    Control::LoseFocus();
}

// -----------------------------------------------------------------------

void Edit::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        PopupMenu* pPopup = Edit::CreatePopupMenu();
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        if ( rStyleSettings.GetOptions() & STYLE_OPTION_HIDEDISABLED )
            pPopup->SetMenuFlags( MENU_FLAG_HIDEDISABLEDENTRIES );

        if ( !maSelection.Len() )
        {
            pPopup->EnableItem( SV_MENU_EDIT_CUT, FALSE );
            pPopup->EnableItem( SV_MENU_EDIT_COPY, FALSE );
            pPopup->EnableItem( SV_MENU_EDIT_DELETE, FALSE );
        }

        if ( IsReadOnly() )
        {
            pPopup->EnableItem( SV_MENU_EDIT_CUT, FALSE );
            pPopup->EnableItem( SV_MENU_EDIT_PASTE, FALSE );
            pPopup->EnableItem( SV_MENU_EDIT_DELETE, FALSE );
            pPopup->EnableItem( SV_MENU_EDIT_INSERTSYMBOL, FALSE );
        }
        else
        {
            // Paste nur, wenn Text im Clipboard
            BOOL bData = FALSE;
            uno::Reference< datatransfer::clipboard::XClipboard > xClipboard = GetClipboard();
            if ( xClipboard.is() )
            {
                const sal_uInt32 nRef = Application::ReleaseSolarMutex();
                uno::Reference< datatransfer::XTransferable > xDataObj = xClipboard->getContents();
                Application::AcquireSolarMutex( nRef );
                if ( xDataObj.is() )
                {
                    datatransfer::DataFlavor aFlavor;
                    SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aFlavor );
                    bData = xDataObj->isDataFlavorSupported( aFlavor );
                }
            }
            pPopup->EnableItem( SV_MENU_EDIT_PASTE, bData );
        }

        if ( maUndoText == maText )
            pPopup->EnableItem( SV_MENU_EDIT_UNDO, FALSE );
        if ( ( maSelection.Min() == 0 ) && ( maSelection.Max() == maText.Len() ) )
            pPopup->EnableItem( SV_MENU_EDIT_SELECTALL, FALSE );
        if ( !pImplFncGetSpecialChars )
        {
            USHORT nPos = pPopup->GetItemPos( SV_MENU_EDIT_INSERTSYMBOL );
            pPopup->RemoveItem( nPos );
            pPopup->RemoveItem( nPos-1 );
        }

        mbActivePopup = TRUE;
        Selection aSaveSel = GetSelection();    // Falls jemand in Get/LoseFocus die Selektion verbiegt, z.B. URL-Zeile...
        Point aPos = rCEvt.GetMousePosPixel();
        if ( !rCEvt.IsMouseEvent() )
        {
            // !!! Irgendwann einmal Menu zentriert in der Selektion anzeigen !!!
            Size aSize = GetOutputSizePixel();
            aPos = Point( aSize.Width()/2, aSize.Height()/2 );
        }
        USHORT n = pPopup->Execute( this, aPos );
        Edit::DeletePopupMenu( pPopup );
        SetSelection( aSaveSel );
        switch ( n )
        {
            case SV_MENU_EDIT_UNDO:
                Undo();
                ImplModified();
                break;
            case SV_MENU_EDIT_CUT:
                Cut();
                ImplModified();
                break;
            case SV_MENU_EDIT_COPY:
                Copy();
                break;
            case SV_MENU_EDIT_PASTE:
                Paste();
                ImplModified();
                break;
            case SV_MENU_EDIT_DELETE:
                DeleteSelected();
                ImplModified();
                break;
            case SV_MENU_EDIT_SELECTALL:
                ImplSetSelection( Selection( 0, maText.Len() ) );
                break;
            case SV_MENU_EDIT_INSERTSYMBOL:
                {
                    XubString aChars = pImplFncGetSpecialChars( this, GetFont() );
                    SetSelection( aSaveSel );
                    if ( aChars.Len() )
                    {
                        ImplInsertText( aChars );
                        ImplModified();
                    }
                }
                break;
        }
        mbActivePopup = FALSE;
    }
    else if ( rCEvt.GetCommand() == COMMAND_VOICE )
    {
        const CommandVoiceData* pData = rCEvt.GetVoiceData();
        if ( pData->GetType() == VOICECOMMANDTYPE_DICTATION )
        {
            switch ( pData->GetCommand() )
            {
                case DICTATIONCOMMAND_UNKNOWN:
                {
                    ReplaceSelected( pData->GetText() );
                }
                break;
                case DICTATIONCOMMAND_LEFT:
                {
                    ImplHandleKeyEvent( KeyEvent( 0, KeyCode( KEY_LEFT, KEY_MOD1  ) ) );
                }
                break;
                case DICTATIONCOMMAND_RIGHT:
                {
                    ImplHandleKeyEvent( KeyEvent( 0, KeyCode( KEY_RIGHT, KEY_MOD1  ) ) );
                }
                break;
                case DICTATIONCOMMAND_UNDO:
                {
                    Undo();
                }
                break;
                case DICTATIONCOMMAND_DEL:
                {
                    ImplHandleKeyEvent( KeyEvent( 0, KeyCode( KEY_LEFT, KEY_MOD1|KEY_SHIFT  ) ) );
                    DeleteSelected();
                }
                break;
            }
        }
    }
    else if ( rCEvt.GetCommand() == COMMAND_STARTEXTTEXTINPUT )
    {
        DeleteSelected();
        delete mpIMEInfos;
        xub_StrLen nPos = (xub_StrLen)maSelection.Max();
        mpIMEInfos = new Impl_IMEInfos( nPos, maText.Copy( nPos ) );
        mpIMEInfos->bWasCursorOverwrite = !IsInsertMode();
    }
    else if ( rCEvt.GetCommand() == COMMAND_ENDEXTTEXTINPUT )
    {
        BOOL bInsertMode = !mpIMEInfos->bWasCursorOverwrite;
        delete mpIMEInfos;
        mpIMEInfos = NULL;
        // Font wieder ohne Attribute einstellen, wird jetzt im Repaint nicht
        // mehr neu initialisiert
        ImplInitSettings( TRUE, FALSE, FALSE );

        SetInsertMode( bInsertMode );

        ImplModified();
    }
    else if ( rCEvt.GetCommand() == COMMAND_EXTTEXTINPUT )
    {
        const CommandExtTextInputData* pData = rCEvt.GetExtTextInputData();

        maText.Erase( mpIMEInfos->nPos, mpIMEInfos->nLen );
        maText.Insert( pData->GetText(), mpIMEInfos->nPos );
        if ( mpIMEInfos->bWasCursorOverwrite )
        {
            USHORT nOldIMETextLen = mpIMEInfos->nLen;
            USHORT nNewIMETextLen = pData->GetText().Len();
            if ( ( nOldIMETextLen > nNewIMETextLen ) &&
                 ( nNewIMETextLen < mpIMEInfos->aOldTextAfterStartPos.Len() ) )
            {
                // restore old characters
                USHORT nRestore = nOldIMETextLen - nNewIMETextLen;
                maText.Insert( mpIMEInfos->aOldTextAfterStartPos.Copy( nNewIMETextLen, nRestore ), mpIMEInfos->nPos + nNewIMETextLen );
            }
            else if ( ( nOldIMETextLen < nNewIMETextLen ) &&
                      ( nOldIMETextLen < mpIMEInfos->aOldTextAfterStartPos.Len() ) )
            {
                // overwrite
                USHORT nOverwrite = nNewIMETextLen - nOldIMETextLen;
                if ( ( nOldIMETextLen + nOverwrite ) > mpIMEInfos->aOldTextAfterStartPos.Len() )
                    nOverwrite = mpIMEInfos->aOldTextAfterStartPos.Len() - nOldIMETextLen;
                maText.Erase( mpIMEInfos->nPos + nNewIMETextLen, nOverwrite );
            }
        }


        if ( pData->GetTextAttr() )
        {
            mpIMEInfos->CopyAttribs( pData->GetTextAttr(), pData->GetText().Len() );
            mpIMEInfos->bCursor = pData->IsCursorVisible();
        }
        else
        {
            mpIMEInfos->DestroyAttribs();
        }

        Invalidate();   // Erstmal einfach zum Testen
        xub_StrLen nCursorPos = mpIMEInfos->nPos + pData->GetCursorPos();
        SetSelection( Selection( nCursorPos, nCursorPos ) );
        SetInsertMode( !pData->IsCursorOverwrite() );

        if ( pData->IsCursorVisible() )
            GetCursor()->Show();
        else
            GetCursor()->Hide();
    }
    else if ( rCEvt.GetCommand() == COMMAND_CURSORPOS )
    {
        if ( mpIMEInfos )
        {
            xub_StrLen nCursorPos = (USHORT)GetSelection().Max();
            SetCursorRect( NULL, GetTextWidth(
                maText, nCursorPos, mpIMEInfos->nPos+mpIMEInfos->nLen-nCursorPos ) );
        }
        else
        {
            SetCursorRect();
        }
    }
    else
        Control::Command( rCEvt );
}

// -----------------------------------------------------------------------

void Edit::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_INITSHOW )
    {
        if ( !mpSubEdit )
        {
            mnXOffset = 0;  // Falls vorher GrabFocus, als Groesse noch falsch.
            ImplAlign();
            if ( !mpSubEdit )
                ImplShowCursor( FALSE );
        }
    }
    else if ( nType == STATE_CHANGE_ENABLE )
    {
        if ( !mpSubEdit )
        {
            // Es aendert sich nur die Textfarbe...
            ImplRepaint( 0, 0xFFFF );
        }
    }
    else if ( nType == STATE_CHANGE_STYLE )
    {
        WinBits nStyle = ImplInitStyle( GetStyle() );
        SetStyle( nStyle );

        USHORT nOldAlign = mnAlign;
        mnAlign = EDIT_ALIGN_LEFT;
        if ( nStyle & WB_RIGHT )
            mnAlign = EDIT_ALIGN_RIGHT;
        else if ( nStyle & WB_CENTER )
            mnAlign = EDIT_ALIGN_CENTER;
        if ( maText.Len() && ( mnAlign != nOldAlign ) )
        {
            ImplAlign();
            Invalidate();
        }

    }
    else if ( nType == STATE_CHANGE_ZOOM )
    {
        if ( !mpSubEdit )
        {
            ImplInitSettings( TRUE, FALSE, FALSE );
            ImplShowCursor( TRUE );
            Invalidate();
        }
    }
    else if ( nType == STATE_CHANGE_CONTROLFONT )
    {
        if ( !mpSubEdit )
        {
            ImplInitSettings( TRUE, FALSE, FALSE );
            ImplShowCursor();
            Invalidate();
        }
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        if ( !mpSubEdit )
        {
            ImplInitSettings( FALSE, TRUE, FALSE );
            Invalidate();
        }
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        if ( !mpSubEdit )
        {
            ImplInitSettings( FALSE, FALSE, TRUE );
            Invalidate();
        }
    }

    Control::StateChanged( nType );
}

// -----------------------------------------------------------------------

void Edit::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        if ( !mpSubEdit )
        {
            ImplInitSettings( TRUE, TRUE, TRUE );
            ImplShowCursor( TRUE );
            Invalidate();
        }
    }

    Control::DataChanged( rDCEvt );
}

// -----------------------------------------------------------------------

void Edit::ImplShowDDCursor()
{
    if ( !mpDDInfo->bVisCursor )
    {
        long nTextWidth = GetTextWidth( maText, 0, mpDDInfo->nDropPos );
        long nTextHeight = GetTextHeight();
        Rectangle aCursorRect( Point( nTextWidth + mnXOffset, (GetOutputSize().Height()-nTextHeight)/2 ), Size( 2, nTextHeight ) );
        mpDDInfo->aCursor.SetWindow( this );
        mpDDInfo->aCursor.SetPos( aCursorRect.TopLeft() );
        mpDDInfo->aCursor.SetSize( aCursorRect.GetSize() );
        mpDDInfo->aCursor.Show();
        mpDDInfo->bVisCursor = TRUE;
    }
}

// -----------------------------------------------------------------------

void Edit::ImplHideDDCursor()
{
    if ( mpDDInfo && mpDDInfo->bVisCursor )
    {
        mpDDInfo->aCursor.Hide();
        mpDDInfo->bVisCursor = FALSE;
    }
}

// -----------------------------------------------------------------------

void Edit::Modify()
{
        if ( mbIsSubEdit )
    {
        ((Edit*)GetParent())->Modify();
    }
    else
    {
        if ( mpUpdateDataTimer )
            mpUpdateDataTimer->Start();

        maModifyHdl.Call( this );
    }
}

// -----------------------------------------------------------------------

void Edit::UpdateData()
{
    maUpdateDataHdl.Call( this );
}

// -----------------------------------------------------------------------

IMPL_LINK( Edit, ImplUpdateDataHdl, Timer*, EMPTYARG )
{
    UpdateData();
    return 0;
}

// -----------------------------------------------------------------------

void Edit::EnableUpdateData( ULONG nTimeout )
{
    if ( !nTimeout )
        DisableUpdateData();
    else
    {
        if ( !mpUpdateDataTimer )
        {
            mpUpdateDataTimer = new Timer;
            mpUpdateDataTimer->SetTimeoutHdl( LINK( this, Edit, ImplUpdateDataHdl ) );
        }

        mpUpdateDataTimer->SetTimeout( nTimeout );
    }
}

// -----------------------------------------------------------------------

void Edit::SetEchoChar( xub_Unicode c )
{
    mcEchoChar = c;
    if ( mpSubEdit )
        mpSubEdit->SetEchoChar( c );
}

// -----------------------------------------------------------------------

void Edit::SetReadOnly( BOOL bReadOnly )
{
    if ( mbReadOnly != bReadOnly )
    {
        mbReadOnly = bReadOnly;
        if ( mpSubEdit )
            mpSubEdit->SetReadOnly( bReadOnly );

        StateChanged( STATE_CHANGE_READONLY );
    }
}

// -----------------------------------------------------------------------

void Edit::SetAutocompleteHdl( const Link& rHdl )
{
    maAutocompleteHdl = rHdl;
    if ( mpSubEdit )
        mpSubEdit->SetAutocompleteHdl( rHdl );
}

// -----------------------------------------------------------------------

void Edit::SetInsertMode( BOOL bInsert )
{
    if ( bInsert != mbInsertMode )
    {
        mbInsertMode = bInsert;
        if ( mpSubEdit )
            mpSubEdit->SetInsertMode( bInsert );
        else
            ImplShowCursor();
    }
}

// -----------------------------------------------------------------------

BOOL Edit::IsInsertMode() const
{
    if ( mpSubEdit )
        return mpSubEdit->IsInsertMode();
    else
        return mbInsertMode;
}

// -----------------------------------------------------------------------

void Edit::SetMaxTextLen( xub_StrLen nMaxLen )
{
    mnMaxTextLen = nMaxLen ? nMaxLen : EDIT_NOLIMIT;

    if ( mpSubEdit )
        mpSubEdit->SetMaxTextLen( nMaxLen );
    else
    {
        if ( maText.Len() > nMaxLen )
            ImplDelete( Selection( nMaxLen, maText.Len() ), EDIT_DEL_RIGHT, EDIT_DELMODE_SIMPLE );
    }
}

// -----------------------------------------------------------------------

void Edit::SetSelection( const Selection& rSelection )
{
    // Wenn von aussen z.B. im MouseButtonDown die Selektion geaendert wird,
    // soll nicht gleich ein Tracking() zuschlagen und die Selektion aendern.
    if ( IsTracking() )
        EndTracking();
    else if ( mpSubEdit && mpSubEdit->IsTracking() )
        mpSubEdit->EndTracking();

    ImplSetSelection( rSelection );
}

// -----------------------------------------------------------------------

void Edit::ImplSetSelection( const Selection& rSelection, BOOL bPaint )
{
    if ( mpSubEdit )
        mpSubEdit->ImplSetSelection( rSelection );
    else
    {
        if ( rSelection != maSelection )
        {
            Selection aOld( maSelection );
            Selection aNew( rSelection );

            if ( aNew.Min() > maText.Len() )
                aNew.Min() = maText.Len();
            if ( aNew.Max() > maText.Len() )
                aNew.Max() = maText.Len();
            if ( aNew.Min() < 0 )
                aNew.Min() = 0;
            if ( aNew.Max() < 0 )
                aNew.Max() = 0;

            if ( aNew != maSelection )
            {
                maSelection = aNew;

                if ( bPaint && ( aOld.Len() || aNew.Len() ) )
                {
                    aOld.Justify();
                    aNew.Justify();
                    xub_StrLen nStart = (xub_StrLen)Min( aOld.Min(), aNew.Min() );
                    xub_StrLen nEnd = (xub_StrLen)Max( aOld.Max(), aNew.Max() );
                    ImplRepaint( nStart, nEnd );
                }
                ImplShowCursor();
            }
        }
    }
}

// -----------------------------------------------------------------------

const Selection& Edit::GetSelection() const
{
    if ( mpSubEdit )
        return mpSubEdit->GetSelection();
    else
        return maSelection;
}

// -----------------------------------------------------------------------

void Edit::ReplaceSelected( const XubString& rStr )
{
    if ( mpSubEdit )
        mpSubEdit->ReplaceSelected( rStr );
    else
        ImplInsertText( rStr );
}

// -----------------------------------------------------------------------

void Edit::DeleteSelected()
{
    if ( mpSubEdit )
        mpSubEdit->DeleteSelected();
    else
    {
        if ( maSelection.Len() )
            ImplDelete( maSelection, EDIT_DEL_RIGHT, EDIT_DELMODE_SIMPLE );
    }
}

// -----------------------------------------------------------------------

XubString Edit::GetSelected() const
{
    if ( mpSubEdit )
        return mpSubEdit->GetSelected();
    else
    {
        Selection aSelection( maSelection );
        aSelection.Justify();
        return maText.Copy( (xub_StrLen)aSelection.Min(), (xub_StrLen)aSelection.Len() );
    }
}

// -----------------------------------------------------------------------

void Edit::Cut()
{
    if ( !(GetStyle() & WB_PASSWORD ) )
    {
        Copy();
        ReplaceSelected( ImplGetSVEmptyStr() );
    }
}

// -----------------------------------------------------------------------

void Edit::Copy()
{
    if ( !(GetStyle() & WB_PASSWORD ) )
    {
        ImplCopy( GetClipboard() );
    }
}

// -----------------------------------------------------------------------

void Edit::Paste()
{
    ImplPaste( GetClipboard() );
}

// -----------------------------------------------------------------------

void Edit::Undo()
{
    if ( mpSubEdit )
        mpSubEdit->Undo();
    else
    {
        XubString aText( maText );
        ImplDelete( Selection( 0, aText.Len() ), EDIT_DEL_RIGHT, EDIT_DELMODE_SIMPLE );
        ImplInsertText( maUndoText );
        ImplSetSelection( Selection( 0, maUndoText.Len() ) );
        maUndoText = aText;
    }
}

// -----------------------------------------------------------------------

void Edit::SetText( const XubString& rStr )
{
    if ( mpSubEdit )
        mpSubEdit->SetText( rStr );     // Nicht direkt ImplSetText, falls SetText ueberladen
    else
    {
        Selection aNewSel( 0, 0 );  // Damit nicht gescrollt wird
        ImplSetText( rStr, &aNewSel );
    }
}

// -----------------------------------------------------------------------

void Edit::SetText( const XubString& rStr, const Selection& rSelection )
{
    if ( mpSubEdit )
        mpSubEdit->SetText( rStr, rSelection );
    else
        ImplSetText( rStr, &rSelection );
}

// -----------------------------------------------------------------------

XubString Edit::GetText() const
{
    if ( mpSubEdit )
        return mpSubEdit->GetText();
    else
        return maText;
}

// -----------------------------------------------------------------------

void Edit::SetModifyFlag()
{
    if ( mpSubEdit )
        mpSubEdit->mbModified = TRUE;
    else
        mbModified = TRUE;
}

// -----------------------------------------------------------------------

void Edit::ClearModifyFlag()
{
    if ( mpSubEdit )
        mpSubEdit->mbModified = FALSE;
    else
        mbModified = FALSE;
}

// -----------------------------------------------------------------------

void Edit::SetSubEdit( Edit* pEdit )
{
    mpSubEdit = pEdit;
    if ( mpSubEdit )
    {
        SetPointer( POINTER_ARROW );    // Nur das SubEdit hat den BEAM...
        mpSubEdit->mbIsSubEdit = TRUE;
    }
}

// -----------------------------------------------------------------------

Size Edit::CalcMinimumSize() const
{
    Size aSz( GetTextWidth( GetText() ), GetTextHeight() );
    aSz = CalcWindowSize( aSz );
    return aSz;
}

// -----------------------------------------------------------------------

Size Edit::CalcSize( xub_StrLen nChars ) const
{
    // Breite fuer n Zeichen, unabhaengig vom Inhalt.
    // Funktioniert nur bei FixedFont richtig, sonst Mittelwert.
    Size aSz( GetTextWidth( XubString( 'x' ) ), GetTextHeight() );
    aSz.Width() *= nChars;
    aSz = CalcWindowSize( aSz );
    return aSz;
}

// -----------------------------------------------------------------------

xub_StrLen Edit::GetMaxVisChars() const
{
    const Window* pW = mpSubEdit ? mpSubEdit : this;
    long nOutWidth = pW->GetOutputSizePixel().Width();
    long nCharWidth = GetTextWidth( XubString( 'x' ) );
    return nCharWidth ? (xub_StrLen)(nOutWidth/nCharWidth) : 0;
}

// -----------------------------------------------------------------------

void Edit::SetGetSpecialCharsFunction( FncGetSpecialChars fn )
{
    pImplFncGetSpecialChars = fn;
}

// -----------------------------------------------------------------------

FncGetSpecialChars Edit::GetGetSpecialCharsFunction()
{
    return pImplFncGetSpecialChars;
}

// -----------------------------------------------------------------------

PopupMenu* Edit::CreatePopupMenu()
{
    PopupMenu* pPopup = new PopupMenu( ResId( SV_RESID_MENU_EDIT, ImplGetResMgr() ) );
    pPopup->SetAccelKey( SV_MENU_EDIT_UNDO, KeyCode( KEYFUNC_UNDO ) );
    pPopup->SetAccelKey( SV_MENU_EDIT_CUT, KeyCode( KEYFUNC_CUT ) );
    pPopup->SetAccelKey( SV_MENU_EDIT_COPY, KeyCode( KEYFUNC_COPY ) );
    pPopup->SetAccelKey( SV_MENU_EDIT_PASTE, KeyCode( KEYFUNC_PASTE ) );
    pPopup->SetAccelKey( SV_MENU_EDIT_DELETE, KeyCode( KEYFUNC_DELETE ) );
    pPopup->SetAccelKey( SV_MENU_EDIT_SELECTALL, KeyCode( KEY_A, FALSE, TRUE, FALSE ) );
    pPopup->SetAccelKey( SV_MENU_EDIT_INSERTSYMBOL, KeyCode( KEY_S, TRUE, TRUE, FALSE ) );
    return pPopup;
}

// -----------------------------------------------------------------------

void Edit::DeletePopupMenu( PopupMenu* pMenu )
{
    delete pMenu;
}

// ::com::sun::star::datatransfer::dnd::XDragGestureListener
void Edit::dragGestureRecognized( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& rDGE ) throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aVclGuard( Application::GetSolarMutex() );

    if ( !IsTracking() && maSelection.Len() &&
         !(GetStyle() & WB_PASSWORD) && (!mpDDInfo || mpDDInfo->bStarterOfDD == FALSE) ) // Kein Mehrfach D&D
    {
        Selection aSel( maSelection );
        aSel.Justify();

        // Nur wenn Maus in der Selektion...
        Point aMousePos( rDGE.DragOriginX, rDGE.DragOriginY );
        xub_StrLen nChar = ImplGetCharPos( aMousePos );
        if ( (nChar >= aSel.Min()) && (nChar < aSel.Max()) )
        {
            if ( !mpDDInfo )
                mpDDInfo = new DDInfo;

            mpDDInfo->bStarterOfDD = TRUE;
            mpDDInfo->aDndStartSel = aSel;

            if ( GetCursor() )
                GetCursor()->Hide();

            if ( IsTracking() )
                EndTracking();  // Vor D&D Tracking ausschalten

            TextDataObject* pDataObj = new TextDataObject( GetSelected() );
            rDGE.DragSource->startDrag( rDGE, datatransfer::dnd::DNDConstants::ACTION_COPY_OR_MOVE, 0 /*cursor*/, 0 /*image*/, pDataObj, mxDnDListener );
        }
    }
}

// ::com::sun::star::datatransfer::dnd::XDragSourceListener
void Edit::dragDropEnd( const ::com::sun::star::datatransfer::dnd::DragSourceDropEvent& rDSDE ) throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aVclGuard( Application::GetSolarMutex() );

    if ( rDSDE.DropSuccess && ( rDSDE.DropAction == datatransfer::dnd::DNDConstants::ACTION_MOVE ) )
    {
        Selection aSel( mpDDInfo->aDndStartSel );
        if ( mpDDInfo->bDroppedInMe )
        {
            if ( aSel.Max() > mpDDInfo->nDropPos )
            {
                long nLen = aSel.Len();
                aSel.Min() += nLen;
                aSel.Max() += nLen;
            }
        }
        ImplDelete( aSel, EDIT_DEL_RIGHT, EDIT_DELMODE_SIMPLE );
        ImplModified();
    }

    ImplHideDDCursor();
    delete mpDDInfo;
    mpDDInfo = NULL;
}

// ::com::sun::star::datatransfer::dnd::XDropTargetListener
void Edit::drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& rDTDE ) throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aVclGuard( Application::GetSolarMutex() );

    BOOL bChanges = FALSE;
    if ( !mbReadOnly && mpDDInfo )
    {
        ImplHideDDCursor();
        Point aMousePos( rDTDE.LocationX, rDTDE.LocationY );

        Selection aSel( maSelection );
        aSel.Justify();

        if ( aSel.Len() && !mpDDInfo->bStarterOfDD )
            ImplDelete( aSel, EDIT_DEL_RIGHT, EDIT_DELMODE_SIMPLE );

        mpDDInfo->bDroppedInMe = TRUE;

        aSel.Min() = mpDDInfo->nDropPos;
        aSel.Max() = mpDDInfo->nDropPos;
        ImplSetSelection( aSel );

        uno::Reference< datatransfer::XTransferable > xDataObj = rDTDE.Transferable;
        if ( xDataObj.is() )
        {
            datatransfer::DataFlavor aFlavor;
            SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aFlavor );
            if ( xDataObj->isDataFlavorSupported( aFlavor ) )
            {
                uno::Any aData = xDataObj->getTransferData( aFlavor );
                ::rtl::OUString aText;
                aData >>= aText;
                ImplInsertText( aText );
                bChanges = TRUE;
                ImplModified();
            }
        }

        if ( !mpDDInfo->bStarterOfDD )
        {
            delete mpDDInfo;
            mpDDInfo = NULL;
        }
    }

    rDTDE.Context->dropComplete( bChanges );
}

void Edit::dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& rDTDEE ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( !mpDDInfo )
    {
        mpDDInfo = new DDInfo;
    }
    sal_Bool bTextContent = mbReadOnly ? sal_False : sal_True;   // quiery from rDTDEE.SupportedDataFlavors()
//    if ( bTextContent )
//        rDTDEE.Context->acceptDrop(datatransfer::dnd::DNDConstants::ACTION_COPY_OR_MOVE);
//    else
//        rDTDEE.Context->rejectDrop();
}

void Edit::dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& dte ) throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aVclGuard( Application::GetSolarMutex() );

    ImplHideDDCursor();
}

void Edit::dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& rDTDE ) throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aVclGuard( Application::GetSolarMutex() );

    BOOL bDrop = FALSE;

    Point aMousePos( rDTDE.LocationX, rDTDE.LocationY );

    xub_StrLen nPrevDropPos = mpDDInfo->nDropPos;
    mpDDInfo->nDropPos = ImplGetCharPos( aMousePos );

    Size aOutSize = GetOutputSizePixel();
    if ( ( aMousePos.X() < 0 ) || ( aMousePos.X() > aOutSize.Width() ) )
    {
        // Scrollen ?
    }

    Selection aSel( maSelection );
    aSel.Justify();

    // Nicht in Selektion droppen:
    if ( aSel.IsInside( mpDDInfo->nDropPos ) )
    {
        ImplHideDDCursor();
        rDTDE.Context->rejectDrag();
    }
    else
    {
        // Alten Cursor wegzeichnen...
        if ( !mpDDInfo->bVisCursor || ( nPrevDropPos != mpDDInfo->nDropPos ) )
        {
            ImplHideDDCursor();
            ImplShowDDCursor();
        }
        rDTDE.Context->acceptDrag( rDTDE.DropAction );
    }
}

ImplSubEdit::ImplSubEdit( Edit* pParent, WinBits nStyle ) :
    Edit( pParent, nStyle )
{
    pParent->SetSubEdit( this );
}

// -----------------------------------------------------------------------

void ImplSubEdit::Modify()
{
    GetParent()->Modify();
}
