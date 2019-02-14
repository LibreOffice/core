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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_CFGITEMS_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_CFGITEMS_HXX

#include <tools/color.hxx>
#include <svl/poolitem.hxx>
#include <swdllapi.h>
#include <printdata.hxx>

#include <cmdid.h>

class SwModule;
#ifdef DBG_UTIL
class SwTestTabPage;
#endif
class SwAddPrinterTabPage;
class SfxPrinter;
class SwViewShell;
class SwViewOption;
class SwContentOptPage;
class SwShdwCursorOptionsTabPage;

// OS 12.01.95
// Item for settings dialog - document view
class SW_DLLPUBLIC SwDocDisplayItem : public SfxPoolItem
{
    friend class SwShdwCursorOptionsTabPage;
    friend class SwModule;

    bool bParagraphEnd      :1;
    bool bTab               :1;
    bool bSpace             :1;
    bool bNonbreakingSpace  :1;
    bool bSoftHyphen        :1;
    bool bCharHiddenText    :1;
    bool bManualBreak       :1;

public:
                                SwDocDisplayItem();
                                SwDocDisplayItem( const SwViewOption& rVOpt );

    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool                operator==( const SfxPoolItem& ) const override;
    void                        FillViewOptions( SwViewOption& rVOpt) const;
};

// OS 12.01.95
// Item for settings dialog, element page
class SW_DLLPUBLIC SwElemItem : public SfxPoolItem
{
    //view
    bool bVertRuler     :1;
    bool bVertRulerRight:1;
    bool bSmoothScroll  :1;
    //visual aids
    bool bCrosshair     :1;
    //display
    bool bTable             :1;
    bool bGraphic           :1;
    bool bDrawing           :1;
    bool bFieldName         :1;
    bool bNotes             :1;
    bool bShowInlineTooltips :1;
    bool bFieldHiddenText   :1;
    bool bShowHiddenPara    :1;

    friend class SwContentOptPage;

public:
                            SwElemItem();
                            SwElemItem(const SwViewOption& rVOpt);

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;

    void                    FillViewOptions( SwViewOption& rVOpt) const;

};

// OS 12.01.95
// Item for settings dialog - printer/add-ons
class SW_DLLPUBLIC SwAddPrinterItem : public SfxPoolItem, public SwPrintData
{
    friend class SwAddPrinterTabPage;

    using  SwPrintData::operator ==;

public:
    SwAddPrinterItem();
    SwAddPrinterItem( const SwPrintData& rPrtData );

    virtual SfxPoolItem* Clone( SfxItemPool *pPool = nullptr ) const override;

    virtual bool         operator==( const SfxPoolItem& ) const override;

    const OUString &GetFax() const              { return m_sFaxName; }
};

// Item for settings dialog, ShadowCursorPage
class SW_DLLPUBLIC SwShadowCursorItem : public SfxPoolItem
{
    sal_uInt8 eMode;
    bool bOn;
public:
    SwShadowCursorItem();
    SwShadowCursorItem( const SwViewOption& rVOpt );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;

    void FillViewOptions( SwViewOption& rVOpt) const;

    sal_uInt8 GetMode() const               { return eMode; }
    bool IsOn() const                   { return bOn; }

    void SetMode( sal_uInt8 eM )            { eMode = eM; }
    void SetOn( bool bFlag )            { bOn = bFlag; }
};

#ifdef DBG_UTIL

// OS 12.01.95
// Item for settings dialog - test settings
class SW_DLLPUBLIC SwTestItem : public SfxPoolItem
{
    friend class SwModule;
    friend class SwTestTabPage;

    bool    bTest1:1;
    bool    bTest2:1;
    bool    bTest3:1;
    bool    bTest4:1;
    bool    bTest5:1;
    bool    bTest6:1;
    bool    bTest7:1;
    bool    bTest8:1;
    bool    bTest9:1;
    bool    bTest10:1;

public:
                            SwTestItem() : SfxPoolItem(FN_PARAM_SWTEST) {};

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;

};
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
