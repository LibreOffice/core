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

#include <vcl/ctrl.hxx>

#include <functional>
#include <memory>

#include <o3tl/deleter.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>
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
namespace weld {
    class Widget;
}

class VclBuilder;
struct DDInfo;
struct Impl_IMEInfos;

#define EDIT_NOLIMIT                SAL_MAX_INT32
#define EDIT_UPDATEDATA_TIMEOUT     350

typedef OUString (*FncGetSpecialChars)( vcl::Window* pWin, const vcl::Font& rFont );

class VCL_DLLPUBLIC TextFilter
{
private:
    OUString sForbiddenChars;
public:
    void SetForbiddenChars(const OUString& rSet) { sForbiddenChars = rSet; }

    virtual OUString filter(const OUString &rText);

    TextFilter(const OUString &rForbiddenChars = OUString(" "));
    virtual ~TextFilter();
};

class Timer;

class VCL_DLLPUBLIC Edit : public Control, public vcl::unohelper::DragAndDropClient
{
private:
    VclPtr<Edit>        mpSubEdit;
    std::unique_ptr<Timer> mpUpdateDataTimer;
    TextFilter*         mpFilterText;
    std::unique_ptr<DDInfo, o3tl::default_delete<DDInfo>> mpDDInfo;
    std::unique_ptr<Impl_IMEInfos> mpIMEInfos;
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
    sal_Unicode         mcEchoChar;
    bool                mbModified:1,
                        mbSelectAllSingleClick:1,
                        mbInternModified:1,
                        mbReadOnly:1,
                        mbInsertMode:1,
                        mbClickedInSelection:1,
                        mbIsSubEdit:1,
                        mbActivePopup:1,
                        mbForceControlBackground:1,
                        mbPassword;
    Link<Edit&,void>    maModifyHdl;
    Link<Edit&,void>    maUpdateDataHdl;
    Link<Edit&,void>    maAutocompleteHdl;
    Link<Edit&,void>    maActivateHdl;
    std::unique_ptr<VclBuilder> mpUIBuilder;

    css::uno::Reference<css::i18n::XExtendedInputSequenceChecker> mxISC;

    DECL_DLLPRIVATE_LINK(ImplUpdateDataHdl, Timer*, void);

    SAL_DLLPRIVATE bool        ImplTruncateToMaxLen( OUString&, sal_Int32 nSelectionLen ) const;
    SAL_DLLPRIVATE void        ImplInitEditData();
    SAL_DLLPRIVATE void        ImplModified();
    SAL_DLLPRIVATE OUString    ImplGetText() const;
    SAL_DLLPRIVATE void        ImplRepaint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRectangle);
    SAL_DLLPRIVATE void        ImplInvalidateOrRepaint();
    SAL_DLLPRIVATE void        ImplDelete( const Selection& rSelection, sal_uInt8 nDirection, sal_uInt8 nMode );
    SAL_DLLPRIVATE void        ImplSetText( const OUString& rStr, const Selection* pNewSelection );
    SAL_DLLPRIVATE void        ImplInsertText( const OUString& rStr, const Selection* pNewSelection = nullptr, bool bIsUserInput = false );
    SAL_DLLPRIVATE static OUString ImplGetValidString( const OUString& rString );
    SAL_DLLPRIVATE void        ImplClearBackground(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRectangle, long nXStart, long nXEnd);
    SAL_DLLPRIVATE void        ImplPaintBorder(vcl::RenderContext const & rRenderContext);
    SAL_DLLPRIVATE void        ImplShowCursor( bool bOnlyIfVisible = true );
    SAL_DLLPRIVATE void        ImplAlign();
    SAL_DLLPRIVATE void        ImplAlignAndPaint();
    SAL_DLLPRIVATE sal_Int32   ImplGetCharPos( const Point& rWindowPos ) const;
    SAL_DLLPRIVATE void        ImplSetCursorPos( sal_Int32 nChar, bool bSelect );
    SAL_DLLPRIVATE void        ImplShowDDCursor();
    SAL_DLLPRIVATE void        ImplHideDDCursor();
    SAL_DLLPRIVATE bool        ImplHandleKeyEvent( const KeyEvent& rKEvt );
    SAL_DLLPRIVATE void        ImplCopyToSelectionClipboard();
    SAL_DLLPRIVATE void        ImplCopy(css::uno::Reference<css::datatransfer::clipboard::XClipboard> const & rxClipboard);
    SAL_DLLPRIVATE void        ImplPaste(css::uno::Reference<css::datatransfer::clipboard::XClipboard> const & rxClipboard);
    SAL_DLLPRIVATE long        ImplGetTextYPosition() const;
    SAL_DLLPRIVATE css::uno::Reference<css::i18n::XExtendedInputSequenceChecker > const & ImplGetInputSequenceChecker();
    SAL_DLLPRIVATE static css::uno::Reference<css::i18n::XBreakIterator > ImplGetBreakIterator();
    SAL_DLLPRIVATE void        filterText();

protected:
    using Control::ImplInitSettings;
    using Window::ImplInit;
    SAL_DLLPRIVATE void        ImplInit( vcl::Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE static WinBits ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void        ImplSetSelection( const Selection& rSelection, bool bPaint = true );
    SAL_DLLPRIVATE ControlType ImplGetNativeControlType() const;
    SAL_DLLPRIVATE long        ImplGetExtraXOffset() const;
    SAL_DLLPRIVATE long        ImplGetExtraYOffset() const;
    static SAL_DLLPRIVATE void ImplInvalidateOutermostBorder( vcl::Window* pWin );

    css::uno::Reference<css::datatransfer::dnd::XDragSourceListener > mxDnDListener;

    // DragAndDropClient
    using vcl::unohelper::DragAndDropClient::dragEnter;
    using vcl::unohelper::DragAndDropClient::dragExit;
    using vcl::unohelper::DragAndDropClient::dragOver;
    virtual void dragGestureRecognized(const css::datatransfer::dnd::DragGestureEvent& dge) override;
    virtual void dragDropEnd(const css::datatransfer::dnd::DragSourceDropEvent& dsde) override;
    virtual void drop(const css::datatransfer::dnd::DropTargetDropEvent& dtde) override;
    virtual void dragEnter(const css::datatransfer::dnd::DropTargetDragEnterEvent& dtdee) override;
    virtual void dragExit(const css::datatransfer::dnd::DropTargetEvent& dte) override;
    virtual void dragOver(const css::datatransfer::dnd::DropTargetDragEvent& dtde) override;

protected:
    Edit(WindowType nType);
    virtual void FillLayoutData() const override;
    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;
public:
    // public because needed in button.cxx
    SAL_DLLPRIVATE bool ImplUseNativeBorder(vcl::RenderContext const & rRenderContext, WinBits nStyle);

    Edit( vcl::Window* pParent, WinBits nStyle = WB_BORDER );
    virtual ~Edit() override;
    virtual void dispose() override;

    virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void        MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void        KeyInput( const KeyEvent& rKEvt ) override;
    virtual void        Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void        Resize() override;
    virtual void        Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, DrawFlags nFlags ) override;
    virtual void        GetFocus() override;
    virtual void        LoseFocus() override;
    virtual void        Tracking( const TrackingEvent& rTEvt ) override;
    virtual void        Command( const CommandEvent& rCEvt ) override;
    virtual void        StateChanged( StateChangedType nType ) override;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual void        Modify();
    virtual void        UpdateData();

    static bool         IsCharInput( const KeyEvent& rKEvt );

    virtual void        SetModifyFlag();
    virtual void        ClearModifyFlag();
    virtual bool        IsModified() const { return mpSubEdit ? mpSubEdit->mbModified : mbModified; }

    virtual void        EnableUpdateData( sal_uLong nTimeout = EDIT_UPDATEDATA_TIMEOUT );
    virtual void        DisableUpdateData();

    void                SetEchoChar( sal_Unicode c );
    sal_Unicode         GetEchoChar() const { return mcEchoChar; }

    virtual void        SetReadOnly( bool bReadOnly = true );
    virtual bool        IsReadOnly() const { return mbReadOnly; }

    void                SetSelectAllSingleClick(bool bSelectAllSingleClick );
    bool                IsSelectAllSingleClick() const { return mbSelectAllSingleClick; }

    void                SetInsertMode( bool bInsert );
    bool                IsInsertMode() const;

    virtual void        SetMaxTextLen( sal_Int32 nMaxLen );
    virtual sal_Int32   GetMaxTextLen() const { return mnMaxTextLen; }

    void                SetWidthInChars(sal_Int32 nWidthInChars);

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

    virtual void        SetText( const OUString& rStr ) override;
    virtual void        SetText( const OUString& rStr, const Selection& rNewSelection );
    virtual OUString    GetText() const override;

    void                SetCursorAtLast();

    void                SetPlaceholderText( const OUString& rStr );
    OUString            GetPlaceholderText() const;

    void                SaveValue() { maSaveValue = GetText(); }
    const OUString&     GetSavedValue() const { return maSaveValue; }
    bool                IsValueChangedFromSaved() const { return maSaveValue != GetText(); }

    virtual void        SetModifyHdl( const Link<Edit&,void>& rLink ) { maModifyHdl = rLink; }
    virtual const Link<Edit&,void>& GetModifyHdl() const { return maModifyHdl; }
    virtual void        SetUpdateDataHdl( const Link<Edit&,void>& rLink ) { maUpdateDataHdl = rLink; }

    void                SetActivateHdl(const Link<Edit&,void>& rLink) { maActivateHdl = rLink; }

    void                SetSubEdit( Edit* pEdit );
    Edit*               GetSubEdit() const { return mpSubEdit; }

    void                SetAutocompleteHdl( const Link<Edit&,void>& rLink ) { maAutocompleteHdl = rLink; }
    const Link<Edit&,void>& GetAutocompleteHdl() const { return maAutocompleteHdl; }

    virtual Size        CalcMinimumSize() const;
    virtual Size        CalcMinimumSizeForText(const OUString &rString) const;
    virtual Size        GetOptimalSize() const override;
    virtual Size        CalcSize(sal_Int32 nChars) const;
    sal_Int32           GetMaxVisChars() const;

    sal_Int32           GetCharPos( const Point& rWindowPos ) const;

    // shows a warning box saying "text too long, truncated"
    static void         ShowTruncationWarning(weld::Widget* pParent);

    static void                 SetGetSpecialCharsFunction( FncGetSpecialChars fn );
    static FncGetSpecialChars   GetGetSpecialCharsFunction();

    VclPtr<PopupMenu>           CreatePopupMenu();

    virtual OUString GetSurroundingText() const override;
    virtual Selection GetSurroundingTextSelection() const override;
    virtual bool set_property(const OString &rKey, const OUString &rValue) override;

    void SetTextFilter(TextFilter* pFilter) { mpFilterText = pFilter; }

    virtual FactoryFunction GetUITestFactory() const override;

    // returns the minimum size a bordered Edit should have given the current
    // global style settings (needed by sc's inputwin.cxx)
    static Size GetMinimumEditSize();

    void SetForceControlBackground(bool b) { mbForceControlBackground = b; }

    bool IsPassword() const { return mbPassword; }
};

#endif // INCLUDED_VCL_EDIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
