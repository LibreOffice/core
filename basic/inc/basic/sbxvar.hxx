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

#ifndef SBXVAR_HXX
#define SBXVAR_HXX

#include <rtl/ustring.hxx>
#include <tools/string.hxx>
#include <com/sun/star/bridge/oleautomation/Decimal.hpp>
#include <basic/sbxcore.hxx>
#include "basicdllapi.h"


class SbxDecimal;

struct SbxValues
{
    union {
        sal_uInt8       nByte;
        sal_uInt16      nUShort;
        sal_Unicode     nChar;
        sal_Int16       nInteger;
        sal_uInt32      nULong;
        sal_Int32       nLong;
        unsigned int    nUInt;
        int             nInt;
        sal_uInt64      uInt64;
        sal_Int64       nInt64;

        float           nSingle;
        double          nDouble;

        rtl::OUString*  pOUString;
        SbxDecimal*     pDecimal;

        SbxBase*        pObj;

        sal_uInt8*      pByte;
        sal_uInt16*     pUShort;
        sal_Unicode*    pChar;
        sal_Int16*      pInteger;
        sal_uInt32*     pULong;
        sal_Int32*      pLong;
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
    SbxValues( sal_uInt8 _nByte ): nByte( _nByte ), eType(SbxBYTE) {}
    SbxValues( short _nInteger ): nInteger( _nInteger ), eType(SbxINTEGER ) {}
    SbxValues( long _nLong ): nLong( _nLong ), eType(SbxLONG) {}
    SbxValues( sal_uInt16 _nUShort ): nUShort( _nUShort ), eType(SbxUSHORT) {}
    SbxValues( sal_uIntPtr _nULong ): nULong( _nULong ), eType(SbxULONG) {}
    SbxValues( int _nInt ): nInt( _nInt ), eType(SbxINT) {}
    SbxValues( unsigned int _nUInt ): nUInt( _nUInt ), eType(SbxUINT) {}
    SbxValues( float _nSingle ): nSingle( _nSingle ), eType(SbxSINGLE) {}
    SbxValues( double _nDouble ): nDouble( _nDouble ), eType(SbxDOUBLE) {}
    SbxValues( const ::rtl::OUString* _pString ): pOUString( (::rtl::OUString*)_pString ), eType(SbxSTRING) {}
    SbxValues( SbxBase* _pObj ): pObj( _pObj ), eType(SbxOBJECT) {}
    SbxValues( sal_Unicode* _pChar ): pChar( _pChar ), eType(SbxLPSTR) {}
    SbxValues( void* _pData ): pData( _pData ), eType(SbxPOINTER) {}

};

class BASIC_DLLPUBLIC SbxValue : public SbxBase
{
    // #55226 Transport additional infos
    BASIC_DLLPRIVATE SbxValue* TheRealValue( sal_Bool bObjInObjError ) const;
    BASIC_DLLPRIVATE SbxValue* TheRealValue() const;
protected:
    SbxValues aData; // Data
    ::rtl::OUString aPic;  // Picture-String
    String          aToolString;  // tool string copy

    virtual void Broadcast( sal_uIntPtr );      // Broadcast-Call
    virtual ~SbxValue();
    virtual sal_Bool LoadData( SvStream&, sal_uInt16 );
    virtual sal_Bool StoreData( SvStream& ) const;
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_VALUE,1);
    TYPEINFO();
    SbxValue();
    SbxValue( SbxDataType, void* = NULL );
    SbxValue( const SbxValue& );
    SbxValue& operator=( const SbxValue& );
    virtual void Clear();
    virtual sal_Bool IsFixed() const;

    sal_Bool IsInteger() const { return sal_Bool( GetType() == SbxINTEGER  ); }
    sal_Bool IsLong()    const { return sal_Bool( GetType() == SbxLONG     ); }
    sal_Bool IsSingle()  const { return sal_Bool( GetType() == SbxSINGLE   ); }
    sal_Bool IsDouble()  const { return sal_Bool( GetType() == SbxDOUBLE   ); }
    sal_Bool IsString()  const { return sal_Bool( GetType() == SbxSTRING   ); }
    sal_Bool IsDate()    const { return sal_Bool( GetType() == SbxDATE     ); }
    sal_Bool IsCurrency()const { return sal_Bool( GetType() == SbxCURRENCY ); }
    sal_Bool IsObject()  const { return sal_Bool( GetType() == SbxOBJECT   ); }
    sal_Bool IsDataObject()const{return sal_Bool( GetType() == SbxDATAOBJECT);}
    sal_Bool IsBool()    const { return sal_Bool( GetType() == SbxBOOL     ); }
    sal_Bool IsErr()     const { return sal_Bool( GetType() == SbxERROR    ); }
    sal_Bool IsEmpty()   const { return sal_Bool( GetType() == SbxEMPTY    ); }
    sal_Bool IsNull()    const { return sal_Bool( GetType() == SbxNULL     ); }
    sal_Bool IsChar()    const { return sal_Bool( GetType() == SbxCHAR     ); }
    sal_Bool IsByte()    const { return sal_Bool( GetType() == SbxBYTE     ); }
    sal_Bool IsUShort()  const { return sal_Bool( GetType() == SbxUSHORT   ); }
    sal_Bool IsULong()   const { return sal_Bool( GetType() == SbxULONG    ); }
    sal_Bool IsInt()     const { return sal_Bool( GetType() == SbxINT      ); }
    sal_Bool IsUInt()    const { return sal_Bool( GetType() == SbxUINT     ); }
    sal_Bool IspChar()   const { return sal_Bool( GetType() == SbxLPSTR    ); }
    sal_Bool IsNumeric() const;
    sal_Bool IsNumericRTL() const;  // #41692 Interface for Basic
    sal_Bool ImpIsNumeric( bool bOnlyIntntl ) const;    // Implementation

    virtual SbxClassType GetClass() const;
    virtual SbxDataType GetType() const;
    SbxDataType GetFullType() const;
    sal_Bool SetType( SbxDataType );

    virtual sal_Bool Get( SbxValues& ) const;
    const SbxValues& GetValues_Impl() const { return aData; }
    virtual sal_Bool Put( const SbxValues& );

    inline SbxValues * data() { return &aData; }

    sal_Unicode GetChar() const;
    sal_Int16   GetInteger() const;
    sal_Int32   GetLong() const;
    sal_Int64   GetInt64() const;
    sal_uInt64  GetUInt64() const;

    sal_Int64   GetCurrency() const;
    SbxDecimal* GetDecimal() const;

    float       GetSingle() const;
    double      GetDouble() const;
    double      GetDate() const;

    sal_Bool   GetBool() const;
    const String&   GetString() const;
    const String&   GetCoreString() const;
    rtl::OUString   GetOUString() const;

    SbxBase*    GetObject() const;
    sal_uInt8   GetByte() const;
    sal_uInt16 GetUShort() const;
    sal_uInt32 GetULong() const;

    sal_Bool PutInteger( sal_Int16 );
    sal_Bool PutLong( sal_Int32 );
    sal_Bool PutSingle( float );
    sal_Bool PutDouble( double );
    sal_Bool PutDate( double );
    sal_Bool PutBool( sal_Bool );
    sal_Bool PutErr( sal_uInt16 );
    sal_Bool PutStringExt( const ::rtl::OUString& );     // with extended analysis (International, "sal_True"/"sal_False")
    sal_Bool PutInt64( sal_Int64 );
    sal_Bool PutUInt64( sal_uInt64 );
    sal_Bool PutString( const ::rtl::OUString& );
    sal_Bool PutChar( sal_Unicode );
    sal_Bool PutByte( sal_uInt8 );
    sal_Bool PutUShort( sal_uInt16 );
    sal_Bool PutULong( sal_uInt32 );
    sal_Bool PutEmpty();
    sal_Bool PutNull();

            // Special methods
    sal_Bool PutDecimal( com::sun::star::bridge::oleautomation::Decimal& rAutomationDec );
    sal_Bool PutDecimal( SbxDecimal* pDecimal ); // This function is needed for Windows build, don't remove
    sal_Bool fillAutomationDecimal( com::sun::star::bridge::oleautomation::Decimal& rAutomationDec );
    sal_Bool PutCurrency( const sal_Int64& );
            // Interface for CDbl in Basic
    static SbxError ScanNumIntnl( const String& rSrc, double& nVal, bool bSingle = false );

    sal_Bool PutObject( SbxBase* );

    virtual sal_Bool Convert( SbxDataType );
    virtual sal_Bool Compute( SbxOperator, const SbxValue& );
    virtual sal_Bool Compare( SbxOperator, const SbxValue& ) const;
    sal_Bool Scan( const String&, sal_uInt16* = NULL );
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
class StarBASIC;

class BASIC_DLLPUBLIC SbxVariable : public SbxValue
{
    friend class SbMethod;

    SbxVariableImpl* mpSbxVariableImpl; // Impl data
    SfxBroadcaster*  pCst;              // Broadcaster, if needed
    String           maName;            // Name, if available
    SbxArrayRef      mpPar;             // Parameter-Array, if set
    sal_uInt16           nHash;             // Hash-ID for search

    BASIC_DLLPRIVATE SbxVariableImpl* getImpl( void );

protected:
    SbxInfoRef  pInfo;              // Probably called information
    sal_uIntPtr nUserData;          // User data for Call()
    SbxObject* pParent;             // Currently attached object
    virtual ~SbxVariable();
    virtual sal_Bool LoadData( SvStream&, sal_uInt16 );
    virtual sal_Bool StoreData( SvStream& ) const;
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_VARIABLE,2);
    TYPEINFO();
    SbxVariable();
    SbxVariable( SbxDataType, void* = NULL );
    SbxVariable( const SbxVariable& );
    SbxVariable& operator=( const SbxVariable& );

    void Dump( SvStream&, sal_Bool bDumpAll=sal_False );

    virtual void SetName( const String& );
    virtual const String& GetName( SbxNameType = SbxNAME_NONE ) const;
    sal_uInt16 GetHashCode() const          { return nHash; }

    virtual void SetModified( sal_Bool );

    sal_uIntPtr GetUserData() const        { return nUserData; }
    void SetUserData( sal_uIntPtr n ) { nUserData = n;    }

    virtual SbxDataType  GetType()  const;
    virtual SbxClassType GetClass() const;

    // Parameter-Interface
    virtual SbxInfo* GetInfo();
    void SetInfo( SbxInfo* p );
    void SetParameters( SbxArray* p );
    SbxArray* GetParameters() const;

    // Sfx-Broadcasting-Support:
    // Due to data reduction and better DLL-hierarchie currently via casting
    SfxBroadcaster& GetBroadcaster();
    sal_Bool IsBroadcaster() const { return sal_Bool( pCst != NULL ); }
    virtual void Broadcast( sal_uIntPtr nHintId );

    inline const SbxObject* GetParent() const { return pParent; }
    SbxObject* GetParent();
    virtual void SetParent( SbxObject* );

    const String& GetDeclareClassName( void );
    void SetDeclareClassName( const String& );
    void SetComListener( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xComListener,
        StarBASIC* pParentBasic );
    void ClearComListener( void );

    static sal_uInt16 MakeHashCode( const String& rName );
};

#ifndef SBX_VARIABLE_DECL_DEFINED
#define SBX_VARIABLE_DECL_DEFINED
SV_DECL_REF(SbxVariable)
#endif

#endif  // SBXVAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
