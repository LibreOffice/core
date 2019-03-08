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

#ifndef INCLUDED_VCL_VCLMEDIT_HXX
#define INCLUDED_VCL_VCLMEDIT_HXX

#include <tools/lineend.hxx>
#include <tools/wintypes.hxx>
#include <vcl/edit.hxx>
#include <vcl/dllapi.h>
#include <vcl/timer.hxx>
#include <memory>

class ImpVclMEdit;
class ExtTextEngine;
class TextView;

class TextWindow : public vcl::Window
{
private:
    VclPtr<Edit>    mxParent;
    std::unique_ptr<ExtTextEngine> mpExtTextEngine;
    std::unique_ptr<TextView> mpExtTextView;

    bool            mbInMBDown;
    bool            mbFocusSelectionHide;
    bool            mbIgnoreTab;
    bool            mbActivePopup;
    bool            mbSelectOnTab;

public:
    explicit        TextWindow(Edit* pParent);
    virtual         ~TextWindow() override;
    virtual void    dispose() override;

    ExtTextEngine*  GetTextEngine() const { return mpExtTextEngine.get(); }
    TextView*       GetTextView() const { return mpExtTextView.get(); }

    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    KeyInput( const KeyEvent& rKEvent ) override;

    virtual void    Command( const CommandEvent& rCEvt ) override;

    virtual void    Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void    Resize() override;

    virtual void    GetFocus() override;
    virtual void    LoseFocus() override;

    void            SetAutoFocusHide( bool bAutoHide ) { mbFocusSelectionHide = bAutoHide; }

    void            SetIgnoreTab( bool bIgnore ) { mbIgnoreTab = bIgnore; }

    void            DisableSelectionOnFocus() { mbSelectOnTab = false; }
};

class VCL_DLLPUBLIC VclMultiLineEdit : public Edit
{
    friend class VCLXAccessibleEdit;

private:
    std::unique_ptr<ImpVclMEdit> pImpVclMEdit;

    Link<Edit&,void>  aModifyHdlLink;

    std::unique_ptr<Timer> pUpdateDataTimer;
    Link<Edit&,void>  aUpdateDataHdlLink;

protected:

    DECL_LINK( ImpUpdateDataHdl, Timer*, void );
    void            StateChanged( StateChangedType nType ) override;
    void            DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;
    using Control::ImplInitSettings;
    void            ImplInitSettings( bool bBackground );
    static WinBits  ImplInitStyle( WinBits nStyle );

    TextView*       GetTextView() const;
    ExtTextEngine*  GetTextEngine() const;

    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;
public:
                    VclMultiLineEdit( vcl::Window* pParent,
                                      WinBits nWinStyle );
    virtual         ~VclMultiLineEdit() override;
    virtual void    dispose() override;

    void            SelectionChanged();
    void            CaretChanged();
    virtual void    Modify() override;
    virtual void    UpdateData() override;

    virtual void    SetModifyFlag() override;
    virtual void    ClearModifyFlag() override;
    virtual bool    IsModified() const override;

    virtual void    EnableUpdateData( sal_uLong nTimeout = EDIT_UPDATEDATA_TIMEOUT ) override;
    virtual void    DisableUpdateData() override { pUpdateDataTimer.reset(); }

    virtual void    SetReadOnly( bool bReadOnly = true ) override;
    virtual bool    IsReadOnly() const override;

    void            EnableFocusSelectionHide( bool bHide );

    virtual void    SetMaxTextLen(sal_Int32 nMaxLen) override;
    virtual sal_Int32 GetMaxTextLen() const override;

    void            SetMaxTextWidth(long nMaxWidth);

    virtual void    SetSelection( const Selection& rSelection ) override;
    virtual const Selection& GetSelection() const override;

    virtual void        ReplaceSelected( const OUString& rStr ) override;
    virtual void        DeleteSelected() override;
    virtual OUString    GetSelected() const override;
    OUString            GetSelected( LineEnd aSeparator ) const;

    virtual void    Cut() override;
    virtual void    Copy() override;
    virtual void    Paste() override;

    virtual void    SetText( const OUString& rStr ) override;
    virtual void    SetText( const OUString& rStr, const Selection& rNewSelection ) override
                    { SetText( rStr ); SetSelection( rNewSelection ); }
    OUString        GetText() const override;
    OUString        GetText( LineEnd aSeparator ) const;
    OUString        GetTextLines( LineEnd aSeparator ) const;

    void            SetRightToLeft( bool bRightToLeft );

    void            SetModifyHdl( const Link<Edit&,void>& rLink ) override { aModifyHdlLink = rLink; }
    const Link<Edit&,void>&   GetModifyHdl() const override                { return aModifyHdlLink; }

    void            SetUpdateDataHdl( const Link<Edit&,void>& rLink ) override { aUpdateDataHdlLink = rLink; }

    virtual void    Resize() override;
    virtual void    GetFocus() override;

    virtual Size    CalcMinimumSize() const override;
    Size            CalcAdjustedSize( const Size& rPrefSize ) const;
    Size            CalcBlockSize( sal_uInt16 nColumns, sal_uInt16 nLines ) const;
    void            GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const;

    void            Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, DrawFlags nFlags ) override;

    void            DisableSelectionOnFocus();

    void            EnableCursor( bool bEnable );

    ScrollBar&      GetVScrollBar() const;

    TextWindow*     GetTextWindow();

    virtual FactoryFunction GetUITestFactory() const override;

    virtual bool set_property(const OString &rKey, const OUString &rValue) override;
};

#endif // INCLUDED_VCL_VCLMEDIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
