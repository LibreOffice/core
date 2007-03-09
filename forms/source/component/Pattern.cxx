/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Pattern.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: obo $ $Date: 2007-03-09 13:30:52 $
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
#include "precompiled_forms.hxx"

#ifndef _FORMS_PATTERN_HXX_
#include "Pattern.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//.........................................................................
namespace frm
{
//.........................................................................
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

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
            try
            {
                m_xColumnUpdate->updateString( sNewValue );
            }
            catch(Exception&)
            {
                return sal_False;
            }
        }
        m_aSaveValue = sNewValue;
    }
    return sal_True;
}

// XPropertyChangeListener
//------------------------------------------------------------------------------
Any OPatternModel::translateDbColumnToControlValue()
{
    m_aSaveValue = m_xColumn->getString();
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

