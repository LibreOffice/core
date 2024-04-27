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

#include <CachedDataSequence.hxx>
#include <CommonFunctors.hxx>
#include <ModifyListenerHelper.hxx>

#include <comphelper/sequenceashashmap.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

// necessary for MS compiler
using ::chart::impl::CachedDataSequence_Base;

namespace
{
constexpr OUString lcl_aServiceName = u"com.sun.star.comp.chart.CachedDataSequence"_ustr;

enum
{
//     PROP_SOURCE_IDENTIFIER,
    PROP_NUMBERFORMAT_KEY,
    PROP_PROPOSED_ROLE
};
}  // anonymous namespace

namespace chart
{

CachedDataSequence::CachedDataSequence()
        : m_eCurrentDataType( NUMERICAL ),
          m_xModifyEventForwarder( new ModifyEventForwarder() )
{
    registerProperties();
}
CachedDataSequence::CachedDataSequence( const Reference< uno::XComponentContext > & /*xContext*/ )
        : m_eCurrentDataType( MIXED ),
          m_xModifyEventForwarder( new ModifyEventForwarder() )
{
    registerProperties();
}

CachedDataSequence::CachedDataSequence( const OUString & rSingleText )
        : m_eCurrentDataType( TEXTUAL ),
          m_aTextualSequence({rSingleText}),
          m_xModifyEventForwarder( new ModifyEventForwarder() )
{
    registerProperties();
}

CachedDataSequence::CachedDataSequence( const CachedDataSequence & rSource )
        : m_nNumberFormatKey( rSource.m_nNumberFormatKey ),
          m_sRole( rSource.m_sRole ),
          m_eCurrentDataType( rSource.m_eCurrentDataType ),
          m_xModifyEventForwarder( new ModifyEventForwarder() )
{
    switch( m_eCurrentDataType )
    {
        case TEXTUAL:
            m_aTextualSequence = rSource.m_aTextualSequence;
            break;
        case NUMERICAL:
            m_aNumericalSequence = rSource.m_aNumericalSequence;
            break;
        case MIXED:
            m_aMixedSequence = rSource.m_aMixedSequence;
            break;
    }

    registerProperties();
}

CachedDataSequence::~CachedDataSequence()
{}

void CachedDataSequence::registerProperties()
{
    registerProperty( "NumberFormatKey",
                      PROP_NUMBERFORMAT_KEY,
                      0,   // PropertyAttributes
                      & m_nNumberFormatKey,
                      cppu::UnoType<decltype(m_nNumberFormatKey)>::get() );

    registerProperty( "Role",
                      PROP_PROPOSED_ROLE,
                      0,   // PropertyAttributes
                      & m_sRole,
                      cppu::UnoType<decltype(m_sRole)>::get() );
}

Sequence< double > CachedDataSequence::Impl_getNumericalData() const
{
    if( m_eCurrentDataType == NUMERICAL )
        return m_aNumericalSequence;

    if( m_eCurrentDataType == TEXTUAL )
        return CommonFunctors::convertToSequence(m_aTextualSequence, CommonFunctors::ToDouble());

    OSL_ASSERT(m_eCurrentDataType == MIXED);
    return CommonFunctors::convertToSequence(m_aMixedSequence, CommonFunctors::ToDouble());
}

Sequence< OUString > CachedDataSequence::Impl_getTextualData() const
{
    if( m_eCurrentDataType == TEXTUAL )
        return m_aTextualSequence;

    if( m_eCurrentDataType == NUMERICAL )
        return CommonFunctors::convertToSequence(m_aNumericalSequence, CommonFunctors::ToString());

    OSL_ASSERT(m_eCurrentDataType == MIXED);
    return CommonFunctors::convertToSequence(m_aMixedSequence, CommonFunctors::ToString());
}

Sequence< Any > CachedDataSequence::Impl_getMixedData() const
{
    if( m_eCurrentDataType == MIXED )
        return m_aMixedSequence;

    if( m_eCurrentDataType == NUMERICAL )
        return CommonFunctors::convertToSequence(m_aNumericalSequence, CommonFunctors::makeAny());

    OSL_ASSERT(m_eCurrentDataType == TEXTUAL);
    return CommonFunctors::convertToSequence(m_aTextualSequence, CommonFunctors::makeAny());
}

IMPLEMENT_FORWARD_XINTERFACE2( CachedDataSequence, CachedDataSequence_Base, comphelper::OPropertyContainer2 )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( CachedDataSequence, CachedDataSequence_Base, comphelper::OPropertyContainer2 )

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL CachedDataSequence::getPropertySetInfo()
{
    return createPropertySetInfo( getInfoHelper() );
}

// ____ ::comphelper::OPropertySetHelper ____
::cppu::IPropertyArrayHelper& CachedDataSequence::getInfoHelper()
{
    return *getArrayHelper();
}

// ____ ::comphelper::OPropertyArrayHelper ____
::cppu::IPropertyArrayHelper* CachedDataSequence::createArrayHelper() const
{
    Sequence< beans::Property > aProps;
    // describes all properties which have been registered in the ctor
    describeProperties( aProps );

    return new ::cppu::OPropertyArrayHelper( aProps );
}

OUString SAL_CALL CachedDataSequence::getImplementationName()
{
    return lcl_aServiceName;
}

sal_Bool SAL_CALL CachedDataSequence::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL CachedDataSequence::getSupportedServiceNames()
{
    return {
        lcl_aServiceName,
        "com.sun.star.chart2.data.DataSequence",
        "com.sun.star.chart2.data.NumericalDataSequence",
        "com.sun.star.chart2.data.TextualDataSequence"
    };
}

// ________ XNumericalDataSequence ________
Sequence< double > SAL_CALL CachedDataSequence::getNumericalData()
{
    std::unique_lock aGuard( m_aMutex );

    if( m_eCurrentDataType == NUMERICAL )
        return m_aNumericalSequence;
    else
        return Impl_getNumericalData();
}

// ________ XTextualDataSequence ________
Sequence< OUString > SAL_CALL CachedDataSequence::getTextualData()
{
    std::unique_lock aGuard( m_aMutex );

    if( m_eCurrentDataType == TEXTUAL )
        return m_aTextualSequence;
    else
        return Impl_getTextualData();
}

// ________ XDataSequence  ________
Sequence< Any > SAL_CALL CachedDataSequence::getData()
{
    std::unique_lock aGuard( m_aMutex );
    return Impl_getMixedData();
}

OUString SAL_CALL CachedDataSequence::getSourceRangeRepresentation()
{
    return m_sRole;
}

Sequence< OUString > SAL_CALL CachedDataSequence::generateLabel( chart2::data::LabelOrigin  /*eLabelOrigin*/ )
{
    // return empty label, as we have no range representations to determine something useful
    return Sequence< OUString >();
}

::sal_Int32 SAL_CALL CachedDataSequence::getNumberFormatKeyByIndex( ::sal_Int32 /*nIndex*/ )
{
    return 0;
}

Reference< util::XCloneable > SAL_CALL CachedDataSequence::createClone()
{
    return new CachedDataSequence( *this );
}

void SAL_CALL CachedDataSequence::addModifyListener( const Reference< util::XModifyListener >& aListener )
{
    m_xModifyEventForwarder->addModifyListener( aListener );
}

void SAL_CALL CachedDataSequence::removeModifyListener( const Reference< util::XModifyListener >& aListener )
{
    m_xModifyEventForwarder->removeModifyListener( aListener );
}

// lang::XInitialization:
void SAL_CALL CachedDataSequence::initialize(const uno::Sequence< uno::Any > & _aArguments)
{
    ::comphelper::SequenceAsHashMap aMap(_aArguments);
    m_aNumericalSequence = aMap.getUnpackedValueOrDefault( "DataSequence" ,m_aNumericalSequence);
    if ( m_aNumericalSequence.hasElements() )
        m_eCurrentDataType = NUMERICAL;
    else
    {
        m_aTextualSequence = aMap.getUnpackedValueOrDefault( "DataSequence" ,m_aTextualSequence);
        if ( m_aTextualSequence.hasElements() )
            m_eCurrentDataType = TEXTUAL;
        else
        {
            m_aMixedSequence = aMap.getUnpackedValueOrDefault( "DataSequence" ,m_aMixedSequence);
            if ( m_aMixedSequence.hasElements() )
                m_eCurrentDataType = MIXED;
        }
    }
}
}  // namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart_CachedDataSequence_get_implementation(css::uno::XComponentContext *context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::CachedDataSequence(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
