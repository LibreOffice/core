/*************************************************************************
 *
 *  $RCSfile: token.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: er $ $Date: 2001-02-22 15:19:58 $
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

#ifndef SC_TOKEN_HXX
#define SC_TOKEN_HXX

#include <string.h>     // memcpy(), don't use memory.h (MAC)

#ifndef SC_OPCODE_HXX
#include "opcode.hxx"
#endif

#ifndef SC_REFDATA_HXX
#include "refdata.hxx"
#endif


enum StackVar
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

    svMissing = 0x70,                   // 0 or ""
    svErr                               // unknown StackType
};


class ScMatrix;

class ScToken
{
private:

            OpCode              eOp;            // OpCode
            const StackVar      eType;          // type of data
            USHORT              nRefCnt;        // reference count

                                // not implemented, prevent usage
                                ScToken();
            ScToken&            operator=( const ScToken& );

protected:

    static  SingleRefData       aDummySingleRef;
    static  ComplRefData        aDummyDoubleRef;
    static  String              aDummyString;

                                ScToken( OpCode eOpP, StackVar eTypeP ) :
                                    eOp( eOpP ), eType( eTypeP ), nRefCnt(0) {}
                                ScToken( const ScToken& r ) :
                                    eOp( r.eOp ), eType( r.eType ), nRefCnt(0) {}

public:

    virtual                     ~ScToken();

    inline  void                Delete()                { delete this; }
    inline  const StackVar      GetType() const         { return eType; }
    inline  OpCode              GetOpCode() const       { return eOp;   }
            BOOL                IsFunction() const; // pure functions, no operators
            BOOL                IsMatrixFunction() const;   // if a function _always_ returns a Matrix
            BYTE                GetParamCount() const;
    inline  void                NewOpCode( OpCode e )   { eOp = e; }
    inline  void                IncRef()                { nRefCnt++;       }
    inline  void                DecRef()                { if( !--nRefCnt ) Delete(); }
    inline  USHORT              GetRef() const          { return nRefCnt; }

    // Dummy methods to avoid switches and casts where possible,
    // the real token classes have to overload the appropriate method[s].
    // The only method valid anytime if not overloaded is GetByte() since
    // this represents the count of parameters to a function which of course
    // is 0 on non-functions. ScByteToken and ScExternal do overload it.
    // Any other non-overloaded method pops up an assertion.
    virtual BYTE                GetByte() const;
    virtual void                SetByte( BYTE n );
    virtual double              GetDouble() const;
    virtual const String&       GetString() const;
    virtual const SingleRefData&    GetSingleRef() const;
    virtual SingleRefData&      GetSingleRef();
    virtual const ComplRefData& GetDoubleRef() const;
    virtual ComplRefData&       GetDoubleRef();
    virtual void                CalcAbsIfRel( const ScAddress& );
    virtual void                CalcRelFromAbs( const ScAddress& );
    virtual ScMatrix*           GetMatrix() const;
    virtual USHORT              GetIndex() const;
    virtual void                SetIndex( USHORT n );
    virtual short*              GetJump() const;
    virtual const String&       GetExternal() const;
    virtual BYTE*               GetUnknown() const;

            ScToken*            Clone() const;

            void                Store( SvStream& ) const;

    virtual BOOL                operator==( const ScToken& rToken ) const;
            BOOL                TextEqual( const ScToken& rToken ) const;

    static  size_t              GetStrLenBytes( xub_StrLen nLen )
                                    { return nLen * sizeof(sal_Unicode); }
    static  size_t              GetStrLenBytes( const String& rStr )
                                    { return GetStrLenBytes( rStr.Len() ); }
};


class ScByteToken : public ScToken
{
private:
            BYTE                nByte;
public:
                                ScByteToken( OpCode e, BYTE n ) :
                                    ScToken( e, svByte ), nByte( n ) {}
                                ScByteToken( OpCode e ) :
                                    ScToken( e, svByte ), nByte( 0 ) {}
                                ScByteToken( const ScByteToken& r ) :
                                    ScToken( r ), nByte( r.nByte ) {}
    virtual BYTE                GetByte() const;
    virtual void                SetByte( BYTE n );
    virtual BOOL                operator==( const ScToken& rToken ) const;

    DECL_FIXEDMEMPOOL_NEWDEL( ScByteToken );
};


class ScDoubleToken : public ScToken
{
private:
            double              fDouble;
public:
                                ScDoubleToken( double f ) :
                                    ScToken( ocPush, svDouble ), fDouble( f ) {}
                                ScDoubleToken( OpCode e, double f ) :
                                    ScToken( e, svDouble ), fDouble( f ) {}
                                ScDoubleToken( const ScDoubleToken& r ) :
                                    ScToken( r ), fDouble( r.fDouble ) {}
    virtual double              GetDouble() const;
    virtual BOOL                operator==( const ScToken& rToken ) const;

    DECL_FIXEDMEMPOOL_NEWDEL( ScDoubleToken );
};


class ScStringToken : public ScToken
{
private:
            String              aString;
public:
                                ScStringToken( const String& r ) :
                                    ScToken( ocPush, svString ), aString( r ) {}
                                ScStringToken( OpCode e, const String& r ) :
                                    ScToken( e, svString ), aString( r ) {}
                                ScStringToken( const ScStringToken& r ) :
                                    ScToken( r ), aString( r.aString ) {}
    virtual const String&       GetString() const;
    virtual BOOL                operator==( const ScToken& rToken ) const;

    DECL_FIXEDMEMPOOL_NEWDEL( ScStringToken );
};


class ScSingleRefToken : public ScToken
{
private:
            SingleRefData       aSingleRef;
public:
                                ScSingleRefToken( const SingleRefData& r ) :
                                    ScToken( ocPush, svSingleRef ), aSingleRef( r ) {}
                                ScSingleRefToken( OpCode e, const SingleRefData& r ) :
                                    ScToken( e, svSingleRef ), aSingleRef( r ) {}
                                ScSingleRefToken( const ScSingleRefToken& r ) :
                                    ScToken( r ), aSingleRef( r.aSingleRef ) {}
    virtual const SingleRefData&    GetSingleRef() const;
    virtual SingleRefData&      GetSingleRef();
    virtual void                CalcAbsIfRel( const ScAddress& );
    virtual void                CalcRelFromAbs( const ScAddress& );
    virtual BOOL                operator==( const ScToken& rToken ) const;

    DECL_FIXEDMEMPOOL_NEWDEL( ScSingleRefToken );
};


class ScDoubleRefToken : public ScToken
{
private:
            ComplRefData        aDoubleRef;
public:
                                ScDoubleRefToken( const ComplRefData& r ) :
                                    ScToken( ocPush, svDoubleRef ), aDoubleRef( r ) {}
                                ScDoubleRefToken( OpCode e, const ComplRefData& r ) :
                                    ScToken( e, svDoubleRef ), aDoubleRef( r ) {}
                                ScDoubleRefToken( OpCode e, const SingleRefData& r ) :
                                    ScToken( e, svDoubleRef )
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
    virtual void                CalcAbsIfRel( const ScAddress& );
    virtual void                CalcRelFromAbs( const ScAddress& );
    virtual BOOL                operator==( const ScToken& rToken ) const;

    DECL_FIXEDMEMPOOL_NEWDEL( ScDoubleRefToken );
};


class ScMatrixToken : public ScToken
{
private:
            ScMatrix*           pMatrix;
public:
                                ScMatrixToken( ScMatrix* p ) :
                                    ScToken( ocPush, svMatrix ), pMatrix( p ) {}
                                ScMatrixToken( OpCode e, ScMatrix* p ) :
                                    ScToken( e, svMatrix ), pMatrix( p ) {}
                                ScMatrixToken( const ScMatrixToken& r ) :
                                    ScToken( r ), pMatrix( r.pMatrix ) {}
    virtual ScMatrix*           GetMatrix() const;
    virtual BOOL                operator==( const ScToken& rToken ) const;
};


class ScIndexToken : public ScToken
{
private:
            USHORT              nIndex;
public:
                                ScIndexToken( OpCode e, USHORT n ) :
                                    ScToken( e, svIndex ), nIndex( n ) {}
                                ScIndexToken( const ScIndexToken& r ) :
                                    ScToken( r ), nIndex( r.nIndex ) {}
    virtual USHORT              GetIndex() const;
    virtual void                SetIndex( USHORT n );
    virtual BOOL                operator==( const ScToken& rToken ) const;
};


class ScJumpToken : public ScToken
{
private:
            short*              pJump;
public:
                                ScJumpToken( OpCode e, short* p ) :
                                    ScToken( e, svJump )
                                {
                                    pJump = new short[ p[0] + 1 ];
                                    memcpy( pJump, p, (p[0] + 1) * sizeof(short) );
                                }
                                ScJumpToken( const ScJumpToken& r ) :
                                    ScToken( r )
                                {
                                    pJump = new short[ r.pJump[0] + 1 ];
                                    memcpy( pJump, r.pJump, (r.pJump[0] + 1) * sizeof(short) );
                                }
    virtual                     ~ScJumpToken();
    virtual short*              GetJump() const;
    virtual BOOL                operator==( const ScToken& rToken ) const;
};


class ScExternalToken : public ScToken
{
private:
            String              aExternal;
            BYTE                nByte;
public:
                                ScExternalToken( OpCode e, BYTE n, const String& r ) :
                                    ScToken( e, svExternal ), nByte( n ),
                                    aExternal( r ) {}
                                ScExternalToken( OpCode e, const String& r ) :
                                    ScToken( e, svExternal ), nByte( 0 ),
                                    aExternal( r ) {}
                                ScExternalToken( const ScExternalToken& r ) :
                                    ScToken( r ), nByte( r.nByte ),
                                    aExternal( r.aExternal ) {}
    virtual const String&       GetExternal() const;
    virtual BYTE                GetByte() const;
    virtual void                SetByte( BYTE n );
    virtual BOOL                operator==( const ScToken& rToken ) const;
};


class ScMissingToken : public ScToken
{
public:
                                ScMissingToken() :
                                    ScToken( ocPush, svMissing ) {}
                                ScMissingToken( OpCode e ) :
                                    ScToken( e, svMissing ) {}
                                ScMissingToken( const ScMissingToken& r ) :
                                    ScToken( r ) {}
    virtual double              GetDouble() const;
    virtual const String&       GetString() const;
    virtual BOOL                operator==( const ScToken& rToken ) const;
};


class ScErrToken : public ScToken
{
public:
                                ScErrToken( OpCode e ) :
                                    ScToken( e, svErr ) {}
                                ScErrToken( const ScErrToken& r ) :
                                    ScToken( r ) {}
    virtual BOOL                operator==( const ScToken& rToken ) const;
};


class ScUnknownToken : public ScToken
{
private:
            BYTE*               pUnknown;
public:
                                ScUnknownToken( OpCode e, StackVar v, BYTE* p ) :
                                    ScToken( e, v )
                                {
                                    pUnknown = new BYTE[ int(p[0]) + 1 ];
                                    memcpy( pUnknown, p, int(p[0]) + 1 );
                                }
                                ScUnknownToken( const ScUnknownToken& r ) :
                                    ScToken( r )
                                {
                                    pUnknown = new BYTE[ int(r.pUnknown[0]) + 1 ];
                                    memcpy( pUnknown, r.pUnknown, int(r.pUnknown[0]) + 1 );
                                }
    virtual                     ~ScUnknownToken();
    virtual BYTE*               GetUnknown() const;
    virtual BOOL                operator==( const ScToken& rToken ) const;
};


class ScTokenRef
{
    ScToken* p;
public:
    inline ScTokenRef() { p = NULL; }
    inline ScTokenRef( const ScTokenRef& r ) { if( ( p = r.p ) != NULL ) p->IncRef(); }
    inline ScTokenRef( ScToken *t )          { if( ( p = t ) != NULL ) t->IncRef(); }
    inline void Clear()                      { if( p ) p->DecRef(); }
    inline ~ScTokenRef()                     { if( p ) p->DecRef(); }
    inline ScTokenRef& operator=( const ScTokenRef& r ) { return *this = r.p; }
    inline ScTokenRef& operator=( ScToken* t )
    { if( t ) t->IncRef(); if( p ) p->DecRef(); p = t; return *this; }
    inline BOOL Is() const                  { return p != NULL; }
    inline BOOL operator ! () const         { return p == NULL; }
    inline ScToken* operator&() const       { return p; }
    inline ScToken* operator->() const      { return p; }
    inline ScToken& operator*() const       { return *p; }
    inline operator ScToken*() const        { return p; }
};


// Simplify argument passing to RefUpdate methods with ScSingleRefToken or
// ScDoubleRefToken
class SingleDoubleRefModifier
{
    ComplRefData    aDub;
    SingleRefData&  rS;
    ComplRefData&   rD;
    BOOL            bSingle;

public:
                SingleDoubleRefModifier( SingleRefData& r )
                        : rS( r ), rD( aDub ), bSingle( TRUE )
                    { aDub.Ref1 = aDub.Ref2 = rS; }
                SingleDoubleRefModifier( ComplRefData& r )
                        : rD( r ), rS( r.Ref1 ), bSingle( FALSE )
                    {}
                ~SingleDoubleRefModifier()
                    { if ( bSingle ) rS = rD.Ref1; }
    inline  ComplRefData& Ref() { return rD; }
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
