/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LayoutContainer.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:01:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "LayoutContainer.hxx"
#include "algohelper.hxx"
#include "macros.hxx"

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
    return ::chart::helper::VectorToSequence( m_aLayoutElements );
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
