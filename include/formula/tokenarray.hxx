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

#ifndef INCLUDED_FORMULA_TOKENARRAY_HXX
#define INCLUDED_FORMULA_TOKENARRAY_HXX

#include <com/sun/star/sheet/FormulaToken.hpp>
#include <formula/token.hxx>
#include <formula/ExternalReferenceHelper.hxx>
#include <limits.h>

#include <boost/unordered_set.hpp>

namespace svl {

class SharedString;
class SharedStringPool;

}

namespace formula
{

// RecalcMode access only via TokenArray SetRecalcMode / IsRecalcMode...

typedef sal_uInt8 ScRecalcMode;
// Only one of the exclusive bits can be set,
// handled by TokenArray SetRecalcMode... methods
#define RECALCMODE_NORMAL       0x01    // exclusive
#define RECALCMODE_ALWAYS       0x02    // exclusive, always
#define RECALCMODE_ONLOAD       0x04    // exclusive, always after load
#define RECALCMODE_ONLOAD_ONCE  0x08    // exclusive, once after load
#define RECALCMODE_FORCED       0x10    // combined, also if cell isn't visible
#define RECALCMODE_ONREFMOVE    0x20    // combined, if reference was moved
#define RECALCMODE_EMASK        0x0F    // mask of exclusive bits
// If new bits are to be defined, AddRecalcMode has to be adjusted!

class FormulaMissingContext;

class FORMULA_DLLPUBLIC MissingConvention
{
public:
    enum Convention
    {
        FORMULA_MISSING_CONVENTION_PODF,
        FORMULA_MISSING_CONVENTION_ODFF,
        FORMULA_MISSING_CONVENTION_OOXML
    };
    explicit            MissingConvention( Convention eConvention ) : meConvention(eConvention) {}
    inline  bool        isPODF() const  { return meConvention == FORMULA_MISSING_CONVENTION_PODF; }
    inline  Convention  getConvention() const { return meConvention; }
private:
    Convention meConvention;
};

class FORMULA_DLLPUBLIC MissingConventionODF : public MissingConvention
{
public:
    explicit    MissingConventionODF( bool bODFF ) :
        MissingConvention( bODFF ?
                MissingConvention::FORMULA_MISSING_CONVENTION_ODFF :
                MissingConvention::FORMULA_MISSING_CONVENTION_PODF)
        {
        }
    // Implementation and usage only in token.cxx
    inline  bool    isRewriteNeeded( OpCode eOp ) const;
};

class FORMULA_DLLPUBLIC MissingConventionOOXML : public MissingConvention
{
public:
    explicit    MissingConventionOOXML() : MissingConvention( MissingConvention::FORMULA_MISSING_CONVENTION_OOXML) {}
    // Implementation and usage only in token.cxx
    inline  bool    isRewriteNeeded( OpCode eOp ) const;
};

class FORMULA_DLLPUBLIC FormulaTokenArray
{
    friend class FormulaCompiler;
    friend class FormulaTokenIterator;
    friend class FormulaMissingContext;

protected:
    FormulaToken**  pCode;                  // Token code array
    FormulaToken**  pRPN;                   // RPN array
    sal_uInt16          nLen;                   // Length of token array
    sal_uInt16          nRPN;                   // Length of RPN array
    sal_uInt16          nIndex;                 // Current step index
    sal_uInt16          nError;                 // Error code
    short           nRefs;                  // Count of cell references
    ScRecalcMode    nMode;                  // Flags to indicate when to recalc this code
    bool            bHyperLink;             // If HYPERLINK() occurs in the formula.

protected:
    void                    Assign( const FormulaTokenArray& );
    void                    Assign( sal_uInt16 nCode, FormulaToken **pTokens );

    /// Also used by the compiler. The token MUST had been allocated with new!
    FormulaToken*           Add( FormulaToken* );
    inline  void            SetCombinedBitsRecalcMode( ScRecalcMode nBits )
                                { nMode |= (nBits & ~RECALCMODE_EMASK); }
    inline  ScRecalcMode    GetCombinedBitsRecalcMode() const
                                { return nMode & ~RECALCMODE_EMASK; }
                            /** Exclusive bits already set in nMode are
                                zero'ed, nBits may contain combined bits, but
                                only one exclusive bit may be set! */
    inline  void            SetMaskedRecalcMode( ScRecalcMode nBits )
                                { nMode = GetCombinedBitsRecalcMode() | nBits; }

public:
    FormulaTokenArray();
    /// Assignment with references to FormulaToken entries (not copied!)
    FormulaTokenArray( const FormulaTokenArray& );
    virtual ~FormulaTokenArray();
    FormulaTokenArray* Clone() const;    /// True copy!

    void Clear();
    void DelRPN();
    FormulaToken* First() { nIndex = 0; return Next(); }
    FormulaToken* Next();
    FormulaToken* FirstNoSpaces() { nIndex = 0; return NextNoSpaces(); }
    FormulaToken* NextNoSpaces();
    FormulaToken* GetNextName();
    FormulaToken* GetNextReference();
    FormulaToken* GetNextReferenceRPN();
    FormulaToken* GetNextReferenceOrName();
    FormulaToken* GetNextColRowName();
    /// Peek at nIdx-1 if not out of bounds, decrements nIdx if successful. Returns NULL if not.
    FormulaToken* PeekPrev( sal_uInt16 & nIdx );
    FormulaToken* PeekNext();
    FormulaToken* PeekPrevNoSpaces();    /// Only after Reset/First/Next/Last/Prev!
    FormulaToken* PeekNextNoSpaces();    /// Only after Reset/First/Next/Last/Prev!
    FormulaToken* FirstRPN() { nIndex = 0; return NextRPN(); }
    FormulaToken* NextRPN();
    FormulaToken* LastRPN() { nIndex = nRPN; return PrevRPN(); }
    FormulaToken* PrevRPN();

    bool HasReferences() const;

    bool    HasExternalRef() const;
    bool    HasOpCode( OpCode ) const;
    bool    HasOpCodeRPN( OpCode ) const;
    /// Token of type svIndex or opcode ocColRowName
    bool    HasNameOrColRowName() const;

    /**
     * Check if the token array contains any of specified opcode tokens.
     *
     * @param rOpCodes collection of opcodes to check against.
     *
     * @return true if the token array contains at least one of the specified
     *         opcode tokens, false otherwise.
     */
    bool HasOpCodes( const boost::unordered_set<OpCode>& rOpCodes ) const;

    FormulaToken** GetArray() const  { return pCode; }
    FormulaToken** GetCode()  const  { return pRPN; }
    sal_uInt16    GetLen() const     { return nLen; }
    sal_uInt16    GetCodeLen() const { return nRPN; }
    void      Reset()            { nIndex = 0; }
    sal_uInt16    GetCodeError() const      { return nError; }
    void      SetCodeError( sal_uInt16 n )  { nError = n; }
    short     GetRefs()  const { return nRefs;  }
    void      IncrementRefs() { ++nRefs; }
    void      SetHyperLink( bool bVal ) { bHyperLink = bVal; }
    bool      IsHyperLink() const       { return bHyperLink; }

    inline  ScRecalcMode    GetRecalcMode() const { return nMode; }
                            /** Bits aren't set directly but validated and
                                maybe handled according to priority if more
                                than one exclusive bit was set. */
            void            AddRecalcMode( ScRecalcMode nBits );

    inline  void            ClearRecalcMode() { nMode = RECALCMODE_NORMAL; }
    inline  void            SetExclusiveRecalcModeNormal()
                                { SetMaskedRecalcMode( RECALCMODE_NORMAL ); }
    inline  void            SetExclusiveRecalcModeAlways()
                                { SetMaskedRecalcMode( RECALCMODE_ALWAYS ); }
    inline  void            SetExclusiveRecalcModeOnLoad()
                                { SetMaskedRecalcMode( RECALCMODE_ONLOAD ); }
    inline  void            SetExclusiveRecalcModeOnLoadOnce()
                                { SetMaskedRecalcMode( RECALCMODE_ONLOAD_ONCE ); }
    inline  void            SetRecalcModeForced()
                                { nMode |= RECALCMODE_FORCED; }
    inline  void            ClearRecalcModeForced()
                                { nMode &= ~RECALCMODE_FORCED; }
    inline  void            SetRecalcModeOnRefMove()
                                { nMode |= RECALCMODE_ONREFMOVE; }
    inline  void            ClearRecalcModeOnRefMove()
                                { nMode &= ~RECALCMODE_ONREFMOVE; }
    inline  bool            IsRecalcModeNormal() const
                                { return (nMode & RECALCMODE_NORMAL) != 0; }
    inline  bool            IsRecalcModeAlways() const
                                { return (nMode & RECALCMODE_ALWAYS) != 0; }
    inline  bool            IsRecalcModeOnLoad() const
                                { return (nMode & RECALCMODE_ONLOAD) != 0; }
    inline  bool            IsRecalcModeOnLoadOnce() const
                                { return (nMode & RECALCMODE_ONLOAD_ONCE) != 0; }
    inline  bool            IsRecalcModeForced() const
                                { return (nMode & RECALCMODE_FORCED) != 0; }
    inline  bool            IsRecalcModeOnRefMove() const
                                { return (nMode & RECALCMODE_ONREFMOVE) != 0; }

                            /** Get OpCode of the most outer function */
    inline OpCode           GetOuterFuncOpCode();

                            /** Operators +,-,*,/,^,&,=,<>,<,>,<=,>=
                                with DoubleRef in Formula? */
    bool                    HasMatrixDoubleRefOps();

    virtual FormulaToken* AddOpCode(OpCode e);

    /** Adds the single token to array.
        Derived classes must overload it when they want to support derived classes from FormulaToken.
        @return true        when an error occurs
    */
    virtual bool AddFormulaToken(
        const css::sheet::FormulaToken& rToken, svl::SharedStringPool& rSPool,
        ExternalReferenceHelper* pExtRef );

    /** fill the array with the tokens from the sequence.
        It calls AddFormulaToken for each token in the list.
        @param  _aSequence  the token to add
        @return true        when an error occurs
    */
    bool Fill(
        const css::uno::Sequence<css::sheet::FormulaToken>& rSequence,
        svl::SharedStringPool& rSPool, ExternalReferenceHelper* pExtRef );

    /**
     * Do some checking based on the individual tokens. For now, we use this
     * only to check whether we can vectorize the token array.
     */
    virtual void CheckToken( const FormulaToken& t );

    FormulaToken* AddToken( const FormulaToken& );
    FormulaToken* AddString( const svl::SharedString& rStr );
    FormulaToken* AddDouble( double fVal );
    FormulaToken* AddExternal( const sal_Unicode* pStr );
    /** Xcl import may play dirty tricks with OpCode!=ocExternal.
        Others don't use! */
    FormulaToken* AddExternal( const OUString& rStr, OpCode eOp = ocExternal );
    FormulaToken* AddBad( const OUString& rStr );          /// ocBad with OUString
    FormulaToken* AddStringXML( const OUString& rStr );    /// ocStringXML with OUString, temporary during import

    virtual FormulaToken* MergeArray( );

    /// Assignment with references to FormulaToken entries (not copied!)
    FormulaTokenArray& operator=( const FormulaTokenArray& );

    /** Determines if this formula needs any changes to convert it to something
        previous versions of OOo could consume (Plain Old Formula, pre-ODFF, or
        also ODFF) */
    bool                NeedsPodfRewrite( const MissingConventionODF & rConv );

    /** Determines if this formula needs any changes to convert it to OOXML. */
    bool                NeedsOoxmlRewrite( const MissingConventionOOXML & rConv );

    /** Rewrites to Plain Old Formula or OOXML, substituting missing parameters. The
        FormulaTokenArray* returned is new'ed. */
    FormulaTokenArray*  RewriteMissing( const MissingConvention & rConv );

    /** Determines if this formula may be followed by a reference. */
    bool                MayReferenceFollow();
};

inline OpCode FormulaTokenArray::GetOuterFuncOpCode()
{
    if ( pRPN && nRPN )
        return pRPN[nRPN-1]->GetOpCode();
    return ocNone;
}

class FORMULA_DLLPUBLIC FormulaTokenIterator
{
    struct Item
    {
    public:
        const FormulaTokenArray* pArr;
        short nPC;
        short nStop;

        Item(const FormulaTokenArray* arr, short pc, short stop);
    };

    std::vector<Item> *maStack;

public:
    FormulaTokenIterator( const FormulaTokenArray& );
   ~FormulaTokenIterator();
    void    Reset();
    const   FormulaToken* Next();
    const   FormulaToken* PeekNextOperator();
    bool    IsEndOfPath() const;    /// if a jump or subroutine path is done
    bool    HasStacked() const { return maStack->size() > 1; }
    short   GetPC() const { return maStack->back().nPC; }

    /** Jump or subroutine call.
        Program counter values will be incremented before code is executed =>
        positions are to be passed with -1 offset.
        @param nStart
            Start on code at position nStart+1 (yes, pass with offset -1)
        @param nNext
            After subroutine continue with instruction at position nNext+1
        @param nStop
            Stop before reaching code at position nStop. If not specified the
            default is to either run the entire code, or to stop if an ocSep or
            ocClose is encountered, which are only present in ocIf or ocChose
            jumps.
      */
    void Jump( short nStart, short nNext, short nStop = SHRT_MAX );
    void Push( const FormulaTokenArray* );
    void Pop();

private:
    const FormulaToken* GetNonEndOfPathToken( short nIdx ) const;
};
} // formula

#endif // INCLUDED_FORMULA_TOKENARRAY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
