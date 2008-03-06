/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: token.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 15:20:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_TOKEN_HXX
#define SC_TOKEN_HXX

#include <memory>
#include <vector>

#ifndef SC_OPCODE_HXX
#include "opcode.hxx"
#endif

#ifndef SC_REFDATA_HXX
#include "refdata.hxx"
#endif

#ifndef SC_MATRIX_HXX
#include "scmatrix.hxx"
#endif

#ifndef SC_INTRUREF_HXX
#include "intruref.hxx"
#endif

#ifndef _SVMEMPOOL_HXX
#include <tools/mempool.hxx>
#endif


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

    svError,                            // error token
    svMissing = 0x70,                   // 0 or ""
    svUnknown                           // unknown StackType
};

#ifdef PRODUCT
// save memory since compilers tend to int an enum
typedef BYTE StackVar;
#else
// have enum names in debugger
typedef StackVarEnum StackVar;
#endif


class ScJumpMatrix;

class ScToken;
typedef ScSimpleIntrusiveReference< class ScToken > ScTokenRef;
typedef ScSimpleIntrusiveReference< const class ScToken > ScConstTokenRef;

typedef ::std::vector< ComplRefData > ScRefList;


class ScToken
{
private:

            const StackVar      eType;          // type of data
            mutable USHORT      nRefCnt;        // reference count

                                // not implemented, prevent usage
                                ScToken();
            ScToken&            operator=( const ScToken& );

protected:

    static  String              aDummyString;

                                ScToken( StackVar eTypeP ) :
                                    eType( eTypeP ), nRefCnt(0) {}
                                ScToken( const ScToken& r ) :
                                    eType( r.eType ), nRefCnt(0) {}

public:

    virtual                     ~ScToken();

    inline  void                Delete()                { delete this; }
    inline  const StackVar      GetType() const         { return eType; }
            BOOL                IsFunction() const; // pure functions, no operators
            BOOL                IsMatrixFunction() const;   // if a function _always_ returns a Matrix
            BYTE                GetParamCount() const;
    inline  void                IncRef() const          { nRefCnt++; }
    inline  void                DecRef() const
                                    {
                                        if (!--nRefCnt)
                                            const_cast<ScToken*>(this)->Delete();
                                    }
    inline  USHORT              GetRef() const          { return nRefCnt; }

    /**
        Dummy methods to avoid switches and casts where possible,
        the real token classes have to overload the appropriate method[s].
        The only methods valid anytime if not overloaded are:

        - GetOpCode() since for a token type not needing an explicit OpCode set
          the implicit OpCode is ocPush.

        - GetByte() since this represents the count of parameters to a function
          which of course is 0 on non-functions. ScByteToken and ScExternal do
          overload it.

        - HasForceArray() since also this is only used for operators and
          functions and is 0 for other tokens.

        Any other non-overloaded method pops up an assertion.
     */

    virtual OpCode              GetOpCode() const;
    virtual BYTE                GetByte() const;
    virtual void                SetByte( BYTE n );
    virtual bool                HasForceArray() const;
    virtual void                SetForceArray( bool b );
    virtual double              GetDouble() const;
    virtual double&             GetDoubleAsReference();
    virtual const String&       GetString() const;
    virtual const SingleRefData&    GetSingleRef() const;
    virtual SingleRefData&      GetSingleRef();
    virtual const ComplRefData& GetDoubleRef() const;
    virtual ComplRefData&       GetDoubleRef();
    virtual const SingleRefData&    GetSingleRef2() const;
    virtual SingleRefData&      GetSingleRef2();
    virtual void                CalcAbsIfRel( const ScAddress& );
    virtual void                CalcRelFromAbs( const ScAddress& );
    virtual const ScMatrix*     GetMatrix() const;
    virtual ScMatrix*           GetMatrix();
    virtual USHORT              GetIndex() const;
    virtual void                SetIndex( USHORT n );
    virtual short*              GetJump() const;
    virtual const String&       GetExternal() const;
    virtual ScToken*            GetFAPOrigToken() const;
    virtual ScJumpMatrix*       GetJumpMatrix() const;
    virtual const ScRefList*    GetRefList() const;
    virtual       ScRefList*    GetRefList();
    virtual USHORT              GetError() const;
    virtual void                SetError( USHORT );

            ScToken*            Clone() const;

    virtual BOOL                operator==( const ScToken& rToken ) const;
            BOOL                TextEqual( const ScToken& rToken ) const;
            BOOL                Is3DRef() const;    // reference with 3D flag set

    // If token in RPN resulted from resolving a name and contains an absolute
    // reference. Token must be obtained through ScTokenArray::GetNextReferenceRPN()
    // or similar.
            BOOL                IsRPNReferenceAbsName() const;

    /** If rTok1 and rTok2 both are SingleRef or DoubleRef tokens, extend/merge
        ranges as needed for ocRange.
        @param rPos
            The formula's position, used to calculate absolute positions from
            relative references.
        @param bReuseDoubleRef
            If TRUE, a DoubleRef token is reused if passed as rTok1 or rTok2,
            else a new DoubleRef token is created and returned.
        @return
            A reused or new'ed ScDoubleRefToken, or a NULL TokenRef if rTok1 or
            rTok2 are not of sv(Single|Double)Ref
     */
    static  ScTokenRef          ExtendRangeReference( ScToken & rTok1, ScToken & rTok2, const ScAddress & rPos, bool bReuseDoubleRef );

    static  size_t              GetStrLenBytes( xub_StrLen nLen )
                                    { return nLen * sizeof(sal_Unicode); }
    static  size_t              GetStrLenBytes( const String& rStr )
                                    { return GetStrLenBytes( rStr.Len() ); }
};


/** Tokens that need a different OpCode than ocPush are derived from this. */
class ScOpToken : public ScToken
{
private:
            OpCode              eOp;
public:
                                ScOpToken( OpCode e, StackVar v ) :
                                    ScToken( v ), eOp( e ) {}
                                ScOpToken( const ScOpToken & r ) :
                                    ScToken( r ), eOp( r.eOp ) {}
    /** This is dirty and only the compiler should use it! */
    inline  void                NewOpCode( OpCode e ) { eOp = e; }
    virtual OpCode              GetOpCode() const;

    // No operator== to be overloaded, ScToken::operator== already checks the
    // OpCode as well via GetOpCode().
};


class ScByteToken : public ScOpToken
{
private:
            BYTE                nByte;
            bool                bHasForceArray;
protected:
                                ScByteToken( OpCode e, BYTE n, StackVar v, bool b ) :
                                    ScOpToken( e, v ), nByte( n ),
                                    bHasForceArray( b ) {}
public:
                                ScByteToken( OpCode e, BYTE n, bool b ) :
                                    ScOpToken( e, svByte ), nByte( n ),
                                    bHasForceArray( b ) {}
                                ScByteToken( OpCode e, BYTE n ) :
                                    ScOpToken( e, svByte ), nByte( n ),
                                    bHasForceArray( false ) {}
                                ScByteToken( OpCode e ) :
                                    ScOpToken( e, svByte ), nByte( 0 ),
                                    bHasForceArray( false ) {}
                                ScByteToken( const ScByteToken& r ) :
                                    ScOpToken( r ), nByte( r.nByte ),
                                    bHasForceArray( r.bHasForceArray ) {}
    virtual BYTE                GetByte() const;
    virtual void                SetByte( BYTE n );
    virtual bool                HasForceArray() const;
    virtual void                SetForceArray( bool b );
    virtual BOOL                operator==( const ScToken& rToken ) const;

    DECL_FIXEDMEMPOOL_NEWDEL( ScByteToken );
};


// A special token for the FormulaAutoPilot only. Keeps a reference pointer of
// the token of which it was created for comparison.
class ScFAPToken : public ScByteToken
{
private:
            ScTokenRef          pOrigToken;
public:
                                ScFAPToken( OpCode e, BYTE n, ScToken* p ) :
                                    ScByteToken( e, n, svFAP, false ),
                                    pOrigToken( p ) {}
                                ScFAPToken( const ScFAPToken& r ) :
                                    ScByteToken( r ), pOrigToken( r.pOrigToken ) {}
    virtual ScToken*            GetFAPOrigToken() const;
    virtual BOOL                operator==( const ScToken& rToken ) const;
};


class ScDoubleToken : public ScToken
{
private:
            double              fDouble;
public:
                                ScDoubleToken( double f ) :
                                    ScToken( svDouble ), fDouble( f ) {}
                                ScDoubleToken( const ScDoubleToken& r ) :
                                    ScToken( r ), fDouble( r.fDouble ) {}
    virtual double              GetDouble() const;
    virtual double&             GetDoubleAsReference();
    virtual BOOL                operator==( const ScToken& rToken ) const;

    DECL_FIXEDMEMPOOL_NEWDEL( ScDoubleToken );
};


class ScStringToken : public ScToken
{
private:
            String              aString;
public:
                                ScStringToken( const String& r ) :
                                    ScToken( svString ), aString( r ) {}
                                ScStringToken( const ScStringToken& r ) :
                                    ScToken( r ), aString( r.aString ) {}
    virtual const String&       GetString() const;
    virtual BOOL                operator==( const ScToken& rToken ) const;

    DECL_FIXEDMEMPOOL_NEWDEL( ScStringToken );
};


/** Identical to ScStringToken, but with explicit OpCode instead of implicit
    ocPush, and an optional BYTE for ocBad tokens. */
class ScStringOpToken : public ScByteToken
{
private:
            String              aString;
public:
                                ScStringOpToken( OpCode e, const String& r ) :
                                    ScByteToken( e, 0, svString, false ), aString( r ) {}
                                ScStringOpToken( const ScStringOpToken& r ) :
                                    ScByteToken( r ), aString( r.aString ) {}
    virtual const String&       GetString() const;
    virtual BOOL                operator==( const ScToken& rToken ) const;
};


class ScSingleRefToken : public ScToken
{
private:
            SingleRefData       aSingleRef;
public:
                                ScSingleRefToken( const SingleRefData& r ) :
                                    ScToken( svSingleRef ), aSingleRef( r ) {}
                                ScSingleRefToken( const ScSingleRefToken& r ) :
                                    ScToken( r ), aSingleRef( r.aSingleRef ) {}
    virtual const SingleRefData&    GetSingleRef() const;
    virtual SingleRefData&      GetSingleRef();
    virtual void                CalcAbsIfRel( const ScAddress& );
    virtual void                CalcRelFromAbs( const ScAddress& );
    virtual BOOL                operator==( const ScToken& rToken ) const;

    DECL_FIXEDMEMPOOL_NEWDEL( ScSingleRefToken );
};


/** Identical to ScSingleRefToken, but with explicit OpCode instead of implicit
    ocPush. */
class ScSingleRefOpToken : public ScOpToken
{
private:
            SingleRefData       aSingleRef;
public:
                                ScSingleRefOpToken( OpCode e, const SingleRefData& r ) :
                                    ScOpToken( e, svSingleRef ), aSingleRef( r ) {}
                                ScSingleRefOpToken( const ScSingleRefOpToken& r ) :
                                    ScOpToken( r ), aSingleRef( r.aSingleRef ) {}
    virtual const SingleRefData&    GetSingleRef() const;
    virtual SingleRefData&      GetSingleRef();
    virtual void                CalcAbsIfRel( const ScAddress& );
    virtual void                CalcRelFromAbs( const ScAddress& );
    virtual BOOL                operator==( const ScToken& rToken ) const;
};


class ScDoubleRefToken : public ScToken
{
private:
            ComplRefData        aDoubleRef;
public:
                                ScDoubleRefToken( const ComplRefData& r ) :
                                    ScToken( svDoubleRef ), aDoubleRef( r ) {}
                                ScDoubleRefToken( const SingleRefData& r ) :
                                    ScToken( svDoubleRef )
                                {
                                    aDoubleRef.Ref1 = r;
                                    aDoubleRef.Ref2 = r;
                                }
                                ScDoubleRefToken( const ScDoubleRefToken& r ) :
                                    ScToken( r ), aDoubleRef( r.aDoubleRef ) {}
    virtual const SingleRefData&    GetSingleRef() const;
    virtual SingleRefData&      GetSingleRef();
    virtual const ComplRefData& GetDoubleRef() const;
    virtual ComplRefData&       GetDoubleRef();
    virtual const SingleRefData&    GetSingleRef2() const;
    virtual SingleRefData&      GetSingleRef2();
    virtual void                CalcAbsIfRel( const ScAddress& );
    virtual void                CalcRelFromAbs( const ScAddress& );
    virtual BOOL                operator==( const ScToken& rToken ) const;

    DECL_FIXEDMEMPOOL_NEWDEL( ScDoubleRefToken );
};


/** Identical to ScDoubleRefToken, but with explicit OpCode instead of implicit
    ocPush. */
class ScDoubleRefOpToken : public ScOpToken
{
private:
            ComplRefData        aDoubleRef;
public:
                                ScDoubleRefOpToken( OpCode e, const ComplRefData& r ) :
                                    ScOpToken( e, svDoubleRef ), aDoubleRef( r ) {}
                                ScDoubleRefOpToken( OpCode e, const SingleRefData& r ) :
                                    ScOpToken( e, svDoubleRef )
                                {
                                    aDoubleRef.Ref1 = r;
                                    aDoubleRef.Ref2 = r;
                                }
                                ScDoubleRefOpToken( const ScDoubleRefOpToken& r ) :
                                    ScOpToken( r ), aDoubleRef( r.aDoubleRef ) {}
    virtual const SingleRefData&    GetSingleRef() const;
    virtual SingleRefData&      GetSingleRef();
    virtual const ComplRefData& GetDoubleRef() const;
    virtual ComplRefData&       GetDoubleRef();
    virtual const SingleRefData&    GetSingleRef2() const;
    virtual SingleRefData&      GetSingleRef2();
    virtual void                CalcAbsIfRel( const ScAddress& );
    virtual void                CalcRelFromAbs( const ScAddress& );
    virtual BOOL                operator==( const ScToken& rToken ) const;
};


class ScMatrixToken : public ScToken
{
private:
            ScMatrixRef         pMatrix;
public:
                                ScMatrixToken( ScMatrix* p ) :
                                    ScToken( svMatrix ), pMatrix( p ) {}
                                ScMatrixToken( const ScMatrixToken& r ) :
                                    ScToken( r ), pMatrix( r.pMatrix ) {}
    virtual const ScMatrix*     GetMatrix() const;
    virtual ScMatrix*           GetMatrix();
    virtual BOOL                operator==( const ScToken& rToken ) const;
};


class ScIndexToken : public ScOpToken
{
private:
            USHORT              nIndex;
public:
                                ScIndexToken( OpCode e, USHORT n ) :
                                    ScOpToken( e, svIndex ), nIndex( n ) {}
                                ScIndexToken( const ScIndexToken& r ) :
                                    ScOpToken( r ), nIndex( r.nIndex ) {}
    virtual USHORT              GetIndex() const;
    virtual void                SetIndex( USHORT n );
    virtual BOOL                operator==( const ScToken& rToken ) const;
};


class ScJumpToken : public ScOpToken
{
private:
            short*              pJump;
public:
                                ScJumpToken( OpCode e, short* p ) :
                                    ScOpToken( e, svJump )
                                {
                                    pJump = new short[ p[0] + 1 ];
                                    memcpy( pJump, p, (p[0] + 1) * sizeof(short) );
                                }
                                ScJumpToken( const ScJumpToken& r ) :
                                    ScOpToken( r )
                                {
                                    pJump = new short[ r.pJump[0] + 1 ];
                                    memcpy( pJump, r.pJump, (r.pJump[0] + 1) * sizeof(short) );
                                }
    virtual                     ~ScJumpToken();
    virtual short*              GetJump() const;
    virtual BOOL                operator==( const ScToken& rToken ) const;
};


// Only created from within the interpreter, no conversion from ScRawToken,
// never added to ScTokenArray!
class ScJumpMatrixToken : public ScToken
{
private:
            ScJumpMatrix*       pJumpMatrix;
public:
                                ScJumpMatrixToken( ScJumpMatrix* p ) :
                                    ScToken( svJumpMatrix ), pJumpMatrix( p ) {}
                                ScJumpMatrixToken( const ScJumpMatrixToken& r ) :
                                    ScToken( r ), pJumpMatrix( r.pJumpMatrix ) {}
    virtual                     ~ScJumpMatrixToken();
    virtual ScJumpMatrix*       GetJumpMatrix() const;
    virtual BOOL                operator==( const ScToken& rToken ) const;
};


// Only created from within the interpreter, no conversion from ScRawToken,
// never added to ScTokenArray!
class ScRefListToken : public ScToken
{
private:
            ScRefList           aRefList;
public:
                                ScRefListToken() :
                                    ScToken( svRefList ) {}
                                ScRefListToken( const ScRefListToken & r ) :
                                    ScToken( r ), aRefList( r.aRefList ) {}
    virtual void                CalcAbsIfRel( const ScAddress& );
    virtual void                CalcRelFromAbs( const ScAddress& );
    virtual const ScRefList*    GetRefList() const;
    virtual       ScRefList*    GetRefList();
    virtual BOOL                operator==( const ScToken& rToken ) const;
};


class ScExternalToken : public ScOpToken
{
private:
            String              aExternal;
            BYTE                nByte;
public:
                                ScExternalToken( OpCode e, BYTE n, const String& r ) :
                                    ScOpToken( e, svExternal ), aExternal( r ),
                                    nByte( n ) {}
                                ScExternalToken( OpCode e, const String& r ) :
                                    ScOpToken( e, svExternal ), aExternal( r ),
                                    nByte( 0 ) {}
                                ScExternalToken( const ScExternalToken& r ) :
                                    ScOpToken( r ), aExternal( r.aExternal ),
                                    nByte( r.nByte ) {}
    virtual const String&       GetExternal() const;
    virtual BYTE                GetByte() const;
    virtual void                SetByte( BYTE n );
    virtual BOOL                operator==( const ScToken& rToken ) const;
};


class ScMissingToken : public ScOpToken
{
public:
                                ScMissingToken() :
                                    ScOpToken( ocMissing, svMissing ) {}
                                ScMissingToken( const ScMissingToken& r ) :
                                    ScOpToken( r ) {}
    virtual double              GetDouble() const;
    virtual const String&       GetString() const;
    virtual BOOL                operator==( const ScToken& rToken ) const;
};


class ScUnknownToken : public ScOpToken
{
public:
                                ScUnknownToken( OpCode e ) :
                                    ScOpToken( e, svUnknown ) {}
                                ScUnknownToken( const ScUnknownToken& r ) :
                                    ScOpToken( r ) {}
    virtual BOOL                operator==( const ScToken& rToken ) const;
};


class ScErrorToken : public ScToken
{
            USHORT              nError;
public:
                                ScErrorToken( USHORT nErr ) :
                                    ScToken( svError ), nError( nErr) {}
                                ScErrorToken( const ScErrorToken& r ) :
                                    ScToken( r ), nError( r.nError) {}
    virtual USHORT              GetError() const;
    virtual void                SetError( USHORT nErr );
    virtual BOOL                operator==( const ScToken& rToken ) const;
};


class ScEmptyCellToken : public ScToken
{
            bool                bInherited          :1;
            bool                bDisplayedAsString  :1;
public:
    explicit                    ScEmptyCellToken( bool bInheritedP, bool bDisplayAsString ) :
                                    ScToken( svEmptyCell ),
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
    virtual BOOL                operator==( const ScToken& rToken ) const;
};


/**  Transports the result from the interpreter to the formula cell. */
class ScMatrixCellResultToken : public ScToken
{
    // No non-const access implemented, silence down unxsols4 complaining about
    // the public GetMatrix() hiding the one from ScToken.
    virtual ScMatrix*           GetMatrix();

protected:
            ScConstMatrixRef    xMatrix;
            ScConstTokenRef     xUpperLeft;
public:
                                ScMatrixCellResultToken( ScMatrix* pMat, ScToken* pUL ) :
                                    ScToken( svMatrixCell ),
                                    xMatrix( pMat), xUpperLeft( pUL) {}
                                ScMatrixCellResultToken( const ScMatrixCellResultToken& r ) :
                                    ScToken( r ), xMatrix( r.xMatrix ),
                                    xUpperLeft( r.xUpperLeft ) {}
    virtual double              GetDouble() const;
    virtual const String &      GetString() const;
    virtual const ScMatrix*     GetMatrix() const;
    virtual BOOL                operator==( const ScToken& rToken ) const;
            StackVar            GetUpperLeftType() const
                                    {
                                        return xUpperLeft ?
                                            xUpperLeft->GetType() :
                                            static_cast<StackVar>(svUnknown);
                                    }
            ScConstTokenRef     GetUpperLeftToken() const   { return xUpperLeft; }
            void                Assign( const ScMatrixCellResultToken & r )
                                    {
                                        xMatrix = r.xMatrix;
                                        xUpperLeft = r.xUpperLeft;
                                    }
};


/** Stores the matrix result at the formula cell, additionally the range the
    matrix formula occupies. */
class ScMatrixFormulaCellToken : public ScMatrixCellResultToken
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
    virtual BOOL                operator==( const ScToken& rToken ) const;
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
            void                Assign( const ScToken & r );

                                /** Modify xUpperLeft if svDouble, or create
                                    new ScDoubleToken if not set yet. Does
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


class ScHybridCellToken : public ScToken
{
private:
            double              fDouble;
            String              aString;
            String              aFormula;
public:
                                ScHybridCellToken( double f,
                                        const String & rStr,
                                        const String & rFormula ) :
                                    ScToken( svHybridCell ),
                                    fDouble( f ), aString( rStr ),
                                    aFormula( rFormula ) {}
                                ScHybridCellToken( const ScHybridCellToken& r ) :
                                    ScToken( r ), fDouble( r.fDouble),
                                    aString( r.aString), aFormula( r.aFormula) {}
            const String &      GetFormula() const  { return aFormula; }
    virtual double              GetDouble() const;
    virtual const String &      GetString() const;
    virtual BOOL                operator==( const ScToken& rToken ) const;
};


// Simplify argument passing to RefUpdate methods with ScSingleRefToken or
// ScDoubleRefToken
class SingleDoubleRefModifier
{
    ComplRefData    aDub;
    SingleRefData*  pS;
    ComplRefData*   pD;

                // not implemented, prevent usage
                SingleDoubleRefModifier( const SingleDoubleRefModifier& );
        SingleDoubleRefModifier& operator=( const SingleDoubleRefModifier& );

public:
                SingleDoubleRefModifier( ScToken& rT )
                    {
                        if ( rT.GetType() == svSingleRef )
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
                SingleDoubleRefModifier( SingleRefData& rS )
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
    inline  ComplRefData& Ref() { return *pD; }
};

class SingleDoubleRefProvider
{
public:

    const SingleRefData&    Ref1;
    const SingleRefData&    Ref2;

                SingleDoubleRefProvider( const ScToken& r )
                        : Ref1( r.GetSingleRef() ),
                        Ref2( r.GetType() == svDoubleRef ?
                        r.GetDoubleRef().Ref2 : Ref1 )
                    {}
                SingleDoubleRefProvider( const SingleRefData& r )
                        : Ref1( r ), Ref2( r )
                    {}
                SingleDoubleRefProvider( const ComplRefData& r )
                        : Ref1( r.Ref1 ), Ref2( r.Ref2 )
                    {}
                ~SingleDoubleRefProvider()
                    {}
};

#endif
