/*************************************************************************
 *
 *  $RCSfile: invocation.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dbo $ $Date: 2000-10-06 14:25:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _CPPUHELER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#include <cppuhelper/typeprovider.hxx>

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
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/beans/PropertyConcept.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

#include <stdlib.h>
#include <search.h>


#define SERVICE_NAME "com.sun.star.script.Invocation"
#define IMPL_NAME    "com.sun.star.comp.stoc.Invocation"

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::script;
using namespace com::sun::star::reflection;
using namespace com::sun::star::beans;
using namespace com::sun::star::registry;
using namespace com::sun::star::container;
using namespace cppu;
using namespace rtl;
using namespace osl;


namespace stoc_inv
{

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


    // XInterface
    virtual Any         SAL_CALL queryInterface( const Type & aType) throw( RuntimeException );
    virtual void        SAL_CALL acquire() throw() { OWeakObject::acquire(); }
    virtual void        SAL_CALL release() throw() { OWeakObject::release(); }
    //void*             getImplementation(Reflection *p) { return OWeakObject::getImplementation(p); }


    // XTypeProvider
    virtual Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  )
       throw(RuntimeException);
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId(  )
       throw( RuntimeException);

    // Methoden von XMaterialHolder
    virtual Any         SAL_CALL getMaterial(void);

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
    : xIntrospection( rI )
    , xTypeConverter( rTC )
    , xCoreReflection( rCR )
{
    setMaterial( rAdapted );
}

//##################################################################################################
//### INTERFACE IMPLEMENTATIONS ####################################################################
//##################################################################################################


Any SAL_CALL Invocation_Impl::queryInterface( const Type & aType )
    throw( RuntimeException )
{
    // TODO: Aendern, so sehr ineffektiv,

    // PropertySet-Implementation
    Any a = ::cppu::queryInterface( aType,
                                   SAL_STATIC_CAST(XInvocation*, this),
                                   SAL_STATIC_CAST(XMaterialHolder*, this),
                                   SAL_STATIC_CAST(XTypeProvider *,this)    );
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
            return cppu::queryInterface( aType, SAL_STATIC_CAST(XExactName*, this) );
        }
    }
    else if ( aType == getCppuType( (Reference<XNameContainer>*) NULL ) )
    {
        if( _xNameContainer.is() )
            return cppu::queryInterface( aType, SAL_STATIC_CAST(XNameContainer*, this) );
    }
    else if ( aType == getCppuType( (Reference<XNameAccess>*) NULL ) )
    {
        if( _xNameAccess.is() )
            return cppu::queryInterface( aType, SAL_STATIC_CAST(XNameAccess*, this) );
    }
    else if ( aType == getCppuType( (Reference<XIndexContainer>*) NULL ) )
    {
        if (_xIndexContainer.is())
            return cppu::queryInterface( aType, SAL_STATIC_CAST(XIndexContainer*, this) );
    }
    else if ( aType == getCppuType( (Reference<XIndexAccess>*) NULL ) )
    {
        if (_xIndexAccess.is())
            return cppu::queryInterface( aType, SAL_STATIC_CAST(XIndexAccess*, this) );
    }
    else if ( aType == getCppuType( (Reference<XEnumerationAccess>*) NULL ) )
    {
        if (_xEnumerationAccess.is())
            return cppu::queryInterface( aType , SAL_STATIC_CAST(XEnumerationAccess*, this) );
    }
    else if ( aType == getCppuType( (Reference<XElementAccess>*) NULL ) )
    {
        if (_xElementAccess.is())
        {
            return ::cppu::queryInterface
                ( aType , SAL_STATIC_CAST(XElementAccess*, SAL_STATIC_CAST(XNameContainer*, this) ) );
        }
    }
    else if ( aType == getCppuType( (Reference<XInvocation2>*) NULL ) )
    {
        // Invocation does not support XInvocation2, if direct object supports
        // XInvocation, but not XInvocation2.
        if ( ( _xDirect.is() && _xDirect2.is()) ||
             (!_xDirect.is() && _xIntrospectionAccess.is() ) )
        {
            return cppu::queryInterface( aType, SAL_STATIC_CAST(XInvocation2*, this) );
        }
    }

    return OWeakObject::queryInterface( aType );
}


//--------------------------------------------------------------------------------------------------
Any Invocation_Impl::getMaterial(void)
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
    if (!aRet.len() && _xENNameAccess.is())
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
    // PropertySet
    if( _xIntrospectionAccess.is() && _xPropertySet.is()
        && _xIntrospectionAccess->hasProperty( PropertyName, PropertyConcept::ALL ^ PropertyConcept::DANGEROUS ) )
    {
        return _xPropertySet->getPropertyValue( PropertyName );
    }
    // NameAccess
    if( _xNameAccess.is() && _xNameAccess->hasByName( PropertyName ) )
        return _xNameAccess->getByName( PropertyName );

    throw UnknownPropertyException();
    return Any();   // dummy
}

//--------------------------------------------------------------------------------------------------
void Invocation_Impl::setValue( const OUString& PropertyName, const Any& Value )
    throw( UnknownPropertyException, CannotConvertException, InvocationTargetException, RuntimeException )
{
    if (_xDirect.is())
        _xDirect->setValue( PropertyName, Value );
    else
    {
        // Properties

      if( _xIntrospectionAccess.is() && _xPropertySet.is()
            && _xIntrospectionAccess->hasProperty( PropertyName, PropertyConcept::ALL ^ PropertyConcept::DANGEROUS ) )
        {
                 Property aProp = _xIntrospectionAccess->getProperty( PropertyName, PropertyConcept::ALL ^ PropertyConcept::DANGEROUS );
            Reference < XIdlClass > r = TypeToIdlClass( aProp.Type, xCoreReflection );
            if( r->isAssignableFrom(
                                   TypeToIdlClass( Value.getValueType(), xCoreReflection ) ) )
                _xPropertySet->setPropertyValue( PropertyName, Value );
            else if( xTypeConverter.is() )
                _xPropertySet->setPropertyValue( PropertyName, xTypeConverter->convertTo( Value, aProp.Type ) );
            else
                throw CannotConvertException();
        }
        // NameContainer
        else

        if( _xNameContainer.is() )
        {
            Any aConv;
            Reference < XIdlClass > r = TypeToIdlClass( _xNameContainer->getElementType(), xCoreReflection );
            if( r->isAssignableFrom(TypeToIdlClass( Value.getValueType(), xCoreReflection ) ) )
                aConv = Value;
            else if( xTypeConverter.is() )
                aConv = xTypeConverter->convertTo( Value, _xNameContainer->getElementType() );
            else
                throw CannotConvertException();

            // bei Vorhandensein ersetzen, ansonsten einfuegen
            if (_xNameContainer->hasByName( PropertyName ))
                _xNameContainer->replaceByName( PropertyName, aConv );
            else
                _xNameContainer->insertByName( PropertyName, aConv );
        }
        else
            throw UnknownPropertyException();
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
        sal_uInt32 nFParamsLen              = aFParams.getLength();

        // IN Parameter
        const Any* pInParams                = InParams.getConstArray();
        sal_uInt32 nInParamsLen             = InParams.getLength();
        OSL_ENSURE( nInParamsLen == nFParamsLen, "### new convention used for XInvocation::invoke!" );

        // Introspection Invoke Parameter
        Sequence<Any> aInvokeParams( nFParamsLen );
        Any* pInvokeParams                  = aInvokeParams.getArray();

        // OUT Indizes
        OutIndizes.realloc( nFParamsLen );
        sal_Int16* pOutIndizes              = OutIndizes.getArray();
        sal_uInt32 nOutIndex                = 0;

        sal_uInt32 nPos;

        try
        {
            for ( nPos = 0; nPos < nFParamsLen; ++nPos )
            {
                const ParamInfo& rFParam = pFParams[nPos];
                const Reference<XIdlClass>& rDestType = rFParam.aType;

                // is IN/INOUT parameter?
                if (rFParam.aMode != ParamMode_OUT)
                {
//                      // IN parameter available?
//                      if (nInIndex >= nInParamsLen)
//                          throw CannotConvertException();
                    //TODO: Parameter? throw( CannotConvertException( OUString( RTL_CONSTASCII_USTRINGPARAM(not enough IN parameters available!")), Reference<XInterface>(), rDestType->getTypeClass(), FailReason::NO_DEFAULT_AVAILABLE, nInIndex ) );

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
                    pOutIndizes[nOutIndex] = nPos;
                    if (rFParam.aMode == ParamMode_OUT)
                        rDestType->createObject( pInvokeParams[nPos] );     // default init
                    ++nOutIndex;
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
        catch( CannotConvertException& rExc )
        {
            rExc.ArgumentIndex = nPos;  // optionalen Parameter Index hinzufuegen
            throw rExc;
        }
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
    enum { NAMEACCESS, PROPERTYSET, METHOD } eMode;

    // Index to respective sequence
    // (Index to NameAccess sequence for eMode==NAMEACCESS etc.)
    sal_Int32 nIndex;
};

// qsort compare function for MemberItem
//int __cdecl compare(const void *elem1, const void *elem2 )
int SAL_CALL compare(const void *elem1, const void *elem2 )
{
    MemberItem* pItem1 = *(MemberItem**)elem1;
    MemberItem* pItem2 = *(MemberItem**)elem2;
    return (int)pItem1->aName.compareTo( pItem2->aName );
}


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
    MemberItem* pItems = new MemberItem[ nTotalCount ];
    const OUString* pStrings = aNameAccessNames.getConstArray();
    const Property* pProps = aPropertySeq.getConstArray();
    const Reference< XIdlMethod >* pMethods = aMethodSeq.getConstArray();

    // Create array of MemberItem* for sorting
    MemberItem** ppItems = new MemberItem*[ nTotalCount ];

    // Fill array of MemberItems
    sal_Int32 i, iTotal = 0;

    // Name Access
    for( i = 0 ; i < nNameAccessCount ; i++, iTotal++ )
    {
        MemberItem& rItem = pItems[ iTotal ];
        ppItems[ iTotal ] = &rItem;
        rItem.aName = pStrings[ i ];
        rItem.eMode = MemberItem::NAMEACCESS;
        rItem.nIndex = i;
    }

    // Property set
    for( i = 0 ; i < nPropertyCount ; i++, iTotal++ )
    {
        MemberItem& rItem = pItems[ iTotal ];
        ppItems[ iTotal ] = &rItem;
        rItem.aName = pProps[ i ].Name;
        rItem.eMode = MemberItem::PROPERTYSET;
        rItem.nIndex = i;
    }

    // Methods
    for( i = 0 ; i < nMethodCount ; i++, iTotal++ )
    {
        MemberItem& rItem = pItems[ iTotal ];
        ppItems[ iTotal ] = &rItem;
        Reference< XIdlMethod > xMethod = pMethods[ i ];
        rItem.aName = xMethod->getName();
        rItem.eMode = MemberItem::METHOD;
        rItem.nIndex = i;
    }

    // Sort pointer array
    qsort( ppItems, (size_t)nTotalCount, sizeof( MemberItem* ), compare );

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
    if( aExactName.getLength() > 0 )
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


// XIdlClassProvider

// XTypeProvider
Sequence< Type > SAL_CALL Invocation_Impl::getTypes(void) throw( RuntimeException )
{
  // TODO !!!!

  return Sequence< Type > ();
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
  return (*pId).getImplementationId();
}

/*
Sequence< Reference<XIdlClass> > Invocation_Impl::getIdlClasses(void) throw( RuntimeException )
{
    Reflection * ppReflection[7];
    Usal_Int16 i = 0;
    ppReflection[i++] = XInvocation_getReflection();
    if( _xElementAccess.is() )
        ppReflection[i++] = XElementAccess_getReflection();
    if( _xEnumerationAccess.is() )
        ppReflection[i++] = XEnumerationAccess_getReflection();
    if( _xIndexAccess.is() )
        ppReflection[i++] = XIndexAccess_getReflection();
    if( _xNameAccess.is() )
        ppReflection[i++] = XNameAccess_getReflection();
    if( _xIndexContainer.is() )
        ppReflection[i++] = XIndexContainer_getReflection();
    if( _xNameContainer.is() )
        ppReflection[i++] = XNameContainer_getReflection();

    // Ivocation does not support XExactName, if direct object supports
    // XInvocation, but not XExactName.
    if ((_xDirect.is() && _xENDirect.is()) ||
        (!_xDirect.is() && (_xENIntrospection.is() || _xENNameAccess.is())))
    {
        ppReflection[i++] = XExactName_getReflection();
    }

    Reference<XIdlClass> xClass = createStandardClass( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.Invocation")),
                                                  OWeakObject::getStaticIdlClass(), i,
                                                  ppReflection );
    return Sequence<Reference<XIdlClass>>( &xClass, 1 );
}
*/
//==================================================================================================
//==================================================================================================
//==================================================================================================
class InvocationService
    : public OWeakObject
    , public XSingleServiceFactory
    , public XServiceInfo
{
public:
            InvocationService( const Reference<XMultiServiceFactory> & rSMgr )
                : mxSMgr( rSMgr )
                , xTypeConverter( Reference<XTypeConverter>::query( rSMgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.script.Converter")) ) ) )
                , xIntrospection( Reference<XIntrospection>::query( rSMgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.beans.Introspection")) ) ) )
                , xCoreReflection( Reference<XIdlReflection>::query( rSMgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.reflection.CoreReflection")) ) ) )
            {}

    // XInterface
    virtual Any         SAL_CALL queryInterface( const Type & aType ) throw( RuntimeException );
    virtual void        SAL_CALL acquire() throw() { OWeakObject::acquire(); }
    virtual void        SAL_CALL release() throw() { OWeakObject::release(); }
    //void*             getImplementation(Reflection *p) { return OWeakObject::getImplementation(p); }

    // XIdlClassProvider
  //    virtual Sequence< Reference<XIdlClass> >    SAL_CALL getIdlClasses(void) throw( RuntimeException );
    // XTypeProvider
    virtual Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  )
       throw(RuntimeException);
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId(  )
       throw( RuntimeException);

    // XServiceInfo
    OUString                    SAL_CALL getImplementationName() throw( RuntimeException );
    static OUString             SAL_CALL getImplementationName_Static() throw( RuntimeException )
                                {
                                    return OUString::createFromAscii( IMPL_NAME );
                                }
    sal_Bool                        SAL_CALL supportsService(const OUString& ServiceName) throw( RuntimeException );
    Sequence< OUString >        SAL_CALL getSupportedServiceNames(void) throw( RuntimeException );
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(void) throw( RuntimeException );

    // XSingleServiceFactory
    Reference<XInterface>       SAL_CALL createInstance(void) throw( Exception, RuntimeException );
    Reference<XInterface>       SAL_CALL createInstanceWithArguments(
        const Sequence<Any>& rArguments ) throw( Exception, RuntimeException );
private:

    Reference<XMultiServiceFactory> mxSMgr;
    Reference<XTypeConverter> xTypeConverter;
    Reference<XIntrospection> xIntrospection;
    Reference<XIdlReflection> xCoreReflection;
};

//--------------------------------------------------------------------------------------------------
Any SAL_CALL InvocationService::queryInterface( const Type & aType )
    throw( RuntimeException )
{
    // PropertySet-Implementation
    Any a =  cppu::queryInterface( aType,
                                             SAL_STATIC_CAST(XSingleServiceFactory*, this),
                                             SAL_STATIC_CAST(XServiceInfo*, this) );
    if( a.hasValue() )
    {
      return a;
    }

    return OWeakObject::queryInterface( aType );
}


// XTypeProvider
Sequence< Type > SAL_CALL InvocationService::getTypes(void) throw( RuntimeException )
{
  static OTypeCollection *pCollection = 0;
  if( ! pCollection )
  {
      MutexGuard guard( Mutex::getGlobalMutex() );
      if( ! pCollection )
      {
          static OTypeCollection collection(
                    getCppuType( (Reference< XSingleServiceFactory> * )0),
                    getCppuType( (Reference< XServiceInfo > * ) 0 ) );
          pCollection = &collection;
      }
  }

  return (*pCollection).getTypes();
}

Sequence< sal_Int8 > SAL_CALL InvocationService::getImplementationId(  ) throw( RuntimeException)
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
  return (*pId).getImplementationId();
}

// // XIdlClassProvider
// Sequence< Reference<XIdlClass> > InvocationService::getIdlClasses(void) throw( RuntimeException )
// {
//  Sequence< Reference<XIdlClass> > aSeq( &getStaticIdlClass(), 1 );
//  return aSeq;
// }


// XServiceInfo
OUString InvocationService::getImplementationName() throw( RuntimeException )
{
    return getImplementationName_Static();
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
    return getSupportedServiceNames_Static();
}

// ORegistryServiceManager_Static
Sequence< OUString > InvocationService::getSupportedServiceNames_Static(void) throw( RuntimeException )
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString::createFromAscii( SERVICE_NAME );
    return aSNS;
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
Reference<XInterface> SAL_CALL InvocationService_CreateInstance( const Reference<XMultiServiceFactory> & rSMgr )
    throw( RuntimeException )
{
    Reference<XInterface> xService = Reference< XInterface > ( *new InvocationService( rSMgr ) );
    return xService;
}

}

extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString::createFromAscii( "/" IMPL_NAME "/UNO/SERVICES" ) ) );

            const Sequence< OUString > & rSNL =
                stoc_inv::InvocationService::getSupportedServiceNames_Static();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSHURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPL_NAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            stoc_inv::InvocationService_CreateInstance,
            stoc_inv::InvocationService::getSupportedServiceNames_Static() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}


