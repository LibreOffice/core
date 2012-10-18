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

#ifndef _SV_EDIT_HXX
#define _SV_EDIT_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/timer.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/menu.hxx>
#include <vcl/dndhelp.hxx>
#include <com/sun/star/uno/Reference.h>

namespace com {
namespace sun {
namespace star {
namespace i18n {
    class XBreakIterator;
    class XExtendedInputSequenceChecker;
}}}}

struct DDInfo;
struct Impl_IMEInfos;

// --------------
// - Edit-Types -
// --------------

#define EDIT_NOLIMIT                STRING_LEN
#define EDIT_UPDATEDATA_TIMEOUT     350

typedef XubString (*FncGetSpecialChars)( Window* pWin, const Font& rFont );

// --------
// - Edit -
// --------

enum AutocompleteAction{ AUTOCOMPLETE_KEYINPUT, AUTOCOMPLETE_TABFORWARD, AUTOCOMPLETE_TABBACKWARD };

class VCL_DLLPUBLIC Edit : public Control, public vcl::unohelper::DragAndDropClient
{
private:
    Edit*               mpSubEdit;
    Timer*              mpUpdateDataTimer;
    DDInfo*             mpDDInfo;
    Impl_IMEInfos*      mpIMEInfos;
    XubString           maText;
    XubString           maSaveValue;
    XubString           maUndoText;
    XubString           maRedoText;
    long                mnXOffset;
    Selection           maSelection;
    sal_uInt16          mnAlign;
    xub_StrLen          mnMaxTextLen;
    sal_Int32           mnMinWidthInChars;
    AutocompleteAction  meAutocompleteAction;
    xub_Unicode         mcEchoChar;
    sal_Bool                mbModified:1,
                        mbInternModified:1,
                        mbReadOnly:1,
                        mbInsertMode:1,
                        mbClickedInSelection:1,
                        mbIsSubEdit:1,
                        mbInMBDown:1,
                        mbActivePopup:1;
    Link                maModifyHdl;
    Link                maUpdateDataHdl;
    Link                maAutocompleteHdl;

    DECL_DLLPRIVATE_LINK(      ImplUpdateDataHdl, void* );

    SAL_DLLPRIVATE bool        ImplTruncateToMaxLen( rtl::OUString&, sal_uInt32 nSelectionLen ) const;
    SAL_DLLPRIVATE void        ImplInitEditData();
    SAL_DLLPRIVATE void        ImplModified();
    SAL_DLLPRIVATE XubString   ImplGetText() const;
    SAL_DLLPRIVATE void        ImplRepaint( xub_StrLen nStart = 0, xub_StrLen nEnd = STRING_LEN, bool bLayout = false );
    SAL_DLLPRIVATE void        ImplInvalidateOrRepaint( xub_StrLen nStart = 0, xub_StrLen nEnd = STRING_LEN );
    SAL_DLLPRIVATE void        ImplDelete( const Selection& rSelection, sal_uInt8 nDirection, sal_uInt8 nMode );
    SAL_DLLPRIVATE void        ImplSetText( const XubString& rStr, const Selection* pNewSelection = 0 );
    SAL_DLLPRIVATE void        ImplInsertText( const rtl::OUString& rStr, const Selection* pNewSelection = 0, sal_Bool bIsUserInput = sal_False );
    SAL_DLLPRIVATE String      ImplGetValidString( const String& rString ) const;
    SAL_DLLPRIVATE void        ImplClearBackground( long nXStart, long nXEnd );
    SAL_DLLPRIVATE void        ImplPaintBorder( long nXStart, long nXEnd );
    SAL_DLLPRIVATE void        ImplShowCursor( sal_Bool bOnlyIfVisible = sal_True );
    SAL_DLLPRIVATE void        ImplAlign();
    SAL_DLLPRIVATE void        ImplAlignAndPaint();
    SAL_DLLPRIVATE xub_StrLen  ImplGetCharPos( const Point& rWindowPos ) const;
    SAL_DLLPRIVATE void        ImplSetCursorPos( xub_StrLen nChar, sal_Bool bSelect );
    SAL_DLLPRIVATE void        ImplShowDDCursor();
    SAL_DLLPRIVATE void        ImplHideDDCursor();
    SAL_DLLPRIVATE sal_Bool        ImplHandleKeyEvent( const KeyEvent& rKEvt );
    SAL_DLLPRIVATE void        ImplCopyToSelectionClipboard();
    SAL_DLLPRIVATE void        ImplCopy( ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >& rxClipboard );
    SAL_DLLPRIVATE void        ImplPaste( ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >& rxClipboard );
    SAL_DLLPRIVATE long        ImplGetTextYPosition() const;
    SAL_DLLPRIVATE ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XExtendedInputSequenceChecker > ImplGetInputSequenceChecker() const;
    SAL_DLLPRIVATE ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XBreakIterator > ImplGetBreakIterator() const;

protected:
    using Control::ImplInitSettings;
    using Window::ImplInit;
    SAL_DLLPRIVATE void        ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE WinBits     ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void        ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
    SAL_DLLPRIVATE void        ImplLoadRes( const ResId& rResId );
    SAL_DLLPRIVATE void        ImplSetSelection( const Selection& rSelection, sal_Bool bPaint = sal_True );
    SAL_DLLPRIVATE int         ImplGetNativeControlType();
    SAL_DLLPRIVATE long        ImplGetExtraOffset() const;
    static SAL_DLLPRIVATE void ImplInvalidateOutermostBorder( Window* pWin );

    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragSourceListener > mxDnDListener;

    // DragAndDropClient
    using vcl::unohelper::DragAndDropClient::dragEnter;
    using vcl::unohelper::DragAndDropClient::dragExit;
    using vcl::unohelper::DragAndDropClient::dragOver;
    virtual void        dragGestureRecognized( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& dge ) throw (::com::sun::star::uno::RuntimeException);
    virtual void        dragDropEnd( const ::com::sun::star::datatransfer::dnd::DragSourceDropEvent& dsde ) throw (::com::sun::star::uno::RuntimeException);
    virtual void        drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& dtde ) throw (::com::sun::star::uno::RuntimeException);
    virtual void        dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) throw (::com::sun::star::uno::RuntimeException);
    virtual void        dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& dte ) throw (::com::sun::star::uno::RuntimeException);
    virtual void        dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde ) throw (::com::sun::star::uno::RuntimeException);

    protected:
    virtual void FillLayoutData() const;
                        Edit( WindowType nType );

public:
    // public because needed in button.cxx
    SAL_DLLPRIVATE bool        ImplUseNativeBorder( WinBits nStyle );

                        Edit( Window* pParent, WinBits nStyle = WB_BORDER );
                        Edit( Window* pParent, const ResId& rResId );
                        Edit( Window* pParent, const ResId& rResId, bool bDisableAccessibleLabeledByRelation );
                        virtual ~Edit();

    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        MouseButtonUp( const MouseEvent& rMEvt );
    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        Paint( const Rectangle& rRect );
    virtual void        Resize();
    virtual void        Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void        GetFocus();
    virtual void        LoseFocus();
    virtual void        Tracking( const TrackingEvent& rTEvt );
    virtual void        Command( const CommandEvent& rCEvt );
    virtual void        StateChanged( StateChangedType nType );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );
    virtual Window*     GetPreferredKeyInputWindow();

    virtual void        Modify();
    virtual void        UpdateData();

    static sal_Bool         IsCharInput( const KeyEvent& rKEvt );

    virtual void        SetModifyFlag();
    virtual void        ClearModifyFlag();
    virtual sal_Bool        IsModified() const { return mpSubEdit ? mpSubEdit->mbModified : mbModified; }

    virtual void        EnableUpdateData( sal_uLong nTimeout = EDIT_UPDATEDATA_TIMEOUT );
    virtual void        DisableUpdateData() { delete mpUpdateDataTimer; mpUpdateDataTimer = NULL; }
    virtual sal_uLong       IsUpdateDataEnabled() const;

    void                SetEchoChar( xub_Unicode c );
    xub_Unicode         GetEchoChar() const { return mcEchoChar; }

    virtual void        SetReadOnly( sal_Bool bReadOnly = sal_True );
    virtual sal_Bool        IsReadOnly() const { return mbReadOnly; }

    void                SetInsertMode( sal_Bool bInsert );
    sal_Bool                IsInsertMode() const;

    virtual void        SetMaxTextLen( xub_StrLen nMaxLen = EDIT_NOLIMIT );
    virtual xub_StrLen  GetMaxTextLen() const { return mnMaxTextLen; }

    void                SetMinWidthInChars(sal_Int32 nMinWidthInChars);
    sal_Int32           GetMinWidthInChars() const { return mnMinWidthInChars; }

    virtual void        SetSelection( const Selection& rSelection );
    virtual const Selection&    GetSelection() const;

    virtual void        ReplaceSelected( const XubString& rStr );
    virtual void        DeleteSelected();
    virtual XubString   GetSelected() const;

    virtual void        Cut();
    virtual void        Copy();
    virtual void        Paste();
    void                Undo();

    virtual void        SetText( const XubString& rStr );
    virtual void        SetText( const XubString& rStr, const Selection& rNewSelection );
    virtual XubString   GetText() const;

    void                SaveValue() { maSaveValue = GetText(); }
    const XubString&    GetSavedValue() const { return maSaveValue; }

    virtual void        SetModifyHdl( const Link& rLink ) { maModifyHdl = rLink; }
    virtual const Link& GetModifyHdl() const { return maModifyHdl; }
    virtual void        SetUpdateDataHdl( const Link& rLink ) { maUpdateDataHdl = rLink; }
    virtual const Link& GetUpdateDataHdl() const { return maUpdateDataHdl; }

    void                SetSubEdit( Edit* pEdit );
    Edit*               GetSubEdit() const { return mpSubEdit; }

    void                SetAutocompleteHdl( const Link& rHdl );
    const Link&         GetAutocompleteHdl() const { return maAutocompleteHdl; }
    AutocompleteAction  GetAutocompleteAction() const { return meAutocompleteAction; }

    virtual Size        CalcMinimumSize() const;
    virtual Size        CalcMinimumSizeForText(const rtl::OUString &rString) const;
    virtual Size        GetOptimalSize(WindowSizeType eType) const;
    virtual Size        CalcSize( sal_uInt16 nChars ) const;
    virtual xub_StrLen  GetMaxVisChars() const;

    xub_StrLen          GetCharPos( const Point& rWindowPos ) const;

    // shows a warning box saying "text too long, truncated"
    static void         ShowTruncationWarning( Window* pParent );

    static void                 SetGetSpecialCharsFunction( FncGetSpecialChars fn );
    static FncGetSpecialChars   GetGetSpecialCharsFunction();

    static PopupMenu*   CreatePopupMenu();
    static void         DeletePopupMenu( PopupMenu* pMenu );

    virtual rtl::OUString GetSurroundingText() const;
    virtual Selection GetSurroundingTextSelection() const;
    virtual void take_properties(Window &rOther);
    virtual bool set_property(const rtl::OString &rKey, const rtl::OString &rValue);

    // returns the minimum size a bordered Edit should have given the current
    // global style settings (needed by sc's inputwin.cxx)
    static Size GetMinimumEditSize();
};

inline sal_uLong Edit::IsUpdateDataEnabled() const
{
    if ( mpUpdateDataTimer )
        return mpUpdateDataTimer->GetTimeout();
    else
        return sal_False;
}

#endif  // _SV_EDIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
