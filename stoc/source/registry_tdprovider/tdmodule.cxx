/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tdmodule.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:07:35 $
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

#include <vector>

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _STOC_RDBTDP_BASE_HXX
#include "base.hxx"
#endif

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
                OSL_ENSURE( sal_False,
                    "ModuleTypeDescriptionImpl::getMembers - "
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
