/*************************************************************************
 *
 *  $RCSfile: ContentHelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:05:59 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef DBA_CONTENTHELPER_HXX
#include "ContentHelper.hxx"
#endif
#ifndef _UCBHELPER_CANCELCOMMANDEXECUTION_HXX_
#include <ucbhelper/cancelcommandexecution.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_UNSUPPORTEDCOMMANDEXCEPTION_HPP_
#include <com/sun/star/ucb/UnsupportedCommandException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALACCESSEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalAccessException.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _UCBHELPER_PROPERTYVALUESET_HXX
#include <ucbhelper/propertyvalueset.hxx>
#endif
#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif
#ifndef DBA_UCPRESULTSET_HXX
#include "myucp_resultset.hxx"
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef DBACORE_SDBCORETOOLS_HXX
#include "sdbcoretools.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


namespace dbaccess
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::container;
using namespace ::comphelper;
using namespace ::cppu;

// -----------------------------------------------------------------------------
DBG_NAME(OContentHelper_Impl)
OContentHelper_Impl::OContentHelper_Impl()
{
    DBG_CTOR(OContentHelper_Impl,NULL);
}
// -----------------------------------------------------------------------------
OContentHelper_Impl::~OContentHelper_Impl()
{
    DBG_DTOR(OContentHelper_Impl,NULL);
}
// -----------------------------------------------------------------------------

OContentHelper::OContentHelper(const Reference< XMultiServiceFactory >& _xORB
                               ,const Reference< XInterface >&  _xParentContainer
                               ,const TContentPtr& _pImpl)
    : OContentHelper_COMPBASE(m_aMutex)
    ,m_aContentListeners(m_aMutex)
    ,m_aPropertyChangeListeners(m_aMutex)
    ,m_nCommandId(0)
    ,m_pImpl(_pImpl)
    ,m_xORB(_xORB)
    ,m_xParentContainer(_xParentContainer)
{
}
//--------------------------------------------------------------------------
void SAL_CALL OContentHelper::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // say our listeners goobye
    EventObject aEvt(*this);
    m_aContentListeners.disposeAndClear(aEvt);

    m_xParentContainer = NULL;
}
// -----------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO1(OContentHelper,"com.sun.star.comp.sdb.Content","com.sun.star.ucb.Content");
IMPLEMENT_IMPLEMENTATION_ID(OContentHelper)
// -----------------------------------------------------------------------------
// XContent
Reference< XContentIdentifier > SAL_CALL OContentHelper::getIdentifier(  ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::rtl::OUString sContentId = m_pImpl->m_aProps.aTitle;
    Reference< XInterface > xParent = m_xParentContainer;
    while( xParent.is() )
    {
        Reference<XPropertySet> xProp(xParent,UNO_QUERY);
        Reference<XChild> xChild(xParent,UNO_QUERY);
        xParent.set(xChild.is() ? xChild->getParent() : NULL,UNO_QUERY);
        if ( xProp.is() && xParent.is() )
        {
            ::rtl::OUString sName;
            xProp->getPropertyValue(PROPERTY_NAME) >>= sName;
            sContentId = sName + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) + sContentId;
        }
    }

    sContentId = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:")) + sContentId;
    return new ::ucb::ContentIdentifier(m_xORB,sContentId);
    //  return Reference< XContentIdentifier >();
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OContentHelper::getContentType(  ) throw (RuntimeException)
{
    return getImplementationName();
}
// -----------------------------------------------------------------------------
void SAL_CALL OContentHelper::addContentEventListener( const Reference< XContentEventListener >& _rxListener ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( _rxListener.is() )
        m_aContentListeners.addInterface(_rxListener);
}
// -----------------------------------------------------------------------------
void SAL_CALL OContentHelper::removeContentEventListener( const Reference< XContentEventListener >& _rxListener ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if (_rxListener.is())
        m_aContentListeners.removeInterface(_rxListener);
}
// -----------------------------------------------------------------------------

// XCommandProcessor
sal_Int32 SAL_CALL OContentHelper::createCommandIdentifier(  ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    // Just increase counter on every call to generate an identifier.
    return ++m_nCommandId;
}
// -----------------------------------------------------------------------------
Any SAL_CALL OContentHelper::execute( const Command& aCommand, sal_Int32 CommandId, const Reference< XCommandEnvironment >& Environment ) throw (Exception, CommandAbortedException, RuntimeException)
{
    Any aRet;
    if ( aCommand.Name.compareToAscii( "getPropertyValues" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // getPropertyValues
        //////////////////////////////////////////////////////////////////

        Sequence< Property > Properties;
        if ( !( aCommand.Argument >>= Properties ) )
        {
            OSL_ENSURE( sal_False, "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                makeAny( IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }
        aRet <<= getPropertyValues( Properties);
    }
    else if ( aCommand.Name.compareToAscii( "setPropertyValues" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // setPropertyValues
        //////////////////////////////////////////////////////////////////

        Sequence< PropertyValue > aProperties;
        if ( !( aCommand.Argument >>= aProperties ) )
        {
            OSL_ENSURE( sal_False, "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                makeAny( IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        if ( !aProperties.getLength() )
        {
            OSL_ENSURE( sal_False, "No properties!" );
            ucbhelper::cancelCommandExecution(
                makeAny( IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        aRet <<= setPropertyValues( aProperties, Environment );
    }
    else if ( aCommand.Name.compareToAscii( "getPropertySetInfo" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // getPropertySetInfo
        //////////////////////////////////////////////////////////////////

        Reference<XPropertySet> xProp(*this,UNO_QUERY);
        if ( xProp.is() )
            aRet <<= xProp->getPropertySetInfo();
        //  aRet <<= getPropertySetInfo(); // TODO
    }
    else
    {
        //////////////////////////////////////////////////////////////////
        // Unsupported command
        //////////////////////////////////////////////////////////////////

        OSL_ENSURE( sal_False, "Content::execute - unsupported command!" );

        ucbhelper::cancelCommandExecution(
            makeAny( UnsupportedCommandException(
                            rtl::OUString(),
                            static_cast< cppu::OWeakObject * >( this ) ) ),
            Environment );
        // Unreachable
    }

    return aRet;
}
// -----------------------------------------------------------------------------
void SAL_CALL OContentHelper::abort( sal_Int32 CommandId ) throw (RuntimeException)
{
}
// -----------------------------------------------------------------------------

// XPropertiesChangeNotifier
void SAL_CALL OContentHelper::addPropertiesChangeListener( const Sequence< ::rtl::OUString >& PropertyNames, const Reference< XPropertiesChangeListener >& Listener ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    sal_Int32 nCount = PropertyNames.getLength();
    if ( !nCount )
    {
        // Note: An empty sequence means a listener for "all" properties.
        m_aPropertyChangeListeners.addInterface(::rtl::OUString(), Listener );
    }
    else
    {
        const ::rtl::OUString* pSeq = PropertyNames.getConstArray();

        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const ::rtl::OUString& rName = pSeq[ n ];
            if ( rName.getLength() )
                m_aPropertyChangeListeners.addInterface(rName, Listener );
        }
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OContentHelper::removePropertiesChangeListener( const Sequence< ::rtl::OUString >& PropertyNames, const Reference< XPropertiesChangeListener >& Listener ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    sal_Int32 nCount = PropertyNames.getLength();
    if ( !nCount )
    {
        // Note: An empty sequence means a listener for "all" properties.
        m_aPropertyChangeListeners.removeInterface( ::rtl::OUString(), Listener );
    }
    else
    {
        const ::rtl::OUString* pSeq = PropertyNames.getConstArray();

        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const ::rtl::OUString& rName = pSeq[ n ];
            if ( rName.getLength() )
                m_aPropertyChangeListeners.removeInterface( rName, Listener );
        }
    }
}
// -----------------------------------------------------------------------------

// XPropertyContainer
void SAL_CALL OContentHelper::addProperty( const ::rtl::OUString& Name, sal_Int16 Attributes, const Any& DefaultValue ) throw (PropertyExistException, IllegalTypeException, IllegalArgumentException, RuntimeException)
{
}
// -----------------------------------------------------------------------------
void SAL_CALL OContentHelper::removeProperty( const ::rtl::OUString& Name ) throw (UnknownPropertyException, NotRemoveableException, RuntimeException)
{
}
// -----------------------------------------------------------------------------
// XInitialization
void SAL_CALL OContentHelper::initialize( const Sequence< Any >& _aArguments ) throw(Exception, RuntimeException)
{
    const Any* pBegin = _aArguments.getConstArray();
    const Any* pEnd = pBegin + _aArguments.getLength();
    PropertyValue aValue;;
    for(;pBegin != pEnd;++pBegin)
    {
        *pBegin >>= aValue;
        if ( aValue.Name.equalsAscii("Parent") )
        {
            m_xParentContainer.set(aValue.Value,UNO_QUERY);
        }
        else if ( aValue.Name.equalsAscii(PROPERTY_NAME) )
        {
            aValue.Value >>= m_pImpl->m_aProps.aTitle;
        }
        else if ( aValue.Name.equalsAscii(PROPERTY_PERSISTENT_NAME) )
        {
            aValue.Value >>= m_pImpl->m_aProps.sPersistentName;
        }
    }
}
// -----------------------------------------------------------------------------
Sequence< Any > OContentHelper::setPropertyValues(const Sequence< PropertyValue >& rValues,const Reference< XCommandEnvironment >& xEnv )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    Sequence< Any > aRet( rValues.getLength() );
    Sequence< PropertyChangeEvent > aChanges( rValues.getLength() );
    sal_Int32 nChanged = 0;

    PropertyChangeEvent aEvent;
    aEvent.Source         = static_cast< cppu::OWeakObject * >( this );
    aEvent.Further        = sal_False;
//  aEvent.PropertyName   =
    aEvent.PropertyHandle = -1;
//  aEvent.OldValue       =
//  aEvent.NewValue       =

    const PropertyValue* pValues = rValues.getConstArray();
    sal_Int32 nCount = rValues.getLength();

    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const PropertyValue& rValue = pValues[ n ];

        if ( rValue.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ContentType" ) ) )
        {
            // Read-only property!
            aRet[ n ] <<= IllegalAccessException(
                            rtl::OUString::createFromAscii(
                                "Property is read-only!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rValue.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "IsDocument" ) ) )
        {
            // Read-only property!
            aRet[ n ] <<= IllegalAccessException(
                            rtl::OUString::createFromAscii(
                                "Property is read-only!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rValue.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "IsFolder" ) ) )
        {
            // Read-only property!
            aRet[ n ] <<= IllegalAccessException(
                            rtl::OUString::createFromAscii(
                                "Property is read-only!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rValue.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
        {
            rtl::OUString aNewValue;
            if ( rValue.Value >>= aNewValue )
            {
                osl::Guard< osl::Mutex > aGuard( m_aMutex );
                if ( aNewValue != m_pImpl->m_aProps.aTitle )
                {
                    aEvent.PropertyName = rValue.Name;
                    aEvent.OldValue     = makeAny( m_pImpl->m_aProps.aTitle );
                    aEvent.NewValue     = makeAny( aNewValue );

                    aChanges.getArray()[ nChanged ] = aEvent;

                    m_pImpl->m_aProps.aTitle = aNewValue;
                    nChanged++;
                }
                else
                {
                    // Old value equals new value. No error!
                }
            }
            else
            {
                aRet[ n ] <<= IllegalTypeException(
                                rtl::OUString::createFromAscii(
                                    "Property value has wrong type!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
        }

        // @@@ Process other properties supported directly.
#if 0
        else if ( rValue.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "xxxxxx" ) ) )
        {
        }
#endif
        else
        {
            aRet[ n ] <<= Exception(
                            rtl::OUString::createFromAscii(
                                "No property set for storing the value!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
    }

    if ( nChanged > 0 )
    {
        // @@@ Save changes.
//      storeData();

        notifyDataSourceModified();
        aGuard.clear();
        aChanges.realloc( nChanged );
        notifyPropertiesChange( aChanges );
    }

    return aRet;
}
// -----------------------------------------------------------------------------
//=========================================================================
// static
Reference< XRow > OContentHelper::getPropertyValues( const Sequence< Property >& rProperties)
{
    // Note: Empty sequence means "get values of all supported properties".

    rtl::Reference< ::ucb::PropertyValueSet > xRow = new ::ucb::PropertyValueSet( m_xORB );

    sal_Int32 nCount = rProperties.getLength();
    if ( nCount )
    {
        const Property* pProps = rProperties.getConstArray();
        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const Property& rProp = pProps[ n ];

            // Process Core properties.

            if ( rProp.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ContentType" ) ) )
            {
                xRow->appendString ( rProp, m_pImpl->m_aProps.aContentType );
            }
            else if ( rProp.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
            {
                xRow->appendString ( rProp, m_pImpl->m_aProps.aTitle );
            }
            else if ( rProp.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "IsDocument" ) ) )
            {
                xRow->appendBoolean( rProp, m_pImpl->m_aProps.bIsDocument );
            }
            else if ( rProp.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "IsFolder" ) ) )
            {
                xRow->appendBoolean( rProp, m_pImpl->m_aProps.bIsFolder );
            }
            else
                xRow->appendVoid(rProp);

            // @@@ Process other properties supported directly.
#if 0
            else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "xxxxxx" ) ) )
            {
            }
#endif
        }
    }
    else
    {
        // Append all Core Properties.
        xRow->appendString (
            Property( rtl::OUString::createFromAscii( "ContentType" ),
                      -1,
                      getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                      PropertyAttribute::BOUND
                        | PropertyAttribute::READONLY ),
            m_pImpl->m_aProps.aContentType );
        xRow->appendString (
            Property( rtl::OUString::createFromAscii( "Title" ),
                      -1,
                      getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                      PropertyAttribute::BOUND ),
            m_pImpl->m_aProps.aTitle );
        xRow->appendBoolean(
            Property( rtl::OUString::createFromAscii( "IsDocument" ),
                      -1,
                      getCppuBooleanType(),
                      PropertyAttribute::BOUND
                        | PropertyAttribute::READONLY ),
            m_pImpl->m_aProps.bIsDocument );
        xRow->appendBoolean(
            Property( rtl::OUString::createFromAscii( "IsFolder" ),
                      -1,
                      getCppuBooleanType(),
                      PropertyAttribute::BOUND
                        | PropertyAttribute::READONLY ),
            m_pImpl->m_aProps.bIsFolder );

        // @@@ Append other properties supported directly.
    }

    return Reference< XRow >( xRow.get() );
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void OContentHelper::notifyPropertiesChange( const Sequence< PropertyChangeEvent >& evt ) const
{

    sal_Int32 nCount = evt.getLength();
    if ( nCount )
    {
        // First, notify listeners interested in changes of every property.
        OInterfaceContainerHelper* pAllPropsContainer = m_aPropertyChangeListeners.getContainer( ::rtl::OUString() );
        if ( pAllPropsContainer )
        {
            OInterfaceIteratorHelper aIter( *pAllPropsContainer );
            while ( aIter.hasMoreElements() )
            {
                // Propagate event.
                Reference< XPropertiesChangeListener > xListener( aIter.next(), UNO_QUERY );
                if ( xListener.is() )
                    xListener->propertiesChange( evt );
            }
        }

        typedef Sequence< PropertyChangeEvent > PropertyEventSequence;
        typedef ::std::map< XPropertiesChangeListener*, PropertyEventSequence* > PropertiesEventListenerMap;
        PropertiesEventListenerMap aListeners;


        const PropertyChangeEvent* pEvents = evt.getConstArray();

        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const PropertyChangeEvent& rEvent = pEvents[ n ];
            const ::rtl::OUString& rName = rEvent.PropertyName;

            OInterfaceContainerHelper* pPropsContainer = m_aPropertyChangeListeners.getContainer( rName );
            if ( pPropsContainer )
            {
                OInterfaceIteratorHelper aIter( *pPropsContainer );
                while ( aIter.hasMoreElements() )
                {
                    PropertyEventSequence* pEvents = NULL;

                    XPropertiesChangeListener* pListener = static_cast< XPropertiesChangeListener * >( aIter.next() );
                    PropertiesEventListenerMap::iterator it = aListeners.find( pListener );
                    if ( it == aListeners.end() )
                    {
                        // Not in map - create and insert new entry.
                        pEvents = new PropertyEventSequence( nCount );
                        aListeners[ pListener ] = pEvents;
                    }
                    else
                        pEvents = (*it).second;

                    if ( pEvents )
                        (*pEvents)[n] = rEvent;
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
            pListener->propertiesChange( *pSeq );

            delete pSeq;

            it = aListeners.begin();
        }
    }
}
// -----------------------------------------------------------------------------
// com::sun::star::lang::XUnoTunnel
//------------------------------------------------------------------
sal_Int64 OContentHelper::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return reinterpret_cast<sal_Int64>(this);

    return 0;
}

// -----------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OContentHelper::getParent(  ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_xParentContainer;
}
// -----------------------------------------------------------------------------
void SAL_CALL OContentHelper::setParent( const Reference< XInterface >& _xParent ) throw (NoSupportException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_xParentContainer = _xParent;
}
// -----------------------------------------------------------------------------
void SAL_CALL OContentHelper::rename( const ::rtl::OUString& newName ) throw (SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard  aGuard(m_aMutex);
    Reference<XNameContainer> xNameCont(m_xParentContainer,UNO_QUERY);
    if ( xNameCont.is() )
    {
        if ( xNameCont->hasByName(newName) )
            throw ElementExistException(newName,*this);

        try
        {
            if ( xNameCont->hasByName(m_pImpl->m_aProps.aTitle) )
                xNameCont->removeByName(m_pImpl->m_aProps.aTitle);

            m_pImpl->m_aProps.aTitle = newName;
            xNameCont->insertByName(m_pImpl->m_aProps.aTitle,makeAny(Reference<XContent>(*this,UNO_QUERY)));
            notifyDataSourceModified();
        }
        catch(IllegalArgumentException)
        {
            throw SQLException();
        }
        catch(NoSuchElementException)
        {
            throw SQLException();
        }
        catch(WrappedTargetException)
        {
            throw SQLException();
        }
    }
    else
        m_pImpl->m_aProps.aTitle = newName;

}
// -----------------------------------------------------------------------------
void OContentHelper::notifyDataSourceModified()
{
    ::dbaccess::notifyDataSourceModified(m_xParentContainer,sal_True);
}
// -----------------------------------------------------------------------------
Reference< XStorage> OContentHelper::getStorage() const
{
    return NULL;
}
//........................................................................
}   // namespace dbaccess
//........................................................................

