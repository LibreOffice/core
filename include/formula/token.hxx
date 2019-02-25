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

#ifndef INCLUDED_FORMULA_TOKEN_HXX
#define INCLUDED_FORMULA_TOKEN_HXX

#include <sal/config.h>

#include <cstring>
#include <memory>
#include <vector>

#include <formula/formuladllapi.h>
#include <formula/IFunctionDescription.hxx>
#include <formula/opcode.hxx>
#include <formula/types.hxx>
#include <formula/paramclass.hxx>
#include <osl/interlck.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <svl/sharedstring.hxx>

class ScJumpMatrix;
class ScMatrix;
struct ScComplexRefData;
struct ScSingleRefData;
enum class FormulaError : sal_uInt16;

namespace formula
{

enum StackVar : sal_uInt8
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
    svJumpMatrix,
    svRefList,                          // ocUnion result
    svEmptyCell,                        // Result is an empty cell, e.g. in LOOKUP()

    svMatrixCell,                       // Result is a matrix with bells and
                                        // whistles as needed for _the_ matrix
                                        // formula result.

    svHybridCell,                       // A temporary condition of a formula
                                        // cell during import, having a double
                                        // and/or string result and a formula
                                        // string to be compiled.

    svExternalSingleRef,
    svExternalDoubleRef,
    svExternalName,
    svSingleVectorRef,
    svDoubleVectorRef,
    svError,                            // error token
    svMissing,                          // 0 or ""
    svSep,                              // separator, ocSep, ocOpen, ocClose
    svUnknown                           // unknown StackType
};

// Only to be used for debugging output. No guarantee of stability of the
// return value.

// Turn this into an operator<< when StackVar becomes a scoped enum

inline std::string StackVarEnumToString(StackVar const e)
{
    switch (e)
    {
        case svByte:              return "Byte";
        case svDouble:            return "Double";
        case svString:            return "String";
        case svSingleRef:         return "SingleRef";
        case svDoubleRef:         return "DoubleRef";
        case svMatrix:            return "Matrix";
        case svIndex:             return "Index";
        case svJump:              return "Jump";
        case svExternal:          return "External";
        case svFAP:               return "FAP";
        case svJumpMatrix:        return "JumpMatrix";
        case svRefList:           return "RefList";
        case svEmptyCell:         return "EmptyCell";
        case svMatrixCell:        return "MatrixCell";
        case svHybridCell:        return "HybridCell";
        case svExternalSingleRef: return "ExternalSingleRef";
        case svExternalDoubleRef: return "ExternalDoubleRef";
        case svExternalName:      return "ExternalName";
        case svSingleVectorRef:   return "SingleVectorRef";
        case svDoubleVectorRef:   return "DoubleVectorRef";
        case svError:             return "Error";
        case svMissing:           return "Missing";
        case svSep:               return "Sep";
        case svUnknown:           return "Unknown";
    }
    std::ostringstream os;
    os << static_cast<int>(e);
    return os.str();
}

class FORMULA_DLLPUBLIC FormulaToken
{
    OpCode                      eOp;
    const StackVar              eType;          // type of data
    mutable oslInterlockedCount mnRefCnt;        // reference count

    FormulaToken&            operator=( const FormulaToken& ) = delete;
public:
    FormulaToken( StackVar eTypeP,OpCode e = ocPush );
    FormulaToken( const FormulaToken& r );

    virtual                     ~FormulaToken();

    void                Delete()                { delete this; }
    void                DeleteIfZeroRef()       { if (mnRefCnt == 0) delete this; }
    StackVar            GetType() const         { return eType; }
    bool                IsFunction() const; // pure functions, no operators

    bool IsExternalRef() const;
    bool IsRef() const;

            sal_uInt8           GetParamCount() const;

    void IncRef() const
    {
        osl_atomic_increment(&mnRefCnt);
    }

    void DecRef() const
    {
        if (!osl_atomic_decrement(&mnRefCnt))
            const_cast<FormulaToken*>(this)->Delete();
    }

    oslInterlockedCount GetRef() const       { return mnRefCnt; }
    OpCode              GetOpCode() const    { return eOp; }

    bool                IsInForceArray() const;

    /**
        Dummy methods to avoid switches and casts where possible,
        the real token classes have to override the appropriate method[s].
        The only methods valid anytime if not overridden are:

        - GetByte() since this represents the count of parameters to a function
          which of course is 0 on non-functions. FormulaByteToken and ScExternal do
          override it.

        - GetInForceArray() since also this is only used for operators and
          functions and is ParamClass::Unknown for other tokens.

        Any other non-overridden method pops up an assertion.
     */

    virtual sal_uInt8           GetByte() const;
    virtual void                SetByte( sal_uInt8 n );
    virtual ParamClass          GetInForceArray() const;
    virtual void                SetInForceArray( ParamClass c );
    virtual double              GetDouble() const;
    virtual double&             GetDoubleAsReference();
    virtual sal_Int16           GetDoubleType() const;
    virtual void                SetDoubleType( sal_Int16 nType );
    virtual svl::SharedString   GetString() const;
    virtual void                SetString( const svl::SharedString& rStr );
    virtual sal_uInt16          GetIndex() const;
    virtual void                SetIndex( sal_uInt16 n );
    virtual sal_Int16           GetSheet() const;
    virtual void                SetSheet( sal_Int16 n );
    virtual short*              GetJump() const;
    virtual const OUString&     GetExternal() const;
    virtual FormulaToken*       GetFAPOrigToken() const;
    virtual FormulaError        GetError() const;
    virtual void                SetError( FormulaError );

    virtual const ScSingleRefData*  GetSingleRef() const;
    virtual ScSingleRefData*        GetSingleRef();
    virtual const ScComplexRefData* GetDoubleRef() const;
    virtual ScComplexRefData*       GetDoubleRef();
    virtual const ScSingleRefData*  GetSingleRef2() const;
    virtual ScSingleRefData*        GetSingleRef2();
    virtual const ScMatrix*     GetMatrix() const;
    virtual ScMatrix*           GetMatrix();
    virtual ScJumpMatrix*       GetJumpMatrix() const;
    virtual const std::vector<ScComplexRefData>* GetRefList() const;
    virtual       std::vector<ScComplexRefData>* GetRefList();

    virtual FormulaToken*       Clone() const { return new FormulaToken(*this); }

    virtual bool                TextEqual( const formula::FormulaToken& rToken ) const;
    virtual bool                operator==( const FormulaToken& rToken ) const;

    /** This is dirty and only the compiler should use it! */
    struct PrivateAccess { friend class FormulaCompiler; private: PrivateAccess() { }  };
    void                NewOpCode( OpCode e, const PrivateAccess&  ) { eOp = e; }
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
            ParamClass          eInForceArray;
protected:
                                FormulaByteToken( OpCode e, sal_uInt8 n, StackVar v, ParamClass c ) :
                                    FormulaToken( v,e ), nByte( n ),
                                    eInForceArray( c ) {}
public:
                                FormulaByteToken( OpCode e, sal_uInt8 n, ParamClass c ) :
                                    FormulaToken( svByte,e ), nByte( n ),
                                    eInForceArray( c ) {}
                                FormulaByteToken( OpCode e, sal_uInt8 n ) :
                                    FormulaToken( svByte,e ), nByte( n ),
                                    eInForceArray( ParamClass::Unknown ) {}
                                FormulaByteToken( OpCode e ) :
                                    FormulaToken( svByte,e ), nByte( 0 ),
                                    eInForceArray( ParamClass::Unknown ) {}
                                FormulaByteToken( const FormulaByteToken& r ) :
                                    FormulaToken( r ), nByte( r.nByte ),
                                    eInForceArray( r.eInForceArray ) {}

    virtual FormulaToken*       Clone() const override { return new FormulaByteToken(*this); }
    virtual sal_uInt8           GetByte() const override;
    virtual void                SetByte( sal_uInt8 n ) override;
    virtual ParamClass          GetInForceArray() const override;
    virtual void                SetInForceArray( ParamClass c ) override;
    virtual bool                operator==( const FormulaToken& rToken ) const override;
};


// A special token for the FormulaAutoPilot only. Keeps a reference pointer of
// the token of which it was created for comparison.
class FORMULA_DLLPUBLIC FormulaFAPToken : public FormulaByteToken
{
private:
            FormulaTokenRef     pOrigToken;
public:
                                FormulaFAPToken( OpCode e, sal_uInt8 n, FormulaToken* p ) :
                                    FormulaByteToken( e, n, svFAP, ParamClass::Unknown ),
                                    pOrigToken( p ) {}
                                FormulaFAPToken( const FormulaFAPToken& r ) :
                                    FormulaByteToken( r ), pOrigToken( r.pOrigToken ) {}

    virtual FormulaToken*       Clone() const override { return new FormulaFAPToken(*this); }
    virtual FormulaToken*       GetFAPOrigToken() const override;
    virtual bool                operator==( const FormulaToken& rToken ) const override;
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

    virtual FormulaToken*       Clone() const override { return new FormulaDoubleToken(*this); }
    virtual double              GetDouble() const override;
    virtual double&             GetDoubleAsReference() override;
    virtual sal_Int16           GetDoubleType() const override;     ///< always returns 0 for "not typed"
    virtual bool                operator==( const FormulaToken& rToken ) const override;
};

class FORMULA_DLLPUBLIC FormulaTypedDoubleToken : public FormulaDoubleToken
{
private:
            sal_Int16           mnType;     /**< Can hold, for example, a value
                                              of SvNumFormatType, or by
                                              contract any other
                                              classification. */
public:
                                FormulaTypedDoubleToken( double f, sal_Int16 nType ) :
                                    FormulaDoubleToken( f ), mnType( nType ) {}
                                FormulaTypedDoubleToken( const FormulaTypedDoubleToken& r ) :
                                    FormulaDoubleToken( r ), mnType( r.mnType ) {}

    virtual FormulaToken*       Clone() const override { return new FormulaTypedDoubleToken(*this); }
    virtual sal_Int16           GetDoubleType() const override;
    virtual void                SetDoubleType( sal_Int16 nType ) override;
    virtual bool                operator==( const FormulaToken& rToken ) const override;
};


class FORMULA_DLLPUBLIC FormulaStringToken : public FormulaToken
{
    svl::SharedString maString;
public:
    FormulaStringToken( const svl::SharedString& r );
    FormulaStringToken( const FormulaStringToken& r );

    virtual FormulaToken* Clone() const override;
    virtual svl::SharedString GetString() const override;
    virtual void SetString( const svl::SharedString& rStr ) override;
    virtual bool operator==( const FormulaToken& rToken ) const override;
};


/** Identical to FormulaStringToken, but with explicit OpCode instead of implicit
    ocPush, and an optional sal_uInt8 for ocBad tokens. */
class FORMULA_DLLPUBLIC FormulaStringOpToken : public FormulaByteToken
{
    svl::SharedString maString;
public:
    FormulaStringOpToken( OpCode e, const svl::SharedString& r );
    FormulaStringOpToken( const FormulaStringOpToken& r );

    virtual FormulaToken* Clone() const override;
    virtual svl::SharedString GetString() const override;
    virtual void SetString( const svl::SharedString& rStr ) override;
    virtual bool operator==( const FormulaToken& rToken ) const override;
};

class FORMULA_DLLPUBLIC FormulaIndexToken : public FormulaToken
{
private:
            sal_uInt16          nIndex;
            sal_Int16           mnSheet;
public:
                                FormulaIndexToken( OpCode e, sal_uInt16 n, sal_Int16 nSheet = -1 ) :
                                    FormulaToken(  svIndex, e ), nIndex( n ), mnSheet( nSheet ) {}
                                FormulaIndexToken( const FormulaIndexToken& r ) :
                                    FormulaToken( r ), nIndex( r.nIndex ), mnSheet( r.mnSheet ) {}

    virtual FormulaToken*       Clone() const override { return new FormulaIndexToken(*this); }
    virtual sal_uInt16          GetIndex() const override;
    virtual void                SetIndex( sal_uInt16 n ) override;
    virtual sal_Int16           GetSheet() const override;
    virtual void                SetSheet( sal_Int16 n ) override;
    virtual bool                operator==( const FormulaToken& rToken ) const override;
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

    virtual FormulaToken*       Clone() const override { return new FormulaExternalToken(*this); }
    virtual const OUString&       GetExternal() const override;
    virtual sal_uInt8           GetByte() const override;
    virtual void                SetByte( sal_uInt8 n ) override;
    virtual bool                operator==( const FormulaToken& rToken ) const override;
};


class FORMULA_DLLPUBLIC FormulaMissingToken : public FormulaToken
{
public:
                                FormulaMissingToken() :
                                    FormulaToken( svMissing,ocMissing ) {}
                                FormulaMissingToken( const FormulaMissingToken& r ) :
                                    FormulaToken( r ) {}

    virtual FormulaToken*       Clone() const override { return new FormulaMissingToken(*this); }
    virtual double              GetDouble() const override;
    virtual svl::SharedString GetString() const override;
    virtual bool                operator==( const FormulaToken& rToken ) const override;
};

class FORMULA_DLLPUBLIC FormulaJumpToken : public FormulaToken
{
private:
            std::unique_ptr<short[]>
                                pJump;
            ParamClass          eInForceArray;
public:
                                FormulaJumpToken( OpCode e, short const * p ) :
                                    FormulaToken( formula::svJump , e),
                                    eInForceArray( ParamClass::Unknown)
                                {
                                    pJump.reset( new short[ p[0] + 1 ] );
                                    memcpy( pJump.get(), p, (p[0] + 1) * sizeof(short) );
                                }
                                FormulaJumpToken( const FormulaJumpToken& r ) :
                                    FormulaToken( r ),
                                    eInForceArray( r.eInForceArray)
                                {
                                    pJump.reset( new short[ r.pJump[0] + 1 ] );
                                    memcpy( pJump.get(), r.pJump.get(), (r.pJump[0] + 1) * sizeof(short) );
                                }
    virtual                     ~FormulaJumpToken() override;
    virtual short*              GetJump() const override;
    virtual bool                operator==( const formula::FormulaToken& rToken ) const override;
    virtual FormulaToken*       Clone() const override { return new FormulaJumpToken(*this); }
    virtual ParamClass          GetInForceArray() const override;
    virtual void                SetInForceArray( ParamClass c ) override;
};


class FORMULA_DLLPUBLIC FormulaUnknownToken : public FormulaToken
{
public:
                                FormulaUnknownToken( OpCode e ) :
                                    FormulaToken( svUnknown, e ) {}
                                FormulaUnknownToken( const FormulaUnknownToken& r ) :
                                    FormulaToken( r ) {}

    virtual FormulaToken*       Clone() const override { return new FormulaUnknownToken(*this); }
    virtual bool                operator==( const FormulaToken& rToken ) const override;
};


class FORMULA_DLLPUBLIC FormulaErrorToken : public FormulaToken
{
         FormulaError          nError;
public:
                                FormulaErrorToken( FormulaError nErr ) :
                                    FormulaToken( svError ), nError( nErr) {}
                                FormulaErrorToken( const FormulaErrorToken& r ) :
                                    FormulaToken( r ), nError( r.nError) {}

    virtual FormulaToken*       Clone() const override { return new FormulaErrorToken(*this); }
    virtual FormulaError        GetError() const override;
    virtual void                SetError( FormulaError nErr ) override;
    virtual bool                operator==( const FormulaToken& rToken ) const override;
};


} // formula


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
