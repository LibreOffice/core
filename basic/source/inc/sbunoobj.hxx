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
#ifndef INCLUDED_BASIC_SOURCE_INC_SBUNOOBJ_HXX
#define INCLUDED_BASIC_SOURCE_INC_SBUNOOBJ_HXX

#include <basic/sbxobj.hxx>
#include <basic/sbxmeth.hxx>
#include <basic/sbxprop.hxx>
#include <basic/sbxfac.hxx>
#include <basic/sbx.hxx>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/reflection/XIdlClass.hpp>
#include <com/sun/star/reflection/XServiceTypeDescription2.hpp>
#include <com/sun/star/reflection/XSingletonTypeDescription.hpp>
#include <rtl/ustring.hxx>
#include <unordered_map>
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>

void registerComponentToBeDisposedForBasic( css::uno::Reference< css::lang::XComponent > xComponent, StarBASIC* pBasic );

class StructRefInfo
{
    com::sun::star::uno::Any& maAny;
    css::uno::Type maType;
    sal_Int32 mnPos;
public:
    StructRefInfo( com::sun::star::uno::Any& aAny, css::uno::Type const & rType, sal_Int32 nPos ) : maAny( aAny ), maType( rType ), mnPos( nPos ) {}

    sal_Int32 getPos() const { return mnPos; }
    css::uno::Type getType() const { return maType; }
    OUString getTypeName() const;
    com::sun::star::uno::Any& getRootAnyRef() { return maAny; };

    com::sun::star::uno::TypeClass getTypeClass() const;

    void* getInst();
    bool isEmpty() { return (mnPos == -1); }

    ::com::sun::star::uno::Any getValue();
    bool setValue( const ::com::sun::star::uno::Any& );
};

class SbUnoStructRefObject: public SbxObject
{
    struct caseLessComp
    {
        bool operator() (const OUString& rProp, const OUString& rOtherProp ) const
        {
            return rProp.toAsciiUpperCase().compareTo( rOtherProp.toAsciiUpperCase() ) < 0;
        }
    };
    typedef ::std::map< OUString, StructRefInfo*, caseLessComp > StructFieldInfo;
    StructFieldInfo maFields;
    StructRefInfo maMemberInfo;
    bool mbMemberCacheInit;
    void implCreateAll();
    void implCreateDbgProperties();
    void initMemberCache();
    OUString Impl_DumpProperties();
    OUString getDbgObjectName();
public:
    TYPEINFO_OVERRIDE();
    StructRefInfo getStructMember( const OUString& rMember );
    StructRefInfo getStructInfo() { return maMemberInfo; }
    SbUnoStructRefObject( const OUString& aName_, const StructRefInfo& rMemberInfo );
    virtual ~SbUnoStructRefObject();

    // override Find to support e. g. NameAccess
    virtual SbxVariable* Find( const OUString&, SbxClassType ) SAL_OVERRIDE;

    // Force creation of all properties for debugging
    void createAllProperties()
        { implCreateAll(); }

    // give out value
    ::com::sun::star::uno::Any getUnoAny();
    void Notify( SfxBroadcaster&, const SfxHint& rHint ) SAL_OVERRIDE;
};

class SbUnoObject: public SbxObject
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XIntrospectionAccess > mxUnoAccess;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMaterialHolder > mxMaterialHolder;
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XInvocation > mxInvocation;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XExactName > mxExactName;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XExactName > mxExactNameInvocation;
    bool bNeedIntrospection;
    bool bNativeCOMObject;
    ::com::sun::star::uno::Any maTmpUnoObj; // Only to save obj for doIntrospection!
    ::boost::shared_ptr< SbUnoStructRefObject > maStructInfo;
    // help method to establish the dbg_-properties
    void implCreateDbgProperties();

    // help method to establish all properties and methods
    // (on the on-demand-mechanism required for the dbg_-properties)
    void implCreateAll();

public:
    static bool getDefaultPropName( SbUnoObject* pUnoObj, OUString& sDfltProp );
    TYPEINFO_OVERRIDE();
    SbUnoObject( const OUString& aName_, const ::com::sun::star::uno::Any& aUnoObj_ );
    virtual ~SbUnoObject();

    // #76470 do introspection on demand
    void doIntrospection();

    // override Find to support e. g. NameAccess
    virtual SbxVariable* Find( const OUString&, SbxClassType ) SAL_OVERRIDE;

    // Force creation of all properties for debugging
    void createAllProperties()
        { implCreateAll(); }

    // give out value
    ::com::sun::star::uno::Any getUnoAny();
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XIntrospectionAccess > getIntrospectionAccess()    { return mxUnoAccess; }
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XInvocation > getInvocation()         { return mxInvocation; }

    void Notify( SfxBroadcaster&, const SfxHint& rHint ) SAL_OVERRIDE;

    bool isNativeCOMObject()
        { return bNativeCOMObject; }
};
typedef tools::SvRef<SbUnoObject> SbUnoObjectRef;

// #67781 delete return values of the uno-methods
void clearUnoMethods();
void clearUnoMethodsForBasic( StarBASIC* pBasic );

class SbUnoMethod : public SbxMethod
{
    friend class SbUnoObject;
    friend void clearUnoMethods();
    friend void clearUnoMethodsForBasic( StarBASIC* pBasic );

    ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlMethod > m_xUnoMethod;
    ::com::sun::star::uno::Sequence< ::com::sun::star::reflection::ParamInfo >* pParamInfoSeq;

    // #67781 reference to the previous and the next method in the method list
    SbUnoMethod* pPrev;
    SbUnoMethod* pNext;

    bool mbInvocation;       // Method is based on invocation
    bool mbDirectInvocation; // Method should be used with XDirectInvocation interface

public:
    TYPEINFO_OVERRIDE();

    SbUnoMethod( const OUString& aName_, SbxDataType eSbxType, ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlMethod > xUnoMethod_,
        bool bInvocation,
        bool bDirect = false );
    virtual ~SbUnoMethod();
    virtual SbxInfo* GetInfo() SAL_OVERRIDE;

    const ::com::sun::star::uno::Sequence< ::com::sun::star::reflection::ParamInfo >& getParamInfos();

    bool isInvocationBased()
        { return mbInvocation; }
    bool needsDirectInvocation()
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
    SbUnoProperty( const SbUnoProperty&) SAL_DELETED_FUNCTION;
    SbUnoProperty& operator = ( const SbUnoProperty&) SAL_DELETED_FUNCTION;
public:

    TYPEINFO_OVERRIDE();
    SbUnoProperty( const OUString& aName_, SbxDataType eSbxType, SbxDataType eRealSbxType,
        const ::com::sun::star::beans::Property& aUnoProp_, sal_Int32 nId_, bool bInvocation, bool bUnoStruct );

    bool isUnoStruct() { return mbUnoStruct; }
    bool isInvocationBased()
        { return mbInvocation; }
    SbxDataType getRealType() { return mRealType; }
};

// factory class to create uno-structs per DIM AS NEW
class SbUnoFactory : public SbxFactory
{
public:
    virtual SbxBase* Create( sal_uInt16 nSbxId, sal_uInt32 = SBXCR_SBX ) SAL_OVERRIDE;
    virtual SbxObject* CreateObject( const OUString& ) SAL_OVERRIDE;
};

// wrapper for an uno-class
class SbUnoClass : public SbxObject
{
    const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass >   m_xClass;

public:
    TYPEINFO_OVERRIDE();
    SbUnoClass( const OUString& aName_ )
        : SbxObject( aName_ )
    {}
    SbUnoClass( const OUString& aName_, const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass >& xClass_ )
        : SbxObject( aName_ )
        , m_xClass( xClass_ )
    {}


    virtual SbxVariable* Find( const OUString&, SbxClassType ) SAL_OVERRIDE;


    const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass >& getUnoClass() { return m_xClass; }

};


// function to find a global identifier in
// the UnoScope and to wrap it for Sbx
SbUnoClass* findUnoClass( const OUString& rName );


// Wrapper for UNO Service
class SbUnoService : public SbxObject
{
    const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XServiceTypeDescription2 > m_xServiceTypeDesc;
    bool m_bNeedsInit;

public:
    TYPEINFO_OVERRIDE();
    SbUnoService( const OUString& aName_,
        const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XServiceTypeDescription2 >& xServiceTypeDesc )
            : SbxObject( aName_ )
            , m_xServiceTypeDesc( xServiceTypeDesc )
            , m_bNeedsInit( true )
    {}

    virtual SbxVariable* Find( const OUString&, SbxClassType ) SAL_OVERRIDE;

    void Notify( SfxBroadcaster&, const SfxHint& rHint ) SAL_OVERRIDE;
};

SbUnoService* findUnoService( const OUString& rName );


void clearUnoServiceCtors();

class SbUnoServiceCtor : public SbxMethod
{
    friend class SbUnoService;
    friend void clearUnoServiceCtors();

    ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XServiceConstructorDescription > m_xServiceCtorDesc;

    SbUnoServiceCtor* pNext;

public:
    TYPEINFO_OVERRIDE();

    SbUnoServiceCtor( const OUString& aName_, ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XServiceConstructorDescription > xServiceCtorDesc );
    virtual ~SbUnoServiceCtor();
    virtual SbxInfo* GetInfo() SAL_OVERRIDE;

    ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XServiceConstructorDescription > getServiceCtorDesc()
        { return m_xServiceCtorDesc; }
};


// Wrapper for UNO Singleton
class SbUnoSingleton : public SbxObject
{
    const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XSingletonTypeDescription >   m_xSingletonTypeDesc;

public:
    TYPEINFO_OVERRIDE();
    SbUnoSingleton( const OUString& aName_,
        const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XSingletonTypeDescription >& xSingletonTypeDesc );

    void Notify( SfxBroadcaster&, const SfxHint& rHint ) SAL_OVERRIDE;
};

SbUnoSingleton* findUnoSingleton( const OUString& rName );


// #105565 Special Object to wrap a strongly typed Uno Any
class SbUnoAnyObject: public SbxObject
{
    ::com::sun::star::uno::Any     mVal;

public:
    SbUnoAnyObject( const ::com::sun::star::uno::Any& rVal )
        : SbxObject( OUString() )
        , mVal( rVal )
    {}

    const ::com::sun::star::uno::Any& getValue()
        { return mVal; }

    TYPEINFO_OVERRIDE();
};


// #112509 Special SbxArray to transport named parameters for calls
// to OLEAutomation objects through the UNO OLE automation bridge

class AutomationNamedArgsSbxArray : public SbxArray
{
    ::com::sun::star::uno::Sequence< OUString >      maNameSeq;
public:
    TYPEINFO_OVERRIDE();
    AutomationNamedArgsSbxArray( sal_Int32 nSeqSize )
        : maNameSeq( nSeqSize )
    {}

    ::com::sun::star::uno::Sequence< OUString >& getNames()
        { return maNameSeq; }
};


class StarBASIC;

// Impl-methods for RTL
void RTL_Impl_CreateUnoStruct( StarBASIC* pBasic, SbxArray& rPar, bool bWrite );
void RTL_Impl_CreateUnoService( StarBASIC* pBasic, SbxArray& rPar, bool bWrite );
void RTL_Impl_CreateUnoServiceWithArguments( StarBASIC* pBasic, SbxArray& rPar, bool bWrite );
void RTL_Impl_CreateUnoValue( StarBASIC* pBasic, SbxArray& rPar, bool bWrite );
void RTL_Impl_GetProcessServiceManager( StarBASIC* pBasic, SbxArray& rPar, bool bWrite );
void RTL_Impl_HasInterfaces( StarBASIC* pBasic, SbxArray& rPar, bool bWrite );
void RTL_Impl_IsUnoStruct( StarBASIC* pBasic, SbxArray& rPar, bool bWrite );
void RTL_Impl_EqualUnoObjects( StarBASIC* pBasic, SbxArray& rPar, bool bWrite );
void RTL_Impl_GetDefaultContext( StarBASIC* pBasic, SbxArray& rPar, bool bWrite );

void disposeComVariablesForBasic( StarBASIC* pBasic );
void clearNativeObjectWrapperVector();



// #118116 Collection object

class BasicCollection : public SbxObject
{
    friend class SbiRuntime;
    SbxArrayRef xItemArray;
    static SbxInfoRef xAddInfo;
    static SbxInfoRef xItemInfo;

    void Initialize();
    virtual ~BasicCollection();
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;
    sal_Int32 implGetIndex( SbxVariable* pIndexVar );
    sal_Int32 implGetIndexForName( const OUString& rName );
    void CollAdd( SbxArray* pPar_ );
    void CollItem( SbxArray* pPar_ );
    void CollRemove( SbxArray* pPar_ );

public:
    TYPEINFO_OVERRIDE();
    BasicCollection( const OUString& rClassname );
    virtual SbxVariable* Find( const OUString&, SbxClassType ) SAL_OVERRIDE;
    virtual void Clear() SAL_OVERRIDE;
};

typedef std::unordered_map< OUString, ::com::sun::star::uno::Any, OUStringHash, ::std::equal_to< OUString > > VBAConstantsHash;

typedef std::vector< OUString > VBAConstantsVector;

class VBAConstantHelper
{
private:

    VBAConstantsVector aConstCache;
    VBAConstantsHash aConstHash;
    bool isInited;
    VBAConstantHelper():isInited( false ) {}
    VBAConstantHelper(const VBAConstantHelper&) SAL_DELETED_FUNCTION;
    void init();
public:
    static VBAConstantHelper& instance();
    SbxVariable* getVBAConstant( const OUString& rName );
    bool isVBAConstantType( const OUString& rName );
};

SbxVariable* getDefaultProp( SbxVariable* pRef );

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createComListener( const ::com::sun::star::uno::Any& aControlAny,
                                                                                         const OUString& aVBAType,
                                                                                         const OUString& aPrefix,
                                                                                         SbxObjectRef xScopeObj );

bool checkUnoObjectType( SbUnoObject* refVal, const OUString& aClass );

SbUnoObject* createOLEObject_Impl( const OUString& aType );

// #55226 ship additional information
bool handleToStringForCOMObjects( SbxObject* pObj, SbxValue* pVal );

void registerComListenerVariableForBasic( SbxVariable* pVar, StarBASIC* pBasic );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
