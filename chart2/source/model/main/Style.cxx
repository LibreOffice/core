/*************************************************************************
 *
 *  $RCSfile: Style.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:31 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "Style.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

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
