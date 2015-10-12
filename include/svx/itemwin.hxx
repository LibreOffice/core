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

                    DECL_LINK_TYPED(DelayHdl_Impl, Timer *, void);

    void            ReleaseFocus_Impl();

public:
    SvxLineBox( vcl::Window* pParent,
                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                WinBits nBits = WB_BORDER | WB_DROPDOWN | WB_AUTOHSCROLL );

    void FillControl();

protected:
    virtual void    Select() override;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool    Notify( NotifyEvent& rNEvt ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

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

    static void     ReleaseFocus_Impl();

protected:
    virtual void    Modify() override;
    virtual void    Down() override;
    virtual void    Up() override;       // just to be sure

    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool    Notify( NotifyEvent& rNEvt ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

public:
    SvxMetricField( vcl::Window* pParent,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                    WinBits nBits = WB_BORDER | WB_SPIN | WB_REPEAT );

    void            Update( const XLineWidthItem* pItem );
    void            SetCoreUnit( SfxMapUnit eUnit );
    void            RefreshDlgUnit();
};

// class SvxFillTypeBox --------------------------------------------------

class SvxFillTypeBox : public FillTypeLB
{
public:
    SvxFillTypeBox( vcl::Window* pParent, WinBits nBits = WB_BORDER | WB_DROPDOWN | WB_AUTOHSCROLL );

    void            Selected() { bSelect = true; }

protected:
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool    Notify( NotifyEvent& rNEvt ) override;

private:
    sal_uInt16      nCurPos;
    bool            bSelect;
    bool            bRelease;

    static void     ReleaseFocus_Impl();
};

// class SvxFillAttrBox --------------------------------------------------

class SVX_DLLPUBLIC SvxFillAttrBox : public FillAttrLB
{
public:
    SvxFillAttrBox( vcl::Window* pParent, WinBits nBits = WB_BORDER | WB_DROPDOWN | WB_AUTOHSCROLL );

protected:
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool    Notify( NotifyEvent& rNEvt ) override;
    virtual void    Select() override;

private:
    sal_uInt16      nCurPos;
    bool            bRelease;

    static void     ReleaseFocus_Impl();
};

#endif // INCLUDED_SVX_ITEMWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
