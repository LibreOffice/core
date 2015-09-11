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


#include "propacc.hxx"

#include <basic/sbstar.hxx>
#include <basic/sbuno.hxx>
#include <sbunoobj.hxx>

#include <comphelper/propertysetinfo.hxx>

#include <limits.h>

using com::sun::star::uno::Reference;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace cppu;

struct SbCompare_UString_PropertyValue_Impl
{
   bool operator() (PropertyValue const & lhs, const OUString& rhs)
   {
      return lhs.Name.compareTo(rhs) < 0;
   }
};


SbPropertyValues::SbPropertyValues()
{
}



SbPropertyValues::~SbPropertyValues()
{
    m_xInfo.clear();
}

Reference< XPropertySetInfo > SbPropertyValues::getPropertySetInfo() throw( RuntimeException, std::exception )
{
    // create on demand?
    if (!m_xInfo.is())
    {
        uno::Sequence<beans::Property> props(m_aPropVals.size());
        for (size_t n = 0; n < m_aPropVals.size(); ++n)
        {
            Property &rProp = props.getArray()[n];
            const PropertyValue &rPropVal = m_aPropVals[n];
            rProp.Name = rPropVal.Name;
            rProp.Handle = rPropVal.Handle;
            rProp.Type = cppu::UnoType<void>::get();
            rProp.Attributes = 0;
        }
        m_xInfo.set(new ::comphelper::PropertySetInfo(props));
    }
    return m_xInfo;
}



size_t SbPropertyValues::GetIndex_Impl( const OUString &rPropName ) const
{
    SbPropertyValueArr_Impl::const_iterator it = std::lower_bound(
          m_aPropVals.begin(), m_aPropVals.end(), rPropName,
          SbCompare_UString_PropertyValue_Impl() );
    if (it == m_aPropVals.end())
    {
        throw beans::UnknownPropertyException(
                "Property not found: " + rPropName,
                const_cast<SbPropertyValues&>(*this));
    }
    return it - m_aPropVals.begin();
}



void SbPropertyValues::setPropertyValue(
                    const OUString& aPropertyName,
                    const Any& aValue)
                    throw (css::beans::UnknownPropertyException,
                    css::beans::PropertyVetoException,
                    css::lang::IllegalArgumentException,
                    css::lang::WrappedTargetException,
                    css::uno::RuntimeException, std::exception)
{
    size_t const nIndex = GetIndex_Impl( aPropertyName );
    PropertyValue & rPropVal = m_aPropVals[nIndex];
    rPropVal.Value = aValue;
}



Any SbPropertyValues::getPropertyValue(
                    const OUString& aPropertyName)
                    throw(css::beans::UnknownPropertyException,
                    css::lang::WrappedTargetException,
                    css::uno::RuntimeException, std::exception)
{
    size_t const nIndex = GetIndex_Impl( aPropertyName );
    return m_aPropVals[nIndex].Value;
}



void SbPropertyValues::addPropertyChangeListener(
                    const OUString& aPropertyName,
                    const Reference< XPropertyChangeListener >& )
                    throw (std::exception)
{
    (void)aPropertyName;
}



void SbPropertyValues::removePropertyChangeListener(
                    const OUString& aPropertyName,
                    const Reference< XPropertyChangeListener >& )
                    throw (std::exception)
{
    (void)aPropertyName;
}



void SbPropertyValues::addVetoableChangeListener(
                    const OUString& aPropertyName,
                    const Reference< XVetoableChangeListener >& )
                    throw(std::exception)
{
    (void)aPropertyName;
}



void SbPropertyValues::removeVetoableChangeListener(
                    const OUString& aPropertyName,
                    const Reference< XVetoableChangeListener >& )
                    throw(std::exception)
{
    (void)aPropertyName;
}



Sequence< PropertyValue > SbPropertyValues::getPropertyValues() throw (css::uno::RuntimeException, std::exception)
{
    Sequence<PropertyValue> aRet( m_aPropVals.size() );
    for (size_t n = 0; n < m_aPropVals.size(); ++n)
        aRet.getArray()[n] = m_aPropVals[n];
    return aRet;
}



void SbPropertyValues::setPropertyValues(const Sequence< PropertyValue >& rPropertyValues )
                     throw (css::beans::UnknownPropertyException,
                     css::beans::PropertyVetoException,
                     css::lang::IllegalArgumentException,
                     css::lang::WrappedTargetException,
                     css::uno::RuntimeException, std::exception)
{
    if ( !m_aPropVals.empty() )
        throw IllegalArgumentException();

    const PropertyValue *pPropVals = rPropertyValues.getConstArray();
    for (sal_Int32 n = 0; n < rPropertyValues.getLength(); ++n)
    {
        PropertyValue *pPropVal = new PropertyValue(pPropVals[n]);
        m_aPropVals.push_back( pPropVal );
    }
}


void RTL_Impl_CreatePropertySet( StarBASIC* pBasic, SbxArray& rPar, bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    // We need at least one parameter
    // TODO: In this case < 2 is not correct ;-)
    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    // Get class names of struct
    OUString aServiceName( "stardiv.uno.beans.PropertySet");

    Reference< XInterface > xInterface = static_cast<OWeakObject*>(new SbPropertyValues());

    SbxVariableRef refVar = rPar.Get(0);
    if( xInterface.is() )
    {
        // Set PropertyValues
        Any aArgAsAny = sbxToUnoValue( rPar.Get(1),
                cppu::UnoType<Sequence<PropertyValue>>::get() );
        Sequence<PropertyValue> const *pArg =
                static_cast<Sequence<PropertyValue> const *>(aArgAsAny.getValue());
        Reference< XPropertyAccess > xPropAcc = Reference< XPropertyAccess >::query( xInterface );
        xPropAcc->setPropertyValues( *pArg );

        // Build a SbUnoObject and return it
        Any aAny;
        aAny <<= xInterface;
        SbUnoObjectRef xUnoObj = new SbUnoObject( aServiceName, aAny );
        if( xUnoObj->getUnoAny().getValueType().getTypeClass() != TypeClass_VOID )
        {
            // Return object
            refVar->PutObject( static_cast<SbUnoObject*>(xUnoObj) );
            return;
        }
    }

    // Object could not be created
    refVar->PutObject( NULL );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
