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

#ifndef SC_TOKEN_HXX
#define SC_TOKEN_HXX

#include <memory>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "formula/opcode.hxx"
#include "refdata.hxx"
#include "scmatrix.hxx"
#include "formula/intruref.hxx"
#include <tools/mempool.hxx>
#include "scdllapi.h"
#include "formula/IFunctionDescription.hxx"
#include "formula/token.hxx"


class ScJumpMatrix;
class ScToken;

typedef ::std::vector< ScComplexRefData > ScRefList;
typedef formula::SimpleIntrusiveReference< class ScToken > ScTokenRef;

/**
 * Another ref-counted token type using shared_ptr.  <b>Be extra careful
 * not to mix use of this smart pointer type with ScTokenRef</b>, since
 * mixing them might cause a premature object deletion because the same
 * object may be ref-counted by two different smart pointer wrappers.
 *
 * You have been warned.
 */
typedef ::boost::shared_ptr< ScToken > ScSharedTokenRef;

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
    virtual void                CalcAbsIfRel( const ScAddress& );
    virtual void                CalcRelFromAbs( const ScAddress& );
    virtual const ScMatrix*     GetMatrix() const;
    virtual ScMatrix*           GetMatrix();
    virtual ScJumpMatrix*       GetJumpMatrix() const;
    virtual const ScRefList*    GetRefList() const;
    virtual       ScRefList*    GetRefList();

    virtual sal_Bool                TextEqual( const formula::FormulaToken& rToken ) const;
    virtual sal_Bool                Is3DRef() const;    // reference with 3D flag set

    /** If rTok1 and rTok2 both are SingleRef or DoubleRef tokens, extend/merge
        ranges as needed for ocRange.
        @param rPos
            The formula's position, used to calculate absolute positions from
            relative references.
        @param bReuseDoubleRef
            If sal_True, a DoubleRef token is reused if passed as rTok1 or rTok2,
            else a new DoubleRef token is created and returned.
        @return
            A reused or new'ed ScDoubleRefToken, or a NULL TokenRef if rTok1 or
            rTok2 are not of sv(Single|Double)Ref
     */
    static  formula::FormulaTokenRef          ExtendRangeReference( formula::FormulaToken & rTok1, formula::FormulaToken & rTok2, const ScAddress & rPos, bool bReuseDoubleRef );
};

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
    virtual void                CalcAbsIfRel( const ScAddress& );
    virtual void                CalcRelFromAbs( const ScAddress& );
    virtual sal_Bool                operator==( const formula::FormulaToken& rToken ) const;
    virtual FormulaToken*       Clone() const { return new ScSingleRefToken(*this); }

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
    virtual void                CalcAbsIfRel( const ScAddress& );
    virtual void                CalcRelFromAbs( const ScAddress& );
    virtual sal_Bool                operator==( const formula::FormulaToken& rToken ) const;
    virtual FormulaToken*       Clone() const { return new ScDoubleRefToken(*this); }

    DECL_FIXEDMEMPOOL_NEWDEL( ScDoubleRefToken );
};

class ScMatrixToken : public ScToken
{
private:
            ScMatrixRef         pMatrix;
public:
                                ScMatrixToken( ScMatrix* p ) :
                                    ScToken( formula::svMatrix ), pMatrix( p ) {}
                                ScMatrixToken( const ScMatrixToken& r ) :
                                    ScToken( r ), pMatrix( r.pMatrix ) {}
    virtual const ScMatrix*     GetMatrix() const;
    virtual ScMatrix*           GetMatrix();
    virtual sal_Bool                operator==( const formula::FormulaToken& rToken ) const;
    virtual FormulaToken*       Clone() const { return new ScMatrixToken(*this); }
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
    virtual void                CalcAbsIfRel( const ScAddress& );
    virtual void                CalcRelFromAbs( const ScAddress& );
    virtual sal_Bool                operator==( const formula::FormulaToken& rToken ) const;
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
    virtual void                CalcAbsIfRel( const ScAddress& );
    virtual void                CalcRelFromAbs( const ScAddress& );
    virtual sal_Bool                operator==( const formula::FormulaToken& rToken ) const;
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
    virtual sal_Bool                operator==( const formula::FormulaToken& rToken ) const;
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
    virtual sal_Bool                operator==( const formula::FormulaToken& rToken ) const;
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
    virtual void                CalcAbsIfRel( const ScAddress& );
    virtual void                CalcRelFromAbs( const ScAddress& );
    virtual const ScRefList*    GetRefList() const;
    virtual       ScRefList*    GetRefList();
    virtual sal_Bool                operator==( const formula::FormulaToken& rToken ) const;
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
    virtual sal_Bool                operator==( const formula::FormulaToken& rToken ) const;
    virtual FormulaToken*       Clone() const { return new ScEmptyCellToken(*this); }
};


/**  Transports the result from the interpreter to the formula cell. */
class SC_DLLPUBLIC ScMatrixCellResultToken : public ScToken
{
    // No non-const access implemented, silence down unxsols4 complaining about
    // the public GetMatrix() hiding the one from ScToken.
    virtual ScMatrix*           GetMatrix();

protected:
            ScConstMatrixRef    xMatrix;
            formula::FormulaConstTokenRef     xUpperLeft;
public:
                                ScMatrixCellResultToken( ScMatrix* pMat, formula::FormulaToken* pUL ) :
                                    ScToken( formula::svMatrixCell ),
                                    xMatrix( pMat), xUpperLeft( pUL) {}
                                ScMatrixCellResultToken( const ScMatrixCellResultToken& r ) :
                                    ScToken( r ), xMatrix( r.xMatrix ),
                                    xUpperLeft( r.xUpperLeft ) {}
    virtual double              GetDouble() const;
    virtual const String &      GetString() const;
    virtual const ScMatrix*     GetMatrix() const;
    virtual sal_Bool                operator==( const formula::FormulaToken& rToken ) const;
    virtual FormulaToken*       Clone() const { return new ScMatrixCellResultToken(*this); }
    formula::StackVar           GetUpperLeftType() const
                                    {
                                        return xUpperLeft ?
                                            xUpperLeft->GetType() :
                                            static_cast<formula::StackVar>(formula::svUnknown);
                                    }
    inline formula::FormulaConstTokenRef     GetUpperLeftToken() const   { return xUpperLeft; }
            void                Assign( const ScMatrixCellResultToken & r )
                                    {
                                        xMatrix = r.xMatrix;
                                        xUpperLeft = r.xUpperLeft;
                                    }
};


/** Stores the matrix result at the formula cell, additionally the range the
    matrix formula occupies. */
class SC_DLLPUBLIC ScMatrixFormulaCellToken : public ScMatrixCellResultToken
{
private:
            SCROW               nRows;
            SCCOL               nCols;
public:
                                ScMatrixFormulaCellToken( SCCOL nC, SCROW nR ) :
                                    ScMatrixCellResultToken( NULL, NULL ),
                                    nRows( nR ), nCols( nC ) {}
                                ScMatrixFormulaCellToken( const ScMatrixFormulaCellToken& r ) :
                                    ScMatrixCellResultToken( r ),
                                    nRows( r.nRows ), nCols( r.nCols )
                                    {
                                        // xUpperLeft is modifiable through
                                        // SetUpperLeftDouble(), so clone it.
                                        if (xUpperLeft)
                                            xUpperLeft = xUpperLeft->Clone();
                                    }
    virtual sal_Bool                operator==( const formula::FormulaToken& rToken ) const;
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
            void                Assign( const ScMatrixCellResultToken & r )
                                    {
                                        ScMatrixCellResultToken::Assign( r);
                                    }

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
            void                ResetResult()
                                    {
                                        xMatrix = NULL;
                                        xUpperLeft = NULL;
                                    }
};


class SC_DLLPUBLIC ScHybridCellToken : public ScToken
{
private:
            double              fDouble;
            String              aString;
            String              aFormula;
public:
                                ScHybridCellToken( double f,
                                        const String & rStr,
                                        const String & rFormula ) :
                                    ScToken( formula::svHybridCell ),
                                    fDouble( f ), aString( rStr ),
                                    aFormula( rFormula ) {}
                                ScHybridCellToken( const ScHybridCellToken& r ) :
                                    ScToken( r ), fDouble( r.fDouble),
                                    aString( r.aString), aFormula( r.aFormula) {}
            const String &      GetFormula() const  { return aFormula; }
    virtual double              GetDouble() const;
    virtual const String &      GetString() const;
    virtual sal_Bool                operator==( const formula::FormulaToken& rToken ) const;
    virtual FormulaToken*       Clone() const { return new ScHybridCellToken(*this); }
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
                        if ( rT.GetType() == formula::svSingleRef )
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
                        Ref2( r.GetType() == formula::svDoubleRef ?
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
