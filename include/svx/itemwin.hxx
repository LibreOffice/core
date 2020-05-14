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

#include <vcl/field.hxx>

#include <svx/dlgctrl.hxx>
#include <svx/svxdllapi.h>

class XLineWidthItem;
class SfxObjectShell;

class SvxLineBox final : public ListBox
{
    sal_uInt16      nCurPos;
    Timer           aDelayTimer;
    Size const      aLogicalSize;
    bool            bRelease;
    SfxObjectShell* mpSh;
    css::uno::Reference< css::frame::XFrame > mxFrame;

                    DECL_LINK(DelayHdl_Impl, Timer *, void);

    void            ReleaseFocus_Impl();

public:
    SvxLineBox( vcl::Window* pParent,
                const css::uno::Reference< css::frame::XFrame >& rFrame );

    void FillControl();

    void Fill(const XDashListRef &pList);

private:
    virtual void    Select() override;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

};

class SVX_DLLPUBLIC SvxMetricField : public MetricField
{
    using Window::Update;

    OUString        aCurTxt;
    MapUnit         eDestPoolUnit;
    FieldUnit       eDlgUnit;
    Size            aLogicalSize;
    css::uno::Reference< css::frame::XFrame > mxFrame;

    static void     ReleaseFocus_Impl();

protected:
    virtual void    Modify() override;

    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

public:
    SvxMetricField( vcl::Window* pParent,
                    const css::uno::Reference< css::frame::XFrame >& rFrame );

    void            Update( const XLineWidthItem* pItem );
    void            SetDestCoreUnit( MapUnit eUnit );
    void            RefreshDlgUnit();
};

class SVX_DLLPUBLIC SvxFillTypeBox final : public FillTypeLB
{
public:
    SvxFillTypeBox( vcl::Window* pParent );

    void            Selected() { bSelect = true; }

private:
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;

    sal_uInt16      nCurPos;
    bool            bSelect;

    static void     ReleaseFocus_Impl();
};

class SVX_DLLPUBLIC SvxFillAttrBox final : public ListBox
{
public:
    SvxFillAttrBox( vcl::Window* pParent );

    void Fill( const XHatchListRef    &pList );
    void Fill( const XGradientListRef &pList );
    void Fill( const XBitmapListRef   &pList );
    void Fill( const XPatternListRef  &pList );

private:
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;

    sal_uInt16      nCurPos;
    BitmapEx        maBitmapEx;

    static void     ReleaseFocus_Impl();
};

#endif // INCLUDED_SVX_ITEMWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
