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

#ifndef INCLUDED_VCL_EDIT_HXX
#define INCLUDED_VCL_EDIT_HXX

#include <boost/signals2/signal.hpp>
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/menu.hxx>
#include <vcl/dndhelp.hxx>
#include <vcl/vclptr.hxx>
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


// - Edit-Types -
#define EDIT_NOLIMIT                SAL_MAX_INT32
#define EDIT_UPDATEDATA_TIMEOUT     350

typedef OUString (*FncGetSpecialChars)( vcl::Window* pWin, const vcl::Font& rFont );

class VCL_DLLPUBLIC TextFilter
{
private:
    OUString sForbiddenChars;
public:
    void SetForbiddenChars(const OUString& rSet) { sForbiddenChars = rSet; }
    const OUString& GetForbiddenChars() { return sForbiddenChars; }

    virtual OUString filter(const OUString &rText);

    TextFilter(const OUString &rForbiddenChars = OUString(" "));
    virtual ~TextFilter();
};

enum AutocompleteAction{ AUTOCOMPLETE_KEYINPUT, AUTOCOMPLETE_TABFORWARD, AUTOCOMPLETE_TABBACKWARD };

// - Edit -
class VCL_DLLPUBLIC Edit : public Control, public vcl::unohelper::DragAndDropClient
{
private:
    VclPtr<Edit>        mpSubEdit;
    Timer*              mpUpdateDataTimer;
    TextFilter*         mpFilterText;
    DDInfo*             mpDDInfo;
    Impl_IMEInfos*      mpIMEInfos;
    OUStringBuffer      maText;
    OUString            maPlaceholderText;
    OUString            maSaveValue;
    OUString            maUndoText;
    long                mnXOffset;
    Selection           maSelection;
    sal_uInt16          mnAlign;
    sal_Int32           mnMaxTextLen;
    sal_Int32           mnWidthInChars;
    sal_Int32           mnMaxWidthChars;
    AutocompleteAction  meAutocompleteAction;
    sal_Unicode         mcEchoChar;
    bool                mbModified:1,
                        mbInternModified:1,
                        mbReadOnly:1,
                        mbInsertMode:1,
                        mbClickedInSelection:1,
                        mbIsSubEdit:1,
                        mbInMBDown:1,
                        mbActivePopup:1;
    Link<>              maModifyHdl;
    Link<>              maUpdateDataHdl;

    css::uno::Reference < css::i18n::XExtendedInputSequenceChecker > mxISC;

    DECL_DLLPRIVATE_LINK_TYPED( ImplUpdateDataHdl, Timer*, void );

    SAL_DLLPRIVATE bool        ImplTruncateToMaxLen( OUString&, sal_Int32 nSelectionLen ) const;
    SAL_DLLPRIVATE void        ImplInitEditData();
    SAL_DLLPRIVATE void        ImplModified();
    SAL_DLLPRIVATE OUString    ImplGetText() const;
    SAL_DLLPRIVATE void        ImplRepaint(vcl::RenderContext& rRenderContext, bool bLayout = false);
    SAL_DLLPRIVATE void        ImplInvalidateOrRepaint();
    SAL_DLLPRIVATE void        ImplDelete( const Selection& rSelection, sal_uInt8 nDirection, sal_uInt8 nMode );
    SAL_DLLPRIVATE void        ImplSetText( const OUString& rStr, const Selection* pNewSelection = 0 );
    SAL_DLLPRIVATE void        ImplInsertText( const OUString& rStr, const Selection* pNewSelection = 0, bool bIsUserInput = false );
    SAL_DLLPRIVATE OUString    ImplGetValidString( const OUString& rString ) const;
    SAL_DLLPRIVATE void        ImplClearBackground(vcl::RenderContext& rRenderContext, long nXStart, long nXEnd);
    SAL_DLLPRIVATE void        ImplPaintBorder(vcl::RenderContext& rRenderContext, long nXStart, long nXEnd);
    SAL_DLLPRIVATE void        ImplShowCursor( bool bOnlyIfVisible = true );
    SAL_DLLPRIVATE void        ImplAlign();
    SAL_DLLPRIVATE void        ImplAlignAndPaint();
    SAL_DLLPRIVATE sal_Int32   ImplGetCharPos( const Point& rWindowPos ) const;
    SAL_DLLPRIVATE void        ImplSetCursorPos( sal_Int32 nChar, bool bSelect );
    SAL_DLLPRIVATE void        ImplShowDDCursor();
    SAL_DLLPRIVATE void        ImplHideDDCursor();
    SAL_DLLPRIVATE bool        ImplHandleKeyEvent( const KeyEvent& rKEvt );
    SAL_DLLPRIVATE void        ImplCopyToSelectionClipboard();
    SAL_DLLPRIVATE void        ImplCopy( ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >& rxClipboard );
    SAL_DLLPRIVATE void        ImplPaste( ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >& rxClipboard );
    SAL_DLLPRIVATE long        ImplGetTextYPosition() const;
    SAL_DLLPRIVATE ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XExtendedInputSequenceChecker > ImplGetInputSequenceChecker();
    SAL_DLLPRIVATE ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XBreakIterator > ImplGetBreakIterator() const;
    SAL_DLLPRIVATE void        filterText();

protected:
    using Control::ImplInitSettings;
    using Window::ImplInit;
    SAL_DLLPRIVATE void        ImplInit( vcl::Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE WinBits     ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void        ImplInitSettings( bool bFont, bool bForeground, bool bBackground );
    SAL_DLLPRIVATE void        ImplLoadRes( const ResId& rResId );
    SAL_DLLPRIVATE void        ImplSetSelection( const Selection& rSelection, bool bPaint = true );
    SAL_DLLPRIVATE int         ImplGetNativeControlType() const;
    SAL_DLLPRIVATE long        ImplGetExtraXOffset() const;
    SAL_DLLPRIVATE long        ImplGetExtraYOffset() const;
    static SAL_DLLPRIVATE void ImplInvalidateOutermostBorder( vcl::Window* pWin );

    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragSourceListener > mxDnDListener;

    // DragAndDropClient
    using vcl::unohelper::DragAndDropClient::dragEnter;
    using vcl::unohelper::DragAndDropClient::dragExit;
    using vcl::unohelper::DragAndDropClient::dragOver;
    virtual void        dragGestureRecognized( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& dge ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void        dragDropEnd( const ::com::sun::star::datatransfer::dnd::DragSourceDropEvent& dsde ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void        drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& dtde ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void        dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void        dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& dte ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void        dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    protected:
    virtual void FillLayoutData() const SAL_OVERRIDE;
                        Edit( WindowType nType );

public:
    // public because needed in button.cxx
    SAL_DLLPRIVATE bool        ImplUseNativeBorder( WinBits nStyle );

                        Edit( vcl::Window* pParent, WinBits nStyle = WB_BORDER );
                        Edit( vcl::Window* pParent, const ResId& rResId );
                        virtual ~Edit();
                        virtual void dispose() SAL_OVERRIDE;

    virtual void        MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void        MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void        KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
    virtual void        Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void        Resize() SAL_OVERRIDE;
    virtual void        Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags ) SAL_OVERRIDE;
    virtual void        GetFocus() SAL_OVERRIDE;
    virtual void        LoseFocus() SAL_OVERRIDE;
    virtual void        Tracking( const TrackingEvent& rTEvt ) SAL_OVERRIDE;
    virtual void        Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;
    virtual void        StateChanged( StateChangedType nType ) SAL_OVERRIDE;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
    virtual vcl::Window*     GetPreferredKeyInputWindow() SAL_OVERRIDE;

    virtual void        Modify();
    virtual void        UpdateData();

    static bool         IsCharInput( const KeyEvent& rKEvt );

    virtual void        SetModifyFlag();
    virtual void        ClearModifyFlag();
    virtual bool        IsModified() const { return mpSubEdit ? mpSubEdit->mbModified : mbModified; }

    virtual void        EnableUpdateData( sal_uLong nTimeout = EDIT_UPDATEDATA_TIMEOUT );
    virtual void        DisableUpdateData() { delete mpUpdateDataTimer; mpUpdateDataTimer = NULL; }

    void                SetEchoChar( sal_Unicode c );
    sal_Unicode         GetEchoChar() const { return mcEchoChar; }

    virtual void        SetReadOnly( bool bReadOnly = true );
    virtual bool        IsReadOnly() const { return mbReadOnly; }

    void                SetInsertMode( bool bInsert );
    bool                IsInsertMode() const;

    virtual void        SetMaxTextLen( sal_Int32 nMaxLen = EDIT_NOLIMIT );
    virtual sal_Int32   GetMaxTextLen() const { return mnMaxTextLen; }

    void                SetWidthInChars(sal_Int32 nWidthInChars);
    sal_Int32           GetWidthInChars() const { return mnWidthInChars; }

    void                setMaxWidthChars(sal_Int32 nWidth);

    virtual void        SetSelection( const Selection& rSelection );
    virtual const Selection&    GetSelection() const;

    virtual void        ReplaceSelected( const OUString& rStr );
    virtual void        DeleteSelected();
    virtual OUString    GetSelected() const;

    virtual void        Cut();
    virtual void        Copy();
    virtual void        Paste();
    void                Undo();

    virtual void        SetText( const OUString& rStr ) SAL_OVERRIDE;
    virtual void        SetText( const OUString& rStr, const Selection& rNewSelection );
    virtual OUString    GetText() const SAL_OVERRIDE;

    void                SetPlaceholderText( const OUString& rStr );
    OUString            GetPlaceholderText() const;

    void                SaveValue() { maSaveValue = GetText(); }
    const OUString&     GetSavedValue() const { return maSaveValue; }
    bool                IsValueChangedFromSaved() const { return maSaveValue != GetText(); }

    virtual void        SetModifyHdl( const Link<>& rLink ) { maModifyHdl = rLink; }
    virtual const Link<>& GetModifyHdl() const { return maModifyHdl; }
    virtual void        SetUpdateDataHdl( const Link<>& rLink ) { maUpdateDataHdl = rLink; }

    void                SetSubEdit( Edit* pEdit );
    Edit*               GetSubEdit() const { return mpSubEdit; }

    boost::signals2::signal< void ( Edit* ) > autocompleteSignal;
    AutocompleteAction  GetAutocompleteAction() const { return meAutocompleteAction; }

    virtual Size        CalcMinimumSize() const;
    virtual Size        CalcMinimumSizeForText(const OUString &rString) const;
    virtual Size        GetOptimalSize() const SAL_OVERRIDE;
    virtual Size        CalcSize(sal_Int32 nChars) const;
    sal_Int32           GetMaxVisChars() const;

    sal_Int32           GetCharPos( const Point& rWindowPos ) const;

    // shows a warning box saying "text too long, truncated"
    static void         ShowTruncationWarning( vcl::Window* pParent );

    static void                 SetGetSpecialCharsFunction( FncGetSpecialChars fn );
    static FncGetSpecialChars   GetGetSpecialCharsFunction();

    static PopupMenu*   CreatePopupMenu();
    static void         DeletePopupMenu( PopupMenu* pMenu );

    virtual OUString GetSurroundingText() const SAL_OVERRIDE;
    virtual Selection GetSurroundingTextSelection() const SAL_OVERRIDE;
    virtual bool set_property(const OString &rKey, const OString &rValue) SAL_OVERRIDE;

    void SetTextFilter(TextFilter* pFilter) { mpFilterText = pFilter; }

    // returns the minimum size a bordered Edit should have given the current
    // global style settings (needed by sc's inputwin.cxx)
    static Size GetMinimumEditSize();
};

#endif // INCLUDED_VCL_EDIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
