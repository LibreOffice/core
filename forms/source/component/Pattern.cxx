/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Pattern.cxx,v $
 * $Revision: 1.18 $
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

#include "Pattern.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

//.........................................................................
namespace frm
{
//.........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::sdbc::XRowSet;
    using ::com::sun::star::uno::UNO_QUERY;
    /** === end UNO using === **/
    namespace FormComponentType = ::com::sun::star::form::FormComponentType;

//==================================================================
// OPatternControl
//==================================================================
//------------------------------------------------------------------
OPatternControl::OPatternControl(const Reference<XMultiServiceFactory>& _rxFactory)
    :OBoundControl(_rxFactory, VCL_CONTROL_PATTERNFIELD)
{
}

//------------------------------------------------------------------
InterfaceRef SAL_CALL OPatternControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OPatternControl(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> OPatternControl::_getTypes()
{
    return OBoundControl::_getTypes();
}

//------------------------------------------------------------------------------
StringSequence OPatternControl::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_PATTERNFIELD;
    return aSupported;
}

//==================================================================
// OPatternModel
//==================================================================
//------------------------------------------------------------------
InterfaceRef SAL_CALL OPatternModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OPatternModel(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> OPatternModel::_getTypes()
{
    return OEditBaseModel::_getTypes();
}

//------------------------------------------------------------------
DBG_NAME( OPatternModel )
//------------------------------------------------------------------
OPatternModel::OPatternModel(const Reference<XMultiServiceFactory>& _rxFactory)
    :OEditBaseModel( _rxFactory, VCL_CONTROLMODEL_PATTERNFIELD, FRM_SUN_CONTROL_PATTERNFIELD, sal_False, sal_False )
                                    // use the old control name for compytibility reasons
{
    DBG_CTOR( OPatternModel, NULL );

    m_nClassId = FormComponentType::PATTERNFIELD;
    initValueProperty( PROPERTY_TEXT, PROPERTY_ID_TEXT );
}

//------------------------------------------------------------------
OPatternModel::OPatternModel( const OPatternModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory )
    :OEditBaseModel( _pOriginal, _rxFactory )
{
    DBG_CTOR( OPatternModel, NULL );
}

//------------------------------------------------------------------
OPatternModel::~OPatternModel()
{
    DBG_DTOR( OPatternModel, NULL );
}

// XCloneable
//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( OPatternModel )

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL OPatternModel::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = FRM_SUN_COMPONENT_DATABASE_PATTERNFIELD;
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_PATTERNFIELD;
    return aSupported;
}


//------------------------------------------------------------------------------
void OPatternModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 4, OEditBaseModel )
        DECL_PROP2(DEFAULT_TEXT,    ::rtl::OUString,    BOUND, MAYBEDEFAULT);
        DECL_BOOL_PROP1(EMPTY_IS_NULL,                  BOUND);
        DECL_PROP1(TABINDEX,        sal_Int16,          BOUND);
        DECL_PROP2(FILTERPROPOSAL,  sal_Bool,           BOUND, MAYBEDEFAULT);
    END_DESCRIBE_PROPERTIES();
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OPatternModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException)
{
    return FRM_COMPONENT_PATTERNFIELD;  // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
sal_Bool OPatternModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
{
    ::rtl::OUString sNewValue;
    m_xAggregateFastSet->getFastPropertyValue( getValuePropertyAggHandle() ) >>= sNewValue;

    if ( sNewValue != m_aSaveValue )
    {
        if ( !sNewValue.getLength() && !isRequired() && m_bEmptyIsNull )
            m_xColumnUpdate->updateNull();
        else
        {
            OSL_ENSURE( m_pFormattedValue.get(), "OPatternModel::commitControlValueToDbColumn: no value helper!" );
            if ( !m_pFormattedValue.get() )
                return sal_False;

            if ( !m_pFormattedValue->setFormattedValue( sNewValue ) )
                return sal_False;
        }
        m_aSaveValue = sNewValue;
    }
    return sal_True;
}

//------------------------------------------------------------------------------
void OPatternModel::onConnectedDbColumn( const Reference< XInterface >& _rxForm )
{
    OEditBaseModel::onConnectedDbColumn( _rxForm );

    Reference< XPropertySet > xField( getField() );
    if ( !xField.is() )
        return;

    m_pFormattedValue.reset( new ::dbtools::FormattedColumnValue( getContext(), Reference< XRowSet >( _rxForm, UNO_QUERY ), xField ) );
}

//------------------------------------------------------------------------------
void OPatternModel::onDisconnectedDbColumn()
{
    OEditBaseModel::onDisconnectedDbColumn();
    m_pFormattedValue.reset();
}

// XPropertyChangeListener
//------------------------------------------------------------------------------
Any OPatternModel::translateDbColumnToControlValue()
{
    OSL_PRECOND( m_pFormattedValue.get(), "OPatternModel::translateDbColumnToControlValue: no value helper!" );
    if ( m_pFormattedValue.get() )
        m_aSaveValue = m_pFormattedValue->getFormattedValue();
    else
        m_aSaveValue = ::rtl::OUString();
    return makeAny( m_aSaveValue );
}

// XReset
//------------------------------------------------------------------------------
Any OPatternModel::getDefaultForReset() const
{
    return makeAny( m_aDefaultText );
}

//.........................................................................
}   // namespace frm
//.........................................................................

