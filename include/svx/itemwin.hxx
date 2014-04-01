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
#ifndef INCLUDED_SVX_ITEMWIN_HXX
#define INCLUDED_SVX_ITEMWIN_HXX

#include <vcl/bitmap.hxx>

#include <svx/dlgctrl.hxx>
#include <svx/svxdllapi.h>

// forward ---------------------------------------------------------------

class XLineColorItem;
class XLineWidthItem;
class SfxObjectShell;

// class SvxLineBox ------------------------------------------------------

class SvxLineBox : public LineLB
{
    sal_uInt16      nCurPos;
    Timer           aDelayTimer;
    Size            aLogicalSize;
    bool            bRelease;
    SfxObjectShell* mpSh;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > mxFrame;

                    DECL_LINK(DelayHdl_Impl, void *);

    void            ReleaseFocus_Impl();

public:
    SvxLineBox( Window* pParent,
                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                WinBits nBits = WB_BORDER | WB_DROPDOWN | WB_AUTOHSCROLL );
    virtual ~SvxLineBox();

    void FillControl();

protected:
    virtual void    Select() SAL_OVERRIDE;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

};

// class SvxColorBox -----------------------------------------------------

class SvxColorBox : public ColorLB
{
    using Window::Update;

    sal_uInt16      nCurPos;
    Timer           aDelayTimer;
    Size            aLogicalSize;
    bool            bRelease;
    OUString maCommand;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > mxFrame;

    void            ReleaseFocus_Impl();

public:
    SvxColorBox( Window* pParent,
                 const OUString& rCommand,
                 const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                 WinBits nBits = WB_BORDER | WB_DROPDOWN | WB_AUTOHSCROLL );
    virtual ~SvxColorBox();

    void            Update( const XLineColorItem* pItem );

protected:
    virtual void    Select() SAL_OVERRIDE;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

};

// class SvxMetricField --------------------------------------------------

class SVX_DLLPUBLIC SvxMetricField : public MetricField
{
    using Window::Update;

    OUString        aCurTxt;
    SfxMapUnit      ePoolUnit;
    FieldUnit       eDlgUnit;
    Size            aLogicalSize;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > mxFrame;

    void            ReleaseFocus_Impl();

protected:
    virtual void    Modify() SAL_OVERRIDE;
    virtual void    Down() SAL_OVERRIDE;
    virtual void    Up() SAL_OVERRIDE;       // just to be sure

    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

public:
    SvxMetricField( Window* pParent,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                    WinBits nBits = WB_BORDER | WB_SPIN | WB_REPEAT );
    virtual ~SvxMetricField();

    void            Update( const XLineWidthItem* pItem );
    void            SetCoreUnit( SfxMapUnit eUnit );
    void            RefreshDlgUnit();
};

// class SvxFillTypeBox --------------------------------------------------

class SvxFillTypeBox : public FillTypeLB
{
public:
    SvxFillTypeBox( Window* pParent, WinBits nBits = WB_BORDER | WB_DROPDOWN | WB_AUTOHSCROLL );
    virtual ~SvxFillTypeBox();

    void            Selected() { bSelect = true; }
    bool            IsRelease() { return bRelease;}

protected:
    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

private:
    sal_uInt16      nCurPos;
    bool            bSelect;
    bool            bRelease;

    void            ReleaseFocus_Impl();
};

// class SvxFillAttrBox --------------------------------------------------

class SvxFillAttrBox : public FillAttrLB
{
public:
    SvxFillAttrBox( Window* pParent, WinBits nBits = WB_BORDER | WB_DROPDOWN | WB_AUTOHSCROLL );
    virtual ~SvxFillAttrBox();

    bool            IsRelease() { return bRelease;}

protected:
    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void    Select() SAL_OVERRIDE;

private:
    sal_uInt16      nCurPos;
    bool            bRelease;

    void            ReleaseFocus_Impl();
};

#endif // INCLUDED_SVX_ITEMWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
