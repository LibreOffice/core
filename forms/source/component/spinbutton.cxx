/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: spinbutton.cxx,v $
 * $Revision: 1.10 $
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
#include "precompiled_forms.hxx"
#include "spinbutton.hxx"
#include <comphelper/streamsection.hxx>
#include <comphelper/basicio.hxx>

//--------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_OSpinButtonModel()
{
    static ::frm::OMultiInstanceAutoRegistration< ::frm::OSpinButtonModel >   aRegisterModel;
}

//........................................................................
namespace frm
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::io;
    using namespace ::com::sun::star::form::binding;

    //====================================================================
    //= OSpinButtonModel
    //====================================================================
    // implemented elsewhere
    Any translateExternalDoubleToControlIntValue(
        const Reference< XValueBinding >& _rxBinding, const Reference< XPropertySet >& _rxProperties,
        const ::rtl::OUString& _rMinValueName, const ::rtl::OUString& _rMaxValueName );
    Any translateControlIntToExternalDoubleValue( const Any& _rControlIntValue );

    //====================================================================
    //= OSpinButtonModel
    //====================================================================
    //--------------------------------------------------------------------
    DBG_NAME( OSpinButtonModel )
    //--------------------------------------------------------------------
    OSpinButtonModel::OSpinButtonModel( const Reference<XMultiServiceFactory>& _rxFactory )
        :OBoundControlModel( _rxFactory, VCL_CONTROLMODEL_SPINBUTTON, VCL_CONTROL_SPINBUTTON, sal_True, sal_True, sal_False )
        ,m_nDefaultSpinValue( 0 )
    {
        DBG_CTOR( OSpinButtonModel, NULL );

        m_nClassId = FormComponentType::SPINBUTTON;
        initValueProperty( PROPERTY_SPIN_VALUE, PROPERTY_ID_SPIN_VALUE );
    }

    //--------------------------------------------------------------------
    OSpinButtonModel::OSpinButtonModel( const OSpinButtonModel* _pOriginal, const Reference< XMultiServiceFactory >& _rxFactory )
        :OBoundControlModel( _pOriginal, _rxFactory )
    {
        DBG_CTOR( OSpinButtonModel, NULL );
        m_nDefaultSpinValue = _pOriginal->m_nDefaultSpinValue;
    }

    //--------------------------------------------------------------------
    OSpinButtonModel::~OSpinButtonModel( )
    {
        DBG_DTOR( OSpinButtonModel, NULL );
    }

    //--------------------------------------------------------------------
    IMPLEMENT_SERVICE_REGISTRATION_2( OSpinButtonModel, OControlModel, FRM_SUN_COMPONENT_SPINBUTTON, BINDABLE_INTEGER_VALUE_RANGE )
        // note that we're passing OControlModel as "base class". This is because
        // OBoundControlModel, our real base class, claims to support the DataAwareControlModel
        // service, which isn't really true for us. We only derive from this class
        // to benefit from the functionality for binding to spreadsheet cells

    //------------------------------------------------------------------------------
    IMPLEMENT_DEFAULT_CLONING( OSpinButtonModel )

    //------------------------------------------------------------------------------
    void SAL_CALL OSpinButtonModel::disposing()
    {
        OBoundControlModel::disposing();
    }

    //--------------------------------------------------------------------
    void OSpinButtonModel::describeFixedProperties( Sequence< Property >& _rProps ) const
    {
        BEGIN_DESCRIBE_PROPERTIES( 3, OControlModel )
            DECL_PROP1( DEFAULT_SPIN_VALUE,   sal_Int32,       BOUND );
            DECL_PROP1( TABINDEX,             sal_Int16,       BOUND );
            DECL_PROP2( CONTROLSOURCEPROPERTY,::rtl::OUString, READONLY, TRANSIENT );
        END_DESCRIBE_PROPERTIES();
    }

    //------------------------------------------------------------------------------
    void OSpinButtonModel::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
    {
        switch ( _nHandle )
        {
            case PROPERTY_ID_DEFAULT_SPIN_VALUE:
                _rValue <<= m_nDefaultSpinValue;
                break;

            default:
                OBoundControlModel::getFastPropertyValue( _rValue, _nHandle );
        }
    }

    //------------------------------------------------------------------------------
    void OSpinButtonModel::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue ) throw ( Exception )
    {
        switch ( _nHandle )
        {
            case PROPERTY_ID_DEFAULT_SPIN_VALUE:
                OSL_VERIFY( _rValue >>= m_nDefaultSpinValue );
                resetNoBroadcast();
                break;

            default:
                OBoundControlModel::setFastPropertyValue_NoBroadcast( _nHandle, _rValue );
        }
    }

    //------------------------------------------------------------------------------
    sal_Bool OSpinButtonModel::convertFastPropertyValue(
                Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue )
                throw ( IllegalArgumentException )
    {
        sal_Bool bModified( sal_False );
        switch ( _nHandle )
        {
            case PROPERTY_ID_DEFAULT_SPIN_VALUE:
                bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_nDefaultSpinValue );
                break;

            default:
                bModified = OBoundControlModel::convertFastPropertyValue( _rConvertedValue, _rOldValue, _nHandle, _rValue );
                break;
        }
        return bModified;
    }

    //--------------------------------------------------------------------
    Any OSpinButtonModel::getPropertyDefaultByHandle( sal_Int32 _nHandle ) const
    {
        Any aReturn;

        switch ( _nHandle )
        {
        case PROPERTY_ID_DEFAULT_SPIN_VALUE:
            aReturn <<= (sal_Int32)0;
            break;

        default:
            aReturn = OBoundControlModel::getPropertyDefaultByHandle( _nHandle );
            break;
        }

        return aReturn;
    }

    //------------------------------------------------------------------------------
    Any OSpinButtonModel::translateDbColumnToControlValue( )
    {
        OSL_ENSURE( sal_False, "OSpinButtonModel::commitControlValueToDbColumn: never to be called (we're not bound)!" );
        return Any();
    }

    //------------------------------------------------------------------------------
    sal_Bool OSpinButtonModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
    {
        OSL_ENSURE( sal_False, "OSpinButtonModel::commitControlValueToDbColumn: never to be called (we're not bound)!" );
        return sal_True;
    }

    //------------------------------------------------------------------------------
    Any OSpinButtonModel::getDefaultForReset() const
    {
        return makeAny( (sal_Int32)m_nDefaultSpinValue );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OSpinButtonModel::getServiceName() throw( RuntimeException )
    {
        return FRM_SUN_COMPONENT_SPINBUTTON;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OSpinButtonModel::write( const Reference< XObjectOutputStream >& _rxOutStream )
        throw( IOException, RuntimeException )
    {
        OBoundControlModel::write( _rxOutStream );
        ::osl::MutexGuard aGuard( m_aMutex );

        OStreamSection aSection( Reference< XDataOutputStream >( _rxOutStream, UNO_QUERY ) );

        // version
        _rxOutStream->writeShort( 0x0001 );

        // properties
        _rxOutStream << m_nDefaultSpinValue;
        writeHelpTextCompatibly( _rxOutStream );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OSpinButtonModel::read( const Reference< XObjectInputStream>& _rxInStream ) throw( IOException, RuntimeException )
    {
        OBoundControlModel::read( _rxInStream );
        ::osl::MutexGuard aGuard( m_aMutex );

        // version
        {
            OStreamSection aSection( Reference< XDataInputStream >( _rxInStream, UNO_QUERY ) );

            sal_uInt16 nVersion = _rxInStream->readShort();
            if ( nVersion == 0x0001 )
            {
                _rxInStream >> m_nDefaultSpinValue;
                readHelpTextCompatibly( _rxInStream );
            }
            else
                defaultCommonProperties();

            // here, everything in the stream section which is left will be skipped
        }
    }

    //--------------------------------------------------------------------
    Any OSpinButtonModel::translateExternalValueToControlValue( ) const
    {
        return translateExternalDoubleToControlIntValue( getExternalValueBinding(), m_xAggregateSet,
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SpinValueMin" ) ),
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SpinValueMax" ) ) );
    }

    //--------------------------------------------------------------------
    Any OSpinButtonModel::translateControlValueToExternalValue( ) const
    {
        // by definition, the base class simply obtains the property value
        return translateControlIntToExternalDoubleValue( OBoundControlModel::translateControlValueToExternalValue() );
    }

    //--------------------------------------------------------------------
    sal_Bool OSpinButtonModel::approveValueBinding( const Reference< XValueBinding >& _rxBinding )
    {
        OSL_PRECOND( _rxBinding.is(), "OSpinButtonModel::approveValueBinding: invalid binding!" );

        // only strings are accepted for simplicity
        return  _rxBinding.is()
            &&  _rxBinding->supportsType( ::getCppuType( static_cast< double* >( NULL ) ) );
    }

//........................................................................
}   // namespace frm
//........................................................................
