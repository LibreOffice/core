/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tdcomp.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:05:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _STOC_RDBTDP_BASE_HXX
#include "base.hxx"
#endif

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


