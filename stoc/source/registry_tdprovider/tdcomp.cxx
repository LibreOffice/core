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

#include "registry/reader.hxx"
#include "registry/version.h"

namespace stoc_rdbtdp
{

//__________________________________________________________________________________________________
CompoundTypeDescriptionImpl::~CompoundTypeDescriptionImpl()
{
    delete _pMembers;
    delete _pMemberNames;
}

// XTypeDescription
//__________________________________________________________________________________________________
TypeClass CompoundTypeDescriptionImpl::getTypeClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _eTypeClass;
}
//__________________________________________________________________________________________________
OUString CompoundTypeDescriptionImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aName;
}

// XCompoundTypeDescription
//__________________________________________________________________________________________________
Reference< XTypeDescription > CompoundTypeDescriptionImpl::getBaseType()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (!_xBaseTD.is() && !_aBaseType.isEmpty())
    {
        try
        {
            Reference< XTypeDescription > xBaseTD;
            if (_xTDMgr->getByHierarchicalName( _aBaseType ) >>= xBaseTD)
            {
                MutexGuard aGuard( getMutex() );
                if (! _xBaseTD.is())
                    _xBaseTD = xBaseTD;
                return _xBaseTD;
            }
        }
        catch (NoSuchElementException &)
        {
        }
        // never try again, if no base td was found
        _aBaseType = OUString();
    }
    return _xBaseTD;
}
//__________________________________________________________________________________________________

namespace {

class TypeParameter: public WeakImplHelper1< XTypeDescription > {
public:
    explicit TypeParameter(OUString const & name): m_name(name) {}

    virtual TypeClass SAL_CALL getTypeClass() throw (RuntimeException)
    { return TypeClass_UNKNOWN; }

    virtual OUString SAL_CALL getName() throw (RuntimeException)
    { return m_name; }

private:
    OUString m_name;
};

}

Sequence< Reference< XTypeDescription > > CompoundTypeDescriptionImpl::getMemberTypes()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _pMembers)
    {
        typereg::Reader aReader(
            _aBytes.getConstArray(), _aBytes.getLength(), false,
            TYPEREG_VERSION_1);

        sal_uInt16 nFields = aReader.getFieldCount();
        Sequence< Reference< XTypeDescription > > * pTempMembers =
            new Sequence< Reference< XTypeDescription > >( nFields );
        Reference< XTypeDescription > * pMembers = pTempMembers->getArray();

        while (nFields--)
        {
            if ((aReader.getFieldFlags(nFields) & RT_ACCESS_PARAMETERIZED_TYPE)
                != 0)
            {
                pMembers[nFields] = new TypeParameter(
                    aReader.getFieldTypeName(nFields));
            } else {
                try {
                    _xTDMgr->getByHierarchicalName(
                        aReader.getFieldTypeName(nFields).replace('/', '.'))
                            >>= pMembers[nFields];
                } catch (NoSuchElementException &) {}
                OSL_ENSURE(
                    pMembers[nFields].is(), "### compound member unknown!");
            }
        }

        ClearableMutexGuard aGuard( getMutex() );
        if (_pMembers)
        {
            aGuard.clear();
            delete pTempMembers;
        }
        else
        {
            _pMembers = pTempMembers;
        }
    }

    return *_pMembers;
}
//__________________________________________________________________________________________________
Sequence< OUString > CompoundTypeDescriptionImpl::getMemberNames()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _pMemberNames)
    {
        typereg::Reader aReader(
            _aBytes.getConstArray(), _aBytes.getLength(), false,
            TYPEREG_VERSION_1);

        sal_uInt16 nFields = aReader.getFieldCount();
        Sequence< OUString > * pTempMemberNames = new Sequence< OUString >( nFields );
        OUString * pMemberNames = pTempMemberNames->getArray();

        while (nFields--)
        {
            pMemberNames[nFields] = aReader.getFieldName( nFields );
        }

        ClearableMutexGuard aGuard( getMutex() );
        if (_pMemberNames)
        {
            aGuard.clear();
            delete pTempMemberNames;
        }
        else
        {
            _pMemberNames = pTempMemberNames;
        }
    }
    return *_pMemberNames;
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
