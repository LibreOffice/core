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


#include <propacc.hxx>

#include <basic/sberrors.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbuno.hxx>
#include <sbunoobj.hxx>

#include <comphelper/propertysetinfo.hxx>
#include <comphelper/sequence.hxx>
#include <o3tl/any.hxx>

#include <algorithm>
#include <limits.h>

using com::sun::star::uno::Reference;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace cppu;

static bool SbCompare_UString_PropertyValue_Impl(PropertyValue const & lhs, const OUString& rhs)
{
    return lhs.Name.compareTo(rhs) < 0;
}


SbPropertyValues::SbPropertyValues()
{
}


SbPropertyValues::~SbPropertyValues()
{
    m_xInfo.clear();
}

Reference< XPropertySetInfo > SbPropertyValues::getPropertySetInfo()
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
          SbCompare_UString_PropertyValue_Impl );
    if (it == m_aPropVals.end() || it->Name != rPropName)
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
{
    size_t const nIndex = GetIndex_Impl( aPropertyName );
    PropertyValue & rPropVal = m_aPropVals[nIndex];
    rPropVal.Value = aValue;
}


Any SbPropertyValues::getPropertyValue(
                    const OUString& aPropertyName)
{
    size_t const nIndex = GetIndex_Impl( aPropertyName );
    return m_aPropVals[nIndex].Value;
}


void SbPropertyValues::addPropertyChangeListener(
                    const OUString&,
                    const Reference< XPropertyChangeListener >& )
{}


void SbPropertyValues::removePropertyChangeListener(
                    const OUString&,
                    const Reference< XPropertyChangeListener >& )
{}


void SbPropertyValues::addVetoableChangeListener(
                    const OUString&,
                    const Reference< XVetoableChangeListener >& )
{}


void SbPropertyValues::removeVetoableChangeListener(
                    const OUString&,
                    const Reference< XVetoableChangeListener >& )
{}


Sequence< PropertyValue > SbPropertyValues::getPropertyValues()
{
    return comphelper::containerToSequence(m_aPropVals);
}


void SbPropertyValues::setPropertyValues(const Sequence< PropertyValue >& rPropertyValues )
{
    if (!m_aPropVals.empty())
        throw IllegalArgumentException();

    const PropertyValue *pPropVals = rPropertyValues.getConstArray();
    for (sal_Int32 n = 0; n < rPropertyValues.getLength(); ++n)
    {
        m_aPropVals.push_back(pPropVals[n]);
    }
}


void RTL_Impl_CreatePropertySet( SbxArray& rPar )
{
    // We need at least one parameter
    // TODO: In this case < 2 is not correct ;-)
    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    // Get class names of struct

    Reference< XInterface > xInterface = static_cast<OWeakObject*>(new SbPropertyValues());

    SbxVariableRef refVar = rPar.Get(0);
    if( xInterface.is() )
    {
        // Set PropertyValues
        Any aArgAsAny = sbxToUnoValue( rPar.Get(1),
                cppu::UnoType<Sequence<PropertyValue>>::get() );
        auto pArg = o3tl::doAccess<Sequence<PropertyValue>>(aArgAsAny);
        Reference< XPropertyAccess > xPropAcc( xInterface, UNO_QUERY );
        xPropAcc->setPropertyValues( *pArg );

        // Build a SbUnoObject and return it
        auto xUnoObj = tools::make_ref<SbUnoObject>( "stardiv.uno.beans.PropertySet", Any(xInterface) );
        if( xUnoObj->getUnoAny().hasValue() )
        {
            // Return object
            refVar->PutObject( xUnoObj.get() );
            return;
        }
    }

    // Object could not be created
    refVar->PutObject( nullptr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
