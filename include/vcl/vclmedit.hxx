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

class ImpVclMEdit;
class Timer;
class ExtTextEngine;
class ExtTextView;

class VCL_DLLPUBLIC VclMultiLineEdit : public Edit
{
private:
    ImpVclMEdit*    pImpVclMEdit;

    OUString        aSaveValue;
    Link<>          aModifyHdlLink;

    Timer*          pUpdateDataTimer;
    Link<>          aUpdateDataHdlLink;

protected:

    DECL_LINK_TYPED( ImpUpdateDataHdl, Timer*, void );
    void            StateChanged( StateChangedType nType ) SAL_OVERRIDE;
    void            DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    using Control::ImplInitSettings;
    void            ImplInitSettings( bool bFont, bool bForeground, bool bBackground );
    static WinBits  ImplInitStyle( WinBits nStyle );

    ExtTextEngine*  GetTextEngine() const;
    ExtTextView*    GetTextView() const;
    ScrollBar*      GetVScrollBar() const;

    virtual void ApplySettings(vcl::RenderContext& rRenderContext) SAL_OVERRIDE;
public:
                    VclMultiLineEdit( vcl::Window* pParent,
                                      WinBits nWinStyle = WB_LEFT | WB_BORDER );
    virtual         ~VclMultiLineEdit();
    virtual void    dispose() SAL_OVERRIDE;

    void            SelectionChanged();
    void            CaretChanged();
    virtual void    Modify() SAL_OVERRIDE;
    virtual void    UpdateData() SAL_OVERRIDE;

    virtual void    SetModifyFlag() SAL_OVERRIDE;
    virtual void    ClearModifyFlag() SAL_OVERRIDE;
    virtual bool    IsModified() const SAL_OVERRIDE;

    virtual void    EnableUpdateData( sal_uLong nTimeout = EDIT_UPDATEDATA_TIMEOUT ) SAL_OVERRIDE;
    virtual void    DisableUpdateData() SAL_OVERRIDE { delete pUpdateDataTimer; pUpdateDataTimer = NULL; }

    virtual void    SetReadOnly( bool bReadOnly = true ) SAL_OVERRIDE;
    virtual bool    IsReadOnly() const SAL_OVERRIDE;

    void            EnableFocusSelectionHide( bool bHide );

    virtual void    SetMaxTextLen(sal_Int32 nMaxLen = 0) SAL_OVERRIDE;
    virtual sal_Int32 GetMaxTextLen() const SAL_OVERRIDE;

    void            SetMaxTextWidth( sal_uLong nMaxWidth );

    virtual void    SetSelection( const Selection& rSelection ) SAL_OVERRIDE;
    virtual const Selection& GetSelection() const SAL_OVERRIDE;

    virtual void        ReplaceSelected( const OUString& rStr ) SAL_OVERRIDE;
    virtual void        DeleteSelected() SAL_OVERRIDE;
    virtual OUString    GetSelected() const SAL_OVERRIDE;
    OUString            GetSelected( LineEnd aSeparator ) const;

    virtual void    Cut() SAL_OVERRIDE;
    virtual void    Copy() SAL_OVERRIDE;
    virtual void    Paste() SAL_OVERRIDE;

    virtual void    SetText( const OUString& rStr ) SAL_OVERRIDE;
    virtual void    SetText( const OUString& rStr, const Selection& rNewSelection ) SAL_OVERRIDE
                    { SetText( rStr ); SetSelection( rNewSelection ); }
    OUString        GetText() const SAL_OVERRIDE;
    OUString        GetText( LineEnd aSeparator ) const;
    OUString        GetTextLines( LineEnd aSeparator ) const;

    void            SetRightToLeft( bool bRightToLeft );

    void            SaveValue()                         { aSaveValue = GetText(); }
    const OUString&    GetSavedValue() const            { return aSaveValue; }
    bool               IsValueChangedFromSaved() const  { return aSaveValue != GetText(); }

    void            SetModifyHdl( const Link<>& rLink ) SAL_OVERRIDE { aModifyHdlLink = rLink; }
    const Link<>&   GetModifyHdl() const SAL_OVERRIDE                { return aModifyHdlLink; }

    void            SetUpdateDataHdl( const Link<>& rLink ) SAL_OVERRIDE { aUpdateDataHdlLink = rLink; }

    virtual void    Resize() SAL_OVERRIDE;
    virtual void    GetFocus() SAL_OVERRIDE;

    virtual Size    CalcMinimumSize() const SAL_OVERRIDE;
    Size            CalcAdjustedSize( const Size& rPrefSize ) const;
    Size            CalcBlockSize( sal_uInt16 nColumns, sal_uInt16 nLines ) const;
    void            GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const;

    void            Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags ) SAL_OVERRIDE;

    void            SetLeftMargin( sal_uInt16 n );

    void            DisableSelectionOnFocus();

    void            EnableCursor( bool bEnable );

    virtual bool set_property(const OString &rKey, const OString &rValue) SAL_OVERRIDE;
};

#endif // INCLUDED_VCL_VCLMEDIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
