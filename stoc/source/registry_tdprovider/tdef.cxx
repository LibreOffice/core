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

#include "base.hxx"

namespace stoc_rdbtdp
{
TypedefTypeDescriptionImpl::~TypedefTypeDescriptionImpl() {}

// XTypeDescription
//__________________________________________________________________________________________________
TypeClass TypedefTypeDescriptionImpl::getTypeClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    return TypeClass_TYPEDEF;
}
//__________________________________________________________________________________________________
OUString TypedefTypeDescriptionImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aName;
}

// XIndirectTypeDescription
//__________________________________________________________________________________________________
Reference< XTypeDescription > TypedefTypeDescriptionImpl::getReferencedType()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (!_xRefTD.is() && !_aRefName.isEmpty())
    {
        try
        {
            Reference< XTypeDescription > xRefTD;
            if (_xTDMgr->getByHierarchicalName( _aRefName ) >>= xRefTD)
            {
                MutexGuard aGuard( getMutex() );
                if (! _xRefTD.is())
                    _xRefTD = xRefTD;
                return _xRefTD;
            }
        }
        catch (NoSuchElementException &)
        {
        }
        // never try again, if no base td was found
        _aRefName = OUString();
    }
    return _xRefTD;
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
