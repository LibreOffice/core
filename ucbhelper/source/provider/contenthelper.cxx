/*************************************************************************
 *
 *  $RCSfile: contenthelper.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:37 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef __HASH_MAP__
#include <stl/hash_map>
#endif
#ifndef _COM_SUN_STAR_UCB_CONTENTACTION_HPP_
#include <com/sun/star/ucb/ContentAction.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDINFOCHANGE_HPP_
#include <com/sun/star/ucb/CommandInfoChange.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPERSISTENTPROPERTYSET_HPP_
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSETINFOCHANGE_HPP_
#include <com/sun/star/beans/PropertySetInfoChange.hpp>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif
#ifndef _UCBHELPER_CONTENTHELPER_HXX
#include <ucbhelper/contenthelper.hxx>
#endif
#ifndef _UCBHELPER_PROVIDERHELPER_HXX
#include <ucbhelper/providerhelper.hxx>
#endif
#ifndef _UCBHELPER_CONTENTINFO_HXX
#include <ucbhelper/contentinfo.hxx>
#endif

using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace rtl;
using namespace vos;
using namespace ucb;

namespace ucb_impl
{

//=========================================================================
//
// class PropertyEventSequence.
//
//=========================================================================

class PropertyEventSequence
{
    Sequence< PropertyChangeEvent > m_aSeq;
    sal_uInt32                      m_nPos;

public:
    PropertyEventSequence( sal_uInt32 nSize )
    : m_aSeq( nSize ), m_nPos( 0 ) {};

    void append( const PropertyChangeEvent& rEvt )
    { m_aSeq.getArray()[ m_nPos ] = rEvt; ++m_nPos; }

    const Sequence< PropertyChangeEvent >& getEvents()
    { m_aSeq.realloc( m_nPos ); return m_aSeq; }
};

//=========================================================================
//
// PropertiesEventListenerMap.
//
//=========================================================================

typedef void* XPropertiesChangeListenerPtr; // -> Compiler problems!

struct equalPtr
{
    bool operator()( const XPropertiesChangeListenerPtr& rp1,
                     const XPropertiesChangeListenerPtr& rp2 ) const
    {
        return ( rp1 == rp2 );
    }
};

struct hashPtr
{
    size_t operator()( const XPropertiesChangeListenerPtr& rp ) const
    {
        return (size_t)rp;
    }
};

typedef std::hash_map
<
    XPropertiesChangeListenerPtr,
    PropertyEventSequence*,
    hashPtr,
    equalPtr
>
PropertiesEventListenerMap;

//=========================================================================
//
// PropertyChangeListenerContainer.
//
//=========================================================================

struct equalStr
{
    bool operator()( const OUString& s1, const OUString& s2 ) const
      {
        return !!( s1 == s2 );
    }
};

struct hashStr
{
    size_t operator()( const OUString& rName ) const
    {
        return rName.hashCode();
    }
};

typedef OMultiTypeInterfaceContainerHelperVar
<
    OUString,
    hashStr,
    equalStr
> PropertyChangeListeners;

//=========================================================================
//
// struct ContentImplHelper_Impl
//
//=========================================================================

struct ContentImplHelper_Impl
{
    vos::ORef< PropertySetInfo >      m_xPropSetInfo;
    vos::ORef< CommandProcessorInfo > m_xCommandsInfo;
    cppu::OInterfaceContainerHelper*  m_pDisposeEventListeners;
    cppu::OInterfaceContainerHelper*  m_pContentEventListeners;
    cppu::OInterfaceContainerHelper*  m_pPropSetChangeListeners;
    cppu::OInterfaceContainerHelper*  m_pCommandChangeListeners;
    PropertyChangeListeners*          m_pPropertyChangeListeners;

    ContentImplHelper_Impl()
    : m_pDisposeEventListeners( 0 ),
        m_pContentEventListeners( 0 ),
      m_pPropSetChangeListeners( 0 ),
        m_pCommandChangeListeners( 0 ),
      m_pPropertyChangeListeners( 0 ) {}

    ~ContentImplHelper_Impl()
    {
        delete m_pDisposeEventListeners;
        delete m_pContentEventListeners;
        delete m_pPropSetChangeListeners;
        delete m_pCommandChangeListeners;
        delete m_pPropertyChangeListeners;
    }
};

} // namespace ucb_impl

using namespace ucb_impl;

//=========================================================================
//=========================================================================
//
// ContentImplHelper Implementation.
//
//=========================================================================
//=========================================================================

ContentImplHelper::ContentImplHelper(
                    const Reference< XMultiServiceFactory >& rxSMgr,
                    const vos::ORef< ContentProviderImplHelper >& rxProvider,
                    const Reference< XContentIdentifier >& Identifier,
                    sal_Bool bRegisterAtProvider )
: m_pImpl( new ContentImplHelper_Impl ),
  m_xSMgr( rxSMgr ),
  m_xProvider( rxProvider ),
  m_xIdentifier( Identifier ),
  m_nCommandId( 0 )
{
    if ( bRegisterAtProvider )
        m_xProvider->addContent( this );
}

//=========================================================================
// virtual
ContentImplHelper::~ContentImplHelper()
{
    m_xProvider->removeContent( this );
    delete m_pImpl;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_10( ContentImplHelper,
                    XTypeProvider,
                    XServiceInfo,
                    XComponent,
                    XContent,
                    XCommandProcessor,
                    XPropertiesChangeNotifier,
                    XCommandInfoChangeNotifier,
                    XPropertyContainer,
                    XPropertySetInfoChangeNotifier,
                    XChild );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_10( ContentImplHelper,
                       XTypeProvider,
                       XServiceInfo,
                       XComponent,
                       XContent,
                       XCommandProcessor,
                       XPropertiesChangeNotifier,
                       XCommandInfoChangeNotifier,
                       XPropertyContainer,
                       XPropertySetInfoChangeNotifier,
                       XChild );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

/*

 Pure virtual. Must be implemented by derived classes!

// virtual
OUString SAL_CALL ContentImplHelper::getImplementationName()
    throw( RuntimeException )
{
}

//=========================================================================
// virtual
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
ContentImplHelper::getSupportedServiceNames()
    throw( RuntimeException )
{
}

*/

//=========================================================================
// virtual
sal_Bool SAL_CALL ContentImplHelper::supportsService(
                                            const OUString& ServiceName )
    throw( RuntimeException )
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString* pArray = aSNL.getConstArray();
    for ( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
    {
        if ( pArray[ i ] == ServiceName )
            return sal_True;
    }

    return sal_False;
}

//=========================================================================
//
// XComponent methods.
//
//=========================================================================

// virtual
void SAL_CALL ContentImplHelper::dispose()
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_pImpl->m_pDisposeEventListeners &&
         m_pImpl->m_pDisposeEventListeners->getLength() )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XComponent * >( this );
        m_pImpl->m_pDisposeEventListeners->disposeAndClear( aEvt );
    }

    if ( m_pImpl->m_pContentEventListeners &&
         m_pImpl->m_pContentEventListeners->getLength() )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XContent * >( this );
        m_pImpl->m_pContentEventListeners->disposeAndClear( aEvt );
    }

    if ( m_pImpl->m_pPropSetChangeListeners &&
         m_pImpl->m_pPropSetChangeListeners->getLength() )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XPropertySetInfoChangeNotifier * >( this );
        m_pImpl->m_pPropSetChangeListeners->disposeAndClear( aEvt );
    }

    if ( m_pImpl->m_pCommandChangeListeners &&
         m_pImpl->m_pCommandChangeListeners->getLength() )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XCommandInfoChangeNotifier * >( this );
        m_pImpl->m_pCommandChangeListeners->disposeAndClear( aEvt );
    }

    if ( m_pImpl->m_pPropertyChangeListeners )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XPropertiesChangeNotifier * >( this );
        m_pImpl->m_pPropertyChangeListeners->disposeAndClear( aEvt );
    }
}

//=========================================================================
// virtual
void SAL_CALL ContentImplHelper::addEventListener(
                            const Reference< XEventListener >& Listener )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !m_pImpl->m_pDisposeEventListeners )
        m_pImpl->m_pDisposeEventListeners
            = new OInterfaceContainerHelper( m_aMutex );

    m_pImpl->m_pDisposeEventListeners->addInterface( Listener );
}

//=========================================================================
// virtual
void SAL_CALL ContentImplHelper::removeEventListener(
                            const Reference< XEventListener >& Listener )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_pImpl->m_pDisposeEventListeners )
        m_pImpl->m_pDisposeEventListeners->removeInterface( Listener );
}

//=========================================================================
//
// XContent methods.
//
//=========================================================================

// virtual
Reference< XContentIdentifier > SAL_CALL ContentImplHelper::getIdentifier()
    throw( RuntimeException )
{
    return m_xIdentifier;
}

/*

 Pure virtual. Must be implemented by derived classes!

//=========================================================================
// virtual
OUString SAL_CALL ContentImplHelper::getContentType()
    throw( RuntimeException )
{
}

*/

//=========================================================================
// virtual
void SAL_CALL ContentImplHelper::addContentEventListener(
                        const Reference< XContentEventListener >& Listener )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !m_pImpl->m_pContentEventListeners )
        m_pImpl->m_pContentEventListeners
            = new OInterfaceContainerHelper( m_aMutex );

    m_pImpl->m_pContentEventListeners->addInterface( Listener );
}

//=========================================================================
// virtual
void SAL_CALL ContentImplHelper::removeContentEventListener(
                        const Reference< XContentEventListener >& Listener )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_pImpl->m_pContentEventListeners )
        m_pImpl->m_pContentEventListeners->removeInterface( Listener );
}

//=========================================================================
//
// XCommandProcessor methods.
//
//=========================================================================

// virtual
sal_Int32 SAL_CALL ContentImplHelper::createCommandIdentifier()
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    // Just increase counter on every call to generate an identifier.
    return ++m_nCommandId;
}

/*

 Pure virtual. Must be implemented by derived classes!

//=========================================================================
// virtual
Any SAL_CALL ContentImplHelper::execute( const Command& aCommand,
                                     sal_Int32 CommandId,
                                     const Reference<
                                        XCommandEnvironment >& Environment )
    throw( Exception, CommandAbortedException, RuntimeException )
{
    if ( aCommand.Name.compareToAscii( "getPropertyValues" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // getPropertyValues
        //////////////////////////////////////////////////////////////////

        Sequence< Property > Properties;
        if ( !( aCommand.Argument >>= Properties ) )
        {
            VOS_ENSURE( sal_False, "Wrong argument type!" )
            return Any();
        }

        // Note: GET + empty sequence means "get all property values".
    }
    else if ( aCommand.Name.compareToAscii( "setPropertyValues" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // setPropertyValues
        //////////////////////////////////////////////////////////////////

        Sequence< PropertyValue > Properties;
        if ( !( aCommand.Argument >>= Properties ) )
        {
            VOS_ENSURE( sal_False, "Wrong argument type!" )
            return Any();
        }

        if ( !Properties.getLength() )
        {
            VOS_ENSURE( sal_False, "No properties!" )
            return Any();
        }
    }
    else if ( aCommand.Name.compareToAscii( "getPropertySetInfo" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // getPropertySetInfo
        //////////////////////////////////////////////////////////////////

        aRet <<= getPropertySetInfo();
    }
    else if ( aCommand.Name.compareToAscii( "getCommandInfo" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // getCommandInfo
        //////////////////////////////////////////////////////////////////

        aRet <<= getCommandInfo();
    }
    else if ( ( aCommand.Name.compareToAscii( "open" ) == 0 ) &&
              ( open_command_arg contains no data sink )  )
    {
        //////////////////////////////////////////////////////////////////
        // open command for a folder content
        //////////////////////////////////////////////////////////////////

        Reference< XDynamicResultSet > xSet
                = new DynamicResultSet( m_xSMgr, this, aCommand, Environment );
        aRet <<= xSet;
    }
    else if ( aCommand.Name.compareToAscii( "search" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // search command
        //////////////////////////////////////////////////////////////////

        Reference< XDynamicResultSet > xSet
                = new DynamicResultSet( m_xSMgr, this, aCommand, Environment );
        aRet <<= xSet;
    }
    else
    {
        //////////////////////////////////////////////////////////////////
        // any other command
        //////////////////////////////////////////////////////////////////

        // Check the command...
        if ( !aCommand.Name.getLength() && ( aCommand.Handle == -1 ) )
        {
            VOS_ENSURE( sal_False, "No command!" )
            return Any();
        }

    }

    return aRet;
}

//=========================================================================
// virtual
void SAL_CALL ContentImplHelper::abort( sal_Int32 CommandId )
    throw( RuntimeException )
{
}

*/

//=========================================================================
//
// XPropertiesChangeNotifier methods.
//
//=========================================================================

// virtual
void SAL_CALL ContentImplHelper::addPropertiesChangeListener(
                    const Sequence< OUString >& PropertyNames,
                    const Reference< XPropertiesChangeListener >& Listener )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !m_pImpl->m_pPropertyChangeListeners )
        m_pImpl->m_pPropertyChangeListeners
            = new PropertyChangeListeners( m_aMutex );

    sal_Int32 nCount = PropertyNames.getLength();
    if ( !nCount )
    {
        // Note: An empty sequence means a listener for "all" properties.
        m_pImpl->m_pPropertyChangeListeners->addInterface(
                                                OUString(), Listener );
    }
    else
    {
        const OUString* pSeq = PropertyNames.getConstArray();

        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const OUString& rName = pSeq[ n ];
            if ( rName.getLength() )
                m_pImpl->m_pPropertyChangeListeners->addInterface(
                                                        rName, Listener );
        }
    }
}

//=========================================================================
// virtual
void SAL_CALL ContentImplHelper::removePropertiesChangeListener(
                    const Sequence< OUString >& PropertyNames,
                    const Reference< XPropertiesChangeListener >& Listener )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !m_pImpl->m_pPropertyChangeListeners )
        return;

    sal_Int32 nCount = PropertyNames.getLength();
    if ( !nCount )
    {
        // Note: An empty sequence means a listener for "all" properties.
        m_pImpl->m_pPropertyChangeListeners->removeInterface(
                                                OUString(), Listener );
    }
    else
    {
        const OUString* pSeq = PropertyNames.getConstArray();

        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const OUString& rName = pSeq[ n ];
            if ( rName.getLength() )
                m_pImpl->m_pPropertyChangeListeners->removeInterface(
                                                        rName, Listener );
        }
    }
}

//=========================================================================
//
// XCommandInfoChangeNotifier methods.
//
//=========================================================================

// virtual
void SAL_CALL ContentImplHelper::addCommandInfoChangeListener(
                    const Reference< XCommandInfoChangeListener >& Listener )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !m_pImpl->m_pCommandChangeListeners )
        m_pImpl->m_pCommandChangeListeners
            = new OInterfaceContainerHelper( m_aMutex );

    m_pImpl->m_pCommandChangeListeners->addInterface( Listener );
}

//=========================================================================
// virtual
void SAL_CALL ContentImplHelper::removeCommandInfoChangeListener(
                    const Reference< XCommandInfoChangeListener >& Listener )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_pImpl->m_pCommandChangeListeners )
        m_pImpl->m_pCommandChangeListeners->removeInterface( Listener );
}

//=========================================================================
//
// XPropertyContainer methods.
//
//=========================================================================

// virtual
void SAL_CALL ContentImplHelper::addProperty(
        const OUString& Name, sal_Int16 Attributes, const Any& DefaultValue )
    throw( PropertyExistException,
           IllegalTypeException,
           IllegalArgumentException,
           RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    //////////////////////////////////////////////////////////////////////
    // Make sure a property with the requested name does not already
    // exist in dynamic and static(!) properties.
    //////////////////////////////////////////////////////////////////////

    if ( getPropertySetInfo()->hasPropertyByName( Name ) )
    {
        // Property does already exist.
        throw PropertyExistException();
    }

    //////////////////////////////////////////////////////////////////////
    // Add a new dynamic property.
    //////////////////////////////////////////////////////////////////////

    // Open/create persistent property set.
    Reference< XPersistentPropertySet > xSet(
                                    getAdditionalPropertySet( sal_True ) );

    VOS_ENSURE( xSet.is(), "ContentImplHelper::addProperty - No property set!" );

    if ( xSet.is() )
    {
        Reference< XPropertyContainer > xContainer( xSet, UNO_QUERY );

        VOS_ENSURE( xContainer.is(),
                    "ContentImplHelper::addProperty - No property container!" );

        if ( xContainer.is() )
        {
            // Property is always removeable.
            Attributes |= PropertyAttribute::REMOVEABLE;

            try
            {
                xContainer->addProperty( Name, Attributes, DefaultValue );
            }
            catch ( PropertyExistException& e )
            {
                VOS_ENSURE( sal_False,
                            "ContentImplHelper::addProperty - Exists!" );
                throw e;
            }
            catch ( IllegalTypeException& e )
            {
                VOS_ENSURE( sal_False,
                            "ContentImplHelper::addProperty - Wrong Type!" );
                throw e;
            }
            catch ( IllegalArgumentException& e )
            {
                VOS_ENSURE( sal_False,
                            "ContentImplHelper::addProperty - Illegal Arg!" );
                throw e;
            }

            // Success!

            if ( m_pImpl->m_xPropSetInfo.isValid() )
            {
                // Info cached in propertyset info is invalid now!
                m_pImpl->m_xPropSetInfo->reset();
            }

            // Notify propertyset info change listeners.
            if ( m_pImpl->m_pPropSetChangeListeners &&
                 m_pImpl->m_pPropSetChangeListeners->getLength() )
            {
                PropertySetInfoChangeEvent evt(
                            static_cast< OWeakObject * >( this ),
                            Name,
                            -1, // No handle available
                            PropertySetInfoChange::PROPERTY_INSERTED );
                notifyPropertySetInfoChange( evt );
            }
        }
    }
}

//=========================================================================
// virtual
void SAL_CALL ContentImplHelper::removeProperty( const OUString& Name )
    throw( UnknownPropertyException,
           NotRemoveableException,
           RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    try
    {
        Property aProp = getPropertySetInfo()->getPropertyByName( Name );

        if ( !( aProp.Attributes & PropertyAttribute::REMOVEABLE ) )
        {
            // Not removeable!
            throw NotRemoveableException();
        }
    }
    catch ( UnknownPropertyException& e )
    {
        VOS_ENSURE( sal_False, "ContentImplHelper::removeProperty - Unknown!" );
        throw e;
    }

    //////////////////////////////////////////////////////////////////////
    // Try to remove property from dynamic property set.
    //////////////////////////////////////////////////////////////////////

    // Open persistent property set, if exists.
    Reference< XPersistentPropertySet > xSet(
                                    getAdditionalPropertySet( sal_False ) );
    if ( xSet.is() )
    {
        Reference< XPropertyContainer > xContainer( xSet, UNO_QUERY );

        VOS_ENSURE( xContainer.is(),
                    "ContentImplHelper::removeProperty - No property container!" );

        if ( xContainer.is() )
        {
            try
            {
                xContainer->removeProperty( Name );
            }
            catch ( UnknownPropertyException& e )
            {
                VOS_ENSURE( sal_False,
                            "ContentImplHelper::removeProperty - Unknown!" );
                throw e;
            }
            catch ( NotRemoveableException& e )
            {
                VOS_ENSURE( sal_False,
                            "ContentImplHelper::removeProperty - Unremoveable!" );
                throw e;
            }

            xContainer = 0;

            // Success!

            if ( xSet->getPropertySetInfo()->getProperties().getLength() == 0 )
            {
                // Remove empty propertyset from registry.
                Reference< XPropertySetRegistry > xReg = xSet->getRegistry();
                if ( xReg.is() )
                {
                    OUString aKey( xSet->getKey() );
                    xSet = 0;
                    xReg->removePropertySet( aKey );
                }
            }

            if ( m_pImpl->m_xPropSetInfo.isValid() )
            {
                // Info cached in propertyset info is invalid now!
                m_pImpl->m_xPropSetInfo->reset();
            }

            // Notify propertyset info change listeners.
            if ( m_pImpl->m_pPropSetChangeListeners &&
                 m_pImpl->m_pPropSetChangeListeners->getLength() )
            {
                PropertySetInfoChangeEvent evt(
                            static_cast< OWeakObject * >( this ),
                            Name,
                            -1, // No handle available
                            PropertySetInfoChange::PROPERTY_REMOVED );
                notifyPropertySetInfoChange( evt );
            }
        }
    }
}

//=========================================================================
//
// XPropertySetInfoChangeNotifier methods.
//
//=========================================================================

// virtual
void SAL_CALL ContentImplHelper::addPropertySetInfoChangeListener(
                const Reference< XPropertySetInfoChangeListener >& Listener )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !m_pImpl->m_pPropSetChangeListeners )
        m_pImpl->m_pPropSetChangeListeners
            = new OInterfaceContainerHelper( m_aMutex );

    m_pImpl->m_pPropSetChangeListeners->addInterface( Listener );
}

//=========================================================================
// virtual
void SAL_CALL ContentImplHelper::removePropertySetInfoChangeListener(
                const Reference< XPropertySetInfoChangeListener >& Listener )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_pImpl->m_pPropSetChangeListeners )
        m_pImpl->m_pPropSetChangeListeners->removeInterface( Listener );
}

//=========================================================================
//
// XChild methods.
//
//=========================================================================

// virtual
Reference< XInterface > SAL_CALL ContentImplHelper::getParent()
    throw( RuntimeException )
{
    OUString aURL = getParentURL();

    if ( aURL.getLength() )
    {
        Reference< XContentIdentifier > xId(
                    new ::ucb::ContentIdentifier( m_xSMgr, aURL ) );
        return m_xProvider->queryContent( xId );
    }

    return Reference< XInterface >();
}

//=========================================================================
// virtual
void SAL_CALL ContentImplHelper::setParent(
                                    const Reference< XInterface >& Parent )
    throw( NoSupportException, RuntimeException )
{
    throw NoSupportException();
}

//=========================================================================
//
// Non-interface methods
//
//=========================================================================

Reference< XPersistentPropertySet >
                ContentImplHelper::getAdditionalPropertySet( sal_Bool bCreate )
{
    // Get propertyset from provider.
    return m_xProvider->getAdditionalPropertySet(
                            m_xIdentifier->getContentIdentifier(), bCreate );
}

//=========================================================================
sal_Bool ContentImplHelper::renameAdditionalPropertySet(
                                                      const OUString& rOldKey,
                                                      const OUString& rNewKey,
                                                      sal_Bool bRecursive )
{
    return m_xProvider->renameAdditionalPropertySet(
                                            rOldKey, rNewKey, bRecursive );
}

//=========================================================================
sal_Bool ContentImplHelper::removeAdditionalPropertySet( sal_Bool bRecursive )
{
    return m_xProvider->removeAdditionalPropertySet(
                    m_xIdentifier->getContentIdentifier(), bRecursive );
}

//=========================================================================
void ContentImplHelper::notifyPropertiesChange(
                            const Sequence< PropertyChangeEvent >& evt ) const
{
    if ( !m_pImpl->m_pPropertyChangeListeners )
        return;

    sal_Int32 nCount = evt.getLength();
    if ( nCount )
    {
        // First, notify listeners interested in changes of every property.
        OInterfaceContainerHelper* pAllPropsContainer
            = m_pImpl->m_pPropertyChangeListeners->getContainer( OUString() );
        if ( pAllPropsContainer )
        {
            OInterfaceIteratorHelper aIter( *pAllPropsContainer );
            while ( aIter.hasMoreElements() )
            {
                // Propagate event.
                Reference< XPropertiesChangeListener > xListener(
                                                aIter.next(), UNO_QUERY );
                if ( xListener.is() )
                    xListener->propertiesChange( evt );
            }
        }

        PropertiesEventListenerMap aListeners;

        const PropertyChangeEvent* pEvents = evt.getConstArray();

        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const PropertyChangeEvent& rEvent = pEvents[ n ];
            const OUString& rName = rEvent.PropertyName;

            OInterfaceContainerHelper* pPropsContainer
                = m_pImpl->m_pPropertyChangeListeners->getContainer( rName );
            if ( pPropsContainer )
            {
                OInterfaceIteratorHelper aIter( *pPropsContainer );
                while ( aIter.hasMoreElements() )
                {
                    PropertyEventSequence* pEvents = NULL;

                    XPropertiesChangeListener* pListener =
                        static_cast< XPropertiesChangeListener * >(
                                                            aIter.next() );
                    PropertiesEventListenerMap::iterator it =
                            aListeners.find( pListener );
                    if ( it == aListeners.end() )
                    {
                        // Not in map - create and insert new entry.
                        pEvents = new PropertyEventSequence( nCount );
                        aListeners[ pListener ] = pEvents;
                    }
                    else
                        pEvents = (*it).second;

                    if ( pEvents )
                        pEvents->append( rEvent );
                }
            }
        }

        // Notify listeners.
        PropertiesEventListenerMap::iterator it = aListeners.begin();
        while ( !aListeners.empty() )
        {
            XPropertiesChangeListener* pListener =
                    static_cast< XPropertiesChangeListener * >( (*it).first );
            PropertyEventSequence* pSeq = (*it).second;

            // Remove current element.
            aListeners.erase( it );

            // Propagate event.
            pListener->propertiesChange( pSeq->getEvents() );

            delete pSeq;

            it = aListeners.begin();
        }
    }
}

//=========================================================================
void ContentImplHelper::notifyPropertySetInfoChange(
                                const PropertySetInfoChangeEvent& evt ) const
{
    if ( !m_pImpl->m_pPropSetChangeListeners )
        return;

    // Notify event listeners.
    OInterfaceIteratorHelper aIter( *m_pImpl->m_pPropSetChangeListeners );
    while ( aIter.hasMoreElements() )
    {
        // Propagate event.
        Reference< XPropertySetInfoChangeListener >
                            xListener( aIter.next(), UNO_QUERY );
        if ( xListener.is() )
            xListener->propertySetInfoChange( evt );
    }
}

//=========================================================================
void ContentImplHelper::notifyCommandInfoChange(
                                const CommandInfoChangeEvent& evt ) const
{
    if ( !m_pImpl->m_pCommandChangeListeners )
        return;

    // Notify event listeners.
    OInterfaceIteratorHelper aIter( *m_pImpl->m_pCommandChangeListeners );
    while ( aIter.hasMoreElements() )
    {
        // Propagate event.
        Reference< XCommandInfoChangeListener >
                            xListener( aIter.next(), UNO_QUERY );
        if ( xListener.is() )
            xListener->commandInfoChange( evt );
    }
}

//=========================================================================
void ContentImplHelper::notifyContentEvent( const ContentEvent& evt ) const
{
    if ( !m_pImpl->m_pContentEventListeners )
        return;

    // Notify event listeners.
    OInterfaceIteratorHelper aIter( *m_pImpl->m_pContentEventListeners );
    while ( aIter.hasMoreElements() )
    {
        // Propagate event.
        Reference< XContentEventListener > xListener( aIter.next(), UNO_QUERY );
        if ( xListener.is() )
            xListener->contentEvent( evt );
    }
}

//=========================================================================
void ContentImplHelper::inserted()
{
    // Content is not yet registered at provider.
    m_xProvider->addContent( this );

    // If the parent content is currently not instanciated, there can be
    // no listeners interested in changes ;-)

    vos::ORef< ContentImplHelper > xParent
                = m_xProvider->queryExistingContent( getParentURL() );

    if ( xParent.isValid() )
    {
        ContentEvent aEvt( static_cast< OWeakObject * >(
                                            xParent.getBodyPtr() ), // Source
                           ContentAction::INSERTED,                 // Action
                           this,                                    // Content
                           xParent->getIdentifier() );              // Id
        xParent->notifyContentEvent( aEvt );
    }
}

//=========================================================================
void ContentImplHelper::deleted()
{
    Reference< XContent > xThis = this;

    vos::ORef< ContentImplHelper > xParent
                    = m_xProvider->queryExistingContent( getParentURL() );

    if ( xParent.isValid() )
    {
        // Let parent notify "REMOVED" event.
        ContentEvent aEvt( static_cast< OWeakObject * >( xParent.getBodyPtr() ),
                           ContentAction::REMOVED,
                           this,
                           xParent->getIdentifier() );
        xParent->notifyContentEvent( aEvt );
    }

    // Notify "DELETED" event.
    ContentEvent aEvt1( static_cast< OWeakObject * >( this ),
                        ContentAction::DELETED,
                        this,
                        getIdentifier() );
    notifyContentEvent( aEvt1 );

    m_xProvider->removeContent( this );
}

//=========================================================================
sal_Bool ContentImplHelper::exchange(
                            const Reference< XContentIdentifier >& rNewId )
{
    Reference< XContent > xThis = this;

    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    vos::ORef< ContentImplHelper > xContent
                        = m_xProvider->queryExistingContent( rNewId );
    if ( xContent.isValid() )
    {
        // @@@
        // Big trouble. Another object with the new identity exists.
        // How shall I mutate to / merge with the other object?
        return sal_False;
    }

    Reference< XContentIdentifier > xOldId = getIdentifier();

    // Re-insert at provider.
    m_xProvider->removeContent( this );
    m_xIdentifier = rNewId;
    m_xProvider->addContent( this );

    aGuard.clear();

    // Notify "EXCHANGED" event.
    ContentEvent aEvt( static_cast< OWeakObject * >( this ),
                       ContentAction::EXCHANGED,
                       this,
                       xOldId );
    notifyContentEvent( aEvt );
    return sal_True;
}

//=========================================================================
Reference< XCommandInfo > ContentImplHelper::getCommandInfo()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !m_pImpl->m_xCommandsInfo.isValid() )
        m_pImpl->m_xCommandsInfo = new CommandProcessorInfo( m_xSMgr, this );

    return Reference< XCommandInfo >( m_pImpl->m_xCommandsInfo.getBodyPtr() );
}

//=========================================================================
Reference< XPropertySetInfo > ContentImplHelper::getPropertySetInfo()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !m_pImpl->m_xPropSetInfo.isValid() )
        m_pImpl->m_xPropSetInfo = new PropertySetInfo( m_xSMgr, this );

    return Reference< XPropertySetInfo >(
                                    m_pImpl->m_xPropSetInfo.getBodyPtr() );
}

