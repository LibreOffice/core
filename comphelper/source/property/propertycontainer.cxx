/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


