/*************************************************************************
 *
 *  $RCSfile: basmethnode.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: toconnor $ $Date: 2003-10-29 15:00:44 $
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

#ifndef SCRIPTING_BASMETHNODE_HXX
#include "basmethnode.hxx"
#endif

#ifndef _DRAFTS_COM_SUN_STAR_SCRIPT_BROWSE_BROWSENODETYPES_HPP_
#include <drafts/com/sun/star/script/browse/BrowseNodeTypes.hpp>
#endif

#ifndef _SB_SBSTAR_HXX
#include <basic/sbstar.hxx>
#endif
#ifndef _SB_SBMETH_HXX
#include <basic/sbmeth.hxx>
#endif
#ifndef _SB_SBMOD_HXX
#include <basic/sbmod.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::script;


//.........................................................................
namespace basprov
{
//.........................................................................

    // =============================================================================
    // BasicMethodNodeImpl
    // =============================================================================

    BasicMethodNodeImpl::BasicMethodNodeImpl( SbMethod* pMethod, bool isAppScript )
        :m_pMethod( pMethod ), m_bIsAppScript( isAppScript )
    {
        String sModName = m_pMethod->GetModule()->GetName();
        String sLibName = m_pMethod->GetModule()->GetParent()->GetName();
        String sMethName = m_pMethod->GetName();
        String sTmp = String::CreateFromAscii("vnd.sun.star.script://");
        sTmp += sLibName;
        sTmp += '.';
        sTmp += sModName;
        sTmp += '.';
        sTmp += sMethName;
        sTmp += String::CreateFromAscii("?language=Basic&location=");
        if ( m_bIsAppScript )
        {
            sTmp += String::CreateFromAscii("application");
        }
        else
        {
            sTmp += String::CreateFromAscii("document");
        }
        rtl::OUString sUrl( sTmp );
        try
        {
            Any aURIVal;
            aURIVal <<= sUrl;
            setPropertyValue( ::rtl::OUString::createFromAscii("URI"), aURIVal);
        }
        catch ( RuntimeException& re )
        {
            OSL_TRACE("caught exception trying to set property %s",
               ::rtl::OUStringToOString( re.Message, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }
    }

    // -----------------------------------------------------------------------------

    BasicMethodNodeImpl::~BasicMethodNodeImpl()
    {
    }

    // -----------------------------------------------------------------------------
    // XBrowseNode
    // -----------------------------------------------------------------------------

    ::rtl::OUString BasicMethodNodeImpl::getName(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( StarBASIC::GetGlobalMutex() );

        ::rtl::OUString sMethodName;
        if ( m_pMethod )
            sMethodName = m_pMethod->GetName();

        return sMethodName;
    }

    // -----------------------------------------------------------------------------

    Sequence< Reference< browse::XBrowseNode > > BasicMethodNodeImpl::getChildNodes(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( StarBASIC::GetGlobalMutex() );

        return Sequence< Reference< browse::XBrowseNode > >();
    }

    // -----------------------------------------------------------------------------

    sal_Bool BasicMethodNodeImpl::hasChildNodes(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( StarBASIC::GetGlobalMutex() );

        return sal_False;
    }

    // -----------------------------------------------------------------------------

    sal_Int16 BasicMethodNodeImpl::getType(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( StarBASIC::GetGlobalMutex() );

        return browse::BrowseNodeTypes::SCRIPT;
    }

    // -----------------------------------------------------------------------------


//*************************************************************************
// XPropertySet implementation
//*************************************************************************
Reference< beans::XPropertySetInfo > SAL_CALL BasicMethodNodeImpl::getPropertySetInfo( )
    throw ( RuntimeException )
{
    return Reference< beans::XPropertySetInfo > (); // Not supported
}

//*************************************************************************
void SAL_CALL BasicMethodNodeImpl::setPropertyValue( const ::rtl::OUString& aPropertyName,
    const Any& aValue )
    throw ( beans::UnknownPropertyException, beans::PropertyVetoException,
            lang::IllegalArgumentException, lang::WrappedTargetException,
            RuntimeException )
{
    m_hProps[ aPropertyName ] = aValue;
}

//*************************************************************************
Any SAL_CALL BasicMethodNodeImpl::getPropertyValue( const ::rtl::OUString& PropertyName )
    throw ( beans::UnknownPropertyException,
            lang::WrappedTargetException, RuntimeException )
{
    Any returnValue = m_hProps[ PropertyName ];

    return returnValue;
}

//*************************************************************************
void SAL_CALL BasicMethodNodeImpl::addPropertyChangeListener(
    const ::rtl::OUString& aPropertyName,
    const Reference< beans::XPropertyChangeListener >& xListener )
    throw ( beans::UnknownPropertyException, lang::WrappedTargetException,
            RuntimeException )
{
    throw RuntimeException(
        ::rtl::OUString::createFromAscii( "BasicMethodNodeImpl::addPropertyChangeListener: method not supported" ),
        Reference< XInterface >() );
}

//*************************************************************************
void SAL_CALL BasicMethodNodeImpl::removePropertyChangeListener(
    const ::rtl::OUString& aPropertyName,
    const Reference< beans::XPropertyChangeListener >& aListener )
    throw ( beans::UnknownPropertyException, lang::WrappedTargetException,
            RuntimeException )
{
    throw RuntimeException(
        ::rtl::OUString::createFromAscii( "BasicMethodNodeImpl::removePropertyChangeListener: method not supported" ),
        Reference< XInterface >() );
}

//*************************************************************************
void SAL_CALL BasicMethodNodeImpl::addVetoableChangeListener(
    const ::rtl::OUString& PropertyName,
    const Reference< beans::XVetoableChangeListener >& aListener )
    throw ( beans::UnknownPropertyException, lang::WrappedTargetException,
            RuntimeException )
{
    throw RuntimeException(
        ::rtl::OUString::createFromAscii( "BasicMethodNodeImpl::addVetoableChangeListener: method not supported" ),
        Reference< XInterface >() );
}

//*************************************************************************
void SAL_CALL BasicMethodNodeImpl::removeVetoableChangeListener(
    const ::rtl::OUString& PropertyName,
    const Reference< beans::XVetoableChangeListener >& aListener )
    throw ( beans::UnknownPropertyException, lang::WrappedTargetException,
            RuntimeException )
{
    throw RuntimeException(
        ::rtl::OUString::createFromAscii( "BasicMethodNodeImpl::removeVetoableChangeListener: method not supported" ),
        Reference< XInterface >() );
}


//.........................................................................
}   // namespace basprov
//.........................................................................
