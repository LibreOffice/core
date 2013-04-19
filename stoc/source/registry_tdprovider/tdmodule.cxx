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
