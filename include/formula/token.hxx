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

#ifndef FORMULA_TOKEN_HXX
#define FORMULA_TOKEN_HXX

#include <memory>
#include <string.h>
#include <vector>
#include "formula/opcode.hxx"
#include <tools/mempool.hxx>
#include "formula/IFunctionDescription.hxx"
#include "formula/formuladllapi.h"
#include "formula/types.hxx"

namespace formula
{

enum StackVarEnum
{
    svByte,
    svDouble,
    svString,
    svSingleRef,
    svDoubleRef,
    svMatrix,
    svIndex,
    svJump,
    svExternal,                         // Byte + String
    svFAP,                              // FormulaAutoPilot only, ever exported
    svJumpMatrix,                       // 2003-07-02
    svRefList,                          // ocUnion result
    svEmptyCell,                        // Result is an empty cell, e.g. in LOOKUP()

    svMatrixCell,                       // Result is a matrix with bells and
                                        // whistles as needed for _the_ matrix
                                        // formula result.

    svHybridCell,                       // A temporary condition of a formula
                                        // cell during import, having a double
                                        // and/or string result and a formula
                                        // string to be compiled.

    svHybridValueCell,                  // A temporary formula cell with an value
                                        // and possibily a string representation

    svExternalSingleRef,
    svExternalDoubleRef,
    svExternalName,
    svSingleVectorRef,
    svDoubleVectorRef,
    svSubroutine,                       // A token with a subroutine token array.
    svError,                            // error token
    svMissing = 0x70,                   // 0 or ""
    svSep,                              // separator, ocSep, ocOpen, ocClose
    svUnknown                           // unknown StackType
};

#ifndef DBG_UTIL
// save memory since compilers tend to int an enum
typedef sal_uInt8 StackVar;
#else
// have enum names in debugger
typedef StackVarEnum StackVar;
#endif

class FormulaTokenArray;

class FORMULA_DLLPUBLIC FormulaToken : public IFormulaToken
{
    OpCode                      eOp;
    // not implemented, prevent usage
            FormulaToken();
            FormulaToken&            operator=( const FormulaToken& );
protected:

            const StackVar      eType;          // type of data
            mutable sal_uInt16  nRefCnt;        // reference count

public:
                                FormulaToken( StackVar eTypeP,OpCode e = ocPush ) :
                                    eOp(e), eType( eTypeP ), nRefCnt(0) {}
                                FormulaToken( const FormulaToken& r ) : IFormulaToken(),
                                    eOp(r.eOp), eType( r.eType ), nRefCnt(0) {}

    virtual                     ~FormulaToken();

    inline  void                Delete()                { delete this; }
    inline  StackVar            GetType() const         { return eType; }
            bool                IsFunction() const; // pure functions, no operators
            bool                IsExternalRef() const;
            sal_uInt8           GetParamCount() const;
    inline  void                IncRef() const          { nRefCnt++; }
    inline  void                DecRef() const
                                    {
                                        if (!--nRefCnt)
                                            const_cast<FormulaToken*>(this)->Delete();
                                    }
    inline  sal_uInt16          GetRef() const          { return nRefCnt; }
    inline OpCode               GetOpCode() const       { return eOp; }

    /**
        Dummy methods to avoid switches and casts where possible,
        the real token classes have to overload the appropriate method[s].
        The only methods valid anytime if not overloaded are:

        - GetByte() since this represents the count of parameters to a function
          which of course is 0 on non-functions. FormulaByteToken and ScExternal do
          overload it.

        - HasForceArray() since also this is only used for operators and
          functions and is 0 for other tokens.

        Any other non-overloaded method pops up an assertion.
     */

    virtual sal_uInt8           GetByte() const;
    virtual void                SetByte( sal_uInt8 n );
    virtual bool                HasForceArray() const;
    virtual void                SetForceArray( bool b );
    virtual double              GetDouble() const;
    virtual double&             GetDoubleAsReference();
    virtual const OUString&       GetString() const;
    virtual sal_uInt16          GetIndex() const;
    virtual void                SetIndex( sal_uInt16 n );
    virtual bool                IsGlobal() const;
    virtual void                SetGlobal( bool b );
    virtual short*              GetJump() const;
    virtual const OUString&       GetExternal() const;
    virtual FormulaToken*       GetFAPOrigToken() const;
    virtual sal_uInt16          GetError() const;
    virtual void                SetError( sal_uInt16 );

    virtual FormulaToken*       Clone() const { return new FormulaToken(*this); }

    virtual bool                Is3DRef() const;    // reference with 3D flag set
    virtual bool                TextEqual( const formula::FormulaToken& rToken ) const;
    virtual bool                operator==( const FormulaToken& rToken ) const;

    virtual bool isFunction() const
    {
        return IsFunction();
    }

    virtual sal_uInt32 getArgumentCount() const
    {
        return GetParamCount();
    }

    /** This is dirty and only the compiler should use it! */
    struct PrivateAccess { friend class FormulaCompiler; private: PrivateAccess() { }  };
    inline  void                NewOpCode( OpCode e, const PrivateAccess&  ) { eOp = e; }

    static  size_t              GetStrLenBytes( xub_StrLen nLen )
                                    { return nLen * sizeof(sal_Unicode); }
    static  size_t              GetStrLenBytes( const OUString& rStr )
                                    { return GetStrLenBytes( rStr.getLength() ); }
};

inline void intrusive_ptr_add_ref(const FormulaToken* p)
{
    p->IncRef();
}

inline void intrusive_ptr_release(const FormulaToken* p)
{
    p->DecRef();
}

class FORMULA_DLLPUBLIC FormulaByteToken : public FormulaToken
{
private:
            sal_uInt8           nByte;
            bool                bHasForceArray;
protected:
                                FormulaByteToken( OpCode e, sal_uInt8 n, StackVar v, bool b ) :
                                    FormulaToken( v,e ), nByte( n ),
                                    bHasForceArray( b ) {}
public:
                                FormulaByteToken( OpCode e, sal_uInt8 n, bool b ) :
                                    FormulaToken( svByte,e ), nByte( n ),
                                    bHasForceArray( b ) {}
                                FormulaByteToken( OpCode e, sal_uInt8 n ) :
                                    FormulaToken( svByte,e ), nByte( n ),
                                    bHasForceArray( false ) {}
                                FormulaByteToken( OpCode e ) :
                                    FormulaToken( svByte,e ), nByte( 0 ),
                                    bHasForceArray( false ) {}
                                FormulaByteToken( const FormulaByteToken& r ) :
                                    FormulaToken( r ), nByte( r.nByte ),
                                    bHasForceArray( r.bHasForceArray ) {}

    virtual FormulaToken*       Clone() const { return new FormulaByteToken(*this); }
    virtual sal_uInt8           GetByte() const;
    virtual void                SetByte( sal_uInt8 n );
    virtual bool                HasForceArray() const;
    virtual void                SetForceArray( bool b );
    virtual bool                operator==( const FormulaToken& rToken ) const;

    DECL_FIXEDMEMPOOL_NEWDEL_DLL( FormulaByteToken )
};


// A special token for the FormulaAutoPilot only. Keeps a reference pointer of
// the token of which it was created for comparison.
class FORMULA_DLLPUBLIC FormulaFAPToken : public FormulaByteToken
{
private:
            FormulaTokenRef     pOrigToken;
public:
                                FormulaFAPToken( OpCode e, sal_uInt8 n, FormulaToken* p ) :
                                    FormulaByteToken( e, n, svFAP, false ),
                                    pOrigToken( p ) {}
                                FormulaFAPToken( const FormulaFAPToken& r ) :
                                    FormulaByteToken( r ), pOrigToken( r.pOrigToken ) {}

    virtual FormulaToken*       Clone() const { return new FormulaFAPToken(*this); }
    virtual FormulaToken*       GetFAPOrigToken() const;
    virtual bool                operator==( const FormulaToken& rToken ) const;
};

class FORMULA_DLLPUBLIC FormulaDoubleToken : public FormulaToken
{
private:
            double              fDouble;
public:
                                FormulaDoubleToken( double f ) :
                                    FormulaToken( svDouble ), fDouble( f ) {}
                                FormulaDoubleToken( const FormulaDoubleToken& r ) :
                                    FormulaToken( r ), fDouble( r.fDouble ) {}

    virtual FormulaToken*       Clone() const { return new FormulaDoubleToken(*this); }
    virtual double              GetDouble() const;
    virtual double&             GetDoubleAsReference();
    virtual bool                operator==( const FormulaToken& rToken ) const;

    DECL_FIXEDMEMPOOL_NEWDEL_DLL( FormulaDoubleToken )
};


class FORMULA_DLLPUBLIC FormulaStringToken : public FormulaToken
{
private:
            OUString              aString;
public:
                                FormulaStringToken( const OUString& r ) :
                                    FormulaToken( svString ), aString( r ) {}
                                FormulaStringToken( const FormulaStringToken& r ) :
                                    FormulaToken( r ), aString( r.aString ) {}

    virtual FormulaToken*       Clone() const { return new FormulaStringToken(*this); }
    virtual const OUString&       GetString() const;
    virtual bool                operator==( const FormulaToken& rToken ) const;

    DECL_FIXEDMEMPOOL_NEWDEL_DLL( FormulaStringToken )
};


/** Identical to FormulaStringToken, but with explicit OpCode instead of implicit
    ocPush, and an optional sal_uInt8 for ocBad tokens. */
class FORMULA_DLLPUBLIC FormulaStringOpToken : public FormulaByteToken
{
private:
            OUString              aString;
public:
                                FormulaStringOpToken( OpCode e, const OUString& r ) :
                                    FormulaByteToken( e, 0, svString, false ), aString( r ) {}
                                FormulaStringOpToken( const FormulaStringOpToken& r ) :
                                    FormulaByteToken( r ), aString( r.aString ) {}

    virtual FormulaToken*       Clone() const { return new FormulaStringOpToken(*this); }
    virtual const OUString&       GetString() const;
    virtual bool                operator==( const FormulaToken& rToken ) const;
};

class FORMULA_DLLPUBLIC FormulaIndexToken : public FormulaToken
{
private:
            sal_uInt16          nIndex;
            bool                mbGlobal;
public:
                                FormulaIndexToken( OpCode e, sal_uInt16 n, bool bGlobal = true ) :
                                    FormulaToken(  svIndex, e ), nIndex( n ), mbGlobal( bGlobal ) {}
                                FormulaIndexToken( const FormulaIndexToken& r ) :
                                    FormulaToken( r ), nIndex( r.nIndex ), mbGlobal( r.mbGlobal ) {}

    virtual FormulaToken*       Clone() const { return new FormulaIndexToken(*this); }
    virtual sal_uInt16          GetIndex() const;
    virtual void                SetIndex( sal_uInt16 n );
    virtual bool                IsGlobal() const;
    virtual void                SetGlobal( bool b );
    virtual bool                operator==( const FormulaToken& rToken ) const;
};


class FORMULA_DLLPUBLIC FormulaExternalToken : public FormulaToken
{
private:
            OUString              aExternal;
            sal_uInt8           nByte;
public:
                                FormulaExternalToken( OpCode e, sal_uInt8 n, const OUString& r ) :
                                    FormulaToken( svExternal, e ), aExternal( r ),
                                    nByte( n ) {}
                                FormulaExternalToken( OpCode e, const OUString& r ) :
                                    FormulaToken(svExternal, e ), aExternal( r ),
                                    nByte( 0 ) {}
                                FormulaExternalToken( const FormulaExternalToken& r ) :
                                    FormulaToken( r ), aExternal( r.aExternal ),
                                    nByte( r.nByte ) {}

    virtual FormulaToken*       Clone() const { return new FormulaExternalToken(*this); }
    virtual const OUString&       GetExternal() const;
    virtual sal_uInt8           GetByte() const;
    virtual void                SetByte( sal_uInt8 n );
    virtual bool                operator==( const FormulaToken& rToken ) const;
};


class FORMULA_DLLPUBLIC FormulaMissingToken : public FormulaToken
{
public:
                                FormulaMissingToken() :
                                    FormulaToken( svMissing,ocMissing ) {}
                                FormulaMissingToken( const FormulaMissingToken& r ) :
                                    FormulaToken( r ) {}

    virtual FormulaToken*       Clone() const { return new FormulaMissingToken(*this); }
    virtual double              GetDouble() const;
    virtual const OUString&       GetString() const;
    virtual bool                operator==( const FormulaToken& rToken ) const;
};

class FORMULA_DLLPUBLIC FormulaJumpToken : public FormulaToken
{
private:
            short*              pJump;
public:
                                FormulaJumpToken( OpCode e, short* p ) :
                                    FormulaToken( formula::svJump , e)
                                {
                                    pJump = new short[ p[0] + 1 ];
                                    memcpy( pJump, p, (p[0] + 1) * sizeof(short) );
                                }
                                FormulaJumpToken( const FormulaJumpToken& r ) :
                                    FormulaToken( r )
                                {
                                    pJump = new short[ r.pJump[0] + 1 ];
                                    memcpy( pJump, r.pJump, (r.pJump[0] + 1) * sizeof(short) );
                                }
    virtual                     ~FormulaJumpToken();
    virtual short*              GetJump() const;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const;
    virtual FormulaToken*       Clone() const { return new FormulaJumpToken(*this); }
};


class FORMULA_DLLPUBLIC FormulaSubroutineToken : public FormulaToken
{
public:
    /** Takes ownership of pArray and deletes it upon destruction! */
                                FormulaSubroutineToken( const FormulaTokenArray* pArray ) :
                                    FormulaToken( svSubroutine, ocCall ), mpArray( pArray) {}
                                FormulaSubroutineToken( const FormulaSubroutineToken& r );
    virtual                     ~FormulaSubroutineToken();
    virtual FormulaToken*       Clone() const { return new FormulaSubroutineToken(*this); }
    virtual bool                operator==( const FormulaToken& rToken ) const;

private:
    const FormulaTokenArray*    mpArray;
};


class FORMULA_DLLPUBLIC FormulaUnknownToken : public FormulaToken
{
public:
                                FormulaUnknownToken( OpCode e ) :
                                    FormulaToken( svUnknown, e ) {}
                                FormulaUnknownToken( const FormulaUnknownToken& r ) :
                                    FormulaToken( r ) {}

    virtual FormulaToken*       Clone() const { return new FormulaUnknownToken(*this); }
    virtual bool                operator==( const FormulaToken& rToken ) const;
};


class FORMULA_DLLPUBLIC FormulaErrorToken : public FormulaToken
{
            sal_uInt16          nError;
public:
                                FormulaErrorToken( sal_uInt16 nErr ) :
                                    FormulaToken( svError ), nError( nErr) {}
                                FormulaErrorToken( const FormulaErrorToken& r ) :
                                    FormulaToken( r ), nError( r.nError) {}

    virtual FormulaToken*       Clone() const { return new FormulaErrorToken(*this); }
    virtual sal_uInt16          GetError() const;
    virtual void                SetError( sal_uInt16 nErr );
    virtual bool                operator==( const FormulaToken& rToken ) const;
};

// =============================================================================
} // formula
// =============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
