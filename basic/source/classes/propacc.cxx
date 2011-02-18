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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"

#include "propacc.hxx"

#include <tools/urlobj.hxx>
#include <tools/errcode.hxx>
#include <svl/svarray.hxx>
#include <basic/sbstar.hxx>
#include <sbunoobj.hxx>

using com::sun::star::uno::Reference;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace cppu;


//========================================================================

// Declaration conversion from Sbx to UNO with known target type
Any sbxToUnoValue( SbxVariable* pVar, const Type& rType, Property* pUnoProperty = NULL );

//========================================================================

#ifdef WNT
#define CDECL _cdecl
#endif
#if defined(UNX) || defined(OS2)
#define CDECL
#endif

int CDECL SbCompare_PropertyValues_Impl( const void *arg1, const void *arg2 )
{
   return ((PropertyValue*)arg1)->Name.compareTo( ((PropertyValue*)arg2)->Name );
}

extern "C" int CDECL SbCompare_UString_PropertyValue_Impl( const void *arg1, const void *arg2 )
{
    const ::rtl::OUString *pArg1 = (::rtl::OUString*) arg1;
    const PropertyValue **pArg2 = (const PropertyValue**) arg2;
    return pArg1->compareTo( (*pArg2)->Name );
}

int CDECL SbCompare_Properties_Impl( const void *arg1, const void *arg2 )
{
   return ((Property*)arg1)->Name.compareTo( ((Property*)arg2)->Name );
}

extern "C" int CDECL SbCompare_UString_Property_Impl( const void *arg1, const void *arg2 )
{
    const ::rtl::OUString *pArg1 = (::rtl::OUString*) arg1;
    const Property *pArg2 = (Property*) arg2;
    return pArg1->compareTo( pArg2->Name );
}

//----------------------------------------------------------------------------

SbPropertyValues::SbPropertyValues()
{
}

//----------------------------------------------------------------------------

SbPropertyValues::~SbPropertyValues()
{
    _xInfo = Reference< XPropertySetInfo >();

    for ( sal_uInt16 n = 0; n < _aPropVals.Count(); ++n )
        delete _aPropVals.GetObject( n );
}

//----------------------------------------------------------------------------

Reference< XPropertySetInfo > SbPropertyValues::getPropertySetInfo(void) throw( RuntimeException )
{
    // create on demand?
    if ( !_xInfo.is() )
    {
        SbPropertySetInfo *pInfo = new SbPropertySetInfo( _aPropVals );
        ((SbPropertyValues*)this)->_xInfo = (XPropertySetInfo*)pInfo;
    }
    return _xInfo;
}

//-------------------------------------------------------------------------

sal_Int32 SbPropertyValues::GetIndex_Impl( const ::rtl::OUString &rPropName ) const
{
    PropertyValue **ppPV;
    ppPV = (PropertyValue **)
            bsearch( &rPropName, _aPropVals.GetData(), _aPropVals.Count(),
                      sizeof( PropertyValue* ),
                      SbCompare_UString_PropertyValue_Impl );
    return ppPV ? ( (ppPV-_aPropVals.GetData()) / sizeof(ppPV) ) : USHRT_MAX;
}

//----------------------------------------------------------------------------

void SbPropertyValues::setPropertyValue(
                    const ::rtl::OUString& aPropertyName,
                    const Any& aValue)
                    throw (::com::sun::star::beans::UnknownPropertyException,
                    ::com::sun::star::beans::PropertyVetoException,
                    ::com::sun::star::lang::IllegalArgumentException,
                    ::com::sun::star::lang::WrappedTargetException,
                    ::com::sun::star::uno::RuntimeException)
{
    sal_Int32 nIndex = GetIndex_Impl( aPropertyName );
    PropertyValue *pPropVal = _aPropVals.GetObject(
        sal::static_int_cast< sal_uInt16 >(nIndex));
    pPropVal->Value = aValue;
}

//----------------------------------------------------------------------------

Any SbPropertyValues::getPropertyValue(
                    const ::rtl::OUString& aPropertyName)
                    throw(::com::sun::star::beans::UnknownPropertyException,
                    ::com::sun::star::lang::WrappedTargetException,
                    ::com::sun::star::uno::RuntimeException)
{
    sal_Int32 nIndex = GetIndex_Impl( aPropertyName );
    if ( nIndex != USHRT_MAX )
        return _aPropVals.GetObject(
            sal::static_int_cast< sal_uInt16 >(nIndex))->Value;
    return Any();
}

//----------------------------------------------------------------------------

void SbPropertyValues::addPropertyChangeListener(
                    const ::rtl::OUString& aPropertyName,
                    const Reference< XPropertyChangeListener >& )
                    throw ()
{
    (void)aPropertyName;
}

//----------------------------------------------------------------------------

void SbPropertyValues::removePropertyChangeListener(
                    const ::rtl::OUString& aPropertyName,
                    const Reference< XPropertyChangeListener >& )
                    throw ()
{
    (void)aPropertyName;
}

//----------------------------------------------------------------------------

void SbPropertyValues::addVetoableChangeListener(
                    const ::rtl::OUString& aPropertyName,
                    const Reference< XVetoableChangeListener >& )
                    throw()
{
    (void)aPropertyName;
}

//----------------------------------------------------------------------------

void SbPropertyValues::removeVetoableChangeListener(
                    const ::rtl::OUString& aPropertyName,
                    const Reference< XVetoableChangeListener >& )
                    throw()
{
    (void)aPropertyName;
}

//----------------------------------------------------------------------------

Sequence< PropertyValue > SbPropertyValues::getPropertyValues(void) throw (::com::sun::star::uno::RuntimeException)
{
    Sequence<PropertyValue> aRet( _aPropVals.Count());
    for ( sal_uInt16 n = 0; n < _aPropVals.Count(); ++n )
        aRet.getArray()[n] = *_aPropVals.GetObject(n);
    return aRet;
}

//----------------------------------------------------------------------------

void SbPropertyValues::setPropertyValues(const Sequence< PropertyValue >& rPropertyValues )
                     throw (::com::sun::star::beans::UnknownPropertyException,
                     ::com::sun::star::beans::PropertyVetoException,
                     ::com::sun::star::lang::IllegalArgumentException,
                     ::com::sun::star::lang::WrappedTargetException,
                     ::com::sun::star::uno::RuntimeException)
{
    if ( _aPropVals.Count() )
        throw PropertyExistException();

    const PropertyValue *pPropVals = rPropertyValues.getConstArray();
    for ( sal_Int16 n = 0; n < rPropertyValues.getLength(); ++n )
    {
        PropertyValue *pPropVal = new PropertyValue(pPropVals[n]);
        _aPropVals.Insert( pPropVal, n );
    }
}

//============================================================================
//PropertySetInfoImpl

PropertySetInfoImpl::PropertySetInfoImpl()
{
}

sal_Int32 PropertySetInfoImpl::GetIndex_Impl( const ::rtl::OUString &rPropName ) const
{
    Property *pP;
    pP = (Property*)
            bsearch( &rPropName, _aProps.getConstArray(), _aProps.getLength(),
                      sizeof( Property ),
                      SbCompare_UString_Property_Impl );
    return pP ? sal::static_int_cast<sal_Int32>( (pP-_aProps.getConstArray()) / sizeof(pP) ) : -1;
}

Sequence< Property > PropertySetInfoImpl::getProperties(void) throw()
{
    return _aProps;
}

Property PropertySetInfoImpl::getPropertyByName(const ::rtl::OUString& Name) throw( RuntimeException )
{
    sal_Int32 nIndex = GetIndex_Impl( Name );
    if( USHRT_MAX != nIndex )
        return _aProps.getConstArray()[ nIndex ];
    return Property();
}

sal_Bool PropertySetInfoImpl::hasPropertyByName(const ::rtl::OUString& Name) throw( RuntimeException )
{
    sal_Int32 nIndex = GetIndex_Impl( Name );
    return USHRT_MAX != nIndex;
}


//============================================================================

SbPropertySetInfo::SbPropertySetInfo()
{
}

//----------------------------------------------------------------------------

SbPropertySetInfo::SbPropertySetInfo( const SbPropertyValueArr_Impl &rPropVals )
{
    aImpl._aProps.realloc( rPropVals.Count() );
    for ( sal_uInt16 n = 0; n < rPropVals.Count(); ++n )
    {
        Property &rProp = aImpl._aProps.getArray()[n];
        const PropertyValue &rPropVal = *rPropVals.GetObject(n);
        rProp.Name = rPropVal.Name;
        rProp.Handle = rPropVal.Handle;
        rProp.Type = getCppuVoidType();
        rProp.Attributes = 0;
    }
}

//----------------------------------------------------------------------------

SbPropertySetInfo::~SbPropertySetInfo()
{
}

//-------------------------------------------------------------------------

Sequence< Property > SbPropertySetInfo::getProperties(void) throw( RuntimeException )
{
    return aImpl.getProperties();
}

Property SbPropertySetInfo::getPropertyByName(const ::rtl::OUString& Name)
    throw( RuntimeException )
{
    return aImpl.getPropertyByName( Name );
}

sal_Bool SbPropertySetInfo::hasPropertyByName(const ::rtl::OUString& Name)
    throw( RuntimeException )
{
    return aImpl.hasPropertyByName( Name );
}


//----------------------------------------------------------------------------

SbPropertyContainer::SbPropertyContainer()
{
}

//----------------------------------------------------------------------------

SbPropertyContainer::~SbPropertyContainer()
{
}

//----------------------------------------------------------------------------
void SbPropertyContainer::addProperty(const ::rtl::OUString& Name,
                                      sal_Int16 Attributes,
                                      const Any& DefaultValue)
    throw(  PropertyExistException, IllegalTypeException,
            IllegalArgumentException, RuntimeException )
{
    (void)Name;
    (void)Attributes;
    (void)DefaultValue;
}

//----------------------------------------------------------------------------
void SbPropertyContainer::removeProperty(const ::rtl::OUString& Name)
    throw( UnknownPropertyException, RuntimeException )
{
    (void)Name;
}

//----------------------------------------------------------------------------
// XPropertySetInfo
Sequence< Property > SbPropertyContainer::getProperties(void) throw ()
{
    return aImpl.getProperties();
}

Property SbPropertyContainer::getPropertyByName(const ::rtl::OUString& Name)
    throw( RuntimeException )
{
    return aImpl.getPropertyByName( Name );
}

sal_Bool SbPropertyContainer::hasPropertyByName(const ::rtl::OUString& Name)
    throw( RuntimeException )
{
    return aImpl.hasPropertyByName( Name );
}

//----------------------------------------------------------------------------

Sequence< PropertyValue > SbPropertyContainer::getPropertyValues(void)
{
    return Sequence<PropertyValue>();
}

//----------------------------------------------------------------------------

void SbPropertyContainer::setPropertyValues(const Sequence< PropertyValue >& PropertyValues_)
{
    (void)PropertyValues_;
}

//----------------------------------------------------------------------------

void RTL_Impl_CreatePropertySet( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    // We need at least one parameter
    // TODO: In this case < 2 is not correct ;-)
    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // Get class names of struct
    String aServiceName( RTL_CONSTASCII_USTRINGPARAM("stardiv.uno.beans.PropertySet") );

#if 0
    // Service suchen und instanzieren
    Reference< XMultiServiceFactory > xServiceManager = getProcessServiceFactory();
    Reference< XInterface > xInterface;
    if( xProv.is() )
        xInterface = xProv->newInstance();
#else
    Reference< XInterface > xInterface = (OWeakObject*) new SbPropertyValues();
#endif

    SbxVariableRef refVar = rPar.Get(0);
    if( xInterface.is() )
    {
        // Set PropertyValues
        Any aArgAsAny = sbxToUnoValue( rPar.Get(1),
                getCppuType( (Sequence<PropertyValue>*)0 ) );
        Sequence<PropertyValue> *pArg =
                (Sequence<PropertyValue>*) aArgAsAny.getValue();
        Reference< XPropertyAccess > xPropAcc = Reference< XPropertyAccess >::query( xInterface );
        xPropAcc->setPropertyValues( *pArg );

        // Build a SbUnoObject and return it
        Any aAny;
        aAny <<= xInterface;
        SbUnoObjectRef xUnoObj = new SbUnoObject( aServiceName, aAny );
        if( xUnoObj->getUnoAny().getValueType().getTypeClass() != TypeClass_VOID )
        {
            // Return object
            refVar->PutObject( (SbUnoObject*)xUnoObj );
            return;
        }
    }

    // Object could not be created
    refVar->PutObject( NULL );
}

