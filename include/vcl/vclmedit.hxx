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

#ifndef _VCLMEDIT_HXX
#define _VCLMEDIT_HXX

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
    Link            aModifyHdlLink;

    Timer*          pUpdateDataTimer;
    Link            aUpdateDataHdlLink;

protected:

    DECL_LINK(      ImpUpdateDataHdl, void* );
    void            StateChanged( StateChangedType nType );
    void            DataChanged( const DataChangedEvent& rDCEvt );
    virtual long    PreNotify( NotifyEvent& rNEvt );
    long            Notify( NotifyEvent& rNEvt );
    using Control::ImplInitSettings;
    void            ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
    WinBits         ImplInitStyle( WinBits nStyle );

    ExtTextEngine*  GetTextEngine() const;
    ExtTextView*    GetTextView() const;
    ScrollBar*      GetVScrollBar() const;

public:
                    VclMultiLineEdit( Window* pParent, WinBits nWinStyle = WB_LEFT | WB_BORDER );
                    VclMultiLineEdit( Window* pParent, const ResId& rResId );
                    virtual ~VclMultiLineEdit();


    virtual void    Modify();
    virtual void    UpdateData();

    virtual void    SetModifyFlag();
    virtual void    ClearModifyFlag();
    virtual sal_Bool    IsModified() const;

    virtual void    EnableUpdateData( sal_uLong nTimeout = EDIT_UPDATEDATA_TIMEOUT );
    virtual void    DisableUpdateData() { delete pUpdateDataTimer; pUpdateDataTimer = NULL; }
    virtual sal_uLong   IsUpdateDataEnabled() const;

    virtual void    SetReadOnly( sal_Bool bReadOnly = sal_True );
    virtual sal_Bool    IsReadOnly() const;

    void            EnableFocusSelectionHide( sal_Bool bHide );

    virtual void    SetMaxTextLen( xub_StrLen nMaxLen = 0 );
    virtual xub_StrLen GetMaxTextLen() const;

    void            SetMaxTextWidth( sal_uLong nMaxWidth );

    virtual void    SetSelection( const Selection& rSelection );
    virtual const Selection& GetSelection() const;

    virtual void        ReplaceSelected( const OUString& rStr );
    virtual void        DeleteSelected();
    virtual OUString    GetSelected() const;
    virtual OUString    GetSelected( LineEnd aSeparator ) const;

    virtual void    Cut();
    virtual void    Copy();
    virtual void    Paste();

    virtual void    SetText( const OUString& rStr );
    virtual void    SetText( const OUString& rStr, const Selection& rNewSelection )
                    { SetText( rStr ); SetSelection( rNewSelection ); }
    OUString        GetText() const;
    OUString        GetText( LineEnd aSeparator ) const;
    OUString        GetTextLines( LineEnd aSeparator ) const;

    void            SetRightToLeft( sal_Bool bRightToLeft );
    sal_Bool            IsRightToLeft() const;

    void            SaveValue()                         { aSaveValue = GetText(); }
    const OUString&    GetSavedValue() const            { return aSaveValue; }

    void            SetModifyHdl( const Link& rLink )   { aModifyHdlLink = rLink; }
    const Link&     GetModifyHdl() const                { return aModifyHdlLink; }

    void            SetUpdateDataHdl( const Link& rLink ) { aUpdateDataHdlLink = rLink; }

    virtual void    Resize();
    virtual void    GetFocus();

    virtual Size    CalcMinimumSize() const;
    Size            CalcAdjustedSize( const Size& rPrefSize ) const;
    using Edit::CalcSize;
    Size            CalcSize( sal_uInt16 nColumns, sal_uInt16 nLines ) const;
    void            GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const;

    void            Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );

    void            SetLeftMargin( sal_uInt16 n );

    void            DisableSelectionOnFocus();

    void            SetTextSelectable( bool bTextSelectable );
    void            EnableCursor( sal_Bool bEnable );

    virtual bool set_property(const OString &rKey, const OString &rValue);
};

inline sal_uLong VclMultiLineEdit::IsUpdateDataEnabled() const
{
    return pUpdateDataTimer ? pUpdateDataTimer->GetTimeout() : 0;
}

#endif  //_VCLMEDIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
