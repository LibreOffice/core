/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Style.cxx,v $
 * $Revision: 1.4 $
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
#include "precompiled_chart2.hxx"
#include "Style.hxx"
#include "macros.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::rtl::OUString;
using ::osl::MutexGuard;

// necessary for MS compiler
using ::comphelper::OPropertyContainer;
using ::chart::impl::Style_Base;

namespace
{
static const ::rtl::OUString lcl_aImplName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.Style" ));
}  // anonymous namespace

namespace chart
{

Style::Style( const Reference< container::XNameContainer > & xStyleFamiliyToAddTo ) :
        OPropertyContainer( GetBroadcastHelper()),
        Style_Base( GetMutex()),
        m_xStyleFamily( xStyleFamiliyToAddTo ),
        m_aName( C2U( "Default" ) ),
        m_bUserDefined( sal_False )
{
    OSL_ENSURE( m_xStyleFamily.is(), "No StyleFamily to add style to" );
}

Style::~Style()
{}

sal_Bool SAL_CALL Style::isUserDefined()
    throw (RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    return m_bUserDefined;
    // \--
}

sal_Bool SAL_CALL Style::isInUse()
    throw (RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );

    // aBoundLC is a member of cppuhelper::OPropertySetHelper
    // it is assumed that a style is in use whenever some component is
    // registered here as listener
    return ( aBoundLC.getContainedTypes().getLength() > 0 );
    // \--
}

OUString SAL_CALL Style::getParentStyle()
    throw (RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    return m_aParentStyleName;
    // \--
}

void SAL_CALL Style::setParentStyle( const OUString& aParentStyle )
    throw (container::NoSuchElementException,
           RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    m_aParentStyleName = aParentStyle;
    // \--
}


OUString SAL_CALL Style::getName()
    throw (RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    return m_aName;
    // \--
}

void SAL_CALL Style::setName( const OUString& aName )
    throw (RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    OSL_ASSERT( m_xStyleFamily.is());

    if( m_xStyleFamily.is() )
    {
        // remove old name
        if( m_aName.getLength() > 0 )
        {
            Reference< container::XNameAccess > xAccess( m_xStyleFamily, uno::UNO_QUERY );
            OSL_ASSERT( xAccess.is());
            if( xAccess->hasByName( m_aName ))
                m_xStyleFamily->removeByName( m_aName );
        }

        // change name
        m_aName = aName;

        // add new name
        m_xStyleFamily->insertByName( m_aName, uno::makeAny( this ));
    }
    // \--
}

IMPLEMENT_FORWARD_XINTERFACE2( Style, Style_Base, OPropertyContainer )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( Style, Style_Base, OPropertyContainer )

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( Style, lcl_aImplName )

uno::Sequence< OUString > Style::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aServices( 1 );
    aServices[ 0 ] = C2U( "com.sun.star.style.Style" );
    return aServices;
}

} //  namespace chart
