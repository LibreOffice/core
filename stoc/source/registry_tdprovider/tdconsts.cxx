/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tdconsts.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:06:23 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#include "registry/reader.hxx"
#include "registry/version.h"

#ifndef _STOC_RDBTDP_BASE_HXX
#include "base.hxx"
#endif

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
