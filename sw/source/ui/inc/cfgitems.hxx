/*************************************************************************
 *
 *  $RCSfile: cfgitems.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CFGITEMS_HXX
#define _CFGITEMS_HXX

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

#ifndef _AUTHRATR_HXX
#include <authratr.hxx>
#endif

class SwWriterApp;
class SwModule;
#ifndef PRODUCT
class SwTestTabPage;
#endif
class SwAddPrinterTabPage;
class SfxPrinter;
class ViewShell;
class SwViewOption;
class SwPrintOptions;
class SwContentOptPage;
class SwLayoutOptPage;
class SwDocEditDialog;

SfxPrinter* GetPrt( ViewShell* );
void        SetPrt( SfxPrinter* );


/*--------OS 12.01.95 -----------------------------------
Item fuer Einstellungsdialog - Dokumentanzeige
--------------------------------------------------------- */
#ifdef FN_PARAM_DOCDISP

class SwDocDisplayItem : public SfxPoolItem
{
    friend SwWriterApp;
    friend SwContentOptPage;
    friend SwModule;

    BOOL bParagraphEnd      :1;
    BOOL bTab               :1;
    BOOL bSpace             :1;
    BOOL bNonbreakingSpace  :1;
    BOOL bSoftHyphen        :1;
    BOOL bHiddenText        :1;
    BOOL bManualBreak       :1;
    BOOL bIndexEntry        :1;
    BOOL bIndexBackground   :1;
    BOOL bFootnoteBackground:1;
    BOOL bField             :1;
    BOOL bTable             :1;
    BOOL bGraphic           :1;
    BOOL bDrawing           :1;
    BOOL bFieldName         :1;
    BOOL bNotes             :1;
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
#endif
/*--------OS 12.01.95 -----------------------------------
Item fuer Einstellungsdialog, Elementeseite
--------------------------------------------------------- */
#ifdef FN_PARAM_ELEM
class SwElemItem : public SfxPoolItem
{
    BOOL bHorzScrollbar :1;
    BOOL bVertScrollbar :1;
    BOOL bHorzRuler     :1;
    BOOL bVertRuler     :1;
    BOOL bTableBounds   :1;
    BOOL bSectionBounds :1;
    BOOL bCrosshair     :1;
    BOOL bHandles       :1;
    BOOL bBigHandles    :1;
    BOOL bBounds        :1;
    BOOL bStatusLine    :1;
    BOOL bSmoothScroll  :1;

    friend SwLayoutOptPage;

public:
                            TYPEINFO();
                            SwElemItem( USHORT nWhich = FN_PARAM_ELEM );
                            SwElemItem(const SwElemItem& rElemItem);
                            SwElemItem(const SwViewOption& rVOpt, USHORT nWhich);


    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    void                    operator=( const SwElemItem& );

    void                    FillViewOptions( SwViewOption& rVOpt) const;

    void        SetStatusLine(BOOL bSet){bStatusLine = bSet;}
    BOOL        IsStatusLine()const     {return bStatusLine;}
};
#endif
/*--------OS 12.01.95 -----------------------------------
Item fuer Einstellungsdialog - Drucker/Zusaetze
--------------------------------------------------------- */
#ifdef FN_PARAM_ADDPRINTER

class SwAddPrinterItem : public SfxPoolItem
{
    friend  SwAddPrinterTabPage;

    String sFaxName;
    UINT16 nPrintPostIts;

    BOOL bPrintGraphic      :1,
         bPrintTable        :1,
         bPrintDrawing      :1,
         bPrintLeftPage     :1,
         bPrintRightPage    :1,
         bPrintControl      :1,
         bPrintReverse      :1,
         bPaperFromSetup    :1,
         bPrintProspect     :1,
         bPrintPageBackground:1,
         bPrintBlackFont    :1,
         bPrintSingleJobs   :1;

public:
    TYPEINFO();
    SwAddPrinterItem( USHORT nWhich = FN_PARAM_ADDPRINTER );
    SwAddPrinterItem( USHORT nWhich, SwPrintOptions* pPrtOpt );
    SwAddPrinterItem( const SwAddPrinterItem& rAddPrinterItem);

    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;
    virtual int          operator==( const SfxPoolItem& ) const;
    void          SetPrintOptions( SwPrintOptions* pPrtOpt ) const;
    const String &GetFax() const              { return sFaxName; }
    void          SetFax( const String& rFax) { sFaxName = rFax; }


    BOOL   IsPrintProspect() const      { return bPrintProspect; }
    void   SetPrintProspect(BOOL bFlag ){ bPrintProspect = bFlag; }
    BOOL IsPrintGraphic () const { return bPrintGraphic; }
    BOOL IsPrintTable () const { return bPrintTable; }
    BOOL IsPrintDraw () const { return bPrintDrawing; }
    BOOL IsPrintControl () const { return bPrintControl; }
    BOOL IsPrintLeftPage () const { return bPrintLeftPage; }
    BOOL IsPrintRightPage() const { return bPrintRightPage; }
    BOOL IsPrintReverse  () const { return bPrintReverse; }
    BOOL IsPaperFromSetup() const { return bPaperFromSetup; }
    BOOL IsPrintPageBackground() const { return bPrintPageBackground; }
    BOOL IsPrintBlackFont() const { return bPrintBlackFont; }
    BOOL IsPrintSingleJobs() const { return bPrintSingleJobs; }
    UINT16 GetPrintPostIts () const { return nPrintPostIts; }

};
#endif


/*--------OS 12.01.95 -----------------------------------
Item fuer Einstellungsdialog, ShadowCursorSeite
--------------------------------------------------------- */
#ifdef FN_PARAM_SHADOWCURSOR

class SwShadowCursorItem : public SfxPoolItem
{
    Color aColor;
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

    const Color& GetColor() const       { return aColor; }
    BYTE GetMode() const                { return eMode; }
    BOOL IsOn() const                   { return bOn; }

    void SetColor( const Color& rCol )  { aColor = rCol; }
    void SetMode( BYTE eM )             { eMode = eM; }
    void SetOn( BOOL bFlag )            { bOn = bFlag; }
};
#endif

#ifndef PRODUCT
/*--------OS 12.01.95 -----------------------------------
Item fuer Einstellungsdialog - Testeinstellungen
--------------------------------------------------------- */
class SwTestItem : public SfxPoolItem
{
    friend SwModule;
    friend SwWriterApp;
    friend SwTestTabPage;
    friend SwDocEditDialog;

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
                            SwTestItem( USHORT nWhich):
                                            SfxPoolItem(nWhich){};
                            SwTestItem( const SwTestItem& pTestItem);

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

};
#endif

#endif


