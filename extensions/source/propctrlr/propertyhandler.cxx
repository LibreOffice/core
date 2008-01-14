/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propertyhandler.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 15:00:31 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYHANDLER_HXX
#include "propertyhandler.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_FORMMETADATA_HXX_
#include "formmetadata.hxx"
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_FORMBROWSERTOOLS_HXX_
#include "formbrowsertools.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_HANDLERHELPER_HXX
#include "handlerhelper.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_NULLPOINTEREXCEPTION_HPP_
#include <com/sun/star/lang/NullPointerException.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif
/** === end UNO includes === **/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include <algorithm>

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::inspection;
    using namespace ::comphelper;

    //====================================================================
    //= PropertyHandler
    //====================================================================
    DBG_NAME( PropertyHandler )
    //--------------------------------------------------------------------
    PropertyHandler::PropertyHandler( const Reference< XComponentContext >& _rxContext )
        :PropertyHandler_Base( m_aMutex )
        ,m_bSupportedPropertiesAreKnown( false )
        ,m_aPropertyListeners( m_aMutex )
        ,m_aContext( _rxContext )
        ,m_pInfoService  ( new OPropertyInfoService )
    {
        DBG_CTOR( PropertyHandler, NULL );

        m_xTypeConverter = Reference< XTypeConverter >(
            m_aContext.createComponent( "com.sun.star.script.Converter" ),
            UNO_QUERY_THROW
        );
    }

    //--------------------------------------------------------------------
    PropertyHandler::PropertyHandler( const Reference< XMultiServiceFactory >& _rxLegacyFactory )
        :PropertyHandler_Base( m_aMutex )
        ,m_bSupportedPropertiesAreKnown( false )
        ,m_aPropertyListeners( m_aMutex )
        ,m_aContext( _rxLegacyFactory )
        ,m_pInfoService  ( new OPropertyInfoService )
    {
        DBG_CTOR( PropertyHandler, NULL );

        m_xTypeConverter = Reference< XTypeConverter >(
            m_aContext.createComponent( "com.sun.star.script.Converter" ),
            UNO_QUERY_THROW
        );
    }

    //--------------------------------------------------------------------
    PropertyHandler::~PropertyHandler()
    {
        DBG_DTOR( PropertyHandler, NULL );
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyHandler::inspect( const Reference< XInterface >& _rxIntrospectee ) throw (RuntimeException, NullPointerException)
    {
        if ( !_rxIntrospectee.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );

        Reference< XPropertySet > xNewComponent( _rxIntrospectee, UNO_QUERY );
        if ( xNewComponent == m_xComponent )
            return;

        // remove all old property change listeners
        ::std::auto_ptr< ::cppu::OInterfaceIteratorHelper > removeListener = m_aPropertyListeners.createIterator();
        ::std::auto_ptr< ::cppu::OInterfaceIteratorHelper > readdListener = m_aPropertyListeners.createIterator();  // will copy the container as needed
        while ( removeListener->hasMoreElements() )
            removePropertyChangeListener( static_cast< XPropertyChangeListener* >( removeListener->next() ) );
        OSL_ENSURE( m_aPropertyListeners.empty(), "PropertyHandler::inspect: derived classes are expected to forward the removePropertyChangeListener call to their base class (me)!" );

        // remember the new component, and give derived classes the chance to react on it
        m_xComponent = xNewComponent;
        onNewComponent();

        // add the listeners, again
        while ( readdListener->hasMoreElements() )
            addPropertyChangeListener( static_cast< XPropertyChangeListener* >( readdListener->next() ) );
    }

    //--------------------------------------------------------------------
    void PropertyHandler::onNewComponent()
    {
        if ( m_xComponent.is() )
            m_xComponentPropertyInfo = m_xComponent->getPropertySetInfo();
        else
            m_xComponentPropertyInfo.clear();

        m_bSupportedPropertiesAreKnown = false;
        m_aSupportedProperties.realloc( 0 );
    }

    //--------------------------------------------------------------------
    Sequence< Property > SAL_CALL PropertyHandler::getSupportedProperties() throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_bSupportedPropertiesAreKnown )
        {
            m_aSupportedProperties = doDescribeSupportedProperties();
            m_bSupportedPropertiesAreKnown = true;
        }
        return (Sequence< Property >)m_aSupportedProperties;
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL PropertyHandler::getSupersededProperties( ) throw (RuntimeException)
    {
        return Sequence< ::rtl::OUString >();
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL PropertyHandler::getActuatingProperties( ) throw (RuntimeException)
    {
        return Sequence< ::rtl::OUString >();
    }

    //--------------------------------------------------------------------
    Any SAL_CALL PropertyHandler::convertToPropertyValue( const ::rtl::OUString& _rPropertyName, const Any& _rControlValue ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId = m_pInfoService->getPropertyId( _rPropertyName );
        Property aProperty( impl_getPropertyFromName_throw( _rPropertyName ) );

        Any aPropertyValue;
        if ( !_rControlValue.hasValue() )
            // NULL is converted to NULL
            return aPropertyValue;

        if ( ( m_pInfoService->getPropertyUIFlags( nPropId ) & PROP_FLAG_ENUM ) != 0 )
        {
            ::rtl::OUString sControlValue;
            OSL_VERIFY( _rControlValue >>= sControlValue );
            ::rtl::Reference< IPropertyEnumRepresentation > aEnumConversion(
                new DefaultEnumRepresentation( *m_pInfoService, aProperty.Type, nPropId ) );
            // TODO/UNOize: cache those converters?
            aEnumConversion->getValueFromDescription( sControlValue, aPropertyValue );
        }
        else
            aPropertyValue = PropertyHandlerHelper::convertToPropertyValue(
                m_aContext.getContext(),m_xTypeConverter, aProperty, _rControlValue );
        return aPropertyValue;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL PropertyHandler::convertToControlValue( const ::rtl::OUString& _rPropertyName, const Any& _rPropertyValue, const Type& _rControlValueType ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId = m_pInfoService->getPropertyId( _rPropertyName );

        if ( ( m_pInfoService->getPropertyUIFlags( nPropId ) & PROP_FLAG_ENUM ) != 0 )
        {
            DBG_ASSERT( _rControlValueType.getTypeClass() == TypeClass_STRING, "PropertyHandler::convertToControlValue: ENUM, but not STRING?" );

            ::rtl::Reference< IPropertyEnumRepresentation > aEnumConversion(
                new DefaultEnumRepresentation( *m_pInfoService, _rPropertyValue.getValueType(), nPropId ) );
            // TODO/UNOize: cache those converters?
            return makeAny( aEnumConversion->getDescriptionForValue( _rPropertyValue ) );
        }

        return PropertyHandlerHelper::convertToControlValue(
            m_aContext.getContext(),m_xTypeConverter, _rPropertyValue, _rControlValueType );
    }

    //--------------------------------------------------------------------
    PropertyState SAL_CALL PropertyHandler::getPropertyState( const ::rtl::OUString& /*_rPropertyName*/ ) throw (UnknownPropertyException, RuntimeException)
    {
        return PropertyState_DIRECT_VALUE;
    }

    //--------------------------------------------------------------------
    LineDescriptor SAL_CALL PropertyHandler::describePropertyLine( const ::rtl::OUString& _rPropertyName,
        const Reference< XPropertyControlFactory >& _rxControlFactory )
        throw (UnknownPropertyException, NullPointerException, RuntimeException)
    {
        if ( !_rxControlFactory.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throw( _rPropertyName ) );
        const Property& rProperty( impl_getPropertyFromId_throw( nPropId ) );

        LineDescriptor aDescriptor;
        if ( ( m_pInfoService->getPropertyUIFlags( nPropId ) & PROP_FLAG_ENUM ) != 0 )
        {
            aDescriptor.Control = PropertyHandlerHelper::createListBoxControl(
                _rxControlFactory, m_pInfoService->getPropertyEnumRepresentations( nPropId ),
                PropertyHandlerHelper::requiresReadOnlyControl( rProperty.Attributes ), sal_False );
        }
        else
            PropertyHandlerHelper::describePropertyLine( rProperty, aDescriptor, _rxControlFactory );

        aDescriptor.HelpURL = HelpIdUrl::getHelpURL( m_pInfoService->getPropertyHelpId( nPropId ) );
        aDescriptor.DisplayName = m_pInfoService->getPropertyTranslation( nPropId );

        if ( ( m_pInfoService->getPropertyUIFlags( nPropId ) & PROP_FLAG_DATA_PROPERTY ) != 0 )
            aDescriptor.Category = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Data" ) );
        else
            aDescriptor.Category = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "General" ) );
        return aDescriptor;
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL PropertyHandler::isComposable( const ::rtl::OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return m_pInfoService->isComposeable( _rPropertyName );
    }

    //--------------------------------------------------------------------
    InteractiveSelectionResult SAL_CALL PropertyHandler::onInteractivePropertySelection( const ::rtl::OUString& /*_rPropertyName*/, sal_Bool /*_bPrimary*/, Any& /*_rData*/, const Reference< XObjectInspectorUI >& /*_rxInspectorUI*/ ) throw (UnknownPropertyException, NullPointerException, RuntimeException)
    {
        DBG_ERROR( "PropertyHandler::onInteractivePropertySelection: not implemented!" );
        return InteractiveSelectionResult_Cancelled;
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyHandler::actuatingPropertyChanged( const ::rtl::OUString& /*_rActuatingPropertyName*/, const Any& /*_rNewValue*/, const Any& /*_rOldValue*/, const Reference< XObjectInspectorUI >& /*_rxInspectorUI*/, sal_Bool /*_bFirstTimeInit*/ ) throw (NullPointerException, RuntimeException)
    {
        DBG_ERROR( "PropertyHandler::actuatingPropertyChanged: not implemented!" );
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyHandler::addPropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !_rxListener.is() )
            throw NullPointerException();
        m_aPropertyListeners.addListener( _rxListener );
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyHandler::removePropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_aPropertyListeners.removeListener( _rxListener );
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL PropertyHandler::suspend( sal_Bool /*_bSuspend*/ ) throw (RuntimeException)
    {
        return sal_True;
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XCOMPONENT( PropertyHandler, PropertyHandler_Base )
    //--------------------------------------------------------------------
    void SAL_CALL PropertyHandler::disposing()
    {
        m_xComponent.clear();
        m_aPropertyListeners.clear();
        m_xTypeConverter.clear();
        m_aSupportedProperties.realloc( 0 );
    }

    //--------------------------------------------------------------------
    void PropertyHandler::firePropertyChange( const ::rtl::OUString& _rPropName, PropertyId _nPropId, const Any& _rOldValue, const Any& _rNewValue ) SAL_THROW(())
    {
        PropertyChangeEvent aEvent;
        aEvent.Source = m_xComponent;
        aEvent.PropertyHandle = _nPropId;
        aEvent.PropertyName = _rPropName;
        aEvent.OldValue = _rOldValue;
        aEvent.NewValue = _rNewValue;
        m_aPropertyListeners.notify( aEvent, &XPropertyChangeListener::propertyChange );
    }

    //--------------------------------------------------------------------
    const Property* PropertyHandler::impl_getPropertyFromId_nothrow( PropertyId _nPropId ) const
    {
        const_cast< PropertyHandler* >( this )->getSupportedProperties();
        const Property* pFound = ::std::find_if( m_aSupportedProperties.begin(), m_aSupportedProperties.end(),
            FindPropertyByHandle( _nPropId )
        );
        if ( pFound != m_aSupportedProperties.end() )
            return &(*pFound);
        return NULL;
    }

    //--------------------------------------------------------------------
    const Property& PropertyHandler::impl_getPropertyFromId_throw( PropertyId _nPropId ) const
    {
        const Property* pProperty = impl_getPropertyFromId_nothrow( _nPropId );
        if ( !pProperty )
            throw UnknownPropertyException();

        return *pProperty;
    }

    //--------------------------------------------------------------------
    const Property& PropertyHandler::impl_getPropertyFromName_throw( const ::rtl::OUString& _rPropertyName ) const
    {
        const_cast< PropertyHandler* >( this )->getSupportedProperties();
        StlSyntaxSequence< Property >::const_iterator pFound = ::std::find_if( m_aSupportedProperties.begin(), m_aSupportedProperties.end(),
            FindPropertyByName( _rPropertyName )
        );
        if ( pFound == m_aSupportedProperties.end() )
            throw UnknownPropertyException();

        return *pFound;
    }

    //--------------------------------------------------------------------
    void PropertyHandler::implAddPropertyDescription( ::std::vector< Property >& _rProperties, const ::rtl::OUString& _rPropertyName, const Type& _rType, sal_Int16 _nAttribs ) const
    {
        _rProperties.push_back( Property(
            _rPropertyName,
            m_pInfoService->getPropertyId( _rPropertyName ),
            _rType,
            _nAttribs
        ) );
    }

    //------------------------------------------------------------------------
    Window* PropertyHandler::impl_getDefaultDialogParent_nothrow() const
    {
        return PropertyHandlerHelper::getDialogParentWindow( m_aContext );
    }

    //------------------------------------------------------------------------
    PropertyId PropertyHandler::impl_getPropertyId_throw( const ::rtl::OUString& _rPropertyName ) const
    {
        PropertyId nPropId = m_pInfoService->getPropertyId( _rPropertyName );
        if ( nPropId == -1 )
            throw UnknownPropertyException();
        return nPropId;
    }

    //------------------------------------------------------------------------
    void PropertyHandler::impl_setContextDocumentModified_nothrow() const
    {
        Reference< XModifiable > xModifiable( impl_getContextDocument_nothrow(), UNO_QUERY );
        if ( xModifiable.is() )
            xModifiable->setModified( sal_True );
    }

    //------------------------------------------------------------------------
    bool PropertyHandler::impl_componentHasProperty_throw( const ::rtl::OUString& _rPropName ) const
    {
        return m_xComponentPropertyInfo.is() && m_xComponentPropertyInfo->hasPropertyByName( _rPropName );
    }

    //====================================================================
    //= PropertyHandlerComponent
    //====================================================================
    //------------------------------------------------------------------------
    PropertyHandlerComponent::PropertyHandlerComponent( const Reference< XComponentContext >& _rxContext )
        :PropertyHandler( _rxContext )
    {
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( PropertyHandlerComponent, PropertyHandler, PropertyHandlerComponent_Base )
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( PropertyHandlerComponent, PropertyHandler, PropertyHandlerComponent_Base )

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL PropertyHandlerComponent::supportsService( const ::rtl::OUString& ServiceName ) throw (RuntimeException)
    {
        StlSyntaxSequence< ::rtl::OUString > aAllServices( getSupportedServiceNames() );
        return ::std::find( aAllServices.begin(), aAllServices.end(), ServiceName ) != aAllServices.end();
    }

//........................................................................
}   // namespace pcr
//........................................................................

