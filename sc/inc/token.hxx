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

#ifndef INCLUDED_SC_INC_TOKEN_HXX
#define INCLUDED_SC_INC_TOKEN_HXX

#include <memory>
#include <vector>

#include <formula/opcode.hxx>
#include "refdata.hxx"
#include <tools/mempool.hxx>
#include "scdllapi.h"
#include <formula/token.hxx>
#include "calcmacros.hxx"
#include "types.hxx"

// Matrix token constants.
#define MATRIX_TOKEN_HAS_RANGE 1

namespace sc {

struct RangeMatrix;

}

class ScJumpMatrix;
class ScMatrix;

typedef ::std::vector< ScComplexRefData > ScRefList;

#if DEBUG_FORMULA_COMPILER
void DumpToken(formula::FormulaToken const & rToken);
#endif

/** If rTok1 and rTok2 both are SingleRef or DoubleRef tokens, extend/merge
    ranges as needed for ocRange.
    @param rPos
        The formula's position, used to calculate absolute positions from
        relative references.
    @param bReuseDoubleRef
        If true, a DoubleRef token is reused if passed as rTok1 or rTok2,
        else a new DoubleRef token is created and returned.
    @return
        A reused or new'ed ScDoubleRefToken, or a NULL TokenRef if rTok1 or
        rTok2 are not of sv(Single|Double)Ref
*/
formula::FormulaTokenRef extendRangeReference( formula::FormulaToken & rTok1, formula::FormulaToken & rTok2, const ScAddress & rPos, bool bReuseDoubleRef );

class ScSingleRefToken : public formula::FormulaToken
{
private:
            ScSingleRefData       aSingleRef;
public:
                                ScSingleRefToken( const ScSingleRefData& r, OpCode e = ocPush ) :
                                    FormulaToken( formula::svSingleRef, e ), aSingleRef( r ) {}
    virtual const ScSingleRefData*    GetSingleRef() const override;
    virtual ScSingleRefData*      GetSingleRef() override;
    virtual bool                TextEqual( const formula::FormulaToken& rToken ) const override;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const override;
    virtual FormulaToken*       Clone() const override { return new ScSingleRefToken(*this); }

    DECL_FIXEDMEMPOOL_NEWDEL( ScSingleRefToken );
};

class ScDoubleRefToken : public formula::FormulaToken
{
private:
            ScComplexRefData        aDoubleRef;
public:
                                ScDoubleRefToken( const ScComplexRefData& r, OpCode e = ocPush  ) :
                                    FormulaToken( formula::svDoubleRef, e ), aDoubleRef( r ) {}
    virtual const ScSingleRefData*    GetSingleRef() const override;
    virtual ScSingleRefData*      GetSingleRef() override;
    virtual const ScComplexRefData* GetDoubleRef() const override;
    virtual ScComplexRefData*       GetDoubleRef() override;
    virtual const ScSingleRefData*    GetSingleRef2() const override;
    virtual ScSingleRefData*      GetSingleRef2() override;
    virtual bool                TextEqual( const formula::FormulaToken& rToken ) const override;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const override;
    virtual FormulaToken*       Clone() const override { return new ScDoubleRefToken(*this); }

    DECL_FIXEDMEMPOOL_NEWDEL( ScDoubleRefToken );
};

class ScMatrixToken : public formula::FormulaToken
{
private:
            ScMatrixRef         pMatrix;
public:
    ScMatrixToken( const ScMatrixRef& p );
    ScMatrixToken( const ScMatrixToken& );

    virtual const ScMatrix*     GetMatrix() const override;
    virtual ScMatrix*           GetMatrix() override;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const override;
    virtual FormulaToken*       Clone() const override { return new ScMatrixToken(*this); }
};

/**
 * Token storing matrix that represents values in sheet range. It stores
 * both the values in matrix form, and the range address the matrix
 * represents.
 */
class ScMatrixRangeToken : public formula::FormulaToken
{
    ScMatrixRef mpMatrix;
    ScComplexRefData maRef;
public:
    ScMatrixRangeToken( const ScMatrixRef& p, const ScComplexRefData& rRef );
    ScMatrixRangeToken( const sc::RangeMatrix& rMat );
    ScMatrixRangeToken( const ScMatrixRangeToken& );

    virtual sal_uInt8 GetByte() const override;
    virtual const ScMatrix* GetMatrix() const override;
    virtual ScMatrix* GetMatrix() override;
    virtual const ScComplexRefData* GetDoubleRef() const override;
    virtual ScComplexRefData* GetDoubleRef() override;
    virtual bool operator==( const formula::FormulaToken& rToken ) const override;
    virtual FormulaToken* Clone() const override;
};

class ScExternalSingleRefToken : public formula::FormulaToken
{
    sal_uInt16                  mnFileId;
    svl::SharedString           maTabName;
    ScSingleRefData             maSingleRef;

public:
    ScExternalSingleRefToken( sal_uInt16 nFileId, const svl::SharedString& rTabName, const ScSingleRefData& r );
    ScExternalSingleRefToken() = delete;
    virtual ~ScExternalSingleRefToken() override;

    ScExternalSingleRefToken(ScExternalSingleRefToken const &) = default;
    ScExternalSingleRefToken(ScExternalSingleRefToken &&) = default;
    ScExternalSingleRefToken & operator =(ScExternalSingleRefToken const &) = default;
    ScExternalSingleRefToken & operator =(ScExternalSingleRefToken &&) = default;

    virtual sal_uInt16                  GetIndex() const override;
    virtual svl::SharedString GetString() const override;
    virtual const ScSingleRefData*  GetSingleRef() const override;
    virtual ScSingleRefData*          GetSingleRef() override;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const override;
    virtual FormulaToken*       Clone() const override { return new ScExternalSingleRefToken(*this); }
};

class ScExternalDoubleRefToken : public formula::FormulaToken
{
    sal_uInt16                  mnFileId;
    svl::SharedString           maTabName;  // name of the first sheet
    ScComplexRefData            maDoubleRef;

public:
    ScExternalDoubleRefToken() = delete;
    ScExternalDoubleRefToken( sal_uInt16 nFileId, const svl::SharedString& rTabName, const ScComplexRefData& r );
    virtual ~ScExternalDoubleRefToken() override;

    ScExternalDoubleRefToken(ScExternalDoubleRefToken const &) = default;
    ScExternalDoubleRefToken(ScExternalDoubleRefToken &&) = default;
    ScExternalDoubleRefToken & operator =(ScExternalDoubleRefToken const &) = default;
    ScExternalDoubleRefToken & operator =(ScExternalDoubleRefToken &&) = default;

    virtual sal_uInt16                 GetIndex() const override;
    virtual svl::SharedString GetString() const override;
    virtual const ScSingleRefData* GetSingleRef() const override;
    virtual ScSingleRefData*       GetSingleRef() override;
    virtual const ScSingleRefData* GetSingleRef2() const override;
    virtual ScSingleRefData*       GetSingleRef2() override;
    virtual const ScComplexRefData*    GetDoubleRef() const override;
    virtual ScComplexRefData*      GetDoubleRef() override;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const override;
    virtual FormulaToken*       Clone() const override { return new ScExternalDoubleRefToken(*this); }
};

class ScExternalNameToken : public formula::FormulaToken
{
    sal_uInt16                  mnFileId;
    svl::SharedString           maName;

public:
    ScExternalNameToken() = delete;
    ScExternalNameToken( sal_uInt16 nFileId, const svl::SharedString& rName );
    virtual ~ScExternalNameToken() override;

    ScExternalNameToken(ScExternalNameToken const &) = default;
    ScExternalNameToken(ScExternalNameToken &&) = default;
    ScExternalNameToken & operator =(ScExternalNameToken const &) = default;
    ScExternalNameToken & operator =(ScExternalNameToken &&) = default;

    virtual sal_uInt16              GetIndex() const override;
    virtual svl::SharedString GetString() const override;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const override;
    virtual FormulaToken*       Clone() const override { return new ScExternalNameToken(*this); }
};

/** Special token to remember details of ocTableRef "structured references". */
class ScTableRefToken : public formula::FormulaToken
{
public:

    enum Item
    {
        TABLE    =   0,
        ALL      =   1,
        HEADERS  =   2,
        DATA     =   4,
        TOTALS   =   8,
        THIS_ROW =  16,
        HEADERS_DATA = HEADERS | DATA,
        DATA_TOTALS = DATA | TOTALS
    };

    ScTableRefToken() = delete;
    ScTableRefToken( sal_uInt16 nIndex, Item eItem );
    ScTableRefToken( const ScTableRefToken& r );
    virtual ~ScTableRefToken() override;

    virtual sal_uInt16          GetIndex() const override;
    virtual void                SetIndex( sal_uInt16 n ) override;
    virtual sal_Int16           GetSheet() const override;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const override;
    virtual FormulaToken*       Clone() const override { return new ScTableRefToken(*this); }

            Item                GetItem() const;
            void                AddItem( Item );
            void                SetAreaRefRPN( formula::FormulaToken* pToken );
            formula::FormulaToken*  GetAreaRefRPN() const;

private:

    formula::FormulaTokenRef    mxAreaRefRPN;   ///< resulting RPN area
    sal_uInt16                  mnIndex;    ///< index into table / database range collection
    Item                        meItem;
};

// Only created from within the interpreter, no conversion from ScRawToken,
// never added to ScTokenArray!
class ScJumpMatrixToken : public formula::FormulaToken
{
private:
    std::shared_ptr<ScJumpMatrix> mpJumpMatrix;
public:
                                ScJumpMatrixToken( std::shared_ptr<ScJumpMatrix> p );
                                ScJumpMatrixToken( const ScJumpMatrixToken & );
    virtual                     ~ScJumpMatrixToken() override;
    virtual ScJumpMatrix*       GetJumpMatrix() const override;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const override;
    virtual FormulaToken*       Clone() const override { return new ScJumpMatrixToken(*this); }
};

// Only created from within the interpreter, no conversion from ScRawToken,
// never added to ScTokenArray!
class ScRefListToken : public formula::FormulaToken
{
private:
            ScRefList           aRefList;
            bool                mbArrayResult;  // whether RefList is an array result
public:
                                ScRefListToken() :
                                    FormulaToken( formula::svRefList ), mbArrayResult(false) {}
            explicit            ScRefListToken( bool bArrayResult ) :
                                    FormulaToken( formula::svRefList ), mbArrayResult( bArrayResult ) {}
            bool                IsArrayResult() const;
    virtual const ScRefList*    GetRefList() const override;
    virtual       ScRefList*    GetRefList() override;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const override;
    virtual FormulaToken*       Clone() const override { return new ScRefListToken(*this); }
};

class SC_DLLPUBLIC ScEmptyCellToken : public formula::FormulaToken
{
            bool                bInherited          :1;
            bool                bDisplayedAsString  :1;
public:
    explicit                    ScEmptyCellToken( bool bInheritedP, bool bDisplayAsString ) :
                                    FormulaToken( formula::svEmptyCell ),
                                    bInherited( bInheritedP ),
                                    bDisplayedAsString( bDisplayAsString ) {}
            bool                IsInherited() const { return bInherited; }
            bool                IsDisplayedAsString() const { return bDisplayedAsString; }
    virtual double              GetDouble() const override;
    virtual svl::SharedString GetString() const override;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const override;
    virtual FormulaToken*       Clone() const override { return new ScEmptyCellToken(*this); }
};

/**  Transports the result from the interpreter to the formula cell. */
class SC_DLLPUBLIC ScMatrixCellResultToken : public formula::FormulaToken
{
    // No non-const access implemented, silence down unxsols4 complaining about
    // the public GetMatrix() hiding the one from FormulaToken.
    virtual ScMatrix*           GetMatrix() override;

protected:
    ScConstMatrixRef xMatrix;
    formula::FormulaConstTokenRef     xUpperLeft;
public:
    ScMatrixCellResultToken( const ScConstMatrixRef& pMat, const formula::FormulaToken* pUL );
    ScMatrixCellResultToken( const ScMatrixCellResultToken& );
    virtual ~ScMatrixCellResultToken() override;
    virtual double              GetDouble() const override;
    virtual svl::SharedString GetString() const override;
    virtual const ScMatrix*     GetMatrix() const override;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const override;
    virtual FormulaToken*       Clone() const override;
    formula::StackVar           GetUpperLeftType() const
                                    {
                                        return xUpperLeft ?
                                            xUpperLeft->GetType() :
                                            formula::svUnknown;
                                    }
    const formula::FormulaConstTokenRef& GetUpperLeftToken() const { return xUpperLeft; }
    void Assign( const ScMatrixCellResultToken & r );
};

/** Stores the matrix result at the formula cell, additionally the range the
    matrix formula occupies. */
class SC_DLLPUBLIC ScMatrixFormulaCellToken : public ScMatrixCellResultToken
{
private:
            SCROW               nRows;
            SCCOL               nCols;
public:
    ScMatrixFormulaCellToken( SCCOL nC, SCROW nR, const ScConstMatrixRef& pMat, const formula::FormulaToken* pUL );
    ScMatrixFormulaCellToken( SCCOL nC, SCROW nR );
    ScMatrixFormulaCellToken( const ScMatrixFormulaCellToken& r );
    virtual ~ScMatrixFormulaCellToken() override;

    virtual bool                operator==( const formula::FormulaToken& rToken ) const override;
    virtual FormulaToken*       Clone() const override { return new ScMatrixFormulaCellToken(*this); }
            void                SetMatColsRows( SCCOL nC, SCROW nR )
                                    {
                                        nRows = nR;
                                        nCols = nC;
                                    }
            void                GetMatColsRows( SCCOL & nC, SCROW & nR ) const
                                    {
                                        nR = nRows;
                                        nC = nCols;
                                    }
            SCCOL               GetMatCols() const  { return nCols; }
            SCROW               GetMatRows() const  { return nRows; }

    /** Assign matrix result, keep matrix formula
        dimension. */
    void Assign( const ScMatrixCellResultToken & r );

                                /** Assign any result, keep matrix formula
                                    dimension. If token is of type
                                    ScMatrixCellResultToken uses the
                                    appropriate Assign() call, other tokens
                                    are assigned to xUpperLeft and xMatrix will
                                    be assigned NULL. */
            void                Assign( const formula::FormulaToken & r );

                                /** Modify xUpperLeft if formula::svDouble, or create
                                    new formula::FormulaDoubleToken if not set yet. Does
                                    nothing if xUpperLeft is of different type! */
            void                SetUpperLeftDouble( double f);

    /** Reset matrix and upper left, keep matrix
        formula dimension. */
    void ResetResult();

private:

    /** xUpperLeft is modifiable through SetUpperLeftDouble(), so clone it
        whenever an svDouble token is assigned to. */
    void CloneUpperLeftIfNecessary();
};

class SC_DLLPUBLIC ScHybridCellToken : public formula::FormulaToken
{
private:
    double mfDouble;
    svl::SharedString maString;
    OUString maFormula;
    bool mbEmptyDisplayedAsString;
public:
    ScHybridCellToken(
        double f, const svl::SharedString & rStr, const OUString & rFormula, bool bEmptyDisplayedAsString );

    const OUString& GetFormula() const  { return maFormula; }
    bool IsEmptyDisplayedAsString() const   { return mbEmptyDisplayedAsString; }
    virtual double GetDouble() const override;

    virtual svl::SharedString GetString() const override;
    virtual bool operator==( const formula::FormulaToken& rToken ) const override;
    virtual FormulaToken* Clone() const override { return new ScHybridCellToken(*this); }
};

// Simplify argument passing to RefUpdate methods with ScSingleRefToken or
// ScDoubleRefToken
class SingleDoubleRefModifier
{
    ScComplexRefData    aDub;
    ScSingleRefData*  pS;
    ScComplexRefData*   pD;

                SingleDoubleRefModifier( const SingleDoubleRefModifier& ) = delete;
        SingleDoubleRefModifier& operator=( const SingleDoubleRefModifier& ) = delete;

public:
        SingleDoubleRefModifier( formula::FormulaToken& rT )
                    {
                        formula::StackVar eType = rT.GetType();
                        if ( eType == formula::svSingleRef || eType == formula::svExternalSingleRef )
                        {
                            pS = rT.GetSingleRef();
                            aDub.Ref1 = aDub.Ref2 = *pS;
                            pD = &aDub;
                        }
                        else
                        {
                            pS = nullptr;
                            pD = rT.GetDoubleRef();
                            // aDub intentionally not initialized, unnecessary
                            // because unused.
                        }
                    }
                SingleDoubleRefModifier( ScSingleRefData& rS )
                    {
                        pS = &rS;
                        aDub.Ref1 = aDub.Ref2 = *pS;
                        pD = &aDub;
                    }
                ~SingleDoubleRefModifier()
                    {
                        if ( pS )
                            *pS = (*pD).Ref1;
                    }
    ScComplexRefData& Ref() { return *pD; }
};

class SingleDoubleRefProvider
{
public:

    const ScSingleRefData&    Ref1;
    const ScSingleRefData&    Ref2;

                SingleDoubleRefProvider( const formula::FormulaToken& r )
                        : Ref1( *r.GetSingleRef() ),
                        Ref2( (r.GetType() == formula::svDoubleRef ||
                                    r.GetType() == formula::svExternalDoubleRef) ?
                                r.GetDoubleRef()->Ref2 : Ref1 )
                    {}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
