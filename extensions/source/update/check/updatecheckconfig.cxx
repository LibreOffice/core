/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: updatecheckconfig.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-01 10:12:50 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"


#include "updatecheckconfig.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#include <osl/security.hxx>

namespace container = com::sun::star::container ;
namespace beans = com::sun::star::beans ;
namespace lang = com::sun::star::lang ;
namespace util = com::sun::star::util ;
namespace uno = com::sun::star::uno ;

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))


//------------------------------------------------------------------------------

UpdateCheckConfig::UpdateCheckConfig(const uno::Reference<uno::XComponentContext>& xContext)
{
    if( !xContext.is() )
        throw uno::RuntimeException(
            UNISTRING( "UpdateCheckConfig: empty component context" ), *this );

    uno::Reference< lang::XMultiComponentFactory > xServiceManager(xContext->getServiceManager());

    if( !xServiceManager.is() )
        throw uno::RuntimeException(
            UNISTRING( "UpdateCheckConfig: unable to obtain service manager from component context" ), *this );

    uno::Reference< lang::XMultiServiceFactory > xConfigProvider(
        xServiceManager->createInstanceWithContext( UNISTRING( "com.sun.star.configuration.ConfigurationProvider" ), xContext ),
        uno::UNO_QUERY_THROW);

    beans::PropertyValue aProperty;
    aProperty.Name  = UNISTRING( "nodepath" );
    aProperty.Value = uno::makeAny( UNISTRING("org.openoffice.Office.Jobs/Jobs/UpdateCheck/Arguments") );

    uno::Sequence< uno::Any > aArgumentList( 1 );
    aArgumentList[0] = uno::makeAny( aProperty );

    m_xUpdateAccess =  uno::Reference< container::XNameReplace > (
        xConfigProvider->createInstanceWithArguments(
            UNISTRING("com.sun.star.configuration.ConfigurationUpdateAccess"), aArgumentList ),
        uno::UNO_QUERY_THROW );
}

//------------------------------------------------------------------------------

UpdateCheckConfig::~UpdateCheckConfig()
{
    m_aListenerList.clear();
}

//------------------------------------------------------------------------------

uno::Sequence< rtl::OUString >
UpdateCheckConfig::getServiceNames()
{
    uno::Sequence< rtl::OUString > aServiceList(1);
    aServiceList[0] = UNISTRING( "com.sun.star.setup.UpdateCheckConfig");
    return aServiceList;
}

//------------------------------------------------------------------------------

rtl::OUString
UpdateCheckConfig::getImplName()
{
    return UNISTRING( "vnd.sun.UpdateCheckConfig");
}

//------------------------------------------------------------------------------

uno::Type SAL_CALL
UpdateCheckConfig::getElementType() throw (uno::RuntimeException)
{
    return m_xUpdateAccess->getElementType();
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL
UpdateCheckConfig::hasElements() throw (uno::RuntimeException)
{
    return m_xUpdateAccess->hasElements();
}

//------------------------------------------------------------------------------

uno::Any SAL_CALL
UpdateCheckConfig::getByName( const ::rtl::OUString& aName )
    throw (container::NoSuchElementException, lang::WrappedTargetException,  uno::RuntimeException)
{
    uno::Any aValue = m_xUpdateAccess->getByName( aName );

    // Provide dynamic default value
    if( aName.equalsAscii("DownloadDestination") )
    {
        rtl::OUString aStr;
        aValue >>= aStr;

        if( aStr.getLength() == 0 )
        {
            // This should become the desktop ..
            osl::Security().getHomeDir(aStr);
            aValue = uno::makeAny(aStr);
        }
    }

    return aValue;
}

//------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL
UpdateCheckConfig::getElementNames(  ) throw (uno::RuntimeException)
{
    return m_xUpdateAccess->getElementNames();
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL
UpdateCheckConfig::hasByName( const ::rtl::OUString& aName ) throw (uno::RuntimeException)
{
    return m_xUpdateAccess->hasByName( aName );
}

//------------------------------------------------------------------------------

void SAL_CALL
UpdateCheckConfig::replaceByName( const ::rtl::OUString& aName, const uno::Any& aElement )
    throw (lang::IllegalArgumentException, container::NoSuchElementException,
           lang::WrappedTargetException, uno::RuntimeException)
{
    return m_xUpdateAccess->replaceByName( aName, aElement );
}

//------------------------------------------------------------------------------

    // XChangesBatch
void SAL_CALL
UpdateCheckConfig::commitChanges(  )
    throw (lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Reference< util::XChangesBatch > xChangesBatch(m_xUpdateAccess, uno::UNO_QUERY);
    if( xChangesBatch.is() && xChangesBatch->hasPendingChanges() )
    {
        util::ChangesEvent aChangesEvt;
        aChangesEvt.Changes = xChangesBatch->getPendingChanges();
        xChangesBatch->commitChanges();

        ::std::list< uno::Reference< util::XChangesListener > >::iterator it;
        for( it = m_aListenerList.begin(); it != m_aListenerList.end(); ++it )
        {
            (*it)->changesOccurred( aChangesEvt );
        }
    }
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL
UpdateCheckConfig::hasPendingChanges(  ) throw (uno::RuntimeException)
{
    uno::Reference< util::XChangesBatch > xChangesBatch(m_xUpdateAccess, uno::UNO_QUERY);
    if( xChangesBatch.is() )
        return xChangesBatch->hasPendingChanges();

    return sal_False;
}

//------------------------------------------------------------------------------

uno::Sequence< util::ElementChange > SAL_CALL
UpdateCheckConfig::getPendingChanges(  ) throw (uno::RuntimeException)
{
    uno::Reference< util::XChangesBatch > xChangesBatch(m_xUpdateAccess, uno::UNO_QUERY);
    if( xChangesBatch.is() )
        return xChangesBatch->getPendingChanges();

    return uno::Sequence< util::ElementChange >();
}


//------------------------------------------------------------------------------
// XChangesNotifier

void SAL_CALL
UpdateCheckConfig::addChangesListener( const uno::Reference< util::XChangesListener >& aListener )
        throw ( uno::RuntimeException )
{
    m_aListenerList.push_back( aListener );
}

//------------------------------------------------------------------------------

void SAL_CALL
UpdateCheckConfig::removeChangesListener( const uno::Reference< util::XChangesListener >& aListener )
        throw ( uno::RuntimeException )
{
    m_aListenerList.remove( aListener );
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL
UpdateCheckConfig::getImplementationName()  throw (uno::RuntimeException)
{
    return getImplName();
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL
UpdateCheckConfig::supportsService(rtl::OUString const & serviceName)
    throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > aServiceNameList = getServiceNames();

    for( sal_Int32 n=0; n < aServiceNameList.getLength(); n++ )
        if( aServiceNameList[n].equals(serviceName) )
            return sal_True;

    return sal_False;
}

//------------------------------------------------------------------------------

uno::Sequence< rtl::OUString > SAL_CALL
UpdateCheckConfig::getSupportedServiceNames()  throw (uno::RuntimeException)
{
    return getServiceNames();
}

