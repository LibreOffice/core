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

#include <climits>
#include <memory>
#include <ostream>
#include <type_traits>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include <com/sun/star/uno/Sequence.hxx>
#include <formula/ExternalReferenceHelper.hxx>
#include <formula/formuladllapi.h>
#include <formula/opcode.hxx>
#include <formula/token.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace com { namespace sun { namespace star {
    namespace sheet { struct FormulaToken; }
} } }

namespace svl {

class SharedString;
class SharedStringPool;

}

// RecalcMode access only via TokenArray SetRecalcMode / IsRecalcMode...

// Only one of the exclusive bits can be set,
// handled by TokenArray SetRecalcMode... methods
enum class ScRecalcMode : sal_uInt8
{
    NORMAL       = 0x01,    // exclusive
    ALWAYS       = 0x02,    // exclusive, always
    ONLOAD       = 0x04,    // exclusive, always after load
    ONLOAD_ONCE  = 0x08,    // exclusive, once after load
    FORCED       = 0x10,    // combined, also if cell isn't visible
    ONREFMOVE    = 0x20,    // combined, if reference was moved
    EMask        = NORMAL | ALWAYS | ONLOAD | ONLOAD_ONCE  // mask of exclusive bits
};
// If new bits are to be defined, AddRecalcMode has to be adjusted!
namespace o3tl
{
    template<> struct typed_flags<ScRecalcMode> : is_typed_flags<ScRecalcMode, 0x3f> {};
}

namespace formula
{

class MissingConvention
{
public:
    enum Convention
    {
        FORMULA_MISSING_CONVENTION_PODF,
        FORMULA_MISSING_CONVENTION_ODFF,
        FORMULA_MISSING_CONVENTION_OOXML
    };
    explicit            MissingConvention( Convention eConvention ) : meConvention(eConvention) {}
    bool        isPODF() const  { return meConvention == FORMULA_MISSING_CONVENTION_PODF; }
    bool        isODFF() const  { return meConvention == FORMULA_MISSING_CONVENTION_ODFF; }
    bool        isOOXML() const  { return meConvention == FORMULA_MISSING_CONVENTION_OOXML; }
    Convention  getConvention() const { return meConvention; }
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
    static inline bool isRewriteNeeded( OpCode eOp );
};

typedef std::unordered_set<OpCode, std::hash<std::underlying_type<OpCode>::type> > unordered_opcode_set;

class FormulaTokenArrayStandardRange
{
private:
    FormulaToken** mpBegin;
    FormulaToken** mpEnd;

public:
    FormulaTokenArrayStandardRange(FormulaToken** pBegin, sal_uInt16 nSize) :
        mpBegin(pBegin),
        mpEnd(pBegin + nSize)
    {
    }

    FormulaToken** begin() const
    {
        return mpBegin;
    }

    FormulaToken** end() const
    {
        return mpEnd;
    }
};

class FormulaTokenArrayReferencesIterator
{
private:
    FormulaToken** maIter;
    FormulaToken** maEnd;

    void nextReference()
    {
        while (maIter != maEnd)
        {
            switch ((*maIter)->GetType())
            {
            case svSingleRef:
            case svDoubleRef:
            case svExternalSingleRef:
            case svExternalDoubleRef:
                return;
            default:
                ++maIter;
            }
        }
    }

    enum class Dummy { Flag };

    FormulaTokenArrayReferencesIterator(const FormulaTokenArrayStandardRange& rRange, Dummy) :
        maIter(rRange.end()),
        maEnd(rRange.end())
    {
    }

public:
    FormulaTokenArrayReferencesIterator(const FormulaTokenArrayStandardRange& rRange) :
        maIter(rRange.begin()),
        maEnd(rRange.end())
    {
        nextReference();
    }

    FormulaTokenArrayReferencesIterator operator++(int)
    {
        FormulaTokenArrayReferencesIterator result(*this);
        operator++();
        return result;
    }

    FormulaTokenArrayReferencesIterator operator++()
    {
        assert(maIter != maEnd);
        ++maIter;
        nextReference();
        return *this;
    }

    FormulaToken* operator*() const
    {
        return *maIter;
    }

    bool operator==(const FormulaTokenArrayReferencesIterator& rhs) const
    {
        return maIter == rhs.maIter;
    }

    bool operator!=(const FormulaTokenArrayReferencesIterator& rhs) const
    {
        return !operator==(rhs);
    }

    static FormulaTokenArrayReferencesIterator endOf(const FormulaTokenArrayStandardRange& rRange)
    {
        return FormulaTokenArrayReferencesIterator(rRange, Dummy::Flag);
    }
};

class FORMULA_DLLPUBLIC FormulaTokenArrayReferencesRange
{
private:
    const FormulaTokenArray& mrFTA;

public:
    FormulaTokenArrayReferencesRange(const FormulaTokenArray& rFTA) :
        mrFTA(rFTA)
    {
    }

    FormulaTokenArrayReferencesIterator begin();

    FormulaTokenArrayReferencesIterator end();
};

class FORMULA_DLLPUBLIC FormulaTokenArray
{
protected:
    FormulaToken**  pCode;                  // Token code array
    FormulaToken**  pRPN;                   // RPN array
    sal_uInt16      nLen;                   // Length of token array
    sal_uInt16      nRPN;                   // Length of RPN array
    FormulaError    nError;                 // Error code
    ScRecalcMode    nMode;                  // Flags to indicate when to recalc this code
    bool            bHyperLink      :1;     // If HYPERLINK() occurs in the formula.
    bool            mbFromRangeName :1;     // If this array originates from a named expression
    bool            mbShareable     :1;     // Whether or not it can be shared with adjacent cells.
    bool            mbFinalized     :1;     // Whether code arrays have their final used size and no more tokens can be added.

protected:
    void                    Assign( const FormulaTokenArray& );
    void                    Assign( sal_uInt16 nCode, FormulaToken **pTokens );

    /// Also used by the compiler. The token MUST had been allocated with new!
    FormulaToken*           Add( FormulaToken* );

public:
    enum ReplaceMode
    {
        CODE_ONLY,      ///< replacement only in pCode
        CODE_AND_RPN    ///< replacement in pCode and pRPN
    };

    /** Also used by the compiler. The token MUST had been allocated with new!
        @param  nOffset
                Absolute offset in pCode of the token to be replaced.
        @param  eMode
                If CODE_ONLY only the token in pCode at nOffset is replaced.
                If CODE_AND_RPN the token in pCode at nOffset is replaced;
                if the original token was also referenced in the pRPN array
                then that reference is replaced with a reference to the new
                token as well.
     */
    FormulaToken*           ReplaceToken( sal_uInt16 nOffset, FormulaToken*, ReplaceMode eMode );

    /** Remove a sequence of tokens from pCode array, and pRPN array if the
        tokens are referenced there.

        nLen and nRPN are adapted.

        @param  nOffset
                Start offset into pCode.
        @param  nCount
                Count of tokens to remove.

        @return Count of tokens removed.
     */
    sal_uInt16              RemoveToken( sal_uInt16 nOffset, sal_uInt16 nCount );

    void            SetCombinedBitsRecalcMode( ScRecalcMode nBits )
                                { nMode |= (nBits & ~ScRecalcMode::EMask); }
    ScRecalcMode    GetCombinedBitsRecalcMode() const
                                { return nMode & ~ScRecalcMode::EMask; }
                            /** Exclusive bits already set in nMode are
                                zero'ed, nBits may contain combined bits, but
                                only one exclusive bit may be set! */
    void            SetMaskedRecalcMode( ScRecalcMode nBits )
                                { nMode = GetCombinedBitsRecalcMode() | nBits; }

    FormulaTokenArray();
    /** Assignment with incrementing references of FormulaToken entries
        (not copied!) */
    FormulaTokenArray( const FormulaTokenArray& );
    virtual ~FormulaTokenArray();

    virtual void Clear();

    void SetFromRangeName( bool b ) { mbFromRangeName = b; }
    bool IsFromRangeName() const { return mbFromRangeName; }

    void SetShareable( bool b ) { mbShareable = b; }

    /**
     * Check if this token array is shareable between multiple adjacent
     * formula cells. Certain tokens may not function correctly when shared.
     *
     * @return true if the token array is shareable, false otherwise.
     */
    bool IsShareable() const { return mbShareable; }

    void DelRPN();
    FormulaToken* FirstToken() const;

    /// Return pCode[nIdx], or nullptr if nIdx is out of bounds
    FormulaToken* TokenAt( sal_uInt16 nIdx) const
    {
        if (nIdx >= nLen)
            return nullptr;
        return pCode[nIdx];
    }

    /// Peek at nIdx-1 if not out of bounds, decrements nIdx if successful. Returns NULL if not.
    FormulaToken* PeekPrev( sal_uInt16 & nIdx ) const;

    /// Return the opcode at pCode[nIdx-1], ocNone if nIdx-1 is out of bounds
    OpCode OpCodeBefore( sal_uInt16 nIdx) const
    {
        if (nIdx == 0 || nIdx > nLen)
            return ocNone;

        return pCode[nIdx-1]->GetOpCode();
    }

    FormulaToken* FirstRPNToken() const;

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
    bool HasOpCodes( const unordered_opcode_set& rOpCodes ) const;

    /// Assign pRPN to point to a newly created array filled with the data from pData
    void CreateNewRPNArrayFromData( FormulaToken** pData, sal_uInt16 nSize )
    {
        pRPN = new FormulaToken*[ nSize ];
        nRPN = nSize;
        memcpy( pRPN, pData, nSize * sizeof( FormulaToken* ) );
    }

    FormulaToken** GetArray() const  { return pCode; }

    FormulaTokenArrayStandardRange Tokens() const
    {
        return FormulaTokenArrayStandardRange(pCode, nLen);
    }

    FormulaToken** GetCode()  const  { return pRPN; }

    FormulaTokenArrayStandardRange RPNTokens() const
    {
        return FormulaTokenArrayStandardRange(pRPN, nRPN);
    }

    FormulaTokenArrayReferencesRange References() const
    {
        return FormulaTokenArrayReferencesRange(*this);
    }

    sal_uInt16     GetLen() const     { return nLen; }
    sal_uInt16     GetCodeLen() const { return nRPN; }
    FormulaError   GetCodeError() const      { return nError; }
    void      SetCodeError( FormulaError n )  { nError = n; }
    void      SetHyperLink( bool bVal ) { bHyperLink = bVal; }
    bool      IsHyperLink() const       { return bHyperLink; }

    ScRecalcMode    GetRecalcMode() const { return nMode; }
                            /** Bits aren't set directly but validated and
                                maybe handled according to priority if more
                                than one exclusive bit was set. */
            void            AddRecalcMode( ScRecalcMode nBits );

    void            ClearRecalcMode() { nMode = ScRecalcMode::NORMAL; }
    void            SetExclusiveRecalcModeNormal()
                                { SetMaskedRecalcMode( ScRecalcMode::NORMAL ); }
    void            SetExclusiveRecalcModeAlways()
                                { SetMaskedRecalcMode( ScRecalcMode::ALWAYS ); }
    void            SetExclusiveRecalcModeOnLoad()
                                { SetMaskedRecalcMode( ScRecalcMode::ONLOAD ); }
    void            SetExclusiveRecalcModeOnLoadOnce()
                                { SetMaskedRecalcMode( ScRecalcMode::ONLOAD_ONCE ); }
    void            SetRecalcModeForced()
                                { nMode |= ScRecalcMode::FORCED; }
    void            SetRecalcModeOnRefMove()
                                { nMode |= ScRecalcMode::ONREFMOVE; }
    bool            IsRecalcModeNormal() const
                                { return bool(nMode & ScRecalcMode::NORMAL); }
    bool            IsRecalcModeAlways() const
                                { return bool(nMode & ScRecalcMode::ALWAYS); }
    bool            IsRecalcModeOnLoad() const
                                { return bool(nMode & ScRecalcMode::ONLOAD); }
    bool            IsRecalcModeOnLoadOnce() const
                                { return bool(nMode & ScRecalcMode::ONLOAD_ONCE); }
    bool            IsRecalcModeForced() const
                                { return bool(nMode & ScRecalcMode::FORCED); }
    bool            IsRecalcModeOnRefMove() const
                                { return bool(nMode & ScRecalcMode::ONREFMOVE); }

                            /** Get OpCode of the most outer function */
    inline OpCode           GetOuterFuncOpCode();

                            /** Operators +,-,*,/,^,&,=,<>,<,>,<=,>=
                                with DoubleRef in Formula? */
    bool                    HasMatrixDoubleRefOps();

    virtual FormulaToken* AddOpCode(OpCode e);

    /** Adds the single token to array.
        Derived classes must override it when they want to support derived classes from FormulaToken.
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

    /** Clones the token and then adds the clone to the pCode array.
        For just new'ed tokens use Add() instead of cloning it again.
        Use this AddToken() when adding a token from another origin.
     */
    FormulaToken* AddToken( const FormulaToken& );

    FormulaToken* AddString( const svl::SharedString& rStr );
    FormulaToken* AddDouble( double fVal );
    void          AddExternal( const sal_Unicode* pStr );
    /** Xcl import may play dirty tricks with OpCode!=ocExternal.
        Others don't use! */
    FormulaToken* AddExternal( const OUString& rStr, OpCode eOp = ocExternal );
    FormulaToken* AddBad( const OUString& rStr );          /// ocBad with OUString
    FormulaToken* AddStringXML( const OUString& rStr );    /// ocStringXML with OUString, temporary during import

    virtual FormulaToken* MergeArray( );

    /** Assignment with incrementing references of FormulaToken entries
        (not copied!) */
    FormulaTokenArray& operator=( const FormulaTokenArray& );

    /** Determines if this formula needs any changes to convert it to something
        previous versions of OOo could consume (Plain Old Formula, pre-ODFF, or
        also ODFF) */
    bool                NeedsPodfRewrite( const MissingConventionODF & rConv );

    /** Determines if this formula needs any changes to convert it to OOXML. */
    bool                NeedsOoxmlRewrite();

    /** Rewrites to Plain Old Formula or OOXML, substituting missing parameters. The
        FormulaTokenArray* returned is new'ed. */
    FormulaTokenArray*  RewriteMissing( const MissingConvention & rConv );

    /** Determines if this formula may be followed by a reference. */
    bool                MayReferenceFollow();

    /** Re-intern SharedString in case the SharedStringPool differs. */
    void ReinternStrings( svl::SharedStringPool& rPool );
};

inline OpCode FormulaTokenArray::GetOuterFuncOpCode()
{
    if ( pRPN && nRPN )
        return pRPN[nRPN-1]->GetOpCode();
    return ocNone;
}

inline FormulaTokenArrayReferencesIterator FormulaTokenArrayReferencesRange::begin()
{
    return FormulaTokenArrayReferencesIterator(mrFTA.Tokens());
}

inline FormulaTokenArrayReferencesIterator FormulaTokenArrayReferencesRange::end()
{
    return FormulaTokenArrayReferencesIterator::endOf(mrFTA.Tokens());
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

    std::vector<Item> maStack;

public:
    FormulaTokenIterator( const FormulaTokenArray& );
   ~FormulaTokenIterator();
    void    Reset();
    const   FormulaToken* Next();
    const   FormulaToken* PeekNextOperator();
    bool    IsEndOfPath() const;    /// if a jump or subroutine path is done
    bool    HasStacked() const { return maStack.size() > 1; }
    short   GetPC() const { return maStack.back().nPC; }

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
            ocClose is encountered, which are only present in ocIf or ocChoose
            jumps.
      */
    void Jump( short nStart, short nNext, short nStop = SHRT_MAX );
    void Push( const FormulaTokenArray* );
    void Pop();

private:
    const FormulaToken* GetNonEndOfPathToken( short nIdx ) const;
};

// For use in SAL_INFO, SAL_WARN etc

template<typename charT, typename traits>
inline std::basic_ostream<charT, traits> & operator <<(std::basic_ostream<charT, traits> & stream, const FormulaTokenArray& point)
{
    stream <<
        static_cast<const void*>(&point) <<
        ":{nLen=" << point.GetLen() <<
        ",nRPN=" << point.GetCodeLen() <<
        ",pCode=" << static_cast<void*>(point.GetArray()) <<
        ",pRPN=" << static_cast<void*>(point.GetCode()) <<
        "}";

    return stream;
}

class FORMULA_DLLPUBLIC FormulaTokenArrayPlainIterator
{
private:
    const FormulaTokenArray* mpFTA;
    sal_uInt16 mnIndex;                 // Current step index

public:
    FormulaTokenArrayPlainIterator( const FormulaTokenArray& rFTA ) :
        mpFTA( &rFTA ),
        mnIndex( 0 )
    {
    }

    void Reset()
    {
        mnIndex = 0;
    }

    sal_uInt16 GetIndex() const
    {
        return mnIndex;
    }

    FormulaToken* First()
    {
        mnIndex = 0;
        return Next();
    }

    void Jump(sal_uInt16 nIndex)
    {
        mnIndex = nIndex;
    }

    void StepBack()
    {
        assert(mnIndex > 0);
        mnIndex--;
    }

    FormulaToken* Next();
    FormulaToken* NextNoSpaces();
    FormulaToken* GetNextName();
    FormulaToken* GetNextReference();
    FormulaToken* GetNextReferenceRPN();
    FormulaToken* GetNextReferenceOrName();
    FormulaToken* GetNextColRowName();
    FormulaToken* PeekNext();
    FormulaToken* PeekPrevNoSpaces() const;    /// Only after Reset/First/Next/Last/Prev!
    FormulaToken* PeekNextNoSpaces() const;    /// Only after Reset/First/Next/Last/Prev!

    FormulaToken* FirstRPN()
    {
        mnIndex = 0;
        return NextRPN();
    }

    FormulaToken* NextRPN();

    FormulaToken* LastRPN()
    {
        mnIndex = mpFTA->GetCodeLen();
        return PrevRPN();
    }

    FormulaToken* PrevRPN();

    void AfterRemoveToken( sal_uInt16 nOffset, sal_uInt16 nCount );
};


} // formula

#endif // INCLUDED_FORMULA_TOKENARRAY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
