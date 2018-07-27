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

#ifndef INCLUDED_SW_INC_CELLFML_HXX
#define INCLUDED_SW_INC_CELLFML_HXX

#include <memory>
#include <rtl/ustring.hxx>

class SwTable;
class SwNode;
class SwTableSortBoxes;
class SwSelBoxes;
class SwCalc;
class SwTableBox;
class SwTableFormulaUpdate;
class SwDoc;

class SwTableCalcPara
{
    const SwTableBox* m_pLastTableBox;
    sal_uInt16 m_nStackCount, m_nMaxSize;

public:
    std::unique_ptr<SwTableSortBoxes> m_pBoxStack;  ///< stack for recognizing recursion
    SwCalc& m_rCalc;              ///< current Calculator
    const SwTable* m_pTable;        ///< current table

    SwTableCalcPara( SwCalc& rCalculator, const SwTable& rTable );
    ~SwTableCalcPara();

    bool CalcWithStackOverflow();
    bool IsStackOverflow() const        { return m_nMaxSize == m_nStackCount; }
    bool IncStackCnt()                  { return m_nMaxSize == ++m_nStackCount; }
    void DecStackCnt()                  { if( m_nStackCount ) --m_nStackCount; }
    void SetLastTableBox( const SwTableBox* pBox )    { m_pLastTableBox = pBox; }
};

class SwTableFormula
{
typedef void (SwTableFormula:: *FnScanFormula)( const SwTable&, OUString&,
                                                OUString&, OUString*, void* ) const;

    void BoxNmsToPtr( const SwTable&, OUString&, OUString&, OUString*,
                        void* pPara ) const;
    void PtrToBoxNms( const SwTable&, OUString&, OUString&, OUString*,
                        void* pPara ) const;
    void RelNmsToBoxNms( const SwTable&, OUString&, OUString&, OUString*,
                        void* pPara ) const;
    void RelBoxNmsToPtr( const SwTable&, OUString&, OUString&, OUString*,
                        void* pPara ) const;
    void BoxNmsToRelNm( const SwTable&, OUString&, OUString&, OUString*,
                        void* pPara ) const;
    void MakeFormula_( const SwTable&, OUString&, OUString&, OUString*,
                        void* pPara ) const;
    void GetFormulaBoxes( const SwTable&, OUString&, OUString&, OUString*,
                        void* pPara ) const;
    void HasValidBoxes_( const SwTable&, OUString&, OUString&, OUString*,
                        void* pPara ) const;
    void SplitMergeBoxNm_( const SwTable&, OUString&, OUString&, OUString*,
                        void* pPara ) const;

    static void GetBoxes( const SwTableBox& rStt, const SwTableBox& rEnd,
                    SwSelBoxes& rBoxes );
    OUString ScanString( FnScanFormula fnFormula, const SwTable& rTable,
                        void*) const;

    static const SwTable* FindTable( SwDoc& rDoc, const OUString& rNm );

protected:
    enum NameType { EXTRNL_NAME, INTRNL_NAME, REL_NAME };

    OUString    m_sFormula;         ///< current formula
    NameType    m_eNmType;          ///< current display method
    bool        m_bValidValue;      ///< true: recalculate formula

    // find the node in which the formula is located
    //  TextFeld    -> TextNode,
    //  BoxAttribut -> BoxStartNode
    // !!! every derived class must override this !!!
    virtual const SwNode* GetNodeOfFormula() const = 0;

    SwTableFormula( const OUString& rFormula );

    OUString MakeFormula( SwTableCalcPara& rCalcPara ) const
    {
        return ScanString( &SwTableFormula::MakeFormula_,
                            *rCalcPara.m_pTable, &rCalcPara );
    }

    static sal_uInt16 GetLnPosInTable( const SwTable& rTable, const SwTableBox* pBox );

public:

    virtual ~SwTableFormula();

    SwTableFormula(SwTableFormula const &) = default;
    SwTableFormula(SwTableFormula &&) = default;
    SwTableFormula & operator =(SwTableFormula const &) = default;
    SwTableFormula & operator =(SwTableFormula &&) = default;

    /// create from the internal formula (for CORE) the external formula (for UI)
    void PtrToBoxNm( const SwTable* pTable );
    /// create from the external formula the internal
    void BoxNmToPtr( const SwTable* pTable );
    /// create from the external/internal formula the relative formula
    void ToRelBoxNm( const SwTable* pTable );
    /// gets called before/after merging/splitting of tables
    void ToSplitMergeBoxNm( SwTableFormulaUpdate& rTableUpd );

    bool IsIntrnlName() const               { return m_eNmType == INTRNL_NAME; }
    NameType GetNameType() const            { return m_eNmType; }

    bool IsValid() const                    { return m_bValidValue; }
    void ChgValid( bool bNew )              { m_bValidValue = bNew; }

    const OUString& GetFormula() const             { return m_sFormula; }
    void SetFormula( const OUString& rNew )
    {
        m_sFormula = rNew;
        m_eNmType = EXTRNL_NAME;
        m_bValidValue = false;
    }

    void GetBoxesOfFormula(const SwTable& rTable, SwSelBoxes& rBoxes);
    // are all boxes valid which this formula relies on?
    bool HasValidBoxes() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
