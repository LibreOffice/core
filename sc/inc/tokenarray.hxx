/*************************************************************************
 *
 *  $RCSfile: tokenarray.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $  $Date: 2004-06-04 10:17:30 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_TOKENARRAY_HXX
#define SC_TOKENARRAY_HXX

#ifndef SC_TOKEN_HXX
#include "token.hxx"
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

enum ScRecalcMode40
{                           // old RecalcMode up to and including SO 4.0
    RC_NORMAL,
    RC_ALWAYS,
    RC_ONLOAD,              // always after load
    RC_ONLOAD_ONCE,         // once after load
    RC_FORCED,              // also if cell isn't visible
    RC_ONREFMOVE            // if reference was moved (since SP3, 05.11.97)
};

// New (since 5.0, 14.01.98) RecalcMode access only via TokenArray
// SetRecalcMode / IsRecalcMode...

typedef BYTE ScRecalcMode;
// Only one of the exclusive bits can be set,
// handled by TokenArray SetRecalcMode... methods
#define RECALCMODE_NORMAL       0x01    // exclusive
#define RECALCMODE_ALWAYS       0x02    // exclusive, always
#define RECALCMODE_ONLOAD       0x04    // exclusive, always after load
#define RECALCMODE_ONLOAD_ONCE  0x08    // exclusive, once after load
#define RECALCMODE_FORCED       0x10    // combined, also if cell isn't visible
#define RECALCMODE_ONREFMOVE    0x20    // combined, if reference was moved
#define RECALCMODE_EMASK        0x0F    // mask of exclusive bits
// If new bits are to be defined, ExportRecalcMode40 and AddRecalcMode have to
// be adjusted!

struct ScRawToken;

class ScTokenArray
{
    friend class ScCompiler;
    friend class ScTokenIterator;

    ScToken**       pCode;                  // Token code array
    ScToken**       pRPN;                   // RPN array
    USHORT          nLen;                   // Length of token array
    USHORT          nRPN;                   // Length of RPN array
    USHORT          nIndex;                 // Current step index
    USHORT          nError;                 // Error code
    short           nRefs;                  // Count of cell references
    ScRecalcMode    nMode;                  // Flags to indicate when to recalc this code
    BOOL            bReplacedSharedFormula; // If code was created by replacing
                            // a shared formula, a temporary flag during
                            // UpdateReference() until StartListeningTo()

    void                    Assign( const ScTokenArray& );

    ScToken*                Add( ScToken* );
    void                    ImportRecalcMode40( ScRecalcMode40 );
    ScRecalcMode40          ExportRecalcMode40() const;
    inline  void            SetCombinedBitsRecalcMode( ScRecalcMode nBits )
                            { nMode |= (nBits & ~RECALCMODE_EMASK); }
    inline  ScRecalcMode    GetCombinedBitsRecalcMode() const
                            { return nMode & ~RECALCMODE_EMASK; }
                            /** Exclusive bits already set in nMode are
                                zero'ed, nVal may contain combined bits, but
                                only one exclusive bit may be set! */
    inline  void            SetMaskedRecalcMode( ScRecalcMode nBits )
                            { nMode = GetCombinedBitsRecalcMode() | nBits; }

    BOOL                    ImplGetReference( ScRange& rRange, BOOL bValidOnly ) const;

public:
    ScTokenArray();
    /// Assignment with references to ScToken entries (not copied!)
    ScTokenArray( const ScTokenArray& );
   ~ScTokenArray();
    ScTokenArray* Clone() const;    /// True copy!
    void Clear();
    void DelRPN();
    ScToken* First() { nIndex = 0; return Next(); }
    ScToken* Next();
    ScToken* FirstNoSpaces() { nIndex = 0; return NextNoSpaces(); }
    ScToken* NextNoSpaces();
    ScToken* GetNextName();
    ScToken* GetNextDBArea();
    ScToken* GetNextReference();
    ScToken* GetNextReferenceRPN();
    ScToken* GetNextReferenceOrName();
    ScToken* GetNextColRowName();
    ScToken* GetNextOpCodeRPN( OpCode );
    ScToken* PeekNext();
    ScToken* PeekPrevNoSpaces();    /// Only after Reset/First/Next/Last/Prev!
    ScToken* PeekNextNoSpaces();    /// Only after Reset/First/Next/Last/Prev!
    ScToken* FirstRPN() { nIndex = 0; return NextRPN(); }
    ScToken* NextRPN();
    ScToken* LastRPN() { nIndex = nRPN; return PrevRPN(); }
    ScToken* PrevRPN();

    BOOL    HasOpCodeRPN( OpCode ) const;
    /// Token of type svIndex
    BOOL    HasName() const;
    /// Token of type svIndex or opcode ocColRowName
    BOOL    HasNameOrColRowName() const;
    /// Exactly and only one range (valid or deleted)
    BOOL    IsReference( ScRange& rRange ) const;
    /// Exactly and only one valid range (no #REF!s)
    BOOL    IsValidReference( ScRange& rRange ) const;
    /// Exactly and only one multiple operation
    BOOL    GetTableOpRefs(
                ScAddress& rFormula,
                ScAddress& rColFirstPos, ScAddress& rColRelPos,
                ScAddress& rRowFirstPos, ScAddress& rRowRelPos,
                BOOL& rbIsMode2 ) const;

    ScToken** GetArray() const  { return pCode; }
    ScToken** GetCode()  const  { return pRPN; }
    USHORT    GetLen() const     { return nLen; }
    USHORT    GetCodeLen() const { return nRPN; }
    void      Reset()            { nIndex = 0; }
    USHORT    GetError() const { return nError; }
    void      SetError( USHORT n ) { nError = n; }
    short     GetRefs()  const { return nRefs;  }
    void      SetReplacedSharedFormula( BOOL bVal ) { bReplacedSharedFormula = bVal; }
    BOOL      IsReplacedSharedFormula() const       { return bReplacedSharedFormula; }

    inline  ScRecalcMode    GetRecalcMode() const { return nMode; }
                            /** Bits aren't set directly but validated and
                                maybe handled according to priority if more
                                than one exclusive bit was set. */
            void            AddRecalcMode( ScRecalcMode nBits );

    inline  void            ClearRecalcMode() { nMode = RECALCMODE_NORMAL; }
    inline  void            SetRecalcModeNormal()
                                { SetMaskedRecalcMode( RECALCMODE_NORMAL ); }
    inline  void            SetRecalcModeAlways()
                                { SetMaskedRecalcMode( RECALCMODE_ALWAYS ); }
    inline  void            SetRecalcModeOnLoad()
                                { SetMaskedRecalcMode( RECALCMODE_ONLOAD ); }
    inline  void            SetRecalcModeOnLoadOnce()
                                { SetMaskedRecalcMode( RECALCMODE_ONLOAD_ONCE ); }
    inline  void            SetRecalcModeForced()
                                { nMode |= RECALCMODE_FORCED; }
    inline  void            ClearRecalcModeForced()
                                { nMode &= ~RECALCMODE_FORCED; }
    inline  void            SetRecalcModeOnRefMove()
                                { nMode |= RECALCMODE_ONREFMOVE; }
    inline  void            ClearRecalcModeOnRefMove()
                                { nMode &= ~RECALCMODE_ONREFMOVE; }
    inline  BOOL            IsRecalcModeNormal() const
                                { return (nMode & RECALCMODE_NORMAL) != 0; }
    inline  BOOL            IsRecalcModeAlways() const
                                { return (nMode & RECALCMODE_ALWAYS) != 0; }
    inline  BOOL            IsRecalcModeOnLoad() const
                                { return (nMode & RECALCMODE_ONLOAD) != 0; }
    inline  BOOL            IsRecalcModeOnLoadOnce() const
                                { return (nMode & RECALCMODE_ONLOAD_ONCE) != 0; }
    inline  BOOL            IsRecalcModeForced() const
                                { return (nMode & RECALCMODE_FORCED) != 0; }
    inline  BOOL            IsRecalcModeOnRefMove() const
                                { return (nMode & RECALCMODE_ONREFMOVE) != 0; }

                            /** Get OpCode of the most outer function */
    inline OpCode           GetOuterFuncOpCode();
                            /** Determines the extent of direct adjacent
                                references. Only use with real functions, e.g.
                                GetOuterFuncOpCode() == ocSum ! */
    BOOL                    GetAdjacentExtendOfOuterFuncRefs( SCCOLROW& nExtend,
                                const ScAddress& rPos, ScDirection );

                            /** Operators +,-,*,/,^,&,=,<>,<,>,<=,>=
                                with DoubleRef in Formula? */
    BOOL                    HasMatrixDoubleRefOps();

    void Load30( SvStream&, const ScAddress& );
    void Load( SvStream&, USHORT, const ScAddress& );
    void Store( SvStream&, const ScAddress& ) const;

    ScToken* AddToken( const ScRawToken& );
    ScToken* AddToken( const ScToken& );
    ScToken* AddOpCode( OpCode eCode );
    ScToken* AddString( const sal_Unicode* pStr );
    ScToken* AddString( const String& rStr );
    ScToken* AddDouble( double fVal );
    ScToken* AddSingleReference( const SingleRefData& rRef );
    ScToken* AddDoubleReference( const ComplRefData& rRef );
    ScToken* AddName( USHORT n );
    ScToken* AddMatrix( ScMatrix* p );
    ScToken* AddExternal( const sal_Unicode* pStr );
    ScToken* AddExternal( const String& rStr );
    ScToken* AddColRowName( const SingleRefData& rRef );
    ScToken* AddBad( const sal_Unicode* pStr );     /// ocBad with String
    ScToken* AddBad( const String& rStr );          /// ocBad with String

    /// Assignment with references to ScToken entries (not copied!)
    ScTokenArray& operator=( const ScTokenArray& );

    /// Make 3D references point to old referenced position even if relative
            void            ReadjustRelative3DReferences(
                                const ScAddress& rOldPos,
                                const ScAddress& rNewPos );
};

inline OpCode ScTokenArray::GetOuterFuncOpCode()
{
    if ( pRPN && nRPN )
        return pRPN[nRPN-1]->GetOpCode();
    return ocNone;
}

struct ImpTokenIterator
{
    ImpTokenIterator* pNext;
    const ScTokenArray* pArr;
    short nPC;
    short nStop;

    DECL_FIXEDMEMPOOL_NEWDEL( ImpTokenIterator );
};

class ScTokenIterator
{
    friend class ScInterpreter;     // for Jump()
    ImpTokenIterator* pCur;
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
    void Push( const ScTokenArray* );
    void Pop();
public:
    ScTokenIterator( const ScTokenArray& );
   ~ScTokenIterator();
    void    Reset();
    const   ScToken* First();
    const   ScToken* Next();
    bool    IsEndOfPath() const;    /// if a jump or subroutine path is done
    bool    HasStacked() const { return pCur->pNext != 0; }
    short   GetPC() const { return pCur->nPC; }
};


#endif // SC_TOKENARRAY_HXX

