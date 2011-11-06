/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _BASTYPE_HXX
#define _BASTYPE_HXX


#include <tools/globname.hxx>
#include <tools/gen.hxx>
#include <tools/stream.hxx>
#include <tools/unqid.hxx>
#include <tools/string.hxx>

class SvStringHashEntry;
class SvIdlDataBase;
class SvTokenStream;

/******************** class SvUINT32 **********************************/
class SvUINT32
{
    sal_uInt32  nVal;
public:
                SvUINT32() { nVal = 0; }
                SvUINT32( sal_uInt32 n ) : nVal( n ) {}
    SvUINT32 &  operator = ( sal_uInt32 n ) { nVal = n; return *this; }

    operator    sal_uInt32 &() { return nVal; }

    static sal_uInt32  Read( SvStream & rStm );
    static void    Write( SvStream & rStm, sal_uInt32 nVal );

    friend SvStream& operator << (SvStream & rStm, const SvUINT32 & r )
                { SvUINT32::Write( rStm, r.nVal ); return rStm; }
    friend SvStream& operator >> (SvStream & rStm, SvUINT32 & r )
                { r.nVal = SvUINT32::Read( rStm ); return rStm; }
};


/******************** class SvINT16 **********************************/
class SvINT16
{
    short   nVal;
public:
                SvINT16() { nVal = 0; }
                SvINT16( short n ) : nVal( n ) {}
    SvINT16 &   operator = ( short n ) { nVal = n; return *this; }

    operator    short &() { return nVal; }

    friend SvStream& operator << (SvStream & rStm, const SvINT16 & r )
                { SvUINT32::Write( rStm, (sal_uInt32)r.nVal ); return rStm; }
    friend SvStream& operator >> (SvStream & rStm, SvINT16 & r )
                { r.nVal = (short)SvUINT32::Read( rStm ); return rStm; }
};


/******************** class SvUINT16 **********************************/
class SvUINT16
{
    sal_uInt16  nVal;
public:
                SvUINT16() { nVal = 0; }
                SvUINT16( sal_uInt16 n ) : nVal( n ) {}
    SvUINT16 &  operator = ( sal_uInt16 n ) { nVal = n; return *this; }

    operator    sal_uInt16 &() { return nVal; }

    friend SvStream& operator << (SvStream & rStm, const SvUINT16 & r )
                { SvUINT32::Write( rStm, (sal_uInt32)r.nVal ); return rStm; }
    friend SvStream& operator >> (SvStream & rStm, SvUINT16 & r )
                { r.nVal = (sal_uInt16)SvUINT32::Read( rStm ); return rStm; }
};


/******************** class SvINT32 **********************************/
class SvINT32
{
    sal_Int32   nVal;
public:
                SvINT32() { nVal = 0; }
                SvINT32( sal_Int32 n ) : nVal( n ) {}
    SvINT32 &   operator = ( sal_Int32 n ) { nVal = n; return *this; }

    operator    sal_Int32 &() { return nVal; }

    friend SvStream& operator << (SvStream & rStm, const SvINT32 & r )
                { SvUINT32::Write( rStm, (sal_uInt32)r.nVal ); return rStm; }
    friend SvStream& operator >> (SvStream & rStm, SvINT32 & r )
                { r.nVal = (sal_Int32)SvUINT32::Read( rStm ); return rStm; }
};


/******************** class Svint **********************************/
class Svint
{
    int     nVal;
    sal_Bool    bSet;
public:
                Svint() { nVal = bSet = 0; }
                Svint( int n ) : nVal( n ), bSet( sal_True ) {}
                Svint( int n, sal_Bool bSetP ) : nVal( n ), bSet( bSetP ) {}
    Svint    &  operator = ( int n ) { nVal = n; bSet = sal_True; return *this; }

    operator    int ()const { return nVal; }
    sal_Bool        IsSet() const { return bSet; }

    friend SvStream& operator << (SvStream & rStm, const Svint & r )
                { SvUINT32::Write( rStm, (sal_uInt32)r.nVal ); rStm << r.bSet; return rStm; }
    friend SvStream& operator >> (SvStream & rStm, Svint & r )
                { r.nVal = (int)SvUINT32::Read( rStm ); rStm >> r.bSet ; return rStm; }
};


/******************** class SvBOOL **********************************/
class SvBOOL
{
    sal_Bool  nVal:1,
          bSet:1;
public:
                SvBOOL() { bSet = nVal = sal_False; }
                SvBOOL( sal_Bool n ) : nVal( n ), bSet( sal_True ) {}
                SvBOOL( sal_Bool n, sal_Bool bSetP ) : nVal( n ), bSet( bSetP ) {}
    SvBOOL &    operator = ( sal_Bool n ) { nVal = n; bSet = sal_True; return *this; }

    operator    sal_Bool() const { return nVal; }
#ifdef STC
    operator    int() const { return nVal; }
#endif
    sal_Bool        Is() const { return nVal; }
    sal_Bool        IsSet() const { return bSet; }

    friend SvStream& operator << (SvStream &, const SvBOOL &);
    friend SvStream& operator >> (SvStream &, SvBOOL &);

#ifdef IDL_COMPILER
    sal_Bool        ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
    sal_Bool        WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm );
    ByteString      GetSvIdlString( SvStringHashEntry * pName );
#endif
};


/******************** class SvIdentifier **********************************/
class SvIdentifier : public ByteString
{
public:
                SvIdentifier(){};
    SvIdentifier & operator = ( const ByteString & rStr )
                { ByteString::operator =( rStr ); return *this; }
    friend SvStream& operator << (SvStream &, const SvIdentifier &);
    friend SvStream& operator >> (SvStream &, SvIdentifier &);

    sal_Bool        IsSet() const { return Len() != 0; }
#ifdef IDL_COMPILER
    sal_Bool        ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
    sal_Bool        WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm,
                            sal_uInt16 nTab );
#endif
};


/******************** class SvIdentifier **********************************/
class SvNumberIdentifier : public SvIdentifier
{
    sal_uInt32  nValue;
    // darf nicht benutzt werden
    sal_Bool    ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
public:
                SvNumberIdentifier() : nValue( 0 ) {};
    sal_Bool        IsSet() const
                {
                    return SvIdentifier::IsSet() || nValue != 0;
                }
    sal_uInt32      GetValue() const { return nValue; }
    void        SetValue( sal_uInt32 nVal ) { nValue = nVal; }

    friend SvStream& operator << (SvStream &, const SvNumberIdentifier &);
    friend SvStream& operator >> (SvStream &, SvNumberIdentifier &);
#ifdef IDL_COMPILER
    sal_Bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    sal_Bool        ReadSvIdl( SvIdlDataBase &, SvStringHashEntry * pName,
                           SvTokenStream & rInStm );
#endif
};


/******************** class SvString **********************************/
class SvString : public ByteString
{
public:
                SvString(){};
    SvString &  operator = ( const ByteString & rStr )
                { ByteString::operator =( rStr ); return *this; }
    sal_Bool        IsSet() const { return Len() != 0; }
    friend SvStream& operator << (SvStream &, const SvString &);
    friend SvStream& operator >> (SvStream &, SvString &);

#ifdef IDL_COMPILER
    sal_Bool        ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
    sal_Bool        WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm,
                            sal_uInt16 nTab );
#endif
};


/******************** class SvHelpText **********************************/
class SvHelpText : public SvString
{
public:
                SvHelpText() {}
#ifdef IDL_COMPILER
    sal_Bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    sal_Bool        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                            sal_uInt16 nTab );
#endif
};


/******************** class SvHelpContext *******************************/
class SvHelpContext : public SvNumberIdentifier
{
};

/******************** class SvUUId *************************************/
class SvUUId : public SvGlobalName
{
public:
                SvUUId() {}
#ifdef IDL_COMPILER
    sal_Bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    sal_Bool        WriteSvIdl( SvStream & rOutStm );
#endif
};


/******************** class SvVersion **********************************/
class SvVersion
{
    sal_uInt16  nMajorVersion;
    sal_uInt16  nMinorVersion;
public:
                SvVersion() : nMajorVersion( 1 ), nMinorVersion( 0 ) {}
    sal_Bool        operator == ( const SvVersion & r )
                {
                    return (r.nMajorVersion == nMajorVersion)
                             && (r.nMinorVersion == nMinorVersion);
                }
    sal_Bool        operator != ( const SvVersion & r )
                {
                    return !(*this == r);
                }

    sal_uInt16      GetMajorVersion() const { return nMajorVersion; }
    sal_uInt16      GetMinorVersion() const { return nMinorVersion; }

    friend SvStream& operator << (SvStream &, const SvVersion &);
    friend SvStream& operator >> (SvStream &, SvVersion &);
#ifdef IDL_COMPILER
    sal_Bool        ReadSvIdl( SvTokenStream & rInStm );
    sal_Bool        WriteSvIdl( SvStream & rOutStm );
#endif
};


#endif // _BASTYPE_HXX

