/*************************************************************************
 *
 *  $RCSfile: tdcomp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-07 14:48:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "base.hxx"

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
    if (!_xBaseTD.is() && _aBaseType.getLength())
    {
        MutexGuard aGuard( _aBaseTypeMutex );
        if (!_xBaseTD.is() && _aBaseType.getLength())
        {
            try
            {
                if (_xTDMgr->getByHierarchicalName( _aBaseType ) >>= _xBaseTD)
                    return _xBaseTD;
            }
            catch (NoSuchElementException &)
            {
            }
            // never try again, if no base td was found
            _aBaseType = OUString();
        }
    }
    return _xBaseTD;
}
//__________________________________________________________________________________________________
Sequence< Reference< XTypeDescription > > CompoundTypeDescriptionImpl::getMemberTypes()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _pMembers)
    {
        MutexGuard aGuard( _aMembersMutex );
        if (! _pMembers)
        {
            RegistryTypeReaderLoader aLoader;
            RegistryTypeReader aReader( aLoader, (const sal_uInt8 *)_aBytes.getConstArray(),
                                        _aBytes.getLength(), sal_False );

            sal_uInt16 nFields = (sal_uInt16)aReader.getFieldCount();
            Sequence< Reference< XTypeDescription > > * pTempMembers =
                new Sequence< Reference< XTypeDescription > >( nFields );
            Reference< XTypeDescription > * pMembers = pTempMembers->getArray();

            while (nFields--)
            {
                try
                {
                    _xTDMgr->getByHierarchicalName(
                        aReader.getFieldType( nFields ).replace( '/', '.' ) )
                            >>= pMembers[nFields];
                }
                catch (NoSuchElementException &)
                {
                }
                OSL_ENSHURE( pMembers[nFields].is(), "### compound member unknown!" );
            }

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
        MutexGuard aGuard( _aMemberNamesMutex );
        if (! _pMemberNames)
        {
            RegistryTypeReaderLoader aLoader;
            RegistryTypeReader aReader( aLoader, (const sal_uInt8 *)_aBytes.getConstArray(),
                                        _aBytes.getLength(), sal_False );

            sal_uInt16 nFields = (sal_uInt16)aReader.getFieldCount();
            Sequence< OUString > * pTempMemberNames = new Sequence< OUString >( nFields );
            OUString * pMemberNames = pTempMemberNames->getArray();

            while (nFields--)
            {
                pMemberNames[nFields] = aReader.getFieldName( nFields );
            }

            _pMemberNames = pTempMemberNames;
        }
    }
    return *_pMemberNames;
}

}


