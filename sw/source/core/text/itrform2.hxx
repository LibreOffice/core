/*************************************************************************
 *
 *  $RCSfile: itrform2.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ama $ $Date: 2000-11-09 11:34:54 $
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
#ifndef _ITRFORM2_HXX
#define _ITRFORM2_HXX

#include "itrpaint.hxx"

class SwFlyCntPortion;
class SwInterHyphInfo;
class SwDropPortion;
class SwFmtDrop;
class SwDrawObjs;
class SwTxtAttr;
class SwNumberPortion;
class SwErgoSumPortion;
class SwExpandPortion;
class SwMultiPortion;

/*************************************************************************
 *                      class SwTxtFormatter
 *************************************************************************/

class SwTxtFormatter : public SwTxtPainter
{
    const SwFmtDrop *pDropFmt;
    SwMultiPortion* pMulti; // during formatting a multi-portion
    xub_StrLen nNextChg;
    sal_uInt8 nCntEndHyph;  // zaehlt aufeinanderfolgende Hyphens am Zeilenende
    sal_uInt8 nCntMidHyph;  // zaehlt aufeinanderfolgende Hyphens vor Flies
    sal_Bool bOnceMore : 1; // noch 'ne Runde?
    sal_Bool bFlyInCntBase : 1; // Base-Referenz der zeichengeb. Rahmen setzen
    sal_Bool bChanges : 1; // Flag, fuer die Berechnung des Repaint-Rechtecks
    sal_Bool bTruncLines : 1; // Flag, Repaint-Rechtecks ggf. erweitern
    sal_Bool bUnclipped : 1; // Flag, ob Repaint groesser als feste Zeilenhoehe
    SwLinePortion *NewPortion( SwTxtFormatInfo &rInf );
    SwTxtPortion  *NewTxtPortion( SwTxtFormatInfo &rInf );
    SwLinePortion *NewExtraPortion( SwTxtFormatInfo &rInf );
    SwTabPortion *NewTabPortion( SwTxtFormatInfo &rInf ) const;
    SwNumberPortion *NewNumberPortion( SwTxtFormatInfo &rInf ) const;
    SwDropPortion *NewDropPortion( SwTxtFormatInfo &rInf ) const;
    SwNumberPortion *NewFtnNumPortion( SwTxtFormatInfo &rInf ) const;
    SwErgoSumPortion *NewErgoSumPortion( SwTxtFormatInfo &rInf ) const;
    SwExpandPortion *NewFldPortion( SwTxtFormatInfo &rInf,
                                    const SwTxtAttr *pHt ) const;
    SwFtnPortion *NewFtnPortion( SwTxtFormatInfo &rInf, SwTxtAttr *pHt );
    SwFlyCntPortion *NewFlyCntPortion( SwTxtFormatInfo &rInf,
                                       SwTxtAttr *pHt ) const;
    SwLinePortion *WhichFirstPortion( SwTxtFormatInfo &rInf ) const;
    SwTxtPortion *WhichTxtPor( SwTxtFormatInfo &rInf ) const;

    // Das Herzstueck der Formatierung
    void BuildPortions( SwTxtFormatInfo &rInf );
    BOOL BuildMultiPortion( SwTxtFormatInfo &rInf, SwMultiPortion& rMulti );

    // Initialisierung oder Wiederverwertung alter Portions
    void Recycle( SwTxtFormatInfo &rInf );


    // Berechnung des emulierten rechten Rands
    void CalcFlyWidth( SwTxtFormatInfo &rInf ) const;

    // wird von SwTxtFormatter wegen UpdatePos ueberladen
    void CalcAdjustLine( SwLineLayout *pCurr );

    // uebertraegt die Daten nach rInf
    void FeedInf( SwTxtFormatInfo &rInf ) const;

    // behandelt die Unterlaufsituationen
    SwLinePortion *UnderFlow( SwTxtFormatInfo &rInf );

    // errechnet den Ascent und die Hoehe aus der Fontmetric
    void CalcAscent( SwTxtFormatInfo &rInf, SwLinePortion *pPor );

    // wird von Recycle() gerufen.
    void FormatReset( SwTxtFormatInfo &rInf );

    // Sind wir in der ersten zu formatierenden Zeile?
    inline sal_Bool IsFirstReformat() const
        { return GetInfo().GetIdx() < GetInfo().GetReformatStart(); }

    // durch das Adjustment aendert sich die Position der Portions
    void UpdatePos( SwLineLayout *pCurr, sal_Bool bAllWays = sal_False ) const;

    // Setze alle FlyInCntFrms auf die uebergebene BaseLine
    void AlignFlyInCntBase( long nBaseLine ) const;

    // Unterlaufbedingungen bei Flys
    sal_Bool ChkFlyUnderflow( SwTxtFormatInfo &rInf ) const;

    // Portion einfuegen.
    void InsertPortion( SwTxtFormatInfo &rInf, SwLinePortion *pPor ) const;

    // schaetzt die Hoehe fuer die DropPortion
    void GuessDropHeight( const MSHORT nLines );

public:
    // errechnet die Hoehe fuer die DropPortion
    void CalcDropHeight( const MSHORT nLines );

    // errechnet den Bottom des Absatzes, beruecksichtigt an diesem verankerte
    // Objekte mit Umlauf 1. Absatz.
    SwTwips CalcBottomLine() const;

    // Beruecksichtigt zeichengebundene Objekte bei der Repaintrechteck-
    // berechnung in Zeilen mit fester Zeilenhoehe
    void CalcUnclipped( SwTwips& rTop, SwTwips& rBottom );

    // u.a. fuer DropCaps
    sal_Bool CalcOnceMore();

    void CtorInit( SwTxtFrm *pFrm, SwTxtFormatInfo *pInf );
    inline SwTxtFormatter( SwTxtFrm *pFrm, SwTxtFormatInfo *pInf )
           { CtorInit( pFrm, pInf ); }
    ~SwTxtFormatter();

    xub_StrLen FormatLine( const xub_StrLen nStart );

    // Wir formatieren eine Zeile fuer die interaktive Trennung
    sal_Bool Hyphenate( SwInterHyphInfo &rInf );

    // Spezialmethode fuer QuoVadis-Texte
    // nErgo ist die Seitennummer der ErgoSum-Ftn
    // Bei 0 ist es noch unklar.
    xub_StrLen FormatQuoVadis( const xub_StrLen nStart );

    // Die Notbremse: Formatierung abbrechen, Zeile verwerfen.
    inline sal_Bool IsStop() const { return GetInfo().IsStop(); }

    // Das Gegenstueck: Formatierung unbedingt fortsetzen.
    inline sal_Bool IsNewLine() const { return GetInfo().IsNewLine(); }

    // FormatQuick(); auffrischen von Formatinformationen
    inline sal_Bool IsQuick() const { return GetInfo().IsQuick(); }

    // erzeugt ggfs. ein SwLineLayout, dass Ftn/Fly--Oszillation unterbindet.
    void MakeDummyLine();

    // SwTxtIter-Funktionalitaet
    void Insert( SwLineLayout *pLine );

    // die noch verbleibende Hoehe bis zum Seitenrand
    KSHORT GetFrmRstHeight() const;

    // Wie breit waerest Du ohne rechte Begrenzungen (Flys etc.)?
    KSHORT _CalcFitToContent( );

    SwLinePortion* MakeRestPortion(const SwLineLayout* pLine, xub_StrLen nPos);

    inline const SwFmtDrop *GetDropFmt() const { return pDropFmt; }
    inline void ClearDropFmt() { pDropFmt = 0; }

    inline SwMultiPortion *GetMulti() const { return pMulti; }

    inline const sal_Bool IsOnceMore() const { return bOnceMore; }
    inline void       SetOnceMore( sal_Bool bNew ) { bOnceMore = bNew; }

    inline const sal_Bool HasChanges() const { return bChanges; }
    inline void       SetChanges()       { bChanges = sal_True; }

    inline const sal_Bool HasTruncLines() const { return bTruncLines; }
    inline void       SetTruncLines( sal_Bool bNew ) { bTruncLines = bNew; }

    inline const sal_Bool IsUnclipped() const { return bUnclipped; }
    inline void       SetUnclipped( sal_Bool bNew ) { bUnclipped = bNew; }

    inline const sal_Bool IsFlyInCntBase() const { return bFlyInCntBase; }
    inline void  SetFlyInCntBase( sal_Bool bNew = sal_True ){ bFlyInCntBase = bNew; }

    inline SwTxtFormatInfo &GetInfo()
        { return (SwTxtFormatInfo&)SwTxtIter::GetInfo(); }
    inline const SwTxtFormatInfo &GetInfo() const
        { return (const SwTxtFormatInfo&)SwTxtIter::GetInfo(); }

    inline void InitCntHyph() { CntHyphens( nCntEndHyph, nCntMidHyph ); }
    inline const sal_uInt8 &CntEndHyph() const { return nCntEndHyph; }
    inline const sal_uInt8 &CntMidHyph() const { return nCntMidHyph; }
    inline sal_uInt8 &CntEndHyph() { return nCntEndHyph; }
    inline sal_uInt8 &CntMidHyph() { return nCntMidHyph; }
};



#endif
