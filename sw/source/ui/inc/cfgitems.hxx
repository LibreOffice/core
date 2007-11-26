/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cfgitems.hxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 17:33:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _CFGITEMS_HXX
#define _CFGITEMS_HXX

#include <tools/color.hxx>

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif
#ifndef _SW_PRINTDATA_HXX
#include <printdata.hxx>
#endif

#include <cmdid.h>

class SwWriterApp;
class SwModule;
#ifndef PRODUCT
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
    BOOL   IsPrintProspect_RTL() const      { return bPrintProspect_RTL; }
    void   SetPrintProspect(BOOL bFlag ){ bPrintProspect = bFlag; }
    void   SetPrintProspect_RTL(BOOL bFlag ){ bPrintProspect_RTL = bFlag; }
    BOOL IsPrintGraphic () const { return bPrintGraphic; }
    BOOL IsPrintTable () const { return bPrintTable; }
    BOOL IsPrintDraw () const { return bPrintDraw; }
    BOOL IsPrintControl () const { return bPrintControl; }
    BOOL IsPrintLeftPage () const { return bPrintLeftPage; }
    BOOL IsPrintRightPage() const { return bPrintRightPage; }
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

#ifndef PRODUCT
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


