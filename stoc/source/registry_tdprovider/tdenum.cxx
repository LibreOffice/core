/*************************************************************************
 *
 *  $RCSfile: tdenum.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:29:35 $
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
EnumTypeDescriptionImpl::~EnumTypeDescriptionImpl()
{
    delete _pEnumNames;
    delete _pEnumValues;
}

// XTypeDescription
//__________________________________________________________________________________________________
TypeClass EnumTypeDescriptionImpl::getTypeClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    return TypeClass_ENUM;
}
//__________________________________________________________________________________________________
OUString EnumTypeDescriptionImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aName;
}

// XEnumTypeDescription
//__________________________________________________________________________________________________
sal_Int32 EnumTypeDescriptionImpl::getDefaultEnumValue()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _nDefaultValue;
}
//__________________________________________________________________________________________________
Sequence< OUString > EnumTypeDescriptionImpl::getEnumNames()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _pEnumNames)
    {
        MutexGuard aGuard( _aEnumNamesMutex );
        if (! _pEnumNames)
        {
            RegistryTypeReaderLoader aLoader;
            RegistryTypeReader aReader( aLoader, (const sal_uInt8 *)_aBytes.getConstArray(),
                                        _aBytes.getLength(), sal_False );

            sal_uInt16 nFields = (sal_uInt16)aReader.getFieldCount();
            Sequence< OUString > * pTempEnumNames = new Sequence< OUString >( nFields );
            OUString * pEnumNames = pTempEnumNames->getArray();

            while (nFields--)
            {
                pEnumNames[nFields] = aReader.getFieldName( nFields );
            }

            _pEnumNames = pTempEnumNames;
        }
    }
    return *_pEnumNames;
}
//__________________________________________________________________________________________________
Sequence< sal_Int32 > EnumTypeDescriptionImpl::getEnumValues()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _pEnumValues)
    {
        MutexGuard aGuard( _aEnumValuesMutex );
        if (! _pEnumValues)
        {
            RegistryTypeReaderLoader aLoader;
            RegistryTypeReader aReader( aLoader, (const sal_uInt8 *)_aBytes.getConstArray(),
                                        _aBytes.getLength(), sal_False );

            sal_uInt16 nFields = (sal_uInt16)aReader.getFieldCount();
            Sequence< sal_Int32 > * pTempEnumValues = new Sequence< sal_Int32 >( nFields );
            sal_Int32 * pEnumValues = pTempEnumValues->getArray();

            while (nFields--)
            {
                pEnumValues[nFields] = getRTValueAsInt32( aReader.getFieldConstValue( nFields ) );
            }

            _pEnumValues = pTempEnumValues;
        }
    }
    return *_pEnumValues;
}

}


