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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
