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
#ifndef SB_UNO_OBJ
#define SB_UNO_OBJ

#include <basic/sbxobj.hxx>
#include <basic/sbxmeth.hxx>
#include <basic/sbxprop.hxx>
#include <basic/sbxfac.hxx>
#include <basic/sbx.hxx>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/reflection/XIdlClass.hpp>
#include <com/sun/star/reflection/XServiceTypeDescription2.hpp>
#include <com/sun/star/reflection/XSingletonTypeDescription.hpp>
#include <rtl/ustring.hxx>
#include <boost/unordered_map.hpp>
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>

class StructRefInfo
{
    StructRefInfo();
    com::sun::star::uno::Any& maAny;
    typelib_TypeDescription* mpTD;
    sal_Int32 mnPos;
public:
    StructRefInfo( com::sun::star::uno::Any& aAny, typelib_TypeDescription* pTD, sal_Int32 nPos ) : maAny( aAny ), mpTD( pTD ), mnPos( nPos ) {}

    sal_Int32 getPos() const { return mnPos; }
    typelib_TypeDescription* getTD() const { return mpTD; }
    rtl::OUString getTypeName() const;
    com::sun::star::uno::Any& getRootAnyRef() { return maAny; };

    com::sun::star::uno::TypeClass getTypeClass() const;

    void* getInst();
    bool isEmpty() { return (mnPos == -1); }

    ::com::sun::star::uno::Any getValue();
    void setValue( const ::com::sun::star::uno::Any& );
};

class SbUnoStructRefObject: public SbxObject
{
    struct caseLessComp
    {
        bool operator() (const ::rtl::OUString& rProp, const ::rtl::OUString& rOtherProp ) const
        {
            return rProp.toAsciiUpperCase().compareTo( rOtherProp.toAsciiUpperCase() ) < 0;
        }
    };
    typedef ::std::map< rtl::OUString, StructRefInfo*, caseLessComp > StructFieldInfo;
    StructFieldInfo maFields;
    StructRefInfo maMemberInfo;
    bool mbMemberCacheInit;
    void implCreateAll();
    void implCreateDbgProperties();
    void initMemberCache();
    rtl::OUString Impl_DumpProperties();
    rtl::OUString getDbgObjectName();
public:
    TYPEINFO();
    StructRefInfo getStructMember( const rtl::OUString& rMember );
    StructRefInfo getStructInfo() { return maMemberInfo; }
    SbUnoStructRefObject( const ::rtl::OUString& aName_, const StructRefInfo& rMemberInfo );
    ~SbUnoStructRefObject();

    // Find overloaded to support e. g. NameAccess
    virtual SbxVariable* Find( const rtl::OUString&, SbxClassType );

    // Force creation of all properties for debugging
    void createAllProperties( void  )
        { implCreateAll(); }

    // give out value
    ::com::sun::star::uno::Any getUnoAny();
    void SFX_NOTIFY( SfxBroadcaster&, const TypeId&, const SfxHint& rHint, const TypeId& );
};
SV_DECL_IMPL_REF(SbUnoStructRefObject);

class SbUnoObject: public SbxObject
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XIntrospectionAccess > mxUnoAccess;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMaterialHolder > mxMaterialHolder;
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XInvocation > mxInvocation;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XExactName > mxExactName;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XExactName > mxExactNameInvocation;
    sal_Bool bNeedIntrospection;
    sal_Bool bNativeCOMObject;
    ::com::sun::star::uno::Any maTmpUnoObj; // Only to save obj for doIntrospection!
    ::boost::shared_ptr< SbUnoStructRefObject > maStructInfo;
    // help method to establish the dbg_-properties
    void implCreateDbgProperties( void );

    // help method to establish all properties and methods
    // (on the on-demand-mechanism required for the dbg_-properties)
    void implCreateAll( void );

public:
    static bool getDefaultPropName( SbUnoObject* pUnoObj, ::rtl::OUString& sDfltProp );
    TYPEINFO();
    SbUnoObject( const ::rtl::OUString& aName_, const ::com::sun::star::uno::Any& aUnoObj_ );
    ~SbUnoObject();

    // #76470 do introspection on demand
    void doIntrospection( void );

    // Find overloaded to support e. g. NameAccess
    virtual SbxVariable* Find( const rtl::OUString&, SbxClassType );

    // Force creation of all properties for debugging
    void createAllProperties( void  )
        { implCreateAll(); }

    // give out value
    ::com::sun::star::uno::Any getUnoAny( void );
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XIntrospectionAccess > getIntrospectionAccess( void )    { return mxUnoAccess; }
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XInvocation > getInvocation( void )         { return mxInvocation; }

    void SFX_NOTIFY( SfxBroadcaster&, const TypeId&, const SfxHint& rHint, const TypeId& );

    bool isNativeCOMObject( void )
        { return bNativeCOMObject; }
};
SV_DECL_IMPL_REF(SbUnoObject);

// #67781 delete return values of the uno-methods
void clearUnoMethods( void );
void clearUnoMethodsForBasic( StarBASIC* pBasic );

class SbUnoMethod : public SbxMethod
{
    friend class SbUnoObject;
    friend void clearUnoMethods( void );
    friend void clearUnoMethodsForBasic( StarBASIC* pBasic );

    ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlMethod > m_xUnoMethod;
    ::com::sun::star::uno::Sequence< ::com::sun::star::reflection::ParamInfo >* pParamInfoSeq;

    // #67781 reference to the previous and the next method in the method list
    SbUnoMethod* pPrev;
    SbUnoMethod* pNext;

    bool mbInvocation;       // Method is based on invocation
    bool mbDirectInvocation; // Method should be used with XDirectInvocation interface

public:
    TYPEINFO();

    SbUnoMethod( const rtl::OUString& aName_, SbxDataType eSbxType, ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlMethod > xUnoMethod_,
        bool bInvocation,
        bool bDirect = false );
    virtual ~SbUnoMethod();
    virtual SbxInfo* GetInfo();

    const ::com::sun::star::uno::Sequence< ::com::sun::star::reflection::ParamInfo >& getParamInfos( void );

    bool isInvocationBased( void )
        { return mbInvocation; }
    bool needsDirectInvocation( void )
        { return mbDirectInvocation; }
};



class SbUnoProperty : public SbxProperty
{
    friend class SbUnoObject;
    friend class SbUnoStructRefObject;

    ::com::sun::star::beans::Property aUnoProp;
    sal_Int32 nId;

    bool mbInvocation;      // Property is based on invocation
    SbxDataType mRealType;
    virtual ~SbUnoProperty();
    bool mbUnoStruct;
    SbUnoProperty( const SbUnoProperty&);
    SbUnoProperty& operator = ( const SbUnoProperty&);
public:

    TYPEINFO();
    SbUnoProperty( const rtl::OUString& aName_, SbxDataType eSbxType, SbxDataType eRealSbxType,
        const ::com::sun::star::beans::Property& aUnoProp_, sal_Int32 nId_, bool bInvocation, bool bUnoStruct );

    bool isUnoStruct() { return mbUnoStruct; }
    bool isInvocationBased( void )
        { return mbInvocation; }
    SbxDataType getRealType() { return mRealType; }
};

// factory class to create uno-structs per DIM AS NEW
class SbUnoFactory : public SbxFactory
{
public:
    virtual SbxBase* Create( sal_uInt16 nSbxId, sal_uInt32 = SBXCR_SBX );
    virtual SbxObject* CreateObject( const rtl::OUString& );
};

// wrapper for an uno-class
class SbUnoClass : public SbxObject
{
    const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass >   m_xClass;

public:
    TYPEINFO();
    SbUnoClass( const String& aName_ )
        : SbxObject( aName_ )
    {}
    SbUnoClass( const String& aName_, const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass >& xClass_ )
        : SbxObject( aName_ )
        , m_xClass( xClass_ )
    {}


    virtual SbxVariable* Find( const rtl::OUString&, SbxClassType );


    const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass >& getUnoClass( void ) { return m_xClass; }

};
SV_DECL_IMPL_REF(SbUnoClass);


// function to find a global identifier in
// the UnoScope and to wrap it for Sbx
SbUnoClass* findUnoClass( const ::rtl::OUString& rName );


// Wrapper for UNO Service
class SbUnoService : public SbxObject
{
    const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XServiceTypeDescription2 >    m_xServiceTypeDesc;
    bool                                                                                                m_bNeedsInit;

public:
    TYPEINFO();
    SbUnoService( const String& aName_,
        const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XServiceTypeDescription2 >& xServiceTypeDesc )
            : SbxObject( aName_ )
            , m_xServiceTypeDesc( xServiceTypeDesc )
            , m_bNeedsInit( true )
    {}

    virtual SbxVariable* Find( const rtl::OUString&, SbxClassType );

    void SFX_NOTIFY( SfxBroadcaster&, const TypeId&, const SfxHint& rHint, const TypeId& );
};
SV_DECL_IMPL_REF(SbUnoService);

SbUnoService* findUnoService( const ::rtl::OUString& rName );


void clearUnoServiceCtors( void );

class SbUnoServiceCtor : public SbxMethod
{
    friend class SbUnoService;
    friend void clearUnoServiceCtors( void );

    ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XServiceConstructorDescription > m_xServiceCtorDesc;

    SbUnoServiceCtor* pNext;

public:
    TYPEINFO();

    SbUnoServiceCtor( const::rtl::OUString& aName_, ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XServiceConstructorDescription > xServiceCtorDesc );
    virtual ~SbUnoServiceCtor();
    virtual SbxInfo* GetInfo();

    ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XServiceConstructorDescription > getServiceCtorDesc( void )
        { return m_xServiceCtorDesc; }
};


// Wrapper for UNO Singleton
class SbUnoSingleton : public SbxObject
{
    const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XSingletonTypeDescription >   m_xSingletonTypeDesc;

public:
    TYPEINFO();
    SbUnoSingleton( const ::rtl::OUString& aName_,
        const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XSingletonTypeDescription >& xSingletonTypeDesc );

    void SFX_NOTIFY( SfxBroadcaster&, const TypeId&, const SfxHint& rHint, const TypeId& );
};
SV_DECL_IMPL_REF(SbUnoSingleton);

SbUnoSingleton* findUnoSingleton( const ::rtl::OUString& rName );


// #105565 Special Object to wrap a strongly typed Uno Any
class SbUnoAnyObject: public SbxObject
{
    ::com::sun::star::uno::Any     mVal;

public:
    SbUnoAnyObject( const ::com::sun::star::uno::Any& rVal )
        : SbxObject( String() )
        , mVal( rVal )
    {}

    const ::com::sun::star::uno::Any& getValue( void )
        { return mVal; }

    TYPEINFO();
};


// #112509 Special SbxArray to transport named parameters for calls
// to OLEAutomation objects through the UNO OLE automation bridge

class AutomationNamedArgsSbxArray : public SbxArray
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString >      maNameSeq;
public:
    TYPEINFO();
    AutomationNamedArgsSbxArray( sal_Int32 nSeqSize )
        : maNameSeq( nSeqSize )
    {}

    ::com::sun::star::uno::Sequence< ::rtl::OUString >& getNames( void )
        { return maNameSeq; }
};


class StarBASIC;

// Impl-methods for RTL
void RTL_Impl_CreateUnoStruct( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );
void RTL_Impl_CreateUnoService( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );
void RTL_Impl_CreateUnoServiceWithArguments( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );
void RTL_Impl_CreateUnoValue( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );
void RTL_Impl_GetProcessServiceManager( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );
void RTL_Impl_HasInterfaces( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );
void RTL_Impl_IsUnoStruct( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );
void RTL_Impl_EqualUnoObjects( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );
void RTL_Impl_GetDefaultContext( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );

void disposeComVariablesForBasic( StarBASIC* pBasic );
void clearNativeObjectWrapperVector( void );


//========================================================================
// #118116 Collection object

class BasicCollection : public SbxObject
{
    friend class SbiRuntime;
    SbxArrayRef xItemArray;
    static SbxInfoRef xAddInfo;
    static SbxInfoRef xItemInfo;

    void Initialize();
    virtual ~BasicCollection();
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );
    sal_Int32 implGetIndex( SbxVariable* pIndexVar );
    sal_Int32 implGetIndexForName( const String& rName );
    void CollAdd( SbxArray* pPar_ );
    void CollItem( SbxArray* pPar_ );
    void CollRemove( SbxArray* pPar_ );

public:
    TYPEINFO();
    BasicCollection( const rtl::OUString& rClassname );
    virtual SbxVariable* Find( const rtl::OUString&, SbxClassType );
    virtual void Clear();
};

typedef boost::unordered_map< ::rtl::OUString, ::com::sun::star::uno::Any, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > VBAConstantsHash;

typedef std::vector< rtl::OUString > VBAConstantsVector;

class VBAConstantHelper
{
private:

    VBAConstantsVector aConstCache;
    VBAConstantsHash aConstHash;
    bool isInited;
    VBAConstantHelper():isInited( false ) {}
    VBAConstantHelper(const VBAConstantHelper&);
    void init();
public:
    static VBAConstantHelper& instance();
    SbxVariable* getVBAConstant( const ::rtl::OUString& rName );
    bool isVBAConstantType( const ::rtl::OUString& rName );
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
