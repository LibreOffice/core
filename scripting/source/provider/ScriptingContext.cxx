/*************************************************************************
 *
 *  $RCSfile: ScriptingContext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jmrice $ $Date: 2002-09-27 12:16:25 $
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

#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/factory.hxx>

#include <util/scriptingconstants.hxx>
#include <util/util.hxx>

#include "ScriptingContext.hxx"

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace func_provider
{

//*************************************************************************
// XScriptingContext implementation
//
//*************************************************************************
ScriptingContext::ScriptingContext( const Reference< XComponentContext > & xContext ) :
        m_xContext( xContext )
{
    OSL_TRACE( "< ScriptingContext ctor called >\n" );

    validateXRef( m_xContext,
                  "ScriptingContext::ScriptingContext: No context available\n" );

    //Setup internal hash map
    Any nullAny;

    m_propertyMap[ scripting_constants::DOC_REF ] = nullAny;
    m_propertyMap[ scripting_constants::DOC_STORAGE_ID ] = nullAny;
    m_propertyMap[ scripting_constants::DOC_URI ] = nullAny;
    m_propertyMap[ scripting_constants::RESOLVED_STORAGE_ID ] = nullAny;
}

//*************************************************************************
bool ScriptingContext::validateKey( const ::rtl::OUString& key )
{
    ::osl::Guard< osl::Mutex > aGuard( m_mutex );
    return ( m_propertyMap.find( key ) != m_propertyMap.end() );
}

//*************************************************************************
ScriptingContext::~ScriptingContext()
{
    OSL_TRACE( "< ScriptingContext dtor called >\n" );
}

//*************************************************************************
// XPropertySet implementation
//*************************************************************************
Reference< beans::XPropertySetInfo > SAL_CALL ScriptingContext::getPropertySetInfo( )
    throw ( RuntimeException )
{
    return Reference< beans::XPropertySetInfo > (); // Not supported
}

//*************************************************************************
void SAL_CALL ScriptingContext::setPropertyValue( const ::rtl::OUString& aPropertyName,
    const Any& aValue )
    throw ( beans::UnknownPropertyException, beans::PropertyVetoException,
            lang::IllegalArgumentException, lang::WrappedTargetException,
            RuntimeException )
{
    if ( !validateKey( aPropertyName ) )
    {
        throw RuntimeException(
            OUSTR( "ScriptingContext::setPropertyValue: invalid key" ),
            Reference< XInterface >() );
    }
    ::osl::Guard< osl::Mutex > aGuard( m_mutex );
    m_propertyMap[ aPropertyName ] = aValue;
}

//*************************************************************************
Any SAL_CALL ScriptingContext::getPropertyValue( const ::rtl::OUString& PropertyName )
    throw ( beans::UnknownPropertyException,
            lang::WrappedTargetException, RuntimeException )
{
    if ( !validateKey( PropertyName ) )
    {
        throw RuntimeException(
            OUSTR( "ScriptingContext::getPropertyValue: invalid key" ),
            Reference< XInterface >() );
    }

    ::osl::Guard< osl::Mutex > aGuard( m_mutex );
    Any returnValue = m_propertyMap[ PropertyName ];

    return returnValue;
}

//*************************************************************************
void SAL_CALL ScriptingContext::addPropertyChangeListener(
    const ::rtl::OUString& aPropertyName,
    const Reference< beans::XPropertyChangeListener >& xListener )
    throw ( beans::UnknownPropertyException, lang::WrappedTargetException,
            RuntimeException )
{
    throw RuntimeException(
        OUSTR( "ScriptingContext::addPropertyChangeListener: method not supported" ),
        Reference< XInterface >() );
}

//*************************************************************************
void SAL_CALL ScriptingContext::removePropertyChangeListener(
    const ::rtl::OUString& aPropertyName,
    const Reference< beans::XPropertyChangeListener >& aListener )
    throw ( beans::UnknownPropertyException, lang::WrappedTargetException,
            RuntimeException )
{
    throw RuntimeException(
        OUSTR( "ScriptingContext::removePropertyChangeListener: method not supported" ),
        Reference< XInterface >() );
}

//*************************************************************************
void SAL_CALL ScriptingContext::addVetoableChangeListener(
    const ::rtl::OUString& PropertyName,
    const Reference< beans::XVetoableChangeListener >& aListener )
    throw ( beans::UnknownPropertyException, lang::WrappedTargetException,
            RuntimeException )
{
    throw RuntimeException(
        OUSTR( "ScriptingContext::addVetoableChangeListener: method not supported" ),
        Reference< XInterface >() );
}

//*************************************************************************
void SAL_CALL ScriptingContext::removeVetoableChangeListener(
    const ::rtl::OUString& PropertyName,
    const Reference< beans::XVetoableChangeListener >& aListener )
    throw ( beans::UnknownPropertyException, lang::WrappedTargetException,
            RuntimeException )
{
    throw RuntimeException(
        OUSTR( "ScriptingContext::removeVetoableChangeListener: method not supported" ),
        Reference< XInterface >() );
}

} // namespace func_provider
