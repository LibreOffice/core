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

#ifndef _SBXVAR_HXX
#define _SBXVAR_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _COM_SUN_STAR_BRIDGE_OLEAUTOMATION_DECIMAL_HPP_
#include <com/sun/star/bridge/oleautomation/Decimal.hpp>
#endif

#ifndef _SBXCORE_HXX
#include "sbxcore.hxx"
#endif

#ifndef __SBX_64
#define __SBX_64

class BigInt;

namespace binfilter {

class SfxBroadcaster;

struct SbxINT64
{
    INT32 nHigh; UINT32 nLow;

#if _SOLAR__PRIVATE
#if FALSE
    SbxINT64()           : nHigh( 0 ), nLow( 0 ) {}
    SbxINT64( UINT8  n ) : nHigh( 0 ), nLow( n ) {}
    SbxINT64( UINT16 n ) : nHigh( 0 ), nLow( n ) {}
    SbxINT64( UINT32 n ) : nHigh( 0 ), nLow( n ) {}
    SbxINT64( unsigned int n ) : nHigh( 0 ), nLow( n ) {}
    SbxINT64( INT8   n ) : nHigh( n < 0 ? -1 : 0 ), nLow( n ) {}
    SbxINT64( INT16  n ) : nHigh( n < 0 ? -1 : 0 ), nLow( n ) {}
    SbxINT64( INT32  n ) : nHigh( n < 0 ? -1 : 0 ), nLow( n ) {}
    SbxINT64( int    n ) : nHigh( n < 0 ? -1 : 0 ), nLow( n ) {}
    SbxINT64( SbxINT64 &r ) : nHigh( r.nHigh ), nLow( r.nLow ) {}

    SbxINT64( BigInt &r );
    SbxINT64( double n );
#endif
    void CHS()
    {
        nLow  ^= (UINT32)-1;
        nHigh ^= -1;
        nLow++;
        if( !nLow )
            nHigh++;
    }

    // blc/os2i vertraegt kein operator =
    void Set(double n)
    {
        if( n >= 0 )
        {
            nHigh = (INT32)(n / (double)4294967296.0);
            nLow  = (UINT32)(n - ((double)nHigh * (double)4294967296.0));
        }
        else {
            nHigh = (INT32)(-n / (double)4294967296.0);
            nLow  = (UINT32)(-n - ((double)nHigh * (double)4294967296.0));
            CHS();
        }
    }
    void Set(INT32 n) { nHigh = n < 0 ? -1 : 0; nLow = n; }

    void SetMax()  { nHigh = 0x7FFFFFFF; nLow = 0xFFFFFFFF; }
    void SetMin()  { nHigh = 0x80000000; nLow = 0x00000000; }
    void SetNull() { nHigh = 0x00000000; nLow = 0x00000000; }

    int operator ! () const { return !nHigh && !nLow; }

    SbxINT64 &operator -= ( const SbxINT64 &r );
    SbxINT64 &operator += ( const SbxINT64 &r );
    SbxINT64 &operator /= ( const SbxINT64 &r );
    SbxINT64 &operator %= ( const SbxINT64 &r );
    SbxINT64 &operator *= ( const SbxINT64 &r );
    SbxINT64 &operator &= ( const SbxINT64 &r );
    SbxINT64 &operator |= ( const SbxINT64 &r );
    SbxINT64 &operator ^= ( const SbxINT64 &r );

    friend SbxINT64 operator - ( const SbxINT64 &l, const SbxINT64 &r );
    friend SbxINT64 operator + ( const SbxINT64 &l, const SbxINT64 &r );
    friend SbxINT64 operator / ( const SbxINT64 &l, const SbxINT64 &r );
    friend SbxINT64 operator % ( const SbxINT64 &l, const SbxINT64 &r );
    friend SbxINT64 operator * ( const SbxINT64 &l, const SbxINT64 &r );
    friend SbxINT64 operator & ( const SbxINT64 &l, const SbxINT64 &r );
    friend SbxINT64 operator | ( const SbxINT64 &l, const SbxINT64 &r );
    friend SbxINT64 operator ^ ( const SbxINT64 &l, const SbxINT64 &r );

    friend SbxINT64 operator - ( const SbxINT64 &r );
    friend SbxINT64 operator ~ ( const SbxINT64 &r );

    static double GetMin() { return ((double)0x7FFFFFFF*(double)4294967296.0
                                     + (double)0xFFFFFFFF)
                                    / CURRENCY_FACTOR; }
    static double GetMax() { return ((double)0x80000000*(double)4294967296.0
                                     + (double)0xFFFFFFFF)
                                    / CURRENCY_FACTOR; }
#endif
};

struct SbxUINT64
{
    UINT32 nHigh; UINT32 nLow;
#if _SOLAR__PRIVATE
    void Set(double n)
    {
        nHigh = (UINT32)(n / (double)4294967296.0);
        nLow  = (UINT32)(n - ((double)nHigh * (double)4294967296.0));
    }

    void Set(UINT32 n) { nHigh = 0; nLow = n; }

    void SetMax()  { nHigh = 0xFFFFFFFF; nLow = 0xFFFFFFFF; }
    void SetMin()  { nHigh = 0x00000000; nLow = 0x00000000; }
    void SetNull() { nHigh = 0x00000000; nLow = 0x00000000; }

    int operator ! () const { return !nHigh && !nLow; }

    SbxUINT64 &operator -= ( const SbxUINT64 &r );
    SbxUINT64 &operator += ( const SbxUINT64 &r );
    SbxUINT64 &operator /= ( const SbxUINT64 &r );
    SbxUINT64 &operator %= ( const SbxUINT64 &r );
    SbxUINT64 &operator *= ( const SbxUINT64 &r );
    SbxUINT64 &operator &= ( const SbxUINT64 &r );
    SbxUINT64 &operator |= ( const SbxUINT64 &r );
    SbxUINT64 &operator ^= ( const SbxUINT64 &r );

    friend SbxUINT64 operator - ( const SbxUINT64 &l, const SbxUINT64 &r );
    friend SbxUINT64 operator + ( const SbxUINT64 &l, const SbxUINT64 &r );
    friend SbxUINT64 operator / ( const SbxUINT64 &l, const SbxUINT64 &r );
    friend SbxUINT64 operator % ( const SbxUINT64 &l, const SbxUINT64 &r );
    friend SbxUINT64 operator * ( const SbxUINT64 &l, const SbxUINT64 &r );
    friend SbxUINT64 operator & ( const SbxUINT64 &l, const SbxUINT64 &r );
    friend SbxUINT64 operator | ( const SbxUINT64 &l, const SbxUINT64 &r );
    friend SbxUINT64 operator ^ ( const SbxUINT64 &l, const SbxUINT64 &r );

    friend SbxUINT64 operator ~ ( const SbxUINT64 &r );
#endif
};

#endif


#ifndef __SBX_SBXVALUES_HXX
#define __SBX_SBXVALUES_HXX

class SbxDecimal;

struct SbxValues
{
    union {
        sal_Unicode		nChar;
        BYTE			nByte;
        INT16			nInteger;
        INT32           nLong;
        UINT16          nUShort;
        UINT32          nULong;
        float           nSingle;
        double          nDouble;
        SbxINT64        nLong64;
        SbxUINT64       nULong64;
        sal_Int64       nInt64;
        sal_uInt64      uInt64;
        int             nInt;
        unsigned int    nUInt;
        String*         pString;
        SbxDecimal*		pDecimal;

        SbxBase*        pObj;
        sal_Unicode*    pChar;
        BYTE*	        pByte;
        INT16*	        pInteger;
        INT32*	        pLong;
        UINT16*	        pUShort;
        UINT32*	        pULong;
        float*	        pSingle;
        double*	        pDouble;
        SbxINT64*       pLong64;
        SbxUINT64*      pULong64;
        sal_Int64*      pnInt64;
        sal_uInt64*     puInt64;
        int*	        pInt;
        unsigned int*   pUInt;
        void*	        pData;
    };
    SbxDataType	 eType;

    SbxValues(): pData( NULL ), eType(SbxEMPTY) {}
    SbxValues( SbxDataType e ): eType(e) {}
    SbxValues( char _nChar ): nChar( _nChar ), eType(SbxCHAR) {}
    SbxValues( BYTE _nByte ): nByte( _nByte ), eType(SbxBYTE) {}
    SbxValues( short _nInteger ): nInteger( _nInteger ), eType(SbxINTEGER ) {}
    SbxValues( long _nLong ): nLong( _nLong ), eType(SbxLONG) {}
    SbxValues( USHORT _nUShort ): nUShort( _nUShort ), eType(SbxUSHORT) {}
    SbxValues( ULONG _nULong ): nULong( _nULong ), eType(SbxULONG) {}
    SbxValues( float _nSingle ): nSingle( _nSingle ), eType(SbxSINGLE) {}
    SbxValues( double _nDouble ): nDouble( _nDouble ), eType(SbxDOUBLE) {}
    SbxValues( int _nInt ): nInt( _nInt ), eType(SbxINT) {}
    SbxValues( unsigned int _nUInt ): nUInt( _nUInt ), eType(SbxUINT) {}
    SbxValues( const String* _pString ): pString( (String*) _pString ), eType(SbxSTRING) {}
    SbxValues( SbxBase* _pObj ): pObj( _pObj ), eType(SbxOBJECT) {}
    SbxValues( sal_Unicode* _pChar ): pChar( _pChar ), eType(SbxLPSTR) {}
    SbxValues( void* _pData ): pData( _pData ), eType(SbxPOINTER) {}
    SbxValues( const BigInt &rBig );
};

#endif

#ifndef __SBX_SBXVALUE
#define __SBX_SBXVALUE

struct SbxValues;

class SbxValueImpl;

class SbxValue : public SbxBase
{
    friend class SbiDllMgr;				// BASIC-Runtime, muss an aData ran

    SbxValueImpl* mpSbxValueImplImpl;	// Impl data

    // #55226 Zusaetzliche Info transportieren
    SbxValue* TheRealValue( BOOL bObjInObjError ) const;
    SbxValue* TheRealValue() const;
protected:
    SbxValues aData;	 				// Daten
    String    aPic;						// Picture-String

    virtual void Broadcast( ULONG );   	// Broadcast-Call
    virtual ~SbxValue();
    virtual BOOL LoadData( SvStream&, USHORT );
    virtual BOOL StoreData( SvStream& ) const;
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_VALUE,1);
    TYPEINFO();
    SbxValue();
    SbxValue( SbxDataType, void* = NULL );
    SbxValue( const SbxValue& );
    SbxValue& operator=( const SbxValue& );
    virtual void Clear();
    virtual BOOL IsFixed() const;

    BOOL IsInteger() const { return BOOL( GetType() == SbxINTEGER  ); }
    BOOL IsLong()    const { return BOOL( GetType() == SbxLONG     ); }
    BOOL IsSingle()  const { return BOOL( GetType() == SbxSINGLE   ); }
    BOOL IsDouble()  const { return BOOL( GetType() == SbxDOUBLE   ); }
    BOOL IsString()  const { return BOOL( GetType() == SbxSTRING   ); }
    BOOL IsDate()    const { return BOOL( GetType() == SbxDATE     ); }
    BOOL IsCurrency()const { return BOOL( GetType() == SbxCURRENCY ); }
    BOOL IsObject()  const { return BOOL( GetType() == SbxOBJECT   ); }
    BOOL IsDataObject()const{return BOOL( GetType() == SbxDATAOBJECT);}
    BOOL IsBool()    const { return BOOL( GetType() == SbxBOOL     ); }
    BOOL IsErr()     const { return BOOL( GetType() == SbxERROR    ); }
    BOOL IsEmpty()   const { return BOOL( GetType() == SbxEMPTY    ); }
    BOOL IsNull()    const { return BOOL( GetType() == SbxNULL     ); }
    BOOL IsChar()    const { return BOOL( GetType() == SbxCHAR     ); }
    BOOL IsByte()    const { return BOOL( GetType() == SbxBYTE     ); }
    BOOL IsUShort()  const { return BOOL( GetType() == SbxUSHORT   ); }
    BOOL IsULong()   const { return BOOL( GetType() == SbxULONG    ); }
    BOOL IsInt()     const { return BOOL( GetType() == SbxINT      ); }
    BOOL IsUInt()    const { return BOOL( GetType() == SbxUINT     ); }
    BOOL IspChar()   const { return BOOL( GetType() == SbxLPSTR    ); }
    BOOL IsNumeric() const;
    BOOL ImpIsNumeric( BOOL bOnlyIntntl ) const;	// Implementation

    virtual SbxClassType GetClass() const;
    virtual SbxDataType GetType() const;
    SbxDataType GetFullType() const;
    BOOL SetType( SbxDataType );

    virtual BOOL Get( SbxValues& ) const;
    const SbxValues& GetValues_Impl() const { return aData; }
    virtual BOOL Put( const SbxValues& );

    SbxINT64 GetCurrency() const;
    sal_Int64  GetInt64() const;
    sal_uInt64 GetUInt64() const;
    INT16  GetInteger() const;
    INT32  GetLong() const;
    float  GetSingle() const;
    double GetDouble() const;
    double GetDate() const;
    BOOL   GetBool() const;
    const  String& GetString() const;
    const  String& GetCoreString() const;
#ifdef WNT
    SbxDecimal* GetDecimal() const;
#endif
    SbxBase* GetObject() const;
    sal_Unicode GetChar() const;
    BYTE   GetByte() const;
    UINT16 GetUShort() const;
    UINT32 GetULong() const;

    BOOL PutCurrency( const SbxINT64& );
    BOOL PutInt64( sal_Int64 );
    BOOL PutUInt64( sal_uInt64 );
    BOOL PutInteger( INT16 );
    BOOL PutLong( INT32 );
    BOOL PutSingle( float );
    BOOL PutDouble( double );
    BOOL PutDate( double );
    BOOL PutBool( BOOL );
    BOOL PutString( const String& );
    BOOL PutObject( SbxBase* );
    BOOL PutChar( sal_Unicode );
#ifdef WNT
    BOOL PutDecimal( SbxDecimal* pDecimal );
#endif
    BOOL PutByte( BYTE );
    BOOL PutUShort( UINT16 );
    BOOL PutULong( UINT32 );
    BOOL PutEmpty();

    virtual BOOL Convert( SbxDataType );
    virtual BOOL Compute( SbxOperator, const SbxValue& );
    virtual BOOL Compare( SbxOperator, const SbxValue& ) const;
    BOOL Scan( const String&, USHORT* = NULL );

    // Die folgenden Operatoren sind zwecks einfacherem
    // Zugriff definiert. Fehlerkonditionen wie Ueberlauf
    // oder Konversionen werden nicht beruecksichtigt.

    inline int operator ==( const SbxValue& ) const;
    inline int operator !=( const SbxValue& ) const;
    inline int operator <( const SbxValue& ) const;
    inline int operator >( const SbxValue& ) const;
    inline int operator <=( const SbxValue& ) const;
    inline int operator >=( const SbxValue& ) const;

    inline SbxValue& operator *=( const SbxValue& );
    inline SbxValue& operator /=( const SbxValue& );
    inline SbxValue& operator %=( const SbxValue& );
    inline SbxValue& operator +=( const SbxValue& );
    inline SbxValue& operator -=( const SbxValue& );
    inline SbxValue& operator &=( const SbxValue& );
    inline SbxValue& operator |=( const SbxValue& );
    inline SbxValue& operator ^=( const SbxValue& );
};

inline int SbxValue::operator==( const SbxValue& r ) const
{ return Compare( SbxEQ, r ); }

inline int SbxValue::operator!=( const SbxValue& r ) const
{ return Compare( SbxNE, r ); }

inline int SbxValue::operator<( const SbxValue& r ) const
{ return Compare( SbxLT, r ); }

inline int SbxValue::operator>( const SbxValue& r ) const
{ return Compare( SbxGT, r ); }

inline int SbxValue::operator<=( const SbxValue& r ) const
{ return Compare( SbxLE, r ); }

inline int SbxValue::operator>=( const SbxValue& r ) const
{ return Compare( SbxGE, r ); }

inline SbxValue& SbxValue::operator*=( const SbxValue& r )
{ Compute( SbxMUL, r ); return *this; }

inline SbxValue& SbxValue::operator/=( const SbxValue& r )
{ Compute( SbxDIV, r ); return *this; }

inline SbxValue& SbxValue::operator%=( const SbxValue& r )
{ Compute( SbxMOD, r ); return *this; }

inline SbxValue& SbxValue::operator+=( const SbxValue& r )
{ Compute( SbxPLUS, r ); return *this; }

inline SbxValue& SbxValue::operator-=( const SbxValue& r )
{ Compute( SbxMINUS, r ); return *this; }

inline SbxValue& SbxValue::operator&=( const SbxValue& r )
{ Compute( SbxAND, r ); return *this; }

inline SbxValue& SbxValue::operator|=( const SbxValue& r )
{ Compute( SbxOR, r ); return *this; }

inline SbxValue& SbxValue::operator^=( const SbxValue& r )
{ Compute( SbxXOR, r ); return *this; }

#endif




#ifndef __SBX_SBXVARIABLE_HXX
#define __SBX_SBXVARIABLE_HXX

class SbxArray;
class SbxInfo;

#ifndef SBX_ARRAY_DECL_DEFINED
#define SBX_ARRAY_DECL_DEFINED
SV_DECL_REF(SbxArray)
#endif

#ifndef SBX_INFO_DECL_DEFINED
#define SBX_INFO_DECL_DEFINED
SV_DECL_REF(SbxInfo)
#endif

class SbxVariableImpl;

class SbxVariable : public SbxValue
{
    friend class SbMethod;

    SbxVariableImpl* mpSbxVariableImpl;	// Impl data

    SfxBroadcaster* pCst;		// Broadcaster, falls angefordert
    String       maName;        // Name, falls vorhanden
    SbxArrayRef  mpPar;			// Parameter-Array, falls gesetzt
    USHORT		 nHash;			// Hash-ID fuer die Suche
protected:
    SbxInfoRef  pInfo;			// Evtl. angeforderte Infos
    UINT32      nUserData;		// Benutzerdaten fuer Call()
    SbxObject* 	pParent;  		// aktuell zugeordnetes Objekt
    virtual ~SbxVariable();
    virtual BOOL LoadData( SvStream&, USHORT );
    virtual BOOL StoreData( SvStream& ) const;
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_VARIABLE,2);
    TYPEINFO();
    SbxVariable();
    SbxVariable( SbxDataType, void* = NULL );
    SbxVariable( const SbxVariable& );
    SbxVariable& operator=( const SbxVariable& );

    void Dump( SvStream&, BOOL bDumpAll=FALSE );

    virtual void SetName( const String& );
    virtual const String& GetName( SbxNameType = SbxNAME_NONE ) const;
    USHORT GetHashCode() const			{ return nHash; }

    virtual void SetModified( BOOL );

    UINT32 GetUserData() const 	   { return nUserData; }
    void   SetUserData( UINT32 n ) { nUserData = n;    }

    virtual SbxDataType  GetType()  const;
    virtual SbxClassType GetClass() const;

    // Das Parameter-Interface
    virtual SbxInfo* GetInfo();
    void SetParameters( SbxArray* p );
    SbxArray* GetParameters() const		{ return mpPar; }

    // Sfx-Broadcasting-Support:
    // Zwecks Einsparung von Daten und besserer DLL-Hierarchie
    // erst einmal per Casting
    SfxBroadcaster& GetBroadcaster();
    BOOL IsBroadcaster() const { return BOOL( pCst != NULL ); }
    virtual void Broadcast( ULONG nHintId );

    inline const SbxObject* GetParent() const { return pParent; }
    inline SbxObject* GetParent() { return pParent; }
    virtual void SetParent( SbxObject* );

    static USHORT MakeHashCode( const String& rName );
};

#ifndef SBX_VARIABLE_DECL_DEFINED
#define SBX_VARIABLE_DECL_DEFINED
SV_DECL_REF(SbxVariable)
#endif

#endif

}

#endif	// _SBXVAR_HXX
