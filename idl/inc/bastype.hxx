/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bastype.hxx,v $
 * $Revision: 1.3 $
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

ByteString MakeDefineName( const ByteString & rName );
/************class RangeUniqueIdContainer ****************************/
class RangeUniqueIdContainer : private UniqueIdContainer
{
    UINT32 nMax, nMin;
public:
                    RangeUniqueIdContainer();
                    RangeUniqueIdContainer( const Range & rRange );
                    ~RangeUniqueIdContainer();

    RangeUniqueIdContainer & operator = ( const RangeUniqueIdContainer & );

    BOOL            CreateId( UniqueItemId * );
    BOOL            CreateId( UINT32 nIdValue, UniqueItemId * );

    UINT32          GetMin() const { return nMin; }
    UINT32          GetMax() const { return nMax; }
};


/******************** class SvUINT32 **********************************/
class SvUINT32
{
    UINT32  nVal;
public:
                SvUINT32() { nVal = 0; }
                SvUINT32( UINT32 n ) : nVal( n ) {}
    SvUINT32 &  operator = ( UINT32 n ) { nVal = n; return *this; }

    operator    UINT32 &() { return nVal; }

    static UINT32  Read( SvStream & rStm );
    static void    Write( SvStream & rStm, UINT32 nVal );

    friend SvStream& operator << (SvStream & rStm, const SvUINT32 & r )
                { SvUINT32::Write( rStm, r.nVal ); return rStm; }
    friend SvStream& operator >> (SvStream & rStm, SvUINT32 & r )
                { r.nVal = SvUINT32::Read( rStm ); return rStm; }

#ifdef IDL_COMPILER
    BOOL        ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
    BOOL        WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm );
#endif
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
                { SvUINT32::Write( rStm, (UINT32)r.nVal ); return rStm; }
    friend SvStream& operator >> (SvStream & rStm, SvINT16 & r )
                { r.nVal = (short)SvUINT32::Read( rStm ); return rStm; }

#ifdef IDL_COMPILER
    BOOL        ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
    BOOL        WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm );
#endif
};


/******************** class SvUINT16 **********************************/
class SvUINT16
{
    USHORT  nVal;
public:
                SvUINT16() { nVal = 0; }
                SvUINT16( USHORT n ) : nVal( n ) {}
    SvUINT16 &  operator = ( USHORT n ) { nVal = n; return *this; }

    operator    UINT16 &() { return nVal; }

    friend SvStream& operator << (SvStream & rStm, const SvUINT16 & r )
                { SvUINT32::Write( rStm, (UINT32)r.nVal ); return rStm; }
    friend SvStream& operator >> (SvStream & rStm, SvUINT16 & r )
                { r.nVal = (USHORT)SvUINT32::Read( rStm ); return rStm; }

#ifdef IDL_COMPILER
    BOOL        ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
    BOOL        WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm );
#endif
};


/******************** class SvINT32 **********************************/
class SvINT32
{
    INT32   nVal;
public:
                SvINT32() { nVal = 0; }
                SvINT32( INT32 n ) : nVal( n ) {}
    SvINT32 &   operator = ( INT32 n ) { nVal = n; return *this; }

    operator    INT32 &() { return nVal; }

    friend SvStream& operator << (SvStream & rStm, const SvINT32 & r )
                { SvUINT32::Write( rStm, (UINT32)r.nVal ); return rStm; }
    friend SvStream& operator >> (SvStream & rStm, SvINT32 & r )
                { r.nVal = (INT32)SvUINT32::Read( rStm ); return rStm; }

#ifdef IDL_COMPILER
    BOOL        ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
    BOOL        WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm );
#endif
};


/******************** class Svint **********************************/
class Svint
{
    int     nVal;
    BOOL    bSet;
public:
                Svint() { nVal = bSet = 0; }
                Svint( int n ) : nVal( n ), bSet( TRUE ) {}
                Svint( int n, BOOL bSetP ) : nVal( n ), bSet( bSetP ) {}
    Svint    &  operator = ( int n ) { nVal = n; bSet = TRUE; return *this; }

    operator    int ()const { return nVal; }
    BOOL        IsSet() const { return bSet; }

    friend SvStream& operator << (SvStream & rStm, const Svint & r )
                { SvUINT32::Write( rStm, (UINT32)r.nVal ); rStm << r.bSet; return rStm; }
    friend SvStream& operator >> (SvStream & rStm, Svint & r )
                { r.nVal = (int)SvUINT32::Read( rStm ); rStm >> r.bSet ; return rStm; }

#ifdef IDL_COMPILER
    BOOL        ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
    BOOL        WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm );
#endif
};


/******************** class SvBOOL **********************************/
class SvBOOL
{
    BOOL  nVal:1,
          bSet:1;
public:
                SvBOOL() { bSet = nVal = FALSE; }
                SvBOOL( BOOL n ) : nVal( n ), bSet( TRUE ) {}
                SvBOOL( BOOL n, BOOL bSetP ) : nVal( n ), bSet( bSetP ) {}
    SvBOOL &    operator = ( BOOL n ) { nVal = n; bSet = TRUE; return *this; }

    operator    BOOL() const { return nVal; }
#ifdef STC
    operator    int() const { return nVal; }
#endif
    BOOL        Is() const { return nVal; }
    BOOL        IsSet() const { return bSet; }

    friend SvStream& operator << (SvStream &, const SvBOOL &);
    friend SvStream& operator >> (SvStream &, SvBOOL &);

#ifdef IDL_COMPILER
    BOOL        ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
    BOOL        WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm );
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

    BOOL        IsSet() const { return Len() != 0; }
#ifdef IDL_COMPILER
    BOOL        ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
    BOOL        WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm,
                            USHORT nTab );
#endif
};


/******************** class SvIdentifier **********************************/
class SvNumberIdentifier : public SvIdentifier
{
    UINT32  nValue;
    // darf nicht benutzt werden
    BOOL    ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
public:
                SvNumberIdentifier() : nValue( 0 ) {};
    BOOL        IsSet() const
                {
                    return SvIdentifier::IsSet() || nValue != 0;
                }
    UINT32      GetValue() const { return nValue; }
    void        SetValue( UINT32 nVal ) { nValue = nVal; }

    friend SvStream& operator << (SvStream &, const SvNumberIdentifier &);
    friend SvStream& operator >> (SvStream &, SvNumberIdentifier &);
#ifdef IDL_COMPILER
    BOOL        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    BOOL        ReadSvIdl( SvIdlDataBase &, SvStringHashEntry * pName,
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
    BOOL        IsSet() const { return Len() != 0; }
    friend SvStream& operator << (SvStream &, const SvString &);
    friend SvStream& operator >> (SvStream &, SvString &);

#ifdef IDL_COMPILER
    BOOL        ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
    BOOL        WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm,
                            USHORT nTab );
#endif
};


/******************** class SvHelpText **********************************/
class SvHelpText : public SvString
{
public:
                SvHelpText() {}
#ifdef IDL_COMPILER
    BOOL        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    BOOL        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                            USHORT nTab );
#endif
};


/******************** class SvHelpContext *******************************/
class SvHelpContext : public SvNumberIdentifier
{
/*
    UniqueItemId    aId;
    BOOL            bSet;
public:
                SvHelpContext() : bSet( FALSE ) {}

    BOOL        IsSet() const { return bSet; }
    UINT32      GetId() const { return aId.GetId(); }

#ifdef IDL_COMPILER
    BOOL        ReadSvIdl( RangeUniqueIdContainer & rCont,
                           SvTokenStream & rInStm );
    BOOL        WriteSvIdl( RangeUniqueIdContainer & rCont, SvStream & rOutStm,
                            USHORT nTab );
#endif
*/
};

/******************** class SvUUId *************************************/
class SvUUId : public SvGlobalName
{
public:
                SvUUId() {}
#ifdef IDL_COMPILER
    BOOL        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    BOOL        WriteSvIdl( SvStream & rOutStm );
#endif
};


/******************** class SvVersion **********************************/
class SvVersion
{
    USHORT  nMajorVersion;
    USHORT  nMinorVersion;
public:
                SvVersion() : nMajorVersion( 1 ), nMinorVersion( 0 ) {}
    BOOL        operator == ( const SvVersion & r )
                {
                    return (r.nMajorVersion == nMajorVersion)
                             && (r.nMinorVersion == nMinorVersion);
                }
    BOOL        operator != ( const SvVersion & r )
                {
                    return !(*this == r);
                }

    USHORT      GetMajorVersion() const { return nMajorVersion; }
    USHORT      GetMinorVersion() const { return nMinorVersion; }

    friend SvStream& operator << (SvStream &, const SvVersion &);
    friend SvStream& operator >> (SvStream &, SvVersion &);
#ifdef IDL_COMPILER
    BOOL        ReadSvIdl( SvTokenStream & rInStm );
    BOOL        WriteSvIdl( SvStream & rOutStm );
#endif
};


#endif // _BASTYPE_HXX

