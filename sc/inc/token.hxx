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

#include <vector>
#include <boost/intrusive_ptr.hpp>

#include <formula/opcode.hxx>
#include "refdata.hxx"
#include <tools/mempool.hxx>
#include "scdllapi.h"
#include <formula/IFunctionDescription.hxx>
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
                                ScSingleRefToken( const ScSingleRefToken& r ) :
                                    FormulaToken( r ), aSingleRef( r.aSingleRef ) {}
    virtual const ScSingleRefData*    GetSingleRef() const SAL_OVERRIDE;
    virtual ScSingleRefData*      GetSingleRef() SAL_OVERRIDE;
    virtual bool                TextEqual( const formula::FormulaToken& rToken ) const SAL_OVERRIDE;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const SAL_OVERRIDE;
    virtual FormulaToken*       Clone() const SAL_OVERRIDE { return new ScSingleRefToken(*this); }

    DECL_FIXEDMEMPOOL_NEWDEL( ScSingleRefToken );
};

class ScDoubleRefToken : public formula::FormulaToken
{
private:
            ScComplexRefData        aDoubleRef;
public:
                                ScDoubleRefToken( const ScComplexRefData& r, OpCode e = ocPush  ) :
                                    FormulaToken( formula::svDoubleRef, e ), aDoubleRef( r ) {}
                                ScDoubleRefToken( const ScSingleRefData& r, OpCode e = ocPush  ) :
                                    FormulaToken( formula::svDoubleRef, e )
                                {
                                    aDoubleRef.Ref1 = r;
                                    aDoubleRef.Ref2 = r;
                                }
                                ScDoubleRefToken( const ScDoubleRefToken& r ) :
                                    FormulaToken( r ), aDoubleRef( r.aDoubleRef ) {}
    virtual const ScSingleRefData*    GetSingleRef() const SAL_OVERRIDE;
    virtual ScSingleRefData*      GetSingleRef() SAL_OVERRIDE;
    virtual const ScComplexRefData* GetDoubleRef() const SAL_OVERRIDE;
    virtual ScComplexRefData*       GetDoubleRef() SAL_OVERRIDE;
    virtual const ScSingleRefData*    GetSingleRef2() const SAL_OVERRIDE;
    virtual ScSingleRefData*      GetSingleRef2() SAL_OVERRIDE;
    virtual bool                TextEqual( const formula::FormulaToken& rToken ) const SAL_OVERRIDE;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const SAL_OVERRIDE;
    virtual FormulaToken*       Clone() const SAL_OVERRIDE { return new ScDoubleRefToken(*this); }

    DECL_FIXEDMEMPOOL_NEWDEL( ScDoubleRefToken );
};

class ScMatrixToken : public formula::FormulaToken
{
private:
            ScMatrixRef         pMatrix;
public:
    ScMatrixToken( const ScMatrixRef& p );
    ScMatrixToken( const ScMatrixToken& r );

    virtual const ScMatrix*     GetMatrix() const SAL_OVERRIDE;
    virtual ScMatrix*           GetMatrix() SAL_OVERRIDE;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const SAL_OVERRIDE;
    virtual FormulaToken*       Clone() const SAL_OVERRIDE { return new ScMatrixToken(*this); }
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
    ScMatrixRangeToken( const ScMatrixRangeToken& r );

    virtual sal_uInt8 GetByte() const SAL_OVERRIDE;
    virtual const ScMatrix* GetMatrix() const SAL_OVERRIDE;
    virtual ScMatrix* GetMatrix() SAL_OVERRIDE;
    virtual const ScComplexRefData* GetDoubleRef() const SAL_OVERRIDE;
    virtual ScComplexRefData* GetDoubleRef() SAL_OVERRIDE;
    virtual bool operator==( const formula::FormulaToken& rToken ) const SAL_OVERRIDE;
    virtual FormulaToken* Clone() const SAL_OVERRIDE;
};

class ScExternalSingleRefToken : public formula::FormulaToken
{
    sal_uInt16                  mnFileId;
    svl::SharedString           maTabName;
    ScSingleRefData             maSingleRef;

    ScExternalSingleRefToken(); // disabled
public:
    ScExternalSingleRefToken( sal_uInt16 nFileId, const svl::SharedString& rTabName, const ScSingleRefData& r );
    ScExternalSingleRefToken( const ScExternalSingleRefToken& r );
    virtual ~ScExternalSingleRefToken();

    virtual sal_uInt16                  GetIndex() const SAL_OVERRIDE;
    virtual svl::SharedString GetString() const SAL_OVERRIDE;
    virtual const ScSingleRefData*  GetSingleRef() const SAL_OVERRIDE;
    virtual ScSingleRefData*          GetSingleRef() SAL_OVERRIDE;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const SAL_OVERRIDE;
    virtual FormulaToken*       Clone() const SAL_OVERRIDE { return new ScExternalSingleRefToken(*this); }
};

class ScExternalDoubleRefToken : public formula::FormulaToken
{
    sal_uInt16                  mnFileId;
    svl::SharedString           maTabName;  // name of the first sheet
    ScComplexRefData            maDoubleRef;

    ScExternalDoubleRefToken(); // disabled
public:
    ScExternalDoubleRefToken( sal_uInt16 nFileId, const svl::SharedString& rTabName, const ScComplexRefData& r );
    ScExternalDoubleRefToken( const ScExternalDoubleRefToken& r );
    virtual ~ScExternalDoubleRefToken();

    virtual sal_uInt16                 GetIndex() const SAL_OVERRIDE;
    virtual svl::SharedString GetString() const SAL_OVERRIDE;
    virtual const ScSingleRefData* GetSingleRef() const SAL_OVERRIDE;
    virtual ScSingleRefData*       GetSingleRef() SAL_OVERRIDE;
    virtual const ScSingleRefData* GetSingleRef2() const SAL_OVERRIDE;
    virtual ScSingleRefData*       GetSingleRef2() SAL_OVERRIDE;
    virtual const ScComplexRefData*    GetDoubleRef() const SAL_OVERRIDE;
    virtual ScComplexRefData*      GetDoubleRef() SAL_OVERRIDE;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const SAL_OVERRIDE;
    virtual FormulaToken*       Clone() const SAL_OVERRIDE { return new ScExternalDoubleRefToken(*this); }
};

class ScExternalNameToken : public formula::FormulaToken
{
    sal_uInt16                  mnFileId;
    svl::SharedString           maName;

    ScExternalNameToken(); // disabled
public:
    ScExternalNameToken( sal_uInt16 nFileId, const svl::SharedString& rName );
    ScExternalNameToken( const ScExternalNameToken& r );
    virtual ~ScExternalNameToken();

    virtual sal_uInt16              GetIndex() const SAL_OVERRIDE;
    virtual svl::SharedString GetString() const SAL_OVERRIDE;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const SAL_OVERRIDE;
    virtual FormulaToken*       Clone() const SAL_OVERRIDE { return new ScExternalNameToken(*this); }
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

    ScTableRefToken( sal_uInt16 nIndex, Item eItem );
    ScTableRefToken( const ScTableRefToken& r );
    virtual ~ScTableRefToken();

    virtual sal_uInt16          GetIndex() const SAL_OVERRIDE;
    virtual void                SetIndex( sal_uInt16 n ) SAL_OVERRIDE;
    virtual bool                IsGlobal() const SAL_OVERRIDE;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const SAL_OVERRIDE;
    virtual FormulaToken*       Clone() const SAL_OVERRIDE { return new ScTableRefToken(*this); }

            Item                GetItem() const;
            void                AddItem( Item );
            void                SetAreaRefRPN( formula::FormulaToken* pToken );
            formula::FormulaToken*  GetAreaRefRPN() const;

private:

    formula::FormulaTokenRef    mxAreaRefRPN;   ///< resulting RPN area
    sal_uInt16                  mnIndex;    ///< index into table / database range collection
    Item                        meItem;

    ScTableRefToken(); // disabled

};

// Only created from within the interpreter, no conversion from ScRawToken,
// never added to ScTokenArray!
class ScJumpMatrixToken : public formula::FormulaToken
{
private:
            ScJumpMatrix*       pJumpMatrix;
public:
                                ScJumpMatrixToken( ScJumpMatrix* p ) :
                                    FormulaToken( formula::svJumpMatrix ), pJumpMatrix( p ) {}
                                ScJumpMatrixToken( const ScJumpMatrixToken& r ) :
                                    FormulaToken( r ), pJumpMatrix( r.pJumpMatrix ) {}
    virtual                     ~ScJumpMatrixToken();
    virtual ScJumpMatrix*       GetJumpMatrix() const SAL_OVERRIDE;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const SAL_OVERRIDE;
    virtual FormulaToken*       Clone() const SAL_OVERRIDE { return new ScJumpMatrixToken(*this); }
};

// Only created from within the interpreter, no conversion from ScRawToken,
// never added to ScTokenArray!
class ScRefListToken : public formula::FormulaToken
{
private:
            ScRefList           aRefList;
public:
                                ScRefListToken() :
                                    FormulaToken( formula::svRefList ) {}
                                ScRefListToken( const ScRefListToken & r ) :
                                    FormulaToken( r ), aRefList( r.aRefList ) {}
    virtual const ScRefList*    GetRefList() const SAL_OVERRIDE;
    virtual       ScRefList*    GetRefList() SAL_OVERRIDE;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const SAL_OVERRIDE;
    virtual FormulaToken*       Clone() const SAL_OVERRIDE { return new ScRefListToken(*this); }
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
                                ScEmptyCellToken( const ScEmptyCellToken& r ) :
                                    FormulaToken( r ),
                                    bInherited( r.bInherited ),
                                    bDisplayedAsString( r.bDisplayedAsString ) {}
            bool                IsInherited() const { return bInherited; }
            bool                IsDisplayedAsString() const { return bDisplayedAsString; }
    virtual double              GetDouble() const SAL_OVERRIDE;
    virtual svl::SharedString GetString() const SAL_OVERRIDE;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const SAL_OVERRIDE;
    virtual FormulaToken*       Clone() const SAL_OVERRIDE { return new ScEmptyCellToken(*this); }
};

/**  Transports the result from the interpreter to the formula cell. */
class SC_DLLPUBLIC ScMatrixCellResultToken : public formula::FormulaToken
{
    // No non-const access implemented, silence down unxsols4 complaining about
    // the public GetMatrix() hiding the one from FormulaToken.
    virtual ScMatrix*           GetMatrix() SAL_OVERRIDE;

protected:
    ScConstMatrixRef xMatrix;
    formula::FormulaConstTokenRef     xUpperLeft;
public:
    ScMatrixCellResultToken( const ScConstMatrixRef& pMat, formula::FormulaToken* pUL );
    ScMatrixCellResultToken( const ScMatrixCellResultToken& r );
    virtual ~ScMatrixCellResultToken();
    virtual double              GetDouble() const SAL_OVERRIDE;
    virtual svl::SharedString GetString() const SAL_OVERRIDE;
    virtual const ScMatrix*     GetMatrix() const SAL_OVERRIDE;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const SAL_OVERRIDE;
    virtual FormulaToken*       Clone() const SAL_OVERRIDE;
    formula::StackVar           GetUpperLeftType() const
                                    {
                                        return xUpperLeft ?
                                            xUpperLeft->GetType() :
                                            static_cast<formula::StackVar>(formula::svUnknown);
                                    }
    inline formula::FormulaConstTokenRef     GetUpperLeftToken() const   { return xUpperLeft; }
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
    ScMatrixFormulaCellToken( SCCOL nC, SCROW nR, const ScConstMatrixRef& pMat, formula::FormulaToken* pUL );
    ScMatrixFormulaCellToken( SCCOL nC, SCROW nR );
    ScMatrixFormulaCellToken( const ScMatrixFormulaCellToken& r );
    virtual ~ScMatrixFormulaCellToken();

    virtual bool                operator==( const formula::FormulaToken& rToken ) const SAL_OVERRIDE;
    virtual FormulaToken*       Clone() const SAL_OVERRIDE { return new ScMatrixFormulaCellToken(*this); }
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
public:
    ScHybridCellToken(
        double f, const svl::SharedString & rStr, const OUString & rFormula );

    const OUString& GetFormula() const  { return maFormula; }
    virtual double GetDouble() const SAL_OVERRIDE;

    virtual svl::SharedString GetString() const SAL_OVERRIDE;
    virtual bool operator==( const formula::FormulaToken& rToken ) const SAL_OVERRIDE;
    virtual FormulaToken* Clone() const SAL_OVERRIDE { return new ScHybridCellToken(*this); }
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
                            pS = 0;
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
    inline  ScComplexRefData& Ref() { return *pD; }
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
                SingleDoubleRefProvider( const ScSingleRefData& r )
                        : Ref1( r ), Ref2( r )
                    {}
                SingleDoubleRefProvider( const ScComplexRefData& r )
                        : Ref1( r.Ref1 ), Ref2( r.Ref2 )
                    {}
                ~SingleDoubleRefProvider()
                    {}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
