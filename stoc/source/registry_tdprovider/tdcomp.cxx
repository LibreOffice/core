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
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
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
    if (!_xBaseTD.is() && _aBaseType.getLength())
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
