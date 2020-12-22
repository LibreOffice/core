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
#pragma once

#include <basic/sbxobj.hxx>
#include <basic/sbxmeth.hxx>
#include <sbxprop.hxx>
#include <sbxfac.hxx>
#include <basic/sbx.hxx>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/reflection/XIdlClass.hpp>
#include <com/sun/star/reflection/XServiceTypeDescription2.hpp>
#include <rtl/ustring.hxx>

#include <string_view>
#include <unordered_map>
#include <vector>
#include <map>

void registerComponentToBeDisposedForBasic( const css::uno::Reference< css::lang::XComponent >& xComponent, StarBASIC* pBasic );

class StructRefInfo
{
    css::uno::Any& maAny;
    css::uno::Type maType;
    sal_Int32 mnPos;
public:
    StructRefInfo( css::uno::Any& aAny, css::uno::Type const & rType, sal_Int32 nPos ) : maAny( aAny ), maType( rType ), mnPos( nPos ) {}

    sal_Int32 getPos() const { return mnPos; }
    const css::uno::Type& getType() const { return maType; }
    OUString getTypeName() const;
    css::uno::Any& getRootAnyRef() { return maAny; };

    css::uno::TypeClass getTypeClass() const;

    void* getInst();
    bool isEmpty() const { return (mnPos == -1); }

    css::uno::Any getValue();
    void setValue( const css::uno::Any& );
};

class SbUnoStructRefObject: public SbxObject
{
    struct caseLessComp
    {
        bool operator() (const OUString& rProp, std::u16string_view rOtherProp ) const
        {
            return rProp.compareToIgnoreAsciiCase( rOtherProp ) < 0;
        }
    };
    typedef std::map< OUString, std::unique_ptr<StructRefInfo>, caseLessComp > StructFieldInfo;
    StructFieldInfo maFields;
    StructRefInfo maMemberInfo;
    bool mbMemberCacheInit;
    void implCreateAll();
    void implCreateDbgProperties();
    void initMemberCache();
    OUString Impl_DumpProperties();
    OUString getDbgObjectName() const;
public:
    StructRefInfo getStructMember( const OUString& rMember );
    const StructRefInfo& getStructInfo() const { return maMemberInfo; }
    SbUnoStructRefObject( const OUString& aName_, const StructRefInfo& rMemberInfo );
    virtual ~SbUnoStructRefObject() override;

    // override Find to support e. g. NameAccess
    virtual SbxVariable* Find( const OUString&, SbxClassType ) override;

    // Force creation of all properties for debugging
    void createAllProperties()
        { implCreateAll(); }

    // give out value
    css::uno::Any getUnoAny();
    void Notify( SfxBroadcaster&, const SfxHint& rHint ) override;
};

class SbUnoObject: public SbxObject
{
    css::uno::Reference< css::beans::XIntrospectionAccess > mxUnoAccess;
    css::uno::Reference< css::beans::XMaterialHolder > mxMaterialHolder;
    css::uno::Reference< css::script::XInvocation > mxInvocation;
    css::uno::Reference< css::beans::XExactName > mxExactName;
    css::uno::Reference< css::beans::XExactName > mxExactNameInvocation;
    bool bNeedIntrospection;
    bool bNativeCOMObject;
    css::uno::Any maTmpUnoObj; // Only to save obj for doIntrospection!
    std::shared_ptr< SbUnoStructRefObject > maStructInfo;
    // help method to establish the dbg_-properties
    void implCreateDbgProperties();

    // help method to establish all properties and methods
    // (on the on-demand-mechanism required for the dbg_-properties)
    void implCreateAll();

public:
    static bool getDefaultPropName( SbUnoObject const * pUnoObj, OUString& sDfltProp );
    SbUnoObject( const OUString& aName_, const css::uno::Any& aUnoObj_ );
    virtual ~SbUnoObject() override;

    // #76470 do introspection on demand
    void doIntrospection();

    // override Find to support e. g. NameAccess
    virtual SbxVariable* Find( const OUString&, SbxClassType ) override;

    // Force creation of all properties for debugging
    void createAllProperties()
        { implCreateAll(); }

    // give out value
    css::uno::Any getUnoAny();
    const css::uno::Reference< css::beans::XIntrospectionAccess >& getIntrospectionAccess() const { return mxUnoAccess; }
    const css::uno::Reference< css::script::XInvocation >& getInvocation() const { return mxInvocation; }

    void Notify( SfxBroadcaster&, const SfxHint& rHint ) override;

    bool isNativeCOMObject() const
        { return bNativeCOMObject; }
};
typedef tools::SvRef<SbUnoObject> SbUnoObjectRef;

// #67781 delete return values of the uno-methods
void clearUnoMethods();
void clearUnoMethodsForBasic( StarBASIC const * pBasic );

class SbUnoMethod : public SbxMethod
{
    friend class SbUnoObject;
    friend void clearUnoMethods();
    friend void clearUnoMethodsForBasic( StarBASIC const * pBasic );

    css::uno::Reference< css::reflection::XIdlMethod > m_xUnoMethod;
    std::unique_ptr<css::uno::Sequence< css::reflection::ParamInfo >> pParamInfoSeq;

    // #67781 reference to the previous and the next method in the method list
    SbUnoMethod* pPrev;
    SbUnoMethod* pNext;

    bool mbInvocation;       // Method is based on invocation

public:

    SbUnoMethod( const OUString& aName_, SbxDataType eSbxType, css::uno::Reference< css::reflection::XIdlMethod > const & xUnoMethod_,
        bool bInvocation );
    virtual ~SbUnoMethod() override;
    virtual SbxInfo* GetInfo() override;

    const css::uno::Sequence< css::reflection::ParamInfo >& getParamInfos();

    bool isInvocationBased() const
        { return mbInvocation; }
};


class SbUnoProperty : public SbxProperty
{
    friend class SbUnoObject;
    friend class SbUnoStructRefObject;

    css::beans::Property aUnoProp;
    sal_Int32 nId;

    bool mbInvocation;      // Property is based on invocation
    SbxDataType mRealType;
    virtual ~SbUnoProperty() override;
    bool mbUnoStruct;
    SbUnoProperty( const SbUnoProperty&) = delete;
    SbUnoProperty& operator = ( const SbUnoProperty&) = delete;
public:

    SbUnoProperty( const OUString& aName_, SbxDataType eSbxType, SbxDataType eRealSbxType,
        const css::beans::Property& aUnoProp_, sal_Int32 nId_, bool bInvocation, bool bUnoStruct );

    bool isUnoStruct() const { return mbUnoStruct; }
    bool isInvocationBased() const
        { return mbInvocation; }
    SbxDataType getRealType() const { return mRealType; }
};

// factory class to create uno-structs per DIM AS NEW
class SbUnoFactory : public SbxFactory
{
public:
    virtual SbxBase* Create( sal_uInt16 nSbxId, sal_uInt32 ) override;
    virtual SbxObject* CreateObject( const OUString& ) override;
};

// wrapper for a uno-class
class SbUnoClass : public SbxObject
{
    const css::uno::Reference< css::reflection::XIdlClass >   m_xClass;

public:
    SbUnoClass( const OUString& aName_ )
        : SbxObject( aName_ )
    {}
    SbUnoClass( const OUString& aName_, const css::uno::Reference< css::reflection::XIdlClass >& xClass_ )
        : SbxObject( aName_ )
        , m_xClass( xClass_ )
    {}


    virtual SbxVariable* Find( const OUString&, SbxClassType ) override;


    const css::uno::Reference< css::reflection::XIdlClass >& getUnoClass() const { return m_xClass; }

};


// function to find a global identifier in
// the UnoScope and to wrap it for Sbx
SbUnoClass* findUnoClass( const OUString& rName );


// Wrapper for UNO Service
class SbUnoService : public SbxObject
{
    const css::uno::Reference< css::reflection::XServiceTypeDescription2 > m_xServiceTypeDesc;
    bool m_bNeedsInit;

public:
    SbUnoService( const OUString& aName_,
        const css::uno::Reference< css::reflection::XServiceTypeDescription2 >& xServiceTypeDesc )
            : SbxObject( aName_ )
            , m_xServiceTypeDesc( xServiceTypeDesc )
            , m_bNeedsInit( true )
    {}

    virtual SbxVariable* Find( const OUString&, SbxClassType ) override;

    void Notify( SfxBroadcaster&, const SfxHint& rHint ) override;
};

SbUnoService* findUnoService( const OUString& rName );


class SbUnoServiceCtor : public SbxMethod
{
    friend class SbUnoService;

    css::uno::Reference< css::reflection::XServiceConstructorDescription > m_xServiceCtorDesc;

public:

    SbUnoServiceCtor( const OUString& aName_, css::uno::Reference< css::reflection::XServiceConstructorDescription > const & xServiceCtorDesc );
    virtual ~SbUnoServiceCtor() override;
    virtual SbxInfo* GetInfo() override;

    const css::uno::Reference< css::reflection::XServiceConstructorDescription >& getServiceCtorDesc() const
        { return m_xServiceCtorDesc; }
};


// Wrapper for UNO Singleton
class SbUnoSingleton : public SbxObject
{
public:
    SbUnoSingleton( const OUString& aName_ );

    void Notify( SfxBroadcaster&, const SfxHint& rHint ) override;
};

SbUnoSingleton* findUnoSingleton( const OUString& rName );


// #105565 Special Object to wrap a strongly typed Uno Any
class SbUnoAnyObject: public SbxObject
{
    css::uno::Any     mVal;

public:
    SbUnoAnyObject( const css::uno::Any& rVal )
        : SbxObject( OUString() )
        , mVal( rVal )
    {}

    const css::uno::Any& getValue() const
        { return mVal; }

};


// #112509 Special SbxArray to transport named parameters for calls
// to OLEAutomation objects through the UNO OLE automation bridge

class AutomationNamedArgsSbxArray : public SbxArray
{
    css::uno::Sequence< OUString >      maNameSeq;
public:
    AutomationNamedArgsSbxArray( sal_Int32 nSeqSize )
        : maNameSeq( nSeqSize )
    {}

    css::uno::Sequence< OUString >& getNames()
        { return maNameSeq; }
};


class StarBASIC;

// Impl-methods for RTL
void RTL_Impl_CreateUnoStruct( SbxArray& rPar );
void RTL_Impl_CreateUnoService( SbxArray& rPar );
void RTL_Impl_CreateUnoServiceWithArguments( SbxArray& rPar );
void RTL_Impl_CreateUnoValue( SbxArray& rPar );
void RTL_Impl_GetProcessServiceManager( SbxArray& rPar );
void RTL_Impl_HasInterfaces( SbxArray& rPar );
void RTL_Impl_IsUnoStruct( SbxArray& rPar );
void RTL_Impl_EqualUnoObjects( SbxArray& rPar );
void RTL_Impl_GetDefaultContext( SbxArray& rPar );

void disposeComVariablesForBasic( StarBASIC const * pBasic );
void clearNativeObjectWrapperVector();


// #118116 Collection object

class BasicCollection : public SbxObject
{
    friend class SbiRuntime;
    SbxArrayRef xItemArray;
    static SbxInfoRef xAddInfo;
    static SbxInfoRef xItemInfo;

    void Initialize();
    virtual ~BasicCollection() override;
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
    sal_Int32 implGetIndex( SbxVariable const * pIndexVar );
    sal_Int32 implGetIndexForName(std::u16string_view rName);
    void CollAdd( SbxArray* pPar_ );
    void CollItem( SbxArray* pPar_ );
    void CollRemove( SbxArray* pPar_ );

public:
    BasicCollection( const OUString& rClassname );
    virtual void Clear() override;
};

class VBAConstantHelper
{
private:
    std::vector< OUString > aConstCache;
    std::unordered_map< OUString, css::uno::Any > aConstHash;
    bool isInited;
    VBAConstantHelper():isInited( false ) {}
    VBAConstantHelper(const VBAConstantHelper&) = delete;
    void init();
public:
    static VBAConstantHelper& instance();
    SbxVariable* getVBAConstant( const OUString& rName );
    bool isVBAConstantType( const OUString& rName );
};

SbxVariable* getDefaultProp( SbxVariable* pRef );

css::uno::Reference< css::uno::XInterface > createComListener( const css::uno::Any& aControlAny,
                                                               const OUString& aVBAType,
                                                               const OUString& aPrefix,
                                                               const SbxObjectRef& xScopeObj );

bool checkUnoObjectType(SbUnoObject& refVal, const OUString& aClass);

SbUnoObject* createOLEObject_Impl( const OUString& aType );

// #55226 ship additional information
bool handleToStringForCOMObjects( SbxObject* pObj, SbxValue* pVal );

void registerComListenerVariableForBasic( SbxVariable* pVar, StarBASIC* pBasic );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
