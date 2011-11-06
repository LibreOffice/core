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
#include "precompiled_stoc.hxx"
#include <osl/diagnose.h>
#include "base.hxx"

#include "com/sun/star/uno/RuntimeException.hpp"

using namespace com::sun::star;

namespace stoc_rdbtdp
{

void SingletonTypeDescriptionImpl::init() {
    {
        MutexGuard guard(getMutex());
        if (_xInterfaceTD.is() || _xServiceTD.is()) {
            return;
        }
    }
    Reference< XTypeDescription > base;
    try {
        base = Reference< XTypeDescription >(
            _xTDMgr->getByHierarchicalName(_aBaseName), UNO_QUERY_THROW);
    } catch (NoSuchElementException const & e) {
        throw RuntimeException(
            (OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.container.NoSuchElementException: "))
             + e.Message),
            static_cast< OWeakObject * >(this));
    }
    MutexGuard guard(getMutex());
    if (!_xInterfaceTD.is() && !_xServiceTD.is()) {
        if (resolveTypedefs(base)->getTypeClass() == TypeClass_INTERFACE) {
            _xInterfaceTD = base;
        } else if (base->getTypeClass() == TypeClass_SERVICE) {
            _xServiceTD = Reference< XServiceTypeDescription >(
                base, UNO_QUERY_THROW);
        } else {
            throw RuntimeException(
                OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "Singleton is based on neither interface nor service")),
                static_cast< OWeakObject * >(this));
        }
    }
    OSL_ASSERT(_xInterfaceTD.is() ^ _xServiceTD.is());
}

//__________________________________________________________________________________________________
// virtual
SingletonTypeDescriptionImpl::~SingletonTypeDescriptionImpl()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

// XTypeDescription
//__________________________________________________________________________________________________
// virtual
TypeClass SingletonTypeDescriptionImpl::getTypeClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    return TypeClass_SINGLETON;
}
//__________________________________________________________________________________________________
// virtual
OUString SingletonTypeDescriptionImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aName;
}

// XSingletonTypeDescription
//__________________________________________________________________________________________________
// virtual
Reference< XServiceTypeDescription > SAL_CALL
SingletonTypeDescriptionImpl::getService()
    throw(::com::sun::star::uno::RuntimeException)
{
    init();
    return _xServiceTD;
}

// XSingletonTypeDescription2
//______________________________________________________________________________
// virtual
sal_Bool SAL_CALL
SingletonTypeDescriptionImpl::isInterfaceBased()
    throw(::com::sun::star::uno::RuntimeException)
{
    init();
    return _xInterfaceTD.is();
}

//______________________________________________________________________________
// virtual
Reference< XTypeDescription > SAL_CALL
SingletonTypeDescriptionImpl::getInterface()
    throw(::com::sun::star::uno::RuntimeException)
{
    init();
    return _xInterfaceTD;
}

}
