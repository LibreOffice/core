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

#ifndef _COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _FORMAT_HXX //autogen
#include <format.hxx>
#endif
namespace binfilter {

//Der ColumnDescriptor --------------------------

class SwColumn
{
    USHORT nWish;	//Wunschbreite incl. Raender.
                    //Verhaelt sich proportional zum Verhaeltniss:
                    //Wunschbreite der Umgebung / aktuelle Breite der Spalte
    USHORT nUpper;	//Oberer Rand
    USHORT nLower;	//Unterer Rand
    USHORT nLeft;	//Linker Rand
    USHORT nRight;	//Rechter Rand

public:
    SwColumn();

    BOOL operator==( const SwColumn & );


    void SetWishWidth( USHORT nNew ) { nWish  = nNew; }
    void SetUpper( USHORT  nNew ) { nUpper = nNew; }
    void SetLower( USHORT  nNew ) { nLower = nNew; }
    void SetLeft ( USHORT  nNew ) { nLeft  = nNew; }
    void SetRight( USHORT  nNew ) { nRight = nNew; }

    USHORT GetWishWidth() const { return nWish;  }
    USHORT GetUpper() const { return nUpper; }
    USHORT GetLower() const { return nLower; }
    USHORT GetLeft () const { return nLeft; }
    USHORT GetRight() const { return nRight; }
};

typedef SwColumn* SwColumnPtr;
SV_DECL_PTRARR_DEL( SwColumns, SwColumnPtr, 0, 2 )//STRIP008 ;

enum SwColLineAdj
{
    COLADJ_NONE,
    COLADJ_TOP,
    COLADJ_CENTER,
    COLADJ_BOTTOM
};

class SwFmtCol : public SfxPoolItem
{
//	Pen		 aPen;			//Pen fuer die Linine zwischen den Spalten
    ULONG	nLineWidth;		//width of the separator line
    Color	aLineColor;		//color of the separator line

    BYTE 	 nLineHeight;	//Prozentuale Hoehe der Linien
                            //(Relativ zu der Hoehe der Spalten incl. UL).
    SwColLineAdj eAdj;		//Linie wird oben, mittig oder unten ausgerichtet.

    SwColumns	aColumns;	//Informationen fuer die einzelnen Spalten.
    USHORT		nWidth;		//Gesamtwunschbreite aller Spalten.

    BOOL bOrtho;			//Nur wenn dieses Flag gesetzt ist wird beim setzen
                            //der GutterWidth eine 'optische Verteilung'
                            //vorgenommen.
                            //Es muss zurueckgesetzt werden wenn an den
                            //Spaltenbreiten bzw. den Raendern gedreht wird.
                            //Wenn es wieder gesetzt wird wird automatisch neu
                            //gemischt (optisch verteilt).
                            //Das Flag ist initial gesetzt.

    void Calc( USHORT nGutterWidth, USHORT nAct );

public:
    SwFmtCol();
    SwFmtCol( const SwFmtCol& );
    ~SwFmtCol();

    SwFmtCol& operator=( const SwFmtCol& );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*	Create(SvStream &, USHORT nVer) const;
    virtual SvStream&		Store(SvStream &, USHORT nIVer) const;

    virtual	BOOL        	 QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	BOOL			 PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    const SwColumns &GetColumns() const { return aColumns; }
          SwColumns &GetColumns()		{ return aColumns; }
    USHORT			 GetNumCols() const { return aColumns.Count(); }
//	const Pen&       GetLinePen() const { return aPen; }
    ULONG			GetLineWidth() const  { return nLineWidth;}
    const Color&	GetLineColor() const { return aLineColor;}


    SwColLineAdj	 GetLineAdj() const { return eAdj; }
    BOOL			 IsOrtho()	  const { return bOrtho; }
    USHORT			 GetWishWidth() const { return nWidth; }
    BYTE			 GetLineHeight()const { return nLineHeight; }

    //Return USHRT_MAX wenn uneindeutig.
    //Return die kleinste Breite wenn bMin True ist.
    USHORT GetGutterWidth( BOOL bMin = FALSE ) const;

//	void SetLinePen( const Pen& rNew )  { aPen = rNew; }
    void SetLineWidth(ULONG nWidth)   		{ nLineWidth = nWidth;}
    void SetLineColor(const Color& rCol )  	{ aLineColor = rCol;}
    void SetLineHeight( BYTE nNew )     { nLineHeight = nNew; }
    void SetLineAdj( SwColLineAdj eNew ){ eAdj = eNew; }
    void SetWishWidth( USHORT nNew )	{ nWidth = nNew; }

    //Mit dieser Funktion koennen die Spalten (immer wieder) initialisert
    //werden. Das Ortho Flag wird automatisch gesetzt.
    void Init( USHORT nNumCols, USHORT nGutterWidth, USHORT nAct );

    //Stellt die Raender fuer die Spalten in aColumns ein.
    //Wenn das Flag bOrtho gesetzt ist, werden die Spalten neu optisch
    //verteilt. Ist das Flag nicht gesetzt werden die Spaltenbreiten nicht
    //veraendert und die Raender werden einfach eingestellt.

    //Verteilt ebenfalls automatisch neu wenn das Flag gesetzt wird;
    //nur dann wird auch der zweite Param. benoetigt und beachtet.

    //Fuer den Reader
    void _SetOrtho( BOOL bNew ) { bOrtho = bNew; }

    //Berechnet die aktuelle Breite der Spalte nCol.
    //Das Verhaeltniss von Wunschbreite der Spalte zum Returnwert ist
    //proportional zum Verhaeltniss des Gesamtwunschwertes zu nAct.
    USHORT CalcColWidth( USHORT nCol, USHORT nAct ) const;

    //Wie oben, aber es wir die Breite der PrtArea - also das was fuer
    //den Anwender die Spalte ist - geliefert.
};

#if !(defined(MACOSX) && ( __GNUC__ < 3 ))
// GrP moved to gcc_outl.cxx; revisit with gcc3
inline const SwFmtCol &SwAttrSet::GetCol(BOOL bInP) const
    { return (const SwFmtCol&)Get( RES_COL,bInP); }

inline const SwFmtCol &SwFmt::GetCol(BOOL bInP) const
    { return aSet.GetCol(bInP); }
#endif

} //namespace binfilter
#endif

