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
#ifndef INCLUDED_SW_SOURCE_UI_INC_CFGITEMS_HXX
#define INCLUDED_SW_SOURCE_UI_INC_CFGITEMS_HXX

#include <tools/color.hxx>
#include <svl/poolitem.hxx>
#include "swdllapi.h"
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
class SwShdwCrsrOptionsTabPage;

SfxPrinter* GetPrt( SwViewShell* );
void        SetPrt( SfxPrinter* );

/*--------OS 12.01.95 -----------------------------------
Item for settings dialog - document view
--------------------------------------------------------- */
class SW_DLLPUBLIC SwDocDisplayItem : public SfxPoolItem
{
    friend class SwShdwCrsrOptionsTabPage;
    friend class SwModule;

    sal_Bool bParagraphEnd      :1;
    sal_Bool bTab               :1;
    sal_Bool bSpace             :1;
    sal_Bool bNonbreakingSpace  :1;
    sal_Bool bSoftHyphen        :1;
    sal_Bool bCharHiddenText    :1;
    sal_Bool bFldHiddenText     :1;
    sal_Bool bManualBreak       :1;
    sal_Bool bShowHiddenPara    :1;

    Color aIndexBackgrndCol;

public:
                                TYPEINFO_OVERRIDE();
                                SwDocDisplayItem( sal_uInt16 nWhich = FN_PARAM_DOCDISP );
                                SwDocDisplayItem(
                                    const SwDocDisplayItem& rSwDocDisplayItem );
                                SwDocDisplayItem( const SwViewOption& rVOpt,
                                                                sal_uInt16 nWhich );

    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const SAL_OVERRIDE;
    virtual bool                operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
    void                        operator=( const SwDocDisplayItem& );
    void                        FillViewOptions( SwViewOption& rVOpt) const;
};

/*--------OS 12.01.95 -----------------------------------
Item for settings dialog, element page
--------------------------------------------------------- */
class SW_DLLPUBLIC SwElemItem : public SfxPoolItem
{
    //view
    bool bHorzScrollbar :1;
    bool bVertScrollbar :1;
    bool bAnyRuler : 1;
    bool bHorzRuler     :1;
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

    friend class SwContentOptPage;

public:
                            TYPEINFO_OVERRIDE();
                            SwElemItem( sal_uInt16 nWhich = FN_PARAM_ELEM );
                            SwElemItem(const SwElemItem& rElemItem);
                            SwElemItem(const SwViewOption& rVOpt, sal_uInt16 nWhich);

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const SAL_OVERRIDE;
    virtual bool            operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
    void                    operator=( const SwElemItem& );

    void                    FillViewOptions( SwViewOption& rVOpt) const;

};

/*--------OS 12.01.95 -----------------------------------
Item for settings dialog - printer/add ons
--------------------------------------------------------- */
class SW_DLLPUBLIC SwAddPrinterItem : public SfxPoolItem, public SwPrintData
{
    friend class SwAddPrinterTabPage;

    using  SwPrintData::operator ==;

public:
    TYPEINFO_OVERRIDE();
    SwAddPrinterItem( sal_uInt16 nWhich = FN_PARAM_ADDPRINTER );
    SwAddPrinterItem( sal_uInt16 nWhich, const SwPrintData& rPrtData );
    SwAddPrinterItem( const SwAddPrinterItem& rAddPrinterItem);

    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const SAL_OVERRIDE;

    virtual bool         operator==( const SfxPoolItem& ) const SAL_OVERRIDE;

    const OUString &GetFax() const              { return sFaxName; }
    void          SetFax( const OUString& rFax) { sFaxName = rFax; }

    sal_Bool   IsPrintProspect() const      { return bPrintProspect; }
    sal_Bool   IsPrintProspectRTL() const      { return bPrintProspectRTL; }
    void   SetPrintProspect(sal_Bool bFlag ){ bPrintProspect = bFlag; }
    void   SetPrintProspectRTL(sal_Bool bFlag ){ bPrintProspectRTL = bFlag; }
    sal_Bool IsPrintGraphic () const { return bPrintGraphic; }
    sal_Bool IsPrintTable () const { return bPrintTable; }
    sal_Bool IsPrintDraw () const { return bPrintDraw; }
    sal_Bool IsPrintControl () const { return bPrintControl; }
    sal_Bool IsPrintLeftPage () const { return bPrintLeftPages; }
    sal_Bool IsPrintRightPage() const { return bPrintRightPages; }
    sal_Bool IsPrintReverse  () const { return bPrintReverse; }
    sal_Bool IsPaperFromSetup() const { return bPaperFromSetup; }
    sal_Bool IsPrintEmptyPages() const { return bPrintEmptyPages; }
    sal_Bool IsPrintPageBackground() const { return bPrintPageBackground; }
    sal_Bool IsPrintBlackFont() const { return bPrintBlackFont; }
    //#i81434# - printing of hidden text
    sal_Bool IsPrintHiddenText() const { return bPrintHiddenText; }
    sal_Bool IsPrintTextPlaceholder() const { return bPrintTextPlaceholder; }

    sal_Bool IsPrintSingleJobs() const { return bPrintSingleJobs; }
    sal_uLong GetPrintPostIts () const { return nPrintPostIts; }

};

/*--------OS 12.01.95 -----------------------------------
Item for settings dialog, ShadowCursorPage
--------------------------------------------------------- */
class SW_DLLPUBLIC SwShadowCursorItem : public SfxPoolItem
{
    sal_uInt8 eMode;
    sal_Bool bOn;
public:
    TYPEINFO_OVERRIDE();
    SwShadowCursorItem( sal_uInt16 nWhich = FN_PARAM_SHADOWCURSOR );
    SwShadowCursorItem( const SwShadowCursorItem& rElemItem );
    SwShadowCursorItem( const SwViewOption& rVOpt, sal_uInt16 nWhich );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const SAL_OVERRIDE;
    virtual bool            operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
    void                    operator=( const SwShadowCursorItem& );

    void FillViewOptions( SwViewOption& rVOpt) const;

    sal_uInt8 GetMode() const               { return eMode; }
    sal_Bool IsOn() const                   { return bOn; }

    void SetMode( sal_uInt8 eM )            { eMode = eM; }
    void SetOn( sal_Bool bFlag )            { bOn = bFlag; }
};

#ifdef DBG_UTIL

/*--------OS 12.01.95 -----------------------------------
Item for settings dialog - test settings
--------------------------------------------------------- */
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
                            SwTestItem( sal_uInt16 _nWhich):
                                            SfxPoolItem(_nWhich){};
                            SwTestItem( const SwTestItem& pTestItem);

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const SAL_OVERRIDE;
    virtual bool            operator==( const SfxPoolItem& ) const SAL_OVERRIDE;

};
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
