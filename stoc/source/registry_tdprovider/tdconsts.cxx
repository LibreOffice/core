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
            rtl::OUStringBuffer aName( _aName );
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
