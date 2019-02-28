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
#include <basic/sbxcore.hxx>
#include <basic/basicdllapi.h>

#include <cstddef>
#include <cstring>
#include <memory>


namespace com::sun::star::bridge::oleautomation { struct Decimal; }
namespace com::sun::star::uno { class XInterface; }
namespace com::sun::star::uno { template <typename > class Reference; }

class SbxDecimal;
enum class SfxHintId;

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
        sal_uInt64*     puInt64;
        sal_Int64*      pnInt64;

        float*          pSingle;
        double*         pDouble;

        void*           pData;
    };
    SbxDataType  eType;

    SbxValues(): pData( nullptr ), eType(SbxEMPTY) {}
    SbxValues( SbxDataType e ): eType(e) {}
    SbxValues( double _nDouble ): nDouble( _nDouble ), eType(SbxDOUBLE) {}

    void clear(SbxDataType type) {
        // A hacky way of zeroing the union value corresponding to the given type (even though the
        // relevant zero value need not be represented by all-zero bits, in general) without evoking
        // GCC 8 -Wclass-memaccess, and without having to turn the anonymous union into a non-
        // anonymous one:
        std::memset(static_cast<void *>(this), 0, offsetof(SbxValues, eType));
        eType = type;
    }
};

class BASIC_DLLPUBLIC SbxValue : public SbxBase
{
    // #55226 Transport additional infos
    BASIC_DLLPRIVATE SbxValue* TheRealValue( bool bObjInObjError ) const;
protected:
    SbxValues aData; // Data
    OUString aPic;  // Picture-String
    OUString aToolString;  // tool string copy

    virtual void Broadcast( SfxHintId );      // Broadcast-Call
    virtual ~SbxValue() override;
    virtual bool LoadData( SvStream&, sal_uInt16 ) override;
    virtual bool StoreData( SvStream& ) const override;
public:
    SBX_DECL_PERSIST_NODATA(SBXID_VALUE,1);
    SbxValue();
    SbxValue( SbxDataType );
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

    virtual SbxClassType GetClass() const;
    virtual SbxDataType GetType() const override;
    SbxDataType GetFullType() const { return aData.eType;}
    bool SetType( SbxDataType );

    bool Get( SbxValues& ) const;
    const SbxValues& GetValues_Impl() const { return aData; }
    bool Put( const SbxValues& );

    SbxValues * data() { return &aData; }

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
    void PutDate( double );
    bool PutBool( bool );
    void PutErr( sal_uInt16 );
    void PutStringExt( const OUString& );     // with extended analysis (International, "sal_True"/"sal_False")
    bool PutInt64( sal_Int64 );
    bool PutUInt64( sal_uInt64 );
    bool PutString( const OUString& );
    bool PutChar( sal_Unicode );
    bool PutByte( sal_uInt8 );
    bool PutUShort( sal_uInt16 );
    bool PutULong( sal_uInt32 );
    bool PutEmpty();
    void PutNull();

            // Special methods
    void PutDecimal( css::bridge::oleautomation::Decimal const & rAutomationDec );
    bool PutDecimal( SbxDecimal* pDecimal ); // This function is needed for Windows build, don't remove
    void fillAutomationDecimal( css::bridge::oleautomation::Decimal& rAutomationDec ) const;
    bool PutCurrency( sal_Int64 );
            // Interface for CDbl in Basic
    static ErrCode ScanNumIntnl( const OUString& rSrc, double& nVal, bool bSingle = false );

    bool PutObject( SbxBase* );

    bool Convert( SbxDataType );
    bool Compute( SbxOperator, const SbxValue& );
    bool Compare( SbxOperator, const SbxValue& ) const;
    bool Scan( const OUString&, sal_uInt16* );
    void Format( OUString&, const OUString* = nullptr ) const;

    // The following operators are defined for easier handling.
    // TODO: Ensure error conditions (overflow, conversions)
    // are taken into consideration in Compute and Compare

    inline bool operator <=( const SbxValue& ) const;
    inline bool operator >=( const SbxValue& ) const;

    inline SbxValue& operator *=( const SbxValue& );
    inline SbxValue& operator /=( const SbxValue& );
    inline SbxValue& operator +=( const SbxValue& );
    inline SbxValue& operator -=( const SbxValue& );
};

inline bool SbxValue::operator<=( const SbxValue& r ) const
{ return Compare( SbxLE, r ); }

inline bool SbxValue::operator>=( const SbxValue& r ) const
{ return Compare( SbxGE, r ); }

inline SbxValue& SbxValue::operator*=( const SbxValue& r )
{ Compute( SbxMUL, r ); return *this; }

inline SbxValue& SbxValue::operator/=( const SbxValue& r )
{ Compute( SbxDIV, r ); return *this; }

inline SbxValue& SbxValue::operator+=( const SbxValue& r )
{ Compute( SbxPLUS, r ); return *this; }

inline SbxValue& SbxValue::operator-=( const SbxValue& r )
{ Compute( SbxMINUS, r ); return *this; }

class SbxArray;
class SbxInfo;

typedef tools::SvRef<SbxArray> SbxArrayRef;

typedef tools::SvRef<SbxInfo> SbxInfoRef;

class SfxBroadcaster;

class SbxVariableImpl;
class StarBASIC;

class BASIC_DLLPUBLIC SbxVariable : public SbxValue
{
    friend class SbMethod;

    std::unique_ptr<SbxVariableImpl> mpImpl; // Impl data
    std::unique_ptr<SfxBroadcaster>  mpBroadcaster; // Broadcaster, if needed
    OUString         maName;            // Name, if available
    SbxArrayRef      mpPar;             // Parameter-Array, if set
    sal_uInt16       nHash;             // Hash-ID for search

    BASIC_DLLPRIVATE SbxVariableImpl* getImpl();

protected:
    SbxInfoRef  pInfo;              // Probably called information
    sal_uInt32 nUserData;           // User data for Call()
    SbxObject* pParent;             // Currently attached object
    virtual ~SbxVariable() override;
    virtual bool LoadData( SvStream&, sal_uInt16 ) override;
    virtual bool StoreData( SvStream& ) const override;
public:
    SBX_DECL_PERSIST_NODATA(SBXID_VARIABLE,2);
    SbxVariable();
    SbxVariable( SbxDataType );
    SbxVariable( const SbxVariable& );
    SbxVariable& operator=( const SbxVariable& );

    void Dump( SvStream&, bool bDumpAll );

    void SetName( const OUString& );
    const OUString& GetName( SbxNameType = SbxNameType::NONE ) const;
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
    bool IsBroadcaster() const { return mpBroadcaster != nullptr; }
    virtual void Broadcast( SfxHintId nHintId ) override;

    const SbxObject* GetParent() const { return pParent; }
    SbxObject* GetParent() { return pParent;}
    virtual void SetParent( SbxObject* );

    const OUString& GetDeclareClassName();
    void SetDeclareClassName( const OUString& );
    void SetComListener( const css::uno::Reference< css::uno::XInterface >& xComListener,
                         StarBASIC* pParentBasic );
    void ClearComListener();

    static sal_uInt16 MakeHashCode( const OUString& rName );
};

typedef tools::SvRef<SbxObject> SbxObjectRef;
typedef tools::SvRef<SbxVariable> SbxVariableRef;

//tdf#59222 SbxEnsureParentVariable is a SbxVariable which keeps a reference to
//its parent, ensuring it always exists while this SbxVariable exists
class BASIC_DLLPUBLIC SbxEnsureParentVariable : public SbxVariable
{
    SbxObjectRef xParent;
public:
    SbxEnsureParentVariable(const SbxVariable& r);
    virtual void SetParent(SbxObject* p) override;
};

#endif // INCLUDED_BASIC_SBXVAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
