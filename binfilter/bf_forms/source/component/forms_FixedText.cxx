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

#include "FixedText.hxx"
#include "services.hxx"
#include "property.hrc"

#include <comphelper/property.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <com/sun/star/form/FormComponentType.hpp>

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

//------------------------------------------------------------------------------
InterfaceRef SAL_CALL OFixedTextModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory) throw (RuntimeException)
{
    return *(new OFixedTextModel(_rxFactory));
}

//------------------------------------------------------------------
DBG_NAME( OFixedTextModel )
//------------------------------------------------------------------
OFixedTextModel::OFixedTextModel( const Reference<XMultiServiceFactory>& _rxFactory )
        :OControlModel(_rxFactory, VCL_CONTROLMODEL_FIXEDTEXT)

{
    DBG_CTOR( OFixedTextModel, NULL );
    m_nClassId = FormComponentType::FIXEDTEXT;
}

//------------------------------------------------------------------
OFixedTextModel::OFixedTextModel( const OFixedTextModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory )
    :OControlModel( _pOriginal, _rxFactory )

{
    DBG_CTOR( OFixedTextModel, NULL );
}

//------------------------------------------------------------------
OFixedTextModel::~OFixedTextModel( )
{
    DBG_DTOR( OFixedTextModel, NULL );
}

//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( OFixedTextModel )

//------------------------------------------------------------------------------
StringSequence SAL_CALL OFixedTextModel::getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException)
{
    StringSequence aSupported = OControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_FIXEDTEXT;
    return aSupported;
}

//------------------------------------------------------------------------------
Reference<starbeans::XPropertySetInfo> SAL_CALL OFixedTextModel::getPropertySetInfo() throw(RuntimeException)
{
    Reference<starbeans::XPropertySetInfo> xInfo(createPropertySetInfo(getInfoHelper()));
    return xInfo;
}

//------------------------------------------------------------------------------
cppu::IPropertyArrayHelper& OFixedTextModel::getInfoHelper()
{
    return *const_cast<OFixedTextModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
void OFixedTextModel::fillProperties(
        Sequence< starbeans::Property >& _rProps,
        Sequence< starbeans::Property >& _rAggregateProps ) const
{
    FRM_BEGIN_PROP_HELPER(3)
        RemoveProperty(_rAggregateProps, PROPERTY_TABSTOP);

        DECL_PROP1(NAME,	rtl::OUString,	BOUND);
        DECL_PROP2(CLASSID,	sal_Int16,	READONLY, TRANSIENT);
        DECL_PROP1(TAG,		rtl::OUString,	BOUND);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OFixedTextModel::getServiceName() throw(RuntimeException)
{
    return FRM_COMPONENT_FIXEDTEXT;	// old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void SAL_CALL OFixedTextModel::write(const Reference<XObjectOutputStream>& _rxOutStream)
    throw(IOException, RuntimeException)
{
    OControlModel::write(_rxOutStream);

    // Version
    _rxOutStream->writeShort(0x0002);
    writeHelpTextCompatibly(_rxOutStream);
}

//------------------------------------------------------------------------------
void SAL_CALL OFixedTextModel::read(const Reference<XObjectInputStream>& _rxInStream) throw(IOException, RuntimeException)
{
    OControlModel::read(_rxInStream);

    // Version
    sal_Int16 nVersion = _rxInStream->readShort();
    if (nVersion > 1)
        readHelpTextCompatibly(_rxInStream);
}

//.........................................................................
}
//.........................................................................

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
