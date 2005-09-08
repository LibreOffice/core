/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tdef.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:06:38 $
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

namespace stoc_rdbtdp
{
TypedefTypeDescriptionImpl::~TypedefTypeDescriptionImpl()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

// XTypeDescription
//__________________________________________________________________________________________________
TypeClass TypedefTypeDescriptionImpl::getTypeClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    return TypeClass_TYPEDEF;
}
//__________________________________________________________________________________________________
OUString TypedefTypeDescriptionImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aName;
}

// XIndirectTypeDescription
//__________________________________________________________________________________________________
Reference< XTypeDescription > TypedefTypeDescriptionImpl::getReferencedType()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (!_xRefTD.is() && _aRefName.getLength())
    {
        try
        {
            Reference< XTypeDescription > xRefTD;
            if (_xTDMgr->getByHierarchicalName( _aRefName ) >>= xRefTD)
            {
                MutexGuard aGuard( getMutex() );
                if (! _xRefTD.is())
                    _xRefTD = xRefTD;
                return _xRefTD;
            }
        }
        catch (NoSuchElementException &)
        {
        }
        // never try again, if no base td was found
        _aRefName = OUString();
    }
    return _xRefTD;
}

}


