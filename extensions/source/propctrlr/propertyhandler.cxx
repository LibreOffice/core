/*************************************************************************
 *
 *  $RCSfile: propertyhandler.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 12:11:27 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYHANDLER_HXX
#include "propertyhandler.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_FORMMETADATA_HXX_
#include "formmetadata.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_STRINGREPRESENTATION_HXX
#include "stringrepresentation.hxx"
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_FORMBROWSERTOOLS_HXX_
#include "formbrowsertools.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
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
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::script;

    //====================================================================
    //= PropertyHandler
    //====================================================================
    //--------------------------------------------------------------------
    PropertyHandler::PropertyHandler( const Reference< XPropertySet >& _rxIntrospectee, const Reference< XTypeConverter >& _rxTypeConverter )
        :m_refCount( 0 )
        ,m_xIntrospectee( _rxIntrospectee )
        ,m_xTypeConverter( _rxTypeConverter )
        ,m_pInfoService  ( new OPropertyInfoService )
        ,m_bSupportedPropertiesAreKnown( false )
    {
    }

    //--------------------------------------------------------------------
    PropertyHandler::~PropertyHandler()
    {
    }

    //--------------------------------------------------------------------
    bool SAL_CALL PropertyHandler::supportsUIDescriptor( PropertyId _nPropId ) const
    {
        return false;
    }

    //--------------------------------------------------------------------
    ::std::vector< Property > SAL_CALL PropertyHandler::getSupportedProperties() const
    {
        if ( !m_bSupportedPropertiesAreKnown )
        {
            m_aSupportedProperties = implDescribeSupportedProperties();
            m_bSupportedPropertiesAreKnown = true;
        }
        return m_aSupportedProperties;
    }

    //--------------------------------------------------------------------
    ::std::vector< ::rtl::OUString > SAL_CALL PropertyHandler::getSupersededProperties( ) const
    {
        return ::std::vector< ::rtl::OUString >();
    }

    //--------------------------------------------------------------------
    ::std::vector< ::rtl::OUString > SAL_CALL PropertyHandler::getActuatingProperties( ) const
    {
        return ::std::vector< ::rtl::OUString >();
    }

    //--------------------------------------------------------------------
    Any SAL_CALL PropertyHandler::getPropertyValueFromStringRep( PropertyId _nPropId, const ::rtl::OUString& _rStringRep ) const
    {
        Any aReturn;

        const Property* pProp = getPropertyFromId( _nPropId );
        DBG_ASSERT( pProp, "PropertyHandler::getPropertyValueFromStringRep: this is not one of our supported properties!" );
        if ( pProp )
        {
            // empty strings are mapped to void, if possible
            if ( _rStringRep.getLength() || ( 0 == ( pProp->Attributes & PropertyAttribute::MAYBEVOID ) ) )
            {
                StringRepresentation aConversionHelper( m_xTypeConverter );
                aReturn = aConversionHelper.getPropertyValueFromStringRep( _rStringRep, pProp->Type, _nPropId, m_pInfoService.get() );
            }
        }

        return aReturn;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL PropertyHandler::getStringRepFromPropertyValue( PropertyId _nPropId, const Any& _rValue ) const
    {
        DBG_ASSERT( getPropertyFromId( _nPropId ), "PropertyHandler::getStringRepFromPropertyValue: this is not one of our supported properties!" );

        StringRepresentation aConversionHelper( m_xTypeConverter );
        return aConversionHelper.getStringRepFromPropertyValue( _rValue, _nPropId, m_pInfoService.get() );
    }

    //--------------------------------------------------------------------
    PropertyState SAL_CALL PropertyHandler::getPropertyState( PropertyId _nPropId ) const
    {
        return PropertyState_DIRECT_VALUE;
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyHandler::describePropertyUI( PropertyId _nPropId, PropertyUIDescriptor& /* [out] */ _rDescriptor ) const
    {
        DBG_ERROR( "PropertyHandler::describePropertyUI: not supported!" );
            // we said "no" in supportsUIDescriptor ....
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyHandler::initializePropertyUI( PropertyId _nPropId, IPropertyBrowserUI* _pUpdater )
    {
        DBG_ERROR( "PropertyHandler::initializePropertyUI: not supported!" );
            // we said "no" in supportsUIDescriptor ....
    }

    //--------------------------------------------------------------------
    bool SAL_CALL PropertyHandler::requestUserInputOnButtonClick( PropertyId _nPropId, bool _bPrimary, Any& _rData )
    {
        DBG_ERROR( "PropertyHandler::requestUserInputOnButtonClick: not supported!" );
            // we said "no" in supportsUIDescriptor ....
        return false;
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyHandler::executeButtonClick( PropertyId _nPropId, bool _bPrimary, const Any& _rData, IPropertyBrowserUI* _pUpdater )
    {
        DBG_ERROR( "PropertyHandler::executeButtonClick: not supported!" );
            // we said "no" in supportsUIDescriptor ....
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyHandler::updateDependentProperties( PropertyId _nActuatingPropId, const Any& _rNewValue, const Any& _rOldValue, IPropertyBrowserUI* _pUpdater )
    {
        DBG_ERROR( "PropertyHandler::updateDependentProperties: not supported!" );
            // we did not return any properties in getActuatingProperties
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyHandler::startAllPropertyChangeListening( const Reference< XPropertyChangeListener >& _rxListener )
    {
        DBG_ASSERT( !m_xTheListener.is(), "PropertyHandler::startAllPropertyChangeListening: there already is a listener!" );
        DBG_ASSERT( _rxListener.is(), "PropertyHandler::startAllPropertyChangeListening: nonsense!" );
        m_xTheListener = _rxListener;
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyHandler::stopAllPropertyChangeListening( )
    {
        m_xTheListener.clear();
    }

    //--------------------------------------------------------------------
    void PropertyHandler::firePropertyChange( const ::rtl::OUString& _rPropName, PropertyId _nPropId, const Any& _rOldValue, const Any& _rNewValue ) SAL_THROW(())
    {
        if ( m_xTheListener.is() )
        {
            try
            {
                PropertyChangeEvent aEvent;
                aEvent.Source = m_xIntrospectee.get();
                aEvent.PropertyHandle = _nPropId;
                aEvent.PropertyName = _rPropName;
                aEvent.OldValue = _rOldValue;
                aEvent.NewValue = _rNewValue;
                m_xTheListener->propertyChange( aEvent );
            }
            catch( const Exception& )
            {
                OSL_ENSURE( sal_False, "PropertyHandler::firePropertyChange: caught an exception!" );
            }
        }
    }

    //--------------------------------------------------------------------
    const Property* PropertyHandler::getPropertyFromId( PropertyId _nPropId ) const
    {
        getSupportedProperties();
        ::std::vector< Property >::const_iterator pFound = ::std::find_if( m_aSupportedProperties.begin(), m_aSupportedProperties.end(),
            FindPropertyByHandle( _nPropId )
        );
        if ( pFound != m_aSupportedProperties.end() )
            return pFound;
        return NULL;
    }

    //--------------------------------------------------------------------
    void PropertyHandler::changeTypeOfSupportedProperty( const sal_Int32 _nPropId, const Type& _rNewType )
    {
        OSL_ENSURE( m_bSupportedPropertiesAreKnown, "PropertyHandler::changeTypeOfSupportedProperty: getSupportedProperties has never been called!" );
            // if this asserts, and the caller has a valid reason, then simply call getSupportedProperties once

        ::std::vector< Property >::iterator pProp = ::std::find_if(
            m_aSupportedProperties.begin(),
            m_aSupportedProperties.end(),
            FindPropertyByHandle( _nPropId )
        );
        OSL_ENSURE( pProp, "PropertyHandler::changeTypeOfSupportedProperty: This property is not known!" );
        if ( pProp )
            pProp->Type = _rNewType;
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

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL PropertyHandler::acquire()
    {
        return osl_incrementInterlockedCount( &m_refCount );
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL PropertyHandler::release()
    {
        if ( 0 == osl_decrementInterlockedCount( &m_refCount ) )
        {
           delete this;
           return 0;
        }
        return m_refCount;
    }


//........................................................................
}   // namespace pcr
//........................................................................

