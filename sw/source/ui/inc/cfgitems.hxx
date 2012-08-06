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
#ifndef _CFGITEMS_HXX
#define _CFGITEMS_HXX

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
class ViewShell;
class SwViewOption;
class SwContentOptPage;
class SwShdwCrsrOptionsTabPage;

SfxPrinter* GetPrt( ViewShell* );
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
                                TYPEINFO();
                                SwDocDisplayItem( sal_uInt16 nWhich = FN_PARAM_DOCDISP );
                                SwDocDisplayItem(
                                    const SwDocDisplayItem& rSwDocDisplayItem );
                                SwDocDisplayItem( const SwViewOption& rVOpt,
                                                                sal_uInt16 nWhich );


    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;
    virtual int                 operator==( const SfxPoolItem& ) const;
    void                        operator=( const SwDocDisplayItem& );
    void                        FillViewOptions( SwViewOption& rVOpt) const;
};

/*--------OS 12.01.95 -----------------------------------
Item for settings dialog, element page
--------------------------------------------------------- */
class SW_DLLPUBLIC SwElemItem : public SfxPoolItem
{
    //view
    sal_Bool bHorzScrollbar :1;
    sal_Bool bVertScrollbar :1;
    sal_Bool bAnyRuler : 1;
    sal_Bool bHorzRuler     :1;
    sal_Bool bVertRuler     :1;
    sal_Bool bVertRulerRight:1;
    sal_Bool bSmoothScroll  :1;
    //visual aids
    sal_Bool bCrosshair     :1;
    sal_Bool bHandles       :1;
    //display
    sal_Bool bTable             :1;
    sal_Bool bGraphic           :1;
    sal_Bool bDrawing           :1;
    sal_Bool bFieldName         :1;
    sal_Bool bNotes             :1;

    friend class SwContentOptPage;

public:
                            TYPEINFO();
                            SwElemItem( sal_uInt16 nWhich = FN_PARAM_ELEM );
                            SwElemItem(const SwElemItem& rElemItem);
                            SwElemItem(const SwViewOption& rVOpt, sal_uInt16 nWhich);


    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
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
    TYPEINFO();
    SwAddPrinterItem( sal_uInt16 nWhich = FN_PARAM_ADDPRINTER );
    SwAddPrinterItem( sal_uInt16 nWhich, const SwPrintData& rPrtData );
    SwAddPrinterItem( const SwAddPrinterItem& rAddPrinterItem);

    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;

    virtual int          operator==( const SfxPoolItem& ) const;

    const rtl::OUString &GetFax() const              { return sFaxName; }
    void          SetFax( const String& rFax) { sFaxName = rFax; }


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
    TYPEINFO();
    SwShadowCursorItem( sal_uInt16 nWhich = FN_PARAM_SHADOWCURSOR );
    SwShadowCursorItem( const SwShadowCursorItem& rElemItem );
    SwShadowCursorItem( const SwViewOption& rVOpt, sal_uInt16 nWhich );


    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
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

    sal_Bool    bTest1:1;
    sal_Bool    bTest2:1;
    sal_Bool    bTest3:1;
    sal_Bool    bTest4:1;
    sal_Bool    bTest5:1;
    sal_Bool    bTest6:1;
    sal_Bool    bTest7:1;
    sal_Bool    bTest8:1;
    sal_Bool    bTest9:1;
    sal_Bool    bTest10:1;

public:
                            SwTestItem( sal_uInt16 _nWhich):
                                            SfxPoolItem(_nWhich){};
                            SwTestItem( const SwTestItem& pTestItem);

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

};
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
