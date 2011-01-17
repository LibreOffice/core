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
#ifndef _FMTCLDS_HXX
#define _FMTCLDS_HXX

#include <tools/color.hxx>
#include <svl/poolitem.hxx>
#include "swdllapi.h"
#include <hintids.hxx>
#include <format.hxx>

//Der ColumnDescriptor --------------------------

class SwColumn
{
    sal_uInt16 nWish;   //Wunschbreite incl. Raender.
                    //Verhaelt sich proportional zum Verhaeltniss:
                    //Wunschbreite der Umgebung / aktuelle Breite der Spalte
    sal_uInt16 nUpper;  //Oberer Rand
    sal_uInt16 nLower;  //Unterer Rand
    sal_uInt16 nLeft;   //Linker Rand
    sal_uInt16 nRight;  //Rechter Rand

public:
    SwColumn();

    sal_Bool operator==( const SwColumn & );


    void SetWishWidth( sal_uInt16 nNew ) { nWish  = nNew; }
    void SetUpper( sal_uInt16  nNew ) { nUpper = nNew; }
    void SetLower( sal_uInt16  nNew ) { nLower = nNew; }
    void SetLeft ( sal_uInt16  nNew ) { nLeft  = nNew; }
    void SetRight( sal_uInt16  nNew ) { nRight = nNew; }

    sal_uInt16 GetWishWidth() const { return nWish;  }
    sal_uInt16 GetUpper() const { return nUpper; }
    sal_uInt16 GetLower() const { return nLower; }
    sal_uInt16 GetLeft () const { return nLeft; }
    sal_uInt16 GetRight() const { return nRight; }
};

typedef SwColumn* SwColumnPtr;
SV_DECL_PTRARR_DEL( SwColumns, SwColumnPtr, 0, 2 )

enum SwColLineAdj
{
    COLADJ_NONE,
    COLADJ_TOP,
    COLADJ_CENTER,
    COLADJ_BOTTOM
};

class SW_DLLPUBLIC SwFmtCol : public SfxPoolItem
{
//  Pen      aPen;          //Pen fuer die Linine zwischen den Spalten
    sal_uLong   nLineWidth;     //width of the separator line
    Color   aLineColor;     //color of the separator line

    sal_uInt8    nLineHeight;   //Prozentuale Hoehe der Linien
                            //(Relativ zu der Hoehe der Spalten incl. UL).
    SwColLineAdj eAdj;      //Linie wird oben, mittig oder unten ausgerichtet.

    SwColumns   aColumns;   //Informationen fuer die einzelnen Spalten.
    sal_uInt16      nWidth;     //Gesamtwunschbreite aller Spalten.

    sal_Bool bOrtho;            //Nur wenn dieses Flag gesetzt ist wird beim setzen
                            //der GutterWidth eine 'optische Verteilung'
                            //vorgenommen.
                            //Es muss zurueckgesetzt werden wenn an den
                            //Spaltenbreiten bzw. den Raendern gedreht wird.
                            //Wenn es wieder gesetzt wird wird automatisch neu
                            //gemischt (optisch verteilt).
                            //Das Flag ist initial gesetzt.

    SW_DLLPRIVATE void Calc( sal_uInt16 nGutterWidth, sal_uInt16 nAct );

public:
    SwFmtCol();
    SwFmtCol( const SwFmtCol& );
    ~SwFmtCol();

    SwFmtCol& operator=( const SwFmtCol& );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper* pIntl = 0 ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    const SwColumns &GetColumns() const { return aColumns; }
          SwColumns &GetColumns()       { return aColumns; }
    sal_uInt16           GetNumCols() const { return aColumns.Count(); }
//  const Pen&       GetLinePen() const { return aPen; }
    sal_uLong           GetLineWidth() const  { return nLineWidth;}
    const Color&    GetLineColor() const { return aLineColor;}


    SwColLineAdj     GetLineAdj() const { return eAdj; }
    sal_Bool             IsOrtho()    const { return bOrtho; }
    sal_uInt16           GetWishWidth() const { return nWidth; }
    sal_uInt8            GetLineHeight()const { return nLineHeight; }

    //Return USHRT_MAX wenn uneindeutig.
    //Return die kleinste Breite wenn bMin True ist.
    sal_uInt16 GetGutterWidth( sal_Bool bMin = sal_False ) const;

//  void SetLinePen( const Pen& rNew )  { aPen = rNew; }
    void SetLineWidth(sal_uLong nLWidth)        { nLineWidth = nLWidth;}
    void SetLineColor(const Color& rCol )   { aLineColor = rCol;}
    void SetLineHeight( sal_uInt8 nNew )     { nLineHeight = nNew; }
    void SetLineAdj( SwColLineAdj eNew ){ eAdj = eNew; }
    void SetWishWidth( sal_uInt16 nNew )    { nWidth = nNew; }

    //Mit dieser Funktion koennen die Spalten (immer wieder) initialisert
    //werden. Das Ortho Flag wird automatisch gesetzt.
    void Init( sal_uInt16 nNumCols, sal_uInt16 nGutterWidth, sal_uInt16 nAct );

    //Stellt die Raender fuer die Spalten in aColumns ein.
    //Wenn das Flag bOrtho gesetzt ist, werden die Spalten neu optisch
    //verteilt. Ist das Flag nicht gesetzt werden die Spaltenbreiten nicht
    //veraendert und die Raender werden einfach eingestellt.
    void SetGutterWidth( sal_uInt16 nNew, sal_uInt16 nAct );

    //Verteilt ebenfalls automatisch neu wenn das Flag gesetzt wird;
    //nur dann wird auch der zweite Param. benoetigt und beachtet.
    void SetOrtho( sal_Bool bNew, sal_uInt16 nGutterWidth, sal_uInt16 nAct );

    //Fuer den Reader
    void _SetOrtho( sal_Bool bNew ) { bOrtho = bNew; }

    //Berechnet die aktuelle Breite der Spalte nCol.
    //Das Verhaeltniss von Wunschbreite der Spalte zum Returnwert ist
    //proportional zum Verhaeltniss des Gesamtwunschwertes zu nAct.
    sal_uInt16 CalcColWidth( sal_uInt16 nCol, sal_uInt16 nAct ) const;

    //Wie oben, aber es wir die Breite der PrtArea - also das was fuer
    //den Anwender die Spalte ist - geliefert.
    sal_uInt16 CalcPrtColWidth( sal_uInt16 nCol, sal_uInt16 nAct ) const;
};

inline const SwFmtCol &SwAttrSet::GetCol(sal_Bool bInP) const
    { return (const SwFmtCol&)Get( RES_COL,bInP); }

inline const SwFmtCol &SwFmt::GetCol(sal_Bool bInP) const
    { return aSet.GetCol(bInP); }

#endif

