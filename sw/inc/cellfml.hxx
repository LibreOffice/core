/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _CELLFML_HXX
#define _CELLFML_HXX

#include <rtl/ustring.hxx>

class SwTable;
class SwNode;
class SwTableSortBoxes;
class SwSelBoxes;
class SwCalc;
class SwTableBox;
class SwTableFmlUpdate;
class SwDoc;

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
typedef void (SwTableFormula:: *FnScanFormula)( const SwTable&, OUString&,
                                                OUString&, OUString*, void* ) const;

    void BoxNmsToPtr( const SwTable&, OUString&, OUString&, OUString* = 0,
                        void* pPara = 0 ) const;
    void PtrToBoxNms( const SwTable&, OUString&, OUString&, OUString* = 0,
                        void* pPara = 0 ) const;
    void RelNmsToBoxNms( const SwTable&, OUString&, OUString&, OUString* = 0,
                        void* pPara = 0 ) const;
    void RelBoxNmsToPtr( const SwTable&, OUString&, OUString&, OUString* = 0,
                        void* pPara = 0 ) const;
    void BoxNmsToRelNm( const SwTable&, OUString&, OUString&, OUString* = 0,
                        void* pPara = 0 ) const;
    void _MakeFormula( const SwTable&, OUString&, OUString&, OUString* = 0,
                        void* pPara = 0 ) const;
    void _GetFmlBoxes( const SwTable&, OUString&, OUString&, OUString* = 0,
                        void* pPara = 0 ) const;
    void _HasValidBoxes( const SwTable&, OUString&, OUString&, OUString* = 0,
                        void* pPara = 0 ) const;
    void _SplitMergeBoxNm( const SwTable&, OUString&, OUString&, OUString* = 0,
                        void* pPara = 0 ) const;

    void GetBoxes( const SwTableBox& rStt, const SwTableBox& rEnd,
                    SwSelBoxes& rBoxes ) const;
    OUString ScanString( FnScanFormula fnFormula, const SwTable& rTbl,
                        void* = 0 ) const;

    const SwTable* FindTable( SwDoc& rDoc, const OUString& rNm ) const;

protected:
    enum NameType { EXTRNL_NAME, INTRNL_NAME, REL_NAME };

    OUString    m_sFormula;         ///< current formula
    NameType    m_eNmType;          ///< current display method
    bool        m_bValidValue;      ///< true: recalculate formula

    // find the node in which the formula is located
    //  TextFeld    -> TextNode,
    //  BoxAttribut -> BoxStartNode
    // !!! has to be overloaded by every derivation !!!
    virtual const SwNode* GetNodeOfFormula() const = 0;

    SwTableFormula( const OUString& rFormula );

    OUString MakeFormula( SwTblCalcPara& rCalcPara ) const
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
        m_sFormula = rCpy.m_sFormula;
        m_eNmType = rCpy.m_eNmType;
        m_bValidValue = rCpy.m_bValidValue;
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

    bool IsIntrnlName() const               { return m_eNmType == INTRNL_NAME; }
    NameType GetNameType() const            { return m_eNmType; }

    bool IsValid() const                    { return m_bValidValue; }
    void ChgValid( bool bNew )              { m_bValidValue = bNew; }

    OUString GetFormula() const             { return m_sFormula; }
    void SetFormula( const OUString& rNew )
    {
        m_sFormula = rNew;
        m_eNmType = EXTRNL_NAME;
        m_bValidValue = false;
    }

    void GetBoxesOfFormula(const SwTable& rTbl, SwSelBoxes& rBoxes);
    // are all boxes valid which this formula relies on?
    bool HasValidBoxes() const;
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
