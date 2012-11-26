/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
Item fuer Einstellungsdialog, Elementeseite
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
    sal_Bool bBigHandles    :1;
    //display
    sal_Bool bTable             :1;
    sal_Bool bGraphic           :1;
    sal_Bool bDrawing           :1;
    sal_Bool bFieldName         :1;
    sal_Bool bNotes             :1;

    friend class SwContentOptPage;

public:
                            SwElemItem( sal_uInt16 nWhich = FN_PARAM_ELEM );
                            SwElemItem(const SwElemItem& rElemItem);
                            SwElemItem(const SwViewOption& rVOpt, sal_uInt16 nWhich);


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
Item fuer Einstellungsdialog, ShadowCursorSeite
--------------------------------------------------------- */

class SW_DLLPUBLIC SwShadowCursorItem : public SfxPoolItem
{
    sal_uInt8 eMode;
    sal_Bool bOn;
public:
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
Item fuer Einstellungsdialog - Testeinstellungen
--------------------------------------------------------- */
class SW_DLLPUBLIC SwTestItem : public SfxPoolItem
{
    friend class SwModule;
    friend class SwWriterApp;
    friend class SwTestTabPage;
    friend class SwDocEditDialog;

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


