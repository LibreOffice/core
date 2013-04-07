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
#include <rtl/ustrbuf.hxx>
#include "registry/reader.hxx"
#include "registry/version.h"
#include "base.hxx"

namespace stoc_rdbtdp
{

//__________________________________________________________________________________________________
// virtual
ConstantsTypeDescriptionImpl::~ConstantsTypeDescriptionImpl()
{
    delete _pMembers;

    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

// XTypeDescription
//__________________________________________________________________________________________________
// virtual
TypeClass ConstantsTypeDescriptionImpl::getTypeClass()
    throw( RuntimeException )
{
    return TypeClass_CONSTANTS;
}
//__________________________________________________________________________________________________
// virtual
OUString ConstantsTypeDescriptionImpl::getName()
    throw( RuntimeException )
{
    return _aName;
}

// XConstantsTypeDescription
//__________________________________________________________________________________________________
// virtual
Sequence< Reference< XConstantTypeDescription > > SAL_CALL
ConstantsTypeDescriptionImpl::getConstants()
    throw ( RuntimeException )
{
    if ( !_pMembers )
    {
        typereg::Reader aReader(
            _aBytes.getConstArray(), _aBytes.getLength(), false,
            TYPEREG_VERSION_1);

        sal_uInt16 nFields = aReader.getFieldCount();
        Sequence< Reference< XConstantTypeDescription > > * pTempConsts
            = new Sequence< Reference< XConstantTypeDescription > >( nFields );
        Reference< XConstantTypeDescription > * pConsts
            = pTempConsts->getArray();

        while ( nFields-- )
        {
            OUStringBuffer aName( _aName );
            aName.appendAscii( "." );
            aName.append( aReader.getFieldName( nFields ) );

            Any aValue( getRTValue( aReader.getFieldValue( nFields ) ) );

            pConsts[ nFields ]
                = new ConstantTypeDescriptionImpl( aName.makeStringAndClear(),
                                                   aValue );
        }

        ClearableMutexGuard aGuard( getMutex() );
        if ( _pMembers )
        {
            aGuard.clear();
            delete pTempConsts;
        }
        else
        {
            _pMembers = pTempConsts;
        }
    }
    return *_pMembers;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
