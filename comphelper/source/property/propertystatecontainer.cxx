/*************************************************************************
 *
 *  $RCSfile: propertystatecontainer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:52:11 $
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

#ifndef COMPHELPER_PROPERTYSTATECONTAINER_HXX
#include "comphelper/propertystatecontainer.hxx"
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

//.........................................................................
namespace comphelper
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;

    namespace
    {
        static ::rtl::OUString lcl_getUnknownPropertyErrorMessage( const ::rtl::OUString& _rPropertyName )
        {
            // TODO: perhaps it's time to think about resources in the comphelper module?
            // Would be nice to have localized exception strings (a simply resource file containing
            // strings only would suffice, and could be realized with an UNO service, so we do not
            // need the dependency to the Tools project)
            ::rtl::OUStringBuffer sMessage;
            sMessage.appendAscii( "The property \"" );
            sMessage.append( _rPropertyName );
            sMessage.appendAscii( "\" is unknown." );
            return sMessage.makeStringAndClear();
        }
    }

    //=====================================================================
    //= OPropertyStateContainer
    //=====================================================================
    //---------------------------------------------------------------------
    OPropertyStateContainer::OPropertyStateContainer( ::cppu::OBroadcastHelper& _rBHelper )
        :OPropertyContainer( _rBHelper )
    {
    }

    //--------------------------------------------------------------------
    Any SAL_CALL OPropertyStateContainer::queryInterface( const Type& _rType ) throw (RuntimeException)
    {
        Any aReturn = OPropertyContainer::queryInterface( _rType );
        if ( !aReturn.hasValue() )
            aReturn = OPropertyStateContainer_TBase::queryInterface( _rType );
        return aReturn;
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OPropertyStateContainer, OPropertyContainer, OPropertyStateContainer_TBase )

    //--------------------------------------------------------------------
    sal_Int32 OPropertyStateContainer::getHandleForName( const ::rtl::OUString& _rPropertyName ) SAL_THROW( ( UnknownPropertyException ) )
    {
        // look up the handle for the name
        ::cppu::IPropertyArrayHelper& rPH = getInfoHelper();
        sal_Int32 nHandle = rPH.getHandleByName( _rPropertyName );

        if ( -1 == nHandle )
            throw  UnknownPropertyException( lcl_getUnknownPropertyErrorMessage( _rPropertyName ), static_cast< XPropertyState* >( this ) );

        return nHandle;
    }

    //--------------------------------------------------------------------
    PropertyState SAL_CALL OPropertyStateContainer::getPropertyState( const ::rtl::OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException)
    {
        return getPropertyStateByHandle( getHandleForName( _rPropertyName ) );
    }

    //--------------------------------------------------------------------
    Sequence< PropertyState > SAL_CALL OPropertyStateContainer::getPropertyStates( const Sequence< ::rtl::OUString >& _rPropertyNames ) throw (UnknownPropertyException, RuntimeException)
    {
        sal_Int32 nProperties = _rPropertyNames.getLength();
        Sequence< PropertyState> aStates( nProperties );
        if ( !nProperties )
            return aStates;

#ifdef _DEBUG
        // precondition: property sequence is sorted (the algorythm below relies on this)
        {
            const ::rtl::OUString* pNames = _rPropertyNames.getConstArray();
            const ::rtl::OUString* pNamesCompare = pNames + 1;
            const ::rtl::OUString* pNamesEnd = _rPropertyNames.getConstArray() + _rPropertyNames.getLength();
            for ( ; pNamesCompare != pNamesEnd; ++pNames, ++pNamesCompare )
                OSL_PRECOND( pNames->compareTo( *pNamesCompare ) < 0,
                    "OPropertyStateContainer::getPropertyStates: property sequence not sorted!" );
        }
#endif

        const ::rtl::OUString* pLookup = _rPropertyNames.getConstArray();
        const ::rtl::OUString* pLookupEnd = pLookup + nProperties;
        PropertyState* pStates = aStates.getArray();

        cppu::IPropertyArrayHelper& rHelper = getInfoHelper();
        Sequence< Property> aAllProperties  = rHelper.getProperties();
        sal_Int32 nAllProperties            = aAllProperties.getLength();
        const  Property* pAllProperties     = aAllProperties.getConstArray();
        const  Property* pAllPropertiesEnd  = pAllProperties + nAllProperties;

        osl::MutexGuard aGuard( rBHelper.rMutex );
        for ( ; ( pAllProperties != pAllPropertiesEnd ) && ( pLookup != pLookupEnd ); ++pAllProperties )
        {
#ifdef _DEBUG
            if ( pAllProperties < pAllPropertiesEnd - 1 )
                OSL_ENSURE( pAllProperties->Name.compareTo( (pAllProperties + 1)->Name ) < 0,
                    "OPropertyStateContainer::getPropertyStates: all-properties not sorted!" );
#endif
            if ( pAllProperties->Name.equals( *pLookup ) )
            {
                *pStates++ = getPropertyState( *pLookup );
                ++pLookup;
            }
        }

        if ( pLookup != pLookupEnd )
            // we run out of properties from the IPropertyArrayHelper, but still have properties to lookup
            // -> we were asked for a nonexistent property
            throw UnknownPropertyException( lcl_getUnknownPropertyErrorMessage( *pLookup ), static_cast< XPropertyState* >( this ) );

        return aStates;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OPropertyStateContainer::setPropertyToDefault( const ::rtl::OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException)
    {
        setPropertyToDefaultByHandle( getHandleForName( _rPropertyName ) );
    }

    //--------------------------------------------------------------------
    Any SAL_CALL OPropertyStateContainer::getPropertyDefault( const ::rtl::OUString& _rPropertyName ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
    {
        return getPropertyDefaultByHandle( getHandleForName( _rPropertyName ) );
    }

    //--------------------------------------------------------------------
    PropertyState OPropertyStateContainer::getPropertyStateByHandle( sal_Int32 _nHandle )
    {
        // simply compare the current and the default value
        Any aCurrentValue; getFastPropertyValue( aCurrentValue, _nHandle );
        Any aDefaultValue = getPropertyDefaultByHandle( _nHandle );

        sal_Bool bEqual = uno_type_equalData(
                const_cast< void* >( aCurrentValue.getValue() ), aCurrentValue.getValueType().getTypeLibType(),
                const_cast< void* >( aDefaultValue.getValue() ), aDefaultValue.getValueType().getTypeLibType(),
                cpp_queryInterface, cpp_release
            );
        if ( bEqual )
            return PropertyState_DEFAULT_VALUE;
        else
            return PropertyState_DIRECT_VALUE;
    }

    //--------------------------------------------------------------------
    void OPropertyStateContainer::setPropertyToDefaultByHandle( sal_Int32 _nHandle )
    {
        setFastPropertyValue( _nHandle, getPropertyDefaultByHandle( _nHandle ) );
    }

//.........................................................................
}   // namespace comphelper
//.........................................................................

#ifdef FS_PRIV_DEBUG
#define STATECONTAINER_TEST
#endif

#ifdef STATECONTAINER_TEST

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif

//.........................................................................
namespace comphelper
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;

    //=====================================================================
    //= Test - compiler test
    //=====================================================================
    typedef ::cppu::OWeakAggObject  Test_RefCountBase;
    class Test  :public OMutexAndBroadcastHelper
                ,public OPropertyStateContainer
                ,public OPropertyArrayUsageHelper< Test >
                ,public Test_RefCountBase
    {
    private:
        ::rtl::OUString         m_sStringProperty;
        Reference< XInterface > m_xInterfaceProperty;
        Any                     m_aMayBeVoidProperty;

    protected:
        Test( );

        DECLARE_XINTERFACE( )

    public:
        static Test* Create( );

    protected:
        virtual Reference< XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    protected:
        // OPropertyStateContainer overridables
        virtual Any getPropertyDefaultByHandle( sal_Int32 _nHandle ) const;
    };

    //---------------------------------------------------------------------
    Test::Test( )
        :OPropertyStateContainer( GetBroadcastHelper() )
    {
        registerProperty(
            ::rtl::OUString::createFromAscii( "StringProperty" ),
            1,
            PropertyAttribute::BOUND,
            &m_sStringProperty,
            ::getCppuType( &m_sStringProperty )
        );

        registerProperty(
            ::rtl::OUString::createFromAscii( "InterfaceProperty" ),
            2,
            PropertyAttribute::BOUND,
            &m_xInterfaceProperty,
            ::getCppuType( &m_xInterfaceProperty )
        );

        registerMayBeVoidProperty(
            ::rtl::OUString::createFromAscii( "IntProperty" ),
            3,
            PropertyAttribute::BOUND,
            &m_aMayBeVoidProperty,
            ::getCppuType( static_cast< sal_Int32* >( NULL ) )
        );

        registerPropertyNoMember(
            ::rtl::OUString::createFromAscii( "OtherInterfaceProperty" ),
            4,
            PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
            ::getCppuType( static_cast< Reference< XInterface >* >( NULL ) ),
            NULL
        );
    }

    //---------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( Test, Test_RefCountBase, OPropertyStateContainer )

    //---------------------------------------------------------------------
    Any Test::getPropertyDefaultByHandle( sal_Int32 _nHandle ) const
    {
        Any aDefault;
        switch ( _nHandle )
        {
            case 1:
                aDefault = makeAny( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StringPropertyDefault" ) ) );
                break;
            case 2:
                aDefault = makeAny( Reference< XInterface >( ) );
                break;
            case 3:
                // void
                break;
            case 4:
                aDefault = makeAny( Reference< XInterface >( ) );
                break;
            default:
                OSL_ENSURE( sal_False, "Test::getPropertyDefaultByHandle: invalid handle!" );
        }
        return aDefault;
    }

    //---------------------------------------------------------------------
    Reference< XPropertySetInfo > SAL_CALL Test::getPropertySetInfo(  ) throw(RuntimeException)
    {
        return createPropertySetInfo( getInfoHelper() );
    }

    //---------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& SAL_CALL Test::getInfoHelper()
    {
        return *getArrayHelper();
    }

    //---------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* Test::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );
        return new ::cppu::OPropertyArrayHelper( aProps );
    }

    //---------------------------------------------------------------------
    Test* Test::Create( )
    {
        Test* pInstance = new Test;
        return pInstance;
    }

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif

