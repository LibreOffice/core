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

#ifndef _CELLFML_HXX
#define _CELLFML_HXX

#include <tools/string.hxx>

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
    sal_uInt16 nStackCnt, nMaxSize;

public:
    SwTableSortBoxes *pBoxStk;  ///< stack for recognizing recursion
    SwCalc& rCalc;              ///< current Calculator
    const SwTable* pTbl;        ///< current table

    SwTblCalcPara( SwCalc& rCalculator, const SwTable& rTable );
    ~SwTblCalcPara();

    sal_Bool CalcWithStackOverflow();
    sal_Bool IsStackOverFlow() const        { return nMaxSize == nStackCnt; }
    sal_Bool IncStackCnt()                  { return nMaxSize == ++nStackCnt; }
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
    void _MakeFormula( const SwTable&, String&, String&, String* = 0,
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

    String      sFormel;            ///< current formula
    NameType    eNmType;            ///< current display method
    sal_Bool        bValidValue;        ///< sal_True: recalculate formula

    // find the node in which the formula is located
    //  TextFeld    -> TextNode,
    //  BoxAttribut -> BoxStartNode
    // !!! has to be overloaded by every derivation !!!
    virtual const SwNode* GetNodeOfFormula() const = 0;

    SwTableFormula( const String& rFormel );

    String MakeFormula( SwTblCalcPara& rCalcPara ) const
    {
        return ScanString( &SwTableFormula::_MakeFormula,
                            *rCalcPara.pTbl, &rCalcPara );
    }

    static sal_uInt16 GetLnPosInTbl( const SwTable& rTbl, const SwTableBox* pBox );

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

    /// create from the internal formula (for CORE) the external formula (for UI)
    void PtrToBoxNm( const SwTable* pTbl );
    /// create from the external formula the internal
    void BoxNmToPtr( const SwTable* pTbl );
    /// create from the external/internal formula the relative formula
    void ToRelBoxNm( const SwTable* pTbl );
    /// gets called before/after merging/splitting of tables
    void ToSplitMergeBoxNm( SwTableFmlUpdate& rTblUpd );

    bool IsIntrnlName() const                  { return eNmType == INTRNL_NAME; }
    NameType GetNameType() const        { return eNmType; }

    bool               IsValid() const                         { return bValidValue; }
    inline void        ChgValid( bool bNew )           { bValidValue = bNew; }

    const String& GetFormula() const        { return sFormel; }
    void SetFormula( const String& rNew )
        {
            sFormel = rNew;
            bValidValue = false;
            eNmType = EXTRNL_NAME;
        }

    void GetBoxesOfFormula(const SwTable& rTbl, SwSelBoxes& rBoxes);
    // are all boxes valid which this formula relies on?
    bool HasValidBoxes() const;
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
