/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
                    "com.sun.star.container.NoSuchElementException: ")
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
                        "Singleton is based on neither interface nor service"),
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
