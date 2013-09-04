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

#ifndef SC_TOKEN_HXX
#define SC_TOKEN_HXX

#include <memory>
#include <vector>
#include <boost/intrusive_ptr.hpp>

#include "formula/opcode.hxx"
#include "refdata.hxx"
#include <tools/mempool.hxx>
#include "scdllapi.h"
#include "formula/IFunctionDescription.hxx"
#include "formula/token.hxx"
#include "scmatrix.hxx"
#include "calcmacros.hxx"

// Matrix token constants.
#define MATRIX_TOKEN_HAS_RANGE 1

class ScJumpMatrix;

typedef ::std::vector< ScComplexRefData > ScRefList;

class SC_DLLPUBLIC ScToken : public formula::FormulaToken
{
private:
                                // not implemented, prevent usage
                                ScToken();
            ScToken&            operator=( const ScToken& );

protected:

    ScToken( formula::StackVar eTypeP,OpCode e = ocPush ) : formula::FormulaToken(eTypeP,e) {}
    ScToken( const ScToken& r ): formula::FormulaToken(r) {}

public:

    virtual                     ~ScToken();

    /**
        Dummy methods to avoid switches and casts where possible,
        the real token classes have to overload the appropriate method[s].
        The only methods valid anytime if not overloaded are:

        - GetByte() since this represents the count of parameters to a function
          which of course is 0 on non-functions. formula::FormulaByteToken and ScExternal do
          overload it.

        - HasForceArray() since also this is only used for operators and
          functions and is 0 for other tokens.

        Any other non-overloaded method pops up an assertion.
     */

    virtual const ScSingleRefData&    GetSingleRef() const;
    virtual ScSingleRefData&      GetSingleRef();
    virtual const ScComplexRefData& GetDoubleRef() const;
    virtual ScComplexRefData&       GetDoubleRef();
    virtual const ScSingleRefData&    GetSingleRef2() const;
    virtual ScSingleRefData&      GetSingleRef2();
    virtual const ScMatrix*     GetMatrix() const;
    virtual ScMatrix*           GetMatrix();
    virtual ScJumpMatrix*       GetJumpMatrix() const;
    virtual const ScRefList*    GetRefList() const;
    virtual       ScRefList*    GetRefList();

    virtual bool                TextEqual( const formula::FormulaToken& rToken ) const;
    virtual bool                Is3DRef() const;    // reference with 3D flag set

#if DEBUG_FORMULA_COMPILER
    virtual void Dump() const;
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
    static  formula::FormulaTokenRef          ExtendRangeReference( formula::FormulaToken & rTok1, formula::FormulaToken & rTok2, const ScAddress & rPos, bool bReuseDoubleRef );
};

inline void intrusive_ptr_add_ref(const ScToken* p)
{
    p->IncRef();
}

inline void intrusive_ptr_release(const ScToken* p)
{
    p->DecRef();
}

class ScSingleRefToken : public ScToken
{
private:
            ScSingleRefData       aSingleRef;
public:
                                ScSingleRefToken( const ScSingleRefData& r, OpCode e = ocPush ) :
                                    ScToken( formula::svSingleRef, e ), aSingleRef( r ) {}
                                ScSingleRefToken( const ScSingleRefToken& r ) :
                                    ScToken( r ), aSingleRef( r.aSingleRef ) {}
    virtual const ScSingleRefData&    GetSingleRef() const;
    virtual ScSingleRefData&      GetSingleRef();
    virtual bool                operator==( const formula::FormulaToken& rToken ) const;
    virtual FormulaToken*       Clone() const { return new ScSingleRefToken(*this); }

#if DEBUG_FORMULA_COMPILER
    virtual void Dump() const;
#endif
    DECL_FIXEDMEMPOOL_NEWDEL( ScSingleRefToken );
};

class ScDoubleRefToken : public ScToken
{
private:
            ScComplexRefData        aDoubleRef;
public:
                                ScDoubleRefToken( const ScComplexRefData& r, OpCode e = ocPush  ) :
                                    ScToken( formula::svDoubleRef, e ), aDoubleRef( r ) {}
                                ScDoubleRefToken( const ScSingleRefData& r, OpCode e = ocPush  ) :
                                    ScToken( formula::svDoubleRef, e )
                                {
                                    aDoubleRef.Ref1 = r;
                                    aDoubleRef.Ref2 = r;
                                }
                                ScDoubleRefToken( const ScDoubleRefToken& r ) :
                                    ScToken( r ), aDoubleRef( r.aDoubleRef ) {}
    virtual const ScSingleRefData&    GetSingleRef() const;
    virtual ScSingleRefData&      GetSingleRef();
    virtual const ScComplexRefData& GetDoubleRef() const;
    virtual ScComplexRefData&       GetDoubleRef();
    virtual const ScSingleRefData&    GetSingleRef2() const;
    virtual ScSingleRefData&      GetSingleRef2();
    virtual bool                operator==( const formula::FormulaToken& rToken ) const;
    virtual FormulaToken*       Clone() const { return new ScDoubleRefToken(*this); }

#if DEBUG_FORMULA_COMPILER
    virtual void Dump() const;
#endif
    DECL_FIXEDMEMPOOL_NEWDEL( ScDoubleRefToken );
};

class ScMatrixToken : public ScToken
{
private:
            ScMatrixRef         pMatrix;
public:
    ScMatrixToken( const ScMatrixRef& p );
    ScMatrixToken( const ScMatrixToken& r );

    virtual const ScMatrix*     GetMatrix() const;
    virtual ScMatrix*           GetMatrix();
    virtual bool                operator==( const formula::FormulaToken& rToken ) const;
    virtual FormulaToken*       Clone() const { return new ScMatrixToken(*this); }
};

/**
 * Token storing matrix that represents values in sheet range. It stores
 * both the values in matrix form, and the range address the matrix
 * represents.
 */
class ScMatrixRangeToken : public ScToken
{
    ScMatrixRef mpMatrix;
    ScComplexRefData maRef;
public:
    ScMatrixRangeToken( const ScMatrixRef& p, const ScComplexRefData& rRef );
    ScMatrixRangeToken( const ScMatrixRangeToken& r );

    virtual sal_uInt8 GetByte() const;
    virtual const ScMatrix* GetMatrix() const;
    virtual ScMatrix* GetMatrix();
    virtual const ScComplexRefData& GetDoubleRef() const;
    virtual ScComplexRefData& GetDoubleRef();
    virtual bool operator==( const formula::FormulaToken& rToken ) const;
    virtual FormulaToken* Clone() const;
};

class ScExternalSingleRefToken : public ScToken
{
private:
    sal_uInt16                  mnFileId;
    String                      maTabName;
    ScSingleRefData             maSingleRef;

                                ScExternalSingleRefToken(); // disabled
public:
                                ScExternalSingleRefToken( sal_uInt16 nFileId, const String& rTabName, const ScSingleRefData& r );
                                ScExternalSingleRefToken( const ScExternalSingleRefToken& r );
    virtual                     ~ScExternalSingleRefToken();

    virtual sal_uInt16                  GetIndex() const;
    virtual const String&           GetString() const;
    virtual const ScSingleRefData&  GetSingleRef() const;
    virtual ScSingleRefData&          GetSingleRef();
    virtual bool                operator==( const formula::FormulaToken& rToken ) const;
    virtual FormulaToken*       Clone() const { return new ScExternalSingleRefToken(*this); }
};


class ScExternalDoubleRefToken : public ScToken
{
private:
    sal_uInt16                  mnFileId;
    String                      maTabName;  // name of the first sheet
    ScComplexRefData            maDoubleRef;

                                ScExternalDoubleRefToken(); // disabled
public:
                                ScExternalDoubleRefToken( sal_uInt16 nFileId, const String& rTabName, const ScComplexRefData& r );
                                ScExternalDoubleRefToken( const ScExternalDoubleRefToken& r );
    virtual                     ~ScExternalDoubleRefToken();

    virtual sal_uInt16                 GetIndex() const;
    virtual const String&          GetString() const;
    virtual const ScSingleRefData& GetSingleRef() const;
    virtual ScSingleRefData&       GetSingleRef();
    virtual const ScSingleRefData& GetSingleRef2() const;
    virtual ScSingleRefData&       GetSingleRef2();
    virtual const ScComplexRefData&    GetDoubleRef() const;
    virtual ScComplexRefData&      GetDoubleRef();
    virtual bool                operator==( const formula::FormulaToken& rToken ) const;
    virtual FormulaToken*       Clone() const { return new ScExternalDoubleRefToken(*this); }
};

class ScExternalNameToken : public ScToken
{
private:
    sal_uInt16                  mnFileId;
    String                      maName;
private:
                                ScExternalNameToken(); // disabled
public:
                                ScExternalNameToken( sal_uInt16 nFileId, const String& rName );
                                ScExternalNameToken( const ScExternalNameToken& r );
    virtual                     ~ScExternalNameToken();
    virtual sal_uInt16              GetIndex() const;
    virtual const String&       GetString() const;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const;
    virtual FormulaToken*       Clone() const { return new ScExternalNameToken(*this); }
};


// Only created from within the interpreter, no conversion from ScRawToken,
// never added to ScTokenArray!
class ScJumpMatrixToken : public ScToken
{
private:
            ScJumpMatrix*       pJumpMatrix;
public:
                                ScJumpMatrixToken( ScJumpMatrix* p ) :
                                    ScToken( formula::svJumpMatrix ), pJumpMatrix( p ) {}
                                ScJumpMatrixToken( const ScJumpMatrixToken& r ) :
                                    ScToken( r ), pJumpMatrix( r.pJumpMatrix ) {}
    virtual                     ~ScJumpMatrixToken();
    virtual ScJumpMatrix*       GetJumpMatrix() const;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const;
    virtual FormulaToken*       Clone() const { return new ScJumpMatrixToken(*this); }
};


// Only created from within the interpreter, no conversion from ScRawToken,
// never added to ScTokenArray!
class ScRefListToken : public ScToken
{
private:
            ScRefList           aRefList;
public:
                                ScRefListToken() :
                                    ScToken( formula::svRefList ) {}
                                ScRefListToken( const ScRefListToken & r ) :
                                    ScToken( r ), aRefList( r.aRefList ) {}
    virtual const ScRefList*    GetRefList() const;
    virtual       ScRefList*    GetRefList();
    virtual bool                operator==( const formula::FormulaToken& rToken ) const;
    virtual FormulaToken*       Clone() const { return new ScRefListToken(*this); }
};


class SC_DLLPUBLIC ScEmptyCellToken : public ScToken
{
            bool                bInherited          :1;
            bool                bDisplayedAsString  :1;
public:
    explicit                    ScEmptyCellToken( bool bInheritedP, bool bDisplayAsString ) :
                                    ScToken( formula::svEmptyCell ),
                                    bInherited( bInheritedP ),
                                    bDisplayedAsString( bDisplayAsString ) {}
                                ScEmptyCellToken( const ScEmptyCellToken& r ) :
                                    ScToken( r ),
                                    bInherited( r.bInherited ),
                                    bDisplayedAsString( r.bDisplayedAsString ) {}
            bool                IsInherited() const { return bInherited; }
            bool                IsDisplayedAsString() const { return bDisplayedAsString; }
    virtual double              GetDouble() const;
    virtual const String &      GetString() const;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const;
    virtual FormulaToken*       Clone() const { return new ScEmptyCellToken(*this); }
};


/**  Transports the result from the interpreter to the formula cell. */
class SC_DLLPUBLIC ScMatrixCellResultToken : public ScToken
{
    // No non-const access implemented, silence down unxsols4 complaining about
    // the public GetMatrix() hiding the one from ScToken.
    virtual ScMatrix*           GetMatrix();

protected:
    ScConstMatrixRef xMatrix;
    formula::FormulaConstTokenRef     xUpperLeft;
public:
    ScMatrixCellResultToken( const ScConstMatrixRef& pMat, formula::FormulaToken* pUL );
    ScMatrixCellResultToken( const ScMatrixCellResultToken& r );
    virtual double              GetDouble() const;
    virtual const String &      GetString() const;
    virtual const ScMatrix*     GetMatrix() const;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const;
    virtual FormulaToken*       Clone() const;
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

    virtual bool                operator==( const formula::FormulaToken& rToken ) const;
    virtual FormulaToken*       Clone() const { return new ScMatrixFormulaCellToken(*this); }
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
};


class SC_DLLPUBLIC ScHybridCellToken : public ScToken
{
private:
    double mfDouble;
    String maString;
    OUString maFormula;
public:
    ScHybridCellToken( double f,
            const OUString & rStr,
            const OUString & rFormula ) :
        ScToken( formula::svHybridCell ),
        mfDouble( f ), maString( rStr ),
        maFormula( rFormula ) {}

    const OUString& GetFormula() const  { return maFormula; }
    virtual double GetDouble() const;
    virtual const String& GetString() const;
    virtual bool operator==( const formula::FormulaToken& rToken ) const;
    virtual FormulaToken* Clone() const { return new ScHybridCellToken(*this); }
};

// Simplify argument passing to RefUpdate methods with ScSingleRefToken or
// ScDoubleRefToken
class SingleDoubleRefModifier
{
    ScComplexRefData    aDub;
    ScSingleRefData*  pS;
    ScComplexRefData*   pD;

                // not implemented, prevent usage
                SingleDoubleRefModifier( const SingleDoubleRefModifier& );
        SingleDoubleRefModifier& operator=( const SingleDoubleRefModifier& );

public:
                SingleDoubleRefModifier( ScToken& rT )
                    {
                        formula::StackVar eType = rT.GetType();
                        if ( eType == formula::svSingleRef || eType == formula::svExternalSingleRef )
                        {
                            pS = &rT.GetSingleRef();
                            aDub.Ref1 = aDub.Ref2 = *pS;
                            pD = &aDub;
                        }
                        else
                        {
                            pS = 0;
                            pD = &rT.GetDoubleRef();
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

                SingleDoubleRefProvider( const ScToken& r )
                        : Ref1( r.GetSingleRef() ),
                        Ref2( (r.GetType() == formula::svDoubleRef ||
                                    r.GetType() == formula::svExternalDoubleRef) ?
                                r.GetDoubleRef().Ref2 : Ref1 )
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
