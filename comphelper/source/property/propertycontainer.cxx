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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"
#include <comphelper/propertycontainer.hxx>
#include <comphelper/property.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <osl/diagnose.h>
#include <uno/data.h>
#include <com/sun/star/uno/genfunc.h>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <rtl/ustrbuf.hxx>

#include <algorithm>

//.........................................................................
namespace comphelper
{
//.........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

//==========================================================================
//= OPropertyContainer
//==========================================================================
//--------------------------------------------------------------------------
OPropertyContainer::OPropertyContainer(::cppu::OBroadcastHelper& _rBHelper)
    :OPropertyContainer_Base(_rBHelper)
{
}

// -------------------------------------------------------------------------
OPropertyContainer::~OPropertyContainer()
{
}

//--------------------------------------------------------------------------
Sequence< Type > SAL_CALL OPropertyContainer::getTypes() throw (RuntimeException)
{
    // just the types from our one and only base class
    ::cppu::OTypeCollection aTypes(
        ::getCppuType( static_cast< Reference< XPropertySet >* >(NULL)),
        ::getCppuType( static_cast< Reference< XFastPropertySet >* >(NULL)),
        ::getCppuType( static_cast< Reference< XMultiPropertySet >* >(NULL))
    );
    return aTypes.getTypes();
}

//--------------------------------------------------------------------------
void SAL_CALL OPropertyContainer::setFastPropertyValue( sal_Int32 nHandle, const Any& rValue ) throw ( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    OPropertyContainer_Base::setFastPropertyValue( nHandle, rValue );
}

//--------------------------------------------------------------------------
sal_Bool OPropertyContainer::convertFastPropertyValue(
    Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue ) throw (IllegalArgumentException)
{
    return OPropertyContainerHelper::convertFastPropertyValue( _rConvertedValue, _rOldValue, _nHandle, _rValue );
}

//--------------------------------------------------------------------------
void OPropertyContainer::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue) throw (Exception)
{
    OPropertyContainerHelper::setFastPropertyValue( _nHandle, _rValue );
}

//--------------------------------------------------------------------------
void OPropertyContainer::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle) const
{
    OPropertyContainerHelper::getFastPropertyValue( _rValue, _nHandle );
}

//.........................................................................
}   // namespace comphelper
//.........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
