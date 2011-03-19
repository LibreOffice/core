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

#include <vector>
#include <osl/diagnose.h>
#include "base.hxx"

namespace stoc_rdbtdp
{

//__________________________________________________________________________________________________
// virtual
ModuleTypeDescriptionImpl::~ModuleTypeDescriptionImpl()
{
    delete _pMembers;

    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

// XTypeDescription
//__________________________________________________________________________________________________
// virtual
TypeClass ModuleTypeDescriptionImpl::getTypeClass()
    throw( RuntimeException )
{
    return TypeClass_MODULE;
}
//__________________________________________________________________________________________________
// virtual
OUString ModuleTypeDescriptionImpl::getName()
    throw( RuntimeException )
{
    return _aName;
}

// XModuleTypeDescription
//__________________________________________________________________________________________________
// virtual
Sequence< Reference< XTypeDescription > > SAL_CALL
ModuleTypeDescriptionImpl::getMembers()
    throw ( RuntimeException )
{
    if ( !_pMembers )
    {
        Reference< XTypeDescriptionEnumeration > xEnum;
        try
        {
            xEnum = _xTDMgr->createTypeDescriptionEnumeration(
                        _aName,
                        Sequence< TypeClass >(),
                        TypeDescriptionSearchDepth_ONE );
        }
        catch ( NoSuchTypeNameException const & )
        {
        }
        catch ( InvalidTypeNameException const & )
        {
        }

        OSL_ENSURE( xEnum.is(),
                    "ModuleTypeDescriptionImpl::getMembers - No enumeration!" );

        std::vector< Reference< XTypeDescription > > aTDs;
        while ( xEnum->hasMoreElements() )
        {
            try
            {
                Reference< XTypeDescription > xTD(
                    xEnum->nextTypeDescription() );
                aTDs.push_back( xTD );
            }
            catch ( NoSuchElementException const & )
            {
                OSL_FAIL( "ModuleTypeDescriptionImpl::getMembers - "
                    " Caught NoSuchElementException!" );
            }
        }

        Sequence< Reference< XTypeDescription > > * pMembers
            = new Sequence< Reference< XTypeDescription > >( aTDs.size() );
        for ( sal_Int32 n = 0; n < pMembers->getLength(); n++ )
            (*pMembers)[ n ] = aTDs[ n ];

        ClearableMutexGuard aGuard( getMutex() );
        if ( _pMembers )
        {
            aGuard.clear();
            delete pMembers;
        }
        else
        {
            _pMembers = pMembers;
        }
    }
    return *_pMembers;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
