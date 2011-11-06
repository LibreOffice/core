/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "LayoutContainer.hxx"
#include "macros.hxx"
#include "ContainerHelper.hxx"

#include <algorithm>

using namespace ::com::sun::star;

namespace
{

static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.layout.LayoutContainer" ));
} // anonymous namespace

namespace chart
{

LayoutContainer::LayoutContainer()
{}

LayoutContainer::~LayoutContainer()
{}

// ____ XLayoutContainer ____
void SAL_CALL LayoutContainer::addConstrainedElementByIdentifier(
    const ::rtl::OUString& aIdentifier,
    const layout::Constraint& Constraint )
    throw (layout::IllegalConstraintException,
           lang::IllegalArgumentException,
           uno::RuntimeException)
{
    addElementByIdentifier( aIdentifier );
    m_aConstraints[ aIdentifier ] = Constraint;
}

void SAL_CALL LayoutContainer::addElementByIdentifier( const ::rtl::OUString& aIdentifier )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    if( ::std::find( m_aLayoutElements.begin(),
                     m_aLayoutElements.end(),
                     aIdentifier ) != m_aLayoutElements.end())
        throw lang::IllegalArgumentException();

    m_aLayoutElements.push_back( aIdentifier );
}

void SAL_CALL LayoutContainer::removeElementByIdentifier( const ::rtl::OUString& aIdentifier )
    throw (container::NoSuchElementException,
           uno::RuntimeException)
{
    tLayoutElements::iterator aIt(
        ::std::find( m_aLayoutElements.begin(),
                     m_aLayoutElements.end(),
                     aIdentifier ));

    if( aIt == m_aLayoutElements.end())
        throw container::NoSuchElementException();

    m_aLayoutElements.erase( aIt );
    m_aConstraints.erase( aIdentifier );
}

void SAL_CALL LayoutContainer::setConstraintByIdentifier(
    const ::rtl::OUString& aIdentifier,
    const layout::Constraint& Constraint )
    throw (container::NoSuchElementException,
           uno::RuntimeException)
{
    if( ::std::find( m_aLayoutElements.begin(),
                     m_aLayoutElements.end(),
                     aIdentifier ) == m_aLayoutElements.end())
        throw container::NoSuchElementException();

    m_aConstraints[ aIdentifier ] = Constraint;
}

layout::Constraint SAL_CALL LayoutContainer::getConstraintByIdentifier( const ::rtl::OUString& aIdentifier )
    throw (container::NoSuchElementException,
           uno::RuntimeException)
{
    tConstraintsMap::const_iterator aIt( m_aConstraints.find( aIdentifier ));
    if( aIt == m_aConstraints.end())
        throw container::NoSuchElementException();

    return (*aIt).second;
}

uno::Sequence< ::rtl::OUString > SAL_CALL LayoutContainer::getElementIdentifiers()
    throw (uno::RuntimeException)
{
    return ContainerHelper::ContainerToSequence( m_aLayoutElements );
}

uno::Sequence< ::rtl::OUString > LayoutContainer::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 1 );

    aServices[ 0 ] = C2U( "com.sun.star.layout.LayoutContainer" );
    return aServices;
}

// --------------------------------------------------------------------------------

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( LayoutContainer, lcl_aServiceName );

} //  namespace chart
