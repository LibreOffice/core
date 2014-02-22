/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <tools/errcode.hxx>
#include <svl/hint.hxx>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/processfactory.hxx>

#include <rtl/instance.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/script/ArrayWrapper.hpp>
#include <com/sun/star/script/NativeObjectWrapper.hpp>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/DeploymentException.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyConcept.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/Introspection.hpp>
#include <com/sun/star/script/BasicErrorException.hpp>
#include <com/sun/star/script/InvocationAdapterFactory.hpp>
#include <com/sun/star/script/XAllListener.hpp>
#include <com/sun/star/script/XInvocationAdapterFactory.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/script/XDefaultProperty.hpp>
#include <com/sun/star/script/XDefaultMethod.hpp>
#include <com/sun/star/script/XDirectInvocation.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/reflection/XIdlArray.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/reflection/XServiceConstructorDescription.hpp>
#include <com/sun/star/reflection/theCoreReflection.hpp>
#include <com/sun/star/bridge/oleautomation/NamedArgument.hpp>
#include <com/sun/star/bridge/oleautomation/Date.hpp>
#include <com/sun/star/bridge/oleautomation/Decimal.hpp>
#include <com/sun/star/bridge/oleautomation/Currency.hpp>
#include <com/sun/star/bridge/oleautomation/XAutomationObject.hpp>
#include <com/sun/star/script/XAutomationInvocation.hpp>
#include <basic/codecompletecache.hxx>

using com::sun::star::uno::Reference;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::reflection;
using namespace com::sun::star::beans;
using namespace com::sun::star::script;
using namespace com::sun::star::container;
using namespace com::sun::star::bridge;
using namespace cppu;


#include<basic/sbstar.hxx>
#include<basic/sbuno.hxx>
#include<basic/sberrors.hxx>
#include<sbunoobj.hxx>
#include"sbjsmod.hxx"
#include<basic/basmgr.hxx>
#include<sbintern.hxx>
#include<runtime.hxx>

#include<math.h>
#include <boost/unordered_map.hpp>
#include <com/sun/star/reflection/XTypeDescriptionEnumerationAccess.hpp>
#include <com/sun/star/reflection/XConstantsTypeDescription.hpp>

TYPEINIT1(SbUnoMethod,SbxMethod)
TYPEINIT1(SbUnoProperty,SbxProperty)
TYPEINIT1(SbUnoObject,SbxObject)
TYPEINIT1(SbUnoStructRefObject,SbxObject)
TYPEINIT1(SbUnoClass,SbxObject)
TYPEINIT1(SbUnoService,SbxObject)
TYPEINIT1(SbUnoServiceCtor,SbxMethod)
TYPEINIT1(SbUnoSingleton,SbxObject)

typedef WeakImplHelper1< XAllListener > BasicAllListenerHelper;


static char const ID_DBG_SUPPORTEDINTERFACES[] = "Dbg_SupportedInterfaces";
static char const ID_DBG_PROPERTIES[] = "Dbg_Properties";
static char const ID_DBG_METHODS[] = "Dbg_Methods";

static char const aSeqLevelStr[] = "[]";
static char const defaultNameSpace[] = "ooo.vba";





bool SbUnoObject::getDefaultPropName( SbUnoObject* pUnoObj, OUString& sDfltProp )
{
    bool result = false;
    Reference< XDefaultProperty> xDefaultProp( pUnoObj->maTmpUnoObj, UNO_QUERY );
    if ( xDefaultProp.is() )
    {
        sDfltProp = xDefaultProp->getDefaultPropertyName();
        if ( !sDfltProp.isEmpty() )
            result = true;
    }
    return result;
}

SbxVariable* getDefaultProp( SbxVariable* pRef )
{
    SbxVariable* pDefaultProp = NULL;
    if ( pRef->GetType() == SbxOBJECT )
    {
          SbxObject* pObj = PTR_CAST(SbxObject,(SbxVariable*) pRef);
        if ( !pObj )
        {
            SbxBase* pObjVarObj = pRef->GetObject();
            pObj = PTR_CAST(SbxObject,pObjVarObj);
        }
        if ( pObj && pObj->ISA(SbUnoObject) )
        {
            SbUnoObject* pUnoObj = PTR_CAST(SbUnoObject,(SbxObject*)pObj);
            pDefaultProp = pUnoObj->GetDfltProperty();
        }
    }
    return pDefaultProp;
}

void SetSbUnoObjectDfltPropName( SbxObject* pObj )
{
    SbUnoObject* pUnoObj = PTR_CAST(SbUnoObject,(SbxObject*) pObj);
    if ( pUnoObj )
    {
        OUString sDfltPropName;

        if ( SbUnoObject::getDefaultPropName( pUnoObj, sDfltPropName ) )
        {
            OSL_TRACE("SetSbUnoObjectDfltPropName setting dflt prop for %s", OUStringToOString( pObj->GetName(), RTL_TEXTENCODING_UTF8 ).getStr() );
            pUnoObj->SetDfltProperty( sDfltPropName );
        }
    }
}


Reference< XIdlReflection > getCoreReflection_Impl( void )
{
    return css::reflection::theCoreReflection::get(
        comphelper::getProcessComponentContext());
}


Reference< XHierarchicalNameAccess > getCoreReflection_HierarchicalNameAccess_Impl( void )
{
    static Reference< XHierarchicalNameAccess > xCoreReflection_HierarchicalNameAccess;

    if( !xCoreReflection_HierarchicalNameAccess.is() )
    {
        Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
        if( xCoreReflection.is() )
        {
            xCoreReflection_HierarchicalNameAccess =
                Reference< XHierarchicalNameAccess >( xCoreReflection, UNO_QUERY );
        }
    }
    return xCoreReflection_HierarchicalNameAccess;
}


Reference< XHierarchicalNameAccess > getTypeProvider_Impl( void )
{
    static Reference< XHierarchicalNameAccess > xAccess;

    
    if( !xAccess.is() )
    {
        Reference< XComponentContext > xContext(
            comphelper::getProcessComponentContext() );
        if( xContext.is() )
        {
            xContext->getValueByName(
                OUString( "/singletons/com.sun.star.reflection.theTypeDescriptionManager" ) )
                    >>= xAccess;
            OSL_ENSURE( xAccess.is(), "### TypeDescriptionManager singleton not accessible!?" );
        }
        if( !xAccess.is() )
        {
            throw DeploymentException(
                    OUString("/singletons/com.sun.star.reflection.theTypeDescriptionManager singleton not accessible"),
                Reference< XInterface >() );
        }
    }
    return xAccess;
}


Reference< XTypeConverter > getTypeConverter_Impl( void )
{
    static Reference< XTypeConverter > xTypeConverter;

    
    if( !xTypeConverter.is() )
    {
        Reference< XComponentContext > xContext(
            comphelper::getProcessComponentContext() );
        if( xContext.is() )
        {
            xTypeConverter = Converter::create(xContext);
        }
        if( !xTypeConverter.is() )
        {
            throw DeploymentException(
                OUString("com.sun.star.script.Converter service not accessible"),
                Reference< XInterface >() );
        }
    }
    return xTypeConverter;
}



SbUnoObject* createOLEObject_Impl( const OUString& aType )
{
    static Reference< XMultiServiceFactory > xOLEFactory;
    static bool bNeedsInit = true;

    if( bNeedsInit )
    {
        bNeedsInit = false;

        Reference< XComponentContext > xContext(
            comphelper::getProcessComponentContext() );
        if( xContext.is() )
        {
            Reference<XMultiComponentFactory> xSMgr = xContext->getServiceManager();
            xOLEFactory = Reference<XMultiServiceFactory>(
                xSMgr->createInstanceWithContext(
                    OUString( "com.sun.star.bridge.OleObjectFactory"),
                        xContext ), UNO_QUERY );
        }
    }

    SbUnoObject* pUnoObj = NULL;
    if( xOLEFactory.is() )
    {
        
        OUString aOLEType = aType;
        if ( aOLEType == "SAXXMLReader30" )
        {
            aOLEType = "Msxml2.SAXXMLReader.3.0";
        }
        Reference< XInterface > xOLEObject = xOLEFactory->createInstance( aOLEType );
        if( xOLEObject.is() )
        {
            Any aAny;
            aAny <<= xOLEObject;
            pUnoObj = new SbUnoObject( aType, aAny );
            OUString sDfltPropName;

            if ( SbUnoObject::getDefaultPropName( pUnoObj, sDfltPropName ) )
                pUnoObj->SetDfltProperty( sDfltPropName );
        }
    }
    return pUnoObj;
}


namespace
{
    void lcl_indent( OUStringBuffer& _inout_rBuffer, sal_Int32 _nLevel )
    {
        while ( _nLevel-- > 0 )
        {
            _inout_rBuffer.appendAscii( "  " );
        }
    }
}

void implAppendExceptionMsg( OUStringBuffer& _inout_rBuffer, const Exception& _e, const OUString& _rExceptionType, sal_Int32 _nLevel )
{
    _inout_rBuffer.appendAscii( "\n" );
    lcl_indent( _inout_rBuffer, _nLevel );
    _inout_rBuffer.appendAscii( "Type: " );

    if ( _rExceptionType.isEmpty() )
        _inout_rBuffer.appendAscii( "Unknown" );
    else
        _inout_rBuffer.append( _rExceptionType );

    _inout_rBuffer.appendAscii( "\n" );
    lcl_indent( _inout_rBuffer, _nLevel );
    _inout_rBuffer.appendAscii( "Message: " );
    _inout_rBuffer.append( _e.Message );

}


OUString implGetExceptionMsg( const Exception& e, const OUString& aExceptionType_ )
{
    OUStringBuffer aMessageBuf;
    implAppendExceptionMsg( aMessageBuf, e, aExceptionType_, 0 );
    return aMessageBuf.makeStringAndClear();
}

OUString implGetExceptionMsg( const Any& _rCaughtException )
{
    OSL_PRECOND( _rCaughtException.getValueTypeClass() == TypeClass_EXCEPTION, "implGetExceptionMsg: illegal argument!" );
    if ( _rCaughtException.getValueTypeClass() != TypeClass_EXCEPTION )
    {
        return OUString();
    }
    return implGetExceptionMsg( *static_cast< const Exception* >( _rCaughtException.getValue() ), _rCaughtException.getValueTypeName() );
}

Any convertAny( const Any& rVal, const Type& aDestType )
{
    Any aConvertedVal;
    Reference< XTypeConverter > xConverter = getTypeConverter_Impl();
    try
    {
        aConvertedVal = xConverter->convertTo( rVal, aDestType );
    }
    catch( const IllegalArgumentException& )
    {
        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
            implGetExceptionMsg( ::cppu::getCaughtException() ) );
        return aConvertedVal;
    }
    catch( const CannotConvertException& e2 )
    {
        OUString aCannotConvertExceptionName( "com.sun.star.lang.IllegalArgumentException");
        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
                          implGetExceptionMsg( e2, aCannotConvertExceptionName ) );
        return aConvertedVal;
    }
    return aConvertedVal;
}



TYPEINIT1(SbUnoAnyObject,SbxObject)



Reference<XIdlClass> TypeToIdlClass( const Type& rType )
{
    
    Reference<XIdlClass> xRetClass;
    typelib_TypeDescription * pTD = 0;
    rType.getDescription( &pTD );

    if( pTD )
    {
        OUString sOWName( pTD->pTypeName );
        Reference< XIdlReflection > xRefl = getCoreReflection_Impl();
        xRetClass = xRefl->forName( sOWName );
    }
    return xRetClass;
}


template< class EXCEPTION >
OUString implGetExceptionMsg( const EXCEPTION& e )
{
    return implGetExceptionMsg( e, ::getCppuType( &e ).getTypeName() );
}

void implHandleBasicErrorException( BasicErrorException& e )
{
    SbError nError = StarBASIC::GetSfxFromVBError( (sal_uInt16)e.ErrorCode );
    StarBASIC::Error( nError, e.ErrorMessageArgument );
}

void implHandleWrappedTargetException( const Any& _rWrappedTargetException )
{
    Any aExamine( _rWrappedTargetException );

    
    
    InvocationTargetException aInvocationError;
    if ( aExamine >>= aInvocationError )
        aExamine = aInvocationError.TargetException;

    BasicErrorException aBasicError;

    SbError nError( ERRCODE_BASIC_EXCEPTION );
    OUStringBuffer aMessageBuf;

    
    WrappedTargetException aWrapped;
    sal_Int32 nLevel = 0;
    while ( aExamine >>= aWrapped )
    {
        
        if ( aWrapped.TargetException >>= aBasicError )
        {
            nError = StarBASIC::GetSfxFromVBError( (sal_uInt16)aBasicError.ErrorCode );
            aMessageBuf.append( aBasicError.ErrorMessageArgument );
            aExamine.clear();
            break;
        }

        
        implAppendExceptionMsg( aMessageBuf, aWrapped, aExamine.getValueTypeName(), nLevel );
        if ( aWrapped.TargetException.getValueTypeClass() == TypeClass_EXCEPTION )
            
            aMessageBuf.appendAscii( "\nTargetException:" );

        
        aExamine = aWrapped.TargetException;
        ++nLevel;
    }

    if ( aExamine.getValueTypeClass() == TypeClass_EXCEPTION )
    {
        
        implAppendExceptionMsg( aMessageBuf, *static_cast< const Exception* >( aExamine.getValue() ), aExamine.getValueTypeName(), nLevel );
    }

    StarBASIC::Error( nError, aMessageBuf.makeStringAndClear() );
}

static void implHandleAnyException( const Any& _rCaughtException )
{
    BasicErrorException aBasicError;
    WrappedTargetException aWrappedError;

    if ( _rCaughtException >>= aBasicError )
    {
        implHandleBasicErrorException( aBasicError );
    }
    else if ( _rCaughtException >>= aWrappedError )
    {
        implHandleWrappedTargetException( _rCaughtException );
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( _rCaughtException ) );
    }
}


struct ObjectItem
{
    SbxObjectRef    m_xNativeObj;

    ObjectItem( void )
    {}
    ObjectItem( SbxObject* pNativeObj )
        : m_xNativeObj( pNativeObj )
    {}
};

typedef std::vector< ObjectItem > NativeObjectWrapperVector;
class GaNativeObjectWrapperVector : public rtl::Static<NativeObjectWrapperVector, GaNativeObjectWrapperVector> {};

void clearNativeObjectWrapperVector( void )
{
    GaNativeObjectWrapperVector::get().clear();
}

static sal_uInt32 lcl_registerNativeObjectWrapper( SbxObject* pNativeObj )
{
    NativeObjectWrapperVector &rNativeObjectWrapperVector = GaNativeObjectWrapperVector::get();
    sal_uInt32 nIndex = rNativeObjectWrapperVector.size();
    rNativeObjectWrapperVector.push_back( ObjectItem( pNativeObj ) );
    return nIndex;
}

static SbxObject* lcl_getNativeObject( sal_uInt32 nIndex )
{
    SbxObjectRef xRetObj;
    NativeObjectWrapperVector &rNativeObjectWrapperVector = GaNativeObjectWrapperVector::get();
    if( nIndex < rNativeObjectWrapperVector.size() )
    {
        ObjectItem& rItem = rNativeObjectWrapperVector[ nIndex ];
        xRetObj = rItem.m_xNativeObj;
    }
    return xRetObj;
}


SbxDataType unoToSbxType( TypeClass eType )
{
    SbxDataType eRetType = SbxVOID;

    switch( eType )
    {
        case TypeClass_INTERFACE:
        case TypeClass_TYPE:
        case TypeClass_STRUCT:
        case TypeClass_EXCEPTION:       eRetType = SbxOBJECT;   break;

        case TypeClass_ENUM:            eRetType = SbxLONG;     break;
        case TypeClass_SEQUENCE:
            eRetType = (SbxDataType) ( SbxOBJECT | SbxARRAY );
            break;


        case TypeClass_ANY:             eRetType = SbxVARIANT;  break;
        case TypeClass_BOOLEAN:         eRetType = SbxBOOL;     break;
        case TypeClass_CHAR:            eRetType = SbxCHAR;     break;
        case TypeClass_STRING:          eRetType = SbxSTRING;   break;
        case TypeClass_FLOAT:           eRetType = SbxSINGLE;   break;
        case TypeClass_DOUBLE:          eRetType = SbxDOUBLE;   break;
        case TypeClass_BYTE:            eRetType = SbxINTEGER;  break;
        case TypeClass_SHORT:           eRetType = SbxINTEGER;  break;
        case TypeClass_LONG:            eRetType = SbxLONG;     break;
        case TypeClass_HYPER:           eRetType = SbxSALINT64; break;
        case TypeClass_UNSIGNED_SHORT:  eRetType = SbxUSHORT;   break;
        case TypeClass_UNSIGNED_LONG:   eRetType = SbxULONG;    break;
        case TypeClass_UNSIGNED_HYPER:  eRetType = SbxSALUINT64;break;
        default: break;
    }
    return eRetType;
}

SbxDataType unoToSbxType( const Reference< XIdlClass >& xIdlClass )
{
    SbxDataType eRetType = SbxVOID;
    if( xIdlClass.is() )
    {
        TypeClass eType = xIdlClass->getTypeClass();
        eRetType = unoToSbxType( eType );
    }
    return eRetType;
}

static void implSequenceToMultiDimArray( SbxDimArray*& pArray, Sequence< sal_Int32 >& indices, Sequence< sal_Int32 >& sizes, const Any& aValue, sal_Int32& dimension, sal_Bool bIsZeroIndex, Type* pType = NULL )
{
    Type aType = aValue.getValueType();
    TypeClass eTypeClass = aType.getTypeClass();

    sal_Int32 dimCopy = dimension;

    if ( eTypeClass == TypeClass_SEQUENCE )
    {
        Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( aType );
        Reference< XIdlArray > xIdlArray = xIdlTargetClass->getArray();
        typelib_TypeDescription * pTD = 0;
        aType.getDescription( &pTD );
        Type aElementType( ((typelib_IndirectTypeDescription *)pTD)->pType );
        ::typelib_typedescription_release( pTD );

        sal_Int32 nLen = xIdlArray->getLen( aValue );
        for ( sal_Int32 index = 0; index < nLen; ++index )
        {
            Any aElementAny = xIdlArray->get( aValue, (sal_uInt32)index );
            
            if ( dimCopy == dimension )
            {
                ++dimCopy;
                if ( sizes.getLength() < dimCopy )
                {
                    sizes.realloc( sizes.getLength() + 1 );
                    sizes[ sizes.getLength() - 1 ] = nLen;
                    indices.realloc( indices.getLength() + 1 );
                }
            }

            if ( bIsZeroIndex )
                indices[ dimCopy - 1 ] = index;
            else
                indices[ dimCopy - 1] = index + 1;

            implSequenceToMultiDimArray( pArray, indices, sizes, aElementAny, dimCopy, bIsZeroIndex, &aElementType );
        }

    }
    else
    {
        if ( indices.getLength() < 1 )
        {
            
            
            
            
            StarBASIC::Error( SbERR_INVALID_OBJECT );
            return;
        }

        SbxDataType eSbxElementType = unoToSbxType( pType ? pType->getTypeClass() : aValue.getValueTypeClass() );
        if ( !pArray )
        {
            pArray = new SbxDimArray( eSbxElementType );
            sal_Int32 nIndexLen = indices.getLength();

            
            for ( sal_Int32 index = 0; index < nIndexLen; ++index )
            {
                if ( bIsZeroIndex )
                    pArray->unoAddDim32( 0, sizes[ index ] - 1);
                else
                    pArray->unoAddDim32( 1, sizes[ index ] );

            }
        }

        if ( pArray )
        {
            SbxVariableRef xVar = new SbxVariable( eSbxElementType );
            unoToSbxValue( (SbxVariable*)xVar, aValue );

            sal_Int32* pIndices = indices.getArray();
            pArray->Put32(  (SbxVariable*)xVar, pIndices );

        }
    }
}

void unoToSbxValue( SbxVariable* pVar, const Any& aValue )
{
    Type aType = aValue.getValueType();
    TypeClass eTypeClass = aType.getTypeClass();
    switch( eTypeClass )
    {
        case TypeClass_TYPE:
        {
            
            Type aType_;
            aValue >>= aType_;
            Reference<XIdlClass> xClass = TypeToIdlClass( aType_ );
            Any aClassAny;
            aClassAny <<= xClass;

            
            OUString aName;
            SbUnoObject* pSbUnoObject = new SbUnoObject( aName, aClassAny );
            SbxObjectRef xWrapper = (SbxObject*)pSbUnoObject;

            
            if( pSbUnoObject->getUnoAny().getValueType().getTypeClass() == TypeClass_VOID )
            {
                pVar->PutObject( NULL );
            }
            else
            {
                pVar->PutObject( xWrapper );
            }
        }
        break;
        
        case TypeClass_INTERFACE:
        case TypeClass_STRUCT:
        case TypeClass_EXCEPTION:
        {
            if( eTypeClass == TypeClass_STRUCT )
            {
                ArrayWrapper aWrap;
                NativeObjectWrapper aNativeObjectWrapper;
                if ( (aValue >>= aWrap) )
                {
                    SbxDimArray* pArray = NULL;
                    Sequence< sal_Int32 > indices;
                    Sequence< sal_Int32 > sizes;
                    sal_Int32 dimension = 0;
                    implSequenceToMultiDimArray( pArray, indices, sizes, aWrap.Array, dimension, aWrap.IsZeroIndex );
                    if ( pArray )
                    {
                        SbxDimArrayRef xArray = pArray;
                        sal_uInt16 nFlags = pVar->GetFlags();
                        pVar->ResetFlag( SBX_FIXED );
                        pVar->PutObject( (SbxDimArray*)xArray );
                        pVar->SetFlags( nFlags );
                    }
                    else
                        pVar->PutEmpty();
                    break;
                }
                else if ( (aValue >>= aNativeObjectWrapper) )
                {
                    sal_uInt32 nIndex = 0;
                    if( (aNativeObjectWrapper.ObjectId >>= nIndex) )
                    {
                        SbxObject* pObj = lcl_getNativeObject( nIndex );
                        pVar->PutObject( pObj );
                    }
                    else
                        pVar->PutEmpty();
                    break;
                }
                else
                {
                    SbiInstance* pInst = GetSbData()->pInst;
                    if( pInst && pInst->IsCompatibility() )
                    {
                        oleautomation::Date aDate;
                        if( (aValue >>= aDate) )
                        {
                            pVar->PutDate( aDate.Value );
                            break;
                        }
                        else
                        {
                            oleautomation::Decimal aDecimal;
                            if( (aValue >>= aDecimal) )
                            {
                                pVar->PutDecimal( aDecimal );
                                break;
                            }
                            else
                            {
                                oleautomation::Currency aCurrency;
                                if( (aValue >>= aCurrency) )
                                {
                                    pVar->PutCurrency( aCurrency.Value );
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            
            OUString aName;
            SbUnoObject* pSbUnoObject = new SbUnoObject( aName, aValue );
            
            
            
            
            
            {
                OUString sDfltPropName;

                if ( SbUnoObject::getDefaultPropName( pSbUnoObject, sDfltPropName ) )
                {
                    pSbUnoObject->SetDfltProperty( sDfltPropName );
                }
            }
            SbxObjectRef xWrapper = (SbxObject*)pSbUnoObject;

            
            if( pSbUnoObject->getUnoAny().getValueType().getTypeClass() == TypeClass_VOID )
            {
                pVar->PutObject( NULL );
            }
            else
            {
                pVar->PutObject( xWrapper );
            }
        }
        break;


        case TypeClass_ENUM:
        {
            sal_Int32 nEnum = 0;
            enum2int( nEnum, aValue );
            pVar->PutLong( nEnum );
        }
            break;

        case TypeClass_SEQUENCE:
        {
            Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( aType );
            Reference< XIdlArray > xIdlArray = xIdlTargetClass->getArray();
            sal_Int32 i, nLen = xIdlArray->getLen( aValue );

            typelib_TypeDescription * pTD = 0;
            aType.getDescription( &pTD );
            OSL_ASSERT( pTD && pTD->eTypeClass == typelib_TypeClass_SEQUENCE );
            Type aElementType( ((typelib_IndirectTypeDescription *)pTD)->pType );
            ::typelib_typedescription_release( pTD );

            
            SbxDimArrayRef xArray;
            SbxDataType eSbxElementType = unoToSbxType( aElementType.getTypeClass() );
            xArray = new SbxDimArray( eSbxElementType );
            if( nLen > 0 )
            {
                xArray->unoAddDim32( 0, nLen - 1 );

                
                for( i = 0 ; i < nLen ; i++ )
                {
                    
                    Any aElementAny = xIdlArray->get( aValue, (sal_uInt32)i );
                    SbxVariableRef xVar = new SbxVariable( eSbxElementType );
                    unoToSbxValue( (SbxVariable*)xVar, aElementAny );

                    
                    xArray->Put32( (SbxVariable*)xVar, &i );
                }
            }
            else
            {
                xArray->unoAddDim( 0, -1 );
            }

            
            sal_uInt16 nFlags = pVar->GetFlags();
            pVar->ResetFlag( SBX_FIXED );
            pVar->PutObject( (SbxDimArray*)xArray );
            pVar->SetFlags( nFlags );

        }
        break;


        case TypeClass_BOOLEAN:         pVar->PutBool( *(sal_Bool*)aValue.getValue() ); break;
        case TypeClass_CHAR:
        {
            pVar->PutChar( *(sal_Unicode*)aValue.getValue() );
            break;
        }
        case TypeClass_STRING:          { OUString val; aValue >>= val; pVar->PutString( val ); }  break;
        case TypeClass_FLOAT:           { float val = 0; aValue >>= val; pVar->PutSingle( val ); } break;
        case TypeClass_DOUBLE:          { double val = 0; aValue >>= val; pVar->PutDouble( val ); } break;
        case TypeClass_BYTE:            { sal_Int8 val = 0; aValue >>= val; pVar->PutInteger( val ); } break;
        case TypeClass_SHORT:           { sal_Int16 val = 0; aValue >>= val; pVar->PutInteger( val ); } break;
        case TypeClass_LONG:            { sal_Int32 val = 0; aValue >>= val; pVar->PutLong( val ); } break;
        case TypeClass_HYPER:           { sal_Int64 val = 0; aValue >>= val; pVar->PutInt64( val ); } break;
        case TypeClass_UNSIGNED_SHORT:  { sal_uInt16 val = 0; aValue >>= val; pVar->PutUShort( val ); } break;
        case TypeClass_UNSIGNED_LONG:   { sal_uInt32 val = 0; aValue >>= val; pVar->PutULong( val ); } break;
        case TypeClass_UNSIGNED_HYPER:  { sal_uInt64 val = 0; aValue >>= val; pVar->PutUInt64( val ); } break;
        default:                        pVar->PutEmpty();                       break;
    }
}


Type getUnoTypeForSbxBaseType( SbxDataType eType )
{
    Type aRetType = getCppuVoidType();
    switch( eType )
    {
        case SbxNULL:       aRetType = ::getCppuType( (const Reference< XInterface > *)0 ); break;
        case SbxINTEGER:    aRetType = ::getCppuType( (sal_Int16*)0 ); break;
        case SbxLONG:       aRetType = ::getCppuType( (sal_Int32*)0 ); break;
        case SbxSINGLE:     aRetType = ::getCppuType( (float*)0 ); break;
        case SbxDOUBLE:     aRetType = ::getCppuType( (double*)0 ); break;
        case SbxCURRENCY:   aRetType = ::getCppuType( (oleautomation::Currency*)0 ); break;
        case SbxDECIMAL:    aRetType = ::getCppuType( (oleautomation::Decimal*)0 ); break;
        case SbxDATE:       {
                            SbiInstance* pInst = GetSbData()->pInst;
                            if( pInst && pInst->IsCompatibility() )
                                aRetType = ::getCppuType( (double*)0 );
                            else
                                aRetType = ::getCppuType( (oleautomation::Date*)0 );
                            }
                            break;
        case SbxSTRING:     aRetType = ::getCppuType( (OUString*)0 ); break;
        case SbxBOOL:       aRetType = ::getCppuType( (sal_Bool*)0 ); break;
        case SbxVARIANT:    aRetType = ::getCppuType( (Any*)0 ); break;
        case SbxCHAR:       aRetType = ::getCppuType( (sal_Unicode*)0 ); break;
        case SbxBYTE:       aRetType = ::getCppuType( (sal_Int8*)0 ); break;
        case SbxUSHORT:     aRetType = ::getCppuType( (sal_uInt16*)0 ); break;
        case SbxULONG:      aRetType = ::getCppuType( (sal_uInt32*)0 ); break;
        
        case SbxINT:        aRetType = ::getCppuType( (sal_Int32*)0 ); break;
        case SbxUINT:       aRetType = ::getCppuType( (sal_uInt32*)0 ); break;
        default: break;
    }
    return aRetType;
}


Type getUnoTypeForSbxValue( const SbxValue* pVal )
{
    Type aRetType = getCppuVoidType();
    if( !pVal )
        return aRetType;

    
    SbxDataType eBaseType = pVal->SbxValue::GetType();
    if( eBaseType == SbxOBJECT )
    {
        SbxBaseRef xObj = (SbxBase*)pVal->GetObject();
        if( !xObj )
        {
            aRetType = getCppuType( static_cast<Reference<XInterface> *>(0) );
            return aRetType;
        }

        if( xObj->ISA(SbxDimArray) )
        {
            SbxBase* pObj = (SbxBase*)xObj;
            SbxDimArray* pArray = (SbxDimArray*)pObj;

            short nDims = pArray->GetDims();
            Type aElementType = getUnoTypeForSbxBaseType( (SbxDataType)(pArray->GetType() & 0xfff) );
            TypeClass eElementTypeClass = aElementType.getTypeClass();

            
            sal_Int32 nLower, nUpper;
            if( nDims == 1 && pArray->GetDim32( 1, nLower, nUpper ) )
            {
                if( eElementTypeClass == TypeClass_VOID || eElementTypeClass == TypeClass_ANY )
                {
                    
                    
                    bool bNeedsInit = true;

                    sal_Int32 nSize = nUpper - nLower + 1;
                    sal_Int32 nIdx = nLower;
                    for( sal_Int32 i = 0 ; i < nSize ; i++,nIdx++ )
                    {
                        SbxVariableRef xVar = pArray->Get32( &nIdx );
                        Type aType = getUnoTypeForSbxValue( (SbxVariable*)xVar );
                        if( bNeedsInit )
                        {
                            if( aType.getTypeClass() == TypeClass_VOID )
                            {
                                
                                
                                aElementType = getCppuType( (Any*)0 );
                                break;
                            }
                            aElementType = aType;
                            bNeedsInit = false;
                        }
                        else if( aElementType != aType )
                        {
                            
                            aElementType = getCppuType( (Any*)0 );
                            break;
                        }
                    }
                }

                OUString aSeqTypeName = aSeqLevelStr + aElementType.getTypeName();
                aRetType = Type( TypeClass_SEQUENCE, aSeqTypeName );
            }
            
            else if( nDims > 1 )
            {
                if( eElementTypeClass == TypeClass_VOID || eElementTypeClass == TypeClass_ANY )
                {
                    
                    sal_uInt32 nFlatArraySize = pArray->Count32();

                    bool bNeedsInit = true;
                    for( sal_uInt32 i = 0 ; i < nFlatArraySize ; i++ )
                    {
                        SbxVariableRef xVar = pArray->SbxArray::Get32( i );
                        Type aType = getUnoTypeForSbxValue( (SbxVariable*)xVar );
                        if( bNeedsInit )
                        {
                            if( aType.getTypeClass() == TypeClass_VOID )
                            {
                                
                                
                                aElementType = getCppuType( (Any*)0 );
                                break;
                            }
                            aElementType = aType;
                            bNeedsInit = false;
                        }
                        else if( aElementType != aType )
                        {
                            
                            aElementType = getCppuType( (Any*)0 );
                            break;
                        }
                    }
                }

                OUStringBuffer aSeqTypeName;
                for( short iDim = 0 ; iDim < nDims ; iDim++ )
                {
                    aSeqTypeName.appendAscii(aSeqLevelStr);
                }
                aSeqTypeName.append(aElementType.getTypeName());
                aRetType = Type( TypeClass_SEQUENCE, aSeqTypeName.makeStringAndClear() );
            }
        }
        
        else if( xObj->ISA(SbUnoObject) )
        {
            aRetType = ((SbUnoObject*)(SbxBase*)xObj)->getUnoAny().getValueType();
        }
        
        else if( xObj->ISA(SbUnoAnyObject) )
        {
            aRetType = ((SbUnoAnyObject*)(SbxBase*)xObj)->getValue().getValueType();
        }
        
    }
    
    else
    {
        aRetType = getUnoTypeForSbxBaseType( eBaseType );
    }
    return aRetType;
}


Any sbxToUnoValueImpl( const SbxValue* pVar, bool bBlockConversionToSmallestType = false )
{
    SbxDataType eBaseType = pVar->SbxValue::GetType();
    if( eBaseType == SbxOBJECT )
    {
        SbxBaseRef xObj = (SbxBase*)pVar->GetObject();
        if( xObj.Is() )
        {
            if( xObj->ISA(SbUnoAnyObject) )
                return ((SbUnoAnyObject*)(SbxBase*)xObj)->getValue();
            if( xObj->ISA(SbClassModuleObject) )
            {
                Any aRetAny;
                SbClassModuleObject* pClassModuleObj = (SbClassModuleObject*)(SbxBase*)xObj;
                SbModule* pClassModule = pClassModuleObj->getClassModule();
                if( pClassModule->createCOMWrapperForIface( aRetAny, pClassModuleObj ) )
                    return aRetAny;
            }
            if( !xObj->ISA(SbUnoObject) )
            {
                
                SbxObject* pObj = PTR_CAST(SbxObject,pVar->GetObject());
                if( pObj != NULL )
                {
                    NativeObjectWrapper aNativeObjectWrapper;
                    sal_uInt32 nIndex = lcl_registerNativeObjectWrapper( pObj );
                    aNativeObjectWrapper.ObjectId <<= nIndex;
                    Any aRetAny;
                    aRetAny <<= aNativeObjectWrapper;
                    return aRetAny;
                }
            }
        }
    }

    Type aType = getUnoTypeForSbxValue( pVar );
    TypeClass eType = aType.getTypeClass();

    if( !bBlockConversionToSmallestType )
    {
        
        
        switch( eType )
        {
            case TypeClass_FLOAT:
            case TypeClass_DOUBLE:
            {
                double d = pVar->GetDouble();
                if( d == floor( d ) )
                {
                    if( d >= -128 && d <= 127 )
                        aType = ::getCppuType( (sal_Int8*)0 );
                    else if( d >= SbxMININT && d <= SbxMAXINT )
                        aType = ::getCppuType( (sal_Int16*)0 );
                    else if( d >= -SbxMAXLNG && d <= SbxMAXLNG )
                        aType = ::getCppuType( (sal_Int32*)0 );
                }
                break;
            }
            case TypeClass_SHORT:
            {
                sal_Int16 n = pVar->GetInteger();
                if( n >= -128 && n <= 127 )
                    aType = ::getCppuType( (sal_Int8*)0 );
                break;
            }
            case TypeClass_LONG:
            {
                sal_Int32 n = pVar->GetLong();
                if( n >= -128 && n <= 127 )
                    aType = ::getCppuType( (sal_Int8*)0 );
                else if( n >= SbxMININT && n <= SbxMAXINT )
                    aType = ::getCppuType( (sal_Int16*)0 );
                break;
            }
            case TypeClass_UNSIGNED_SHORT:
            {
                sal_uInt16 n = pVar->GetUShort();
                if( n <= 255 )
                    aType = ::getCppuType( (sal_uInt8*)0 );
                break;
            }
            case TypeClass_UNSIGNED_LONG:
            {
                sal_uInt32 n = pVar->GetLong();
                if( n <= 255 )
                    aType = ::getCppuType( (sal_uInt8*)0 );
                else if( n <= SbxMAXUINT )
                    aType = ::getCppuType( (sal_uInt16*)0 );
                break;
            }
            
            default: break;
        }
    }

    return sbxToUnoValue( pVar, aType );
}




static Any implRekMultiDimArrayToSequence( SbxDimArray* pArray,
    const Type& aElemType, short nMaxDimIndex, short nActualDim,
    sal_Int32* pActualIndices, sal_Int32* pLowerBounds, sal_Int32* pUpperBounds )
{
    sal_Int32 nSeqLevel = nMaxDimIndex - nActualDim + 1;
    OUStringBuffer aSeqTypeName;
    sal_Int32 i;
    for( i = 0 ; i < nSeqLevel ; i++ )
    {
        aSeqTypeName.appendAscii(aSeqLevelStr);
    }
    aSeqTypeName.append(aElemType.getTypeName());
    Type aSeqType( TypeClass_SEQUENCE, aSeqTypeName.makeStringAndClear() );

    
    Any aRetVal;
    Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( aSeqType );
    xIdlTargetClass->createObject( aRetVal );

    
    sal_Int32 nUpper = pUpperBounds[nActualDim];
    sal_Int32 nLower = pLowerBounds[nActualDim];
    sal_Int32 nSeqSize = nUpper - nLower + 1;
    Reference< XIdlArray > xArray = xIdlTargetClass->getArray();
    xArray->realloc( aRetVal, nSeqSize );

    sal_Int32& ri = pActualIndices[nActualDim];

    for( ri = nLower,i = 0 ; ri <= nUpper ; ri++,i++ )
    {
        Any aElementVal;

        if( nActualDim < nMaxDimIndex )
        {
            aElementVal = implRekMultiDimArrayToSequence( pArray, aElemType,
                nMaxDimIndex, nActualDim + 1, pActualIndices, pLowerBounds, pUpperBounds );
        }
        else
        {
            SbxVariable* pSource = pArray->Get32( pActualIndices );
            aElementVal = sbxToUnoValue( pSource, aElemType );
        }

        try
        {
            
            xArray->set( aRetVal, i, aElementVal );
        }
        catch( const IllegalArgumentException& )
        {
            StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
                implGetExceptionMsg( ::cppu::getCaughtException() ) );
        }
        catch (const IndexOutOfBoundsException&)
        {
            StarBASIC::Error( SbERR_OUT_OF_RANGE );
        }
    }
    return aRetVal;
}


Any sbxToUnoValue( const SbxValue* pVar )
{
    return sbxToUnoValueImpl( pVar );
}



static bool implGetTypeByName( const OUString& rName, Type& rRetType )
{
    bool bSuccess = false;

    Reference< XHierarchicalNameAccess > xTypeAccess = getTypeProvider_Impl();
    if( xTypeAccess->hasByHierarchicalName( rName ) )
    {
        Any aRet = xTypeAccess->getByHierarchicalName( rName );
        Reference< XTypeDescription > xTypeDesc;
        aRet >>= xTypeDesc;

        if( xTypeDesc.is() )
        {
            rRetType = Type( xTypeDesc->getTypeClass(), xTypeDesc->getName() );
            bSuccess = true;
        }
    }
    return bSuccess;
}



Any sbxToUnoValue( const SbxValue* pVar, const Type& rType, Property* pUnoProperty )
{
    Any aRetVal;

    
    if( pUnoProperty && pUnoProperty->Attributes & PropertyAttribute::MAYBEVOID )
    {
        if( pVar->IsEmpty() )
            return aRetVal;
    }

    SbxDataType eBaseType = pVar->SbxValue::GetType();
    if( eBaseType == SbxOBJECT )
    {
        SbxBaseRef xObj = (SbxBase*)pVar->GetObject();
        if( xObj.Is() && xObj->ISA(SbUnoAnyObject) )
        {
            return ((SbUnoAnyObject*)(SbxBase*)xObj)->getValue();
        }
    }

    TypeClass eType = rType.getTypeClass();
    switch( eType )
    {
        case TypeClass_INTERFACE:
        case TypeClass_STRUCT:
        case TypeClass_EXCEPTION:
        {
            Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( rType );

            
            if( pVar->IsNull() && eType == TypeClass_INTERFACE )
            {
                Reference< XInterface > xRef;
                OUString aClassName = xIdlTargetClass->getName();
                Type aClassType( xIdlTargetClass->getTypeClass(), aClassName.getStr() );
                aRetVal.setValue( &xRef, aClassType );
            }
            else
            {
                
                if( eType == TypeClass_STRUCT )
                {
                    SbiInstance* pInst = GetSbData()->pInst;
                    if( pInst && pInst->IsCompatibility() )
                    {
                        if( rType == ::getCppuType( (oleautomation::Decimal*)0 ) )
                        {
                            oleautomation::Decimal aDecimal;
                            pVar->fillAutomationDecimal( aDecimal );
                            aRetVal <<= aDecimal;
                            break;
                        }
                        else if( rType == ::getCppuType( (oleautomation::Currency*)0 ) )
                        {
                            
                            aRetVal <<= (sal_Int64)( pVar->GetInt64() );
                            break;
                        }
                        else if( rType == ::getCppuType( (oleautomation::Date*)0 ) )
                        {
                            oleautomation::Date aDate;
                            aDate.Value = pVar->GetDate();
                            aRetVal <<= aDate;
                            break;
                        }
                    }
                }

                SbxBaseRef pObj = (SbxBase*)pVar->GetObject();
                if( pObj && pObj->ISA(SbUnoObject) )
                {
                    aRetVal = ((SbUnoObject*)(SbxBase*)pObj)->getUnoAny();
                }
                else if( pObj && pObj->ISA(SbUnoStructRefObject) )
                {
                    aRetVal = ((SbUnoStructRefObject*)(SbxBase*)pObj)->getUnoAny();
                }
                else
                {
                    
                    Reference<XInterface> xInt;
                    aRetVal <<= xInt;
                }
            }
        }
        break;

        case TypeClass_TYPE:
        {
            if( eBaseType == SbxOBJECT )
            {
                
                Reference< XIdlClass > xIdlClass;

                SbxBaseRef pObj = (SbxBase*)pVar->GetObject();
                if( pObj && pObj->ISA(SbUnoObject) )
                {
                    Any aUnoAny = ((SbUnoObject*)(SbxBase*)pObj)->getUnoAny();
                    aUnoAny >>= xIdlClass;
                }

                if( xIdlClass.is() )
                {
                    OUString aClassName = xIdlClass->getName();
                    Type aType( xIdlClass->getTypeClass(), aClassName.getStr() );
                    aRetVal <<= aType;
                }
            }
            else if( eBaseType == SbxSTRING )
            {
                OUString aTypeName = pVar->GetOUString();
                Type aType;
                bool bSuccess = implGetTypeByName( aTypeName, aType );
                if( bSuccess )
                {
                    aRetVal <<= aType;
                }
            }
        }
        break;


        case TypeClass_ENUM:
        {
            aRetVal = int2enum( pVar->GetLong(), rType );
        }
        break;

        case TypeClass_SEQUENCE:
        {
            SbxBaseRef xObj = (SbxBase*)pVar->GetObject();
            if( xObj && xObj->ISA(SbxDimArray) )
            {
                SbxBase* pObj = (SbxBase*)xObj;
                SbxDimArray* pArray = (SbxDimArray*)pObj;

                short nDims = pArray->GetDims();

                
                sal_Int32 nLower, nUpper;
                if( nDims == 1 && pArray->GetDim32( 1, nLower, nUpper ) )
                {
                    sal_Int32 nSeqSize = nUpper - nLower + 1;

                    
                    Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( rType );
                    xIdlTargetClass->createObject( aRetVal );
                    Reference< XIdlArray > xArray = xIdlTargetClass->getArray();
                    xArray->realloc( aRetVal, nSeqSize );

                    
                    OUString aClassName = xIdlTargetClass->getName();
                    typelib_TypeDescription * pSeqTD = 0;
                    typelib_typedescription_getByName( &pSeqTD, aClassName.pData );
                    OSL_ASSERT( pSeqTD );
                    Type aElemType( ((typelib_IndirectTypeDescription *)pSeqTD)->pType );

                    
                    sal_Int32 nIdx = nLower;
                    for( sal_Int32 i = 0 ; i < nSeqSize ; i++,nIdx++ )
                    {
                        SbxVariableRef xVar = pArray->Get32( &nIdx );

                        
                        Any aAnyValue = sbxToUnoValue( (SbxVariable*)xVar, aElemType );

                        try
                        {
                            
                            xArray->set( aRetVal, i, aAnyValue );
                        }
                        catch( const IllegalArgumentException& )
                        {
                            StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
                                implGetExceptionMsg( ::cppu::getCaughtException() ) );
                        }
                        catch (const IndexOutOfBoundsException&)
                        {
                            StarBASIC::Error( SbERR_OUT_OF_RANGE );
                        }
                    }
                }
                
                else if( nDims > 1 )
                {
                    
                    typelib_TypeDescription * pSeqTD = 0;
                    Type aCurType( rType );
                    sal_Int32 nSeqLevel = 0;
                    Type aElemType;
                    do
                    {
                        OUString aTypeName = aCurType.getTypeName();
                        typelib_typedescription_getByName( &pSeqTD, aTypeName.pData );
                        OSL_ASSERT( pSeqTD );
                        if( pSeqTD->eTypeClass == typelib_TypeClass_SEQUENCE )
                        {
                            aCurType = Type( ((typelib_IndirectTypeDescription *)pSeqTD)->pType );
                            nSeqLevel++;
                        }
                        else
                        {
                            aElemType = aCurType;
                            break;
                        }
                    }
                    while( true );

                    if( nSeqLevel == nDims )
                    {
                        sal_Int32* pLowerBounds = new sal_Int32[nDims];
                        sal_Int32* pUpperBounds = new sal_Int32[nDims];
                        sal_Int32* pActualIndices = new sal_Int32[nDims];
                        for( short i = 1 ; i <= nDims ; i++ )
                        {
                            sal_Int32 lBound, uBound;
                            pArray->GetDim32( i, lBound, uBound );

                            short j = i - 1;
                            pActualIndices[j] = pLowerBounds[j] = lBound;
                            pUpperBounds[j] = uBound;
                        }

                        aRetVal = implRekMultiDimArrayToSequence( pArray, aElemType,
                            nDims - 1, 0, pActualIndices, pLowerBounds, pUpperBounds );

                        delete[] pUpperBounds;
                        delete[] pLowerBounds;
                        delete[] pActualIndices;
                    }
                }
            }
        }
        break;


        
        case TypeClass_ANY:
        {
            aRetVal = sbxToUnoValueImpl( pVar );
        }
        break;

        case TypeClass_BOOLEAN:
        {
            sal_Bool b = pVar->GetBool();
            aRetVal.setValue( &b, getBooleanCppuType() );
            break;
        }
        case TypeClass_CHAR:
        {
            sal_Unicode c = pVar->GetChar();
            aRetVal.setValue( &c , getCharCppuType() );
            break;
        }
        case TypeClass_STRING:          aRetVal <<= pVar->GetOUString(); break;
        case TypeClass_FLOAT:           aRetVal <<= pVar->GetSingle(); break;
        case TypeClass_DOUBLE:          aRetVal <<= pVar->GetDouble(); break;

        case TypeClass_BYTE:
        {
            sal_Int16 nVal = pVar->GetInteger();
            bool bOverflow = false;
            if( nVal < -128 )
            {
                bOverflow = true;
                nVal = -128;
            }
            else if( nVal > 127 )
            {
                bOverflow = true;
                nVal = 127;
            }
            if( bOverflow )
                   StarBASIC::Error( ERRCODE_BASIC_MATH_OVERFLOW );

            sal_Int8 nByteVal = (sal_Int8)nVal;
            aRetVal <<= nByteVal;
            break;
        }
        case TypeClass_SHORT:           aRetVal <<= (sal_Int16)( pVar->GetInteger() );  break;
        case TypeClass_LONG:            aRetVal <<= (sal_Int32)( pVar->GetLong() );     break;
        case TypeClass_HYPER:           aRetVal <<= (sal_Int64)( pVar->GetInt64() );    break;
        case TypeClass_UNSIGNED_SHORT:  aRetVal <<= (sal_uInt16)( pVar->GetUShort() );  break;
        case TypeClass_UNSIGNED_LONG:   aRetVal <<= (sal_uInt32)( pVar->GetULong() );   break;
        case TypeClass_UNSIGNED_HYPER:  aRetVal <<= (sal_uInt64)( pVar->GetUInt64() );  break;
        default: break;
    }

    return aRetVal;
}

void processAutomationParams( SbxArray* pParams, Sequence< Any >& args, bool bOLEAutomation, sal_uInt32 nParamCount )
{
    AutomationNamedArgsSbxArray* pArgNamesArray = NULL;
    if( bOLEAutomation )
        pArgNamesArray = PTR_CAST(AutomationNamedArgsSbxArray,pParams);

    args.realloc( nParamCount );
    Any* pAnyArgs = args.getArray();
    bool bBlockConversionToSmallestType = GetSbData()->pInst->IsCompatibility();
    sal_uInt32 i = 0;
    if( pArgNamesArray )
    {
        Sequence< OUString >& rNameSeq = pArgNamesArray->getNames();
        OUString* pNames = rNameSeq.getArray();
        Any aValAny;
        for( i = 0 ; i < nParamCount ; i++ )
        {
            sal_uInt16 iSbx = (sal_uInt16)(i+1);

            aValAny = sbxToUnoValueImpl( pParams->Get( iSbx ),
            bBlockConversionToSmallestType );

            OUString aParamName = pNames[iSbx];
            if( !aParamName.isEmpty() )
            {
                oleautomation::NamedArgument aNamedArgument;
                aNamedArgument.Name = aParamName;
                aNamedArgument.Value = aValAny;
                pAnyArgs[i] <<= aNamedArgument;
            }
            else
            {
                pAnyArgs[i] = aValAny;
            }
        }
    }
    else
    {
        for( i = 0 ; i < nParamCount ; i++ )
        {
            pAnyArgs[i] = sbxToUnoValueImpl( pParams->Get( (sal_uInt16)(i+1) ),
            bBlockConversionToSmallestType );
        }
    }

}
enum INVOKETYPE
{
   GetProp = 0,
   SetProp,
   Func
};
Any invokeAutomationMethod( const OUString& Name, Sequence< Any >& args, SbxArray* pParams, sal_uInt32 nParamCount, Reference< XInvocation >& rxInvocation, INVOKETYPE invokeType = Func )
{
    Sequence< sal_Int16 > OutParamIndex;
    Sequence< Any > OutParam;

    Any aRetAny;
    switch( invokeType )
    {
        case Func:
            aRetAny = rxInvocation->invoke( Name, args, OutParamIndex, OutParam );
            break;
        case GetProp:
            {
                Reference< XAutomationInvocation > xAutoInv( rxInvocation, UNO_QUERY );
                aRetAny = xAutoInv->invokeGetProperty( Name, args, OutParamIndex, OutParam );
                break;
            }
        case SetProp:
            {
                Reference< XAutomationInvocation > xAutoInv( rxInvocation, UNO_QUERY_THROW );
                aRetAny = xAutoInv->invokePutProperty( Name, args, OutParamIndex, OutParam );
                break;
            }
        default:
            break; 

    }
    const sal_Int16* pIndices = OutParamIndex.getConstArray();
    sal_uInt32 nLen = OutParamIndex.getLength();
    if( nLen )
    {
        const Any* pNewValues = OutParam.getConstArray();
        for( sal_uInt32 j = 0 ; j < nLen ; j++ )
        {
            sal_Int16 iTarget = pIndices[ j ];
            if( iTarget >= (sal_Int16)nParamCount )
                break;
            unoToSbxValue( (SbxVariable*)pParams->Get( (sal_uInt16)(j+1) ), pNewValues[ j ] );
        }
    }
    return aRetAny;
}


OUString Impl_GetInterfaceInfo( const Reference< XInterface >& x, const Reference< XIdlClass >& xClass, sal_uInt16 nRekLevel )
{
    Type aIfaceType = ::getCppuType( (const Reference< XInterface > *)0 );
    static Reference< XIdlClass > xIfaceClass = TypeToIdlClass( aIfaceType );

    OUStringBuffer aRetStr;
    for( sal_uInt16 i = 0 ; i < nRekLevel ; i++ )
        aRetStr.appendAscii( "    " );
    aRetStr.append( xClass->getName() );
    OUString aClassName = xClass->getName();
    Type aClassType( xClass->getTypeClass(), aClassName.getStr() );

    
    if( !x->queryInterface( aClassType ).hasValue() )
    {
        aRetStr.appendAscii( " (ERROR: Not really supported!)\n" );
    }
    
    else
    {
        aRetStr.appendAscii( "\n" );

        
        Sequence< Reference< XIdlClass > > aSuperClassSeq = xClass->getSuperclasses();
        const Reference< XIdlClass >* pClasses = aSuperClassSeq.getConstArray();
        sal_uInt32 nSuperIfaceCount = aSuperClassSeq.getLength();
        for( sal_uInt32 j = 0 ; j < nSuperIfaceCount ; j++ )
        {
            const Reference< XIdlClass >& rxIfaceClass = pClasses[j];
            if( !rxIfaceClass->equals( xIfaceClass ) )
                aRetStr.append( Impl_GetInterfaceInfo( x, rxIfaceClass, nRekLevel + 1 ) );
        }
    }
    return aRetStr.makeStringAndClear();
}

OUString getDbgObjectNameImpl( SbUnoObject* pUnoObj )
{
    OUString aName;
    if( pUnoObj )
    {
        aName = pUnoObj->GetClassName();
        if( aName.isEmpty() )
        {
            Any aToInspectObj = pUnoObj->getUnoAny();
            TypeClass eType = aToInspectObj.getValueType().getTypeClass();
            Reference< XInterface > xObj;
            if( eType == TypeClass_INTERFACE )
                xObj = *(Reference< XInterface >*)aToInspectObj.getValue();
            if( xObj.is() )
            {
                Reference< XServiceInfo > xServiceInfo( xObj, UNO_QUERY );
                if( xServiceInfo.is() )
                    aName = xServiceInfo->getImplementationName();
            }
        }
    }
    return aName;
}

OUString getDbgObjectName( SbUnoObject* pUnoObj )
{
    OUString aName = getDbgObjectNameImpl( pUnoObj );
    if( aName.isEmpty() )
        aName += "Unknown";

    OUStringBuffer aRet;
    if( aName.getLength() > 20 )
    {
        aRet.appendAscii( "\n" );
    }
    aRet.appendAscii( "\"" );
    aRet.append( aName );
    aRet.appendAscii( "\":" );
    return aRet.makeStringAndClear();
}

OUString getBasicObjectTypeName( SbxObject* pObj )
{
    OUString aName;
    if( pObj )
    {
        SbUnoObject* pUnoObj = PTR_CAST(SbUnoObject,pObj);
        SbUnoStructRefObject* pUnoStructObj = PTR_CAST(SbUnoStructRefObject,pObj);
        if( pUnoObj )
            aName = getDbgObjectNameImpl( pUnoObj );
        else if ( pUnoStructObj )
            aName = pUnoStructObj->GetClassName();
    }
    return aName;
}

bool checkUnoObjectType( SbUnoObject* pUnoObj, const OUString& rClass )
{
    Any aToInspectObj = pUnoObj->getUnoAny();
    TypeClass eType = aToInspectObj.getValueType().getTypeClass();
    if( eType != TypeClass_INTERFACE )
    {
        return false;
    }
    const Reference< XInterface > x = *(Reference< XInterface >*)aToInspectObj.getValue();

    
    Reference< XInvocation > xInvocation( x, UNO_QUERY );
    if( xInvocation.is() )
    {
        return true;
    }
    bool result = false;
    Reference< XTypeProvider > xTypeProvider( x, UNO_QUERY );
    if( xTypeProvider.is() )
    {
        /*  Although interfaces in the ooo.vba namespace obey the IDL rules and
            have a leading 'X', in Basic we want to be able to do something
            like 'Dim wb As Workbooks' or 'Dim lb As MSForms.Label'. Here we
            add a leading 'X' to the class name and a leading dot to the entire
            type name. This results e.g. in '.XWorkbooks' or '.MSForms.XLabel'
            which matches the interface names 'ooo.vba.excel.XWorkbooks' or
            'ooo.vba.msforms.XLabel'.
         */
        OUString aClassName;
        if ( SbiRuntime::isVBAEnabled() )
        {
            aClassName = ".";
            sal_Int32 nClassNameDot = rClass.lastIndexOf( '.' );
            if( nClassNameDot >= 0 )
            {
                aClassName += rClass.copy( 0, nClassNameDot + 1 ) + OUString( 'X' ) + rClass.copy( nClassNameDot + 1 );
            }
            else
            {
                aClassName += OUString( 'X' ) + rClass;
            }
        }
        else 
             
            aClassName = rClass;

        Sequence< Type > aTypeSeq = xTypeProvider->getTypes();
        const Type* pTypeArray = aTypeSeq.getConstArray();
        sal_uInt32 nIfaceCount = aTypeSeq.getLength();
        for( sal_uInt32 j = 0 ; j < nIfaceCount ; j++ )
        {
            const Type& rType = pTypeArray[j];

            Reference<XIdlClass> xClass = TypeToIdlClass( rType );
            if( !xClass.is() )
            {
                OSL_FAIL("failed to get XIdlClass for type");
                break;
            }
            OUString aInterfaceName = xClass->getName();
            if ( aInterfaceName == "com.sun.star.bridge.oleautomation.XAutomationObject" )
            {
                
                
                Reference< XInvocation > xInv( aToInspectObj, UNO_QUERY );
                if ( xInv.is() )
                {
                    OUString sTypeName;
                    xInv->getValue( OUString( "$GetTypeName" ) ) >>= sTypeName;
                    if ( sTypeName.isEmpty() || sTypeName == "IDispatch" )
                    {
                        
                        result = true;
                    }
                    else
                    {
                        result = sTypeName.equals( rClass );
                    }
                }
                break; 
            }

            
            OSL_TRACE("Checking if object implements %s", OUStringToOString( aClassName, RTL_TEXTENCODING_UTF8 ).getStr() );
            if ( (aClassName.getLength() <= aInterfaceName.getLength()) &&
                    aInterfaceName.matchIgnoreAsciiCase( aClassName, aInterfaceName.getLength() - aClassName.getLength() ) )
            {
                result = true;
                break;
            }
        }
    }
    return result;
}


OUString Impl_GetSupportedInterfaces( SbUnoObject* pUnoObj )
{
    Any aToInspectObj = pUnoObj->getUnoAny();

    
    TypeClass eType = aToInspectObj.getValueType().getTypeClass();
    OUStringBuffer aRet;
    if( eType != TypeClass_INTERFACE )
    {
        aRet.appendAscii( ID_DBG_SUPPORTEDINTERFACES );
        aRet.appendAscii( " not available.\n(TypeClass is not TypeClass_INTERFACE)\n" );
    }
    else
    {
        
        const Reference< XInterface > x = *(Reference< XInterface >*)aToInspectObj.getValue();

        Reference< XTypeProvider > xTypeProvider( x, UNO_QUERY );

        aRet.appendAscii( "Supported interfaces by object " );
        aRet.append( getDbgObjectName( pUnoObj ) );
        aRet.appendAscii( "\n" );
        if( xTypeProvider.is() )
        {
            
            Sequence< Type > aTypeSeq = xTypeProvider->getTypes();
            const Type* pTypeArray = aTypeSeq.getConstArray();
            sal_uInt32 nIfaceCount = aTypeSeq.getLength();
            for( sal_uInt32 j = 0 ; j < nIfaceCount ; j++ )
            {
                const Type& rType = pTypeArray[j];

                Reference<XIdlClass> xClass = TypeToIdlClass( rType );
                if( xClass.is() )
                {
                    aRet.append( Impl_GetInterfaceInfo( x, xClass, 1 ) );
                }
                else
                {
                    typelib_TypeDescription * pTD = 0;
                    rType.getDescription( &pTD );

                    aRet.appendAscii( "*** ERROR: No IdlClass for type \"" );
                    aRet.append( pTD->pTypeName );
                    aRet.appendAscii( "\"\n*** Please check type library\n" );
                }
            }
        }
    }
    return aRet.makeStringAndClear();
}




OUString Dbg_SbxDataType2String( SbxDataType eType )
{
    OUStringBuffer aRet;
    switch( +eType )
    {
        case SbxEMPTY:      aRet.appendAscii("SbxEMPTY"); break;
        case SbxNULL:       aRet.appendAscii("SbxNULL"); break;
        case SbxINTEGER:    aRet.appendAscii("SbxINTEGER"); break;
        case SbxLONG:       aRet.appendAscii("SbxLONG"); break;
        case SbxSINGLE:     aRet.appendAscii("SbxSINGLE"); break;
        case SbxDOUBLE:     aRet.appendAscii("SbxDOUBLE"); break;
        case SbxCURRENCY:   aRet.appendAscii("SbxCURRENCY"); break;
        case SbxDECIMAL:    aRet.appendAscii("SbxDECIMAL"); break;
        case SbxDATE:       aRet.appendAscii("SbxDATE"); break;
        case SbxSTRING:     aRet.appendAscii("SbxSTRING"); break;
        case SbxOBJECT:     aRet.appendAscii("SbxOBJECT"); break;
        case SbxERROR:      aRet.appendAscii("SbxERROR"); break;
        case SbxBOOL:       aRet.appendAscii("SbxBOOL"); break;
        case SbxVARIANT:    aRet.appendAscii("SbxVARIANT"); break;
        case SbxDATAOBJECT: aRet.appendAscii("SbxDATAOBJECT"); break;
        case SbxCHAR:       aRet.appendAscii("SbxCHAR"); break;
        case SbxBYTE:       aRet.appendAscii("SbxBYTE"); break;
        case SbxUSHORT:     aRet.appendAscii("SbxUSHORT"); break;
        case SbxULONG:      aRet.appendAscii("SbxULONG"); break;
        case SbxSALINT64:   aRet.appendAscii("SbxINT64"); break;
        case SbxSALUINT64:  aRet.appendAscii("SbxUINT64"); break;
        case SbxINT:        aRet.appendAscii("SbxINT"); break;
        case SbxUINT:       aRet.appendAscii("SbxUINT"); break;
        case SbxVOID:       aRet.appendAscii("SbxVOID"); break;
        case SbxHRESULT:    aRet.appendAscii("SbxHRESULT"); break;
        case SbxPOINTER:    aRet.appendAscii("SbxPOINTER"); break;
        case SbxDIMARRAY:   aRet.appendAscii("SbxDIMARRAY"); break;
        case SbxCARRAY:     aRet.appendAscii("SbxCARRAY"); break;
        case SbxUSERDEF:    aRet.appendAscii("SbxUSERDEF"); break;
        case SbxLPSTR:      aRet.appendAscii("SbxLPSTR"); break;
        case SbxLPWSTR:     aRet.appendAscii("SbxLPWSTR"); break;
        case SbxCoreSTRING: aRet.appendAscii("SbxCoreSTRING"); break;
        case SbxOBJECT | SbxARRAY: aRet.appendAscii("SbxARRAY"); break;
        default: aRet.appendAscii("Unknown Sbx-Type!");break;
    }
    return aRet.makeStringAndClear();
}


OUString Impl_DumpProperties( SbUnoObject* pUnoObj )
{
    OUStringBuffer aRet;
    aRet.appendAscii("Properties of object ");
    aRet.append( getDbgObjectName( pUnoObj ) );

    
    Reference< XIntrospectionAccess > xAccess = pUnoObj->getIntrospectionAccess();
    if( !xAccess.is() )
    {
        Reference< XInvocation > xInvok = pUnoObj->getInvocation();
        if( xInvok.is() )
            xAccess = xInvok->getIntrospection();
    }
    if( !xAccess.is() )
    {
        aRet.appendAscii( "\nUnknown, no introspection available\n" );
        return aRet.makeStringAndClear();
    }

    Sequence<Property> props = xAccess->getProperties( PropertyConcept::ALL - PropertyConcept::DANGEROUS );
    sal_uInt32 nUnoPropCount = props.getLength();
    const Property* pUnoProps = props.getConstArray();

    SbxArray* pProps = pUnoObj->GetProperties();
    sal_uInt16 nPropCount = pProps->Count();
    sal_uInt16 nPropsPerLine = 1 + nPropCount / 30;
    for( sal_uInt16 i = 0; i < nPropCount; i++ )
    {
        SbxVariable* pVar = pProps->Get( i );
        if( pVar )
        {
            OUStringBuffer aPropStr;
            if( (i % nPropsPerLine) == 0 )
                aPropStr.appendAscii( "\n" );

            
            
            SbxDataType eType = pVar->GetFullType();

            bool bMaybeVoid = false;
            if( i < nUnoPropCount )
            {
                const Property& rProp = pUnoProps[ i ];

                
                
                if( rProp.Attributes & PropertyAttribute::MAYBEVOID )
                {
                    eType = unoToSbxType( rProp.Type.getTypeClass() );
                    bMaybeVoid = true;
                }
                if( eType == SbxOBJECT )
                {
                    Type aType = rProp.Type;
                    if( aType.getTypeClass() == TypeClass_SEQUENCE )
                        eType = (SbxDataType) ( SbxOBJECT | SbxARRAY );
                }
            }
            aPropStr.append( Dbg_SbxDataType2String( eType ) );
            if( bMaybeVoid )
                aPropStr.appendAscii( "/void" );
            aPropStr.appendAscii( " " );
            aPropStr.append( pVar->GetName() );

            if( i == nPropCount - 1 )
                aPropStr.appendAscii( "\n" );
            else
                aPropStr.appendAscii( "; " );

            aRet.append( aPropStr.makeStringAndClear() );
        }
    }
    return aRet.makeStringAndClear();
}


OUString Impl_DumpMethods( SbUnoObject* pUnoObj )
{
    OUStringBuffer aRet;
    aRet.appendAscii("Methods of object ");
    aRet.append( getDbgObjectName( pUnoObj ) );

    
    Reference< XIntrospectionAccess > xAccess = pUnoObj->getIntrospectionAccess();
    if( !xAccess.is() )
    {
        Reference< XInvocation > xInvok = pUnoObj->getInvocation();
        if( xInvok.is() )
            xAccess = xInvok->getIntrospection();
    }
    if( !xAccess.is() )
    {
        aRet.appendAscii( "\nUnknown, no introspection available\n" );
        return aRet.makeStringAndClear();
    }
    Sequence< Reference< XIdlMethod > > methods = xAccess->getMethods
        ( MethodConcept::ALL - MethodConcept::DANGEROUS );
    const Reference< XIdlMethod >* pUnoMethods = methods.getConstArray();

    SbxArray* pMethods = pUnoObj->GetMethods();
    sal_uInt16 nMethodCount = pMethods->Count();
    if( !nMethodCount )
    {
        aRet.appendAscii( "\nNo methods found\n" );
        return aRet.makeStringAndClear();
    }
    sal_uInt16 nPropsPerLine = 1 + nMethodCount / 30;
    for( sal_uInt16 i = 0; i < nMethodCount; i++ )
    {
        SbxVariable* pVar = pMethods->Get( i );
        if( pVar )
        {
            if( (i % nPropsPerLine) == 0 )
                aRet.appendAscii( "\n" );

            
            const Reference< XIdlMethod >& rxMethod = pUnoMethods[i];

            
            SbxDataType eType = pVar->GetFullType();
            if( eType == SbxOBJECT )
            {
                Reference< XIdlClass > xClass = rxMethod->getReturnType();
                if( xClass.is() && xClass->getTypeClass() == TypeClass_SEQUENCE )
                    eType = (SbxDataType) ( SbxOBJECT | SbxARRAY );
            }
            
            aRet.append( Dbg_SbxDataType2String( eType ) );
            aRet.appendAscii( " " );
            aRet.append ( pVar->GetName() );
            aRet.appendAscii( " ( " );

            
            Sequence< Reference< XIdlClass > > aParamsSeq = rxMethod->getParameterTypes();
            sal_uInt32 nParamCount = aParamsSeq.getLength();
            const Reference< XIdlClass >* pParams = aParamsSeq.getConstArray();

            if( nParamCount > 0 )
            {
                for( sal_uInt16 j = 0; j < nParamCount; j++ )
                {
                    aRet.append ( Dbg_SbxDataType2String( unoToSbxType( pParams[ j ] ) ) );
                    if( j < nParamCount - 1 )
                        aRet.appendAscii( ", " );
                }
            }
            else
                aRet.appendAscii( "void" );

            aRet.appendAscii( " ) " );

            if( i == nMethodCount - 1 )
                aRet.appendAscii( "\n" );
            else
                aRet.appendAscii( "; " );
        }
    }
    return aRet.makeStringAndClear();
}

TYPEINIT1(AutomationNamedArgsSbxArray,SbxArray)


void SbUnoObject::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                           const SfxHint& rHint, const TypeId& rHintType )
{
    if( bNeedIntrospection )
        doIntrospection();

    const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbxArray* pParams = pVar->GetParameters();
        SbUnoProperty* pProp = PTR_CAST(SbUnoProperty,pVar);
        SbUnoMethod* pMeth = PTR_CAST(SbUnoMethod,pVar);
        if( pProp )
        {
            bool bInvocation = pProp->isInvocationBased();
            if( pHint->GetId() == SBX_HINT_DATAWANTED )
            {
                
                sal_Int32 nId = pProp->nId;
                if( nId < 0 )
                {
                    
                    if( nId == -1 )     
                    {
                        OUString aRetStr = Impl_GetSupportedInterfaces( this );
                        pVar->PutString( aRetStr );
                    }
                    
                    else if( nId == -2 )        
                    {
                        
                        implCreateAll();
                        OUString aRetStr = Impl_DumpProperties( this );
                        pVar->PutString( aRetStr );
                    }
                    
                    else if( nId == -3 )        
                    {
                        
                        implCreateAll();
                        OUString aRetStr = Impl_DumpMethods( this );
                        pVar->PutString( aRetStr );
                    }
                    return;
                }

                if( !bInvocation && mxUnoAccess.is() )
                {
                    try
                    {
                        if ( maStructInfo.get()  )
                        {
                            StructRefInfo aMember = maStructInfo->getStructMember( pProp->GetName() );
                            if ( aMember.isEmpty() )
                            {
                                 StarBASIC::Error( SbERR_PROPERTY_NOT_FOUND );
                            }
                            else
                            {
                                if ( pProp->isUnoStruct() )
                                {
                                    SbUnoStructRefObject* pSbUnoObject = new SbUnoStructRefObject( pProp->GetName(), aMember );
                                    SbxObjectRef xWrapper = (SbxObject*)pSbUnoObject;
                                    pVar->PutObject( xWrapper );
                                }
                                else
                                {
                                    Any aRetAny = aMember.getValue();
                                    
                                    unoToSbxValue( pVar, aRetAny );
                                }
                                return;
                            }
                        }
                        
                        Reference< XPropertySet > xPropSet( mxUnoAccess->queryAdapter( ::getCppuType( (const Reference< XPropertySet > *)0 ) ), UNO_QUERY );
                        Any aRetAny = xPropSet->getPropertyValue( pProp->GetName() );
                        
                        
                        

                        
                        unoToSbxValue( pVar, aRetAny );
                    }
                    catch( const Exception& )
                    {
                        implHandleAnyException( ::cppu::getCaughtException() );
                    }
                }
                else if( bInvocation && mxInvocation.is() )
                {
                    try
                    {
                        sal_uInt32 nParamCount = pParams ? ((sal_uInt32)pParams->Count() - 1) : 0;
                        sal_Bool bCanBeConsideredAMethod = mxInvocation->hasMethod( pProp->GetName() );
                        Any aRetAny;
                        if ( bCanBeConsideredAMethod && nParamCount )
                        {
                            
                            
                            Sequence<Any> args;
                            processAutomationParams( pParams, args, true, nParamCount );
                            aRetAny = invokeAutomationMethod( pProp->GetName(), args, pParams, nParamCount, mxInvocation, GetProp );
                        }
                        else
                            aRetAny = mxInvocation->getValue( pProp->GetName() );
                        
                        unoToSbxValue( pVar, aRetAny );
                        if( pParams && bCanBeConsideredAMethod )
                            pVar->SetParameters( NULL );

                    }
                    catch( const Exception& )
                    {
                        implHandleAnyException( ::cppu::getCaughtException() );
                    }
                }
            }
            else if( pHint->GetId() == SBX_HINT_DATACHANGED )
            {
                if( !bInvocation && mxUnoAccess.is() )
                {
                    if( pProp->aUnoProp.Attributes & PropertyAttribute::READONLY )
                    {
                        StarBASIC::Error( SbERR_PROP_READONLY );
                        return;
                    }
                    if (  maStructInfo.get()  )
                    {
                        StructRefInfo aMember = maStructInfo->getStructMember( pProp->GetName() );
                        if ( aMember.isEmpty() )
                        {
                            StarBASIC::Error( SbERR_PROPERTY_NOT_FOUND );
                        }
                        else
                        {
                            Any aAnyValue = sbxToUnoValue( pVar, pProp->aUnoProp.Type, &pProp->aUnoProp );
                            aMember.setValue( aAnyValue );
                        }
                        return;
                   }
                    
                    Any aAnyValue = sbxToUnoValue( pVar, pProp->aUnoProp.Type, &pProp->aUnoProp );
                    try
                    {
                        
                        Reference< XPropertySet > xPropSet( mxUnoAccess->queryAdapter( ::getCppuType( (const Reference< XPropertySet > *)0 ) ), UNO_QUERY );
                        xPropSet->setPropertyValue( pProp->GetName(), aAnyValue );
                        
                        
                        
                    }
                    catch( const Exception& )
                    {
                        implHandleAnyException( ::cppu::getCaughtException() );
                    }
                }
                else if( bInvocation && mxInvocation.is() )
                {
                    
                    Any aAnyValue = sbxToUnoValueImpl( pVar );
                    try
                    {
                        
                        mxInvocation->setValue( pProp->GetName(), aAnyValue );
                    }
                    catch( const Exception& )
                    {
                        implHandleAnyException( ::cppu::getCaughtException() );
                    }
                }
            }
        }
        else if( pMeth )
        {
            bool bInvocation = pMeth->isInvocationBased();
            if( pHint->GetId() == SBX_HINT_DATAWANTED )
            {
                
                sal_uInt32 nParamCount = pParams ? ((sal_uInt32)pParams->Count() - 1) : 0;
                Sequence<Any> args;
                bool bOutParams = false;
                sal_uInt32 i;

                if( !bInvocation && mxUnoAccess.is() )
                {
                    
                    const Sequence<ParamInfo>& rInfoSeq = pMeth->getParamInfos();
                    const ParamInfo* pParamInfos = rInfoSeq.getConstArray();
                    sal_uInt32 nUnoParamCount = rInfoSeq.getLength();
                    sal_uInt32 nAllocParamCount = nParamCount;

                    
                    if( nParamCount > nUnoParamCount )
                    {
                        nParamCount = nUnoParamCount;
                        nAllocParamCount = nParamCount;
                    }
                    else if( nParamCount < nUnoParamCount )
                    {
                        SbiInstance* pInst = GetSbData()->pInst;
                        if( pInst && pInst->IsCompatibility() )
                        {
                            
                            bool bError = false;
                            for( i = nParamCount ; i < nUnoParamCount ; i++ )
                            {
                                const ParamInfo& rInfo = pParamInfos[i];
                                const Reference< XIdlClass >& rxClass = rInfo.aType;
                                if( rxClass->getTypeClass() != TypeClass_ANY )
                                {
                                    bError = true;
                                    StarBASIC::Error( SbERR_NOT_OPTIONAL );
                                }
                            }
                            if( !bError )
                                nAllocParamCount = nUnoParamCount;
                        }
                    }

                    if( nAllocParamCount > 0 )
                    {
                        args.realloc( nAllocParamCount );
                        Any* pAnyArgs = args.getArray();
                        for( i = 0 ; i < nParamCount ; i++ )
                        {
                            const ParamInfo& rInfo = pParamInfos[i];
                            const Reference< XIdlClass >& rxClass = rInfo.aType;

                            com::sun::star::uno::Type aType( rxClass->getTypeClass(), rxClass->getName() );

                            
                            pAnyArgs[i] = sbxToUnoValue( pParams->Get( (sal_uInt16)(i+1) ), aType );

                            
                            if( !bOutParams )
                            {
                                ParamMode aParamMode = rInfo.aMode;
                                if( aParamMode != ParamMode_IN )
                                    bOutParams = true;
                            }
                        }
                    }
                }
                else if( bInvocation && pParams && mxInvocation.is() )
                {
                    bool bOLEAutomation = true;
                    processAutomationParams( pParams, args, bOLEAutomation, nParamCount );
                }

                
                GetSbData()->bBlockCompilerError = true;  
                try
                {
                    if( !bInvocation && mxUnoAccess.is() )
                    {
                        Any aRetAny = pMeth->m_xUnoMethod->invoke( getUnoAny(), args );

                        
                        unoToSbxValue( pVar, aRetAny );

                        
                        if( bOutParams )
                        {
                            const Any* pAnyArgs = args.getConstArray();

                            
                            const Sequence<ParamInfo>& rInfoSeq = pMeth->getParamInfos();
                            const ParamInfo* pParamInfos = rInfoSeq.getConstArray();

                            sal_uInt32 j;
                            for( j = 0 ; j < nParamCount ; j++ )
                            {
                                const ParamInfo& rInfo = pParamInfos[j];
                                ParamMode aParamMode = rInfo.aMode;
                                if( aParamMode != ParamMode_IN )
                                    unoToSbxValue( (SbxVariable*)pParams->Get( (sal_uInt16)(j+1) ), pAnyArgs[ j ] );
                            }
                        }
                    }
                    else if( bInvocation && mxInvocation.is() )
                    {
                        Any aRetAny = invokeAutomationMethod( pMeth->GetName(), args, pParams, nParamCount, mxInvocation );
                        unoToSbxValue( pVar, aRetAny );
                        }

                    
                    
                    if( pParams )
                        pVar->SetParameters( NULL );
                }
                catch( const Exception& )
                {
                    implHandleAnyException( ::cppu::getCaughtException() );
                }
                GetSbData()->bBlockCompilerError = false;  
            }
        }
        else
            SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
    }
}


SbUnoObject::SbUnoObject( const OUString& aName_, const Any& aUnoObj_ )
    : SbxObject( aName_ )
    , bNeedIntrospection( true )
    , bNativeCOMObject( false )
{
    static Reference< XIntrospection > xIntrospection;

    
    Remove( OUString("Name"), SbxCLASS_DONTCARE );
    Remove( OUString("Parent"), SbxCLASS_DONTCARE );

    
    TypeClass eType = aUnoObj_.getValueType().getTypeClass();
    Reference< XInterface > x;
    if( eType == TypeClass_INTERFACE )
    {
        
        x = *(Reference< XInterface >*)aUnoObj_.getValue();
        if( !x.is() )
            return;
    }

    Reference< XTypeProvider > xTypeProvider;
    
    mxInvocation = Reference< XInvocation >( x, UNO_QUERY );

    xTypeProvider = Reference< XTypeProvider >( x, UNO_QUERY );

    if( mxInvocation.is() )
    {

        
        mxExactNameInvocation = Reference< XExactName >::query( mxInvocation );

        
        if( !xTypeProvider.is() )
        {
            bNeedIntrospection = false;
            return;
        }

        
        
        Reference< oleautomation::XAutomationObject > xAutomationObject( aUnoObj_, UNO_QUERY );
        if( xAutomationObject.is() )
            bNativeCOMObject = true;
    }

    maTmpUnoObj = aUnoObj_;


    
    bool bFatalError = true;

    
    bool bSetClassName = false;
    OUString aClassName_;
    if( eType == TypeClass_STRUCT || eType == TypeClass_EXCEPTION )
    {
        
        bFatalError = false;

        
        if( aName_.isEmpty() )
        {
            aClassName_ = aUnoObj_.getValueType().getTypeName();
            bSetClassName = true;
        }
        typelib_TypeDescription * pDeclTD = 0;
        typelib_typedescription_getByName( &pDeclTD, maTmpUnoObj.getValueTypeName().pData );
        StructRefInfo aThisStruct( maTmpUnoObj, pDeclTD, 0 );
        maStructInfo.reset( new SbUnoStructRefObject( GetName(), aThisStruct ) );
    }
    else if( eType == TypeClass_INTERFACE )
    {
        
        bFatalError = false;
    }
    if( bSetClassName )
        SetClassName( aClassName_ );

    
    if( bFatalError )
    {
        StarBASIC::FatalError( ERRCODE_BASIC_EXCEPTION );
        return;
    }

    
}

SbUnoObject::~SbUnoObject()
{
}



void SbUnoObject::doIntrospection( void )
{
    static Reference< XIntrospection > xIntrospection;

    if( !bNeedIntrospection )
        return;
    bNeedIntrospection = false;

    if( !xIntrospection.is() )
    {
        
        Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );
        xIntrospection = Introspection::create( xContext );
    }

    
    try
    {
        mxUnoAccess = xIntrospection->inspect( maTmpUnoObj );
    }
    catch( const RuntimeException& e )
    {
        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e ) );
    }

    if( !mxUnoAccess.is() )
    {
        
        return;
    }

    
    mxMaterialHolder = Reference< XMaterialHolder >::query( mxUnoAccess );

    
    mxExactName = Reference< XExactName >::query( mxUnoAccess );
}





static SbUnoMethod* pFirst = NULL;

void clearUnoMethodsForBasic( StarBASIC* pBasic )
{
    SbUnoMethod* pMeth = pFirst;
    while( pMeth )
    {
        SbxObject* pObject = dynamic_cast< SbxObject* >( pMeth->GetParent() );
        if ( pObject )
        {
            StarBASIC* pModBasic = dynamic_cast< StarBASIC* >( pObject->GetParent() );
            if ( pModBasic == pBasic )
            {
                
                
                
                
                

                if( pMeth == pFirst )
                    pFirst = pMeth->pNext;
                else if( pMeth->pPrev )
                    pMeth->pPrev->pNext = pMeth->pNext;
                if( pMeth->pNext )
                    pMeth->pNext->pPrev = pMeth->pPrev;

                pMeth->pPrev = NULL;
                pMeth->pNext = NULL;

                pMeth->SbxValue::Clear();
                pObject->SbxValue::Clear();

                
                pMeth = pFirst;
            }
            else
                pMeth = pMeth->pNext;
        }
        else
            pMeth = pMeth->pNext;
    }
}

void clearUnoMethods( void )
{
    SbUnoMethod* pMeth = pFirst;
    while( pMeth )
    {
        pMeth->SbxValue::Clear();
        pMeth = pMeth->pNext;
    }
}


SbUnoMethod::SbUnoMethod
(
    const OUString& aName_,
    SbxDataType eSbxType,
    Reference< XIdlMethod > xUnoMethod_,
    bool bInvocation,
    bool bDirect
)
    : SbxMethod( aName_, eSbxType )
    , mbInvocation( bInvocation )
    , mbDirectInvocation( bDirect )
{
    m_xUnoMethod = xUnoMethod_;
    pParamInfoSeq = NULL;

    
    pNext = pFirst;
    pPrev = NULL;
    pFirst = this;
    if( pNext )
        pNext->pPrev = this;
}

SbUnoMethod::~SbUnoMethod()
{
    delete pParamInfoSeq;

    if( this == pFirst )
        pFirst = pNext;
    else if( pPrev )
        pPrev->pNext = pNext;
    if( pNext )
        pNext->pPrev = pPrev;
}

SbxInfo* SbUnoMethod::GetInfo()
{
    if( !pInfo && m_xUnoMethod.is() )
    {
        SbiInstance* pInst = GetSbData()->pInst;
        if( pInst && pInst->IsCompatibility() )
        {
            pInfo = new SbxInfo();

            const Sequence<ParamInfo>& rInfoSeq = getParamInfos();
            const ParamInfo* pParamInfos = rInfoSeq.getConstArray();
            sal_uInt32 nParamCount = rInfoSeq.getLength();

            for( sal_uInt32 i = 0 ; i < nParamCount ; i++ )
            {
                const ParamInfo& rInfo = pParamInfos[i];
                OUString aParamName = rInfo.aName;

                SbxDataType t = SbxVARIANT;
                sal_uInt16 nFlags_ = SBX_READ;
                pInfo->AddParam( aParamName, t, nFlags_ );
            }
        }
    }
    return pInfo;
}

const Sequence<ParamInfo>& SbUnoMethod::getParamInfos( void )
{
    if( !pParamInfoSeq && m_xUnoMethod.is() )
    {
        Sequence<ParamInfo> aTmp = m_xUnoMethod->getParameterInfos() ;
        pParamInfoSeq = new Sequence<ParamInfo>( aTmp );
    }
    return *pParamInfoSeq;
}

SbUnoProperty::SbUnoProperty
(
    const OUString& aName_,
    SbxDataType eSbxType,
    SbxDataType eRealSbxType,
    const Property& aUnoProp_,
    sal_Int32 nId_,
    bool bInvocation,
    bool bUnoStruct
)
    : SbxProperty( aName_, eSbxType )
    , aUnoProp( aUnoProp_ )
    , nId( nId_ )
    , mbInvocation( bInvocation )
    , mRealType( eRealSbxType )
    , mbUnoStruct( bUnoStruct )
{
    
    static SbxArrayRef xDummyArray = new SbxArray( SbxVARIANT );
    if( eSbxType & SbxARRAY )
        PutObject( xDummyArray );
}

SbUnoProperty::~SbUnoProperty()
{}


SbxVariable* SbUnoObject::Find( const OUString& rName, SbxClassType t )
{
    static Reference< XIdlMethod > xDummyMethod;
    static Property aDummyProp;

    SbxVariable* pRes = SbxObject::Find( rName, t );

    if( bNeedIntrospection )
        doIntrospection();

    
    
    if( !pRes )
    {
        OUString aUName( rName );
        if( mxUnoAccess.is() && !bNativeCOMObject )
        {
            if( mxExactName.is() )
            {
                OUString aUExactName = mxExactName->getExactName( aUName );
                if( !aUExactName.isEmpty() )
                {
                    aUName = aUExactName;
                }
            }
            if( mxUnoAccess->hasProperty( aUName, PropertyConcept::ALL - PropertyConcept::DANGEROUS ) )
            {
                const Property& rProp = mxUnoAccess->
                    getProperty( aUName, PropertyConcept::ALL - PropertyConcept::DANGEROUS );

                
                SbxDataType eSbxType;
                if( rProp.Attributes & PropertyAttribute::MAYBEVOID )
                    eSbxType = SbxVARIANT;
                else
                    eSbxType = unoToSbxType( rProp.Type.getTypeClass() );

                SbxDataType eRealSbxType = ( ( rProp.Attributes & PropertyAttribute::MAYBEVOID ) ? unoToSbxType( rProp.Type.getTypeClass() ) : eSbxType );
                
                SbUnoProperty* pProp = new SbUnoProperty( rProp.Name, eSbxType, eRealSbxType, rProp, 0, false, ( rProp.Type.getTypeClass() ==  com::sun::star::uno::TypeClass_STRUCT  ) );
                SbxVariableRef xVarRef = pProp;
                QuickInsert( (SbxVariable*)xVarRef );
                pRes = xVarRef;
            }
            else if( mxUnoAccess->hasMethod( aUName,
                MethodConcept::ALL - MethodConcept::DANGEROUS ) )
            {
                
                const Reference< XIdlMethod >& rxMethod = mxUnoAccess->
                    getMethod( aUName, MethodConcept::ALL - MethodConcept::DANGEROUS );

                
                SbxVariableRef xMethRef = new SbUnoMethod( rxMethod->getName(),
                    unoToSbxType( rxMethod->getReturnType() ), rxMethod, false );
                QuickInsert( (SbxVariable*)xMethRef );
                pRes = xMethRef;
            }

            
            if( !pRes )
            {
                try
                {
                    Reference< XNameAccess > xNameAccess( mxUnoAccess->queryAdapter( ::getCppuType( (const Reference< XPropertySet > *)0 ) ), UNO_QUERY );
                    OUString aUName2( rName );

                    if( xNameAccess.is() && xNameAccess->hasByName( aUName2 ) )
                    {
                        Any aAny = xNameAccess->getByName( aUName2 );

                        
                        
                        
                        
                        
                        
                        
                        pRes = new SbxVariable( SbxVARIANT );
                        unoToSbxValue( pRes, aAny );
                    }
                }
                catch( const NoSuchElementException& e )
                {
                    StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e ) );
                }
                catch( const Exception& )
                {
                    
                    if( !pRes )
                        pRes = new SbxVariable( SbxVARIANT );

                    implHandleAnyException( ::cppu::getCaughtException() );
                }
            }
        }
        if( !pRes && mxInvocation.is() )
        {
            if( mxExactNameInvocation.is() )
            {
                OUString aUExactName = mxExactNameInvocation->getExactName( aUName );
                if( !aUExactName.isEmpty() )
                {
                    aUName = aUExactName;
                }
            }

            try
            {
                if( mxInvocation->hasProperty( aUName ) )
                {
                    
                    SbxVariableRef xVarRef = new SbUnoProperty( aUName, SbxVARIANT, SbxVARIANT, aDummyProp, 0, true, false );
                    QuickInsert( (SbxVariable*)xVarRef );
                    pRes = xVarRef;
                }
                else if( mxInvocation->hasMethod( aUName ) )
                {
                    
                    SbxVariableRef xMethRef = new SbUnoMethod( aUName, SbxVARIANT, xDummyMethod, true );
                    QuickInsert( (SbxVariable*)xMethRef );
                    pRes = xMethRef;
                }
                else
                {
                    Reference< XDirectInvocation > xDirectInvoke( mxInvocation, UNO_QUERY );
                    if ( xDirectInvoke.is() && xDirectInvoke->hasMember( aUName ) )
                    {
                        SbxVariableRef xMethRef = new SbUnoMethod( aUName, SbxVARIANT, xDummyMethod, true, true );
                        QuickInsert( (SbxVariable*)xMethRef );
                        pRes = xMethRef;
                    }

                }
            }
            catch( const RuntimeException& e )
            {
                
                if( !pRes )
                    pRes = new SbxVariable( SbxVARIANT );

                StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e ) );
            }
        }
    }

    

    if( !pRes )
    {
        if( rName.equalsIgnoreAsciiCase(ID_DBG_SUPPORTEDINTERFACES) ||
            rName.equalsIgnoreAsciiCase(ID_DBG_PROPERTIES) ||
            rName.equalsIgnoreAsciiCase(ID_DBG_METHODS) )
        {
            
            implCreateDbgProperties();

            
            pRes = SbxObject::Find( rName, SbxCLASS_DONTCARE );
        }
    }
    return pRes;
}



void SbUnoObject::implCreateDbgProperties( void )
{
    Property aProp;

    
    SbxVariableRef xVarRef = new SbUnoProperty( OUString(ID_DBG_SUPPORTEDINTERFACES), SbxSTRING, SbxSTRING, aProp, -1, false, false );
    QuickInsert( (SbxVariable*)xVarRef );

    
    xVarRef = new SbUnoProperty( OUString(ID_DBG_PROPERTIES), SbxSTRING, SbxSTRING, aProp, -2, false, false );
    QuickInsert( (SbxVariable*)xVarRef );

    
    xVarRef = new SbUnoProperty( OUString(ID_DBG_METHODS), SbxSTRING, SbxSTRING, aProp, -3, false, false );
    QuickInsert( (SbxVariable*)xVarRef );
}

void SbUnoObject::implCreateAll( void )
{
    
    pMethods   = new SbxArray;
    pProps     = new SbxArray;

    if( bNeedIntrospection ) doIntrospection();

    
    Reference< XIntrospectionAccess > xAccess = mxUnoAccess;
    if( !xAccess.is() || bNativeCOMObject )
    {
        if( mxInvocation.is() )
            xAccess = mxInvocation->getIntrospection();
        else if( bNativeCOMObject )
            return;
    }
    if( !xAccess.is() )
        return;

    
    Sequence<Property> props = xAccess->getProperties( PropertyConcept::ALL - PropertyConcept::DANGEROUS );
    sal_uInt32 nPropCount = props.getLength();
    const Property* pProps_ = props.getConstArray();

    sal_uInt32 i;
    for( i = 0 ; i < nPropCount ; i++ )
    {
        const Property& rProp = pProps_[ i ];

        
        SbxDataType eSbxType;
        if( rProp.Attributes & PropertyAttribute::MAYBEVOID )
            eSbxType = SbxVARIANT;
        else
            eSbxType = unoToSbxType( rProp.Type.getTypeClass() );

        SbxDataType eRealSbxType = ( ( rProp.Attributes & PropertyAttribute::MAYBEVOID ) ? unoToSbxType( rProp.Type.getTypeClass() ) : eSbxType );
        
        SbxVariableRef xVarRef = new SbUnoProperty( rProp.Name, eSbxType, eRealSbxType, rProp, i, false, ( rProp.Type.getTypeClass() == com::sun::star::uno::TypeClass_STRUCT   ) );
        QuickInsert( (SbxVariable*)xVarRef );
    }

    
    implCreateDbgProperties();

    
    Sequence< Reference< XIdlMethod > > aMethodSeq = xAccess->getMethods
        ( MethodConcept::ALL - MethodConcept::DANGEROUS );
    sal_uInt32 nMethCount = aMethodSeq.getLength();
    const Reference< XIdlMethod >* pMethods_ = aMethodSeq.getConstArray();
    for( i = 0 ; i < nMethCount ; i++ )
    {
        
        const Reference< XIdlMethod >& rxMethod = pMethods_[i];

        
        SbxVariableRef xMethRef = new SbUnoMethod
            ( rxMethod->getName(), unoToSbxType( rxMethod->getReturnType() ), rxMethod, false );
        QuickInsert( (SbxVariable*)xMethRef );
    }
}



Any SbUnoObject::getUnoAny( void )
{
    Any aRetAny;
    if( bNeedIntrospection ) doIntrospection();
    if ( maStructInfo.get() )
       aRetAny = maTmpUnoObj;
    else if( mxMaterialHolder.is() )
        aRetAny = mxMaterialHolder->getMaterial();
    else if( mxInvocation.is() )
        aRetAny <<= mxInvocation;
    return aRetAny;
}


SbUnoObject* Impl_CreateUnoStruct( const OUString& aClassName )
{
    
    Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
    if( !xCoreReflection.is() )
        return NULL;

    
    Reference< XIdlClass > xClass;
    Reference< XHierarchicalNameAccess > xHarryName =
        getCoreReflection_HierarchicalNameAccess_Impl();
    if( xHarryName.is() && xHarryName->hasByHierarchicalName( aClassName ) )
        xClass = xCoreReflection->forName( aClassName );
    if( !xClass.is() )
        return NULL;

    
    TypeClass eType = xClass->getTypeClass();
    if ( ( eType != TypeClass_STRUCT ) && ( eType != TypeClass_EXCEPTION ) )
        return NULL;

    
    Any aNewAny;
    xClass->createObject( aNewAny );
    
    SbUnoObject* pUnoObj = new SbUnoObject( aClassName, aNewAny );
    return pUnoObj;
}



SbxBase* SbUnoFactory::Create( sal_uInt16, sal_uInt32 )
{
    
    return NULL;
}

SbxObject* SbUnoFactory::CreateObject( const OUString& rClassName )
{
    return Impl_CreateUnoStruct( rClassName );
}




SbxObjectRef GetSbUnoObject( const OUString& aName, const Any& aUnoObj_ )
{
    return new SbUnoObject( aName, aUnoObj_ );
}


void createAllObjectProperties( SbxObject* pObj )
{
    if( !pObj )
        return;

    SbUnoObject* pUnoObj = PTR_CAST(SbUnoObject,pObj);
    SbUnoStructRefObject* pUnoStructObj = PTR_CAST(SbUnoStructRefObject,pObj);
    if( pUnoObj )
    {
        pUnoObj->createAllProperties();
    }
    else if ( pUnoStructObj )
    {
        pUnoStructObj->createAllProperties();
    }
    else
    {
        pObj->GetAll( SbxCLASS_DONTCARE );
    }
}


void RTL_Impl_CreateUnoStruct( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    
    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    
    OUString aClassName = rPar.Get(1)->GetOUString();

    
    SbUnoObjectRef xUnoObj = Impl_CreateUnoStruct( aClassName );
    if( !xUnoObj )
    {
        return;
    }
    
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutObject( (SbUnoObject*)xUnoObj );
}

void RTL_Impl_CreateUnoService( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    
    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    
    OUString aServiceName = rPar.Get(1)->GetOUString();

    
    Reference< XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory() );
    Reference< XInterface > xInterface;
    try
    {
        xInterface = xFactory->createInstance( aServiceName );
    }
    catch( const Exception& )
    {
        implHandleAnyException( ::cppu::getCaughtException() );
    }

    SbxVariableRef refVar = rPar.Get(0);
    if( xInterface.is() )
    {
        Any aAny;
        aAny <<= xInterface;

        
        SbUnoObjectRef xUnoObj = new SbUnoObject( aServiceName, aAny );
        if( xUnoObj->getUnoAny().getValueType().getTypeClass() != TypeClass_VOID )
        {
            
            refVar->PutObject( (SbUnoObject*)xUnoObj );
        }
        else
        {
            refVar->PutObject( NULL );
        }
    }
    else
    {
        refVar->PutObject( NULL );
    }
}

void RTL_Impl_CreateUnoServiceWithArguments( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    
    if ( rPar.Count() < 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    
    OUString aServiceName = rPar.Get(1)->GetOUString();
    Any aArgAsAny = sbxToUnoValue( rPar.Get(2),
                getCppuType( (Sequence<Any>*)0 ) );
    Sequence< Any > aArgs;
    aArgAsAny >>= aArgs;

    
    Reference< XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory() );
    Reference< XInterface > xInterface;
    try
    {
        xInterface = xFactory->createInstanceWithArguments( aServiceName, aArgs );
    }
    catch( const Exception& )
    {
        implHandleAnyException( ::cppu::getCaughtException() );
    }

    SbxVariableRef refVar = rPar.Get(0);
    if( xInterface.is() )
    {
        Any aAny;
        aAny <<= xInterface;

        
        SbUnoObjectRef xUnoObj = new SbUnoObject( aServiceName, aAny );
        if( xUnoObj->getUnoAny().getValueType().getTypeClass() != TypeClass_VOID )
        {
            
            refVar->PutObject( (SbUnoObject*)xUnoObj );
        }
        else
        {
            refVar->PutObject( NULL );
        }
    }
    else
    {
        refVar->PutObject( NULL );
    }
}

void RTL_Impl_GetProcessServiceManager( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    SbxVariableRef refVar = rPar.Get(0);

    
    Reference< XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory() );
    Any aAny;
    aAny <<= xFactory;

    
    SbUnoObjectRef xUnoObj = new SbUnoObject( OUString( "ProcessServiceManager" ), aAny );
    refVar->PutObject( (SbUnoObject*)xUnoObj );
}

void RTL_Impl_HasInterfaces( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    
    sal_uInt16 nParCount = rPar.Count();
    if( nParCount < 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutBool( sal_False );

    
    SbxBaseRef pObj = (SbxBase*)rPar.Get( 1 )->GetObject();
    if( !(pObj && pObj->ISA(SbUnoObject)) )
    {
        return;
    }
    Any aAny = ((SbUnoObject*)(SbxBase*)pObj)->getUnoAny();
    TypeClass eType = aAny.getValueType().getTypeClass();
    if( eType != TypeClass_INTERFACE )
    {
        return;
    }
    
    Reference< XInterface > x = *(Reference< XInterface >*)aAny.getValue();

    
    Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
    if( !xCoreReflection.is() )
    {
        return;
    }
    for( sal_uInt16 i = 2 ; i < nParCount ; i++ )
    {
        
        OUString aIfaceName = rPar.Get( i )->GetOUString();

        
        Reference< XIdlClass > xClass = xCoreReflection->forName( aIfaceName );
        if( !xClass.is() )
        {
            return;
        }
        
        OUString aClassName = xClass->getName();
        Type aClassType( xClass->getTypeClass(), aClassName.getStr() );
        if( !x->queryInterface( aClassType ).hasValue() )
        {
            return;
        }
    }

    
    refVar->PutBool( sal_True );
}

void RTL_Impl_IsUnoStruct( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    
    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutBool( sal_False );

    
    SbxVariableRef xParam = rPar.Get( 1 );
    if( !xParam->IsObject() )
    {
        return;
    }
    SbxBaseRef pObj = (SbxBase*)rPar.Get( 1 )->GetObject();
    if( !(pObj && pObj->ISA(SbUnoObject)) )
    {
        return;
    }
    Any aAny = ((SbUnoObject*)(SbxBase*)pObj)->getUnoAny();
    TypeClass eType = aAny.getValueType().getTypeClass();
    if( eType == TypeClass_STRUCT )
    {
        refVar->PutBool( sal_True );
    }
}


void RTL_Impl_EqualUnoObjects( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutBool( sal_False );

    
    SbxVariableRef xParam1 = rPar.Get( 1 );
    if( !xParam1->IsObject() )
    {
        return;
    }
    SbxBaseRef pObj1 = (SbxBase*)xParam1->GetObject();
    if( !(pObj1 && pObj1->ISA(SbUnoObject)) )
    {
        return;
    }
    Any aAny1 = ((SbUnoObject*)(SbxBase*)pObj1)->getUnoAny();
    TypeClass eType1 = aAny1.getValueType().getTypeClass();
    if( eType1 != TypeClass_INTERFACE )
    {
        return;
    }
    Reference< XInterface > x1;
    aAny1 >>= x1;

    SbxVariableRef xParam2 = rPar.Get( 2 );
    if( !xParam2->IsObject() )
    {
        return;
    }
    SbxBaseRef pObj2 = (SbxBase*)xParam2->GetObject();
    if( !(pObj2 && pObj2->ISA(SbUnoObject)) )
    {
        return;
    }
    Any aAny2 = ((SbUnoObject*)(SbxBase*)pObj2)->getUnoAny();
    TypeClass eType2 = aAny2.getValueType().getTypeClass();
    if( eType2 != TypeClass_INTERFACE )
    {
        return;
    }
    Reference< XInterface > x2;
    aAny2 >>= x2;

    if( x1 == x2 )
    {
        refVar->PutBool( sal_True );
    }
}







Reference< XTypeDescriptionEnumeration > getTypeDescriptorEnumeration( const OUString& sSearchRoot,
                                                                       const Sequence< TypeClass >& types,
                                                                       TypeDescriptionSearchDepth depth )
{
    Reference< XTypeDescriptionEnumeration > xEnum;
    Reference< XTypeDescriptionEnumerationAccess> xTypeEnumAccess( getTypeProvider_Impl(), UNO_QUERY );
    if ( xTypeEnumAccess.is() )
    {
        try
        {
            xEnum = xTypeEnumAccess->createTypeDescriptionEnumeration(
                sSearchRoot, types, depth );
        }
        catch(const NoSuchTypeNameException& /*nstne*/ ) {}
        catch(const InvalidTypeNameException& /*nstne*/ ) {}
    }
    return xEnum;
}

typedef boost::unordered_map< OUString, Any, OUStringHash, ::std::equal_to< OUString > > VBAConstantsHash;

VBAConstantHelper&
VBAConstantHelper::instance()
{
    static VBAConstantHelper aHelper;
    return aHelper;
}

void VBAConstantHelper::init()
{
    if ( !isInited )
    {
        Sequence< TypeClass > types(1);
        types[ 0 ] = TypeClass_CONSTANTS;
        Reference< XTypeDescriptionEnumeration > xEnum = getTypeDescriptorEnumeration( OUString(defaultNameSpace), types, TypeDescriptionSearchDepth_INFINITE  );

        if ( !xEnum.is())
        {
            return; 
        }
        while ( xEnum->hasMoreElements() )
        {
            Reference< XConstantsTypeDescription > xConstants( xEnum->nextElement(), UNO_QUERY );
            if ( xConstants.is() )
            {
                
                OUString sFullName = xConstants->getName();
                sal_Int32 indexLastDot = sFullName.lastIndexOf('.');
                OUString sLeafName( sFullName );
                if ( indexLastDot > -1 )
                {
                    sLeafName = sFullName.copy( indexLastDot + 1);
                }
                aConstCache.push_back( sLeafName ); 
                Sequence< Reference< XConstantTypeDescription > > aConsts = xConstants->getConstants();
                for (sal_Int32 i = 0; i != aConsts.getLength(); ++i)
                {
                    
                    sFullName = aConsts[i]->getName();
                    indexLastDot = sFullName.lastIndexOf('.');
                    sLeafName = sFullName;
                    if ( indexLastDot > -1 )
                    {
                        sLeafName = sFullName.copy( indexLastDot + 1);
                    }
                    aConstHash[ sLeafName.toAsciiLowerCase() ] = aConsts[i]->getConstantValue();
                }
            }
        }
        isInited = true;
    }
}

bool
VBAConstantHelper::isVBAConstantType( const OUString& rName )
{
    init();
    bool bConstant = false;
    OUString sKey( rName );
    VBAConstantsVector::const_iterator it = aConstCache.begin();

    for( ; it != aConstCache.end(); ++it )
    {
        if( sKey.equalsIgnoreAsciiCase( *it ) )
        {
            bConstant = true;
            break;
        }
    }
    return bConstant;
}

SbxVariable*
VBAConstantHelper::getVBAConstant( const OUString& rName )
{
    SbxVariable* pConst = NULL;
    init();

    OUString sKey( rName );

    VBAConstantsHash::const_iterator it = aConstHash.find( sKey.toAsciiLowerCase() );

    if ( it != aConstHash.end() )
    {
        pConst = new SbxVariable( SbxVARIANT );
        pConst->SetName( rName );
        unoToSbxValue( pConst, it->second );
    }

    return pConst;
}



SbUnoClass* findUnoClass( const OUString& rName )
{
    
    SbUnoClass* pUnoClass = NULL;

    Reference< XHierarchicalNameAccess > xTypeAccess = getTypeProvider_Impl();
    if( xTypeAccess->hasByHierarchicalName( rName ) )
    {
        Any aRet = xTypeAccess->getByHierarchicalName( rName );
        Reference< XTypeDescription > xTypeDesc;
        aRet >>= xTypeDesc;

        if( xTypeDesc.is() )
        {
            TypeClass eTypeClass = xTypeDesc->getTypeClass();
            if( eTypeClass == TypeClass_MODULE || eTypeClass == TypeClass_CONSTANTS )
            {
                pUnoClass = new SbUnoClass( rName );
            }
        }
    }
    return pUnoClass;
}

SbxVariable* SbUnoClass::Find( const OUString& rName, SbxClassType )
{
    SbxVariable* pRes = SbxObject::Find( rName, SbxCLASS_VARIABLE );

    
    if( !pRes )
    {
        
        if( m_xClass.is() )
        {
            
            OUString aUStr( rName );
            Reference< XIdlField > xField = m_xClass->getField( aUStr );
            Reference< XIdlClass > xClass;
            if( xField.is() )
            {
                try
                {
                    Any aAny;
                    aAny = xField->get( aAny );

                    
                    pRes = new SbxVariable( SbxVARIANT );
                    pRes->SetName( rName );
                    unoToSbxValue( pRes, aAny );
                }
                catch( const Exception& )
                {
                    implHandleAnyException( ::cppu::getCaughtException() );
                }
            }
        }
        else
        {
            
            OUString aNewName = GetName();
            aNewName += ".";
            aNewName += rName;

            
            Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
            if( xCoreReflection.is() )
            {
                
                Reference< XHierarchicalNameAccess > xHarryName( xCoreReflection, UNO_QUERY );
                if( xHarryName.is() )
                {
                    try
                    {
                        Any aValue = xHarryName->getByHierarchicalName( aNewName );
                        TypeClass eType = aValue.getValueType().getTypeClass();

                        
                        if( eType == TypeClass_INTERFACE )
                        {
                            Reference< XInterface > xIface = *(Reference< XInterface >*)aValue.getValue();
                            Reference< XIdlClass > xClass( xIface, UNO_QUERY );
                            if( xClass.is() )
                            {
                                pRes = new SbxVariable( SbxVARIANT );
                                SbxObjectRef xWrapper = (SbxObject*)new SbUnoClass( aNewName, xClass );
                                pRes->PutObject( xWrapper );
                            }
                        }
                        else
                        {
                            pRes = new SbxVariable( SbxVARIANT );
                            unoToSbxValue( pRes, aValue );
                        }
                    }
                    catch( const NoSuchElementException& )
                    {
                    }
                }

                
                if( !pRes )
                {
                    SbUnoClass* pNewClass = findUnoClass( aNewName );
                    if( pNewClass )
                    {
                        pRes = new SbxVariable( SbxVARIANT );
                        SbxObjectRef xWrapper = (SbxObject*)pNewClass;
                        pRes->PutObject( xWrapper );
                    }
                }

                
                if( !pRes )
                {
                    SbUnoService* pUnoService = findUnoService( aNewName );
                    if( pUnoService )
                    {
                        pRes = new SbxVariable( SbxVARIANT );
                        SbxObjectRef xWrapper = (SbxObject*)pUnoService;
                        pRes->PutObject( xWrapper );
                    }
                }

                
                if( !pRes )
                {
                    SbUnoSingleton* pUnoSingleton = findUnoSingleton( aNewName );
                    if( pUnoSingleton )
                    {
                        pRes = new SbxVariable( SbxVARIANT );
                        SbxObjectRef xWrapper = (SbxObject*)pUnoSingleton;
                        pRes->PutObject( xWrapper );
                    }
                }
            }
        }

        if( pRes )
        {
            pRes->SetName( rName );

            
            QuickInsert( pRes );

            
            
            if( pRes->IsBroadcaster() )
                EndListening( pRes->GetBroadcaster(), true );
        }
    }
    return pRes;
}


SbUnoService* findUnoService( const OUString& rName )
{
    SbUnoService* pSbUnoService = NULL;

    Reference< XHierarchicalNameAccess > xTypeAccess = getTypeProvider_Impl();
    if( xTypeAccess->hasByHierarchicalName( rName ) )
    {
        Any aRet = xTypeAccess->getByHierarchicalName( rName );
        Reference< XTypeDescription > xTypeDesc;
        aRet >>= xTypeDesc;

        if( xTypeDesc.is() )
        {
            TypeClass eTypeClass = xTypeDesc->getTypeClass();
            if( eTypeClass == TypeClass_SERVICE )
            {
                Reference< XServiceTypeDescription2 > xServiceTypeDesc( xTypeDesc, UNO_QUERY );
                if( xServiceTypeDesc.is() )
                    pSbUnoService = new SbUnoService( rName, xServiceTypeDesc );
            }
        }
    }
    return pSbUnoService;
}

SbxVariable* SbUnoService::Find( const OUString& rName, SbxClassType )
{
    SbxVariable* pRes = SbxObject::Find( rName, SbxCLASS_METHOD );

    if( !pRes )
    {
        
        if( m_bNeedsInit && m_xServiceTypeDesc.is() )
        {
            m_bNeedsInit = false;

            Sequence< Reference< XServiceConstructorDescription > > aSCDSeq = m_xServiceTypeDesc->getConstructors();
            const Reference< XServiceConstructorDescription >* pCtorSeq = aSCDSeq.getConstArray();
            int nCtorCount = aSCDSeq.getLength();
            for( int i = 0 ; i < nCtorCount ; ++i )
            {
                Reference< XServiceConstructorDescription > xCtor = pCtorSeq[i];

                OUString aName( xCtor->getName() );
                if( aName.isEmpty() )
                {
                    if( xCtor->isDefaultConstructor() )
                    {
                        aName = "create";
                    }
                }

                if( !aName.isEmpty() )
                {
                    
                    SbxVariableRef xSbCtorRef = new SbUnoServiceCtor( aName, xCtor );
                    QuickInsert( (SbxVariable*)xSbCtorRef );
                }
            }
            pRes = SbxObject::Find( rName, SbxCLASS_METHOD );
        }
    }

    return pRes;
}

void SbUnoService::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                           const SfxHint& rHint, const TypeId& rHintType )
{
    const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbxArray* pParams = pVar->GetParameters();
        SbUnoServiceCtor* pUnoCtor = PTR_CAST(SbUnoServiceCtor,pVar);
        if( pUnoCtor && pHint->GetId() == SBX_HINT_DATAWANTED )
        {
            
            sal_uInt32 nParamCount = pParams ? ((sal_uInt32)pParams->Count() - 1) : 0;
            Sequence<Any> args;
            bool bOutParams = false;

            Reference< XServiceConstructorDescription > xCtor = pUnoCtor->getServiceCtorDesc();
            Sequence< Reference< XParameter > > aParameterSeq = xCtor->getParameters();
            const Reference< XParameter >* pParameterSeq = aParameterSeq.getConstArray();
            sal_uInt32 nUnoParamCount = aParameterSeq.getLength();

            
            bool bParameterError = false;

            
            bool bRestParameterMode = false;
            if( nUnoParamCount > 0 )
            {
                Reference< XParameter > xLastParam = pParameterSeq[ nUnoParamCount - 1 ];
                if( xLastParam.is() )
                {
                    if( xLastParam->isRestParameter() )
                        bRestParameterMode = true;
                }
            }

            
            sal_uInt16 nSbxParameterOffset = 1;
            sal_uInt16 nParameterOffsetByContext = 0;
            Reference < XComponentContext > xFirstParamContext;
            if( nParamCount > nUnoParamCount )
            {
                
                
                Any aArg0 = sbxToUnoValue( pParams->Get( nSbxParameterOffset ) );
                if( (aArg0 >>= xFirstParamContext) && xFirstParamContext.is() )
                    nParameterOffsetByContext = 1;
            }

            sal_uInt32 nEffectiveParamCount = nParamCount - nParameterOffsetByContext;
            sal_uInt32 nAllocParamCount = nEffectiveParamCount;
            if( nEffectiveParamCount > nUnoParamCount )
            {
                if( !bRestParameterMode )
                {
                    nEffectiveParamCount = nUnoParamCount;
                    nAllocParamCount = nUnoParamCount;
                }
            }
            
            else if( nUnoParamCount > nEffectiveParamCount )
            {
                
                int nDiff = nUnoParamCount - nEffectiveParamCount;
                if( !bRestParameterMode || nDiff > 1 )
                {
                    bParameterError = true;
                    StarBASIC::Error( SbERR_NOT_OPTIONAL );
                }
            }

            if( !bParameterError )
            {
                if( nAllocParamCount > 0 )
                {
                    args.realloc( nAllocParamCount );
                    Any* pAnyArgs = args.getArray();
                    for( sal_uInt32 i = 0 ; i < nEffectiveParamCount ; i++ )
                    {
                        sal_uInt16 iSbx = (sal_uInt16)(i + nSbxParameterOffset + nParameterOffsetByContext);

                        
                        Reference< XParameter > xParam;
                        if( i < nUnoParamCount )
                        {
                            xParam = pParameterSeq[i];
                            if( !xParam.is() )
                                continue;

                            Reference< XTypeDescription > xParamTypeDesc = xParam->getType();
                            if( !xParamTypeDesc.is() )
                                continue;
                            com::sun::star::uno::Type aType( xParamTypeDesc->getTypeClass(), xParamTypeDesc->getName() );

                            
                            pAnyArgs[i] = sbxToUnoValue( pParams->Get( iSbx ), aType );

                            
                            if( !bOutParams )
                            {
                                if( xParam->isOut() )
                                    bOutParams = true;
                            }
                        }
                        else
                        {
                            pAnyArgs[i] = sbxToUnoValue( pParams->Get( iSbx ) );
                        }
                    }
                }

                
                Reference < XComponentContext > xContext(
                    xFirstParamContext.is()
                    ? xFirstParamContext
                    : comphelper::getProcessComponentContext() );
                Reference< XMultiComponentFactory > xServiceMgr( xContext->getServiceManager() );

                Any aRetAny;
                OUString aServiceName = GetName();
                Reference < XInterface > xRet;
                try
                {
                    xRet = xServiceMgr->createInstanceWithArgumentsAndContext( aServiceName, args, xContext );
                }
                catch( const Exception& )
                {
                    implHandleAnyException( ::cppu::getCaughtException() );
                }
                aRetAny <<= xRet;
                unoToSbxValue( pVar, aRetAny );

                
                if( bOutParams )
                {
                    const Any* pAnyArgs = args.getConstArray();

                    for( sal_uInt32 j = 0 ; j < nUnoParamCount ; j++ )
                    {
                        Reference< XParameter > xParam = pParameterSeq[j];
                        if( !xParam.is() )
                            continue;

                        if( xParam->isOut() )
                            unoToSbxValue( (SbxVariable*)pParams->Get( (sal_uInt16)(j+1) ), pAnyArgs[ j ] );
                    }
                }
            }
        }
        else
            SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
    }
}



static SbUnoServiceCtor* pFirstCtor = NULL;

void clearUnoServiceCtors( void )
{
    SbUnoServiceCtor* pCtor = pFirstCtor;
    while( pCtor )
    {
        pCtor->SbxValue::Clear();
        pCtor = pCtor->pNext;
    }
}

SbUnoServiceCtor::SbUnoServiceCtor( const OUString& aName_, Reference< XServiceConstructorDescription > xServiceCtorDesc )
    : SbxMethod( aName_, SbxOBJECT )
    , m_xServiceCtorDesc( xServiceCtorDesc )
    , pNext(0)
{
}

SbUnoServiceCtor::~SbUnoServiceCtor()
{
}

SbxInfo* SbUnoServiceCtor::GetInfo()
{
    SbxInfo* pRet = NULL;

    return pRet;
}


SbUnoSingleton* findUnoSingleton( const OUString& rName )
{
    SbUnoSingleton* pSbUnoSingleton = NULL;

    Reference< XHierarchicalNameAccess > xTypeAccess = getTypeProvider_Impl();
    if( xTypeAccess->hasByHierarchicalName( rName ) )
    {
        Any aRet = xTypeAccess->getByHierarchicalName( rName );
        Reference< XTypeDescription > xTypeDesc;
        aRet >>= xTypeDesc;

        if( xTypeDesc.is() )
        {
            TypeClass eTypeClass = xTypeDesc->getTypeClass();
            if( eTypeClass == TypeClass_SINGLETON )
            {
                Reference< XSingletonTypeDescription > xSingletonTypeDesc( xTypeDesc, UNO_QUERY );
                if( xSingletonTypeDesc.is() )
                    pSbUnoSingleton = new SbUnoSingleton( rName, xSingletonTypeDesc );
            }
        }
    }
    return pSbUnoSingleton;
}

SbUnoSingleton::SbUnoSingleton( const OUString& aName_,
    const Reference< XSingletonTypeDescription >& xSingletonTypeDesc )
        : SbxObject( aName_ )
        , m_xSingletonTypeDesc( xSingletonTypeDesc )
{
    SbxVariableRef xGetMethodRef = new SbxMethod( OUString( "get"  ), SbxOBJECT );
    QuickInsert( (SbxVariable*)xGetMethodRef );
}

void SbUnoSingleton::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                           const SfxHint& rHint, const TypeId& rHintType )
{
    const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbxArray* pParams = pVar->GetParameters();
        sal_uInt32 nParamCount = pParams ? ((sal_uInt32)pParams->Count() - 1) : 0;
        sal_uInt32 nAllowedParamCount = 1;

        Reference < XComponentContext > xContextToUse;
        if( nParamCount > 0 )
        {
            
            Reference < XComponentContext > xFirstParamContext;
            Any aArg1 = sbxToUnoValue( pParams->Get( 1 ) );
            if( (aArg1 >>= xFirstParamContext) && xFirstParamContext.is() )
                xContextToUse = xFirstParamContext;
        }

        if( !xContextToUse.is() )
        {
            xContextToUse = comphelper::getProcessComponentContext();
            --nAllowedParamCount;
        }

        if( nParamCount > nAllowedParamCount )
        {
            StarBASIC::Error( SbERR_BAD_ARGUMENT );
            return;
        }

        Any aRetAny;
        if( xContextToUse.is() )
        {
            OUString aSingletonName( "/singletons/" );
            aSingletonName += GetName();
            Reference < XInterface > xRet;
            xContextToUse->getValueByName( aSingletonName ) >>= xRet;
            aRetAny <<= xRet;
        }
        unoToSbxValue( pVar, aRetAny );
    }
    else
    {
        SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
    }
}






class BasicAllListener_Impl : public BasicAllListenerHelper
{
    virtual void firing_impl(const AllEventObject& Event, Any* pRet);

public:
    SbxObjectRef    xSbxObj;
    OUString        aPrefixName;

    BasicAllListener_Impl( const OUString& aPrefixName );
    ~BasicAllListener_Impl();

    
    virtual void SAL_CALL firing(const AllEventObject& Event) throw ( RuntimeException );
    virtual Any SAL_CALL approveFiring(const AllEventObject& Event) throw ( RuntimeException );

    
    virtual void SAL_CALL disposing(const EventObject& Source) throw ( RuntimeException );
};



BasicAllListener_Impl::BasicAllListener_Impl(const OUString& aPrefixName_)
    : aPrefixName( aPrefixName_ )
{
}


BasicAllListener_Impl::~BasicAllListener_Impl()
{
}



void BasicAllListener_Impl::firing_impl( const AllEventObject& Event, Any* pRet )
{
    SolarMutexGuard guard;

    if( xSbxObj.Is() )
    {
        OUString aMethodName = aPrefixName;
        aMethodName = aMethodName + Event.MethodName;

        SbxVariable * pP = xSbxObj;
        while( pP->GetParent() )
        {
            pP = pP->GetParent();
            StarBASIC * pLib = PTR_CAST(StarBASIC,pP);
            if( pLib )
            {
                
                SbxArrayRef xSbxArray = new SbxArray( SbxVARIANT );
                const Any * pArgs = Event.Arguments.getConstArray();
                sal_Int32 nCount = Event.Arguments.getLength();
                for( sal_Int32 i = 0; i < nCount; i++ )
                {
                    
                    SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
                    unoToSbxValue( (SbxVariable*)xVar, pArgs[i] );
                    xSbxArray->Put( xVar, sal::static_int_cast< sal_uInt16 >(i+1) );
                }

                pLib->Call( aMethodName, xSbxArray );

                
                if( pRet )
                {
                    SbxVariable* pVar = xSbxArray->Get( 0 );
                    if( pVar )
                    {
                        
                        sal_uInt16 nFlags = pVar->GetFlags();
                        pVar->SetFlag( SBX_NO_BROADCAST );
                        *pRet = sbxToUnoValueImpl( pVar );
                        pVar->SetFlags( nFlags );
                    }
                }
                break;
            }
        }
    }
}



void BasicAllListener_Impl::firing( const AllEventObject& Event ) throw ( RuntimeException )
{
    firing_impl( Event, NULL );
}

Any BasicAllListener_Impl::approveFiring( const AllEventObject& Event ) throw ( RuntimeException )
{
    Any aRetAny;
    firing_impl( Event, &aRetAny );
    return aRetAny;
}



void BasicAllListener_Impl ::disposing(const EventObject& ) throw ( RuntimeException )
{
    SolarMutexGuard guard;

    xSbxObj.Clear();
}







class InvocationToAllListenerMapper : public WeakImplHelper1< XInvocation >
{
public:
    InvocationToAllListenerMapper( const Reference< XIdlClass >& ListenerType,
        const Reference< XAllListener >& AllListener, const Any& Helper );

    
    virtual Reference< XIntrospectionAccess > SAL_CALL getIntrospection(void) throw( RuntimeException );
    virtual Any SAL_CALL invoke(const OUString& FunctionName, const Sequence< Any >& Params, Sequence< sal_Int16 >& OutParamIndex, Sequence< Any >& OutParam)
        throw( IllegalArgumentException, CannotConvertException, InvocationTargetException, RuntimeException );
    virtual void SAL_CALL setValue(const OUString& PropertyName, const Any& Value)
        throw( UnknownPropertyException, CannotConvertException, InvocationTargetException, RuntimeException );
    virtual Any SAL_CALL getValue(const OUString& PropertyName) throw( UnknownPropertyException, RuntimeException );
    virtual sal_Bool SAL_CALL hasMethod(const OUString& Name) throw( RuntimeException );
    virtual sal_Bool SAL_CALL hasProperty(const OUString& Name) throw( RuntimeException );

private:
    Reference< XIdlReflection >  m_xCoreReflection;
    Reference< XAllListener >    m_xAllListener;
    Reference< XIdlClass >       m_xListenerType;
    Any                          m_Helper;
};



Reference< XInterface > createAllListenerAdapter
(
    const Reference< XInvocationAdapterFactory2 >& xInvocationAdapterFactory,
    const Reference< XIdlClass >& xListenerType,
    const Reference< XAllListener >& xListener,
    const Any& Helper
)
{
    Reference< XInterface > xAdapter;
    if( xInvocationAdapterFactory.is() && xListenerType.is() && xListener.is() )
    {
        Reference< XInvocation > xInvocationToAllListenerMapper =
            (XInvocation*)new InvocationToAllListenerMapper( xListenerType, xListener, Helper );
        Type aListenerType( xListenerType->getTypeClass(), xListenerType->getName() );
        Sequence<Type> arg2(1);
        arg2[0] = aListenerType;
        xAdapter = xInvocationAdapterFactory->createAdapter( xInvocationToAllListenerMapper, arg2 );
    }
    return xAdapter;
}




InvocationToAllListenerMapper::InvocationToAllListenerMapper
    ( const Reference< XIdlClass >& ListenerType, const Reference< XAllListener >& AllListener, const Any& Helper )
        : m_xAllListener( AllListener )
        , m_xListenerType( ListenerType )
        , m_Helper( Helper )
{
}


Reference< XIntrospectionAccess > SAL_CALL InvocationToAllListenerMapper::getIntrospection(void)
    throw( RuntimeException )
{
    return Reference< XIntrospectionAccess >();
}


Any SAL_CALL InvocationToAllListenerMapper::invoke(const OUString& FunctionName, const Sequence< Any >& Params,
    Sequence< sal_Int16 >& OutParamIndex, Sequence< Any >& OutParam)
        throw( IllegalArgumentException, CannotConvertException,
        InvocationTargetException, RuntimeException )
{
    (void)OutParamIndex;
    (void)OutParam     ;

    Any aRet;

    
    Reference< XIdlMethod > xMethod = m_xListenerType->getMethod( FunctionName );
    bool bApproveFiring = false;
    if( !xMethod.is() )
        return aRet;
    Reference< XIdlClass > xReturnType = xMethod->getReturnType();
    Sequence< Reference< XIdlClass > > aExceptionSeq = xMethod->getExceptionTypes();
    if( ( xReturnType.is() && xReturnType->getTypeClass() != TypeClass_VOID ) ||
        aExceptionSeq.getLength() > 0 )
    {
        bApproveFiring = true;
    }
    else
    {
        Sequence< ParamInfo > aParamSeq = xMethod->getParameterInfos();
        sal_uInt32 nParamCount = aParamSeq.getLength();
        if( nParamCount > 1 )
        {
            const ParamInfo* pInfos = aParamSeq.getConstArray();
            for( sal_uInt32 i = 0 ; i < nParamCount ; i++ )
            {
                if( pInfos[ i ].aMode != ParamMode_IN )
                {
                    bApproveFiring = true;
                    break;
                }
            }
        }
    }

    AllEventObject aAllEvent;
    aAllEvent.Source = (OWeakObject*) this;
    aAllEvent.Helper = m_Helper;
    aAllEvent.ListenerType = Type(m_xListenerType->getTypeClass(), m_xListenerType->getName() );
    aAllEvent.MethodName = FunctionName;
    aAllEvent.Arguments = Params;
    if( bApproveFiring )
        aRet = m_xAllListener->approveFiring( aAllEvent );
    else
        m_xAllListener->firing( aAllEvent );
    return aRet;
}


void SAL_CALL InvocationToAllListenerMapper::setValue(const OUString& PropertyName, const Any& Value)
    throw( UnknownPropertyException, CannotConvertException,
           InvocationTargetException, RuntimeException )
{
    (void)PropertyName;
    (void)Value;
}


Any SAL_CALL InvocationToAllListenerMapper::getValue(const OUString& PropertyName)
    throw( UnknownPropertyException, RuntimeException )
{
    (void)PropertyName;

    return Any();
}


sal_Bool SAL_CALL InvocationToAllListenerMapper::hasMethod(const OUString& Name)
    throw( RuntimeException )
{
    Reference< XIdlMethod > xMethod = m_xListenerType->getMethod( Name );
    return xMethod.is();
}


sal_Bool SAL_CALL InvocationToAllListenerMapper::hasProperty(const OUString& Name)
    throw( RuntimeException )
{
    Reference< XIdlField > xField = m_xListenerType->getField( Name );
    return xField.is();
}





void SbRtl_CreateUnoListener( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )

{
    (void)bWrite;

    
    if ( rPar.Count() != 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    
    OUString aPrefixName = rPar.Get(1)->GetOUString();
    OUString aListenerClassName = rPar.Get(2)->GetOUString();

    
    Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
    if( !xCoreReflection.is() )
        return;

    
    Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );

    
    Reference< XIdlClass > xClass = xCoreReflection->forName( aListenerClassName );
    if( !xClass.is() )
        return;

    
    Reference< XInvocationAdapterFactory2 > xInvocationAdapterFactory =
         InvocationAdapterFactory::create( xContext );

    BasicAllListener_Impl * p;
    Reference< XAllListener > xAllLst = p = new BasicAllListener_Impl( aPrefixName );
    Any aTmp;
    Reference< XInterface > xLst = createAllListenerAdapter( xInvocationAdapterFactory, xClass, xAllLst, aTmp );
    if( !xLst.is() )
        return;

    OUString aClassName = xClass->getName();
    Type aClassType( xClass->getTypeClass(), aClassName.getStr() );
    aTmp = xLst->queryInterface( aClassType );
    if( !aTmp.hasValue() )
        return;

    SbUnoObject* pUnoObj = new SbUnoObject( aListenerClassName, aTmp );
    p->xSbxObj = pUnoObj;
    p->xSbxObj->SetParent( pBasic );

    
    SbxArrayRef xBasicUnoListeners = pBasic->getUnoListeners();
    xBasicUnoListeners->Insert( pUnoObj, xBasicUnoListeners->Count() );

    
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutObject( p->xSbxObj );
}




void RTL_Impl_GetDefaultContext( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    SbxVariableRef refVar = rPar.Get(0);

    Any aContextAny( comphelper::getProcessComponentContext() );

    SbUnoObjectRef xUnoObj = new SbUnoObject( OUString( "DefaultContext" ), aContextAny );
    refVar->PutObject( (SbUnoObject*)xUnoObj );
}




void RTL_Impl_CreateUnoValue( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    static OUString aTypeTypeString( "type" );

    
    if ( rPar.Count() != 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    
    OUString aTypeName = rPar.Get(1)->GetOUString();
    SbxVariable* pVal = rPar.Get(2);

    if( aTypeName == aTypeTypeString )
    {
        SbxDataType eBaseType = pVal->SbxValue::GetType();
        OUString aValTypeName;
        if( eBaseType == SbxSTRING )
        {
            aValTypeName = pVal->GetOUString();
        }
        else if( eBaseType == SbxOBJECT )
        {
            
            Reference< XIdlClass > xIdlClass;

            SbxBaseRef pObj = (SbxBase*)pVal->GetObject();
            if( pObj && pObj->ISA(SbUnoObject) )
            {
                Any aUnoAny = ((SbUnoObject*)(SbxBase*)pObj)->getUnoAny();
                aUnoAny >>= xIdlClass;
            }

            if( xIdlClass.is() )
            {
                aValTypeName = xIdlClass->getName();
            }
        }
        Type aType;
        bool bSuccess = implGetTypeByName( aValTypeName, aType );
        if( bSuccess )
        {
            Any aTypeAny( aType );
            SbxVariableRef refVar = rPar.Get(0);
            SbxObjectRef xUnoAnyObject = new SbUnoAnyObject( aTypeAny );
            refVar->PutObject( xUnoAnyObject );
        }
        return;
    }

    
    Reference< XHierarchicalNameAccess > xTypeAccess = getTypeProvider_Impl();
    Any aRet;
    try
    {
        aRet = xTypeAccess->getByHierarchicalName( aTypeName );
    }
    catch( const NoSuchElementException& e1 )
    {
        OUString aNoSuchElementExceptionName( "com.sun.star.container.NoSuchElementException"  );
        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
            implGetExceptionMsg( e1, aNoSuchElementExceptionName ) );
        return;
    }
    Reference< XTypeDescription > xTypeDesc;
    aRet >>= xTypeDesc;
    TypeClass eTypeClass = xTypeDesc->getTypeClass();
    Type aDestType( eTypeClass, aTypeName );


    
    Any aVal = sbxToUnoValueImpl( pVal );
    Any aConvertedVal = convertAny( aVal, aDestType );

    SbxVariableRef refVar = rPar.Get(0);
    SbxObjectRef xUnoAnyObject = new SbUnoAnyObject( aConvertedVal );
    refVar->PutObject( xUnoAnyObject );
}



namespace {
class OMutexBasis
{
protected:
    
    ::osl::Mutex m_aMutex;
};
} 

typedef WeakImplHelper2< XInvocation, XComponent > ModuleInvocationProxyHelper;

class ModuleInvocationProxy : public OMutexBasis,
                              public ModuleInvocationProxyHelper
{
    OUString            m_aPrefix;
    SbxObjectRef        m_xScopeObj;
    bool                m_bProxyIsClassModuleObject;

    ::cppu::OInterfaceContainerHelper m_aListeners;

public:
    ModuleInvocationProxy( const OUString& aPrefix, SbxObjectRef xScopeObj );
    ~ModuleInvocationProxy()
    {}

    
    virtual Reference< XIntrospectionAccess > SAL_CALL getIntrospection() throw();
    virtual void SAL_CALL setValue( const OUString& rProperty, const Any& rValue )
        throw( UnknownPropertyException );
    virtual Any SAL_CALL getValue( const OUString& rProperty )
        throw( UnknownPropertyException );
    virtual sal_Bool SAL_CALL hasMethod( const OUString& rName ) throw();
    virtual sal_Bool SAL_CALL hasProperty( const OUString& rProp ) throw();

    virtual Any SAL_CALL invoke( const OUString& rFunction,
                                 const Sequence< Any >& rParams,
                                 Sequence< sal_Int16 >& rOutParamIndex,
                                 Sequence< Any >& rOutParam )
        throw( CannotConvertException, InvocationTargetException );

    
    virtual void SAL_CALL dispose() throw(RuntimeException);
    virtual void SAL_CALL addEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException);
    virtual void SAL_CALL removeEventListener( const Reference< XEventListener >& aListener ) throw (RuntimeException);
};

ModuleInvocationProxy::ModuleInvocationProxy( const OUString& aPrefix, SbxObjectRef xScopeObj )
    : m_aPrefix( aPrefix + OUString( "_" ) )
    , m_xScopeObj( xScopeObj )
    , m_aListeners( m_aMutex )
{
    m_bProxyIsClassModuleObject = xScopeObj.Is() ? xScopeObj->ISA(SbClassModuleObject) : false;
}

Reference< XIntrospectionAccess > SAL_CALL ModuleInvocationProxy::getIntrospection() throw()
{
    return Reference< XIntrospectionAccess >();
}

void SAL_CALL ModuleInvocationProxy::setValue( const OUString& rProperty, const Any& rValue ) throw( UnknownPropertyException )
{
    if( !m_bProxyIsClassModuleObject )
        throw UnknownPropertyException();

    SolarMutexGuard guard;

    OUString aPropertyFunctionName( "Property Set " );
    aPropertyFunctionName += m_aPrefix;
    aPropertyFunctionName += rProperty;

    SbxVariable* p = m_xScopeObj->Find( aPropertyFunctionName, SbxCLASS_METHOD );
    SbMethod* pMeth = p != NULL ? PTR_CAST(SbMethod,p) : NULL;
    if( pMeth == NULL )
    {
        
        
        throw UnknownPropertyException();
    }

    
    SbxArrayRef xArray = new SbxArray;
    SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
    unoToSbxValue( (SbxVariable*)xVar, rValue );
    xArray->Put( xVar, 1 );

    
    SbxVariableRef xValue = new SbxVariable;
    pMeth->SetParameters( xArray );
    pMeth->Call( xValue );
    pMeth->SetParameters( NULL );

    



}

Any SAL_CALL ModuleInvocationProxy::getValue( const OUString& rProperty ) throw( UnknownPropertyException )
{
    if( !m_bProxyIsClassModuleObject )
    {
        throw UnknownPropertyException();
    }
    SolarMutexGuard guard;

    OUString aPropertyFunctionName( "Property Get " );
    aPropertyFunctionName += m_aPrefix;
    aPropertyFunctionName += rProperty;

    SbxVariable* p = m_xScopeObj->Find( aPropertyFunctionName, SbxCLASS_METHOD );
    SbMethod* pMeth = p != NULL ? PTR_CAST(SbMethod,p) : NULL;
    if( pMeth == NULL )
    {
        
        
        throw UnknownPropertyException();
    }

    
    SbxVariableRef xValue = new SbxVariable;
    pMeth->Call( xValue );
    Any aRet = sbxToUnoValue( xValue );
    return aRet;
}

sal_Bool SAL_CALL ModuleInvocationProxy::hasMethod( const OUString& ) throw()
{
    return sal_False;
}

sal_Bool SAL_CALL ModuleInvocationProxy::hasProperty( const OUString& ) throw()
{
    return sal_False;
}

Any SAL_CALL ModuleInvocationProxy::invoke( const OUString& rFunction,
                                            const Sequence< Any >& rParams,
                                            Sequence< sal_Int16 >&,
                                            Sequence< Any >& )
    throw( CannotConvertException, InvocationTargetException )
{
    SolarMutexGuard guard;

    Any aRet;
    SbxObjectRef xScopeObj = m_xScopeObj;
    if( !xScopeObj.Is() )
    {
        return aRet;
    }
    OUString aFunctionName = m_aPrefix;
    aFunctionName += rFunction;

    bool bSetRescheduleBack = false;
    sal_Bool bOldReschedule = sal_True;
    SbiInstance* pInst = GetSbData()->pInst;
    if( pInst && pInst->IsCompatibility() )
    {
        bOldReschedule = pInst->IsReschedule();
        if ( bOldReschedule )
        {
            pInst->EnableReschedule( sal_False );
            bSetRescheduleBack = true;
        }
    }

    SbxVariable* p = xScopeObj->Find( aFunctionName, SbxCLASS_METHOD );
    SbMethod* pMeth = p != NULL ? PTR_CAST(SbMethod,p) : NULL;
    if( pMeth == NULL )
    {
        
        
        return aRet;
    }

    
    SbxArrayRef xArray;
    sal_Int32 nParamCount = rParams.getLength();
    if( nParamCount )
    {
        xArray = new SbxArray;
        const Any *pArgs = rParams.getConstArray();
        for( sal_Int32 i = 0 ; i < nParamCount ; i++ )
        {
            SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
            unoToSbxValue( (SbxVariable*)xVar, pArgs[i] );
            xArray->Put( xVar, sal::static_int_cast< sal_uInt16 >(i+1) );
        }
    }

    
    SbxVariableRef xValue = new SbxVariable;
    if( xArray.Is() )
        pMeth->SetParameters( xArray );
    pMeth->Call( xValue );
    aRet = sbxToUnoValue( xValue );
    pMeth->SetParameters( NULL );

    if( bSetRescheduleBack )
        pInst->EnableReschedule( bOldReschedule );

    

    return aRet;
}

void SAL_CALL ModuleInvocationProxy::dispose()
    throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    EventObject aEvent( (XComponent*)this );
    m_aListeners.disposeAndClear( aEvent );

    m_xScopeObj = NULL;
}

void SAL_CALL ModuleInvocationProxy::addEventListener( const Reference< XEventListener >& xListener )
    throw (RuntimeException)
{
    m_aListeners.addInterface( xListener );
}

void SAL_CALL ModuleInvocationProxy::removeEventListener( const Reference< XEventListener >& xListener )
    throw (RuntimeException)
{
    m_aListeners.removeInterface( xListener );
}


Reference< XInterface > createComListener( const Any& aControlAny, const OUString& aVBAType,
                                           const OUString& aPrefix, SbxObjectRef xScopeObj )
{
    Reference< XInterface > xRet;

    Reference< XComponentContext > xContext(
        comphelper::getProcessComponentContext() );
    Reference< XMultiComponentFactory > xServiceMgr( xContext->getServiceManager() );

    Reference< XInvocation > xProxy = new ModuleInvocationProxy( aPrefix, xScopeObj );

    Sequence<Any> args( 3 );
    args[0] <<= aControlAny;
    args[1] <<= aVBAType;
    args[2] <<= xProxy;

    try
    {
        xRet = xServiceMgr->createInstanceWithArgumentsAndContext(
            OUString( "com.sun.star.custom.UnoComListener"),
            args, xContext );
    }
    catch( const Exception& )
    {
        implHandleAnyException( ::cppu::getCaughtException() );
    }

    return xRet;
}

typedef std::vector< WeakReference< XComponent > >  ComponentRefVector;

struct StarBasicDisposeItem
{
    StarBASIC*              m_pBasic;
    SbxArrayRef             m_pRegisteredVariables;
    ComponentRefVector      m_vComImplementsObjects;

    StarBasicDisposeItem( StarBASIC* pBasic )
        : m_pBasic( pBasic )
    {
        m_pRegisteredVariables = new SbxArray();
    }
};

typedef std::vector< StarBasicDisposeItem* > DisposeItemVector;

static DisposeItemVector GaDisposeItemVector;

static DisposeItemVector::iterator lcl_findItemForBasic( StarBASIC* pBasic )
{
    DisposeItemVector::iterator it;
    for( it = GaDisposeItemVector.begin() ; it != GaDisposeItemVector.end() ; ++it )
    {
        StarBasicDisposeItem* pItem = *it;
        if( pItem->m_pBasic == pBasic )
            return it;
    }
    return GaDisposeItemVector.end();
}

static StarBasicDisposeItem* lcl_getOrCreateItemForBasic( StarBASIC* pBasic )
{
    DisposeItemVector::iterator it = lcl_findItemForBasic( pBasic );
    StarBasicDisposeItem* pItem = (it != GaDisposeItemVector.end()) ? *it : NULL;
    if( pItem == NULL )
    {
        pItem = new StarBasicDisposeItem( pBasic );
        GaDisposeItemVector.push_back( pItem );
    }
    return pItem;
}

void registerComponentToBeDisposedForBasic
    ( Reference< XComponent > xComponent, StarBASIC* pBasic )
{
    StarBasicDisposeItem* pItem = lcl_getOrCreateItemForBasic( pBasic );
    pItem->m_vComImplementsObjects.push_back( xComponent );
}

void registerComListenerVariableForBasic( SbxVariable* pVar, StarBASIC* pBasic )
{
    StarBasicDisposeItem* pItem = lcl_getOrCreateItemForBasic( pBasic );
    SbxArray* pArray = pItem->m_pRegisteredVariables;
    pArray->Put( pVar, pArray->Count() );
}

void disposeComVariablesForBasic( StarBASIC* pBasic )
{
    DisposeItemVector::iterator it = lcl_findItemForBasic( pBasic );
    if( it != GaDisposeItemVector.end() )
    {
        StarBasicDisposeItem* pItem = *it;

        SbxArray* pArray = pItem->m_pRegisteredVariables;
        sal_uInt16 nCount = pArray->Count();
        for( sal_uInt16 i = 0 ; i < nCount ; ++i )
        {
            SbxVariable* pVar = pArray->Get( i );
            pVar->ClearComListener();
        }

        ComponentRefVector& rv = pItem->m_vComImplementsObjects;
        ComponentRefVector::iterator itCRV;
        for( itCRV = rv.begin() ; itCRV != rv.end() ; ++itCRV )
        {
            try
            {
                Reference< XComponent > xComponent( (*itCRV).get(), UNO_QUERY_THROW );
                xComponent->dispose();
            }
            catch(const Exception& )
            {}
        }

        delete pItem;
        GaDisposeItemVector.erase( it );
    }
}



bool SbModule::createCOMWrapperForIface( Any& o_rRetAny, SbClassModuleObject* pProxyClassModuleObject )
{
    
    

    Reference< XComponentContext > xContext(
        comphelper::getProcessComponentContext() );
    Reference< XMultiComponentFactory > xServiceMgr( xContext->getServiceManager() );
    Reference< XSingleServiceFactory > xComImplementsFactory
    (
        xServiceMgr->createInstanceWithContext(
            OUString( "com.sun.star.custom.ComImplementsFactory"), xContext ),
        UNO_QUERY
    );
    if( !xComImplementsFactory.is() )
        return false;

    bool bSuccess = false;

    SbxArray* pModIfaces = pClassData->mxIfaces;
    sal_uInt16 nCount = pModIfaces->Count();
    for( sal_uInt16 i = 0 ; i < nCount ; ++i )
    {
        SbxVariable* pVar = pModIfaces->Get( i );
        OUString aIfaceName = pVar->GetName();

        if( !aIfaceName.isEmpty() )
        {
            OUString aPureIfaceName = aIfaceName;
            sal_Int32 indexLastDot = aIfaceName.lastIndexOf('.');
            if ( indexLastDot > -1 )
            {
                aPureIfaceName = aIfaceName.copy( indexLastDot + 1 );
            }
            Reference< XInvocation > xProxy = new ModuleInvocationProxy( aPureIfaceName, pProxyClassModuleObject );

            Sequence<Any> args( 2 );
            args[0] <<= aIfaceName;
            args[1] <<= xProxy;

            Reference< XInterface > xRet;
            bSuccess = false;
            try
            {
                xRet = xComImplementsFactory->createInstanceWithArguments( args );
                bSuccess = true;
            }
            catch( const Exception& )
            {
                implHandleAnyException( ::cppu::getCaughtException() );
            }

            if( bSuccess )
            {
                Reference< XComponent > xComponent( xProxy, UNO_QUERY );
                if( xComponent.is() )
                {
                    StarBASIC* pParentBasic = NULL;
                    SbxObject* pCurObject = this;
                    do
                    {
                        SbxObject* pObjParent = pCurObject->GetParent();
                        pParentBasic = PTR_CAST( StarBASIC, pObjParent );
                        pCurObject = pObjParent;
                    }
                    while( pParentBasic == NULL && pCurObject != NULL );

                    OSL_ASSERT( pParentBasic != NULL );
                    registerComponentToBeDisposedForBasic( xComponent, pParentBasic );
                }

                o_rRetAny <<= xRet;
                break;
            }
        }
     }

    return bSuccess;
}





bool handleToStringForCOMObjects( SbxObject* pObj, SbxValue* pVal )
{
    bool bSuccess = false;

    SbUnoObject* pUnoObj = NULL;
    if( pObj != NULL && (pUnoObj = PTR_CAST(SbUnoObject,(SbxObject*)pObj)) != NULL )
    {
        
        if( pUnoObj->isNativeCOMObject() )
        {
            SbxVariableRef pMeth = pObj->Find( OUString( "toString"  ), SbxCLASS_METHOD );
            if ( pMeth.Is() )
            {
                SbxValues aRes;
                pMeth->Get( aRes );
                pVal->Put( aRes );
                bSuccess = true;
            }
        }
    }
    return bSuccess;
}

Any StructRefInfo::getValue()
{
    Any aRet;
    uno_any_destruct(
        &aRet, reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
    uno_any_construct(
        &aRet, getInst(), mpTD,
                reinterpret_cast< uno_AcquireFunc >(cpp_acquire) );
    return aRet;
}

void StructRefInfo::setValue( const Any& rValue )
{
    uno_type_assignData( getInst(),
        mpTD->pWeakRef,
       (void*)rValue.getValue(),
       rValue.getValueTypeRef(),
       reinterpret_cast< uno_QueryInterfaceFunc >(cpp_queryInterface),
       reinterpret_cast< uno_AcquireFunc >(cpp_acquire),
       reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
}

OUString StructRefInfo::getTypeName() const
{
    OUString sTypeName;
    if ( mpTD )
    {
        sTypeName = mpTD->pTypeName;
    }
    return sTypeName;
}

void* StructRefInfo::getInst()
{
    return ((char*)maAny.getValue() + mnPos );
}

TypeClass StructRefInfo::getTypeClass() const
{
    TypeClass t = TypeClass_VOID;
    if ( mpTD )
        t =  (TypeClass)mpTD->eTypeClass;
    return t;
}

SbUnoStructRefObject::SbUnoStructRefObject( const OUString& aName_, const StructRefInfo& rMemberInfo ) :  SbxObject( aName_ ), maMemberInfo( rMemberInfo ), mbMemberCacheInit( false )
{
   SetClassName( OUString( maMemberInfo.getTypeName() ) );
}

SbUnoStructRefObject::~SbUnoStructRefObject()
{
    for ( StructFieldInfo::iterator it = maFields.begin(), it_end = maFields.end(); it != it_end; ++it )
        delete it->second;
}

void SbUnoStructRefObject::initMemberCache()
{
    if ( mbMemberCacheInit )
        return;
    sal_Int32 nAll = 0;
    typelib_TypeDescription * pTD = maMemberInfo.getTD();
    typelib_CompoundTypeDescription * pCompTypeDescr = (typelib_CompoundTypeDescription *)pTD;
    for ( ; pCompTypeDescr; pCompTypeDescr = pCompTypeDescr->pBaseTypeDescription )
        nAll += pCompTypeDescr->nMembers;
    for ( pCompTypeDescr = (typelib_CompoundTypeDescription *)pTD; pCompTypeDescr;
        pCompTypeDescr = pCompTypeDescr->pBaseTypeDescription )
    {
        typelib_TypeDescriptionReference ** ppTypeRefs = pCompTypeDescr->ppTypeRefs;
        rtl_uString ** ppNames                         = pCompTypeDescr->ppMemberNames;
        sal_Int32 * pMemberOffsets                     = pCompTypeDescr->pMemberOffsets;
        for ( sal_Int32 nPos = pCompTypeDescr->nMembers; nPos--; )
        {
            typelib_TypeDescription * pMemberTD = 0;
            TYPELIB_DANGER_GET( &pMemberTD, ppTypeRefs[nPos] );
            OSL_ENSURE( pMemberTD, "### cannot get field in struct!" );
            if (pMemberTD)
            {
                OUString aName( ppNames[nPos] );
                TYPELIB_DANGER_RELEASE( pMemberTD );
                maFields[ aName ] = new StructRefInfo( maMemberInfo.getRootAnyRef(), pMemberTD, maMemberInfo.getPos() + pMemberOffsets[nPos] );
            }
        }
    }
    mbMemberCacheInit = true;
}

SbxVariable* SbUnoStructRefObject::Find( const OUString& rName, SbxClassType t )
{
    SbxVariable* pRes = SbxObject::Find( rName, t );
    if ( !pRes )
    {
        if ( !mbMemberCacheInit )
            initMemberCache();
        StructFieldInfo::iterator it = maFields.find( OUString( rName ).toAsciiUpperCase() );
        if ( it != maFields.end() )
        {
            SbxDataType eSbxType;
            eSbxType = unoToSbxType( it->second->getTypeClass() );
            SbxDataType eRealSbxType = eSbxType;
            Property aProp;
            aProp.Name = rName;
            aProp.Type = com::sun::star::uno::Type( it->second->getTypeClass(), it->second->getTypeName() );
            SbUnoProperty* pProp = new SbUnoProperty( rName, eSbxType, eRealSbxType, aProp, 0, false, ( aProp.Type.getTypeClass() == com::sun::star::uno::TypeClass_STRUCT) );
            SbxVariableRef xVarRef = pProp;
            QuickInsert( (SbxVariable*)xVarRef );
            pRes = xVarRef;
        }
    }

    if( !pRes )
    {
        if( rName.equalsIgnoreAsciiCase(ID_DBG_SUPPORTEDINTERFACES) ||
            rName.equalsIgnoreAsciiCase(ID_DBG_PROPERTIES) ||
            rName.equalsIgnoreAsciiCase(ID_DBG_METHODS) )
        {
            
            implCreateDbgProperties();

            
            pRes = SbxObject::Find( rName, SbxCLASS_DONTCARE );
        }
    }

    return pRes;
}


void SbUnoStructRefObject::implCreateDbgProperties( void )
{
    Property aProp;

    
    SbxVariableRef xVarRef = new SbUnoProperty( OUString(ID_DBG_SUPPORTEDINTERFACES), SbxSTRING, SbxSTRING, aProp, -1, false, false );
    QuickInsert( (SbxVariable*)xVarRef );

    
    xVarRef = new SbUnoProperty( OUString(ID_DBG_PROPERTIES), SbxSTRING, SbxSTRING, aProp, -2, false, false );
    QuickInsert( (SbxVariable*)xVarRef );

    
    xVarRef = new SbUnoProperty( OUString(ID_DBG_METHODS), SbxSTRING, SbxSTRING, aProp, -3, false, false );
    QuickInsert( (SbxVariable*)xVarRef );
}

void SbUnoStructRefObject::implCreateAll()
{
     
    pMethods   = new SbxArray;
    pProps     = new SbxArray;

    if (!mbMemberCacheInit)
        initMemberCache();

    for ( StructFieldInfo::iterator it = maFields.begin(), it_end = maFields.end(); it != it_end; ++it )
    {
        const OUString& rName = it->first;
        SbxDataType eSbxType;
        eSbxType = unoToSbxType( it->second->getTypeClass() );
        SbxDataType eRealSbxType = eSbxType;
        Property aProp;
        aProp.Name = rName;
        aProp.Type = com::sun::star::uno::Type( it->second->getTypeClass(), it->second->getTypeName() );
        SbUnoProperty* pProp = new SbUnoProperty( rName, eSbxType, eRealSbxType, aProp, 0, false, ( aProp.Type.getTypeClass() == com::sun::star::uno::TypeClass_STRUCT) );
        SbxVariableRef xVarRef = pProp;
        QuickInsert( (SbxVariable*)xVarRef );
    }

    
    implCreateDbgProperties();
}

 
Any SbUnoStructRefObject::getUnoAny( void )
{
    return maMemberInfo.getValue();
}

OUString SbUnoStructRefObject::Impl_DumpProperties()
{
    OUStringBuffer aRet;
    aRet.appendAscii("Properties of object ");
    aRet.append( getDbgObjectName() );

    sal_uInt16 nPropCount = pProps->Count();
    sal_uInt16 nPropsPerLine = 1 + nPropCount / 30;
    for( sal_uInt16 i = 0; i < nPropCount; i++ )
    {
        SbxVariable* pVar = pProps->Get( i );
        if( pVar )
        {
            OUStringBuffer aPropStr;
            if( (i % nPropsPerLine) == 0 )
            {
                aPropStr.appendAscii( "\n" );
            }
            
            
            SbxDataType eType = pVar->GetFullType();

            bool bMaybeVoid = false;
            OUString aName( pVar->GetName() );
            StructFieldInfo::iterator it = maFields.find( aName );

            if ( it != maFields.end() )
            {
                const StructRefInfo& rPropInfo = *it->second;

                if( eType == SbxOBJECT )
                {
                    if( rPropInfo.getTypeClass() == TypeClass_SEQUENCE )
                    {
                        eType = (SbxDataType) ( SbxOBJECT | SbxARRAY );
                    }
                }
            }
            aPropStr.append( Dbg_SbxDataType2String( eType ) );
            if( bMaybeVoid )
            {
                aPropStr.appendAscii( "/void" );
            }
            aPropStr.appendAscii( " " );
            aPropStr.append( pVar->GetName() );

            if( i == nPropCount - 1 )
            {
                aPropStr.appendAscii( "\n" );
            }
            else
            {
                aPropStr.appendAscii( "; " );
            }
            aRet.append( aPropStr.makeStringAndClear() );
        }
    }
    return aRet.makeStringAndClear();
}

void SbUnoStructRefObject::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                           const SfxHint& rHint, const TypeId& rHintType )
{
    if ( !mbMemberCacheInit )
        initMemberCache();
    const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbUnoProperty* pProp = PTR_CAST(SbUnoProperty,pVar);
        if( pProp )
        {
             StructFieldInfo::iterator it =  maFields.find(  pProp->GetName() );
            
            if( pHint->GetId() == SBX_HINT_DATAWANTED )
            {
                
                sal_Int32 nId = pProp->nId;
                if( nId < 0 )
                {
                    
                    if( nId == -1 )     
                    {
                        OUStringBuffer aRet;
                        aRet.appendAscii( ID_DBG_SUPPORTEDINTERFACES );
                        aRet.appendAscii( " not available.\n(TypeClass is not TypeClass_INTERFACE)\n" );

                        pVar->PutString( aRet.makeStringAndClear() );
                    }
                    
                    else if( nId == -2 )        
                    {
                        
                        implCreateAll();
                        OUString aRetStr = Impl_DumpProperties();
                        pVar->PutString( aRetStr );
                    }
                    
                    else if( nId == -3 )        
                    {
                        
                        implCreateAll();
                        OUStringBuffer aRet;
                        aRet.appendAscii("Methods of object ");
                        aRet.append( getDbgObjectName() );
                        aRet.appendAscii( "\nNo methods found\n" );
                        pVar->PutString( aRet.makeStringAndClear() );
                    }
                    return;
                }

                if ( it != maFields.end() )
                {
                    Any aRetAny = it->second->getValue();
                    unoToSbxValue( pVar, aRetAny );
                }
                else
                    StarBASIC::Error( SbERR_PROPERTY_NOT_FOUND );
            }
            else if( pHint->GetId() == SBX_HINT_DATACHANGED )
            {
                if ( it != maFields.end() )
                {
                    
                    Any aAnyValue = sbxToUnoValue( pVar, pProp->aUnoProp.Type, &pProp->aUnoProp );
                    it->second->setValue( aAnyValue );
                }
                else
                    StarBASIC::Error( SbERR_PROPERTY_NOT_FOUND );
            }
        }
        else
           SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
    }
}

StructRefInfo SbUnoStructRefObject::getStructMember( const OUString& rMemberName )
{
    if (!mbMemberCacheInit)
    {
        initMemberCache();
    }
    StructFieldInfo::iterator it = maFields.find( rMemberName );

    typelib_TypeDescription * pFoundTD = NULL;
    sal_Int32 nFoundPos = -1;

    if ( it != maFields.end() )
    {
        pFoundTD = it->second->getTD();
        nFoundPos = it->second->getPos();
    }
    StructRefInfo aRet( maMemberInfo.getRootAnyRef(), pFoundTD, nFoundPos );
    return aRet;
}

OUString SbUnoStructRefObject::getDbgObjectName()
{
    OUString aName = GetClassName();
    if( aName.isEmpty() )
    {
        aName += "Unknown";
    }
    OUStringBuffer aRet;
    if( aName.getLength() > 20 )
    {
        aRet.appendAscii( "\n" );
    }
    aRet.appendAscii( "\"" );
    aRet.append( aName );
    aRet.appendAscii( "\":" );
    return aRet.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
