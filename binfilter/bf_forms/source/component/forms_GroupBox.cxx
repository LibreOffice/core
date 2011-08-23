/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <comphelper/proparrhlp.hxx>

#include "GroupBox.hxx"
#include "property.hrc"
#include "services.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <com/sun/star/form/FormComponentType.hpp>

#include <comphelper/property.hxx>

namespace binfilter {

//.........................................................................
namespace frm
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
//using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

//==================================================================
// OGroupBoxModel
//==================================================================

//------------------------------------------------------------------
InterfaceRef SAL_CALL OGroupBoxModel_CreateInstance(const Reference<starlang::XMultiServiceFactory>& _rxFactory) throw (RuntimeException)
{
    return *(new OGroupBoxModel(_rxFactory));
}

//------------------------------------------------------------------
DBG_NAME( OGroupBoxModel )
//------------------------------------------------------------------
OGroupBoxModel::OGroupBoxModel(const Reference<starlang::XMultiServiceFactory>& _rxFactory)
    :OControlModel(_rxFactory, VCL_CONTROLMODEL_GROUPBOX, VCL_CONTROL_GROUPBOX)
{
    DBG_CTOR( OGroupBoxModel, NULL );
    m_nClassId = FormComponentType::GROUPBOX;
}

//------------------------------------------------------------------
OGroupBoxModel::OGroupBoxModel( const OGroupBoxModel* _pOriginal, const Reference<starlang::XMultiServiceFactory>& _rxFactory )
    :OControlModel( _pOriginal, _rxFactory )
{
    DBG_CTOR( OGroupBoxModel, NULL );
}

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL	OGroupBoxModel::getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException)
{
    StringSequence aSupported = OControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_GROUPBOX;
    return aSupported;
}

//------------------------------------------------------------------
OGroupBoxModel::~OGroupBoxModel()
{
    DBG_DTOR( OGroupBoxModel, NULL );
}

//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( OGroupBoxModel )

//------------------------------------------------------------------------------
Reference<XPropertySetInfo> SAL_CALL OGroupBoxModel::getPropertySetInfo() throw(RuntimeException)
{
    Reference<XPropertySetInfo> xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
cppu::IPropertyArrayHelper& OGroupBoxModel::getInfoHelper()
{
    return *const_cast<OGroupBoxModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
void OGroupBoxModel::fillProperties(
        Sequence< Property >& _rProps,
        Sequence< Property >& _rAggregateProps ) const
{
    FRM_BEGIN_PROP_HELPER(3)
        // don't want to have the TabStop property
        RemoveProperty(_rAggregateProps, PROPERTY_TABSTOP);

        DECL_PROP2(CLASSID,		sal_Int16,			READONLY, TRANSIENT);
        DECL_PROP1(NAME,		::rtl::OUString,	BOUND);
        DECL_PROP1(TAG,			::rtl::OUString,	BOUND);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OGroupBoxModel::getServiceName() throw(RuntimeException)
{
    return FRM_COMPONENT_GROUPBOX;	// old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void SAL_CALL OGroupBoxModel::write(const Reference<stario::XObjectOutputStream>& _rxOutStream)
    throw(stario::IOException, RuntimeException)
{
    OControlModel::write(_rxOutStream);

    // Version
    _rxOutStream->writeShort(0x0002);
    writeHelpTextCompatibly(_rxOutStream);
}

//------------------------------------------------------------------------------
void SAL_CALL OGroupBoxModel::read(const Reference<stario::XObjectInputStream>& _rxInStream) throw(stario::IOException, RuntimeException)
{
    OControlModel::read( _rxInStream );

    // Version
    sal_uInt16 nVersion = _rxInStream->readShort();
    DBG_ASSERT(nVersion > 0, "OGroupBoxModel::read : version 0 ? this should never have been written !");
        // ups, ist das Englisch richtig ? ;)

    if (nVersion == 2)
        readHelpTextCompatibly(_rxInStream);

    if (nVersion > 0x0002)
    {
        DBG_ERROR("OGroupBoxModel::read : unknown version !");
    }
};

//==================================================================
// OGroupBoxControl
//==================================================================

//------------------------------------------------------------------
InterfaceRef SAL_CALL OGroupBoxControl_CreateInstance(const Reference<starlang::XMultiServiceFactory>& _rxFactory) throw (RuntimeException)
{
    return *(new OGroupBoxControl(_rxFactory));
}

//------------------------------------------------------------------------------
OGroupBoxControl::OGroupBoxControl(const Reference<starlang::XMultiServiceFactory>& _rxFactory)
                   :OControl(_rxFactory, VCL_CONTROL_GROUPBOX)
{
}

//------------------------------------------------------------------------------
StringSequence SAL_CALL	OGroupBoxControl::getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException)
{
    StringSequence aSupported = OControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_GROUPBOX;
    return aSupported;
}

//.........................................................................
}
//.........................................................................

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
