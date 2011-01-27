/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <rtl/ustring.hxx>
#include <tools/string.hxx>
#include <com/sun/star/bridge/oleautomation/Decimal.hpp>
#include <basic/sbxcore.hxx>

#ifndef __SBX_SBXVALUES_HXX
#define __SBX_SBXVALUES_HXX

class SbxDecimal;

struct SbxValues
{
    union {
        BYTE            nByte;
        UINT16          nUShort;
        sal_Unicode     nChar;
        INT16           nInteger;
        UINT32          nULong;
        INT32           nLong;
        unsigned int    nUInt;
        int             nInt;
        sal_uInt64      uInt64;
        sal_Int64       nInt64;

        float           nSingle;
        double          nDouble;

        rtl::OUString*  pOUString;
        SbxDecimal*     pDecimal;

        SbxBase*        pObj;

        BYTE*           pByte;
        UINT16*         pUShort;
        sal_Unicode*    pChar;
        INT16*          pInteger;
        UINT32*         pULong;
        INT32*          pLong;
        unsigned int*   pUInt;
        int*            pInt;
        sal_uInt64*     puInt64;
        sal_Int64*      pnInt64;

        float*          pSingle;
        double*         pDouble;

        void*           pData;
    };
    SbxDataType  eType;

    SbxValues(): pData( NULL ), eType(SbxEMPTY) {}
    SbxValues( SbxDataType e ): eType(e) {}
    SbxValues( char _nChar ): nChar( _nChar ), eType(SbxCHAR) {}
    SbxValues( BYTE _nByte ): nByte( _nByte ), eType(SbxBYTE) {}
    SbxValues( short _nInteger ): nInteger( _nInteger ), eType(SbxINTEGER ) {}
    SbxValues( long _nLong ): nLong( _nLong ), eType(SbxLONG) {}
    SbxValues( USHORT _nUShort ): nUShort( _nUShort ), eType(SbxUSHORT) {}
    SbxValues( ULONG _nULong ): nULong( _nULong ), eType(SbxULONG) {}
    SbxValues( int _nInt ): nInt( _nInt ), eType(SbxINT) {}
    SbxValues( unsigned int _nUInt ): nUInt( _nUInt ), eType(SbxUINT) {}
    SbxValues( float _nSingle ): nSingle( _nSingle ), eType(SbxSINGLE) {}
    SbxValues( double _nDouble ): nDouble( _nDouble ), eType(SbxDOUBLE) {}
    SbxValues( const ::rtl::OUString* _pString ): pOUString( (::rtl::OUString*)_pString ), eType(SbxSTRING) {}
    SbxValues( SbxBase* _pObj ): pObj( _pObj ), eType(SbxOBJECT) {}
    SbxValues( sal_Unicode* _pChar ): pChar( _pChar ), eType(SbxLPSTR) {}
    SbxValues( void* _pData ): pData( _pData ), eType(SbxPOINTER) {}

};

#endif

#ifndef __SBX_SBXVALUE
#define __SBX_SBXVALUE

struct SbxValues;

class SbxValueImpl;

class SbxValue : public SbxBase
{
    SbxValueImpl* mpSbxValueImplImpl;   // Impl data

    // #55226 Transport additional infos
    SbxValue* TheRealValue( BOOL bObjInObjError ) const;
    SbxValue* TheRealValue() const;
protected:
    SbxValues aData; // Data
    ::rtl::OUString aPic;  // Picture-String
    String          aToolString;  // tool string copy

    virtual void Broadcast( ULONG );    // Broadcast-Call
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

    BOOL IsInteger()    const { return BOOL( GetType() == SbxINTEGER  ); }
    BOOL IsLong()       const { return BOOL( GetType() == SbxLONG     ); }
    BOOL IsSingle()     const { return BOOL( GetType() == SbxSINGLE   ); }
    BOOL IsDouble()     const { return BOOL( GetType() == SbxDOUBLE   ); }
    BOOL IsString()     const { return BOOL( GetType() == SbxSTRING   ); }
    BOOL IsDate()       const { return BOOL( GetType() == SbxDATE     ); }
    BOOL IsCurrency()   const { return BOOL( GetType() == SbxCURRENCY ); }
    BOOL IsObject()     const { return BOOL( GetType() == SbxOBJECT   ); }
    BOOL IsDataObject() const { return BOOL( GetType() == SbxDATAOBJECT);}
    BOOL IsBool()       const { return BOOL( GetType() == SbxBOOL     ); }
    BOOL IsErr()        const { return BOOL( GetType() == SbxERROR    ); }
    BOOL IsEmpty()      const { return BOOL( GetType() == SbxEMPTY    ); }
    BOOL IsNull()       const { return BOOL( GetType() == SbxNULL     ); }
    BOOL IsChar()       const { return BOOL( GetType() == SbxCHAR     ); }
    BOOL IsByte()       const { return BOOL( GetType() == SbxBYTE     ); }
    BOOL IsUShort()     const { return BOOL( GetType() == SbxUSHORT   ); }
    BOOL IsULong()      const { return BOOL( GetType() == SbxULONG    ); }
    BOOL IsInt()        const { return BOOL( GetType() == SbxINT      ); }
    BOOL IsUInt()       const { return BOOL( GetType() == SbxUINT     ); }
    BOOL IspChar()      const { return BOOL( GetType() == SbxLPSTR    ); }
    BOOL IsNumeric()    const;
    BOOL IsNumericRTL() const;          // #41692 Interface for Basic
    BOOL ImpIsNumeric( BOOL bOnlyIntntl ) const;    // Implementation

    virtual SbxClassType GetClass() const;
    virtual SbxDataType GetType() const;
    SbxDataType GetFullType() const;
    BOOL SetType( SbxDataType );

    virtual BOOL Get( SbxValues& ) const;
    BOOL GetNoBroadcast( SbxValues& );
    const SbxValues& GetValues_Impl() const { return aData; }
    virtual BOOL Put( const SbxValues& );

    inline SbxValues * data() { return &aData; }

    UINT16      GetErr() const;

    BOOL        GetBool() const;
    BYTE        GetByte() const;
    sal_Unicode GetChar() const;
    UINT16      GetUShort() const;
    UINT32      GetULong() const;
    int         GetInt() const;
    INT16       GetInteger() const;
    INT32       GetLong() const;
    sal_Int64   GetInt64() const;
    sal_uInt64  GetUInt64() const;

    sal_Int64   GetCurrency() const;
    SbxDecimal* GetDecimal() const;

    float       GetSingle() const;
    double      GetDouble() const;
    double      GetDate() const;

    const String&   GetString() const;
    const String&   GetCoreString() const;
    rtl::OUString   GetOUString() const;

    SbxBase*    GetObject() const;
    BOOL        HasObject() const;
    void*       GetData() const;


    BOOL PutEmpty();
    BOOL PutNull();
    BOOL PutErr( USHORT );

    BOOL PutBool( BOOL );
    BOOL PutByte( BYTE );
    BOOL PutChar( sal_Unicode );
    BOOL PutUShort( UINT16 );
    BOOL PutULong( UINT32 );
    BOOL PutInt( int );
    BOOL PutInteger( INT16 );
    BOOL PutLong( INT32 );
    BOOL PutInt64( sal_Int64 );
    BOOL PutUInt64( sal_uInt64 );

    BOOL PutSingle( float );
    BOOL PutDouble( double );
    BOOL PutDate( double );

            // with extended analysis (International, "TRUE"/"FALSE")
    BOOL PutStringExt( const ::rtl::OUString& );
    BOOL PutString( const ::rtl::OUString& );
    BOOL PutString( const sal_Unicode* );   // Type = SbxSTRING
    BOOL PutpChar( const sal_Unicode* );    // Type = SbxLPSTR

            // Special methods
    BOOL PutDecimal( com::sun::star::bridge::oleautomation::Decimal& rAutomationDec );
    BOOL fillAutomationDecimal( com::sun::star::bridge::oleautomation::Decimal& rAutomationDec );
    BOOL PutDecimal( SbxDecimal* pDecimal );
    BOOL PutCurrency( const sal_Int64& );
            // Interface for CDbl in Basic
    static SbxError ScanNumIntnl( const String& rSrc, double& nVal, BOOL bSingle=FALSE );

    BOOL PutObject( SbxBase* );
    BOOL PutData( void* );

    virtual BOOL Convert( SbxDataType );
    virtual BOOL Compute( SbxOperator, const SbxValue& );
    virtual BOOL Compare( SbxOperator, const SbxValue& ) const;
    BOOL Scan( const String&, USHORT* = NULL );
    void Format( String&, const String* = NULL ) const;

    // The following operators are definied for easier handling.
    // TODO: Ensure error conditions (overflow, conversions)
    // are taken into consideration in Compute and Compare

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

class SfxBroadcaster;

class SbxVariableImpl;

class SbxVariable : public SbxValue
{
    friend class SbMethod;

    SbxVariableImpl* mpSbxVariableImpl; // Impl data
    SfxBroadcaster*  pCst;              // Broadcaster, if needed
    String           maName;            // Name, if available
    SbxArrayRef      mpPar;             // Parameter-Array, if set
    USHORT           nHash;             // Hash-ID for search

    SbxVariableImpl* getImpl( void );

protected:
    SbxInfoRef  pInfo;              // Probably called information
    sal_uIntPtr nUserData;          // User data for Call()
    SbxObject* pParent;             // Currently attached object
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
    USHORT GetHashCode() const          { return nHash; }

    virtual void SetModified( BOOL );

    sal_uIntPtr GetUserData() const        { return nUserData; }
    void SetUserData( sal_uIntPtr n ) { nUserData = n;    }

    virtual SbxDataType  GetType()  const;
    virtual SbxClassType GetClass() const;

    // Parameter-Interface
    virtual SbxInfo* GetInfo();
    void SetInfo( SbxInfo* p );
    void SetParameters( SbxArray* p );
    SbxArray* GetParameters() const     { return mpPar; }

    // Sfx-Broadcasting-Support:
    // Due to data reduction and better DLL-hierarchie currently via casting
    SfxBroadcaster& GetBroadcaster();
    BOOL IsBroadcaster() const { return BOOL( pCst != NULL ); }
    virtual void Broadcast( ULONG nHintId );

    inline const SbxObject* GetParent() const { return pParent; }
    inline SbxObject* GetParent() { return pParent; }
    virtual void SetParent( SbxObject* );

    const String& GetDeclareClassName( void );
    void SetDeclareClassName( const String& );
    void SetComListener( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xComListener );

    static USHORT MakeHashCode( const String& rName );
};

#ifndef SBX_VARIABLE_DECL_DEFINED
#define SBX_VARIABLE_DECL_DEFINED
SV_DECL_REF(SbxVariable)
#endif

#endif

#endif  // _SBXVAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
