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

#ifndef INCLUDED_BASIC_SBXVAR_HXX
#define INCLUDED_BASIC_SBXVAR_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/bridge/oleautomation/Decimal.hpp>
#include <basic/sbxcore.hxx>
#include <basic/basicdllapi.h>


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

        OUString*       pOUString;
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
    SbxValues( const OUString* _pString ): pOUString( const_cast<OUString*>(_pString) ), eType(SbxSTRING) {}
    SbxValues( SbxBase* _pObj ): pObj( _pObj ), eType(SbxOBJECT) {}
    SbxValues( sal_Unicode* _pChar ): pChar( _pChar ), eType(SbxLPSTR) {}
    SbxValues( void* _pData ): pData( _pData ), eType(SbxPOINTER) {}

};

class BASIC_DLLPUBLIC SbxValue : public SbxBase
{
    // #55226 Transport additional infos
    BASIC_DLLPRIVATE SbxValue* TheRealValue( bool bObjInObjError ) const;
    BASIC_DLLPRIVATE SbxValue* TheRealValue() const;
protected:
    SbxValues aData; // Data
    OUString aPic;  // Picture-String
    OUString aToolString;  // tool string copy

    virtual void Broadcast( sal_uInt32 );      // Broadcast-Call
    virtual ~SbxValue();
    virtual bool LoadData( SvStream&, sal_uInt16 ) override;
    virtual bool StoreData( SvStream& ) const override;
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_VALUE,1);
    SbxValue();
    SbxValue( SbxDataType, void* = NULL );
    SbxValue( const SbxValue& );
    SbxValue& operator=( const SbxValue& );
    virtual void Clear() override;
    virtual bool IsFixed() const override;

    bool IsInteger()    const { return GetType() == SbxINTEGER   ; }
    bool IsLong()       const { return GetType() == SbxLONG      ; }
    bool IsDouble()     const { return GetType() == SbxDOUBLE    ; }
    bool IsString()     const { return GetType() == SbxSTRING    ; }
    bool IsCurrency()   const { return GetType() == SbxCURRENCY  ; }
    bool IsObject()     const { return GetType() == SbxOBJECT    ; }
    bool IsBool()       const { return GetType() == SbxBOOL      ; }
    bool IsErr()        const { return GetType() == SbxERROR     ; }
    bool IsEmpty()      const { return GetType() == SbxEMPTY     ; }
    bool IsNull()       const { return GetType() == SbxNULL      ; }
    bool IsNumeric() const;
    bool IsNumericRTL() const;  // #41692 Interface for Basic
    bool ImpIsNumeric( bool bOnlyIntntl ) const;    // Implementation

    virtual SbxClassType GetClass() const override;
    virtual SbxDataType GetType() const override;
    SbxDataType GetFullType() const { return aData.eType;}
    bool SetType( SbxDataType );

    bool Get( SbxValues& ) const;
    const SbxValues& GetValues_Impl() const { return aData; }
    bool Put( const SbxValues& );

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

    bool    GetBool() const;
    const OUString&   GetCoreString() const;
    OUString    GetOUString() const;

    SbxBase*    GetObject() const;
    sal_uInt8   GetByte() const;
    sal_uInt16  GetUShort() const;
    sal_uInt32  GetULong() const;

    bool PutInteger( sal_Int16 );
    bool PutLong( sal_Int32 );
    bool PutSingle( float );
    bool PutDouble( double );
    bool PutDate( double );
    bool PutBool( bool );
    bool PutErr( sal_uInt16 );
    bool PutStringExt( const OUString& );     // with extended analysis (International, "sal_True"/"sal_False")
    bool PutInt64( sal_Int64 );
    bool PutUInt64( sal_uInt64 );
    bool PutString( const OUString& );
    bool PutChar( sal_Unicode );
    bool PutByte( sal_uInt8 );
    bool PutUShort( sal_uInt16 );
    bool PutULong( sal_uInt32 );
    bool PutEmpty();
    bool PutNull();

            // Special methods
    bool PutDecimal( css::bridge::oleautomation::Decimal& rAutomationDec );
    bool PutDecimal( SbxDecimal* pDecimal ); // This function is needed for Windows build, don't remove
    bool fillAutomationDecimal( css::bridge::oleautomation::Decimal& rAutomationDec ) const;
    bool PutCurrency( const sal_Int64& );
            // Interface for CDbl in Basic
    static SbxError ScanNumIntnl( const OUString& rSrc, double& nVal, bool bSingle = false );

    bool PutObject( SbxBase* );

    bool Convert( SbxDataType );
    bool Compute( SbxOperator, const SbxValue& );
    bool Compare( SbxOperator, const SbxValue& ) const;
    bool Scan( const OUString&, sal_uInt16* = NULL );
    void Format( OUString&, const OUString* = NULL ) const;

    // The following operators are definied for easier handling.
    // TODO: Ensure error conditions (overflow, conversions)
    // are taken into consideration in Compute and Compare

    inline bool operator ==( const SbxValue& ) const;
    inline bool operator !=( const SbxValue& ) const;
    inline bool operator <( const SbxValue& ) const;
    inline bool operator >( const SbxValue& ) const;
    inline bool operator <=( const SbxValue& ) const;
    inline bool operator >=( const SbxValue& ) const;

    inline SbxValue& operator *=( const SbxValue& );
    inline SbxValue& operator /=( const SbxValue& );
    inline SbxValue& operator %=( const SbxValue& );
    inline SbxValue& operator +=( const SbxValue& );
    inline SbxValue& operator -=( const SbxValue& );
    inline SbxValue& operator &=( const SbxValue& );
    inline SbxValue& operator |=( const SbxValue& );
    inline SbxValue& operator ^=( const SbxValue& );
};

inline bool SbxValue::operator==( const SbxValue& r ) const
{ return Compare( SbxEQ, r ); }

inline bool SbxValue::operator!=( const SbxValue& r ) const
{ return Compare( SbxNE, r ); }

inline bool SbxValue::operator<( const SbxValue& r ) const
{ return Compare( SbxLT, r ); }

inline bool SbxValue::operator>( const SbxValue& r ) const
{ return Compare( SbxGT, r ); }

inline bool SbxValue::operator<=( const SbxValue& r ) const
{ return Compare( SbxLE, r ); }

inline bool SbxValue::operator>=( const SbxValue& r ) const
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
typedef tools::SvRef<SbxArray> SbxArrayRef;
#endif

#ifndef SBX_INFO_DECL_DEFINED
#define SBX_INFO_DECL_DEFINED
typedef tools::SvRef<SbxInfo> SbxInfoRef;
#endif

class SfxBroadcaster;

class SbxVariableImpl;
class StarBASIC;

class BASIC_DLLPUBLIC SbxVariable : public SbxValue
{
    friend class SbMethod;

    SbxVariableImpl* mpSbxVariableImpl; // Impl data
    SfxBroadcaster*  pCst;              // Broadcaster, if needed
    OUString         maName;            // Name, if available
    SbxArrayRef      mpPar;             // Parameter-Array, if set
    sal_uInt16       nHash;             // Hash-ID for search

    BASIC_DLLPRIVATE SbxVariableImpl* getImpl();

protected:
    SbxInfoRef  pInfo;              // Probably called information
    sal_uInt32 nUserData;           // User data for Call()
    SbxObject* pParent;             // Currently attached object
    virtual ~SbxVariable();
    virtual bool LoadData( SvStream&, sal_uInt16 ) override;
    virtual bool StoreData( SvStream& ) const override;
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_VARIABLE,2);
    SbxVariable();
    SbxVariable( SbxDataType, void* = NULL );
    SbxVariable( const SbxVariable& );
    SbxVariable& operator=( const SbxVariable& );

    void Dump( SvStream&, bool bDumpAll=false );

    void SetName( const OUString& );
    const OUString& GetName( SbxNameType = SbxNAME_NONE ) const;
    sal_uInt16 GetHashCode() const          { return nHash; }

    virtual void SetModified( bool ) override;

    sal_uInt32 GetUserData() const { return nUserData; }
    void SetUserData( sal_uInt32 n ) { nUserData = n; }

    virtual SbxDataType  GetType()  const override;
    virtual SbxClassType GetClass() const override;

    // Parameter-Interface
    virtual SbxInfo* GetInfo();
    void SetInfo( SbxInfo* p );
    void SetParameters( SbxArray* p );
    SbxArray* GetParameters() const;

    // Sfx-Broadcasting-Support:
    // Due to data reduction and better DLL-hierarchy currently via casting
    SfxBroadcaster& GetBroadcaster();
    bool IsBroadcaster() const { return pCst != NULL; }
    virtual void Broadcast( sal_uInt32 nHintId ) override;

    inline const SbxObject* GetParent() const { return pParent; }
    SbxObject* GetParent() { return pParent;}
    virtual void SetParent( SbxObject* );

    const OUString& GetDeclareClassName();
    void SetDeclareClassName( const OUString& );
    void SetComListener( css::uno::Reference< css::uno::XInterface > xComListener,
                         StarBASIC* pParentBasic );
    void ClearComListener();

    static sal_uInt16 MakeHashCode( const OUString& rName );
};

typedef tools::SvRef<SbxVariable> SbxVariableRef;

#endif // INCLUDED_BASIC_SBXVAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
