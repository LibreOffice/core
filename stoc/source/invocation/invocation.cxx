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

#include <osl/mutex.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/uno/DeploymentException.hpp>
#include <com/sun/star/script/FailReason.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/script/XInvocation2.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/beans/Introspection.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/beans/PropertyConcept.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include <boost/scoped_array.hpp>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

#define SERVICENAME "com.sun.star.script.Invocation"
#define IMPLNAME     "com.sun.star.comp.stoc.Invocation"

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::script;
using namespace com::sun::star::reflection;
using namespace com::sun::star::beans;
using namespace com::sun::star::registry;
using namespace com::sun::star::container;
using namespace cppu;
using namespace osl;
using ::rtl::OUString;

namespace stoc_inv
{
static rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;

static Sequence< OUString > inv_getSupportedServiceNames()
{
    Sequence< OUString > seqNames(1);
    seqNames.getArray()[0] = OUString(RTL_CONSTASCII_USTRINGPARAM(SERVICENAME));
    return seqNames;
}

static OUString inv_getImplementationName()
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(IMPLNAME));
}

// TODO: Zentral implementieren
inline Reference<XIdlClass> TypeToIdlClass( const Type& rType, const Reference< XIdlReflection > & xRefl )
{
    return xRefl->forName( rType.getTypeName() );
}


//==================================================================================================
class Invocation_Impl
    : public OWeakObject
    , public XInvocation2
    , public XNameContainer
    , public XIndexContainer
    , public XEnumerationAccess
    , public XExactName
    , public XMaterialHolder
    , public XTypeProvider
{
public:
    Invocation_Impl( const Any & rAdapted, const Reference<XTypeConverter> &,
                                           const Reference<XIntrospection> &,
                                           const Reference<XIdlReflection> & );
    virtual ~Invocation_Impl();

    // XInterface
    virtual Any         SAL_CALL queryInterface( const Type & aType) throw( RuntimeException );
    virtual void        SAL_CALL acquire() throw() { OWeakObject::acquire(); }
    virtual void        SAL_CALL release() throw() { OWeakObject::release(); }


    // XTypeProvider
    virtual Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  )
       throw(RuntimeException);
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId(  )
       throw( RuntimeException);

    // Methoden von XMaterialHolder
    virtual Any         SAL_CALL getMaterial(void) throw(RuntimeException);

    // ? XTool
    virtual void        SAL_CALL setMaterial( const Any& rMaterial );

    // XInvocation
    virtual Reference<XIntrospectionAccess> SAL_CALL getIntrospection(void) throw( RuntimeException );
    virtual Any SAL_CALL invoke(const OUString& FunctionName, const Sequence< Any >& Params, Sequence< sal_Int16 >& OutParamIndex, Sequence< Any >& OutParam)
        throw( IllegalArgumentException, CannotConvertException, InvocationTargetException, RuntimeException );
    virtual void SAL_CALL setValue(const OUString& PropertyName, const Any& Value)
        throw( UnknownPropertyException, CannotConvertException, InvocationTargetException, RuntimeException );
    virtual Any SAL_CALL getValue(const OUString& PropertyName)
        throw( UnknownPropertyException, RuntimeException );
    virtual sal_Bool SAL_CALL hasMethod(const OUString& Name) throw( RuntimeException );
    virtual sal_Bool SAL_CALL hasProperty(const OUString& Name) throw( RuntimeException );

    // XInvocation2
    virtual Sequence< OUString > SAL_CALL getMemberNames(  )
        throw( RuntimeException );
    virtual Sequence< InvocationInfo > SAL_CALL getInfo(  )
        throw( RuntimeException );
    virtual InvocationInfo SAL_CALL getInfoForName( const OUString& aName, sal_Bool bExact )
        throw( IllegalArgumentException, RuntimeException );

    // All Access and Container methods are not thread save
    // XElementAccess
    virtual Type SAL_CALL getElementType(void) throw( RuntimeException )
        { return _xElementAccess->getElementType(); }

    virtual sal_Bool SAL_CALL hasElements(void) throw( RuntimeException )
        { return _xElementAccess->hasElements(); }

    // XNameContainer
    virtual void SAL_CALL insertByName( const OUString& Name, const Any& Element )
        throw( IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException )
        { _xNameContainer->insertByName( Name, Element ); }

    virtual void SAL_CALL replaceByName( const OUString& Name, const Any& Element )
        throw( IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException )
        { _xNameContainer->replaceByName( Name, Element ); }

    virtual void SAL_CALL removeByName( const OUString& Name )
        throw( NoSuchElementException, WrappedTargetException, RuntimeException )
        { _xNameContainer->removeByName( Name ); }

    // XNameAccess
    virtual Any SAL_CALL getByName( const OUString& Name )
        throw( NoSuchElementException, WrappedTargetException, RuntimeException )
        { return _xNameAccess->getByName( Name ); }

    virtual Sequence<OUString> SAL_CALL getElementNames(void) throw( RuntimeException )
        { return _xNameAccess->getElementNames(); }

    virtual sal_Bool SAL_CALL hasByName( const OUString& Name ) throw( RuntimeException )
        { return _xNameAccess->hasByName( Name ); }

    // XIndexContainer
    virtual void SAL_CALL insertByIndex( sal_Int32 Index, const Any& Element )
        throw( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
        { _xIndexContainer->insertByIndex( Index, Element ); }

    virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const Any& Element )
        throw( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
        { _xIndexContainer->replaceByIndex( Index, Element ); }

    virtual void SAL_CALL removeByIndex( sal_Int32 Index )
        throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
        { _xIndexContainer->removeByIndex( Index ); }

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(void) throw( RuntimeException )
        { return _xIndexAccess->getCount(); }

    virtual Any SAL_CALL getByIndex( sal_Int32 Index )
        throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
        { return _xIndexAccess->getByIndex( Index ); }

    // XEnumerationAccess
    virtual Reference<XEnumeration> SAL_CALL createEnumeration(void) throw( RuntimeException )
        { return _xEnumerationAccess->createEnumeration(); }

    // XExactName
    virtual OUString SAL_CALL getExactName( const OUString& rApproximateName ) throw( RuntimeException );


    //=====================================================================================================
private:
    void getInfoSequenceImpl( Sequence< OUString >* pStringSeq, Sequence< InvocationInfo >* pInfoSeq );
    void fillInfoForNameAccess( InvocationInfo& rInfo, const OUString& aName );
    void fillInfoForProperty( InvocationInfo& rInfo, const Property& rProp );
    void fillInfoForMethod( InvocationInfo& rInfo, const Reference< XIdlMethod > xMethod );

    Reference<XTypeConverter>           xTypeConverter;
    Reference<XIntrospection>           xIntrospection;
    Reference<XIdlReflection>           xCoreReflection;

    Any                             _aMaterial;
    // _xDirect and (_xIntrospectionAccess, xPropertySet) are exclusive
    Reference<XInvocation>              _xDirect;
    Reference<XInvocation2>             _xDirect2;
    Reference<XPropertySet>             _xPropertySet;
    Reference<XIntrospectionAccess>     _xIntrospectionAccess;

    // supplied Interfaces
    Reference<XNameContainer>           _xNameContainer;
    Reference<XNameAccess>              _xNameAccess;
    Reference<XIndexContainer>          _xIndexContainer;
    Reference<XIndexAccess>             _xIndexAccess;
    Reference<XEnumerationAccess>       _xEnumerationAccess;
    Reference<XElementAccess>           _xElementAccess;

    //
    Reference<XExactName>               _xENDirect, _xENIntrospection, _xENNameAccess;
};


//==================================================================================================
//==================================================================================================
//==================================================================================================

//--------------------------------------------------------------------------------------------------
Invocation_Impl::Invocation_Impl
(
    const Any & rAdapted,
    const Reference<XTypeConverter> & rTC,
    const Reference<XIntrospection> & rI,
    const Reference<XIdlReflection> & rCR
)
    : xTypeConverter( rTC )
    , xIntrospection( rI )
    , xCoreReflection( rCR )
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    setMaterial( rAdapted );
}

Invocation_Impl::~Invocation_Impl()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

//##################################################################################################
//### INTERFACE IMPLEMENTATIONS ####################################################################
//##################################################################################################


Any SAL_CALL Invocation_Impl::queryInterface( const Type & aType )
    throw( RuntimeException )
{
    // PropertySet-Implementation
    Any a = ::cppu::queryInterface( aType,
                                   (static_cast< XInvocation* >(this)),
                                   (static_cast< XMaterialHolder* >(this)),
                                   (static_cast< XTypeProvider * >(this))    );
    if( a.hasValue() )
    {
        return a;
    }

    if( aType  == getCppuType( (Reference<XExactName>*) NULL ) )
    {
        // Ivocation does not support XExactName, if direct object supports
        // XInvocation, but not XExactName.
        if ((_xDirect.is() && _xENDirect.is()) ||
            (!_xDirect.is() && (_xENIntrospection.is() || _xENNameAccess.is())))
        {
            return makeAny( Reference< XExactName >( (static_cast< XExactName* >(this)) ) );
        }
    }
    else if ( aType == getCppuType( (Reference<XNameContainer>*) NULL ) )
    {
        if( _xNameContainer.is() )
            return makeAny( Reference< XNameContainer >( (static_cast< XNameContainer* >(this)) ) );
    }
    else if ( aType == getCppuType( (Reference<XNameAccess>*) NULL ) )
    {
        if( _xNameAccess.is() )
            return makeAny( Reference< XNameAccess >( (static_cast< XNameAccess* >(this)) ) );
    }
    else if ( aType == getCppuType( (Reference<XIndexContainer>*) NULL ) )
    {
        if (_xIndexContainer.is())
            return makeAny( Reference< XIndexContainer >( (static_cast< XIndexContainer* >(this)) ) );
    }
    else if ( aType == getCppuType( (Reference<XIndexAccess>*) NULL ) )
    {
        if (_xIndexAccess.is())
            return makeAny( Reference< XIndexAccess >( (static_cast< XIndexAccess* >(this)) ) );
    }
    else if ( aType == getCppuType( (Reference<XEnumerationAccess>*) NULL ) )
    {
        if (_xEnumerationAccess.is())
            return makeAny( Reference< XEnumerationAccess >( (static_cast< XEnumerationAccess* >(this)) ) );
    }
    else if ( aType == getCppuType( (Reference<XElementAccess>*) NULL ) )
    {
        if (_xElementAccess.is())
        {
            return makeAny( Reference< XElementAccess >(
                (static_cast< XElementAccess* >((static_cast< XNameContainer* >(this))) ) ) );
        }
    }
    else if ( aType == getCppuType( (Reference<XInvocation2>*) NULL ) )
    {
        // Invocation does not support XInvocation2, if direct object supports
        // XInvocation, but not XInvocation2.
        if ( ( _xDirect.is() && _xDirect2.is()) ||
             (!_xDirect.is() && _xIntrospectionAccess.is() ) )
        {
            return makeAny( Reference< XInvocation2 >( (static_cast< XInvocation2* >(this)) ) );
        }
    }

    return OWeakObject::queryInterface( aType );
}


//--------------------------------------------------------------------------------------------------
Any Invocation_Impl::getMaterial(void) throw(RuntimeException)
{
    // AB, 12.2.1999  Sicherstellen, dass das Material wenn moeglich
    // aus der direkten Invocation bzw. von der Introspection geholt
    // wird, da sonst Structs nicht korrekt behandelt werden
    Reference<XMaterialHolder> xMaterialHolder;
    if( _xDirect.is() )
    {
        xMaterialHolder = Reference<XMaterialHolder>::query( _xDirect );
        //_xDirect->queryInterface( XMaterialHolder::getSmartUik(), xMaterialHolder );
    }
    else if( _xIntrospectionAccess.is() )
    {
        xMaterialHolder = Reference<XMaterialHolder>::query( _xIntrospectionAccess );
        //_xIntrospectionAccess->queryInterface( XMaterialHolder::getSmartUik(), xMaterialHolder );
    }
    if( xMaterialHolder.is() )
    {
        return xMaterialHolder->getMaterial();
    }
    return _aMaterial;
}

//--------------------------------------------------------------------------------------------------
void Invocation_Impl::setMaterial( const Any& rMaterial )
{
    // set the material first and only once
    Reference<XInterface> xObj;

    if (rMaterial.getValueType().getTypeClass() == TypeClass_INTERFACE)
        xObj = *(Reference<XInterface>*)rMaterial.getValue();
    _aMaterial = rMaterial;

    // Ersteinmal alles ausserhalb des guards machen
    _xDirect = Reference<XInvocation>::query( xObj );

    if( _xDirect.is() )
    {
        // Objekt direkt befragen
        _xElementAccess     = Reference<XElementAccess>::query( _xDirect );
        _xEnumerationAccess = Reference<XEnumerationAccess>::query( _xDirect );
        _xIndexAccess       = Reference<XIndexAccess>::query( _xDirect );
        _xIndexContainer    = Reference<XIndexContainer>::query( _xDirect );
        _xNameAccess        = Reference<XNameAccess>::query( _xDirect );
        _xNameContainer     = Reference<XNameContainer>::query( _xDirect );
        _xENDirect          = Reference<XExactName>::query( _xDirect );
        _xDirect2           = Reference<XInvocation2>::query( _xDirect );

        // only once!!!
        //_xIntrospectionAccess = XIntrospectionAccessRef();
        //_xPropertySet         = XPropertySetRef();
    }
    else
    {
        // Invocation ueber die Introspection machen
        if (xIntrospection.is())
        {
            _xIntrospectionAccess = xIntrospection->inspect( _aMaterial );
            if( _xIntrospectionAccess.is() )
            {

                _xElementAccess = Reference<XElementAccess>::query(
                      _xIntrospectionAccess->queryAdapter(
                                 getCppuType( (Reference<XElementAccess>*) NULL ) ) );

                _xEnumerationAccess = Reference<XEnumerationAccess>::query(
                           _xIntrospectionAccess->queryAdapter(
                                getCppuType( (Reference<XEnumerationAccess>*) NULL )) );

                _xIndexAccess = Reference<XIndexAccess>::query(
                       _xIntrospectionAccess->queryAdapter(
                                getCppuType( (Reference<XIndexAccess>*) NULL ) ) );

                _xIndexContainer = Reference<XIndexContainer>::query(
                     _xIntrospectionAccess->queryAdapter(
                                getCppuType( (Reference<XIndexContainer>*) NULL ) ) );

                _xNameAccess = Reference<XNameAccess>::query(
                     _xIntrospectionAccess->queryAdapter(
                                getCppuType( (Reference<XNameAccess>*) NULL ) ) );

                _xNameContainer = Reference<XNameContainer>::query(
                           _xIntrospectionAccess->queryAdapter(
                               getCppuType( (Reference<XNameContainer>*) NULL ) ) );

                _xPropertySet = Reference<XPropertySet>::query(
                           _xIntrospectionAccess->queryAdapter(
                               getCppuType( (Reference<XPropertySet>*) NULL )) );

                _xENIntrospection = Reference<XExactName>::query( _xIntrospectionAccess );
                if (_xNameAccess.is())
                    _xENNameAccess = Reference<XExactName>::query( _xNameAccess );
            }
        }
        /* only once !!!
        _xDirect = XInvocationRef();
        if( !_xIntrospectionAccess.is() )
        {
            // reset
            _xElementAccess     = XElementAccessRef();
            _xEnumerationAccess = XEnumerationAccessRef();
            _xIndexAccess       = XIndexAccessRef();
            _xIndexContainer    = XIndexContainerRef();
            _xNameAccess        = XNameAccessRef();
            _xNameContainer     = XNameContainerRef();
            _xPropertySet       = XPropertySetRef();
        }
        */
    }
}

//--------------------------------------------------------------------------------------------------
OUString Invocation_Impl::getExactName( const OUString& rApproximateName )
    throw( RuntimeException )
{
    if (_xENDirect.is())
        return _xENDirect->getExactName( rApproximateName );

    OUString aRet;
    if (_xENIntrospection.is())
        aRet = _xENIntrospection->getExactName( rApproximateName );
    if (aRet.isEmpty() && _xENNameAccess.is())
        aRet = _xENNameAccess->getExactName( rApproximateName );
    return aRet;
}

//--------------------------------------------------------------------------------------------------
Reference<XIntrospectionAccess> Invocation_Impl::getIntrospection(void)
    throw( RuntimeException )
{
    if( _xDirect.is() )
        return _xDirect->getIntrospection();
    else
        return _xIntrospectionAccess;
}

//--------------------------------------------------------------------------------------------------
sal_Bool Invocation_Impl::hasMethod( const OUString& Name )
    throw( RuntimeException )
{
    if (_xDirect.is())
        return _xDirect->hasMethod( Name );
    if( _xIntrospectionAccess.is() )
        return _xIntrospectionAccess->hasMethod( Name, MethodConcept::ALL ^ MethodConcept::DANGEROUS );
    return sal_False;
}

//--------------------------------------------------------------------------------------------------
sal_Bool Invocation_Impl::hasProperty( const OUString& Name )
    throw( RuntimeException )
{
    if (_xDirect.is())
        return _xDirect->hasProperty( Name );
    // PropertySet
    if( _xIntrospectionAccess.is()
        && _xIntrospectionAccess->hasProperty( Name, PropertyConcept::ALL ^ PropertyConcept::DANGEROUS ) )
        return sal_True;
    // NameAccess
    if( _xNameAccess.is() )
        return _xNameAccess->hasByName( Name );
    return sal_False;
}

//--------------------------------------------------------------------------------------------------
Any Invocation_Impl::getValue( const OUString& PropertyName )
    throw( UnknownPropertyException, RuntimeException )
{
    if (_xDirect.is())
        return _xDirect->getValue( PropertyName );
    try
    {
        // PropertySet
        if( _xIntrospectionAccess.is() && _xPropertySet.is()
            && _xIntrospectionAccess->hasProperty
            ( PropertyName, PropertyConcept::ALL ^ PropertyConcept::DANGEROUS ) )
        {
            return _xPropertySet->getPropertyValue( PropertyName );
        }
        // NameAccess
        if( _xNameAccess.is() && _xNameAccess->hasByName( PropertyName ) )
            return _xNameAccess->getByName( PropertyName );
    }
    catch (UnknownPropertyException &)
    {
        throw;
    }
    catch (RuntimeException &)
    {
        throw;
    }
    catch (Exception &)
    {
    }

    throw UnknownPropertyException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("cannot get value ") ) + PropertyName,
        Reference< XInterface >() );
}

//--------------------------------------------------------------------------------------------------
void Invocation_Impl::setValue( const OUString& PropertyName, const Any& Value )
    throw( UnknownPropertyException, CannotConvertException, InvocationTargetException, RuntimeException )
{
    if (_xDirect.is())
        _xDirect->setValue( PropertyName, Value );
    else
    {
        try
        {
            // Properties
            if( _xIntrospectionAccess.is() && _xPropertySet.is()
                && _xIntrospectionAccess->hasProperty(
                    PropertyName, PropertyConcept::ALL ^ PropertyConcept::DANGEROUS ) )
            {
                Property aProp = _xIntrospectionAccess->getProperty(
                    PropertyName, PropertyConcept::ALL ^ PropertyConcept::DANGEROUS );
                Reference < XIdlClass > r = TypeToIdlClass( aProp.Type, xCoreReflection );
                if( r->isAssignableFrom( TypeToIdlClass( Value.getValueType(), xCoreReflection ) ) )
                    _xPropertySet->setPropertyValue( PropertyName, Value );
                else if( xTypeConverter.is() )
                    _xPropertySet->setPropertyValue(
                        PropertyName, xTypeConverter->convertTo( Value, aProp.Type ) );
                else
                    throw RuntimeException(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("no type converter service!") ),
                        Reference< XInterface >() );
            }
            // NameContainer
            else if( _xNameContainer.is() )
            {
                Any aConv;
                Reference < XIdlClass > r =
                    TypeToIdlClass( _xNameContainer->getElementType(), xCoreReflection );
                if( r->isAssignableFrom(TypeToIdlClass( Value.getValueType(), xCoreReflection ) ) )
                    aConv = Value;
                else if( xTypeConverter.is() )
                    aConv = xTypeConverter->convertTo( Value, _xNameContainer->getElementType() );
                else
                    throw RuntimeException(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("no type converter service!") ),
                        Reference< XInterface >() );

                // bei Vorhandensein ersetzen, ansonsten einfuegen
                if (_xNameContainer->hasByName( PropertyName ))
                    _xNameContainer->replaceByName( PropertyName, aConv );
                else
                    _xNameContainer->insertByName( PropertyName, aConv );
            }
            else
                throw UnknownPropertyException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("no introspection nor name container!") ),
                    Reference< XInterface >() );
        }
        catch (UnknownPropertyException &)
        {
            throw;
        }
        catch (CannotConvertException &)
        {
            throw;
        }
        catch (InvocationTargetException &)
        {
            throw;
        }
        catch (RuntimeException &)
        {
            throw;
        }
        catch (const Exception & exc)
        {
            throw InvocationTargetException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("exception occurred in setValue(): ") ) +
                exc.Message, Reference< XInterface >(), makeAny( exc /* though sliced */ ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
Any Invocation_Impl::invoke( const OUString& FunctionName, const Sequence<Any>& InParams,
                                Sequence<sal_Int16>& OutIndizes, Sequence<Any>& OutParams )
    throw( IllegalArgumentException, CannotConvertException, InvocationTargetException, RuntimeException )
{
    if (_xDirect.is())
        return _xDirect->invoke( FunctionName, InParams, OutIndizes, OutParams );

    if (_xIntrospectionAccess.is())
    {
        // throw NoSuchMethodException if not exist
        Reference<XIdlMethod> xMethod = _xIntrospectionAccess->getMethod(
            FunctionName, MethodConcept::ALL ^ MethodConcept::DANGEROUS );

        // ParameterInfos
        Sequence<ParamInfo> aFParams        = xMethod->getParameterInfos();
        const ParamInfo* pFParams           = aFParams.getConstArray();
        sal_Int32 nFParamsLen               = aFParams.getLength();
        if (nFParamsLen != InParams.getLength())
        {
            throw IllegalArgumentException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("incorrect number of parameters passed invoking function ") ) + FunctionName,
                (OWeakObject *) this, (sal_Int16) 1 );
        }

        // IN Parameter
        const Any* pInParams                = InParams.getConstArray();

        // Introspection Invoke Parameter
        Sequence<Any> aInvokeParams( nFParamsLen );
        Any* pInvokeParams                  = aInvokeParams.getArray();

        // OUT Indizes
        OutIndizes.realloc( nFParamsLen );
        sal_Int16* pOutIndizes              = OutIndizes.getArray();
        sal_uInt32 nOutIndex                = 0;

        for ( sal_Int32 nPos = 0; nPos < nFParamsLen; ++nPos )
        {
            try
            {
                const ParamInfo& rFParam = pFParams[nPos];
                const Reference<XIdlClass>& rDestType = rFParam.aType;

                // is IN/INOUT parameter?
                if (rFParam.aMode != ParamMode_OUT)
                {
                    if (rDestType->isAssignableFrom( TypeToIdlClass( pInParams[nPos].getValueType(), xCoreReflection ) ))
                    {
                        pInvokeParams[nPos] = pInParams[nPos];
                    }
                    else if (xTypeConverter.is())
                    {
                        Type aDestType( rDestType->getTypeClass(), rDestType->getName() );
                        pInvokeParams[nPos] = xTypeConverter->convertTo( pInParams[nPos], aDestType );
                    }
                    else
                    {
                        CannotConvertException aExc;
                        aExc.Context = *this;
                        aExc.Message = OUString( RTL_CONSTASCII_USTRINGPARAM("invocation type mismatch!") );
                        throw aExc;
                    }
                }

                // is OUT/INOUT parameter?
                if (rFParam.aMode != ParamMode_IN)
                {
                    pOutIndizes[nOutIndex] = (sal_Int16)nPos;
                    if (rFParam.aMode == ParamMode_OUT)
                        rDestType->createObject( pInvokeParams[nPos] );     // default init
                    ++nOutIndex;
                }
            }
            catch( CannotConvertException& rExc )
            {
                rExc.ArgumentIndex = nPos;  // optionalen Parameter Index hinzufuegen
                throw;
            }
        }

        // execute Method
        Any aRet = xMethod->invoke( _aMaterial, aInvokeParams );

        // OUT Params
        OutIndizes.realloc( nOutIndex );
        pOutIndizes        = OutIndizes.getArray();
        OutParams.realloc( nOutIndex );
        Any* pOutParams = OutParams.getArray();

        while (nOutIndex--)
        {
            pOutParams[nOutIndex] = pInvokeParams[ pOutIndizes[nOutIndex] ];
        }

        return aRet;
    }

    RuntimeException aExc;
    aExc.Context = *this;
    aExc.Message = OUString( RTL_CONSTASCII_USTRINGPARAM("invocation lacking of introspection access!") );
    throw aExc;
}

//--------------------------------------------------------------------------------------------------

// Struct to optimize sorting
struct MemberItem
{
    OUString aName;

    // Defines where the member comes from
    enum Mode { NAMEACCESS, PROPERTYSET, METHOD } eMode;

    // Index to respective sequence
    // (Index to NameAccess sequence for eMode==NAMEACCESS etc.)
    sal_Int32 nIndex;
};

// Implementation of getting name or info
// String sequence will be filled when pStringSeq != NULL
// Info sequence will be filled when pInfoSeq != NULL
void Invocation_Impl::getInfoSequenceImpl
(
    Sequence< OUString >* pStringSeq,
    Sequence< InvocationInfo >* pInfoSeq
)
{
    //Sequence< OUString > aStrSeq;
    //if( !pStringSeq )
        //pStringSeq = &aStrSeq;


    // Get all needed sequences
    Sequence<OUString> aNameAccessNames;
    Sequence<Property> aPropertySeq;
    Sequence< Reference< XIdlMethod > > aMethodSeq;

    if( _xNameAccess.is() )
    {
        aNameAccessNames = _xNameAccess->getElementNames();
    }

    if( _xIntrospectionAccess.is() )
    {
        aPropertySeq = _xIntrospectionAccess->getProperties
            ( PropertyConcept::ALL - PropertyConcept::DANGEROUS );

        aMethodSeq = _xIntrospectionAccess->getMethods
            ( MethodConcept::ALL - MethodConcept::DANGEROUS );
    }

    sal_Int32 nNameAccessCount = aNameAccessNames.getLength();
    sal_Int32 nPropertyCount = aPropertySeq.getLength();
    sal_Int32 nMethodCount = aMethodSeq.getLength();
    sal_Int32 nTotalCount = nNameAccessCount + nPropertyCount + nMethodCount;

    // Create and fill array of MemberItems
    boost::scoped_array< MemberItem > pItems( new MemberItem[ nTotalCount ] );
    const OUString* pStrings = aNameAccessNames.getConstArray();
    const Property* pProps = aPropertySeq.getConstArray();
    const Reference< XIdlMethod >* pMethods = aMethodSeq.getConstArray();

    // Fill array of MemberItems
    sal_Int32 i, iTotal = 0;

    // Name Access
    for( i = 0 ; i < nNameAccessCount ; i++, iTotal++ )
    {
        MemberItem& rItem = pItems[ iTotal ];
        rItem.aName = pStrings[ i ];
        rItem.eMode = MemberItem::NAMEACCESS;
        rItem.nIndex = i;
    }

    // Property set
    for( i = 0 ; i < nPropertyCount ; i++, iTotal++ )
    {
        MemberItem& rItem = pItems[ iTotal ];
        rItem.aName = pProps[ i ].Name;
        rItem.eMode = MemberItem::PROPERTYSET;
        rItem.nIndex = i;
    }

    // Methods
    for( i = 0 ; i < nMethodCount ; i++, iTotal++ )
    {
        MemberItem& rItem = pItems[ iTotal ];
        Reference< XIdlMethod > xMethod = pMethods[ i ];
        rItem.aName = xMethod->getName();
        rItem.eMode = MemberItem::METHOD;
        rItem.nIndex = i;
    }

    // Setting up result sequences
    OUString* pRetStrings = NULL;
    if( pStringSeq )
    {
        pStringSeq->realloc( nTotalCount );
        pRetStrings = pStringSeq->getArray();
    }

    InvocationInfo* pRetInfos = NULL;
    if( pInfoSeq )
    {
        pInfoSeq->realloc( nTotalCount );
        pRetInfos = pInfoSeq->getArray();
    }

    // Fill result sequences in the correct order of members
    for( iTotal = 0 ; iTotal < nTotalCount ; iTotal++ )
    {
        MemberItem& rItem = pItems[ iTotal ];
        if( pRetStrings )
        {
            pRetStrings[ iTotal ] = rItem.aName;
        }

        if( pRetInfos )
        {
            if( rItem.eMode == MemberItem::NAMEACCESS )
            {
                fillInfoForNameAccess( pRetInfos[ iTotal ], rItem.aName );
            }
            else if( rItem.eMode == MemberItem::PROPERTYSET )
            {
                fillInfoForProperty( pRetInfos[ iTotal ], pProps[ rItem.nIndex ] );
            }
            else if( rItem.eMode == MemberItem::METHOD )
            {
                fillInfoForMethod( pRetInfos[ iTotal ], pMethods[ rItem.nIndex ] );
            }
        }
    }
}

// XInvocation2
Sequence< OUString > SAL_CALL Invocation_Impl::getMemberNames(  )
    throw( RuntimeException )
{
    if( _xDirect2.is() )
    {
        return _xDirect2->getMemberNames();
    }
    Sequence< OUString > aRetSeq;
    getInfoSequenceImpl( &aRetSeq, NULL );
    return aRetSeq;
}

Sequence< InvocationInfo > SAL_CALL Invocation_Impl::getInfo(  )
    throw( RuntimeException )
{
    if( _xDirect2.is() )
    {
        return _xDirect2->getInfo();
    }
    Sequence< InvocationInfo > aRetSeq;
    getInfoSequenceImpl( NULL, &aRetSeq );
    return aRetSeq;
}

InvocationInfo SAL_CALL Invocation_Impl::getInfoForName( const OUString& aName, sal_Bool bExact )
    throw( IllegalArgumentException, RuntimeException )
{
    if( _xDirect2.is() )
    {
        return _xDirect2->getInfoForName( aName, bExact );
    }

    sal_Bool bFound = sal_False;
    OUString aExactName = aName;
    InvocationInfo aRetInfo;

    if( bExact )
        aExactName = getExactName( aName );
    if( !aExactName.isEmpty() )
    {
        if( _xIntrospectionAccess->hasMethod( aExactName, MethodConcept::ALL ^ MethodConcept::DANGEROUS ) )
        {
            Reference<XIdlMethod> xMethod = _xIntrospectionAccess->getMethod
                ( aExactName, MethodConcept::ALL ^ MethodConcept::DANGEROUS );
            fillInfoForMethod( aRetInfo, xMethod );
            bFound = sal_True;
        }
        else
        {
            if( _xIntrospectionAccess.is() && _xIntrospectionAccess->hasProperty
                 ( aExactName, PropertyConcept::ALL ^ PropertyConcept::DANGEROUS ) )
            {
                 Property aProp = _xIntrospectionAccess->getProperty
                    ( aExactName, PropertyConcept::ALL ^ PropertyConcept::DANGEROUS );
                fillInfoForProperty( aRetInfo, aProp );
                bFound = sal_True;
            }
            // NameAccess
            else if( _xNameAccess.is() && _xNameAccess->hasByName( aExactName ) )
            {
                fillInfoForNameAccess( aRetInfo, aExactName );
                bFound = sal_True;
            }
        }
    }
    if( !bFound )
    {
        throw IllegalArgumentException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("Unknown name, getExactName() failed!") ),
            (XWeak *)(OWeakObject *)this, 0 );
    }
    return aRetInfo;
}

// Helper functions to fill InvocationInfo for XNameAccess
void Invocation_Impl::fillInfoForNameAccess
(
    InvocationInfo& rInfo,
    const OUString& aName
)
{
    rInfo.aName = aName;
    rInfo.eMemberType = MemberType_PROPERTY;
    rInfo.PropertyAttribute = 0;
    if( !_xNameContainer.is() )
    {
        rInfo.PropertyAttribute = PropertyAttribute::READONLY;
    }
    rInfo.aType = _xNameAccess->getElementType();
}

void Invocation_Impl::fillInfoForProperty
(
    InvocationInfo& rInfo,
    const Property& rProp
)
{
    rInfo.aName = rProp.Name;
    rInfo.eMemberType = MemberType_PROPERTY;
    rInfo.PropertyAttribute = rProp.Attributes;
    rInfo.aType = rProp.Type;
}

void Invocation_Impl::fillInfoForMethod
(
    InvocationInfo& rInfo,
    const Reference< XIdlMethod > xMethod
)
{
    rInfo.aName = xMethod->getName();
    rInfo.eMemberType = MemberType_METHOD;
    Reference< XIdlClass > xReturnClass = xMethod->getReturnType();
    Type aReturnType( xReturnClass->getTypeClass(), xReturnClass->getName() );
    rInfo.aType = aReturnType;
    Sequence<ParamInfo> aParamInfos = xMethod->getParameterInfos();
    sal_Int32 nParamCount = aParamInfos.getLength();
    if( nParamCount > 0 )
    {
        const ParamInfo* pInfos = aParamInfos.getConstArray();

        rInfo.aParamTypes.realloc( nParamCount );
        Type* pParamTypes = rInfo.aParamTypes.getArray();
        rInfo.aParamModes.realloc( nParamCount );
        ParamMode* pParamModes = rInfo.aParamModes.getArray();

        for( sal_Int32 i = 0 ; i < nParamCount ; i++ )
        {
            Reference< XIdlClass > xParamClass = pInfos[i].aType;
            Type aParamType( xParamClass->getTypeClass(), xParamClass->getName() );
            pParamTypes[ i ] = aParamType;
            pParamModes[ i ] = pInfos[i].aMode;
        }
    }
}


// XTypeProvider
Sequence< Type > SAL_CALL Invocation_Impl::getTypes(void) throw( RuntimeException )
{
    static Sequence< Type > const * s_pTypes = 0;
    if (! s_pTypes)
    {
        Sequence< Type > types( 4 +8 );
        Type * pTypes = types.getArray();
        sal_Int32 n = 0;

        pTypes[ n++ ] = ::getCppuType( (Reference< XTypeProvider > const *)0 );
        pTypes[ n++ ] = ::getCppuType( (Reference< XWeak > const *)0 );
        pTypes[ n++ ] = ::getCppuType( (Reference< XInvocation > const *)0 );
        pTypes[ n++ ] = ::getCppuType( (Reference< XMaterialHolder > const *)0 );

        // Ivocation does not support XExactName, if direct object supports
        // XInvocation, but not XExactName.
        if ((_xDirect.is() && _xENDirect.is()) ||
            (!_xDirect.is() && (_xENIntrospection.is() || _xENNameAccess.is())))
        {
            pTypes[ n++ ] = ::getCppuType( (Reference< XExactName > const *)0 );
        }
        if( _xNameContainer.is() )
        {
            pTypes[ n++ ] = ::getCppuType( (Reference< XNameContainer > const *)0 );
        }
        if( _xNameAccess.is() )
        {
            pTypes[ n++ ] = ::getCppuType( (Reference< XNameAccess > const *)0 );
        }
        if (_xIndexContainer.is())
        {
            pTypes[ n++ ] = ::getCppuType( (Reference< XIndexContainer > const *)0 );
        }
        if (_xIndexAccess.is())
        {
            pTypes[ n++ ] = ::getCppuType( (Reference< XIndexAccess > const *)0 );
        }
        if (_xEnumerationAccess.is())
        {
            pTypes[ n++ ] = ::getCppuType( (Reference< XEnumerationAccess > const *)0 );
        }
        if (_xElementAccess.is())
        {
            pTypes[ n++ ] = ::getCppuType( (Reference< XElementAccess > const *)0 );
        }
        // Invocation does not support XInvocation2, if direct object supports
        // XInvocation, but not XInvocation2.
        if ( ( _xDirect.is() && _xDirect2.is()) ||
             (!_xDirect.is() && _xIntrospectionAccess.is() ) )
        {
            pTypes[ n++ ] = ::getCppuType( (Reference< XInvocation2 > const *)0 );
        }

        types.realloc( n );

        // store types
        MutexGuard guard( Mutex::getGlobalMutex() );
        if (! s_pTypes)
        {
            static Sequence< Type > s_types( types );
            s_pTypes = &s_types;
        }
    }
    return *s_pTypes;
}

Sequence< sal_Int8 > SAL_CALL Invocation_Impl::getImplementationId(  ) throw( RuntimeException)
{
    static OImplementationId *pId = 0;
    if( ! pId )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( ! pId )
        {
            static OImplementationId id( sal_False );
            pId = &id;
        }
    }
    return pId->getImplementationId();
}

//==================================================================================================
//==================================================================================================
//==================================================================================================
class InvocationService
    : public WeakImplHelper2< XSingleServiceFactory, XServiceInfo >
{
public:
    InvocationService( const Reference<XComponentContext> & xCtx );
    virtual ~InvocationService();

    // XServiceInfo
    OUString                    SAL_CALL getImplementationName() throw( RuntimeException );
    sal_Bool                        SAL_CALL supportsService(const OUString& ServiceName) throw( RuntimeException );
    Sequence< OUString >        SAL_CALL getSupportedServiceNames(void) throw( RuntimeException );

    // XSingleServiceFactory
    Reference<XInterface>       SAL_CALL createInstance(void) throw( Exception, RuntimeException );
    Reference<XInterface>       SAL_CALL createInstanceWithArguments(
        const Sequence<Any>& rArguments ) throw( Exception, RuntimeException );
private:
    Reference<XComponentContext> mxCtx;
    Reference<XMultiComponentFactory> mxSMgr;
    Reference<XTypeConverter> xTypeConverter;
    Reference<XIntrospection> xIntrospection;
    Reference<XIdlReflection> xCoreReflection;
};

InvocationService::InvocationService( const Reference<XComponentContext> & xCtx )
    : mxCtx( xCtx )
    , mxSMgr( xCtx->getServiceManager() )
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    xTypeConverter = Reference<XTypeConverter>(
        mxSMgr->createInstanceWithContext(
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.script.Converter")),
            xCtx ),
        UNO_QUERY );
    xIntrospection = Introspection::create(xCtx);
    mxCtx->getValueByName(
        OUString(
            RTL_CONSTASCII_USTRINGPARAM("/singletons/com.sun.star.reflection.theCoreReflection")) )
                >>= xCoreReflection;
    OSL_ENSURE( xCoreReflection.is(), "### CoreReflection singleton not accessable!?" );
    if (! xCoreReflection.is())
    {
        throw DeploymentException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("/singletons/com.sun.star.reflection.theCoreReflection singleton not accessable") ),
            Reference< XInterface >() );
    }
//         xCoreReflection = Reference<XIdlReflection>(
//      mxSMgr->createInstanceWithContext(
//          OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.reflection.CoreReflection")),
//          xCtx),
//      UNO_QUERY);
}

InvocationService::~InvocationService()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

// XServiceInfo
OUString InvocationService::getImplementationName() throw( RuntimeException )
{
    return inv_getImplementationName();
}

// XServiceInfo
sal_Bool InvocationService::supportsService(const OUString& ServiceName) throw( RuntimeException )
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

// XServiceInfo
Sequence< OUString > InvocationService::getSupportedServiceNames(void) throw( RuntimeException )
{
    return inv_getSupportedServiceNames();
}

//--------------------------------------------------------------------------------------------------
Reference<XInterface> InvocationService::createInstance(void) throw( Exception, RuntimeException )
{
    //TODO:throw( Exception(OUString( RTL_CONSTASCII_USTRINGPARAM("no default construction of invocation adapter possible!")), *this) );
    return Reference<XInterface>(); // dummy
}

//--------------------------------------------------------------------------------------------------
Reference<XInterface> InvocationService::createInstanceWithArguments(
    const Sequence<Any>& rArguments ) throw( Exception, RuntimeException )
{
    if (rArguments.getLength() == 1)
    {
        return Reference< XInterface >
            ( *new Invocation_Impl( *rArguments.getConstArray(),
              xTypeConverter, xIntrospection, xCoreReflection ) );
    }
    else
    {
        //TODO:throw( Exception(OUString( RTL_CONSTASCII_USTRINGPARAM("no default construction of invocation adapter possible!")), *this) );
        return Reference<XInterface>();
    }
}


//*************************************************************************
Reference<XInterface> SAL_CALL InvocationService_CreateInstance( const Reference<XComponentContext> & xCtx )
    throw( RuntimeException )
{
    Reference<XInterface> xService = Reference< XInterface > ( *new InvocationService( xCtx ) );
    return xService;
}

}

using namespace stoc_inv;
static struct ImplementationEntry g_entries[] =
{
    {
        InvocationService_CreateInstance, inv_getImplementationName,
        inv_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt , 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C"
{
SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_canUnload( TimeValue *pTime )
{
    return g_moduleCount.canUnload( &g_moduleCount , pTime );
}

//==================================================================================================
SAL_DLLPUBLIC_EXPORT void * SAL_CALL invocation_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
