/*************************************************************************
 *
 *  $RCSfile: fmtclds.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:25 $
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
#ifndef _FMTCLDS_HXX
#define _FMTCLDS_HXX

#ifndef _SV_COLOR_HXX
#include <vcl/color.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _FORMAT_HXX //autogen
#include <format.hxx>
#endif

//Der ColumnDescriptor --------------------------

class SwColumn
{
    USHORT nWish;   //Wunschbreite incl. Raender.
                    //Verhaelt sich proportional zum Verhaeltniss:
                    //Wunschbreite der Umgebung / aktuelle Breite der Spalte
    USHORT nUpper;  //Oberer Rand
    USHORT nLower;  //Unterer Rand
    USHORT nLeft;   //Linker Rand
    USHORT nRight;  //Rechter Rand

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
SV_DECL_PTRARR_DEL( SwColumns, SwColumnPtr, 0, 2 );

enum SwColLineAdj
{
    COLADJ_NONE,
    COLADJ_TOP,
    COLADJ_CENTER,
    COLADJ_BOTTOM
};

class SwFmtCol : public SfxPoolItem
{
//  Pen      aPen;          //Pen fuer die Linine zwischen den Spalten
    ULONG   nLineWidth;     //width of the separator line
    Color   aLineColor;     //color of the separator line

    BYTE     nLineHeight;   //Prozentuale Hoehe der Linien
                            //(Relativ zu der Hoehe der Spalten incl. UL).
    SwColLineAdj eAdj;      //Linie wird oben, mittig oder unten ausgerichtet.

    SwColumns   aColumns;   //Informationen fuer die einzelnen Spalten.
    USHORT      nWidth;     //Gesamtwunschbreite aller Spalten.

    BOOL bOrtho;            //Nur wenn dieses Flag gesetzt ist wird beim setzen
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
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, USHORT nVer) const;
    virtual SvStream&       Store(SvStream &, USHORT nIVer) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const International* pIntl = 0 ) const;

    virtual BOOL             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual BOOL             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    const SwColumns &GetColumns() const { return aColumns; }
          SwColumns &GetColumns()       { return aColumns; }
    USHORT           GetNumCols() const { return aColumns.Count(); }
//  const Pen&       GetLinePen() const { return aPen; }
    ULONG           GetLineWidth() const  { return nLineWidth;}
    const Color&    GetLineColor() const { return aLineColor;}


    SwColLineAdj     GetLineAdj() const { return eAdj; }
    BOOL             IsOrtho()    const { return bOrtho; }
    USHORT           GetWishWidth() const { return nWidth; }
    BYTE             GetLineHeight()const { return nLineHeight; }

    //Return USHRT_MAX wenn uneindeutig.
    //Return die kleinste Breite wenn bMin True ist.
    USHORT GetGutterWidth( BOOL bMin = FALSE ) const;

//  void SetLinePen( const Pen& rNew )  { aPen = rNew; }
    void SetLineWidth(ULONG nWidth)         { nLineWidth = nWidth;}
    void SetLineColor(const Color& rCol )   { aLineColor = rCol;}
    void SetLineHeight( BYTE nNew )     { nLineHeight = nNew; }
    void SetLineAdj( SwColLineAdj eNew ){ eAdj = eNew; }
    void SetWishWidth( USHORT nNew )    { nWidth = nNew; }

    //Mit dieser Funktion koennen die Spalten (immer wieder) initialisert
    //werden. Das Ortho Flag wird automatisch gesetzt.
    void Init( USHORT nNumCols, USHORT nGutterWidth, USHORT nAct );

    //Stellt die Raender fuer die Spalten in aColumns ein.
    //Wenn das Flag bOrtho gesetzt ist, werden die Spalten neu optisch
    //verteilt. Ist das Flag nicht gesetzt werden die Spaltenbreiten nicht
    //veraendert und die Raender werden einfach eingestellt.
    void SetGutterWidth( USHORT nNew, USHORT nAct );

    //Verteilt ebenfalls automatisch neu wenn das Flag gesetzt wird;
    //nur dann wird auch der zweite Param. benoetigt und beachtet.
    void SetOrtho( BOOL bNew, USHORT nGutterWidth, USHORT nAct );

    //Fuer den Reader
    void _SetOrtho( BOOL bNew ) { bOrtho = bNew; }

    //Berechnet die aktuelle Breite der Spalte nCol.
    //Das Verhaeltniss von Wunschbreite der Spalte zum Returnwert ist
    //proportional zum Verhaeltniss des Gesamtwunschwertes zu nAct.
    USHORT CalcColWidth( USHORT nCol, USHORT nAct ) const;

    //Wie oben, aber es wir die Breite der PrtArea - also das was fuer
    //den Anwender die Spalte ist - geliefert.
    USHORT CalcPrtColWidth( USHORT nCol, USHORT nAct ) const;
};

inline const SwFmtCol &SwAttrSet::GetCol(BOOL bInP) const
    { return (const SwFmtCol&)Get( RES_COL,bInP); }

inline const SwFmtCol &SwFmt::GetCol(BOOL bInP) const
    { return aSet.GetCol(bInP); }

#endif

