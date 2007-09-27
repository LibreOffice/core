/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cellfml.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 07:56:52 $
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

#ifndef _CELLFML_HXX
#define _CELLFML_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

class SwTable;
class SwNode;
class SwTableSortBoxes;
class SwSelBoxes;
class SwCalc;
class SwTableBox;
class SwTableFmlUpdate;
class SwDoc;
class String;

class SwTblCalcPara
{
    const SwTableBox* pLastTblBox;
    USHORT nStackCnt, nMaxSize;

public:
    SwTableSortBoxes *pBoxStk;  // Stack fuers erkennen von Rekursionen !
    SwCalc& rCalc;              // akt. Calculator
    const SwTable* pTbl;        // akt. Tabelle

    SwTblCalcPara( SwCalc& rCalculator, const SwTable& rTable );
    ~SwTblCalcPara();

    BOOL CalcWithStackOverflow();
    BOOL IsStackOverFlow() const        { return nMaxSize == nStackCnt; }
    BOOL IncStackCnt()                  { return nMaxSize == ++nStackCnt; }
    void DecStackCnt()                  { if( nStackCnt ) --nStackCnt; }
    void SetLastTblBox( const SwTableBox* pBox )    { pLastTblBox = pBox; }
};



class SwTableFormula
{
typedef void (SwTableFormula:: *FnScanFormel)( const SwTable&, String&,
                                            String&, String*, void* ) const;

    void BoxNmsToPtr( const SwTable&, String&, String&, String* = 0,
                        void* pPara = 0 ) const;
    void PtrToBoxNms( const SwTable&, String&, String&, String* = 0,
                        void* pPara = 0 ) const;
    void RelNmsToBoxNms( const SwTable&, String&, String&, String* = 0,
                        void* pPara = 0 ) const;
    void RelBoxNmsToPtr( const SwTable&, String&, String&, String* = 0,
                        void* pPara = 0 ) const;
    void BoxNmsToRelNm( const SwTable&, String&, String&, String* = 0,
                        void* pPara = 0 ) const;
    void _MakeFormel( const SwTable&, String&, String&, String* = 0,
                        void* pPara = 0 ) const;
    void _GetFmlBoxes( const SwTable&, String&, String&, String* = 0,
                        void* pPara = 0 ) const;
    void _HasValidBoxes( const SwTable&, String&, String&, String* = 0,
                        void* pPara = 0 ) const;
    void _SplitMergeBoxNm( const SwTable&, String&, String&, String* = 0,
                        void* pPara = 0 ) const;

    void GetBoxes( const SwTableBox& rStt, const SwTableBox& rEnd,
                    SwSelBoxes& rBoxes ) const;
    String ScanString( FnScanFormel fnFormel, const SwTable& rTbl,
                        void* = 0 ) const;

    const SwTable* FindTable( SwDoc& rDoc, const String& rNm ) const;

protected:
    enum NameType { EXTRNL_NAME, INTRNL_NAME, REL_NAME };

    String      sFormel;            // akt. Formel
    NameType    eNmType;            // akt. Darstellungs Art
    BOOL        bValidValue;        // TRUE: Formel neu berechnen

    // suche den Node, in dem die Formel steht:
    //  TextFeld    -> TextNode,
    //  BoxAttribut -> BoxStartNode
    // !!! MUSS VON JEDER ABLEITUNG UEBERLADEN WERDEN !!!
    virtual const SwNode* GetNodeOfFormula() const = 0;

    SwTableFormula( const String& rFormel );

    String MakeFormel( SwTblCalcPara& rCalcPara ) const
    {
        return ScanString( &SwTableFormula::_MakeFormel,
                            *rCalcPara.pTbl, &rCalcPara );
    }

    static USHORT GetLnPosInTbl( const SwTable& rTbl, const SwTableBox* pBox );

public:

    SwTableFormula( const SwTableFormula& rCpy )    { *this = rCpy; }
    virtual ~SwTableFormula();
    SwTableFormula& operator=( const SwTableFormula& rCpy )
        {
                                    sFormel = rCpy.sFormel;
                                    eNmType = rCpy.eNmType;
                                    bValidValue = rCpy.bValidValue;
                                    return *this;
        }

    // erzeuge aus der internen (fuer CORE) die externe (fuer UI) Formel
    void PtrToBoxNm( const SwTable* pTbl );
    // erzeuge aus der externen (fuer UI) die interne (fuer CORE) Formel
    void BoxNmToPtr( const SwTable* pTbl );
    // erzeuge aus der externen/internen Formel die relative Formel
    void ToRelBoxNm( const SwTable* pTbl );
    // wird vorm/nach dem mergen/splitten von Tabellen rerufen
    void ToSplitMergeBoxNm( SwTableFmlUpdate& rTblUpd );

    // ist gerade eine intern Darstellung aktiv
    BOOL IsIntrnlName() const           { return eNmType == INTRNL_NAME; }
    // erfrage die akt. Darstellung der Formel
    NameType GetNameType() const        { return eNmType; }

    // erfrage/setze das Flag, ob der akt. Wert gueltig ist
    BOOL        IsValid() const             { return bValidValue; }
    inline void ChgValid( BOOL bNew )       { bValidValue = bNew; }

    const String& GetFormula() const        { return sFormel; }
    void SetFormula( const String& rNew )
        {
            sFormel = rNew;
            bValidValue = FALSE;
            eNmType = EXTRNL_NAME;
        }

    USHORT GetBoxesOfFormula( const SwTable& rTbl, SwSelBoxes& rBoxes );
    // sind alle Boxen gueltig, auf die sich die Formel bezieht?
    BOOL HasValidBoxes() const;
};



#endif
