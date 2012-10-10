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

#ifndef _VCLMEDIT_HXX
#define _VCLMEDIT_HXX

#include <tools/wintypes.hxx>
#include <vcl/edit.hxx>
#include <vcl/dllapi.h>


class ImpSvMEdit;
class Timer;
class ExtTextEngine;
class ExtTextView;

class VCL_DLLPUBLIC VclMultiLineEdit : public Edit
{
private:
    ImpSvMEdit*     pImpSvMEdit;

    XubString       aSaveValue;
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

    virtual void    SetSelection( const Selection& rSelection );
    virtual const Selection& GetSelection() const;

    virtual void        ReplaceSelected( const XubString& rStr );
    virtual void        DeleteSelected();
    virtual XubString   GetSelected() const;
    virtual XubString   GetSelected( LineEnd aSeparator ) const;

    virtual void    Cut();
    virtual void    Copy();
    virtual void    Paste();

    virtual void    SetText( const XubString& rStr );
    virtual void    SetText( const XubString& rStr, const Selection& rNewSelection )
                    { SetText( rStr ); SetSelection( rNewSelection ); }
    String          GetText() const;
    String          GetText( LineEnd aSeparator ) const;
    String          GetTextLines( LineEnd aSeparator ) const;

    void            SetRightToLeft( sal_Bool bRightToLeft );
    sal_Bool            IsRightToLeft() const;

    void            SaveValue()                         { aSaveValue = GetText(); }
    const XubString&    GetSavedValue() const               { return aSaveValue; }

    void            SetModifyHdl( const Link& rLink )   { aModifyHdlLink = rLink; }
    const Link&     GetModifyHdl() const                { return aModifyHdlLink; }

    void            SetUpdateDataHdl( const Link& rLink ) { aUpdateDataHdlLink = rLink; }
    const Link&     GetUpdateDataHdl() const { return aUpdateDataHdlLink; }

    virtual void    Resize();
    virtual void    GetFocus();

    Size            CalcMinimumSize() const;
    Size            CalcAdjustedSize( const Size& rPrefSize ) const;
    using Edit::CalcSize;
    Size            CalcSize( sal_uInt16 nColumns, sal_uInt16 nLines ) const;
    void            GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const;

    void            Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );

    void            SetLeftMargin( sal_uInt16 n );

    void            DisableSelectionOnFocus();

    void            SetTextSelectable( sal_Bool bTextSelectable );
};

inline sal_uLong VclMultiLineEdit::IsUpdateDataEnabled() const
{
    return pUpdateDataTimer ? pUpdateDataTimer->GetTimeout() : 0;
}

#endif  //_VCLMEDIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
