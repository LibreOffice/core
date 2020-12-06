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
enum class SwFillMode;

/// Item for settings dialog - document view
class SW_DLLPUBLIC SwDocDisplayItem : public SfxPoolItem
{
    friend class SwShdwCursorOptionsTabPage;
    friend class SwModule;

    bool m_bParagraphEnd      :1;
    bool m_bTab               :1;
    bool m_bSpace             :1;
    bool m_bNonbreakingSpace  :1;
    bool m_bSoftHyphen        :1;
    bool m_bCharHiddenText    :1;
    bool m_bBookmarks         :1;
    bool m_bManualBreak       :1;
    sal_Int32 m_xDefaultAnchor;

public:
                                SwDocDisplayItem();
                                SwDocDisplayItem( const SwViewOption& rVOpt );

    virtual SwDocDisplayItem*   Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool                operator==( const SfxPoolItem& ) const override;
    void                        FillViewOptions( SwViewOption& rVOpt) const;
};

// OS 12.01.95
// Item for settings dialog, element page
class SW_DLLPUBLIC SwElemItem : public SfxPoolItem
{
    //view
    bool m_bVertRuler     :1;
    bool m_bVertRulerRight:1;
    bool m_bSmoothScroll  :1;
    //visual aids
    bool m_bCrosshair     :1;
    //display
    bool m_bTable             :1;
    bool m_bGraphic           :1;
    bool m_bDrawing           :1;
    bool m_bNotes             :1;
    bool m_bShowInlineTooltips :1;
    bool m_bShowOutlineContentVisibilityButton :1;
    bool m_bTreatSubOutlineLevelsAsContent :1;
    bool m_bShowChangesInMargin :1;
    bool m_bFieldHiddenText   :1;
    bool m_bShowHiddenPara    :1;

    friend class SwContentOptPage;

public:
                            SwElemItem();
                            SwElemItem(const SwViewOption& rVOpt);

    virtual SwElemItem*     Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;

    void                    FillViewOptions( SwViewOption& rVOpt) const;

};

// OS 12.01.95
// Item for settings dialog - printer/add-ons
class SW_DLLPUBLIC SwAddPrinterItem : public SfxPoolItem, public SwPrintData
{
    using  SwPrintData::operator ==;

public:
    SwAddPrinterItem();
    SwAddPrinterItem( const SwPrintData& rPrtData );

    virtual SwAddPrinterItem* Clone( SfxItemPool *pPool = nullptr ) const override;

    virtual bool         operator==( const SfxPoolItem& ) const override;
};

/// Item for settings dialog, ShadowCursorPage
class SW_DLLPUBLIC SwShadowCursorItem : public SfxPoolItem
{
    SwFillMode m_eMode;
    bool m_bOn;
public:
    SwShadowCursorItem();
    SwShadowCursorItem( const SwViewOption& rVOpt );

    virtual SwShadowCursorItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;

    void FillViewOptions( SwViewOption& rVOpt) const;

    SwFillMode GetMode() const          { return m_eMode; }
    bool IsOn() const                   { return m_bOn; }

    void SetMode( SwFillMode eM )       { m_eMode = eM; }
    void SetOn( bool bFlag )            { m_bOn = bFlag; }
};

#ifdef DBG_UTIL

// Item for settings dialog - test settings
class SW_DLLPUBLIC SwTestItem : public SfxPoolItem
{
    friend class SwModule;
    friend class SwTestTabPage;

    bool    m_bTest1:1;
    bool    m_bTest2:1;
    bool    m_bTest3:1;
    bool    m_bTest4:1;
    bool    m_bTest5:1;
    bool    m_bTest6:1;
    bool    m_bTest7:1;
    bool    m_bTest8:1;
    bool    m_bTest9:1;
    bool    m_bTest10:1;

public:
                            SwTestItem() : SfxPoolItem(FN_PARAM_SWTEST) {};

    virtual SwTestItem*     Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;

};
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
