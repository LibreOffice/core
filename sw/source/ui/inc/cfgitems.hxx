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

class SwWriterApp;
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
class SwDocEditDialog;

SfxPrinter* GetPrt( ViewShell* );
void        SetPrt( SfxPrinter* );

/*--------OS 12.01.95 -----------------------------------
Item fuer Einstellungsdialog - Dokumentanzeige
--------------------------------------------------------- */
class SW_DLLPUBLIC SwDocDisplayItem : public SfxPoolItem
{
    friend class SwWriterApp;
    friend class SwShdwCrsrOptionsTabPage;
    friend class SwModule;

    BOOL bParagraphEnd      :1;
    BOOL bTab               :1;
    BOOL bSpace             :1;
    BOOL bNonbreakingSpace  :1;
    BOOL bSoftHyphen        :1;
    BOOL bCharHiddenText    :1;
    BOOL bFldHiddenText     :1;
    BOOL bManualBreak       :1;
    BOOL bShowHiddenPara    :1;

    Color aIndexBackgrndCol;


public:
                                TYPEINFO();
                                SwDocDisplayItem( USHORT nWhich = FN_PARAM_DOCDISP );
                                SwDocDisplayItem(
                                    const SwDocDisplayItem& rSwDocDisplayItem );
                                SwDocDisplayItem( const SwViewOption& rVOpt,
                                                                USHORT nWhich );


    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;
    virtual int                 operator==( const SfxPoolItem& ) const;
    void                        operator=( const SwDocDisplayItem& );
    void                        FillViewOptions( SwViewOption& rVOpt) const;
};

/*--------OS 12.01.95 -----------------------------------
Item fuer Einstellungsdialog, Elementeseite
--------------------------------------------------------- */
class SW_DLLPUBLIC SwElemItem : public SfxPoolItem
{
    //view
    BOOL bHorzScrollbar :1;
    BOOL bVertScrollbar :1;
    BOOL bAnyRuler : 1;
    BOOL bHorzRuler     :1;
    BOOL bVertRuler     :1;
    BOOL bVertRulerRight:1;
    BOOL bSmoothScroll  :1;
    //visual aids
    BOOL bCrosshair     :1;
    BOOL bHandles       :1;
    BOOL bBigHandles    :1;
    //display
    BOOL bTable             :1;
    BOOL bGraphic           :1;
    BOOL bDrawing           :1;
    BOOL bFieldName         :1;
    BOOL bNotes             :1;

    friend class SwContentOptPage;

public:
                            TYPEINFO();
                            SwElemItem( USHORT nWhich = FN_PARAM_ELEM );
                            SwElemItem(const SwElemItem& rElemItem);
                            SwElemItem(const SwViewOption& rVOpt, USHORT nWhich);


    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    void                    operator=( const SwElemItem& );

    void                    FillViewOptions( SwViewOption& rVOpt) const;

};

/*--------OS 12.01.95 -----------------------------------
Item fuer Einstellungsdialog - Drucker/Zusaetze
--------------------------------------------------------- */
class SW_DLLPUBLIC SwAddPrinterItem : public SfxPoolItem, public SwPrintData
{
    friend class SwAddPrinterTabPage;

    using  SwPrintData::operator ==;

public:
    TYPEINFO();
    SwAddPrinterItem( USHORT nWhich = FN_PARAM_ADDPRINTER );
    SwAddPrinterItem( USHORT nWhich, const SwPrintData& rPrtData );
    SwAddPrinterItem( const SwAddPrinterItem& rAddPrinterItem);

    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;

    virtual int          operator==( const SfxPoolItem& ) const;

    const rtl::OUString &GetFax() const              { return sFaxName; }
    void          SetFax( const String& rFax) { sFaxName = rFax; }


    BOOL   IsPrintProspect() const      { return bPrintProspect; }
    BOOL   IsPrintProspectRTL() const      { return bPrintProspectRTL; }
    void   SetPrintProspect(BOOL bFlag ){ bPrintProspect = bFlag; }
    void   SetPrintProspectRTL(BOOL bFlag ){ bPrintProspectRTL = bFlag; }
    BOOL IsPrintGraphic () const { return bPrintGraphic; }
    BOOL IsPrintTable () const { return bPrintTable; }
    BOOL IsPrintDraw () const { return bPrintDraw; }
    BOOL IsPrintControl () const { return bPrintControl; }
    BOOL IsPrintLeftPage () const { return bPrintLeftPages; }
    BOOL IsPrintRightPage() const { return bPrintRightPages; }
    BOOL IsPrintReverse  () const { return bPrintReverse; }
    BOOL IsPaperFromSetup() const { return bPaperFromSetup; }
    BOOL IsPrintEmptyPages() const { return bPrintEmptyPages; }
    BOOL IsPrintPageBackground() const { return bPrintPageBackground; }
    BOOL IsPrintBlackFont() const { return bPrintBlackFont; }
    //#i81434# - printing of hidden text
    sal_Bool IsPrintHiddenText() const { return bPrintHiddenText; }
    sal_Bool IsPrintTextPlaceholder() const { return bPrintTextPlaceholder; }

    BOOL IsPrintSingleJobs() const { return bPrintSingleJobs; }
    ULONG GetPrintPostIts () const { return nPrintPostIts; }

};

/*--------OS 12.01.95 -----------------------------------
Item fuer Einstellungsdialog, ShadowCursorSeite
--------------------------------------------------------- */
class SW_DLLPUBLIC SwShadowCursorItem : public SfxPoolItem
{
    BYTE eMode;
    BOOL bOn;
public:
    TYPEINFO();
    SwShadowCursorItem( USHORT nWhich = FN_PARAM_SHADOWCURSOR );
    SwShadowCursorItem( const SwShadowCursorItem& rElemItem );
    SwShadowCursorItem( const SwViewOption& rVOpt, USHORT nWhich );


    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    void                    operator=( const SwShadowCursorItem& );

    void FillViewOptions( SwViewOption& rVOpt) const;

    BYTE GetMode() const                { return eMode; }
    BOOL IsOn() const                   { return bOn; }

    void SetMode( BYTE eM )             { eMode = eM; }
    void SetOn( BOOL bFlag )            { bOn = bFlag; }
};

#ifdef DBG_UTIL

/*--------OS 12.01.95 -----------------------------------
Item fuer Einstellungsdialog - Testeinstellungen
--------------------------------------------------------- */
class SW_DLLPUBLIC SwTestItem : public SfxPoolItem
{
    friend class SwModule;
    friend class SwWriterApp;
    friend class SwTestTabPage;
    friend class SwDocEditDialog;

    BOOL    bTest1:1;
    BOOL    bTest2:1;
    BOOL    bTest3:1;
    BOOL    bTest4:1;
    BOOL    bTest5:1;
    BOOL    bTest6:1;
    BOOL    bTest7:1;
    BOOL    bTest8:1;
    BOOL    bTest9:1;
    BOOL    bTest10:1;

public:
                            SwTestItem( USHORT _nWhich):
                                            SfxPoolItem(_nWhich){};
                            SwTestItem( const SwTestItem& pTestItem);

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

};
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
