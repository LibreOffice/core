/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tdprop.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:07:55 $
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

#ifndef _STOC_RDBTDP_BASE_HXX
#include "base.hxx"
#endif

namespace stoc_rdbtdp
{

//__________________________________________________________________________________________________
// virtual
ConstantTypeDescriptionImpl::~ConstantTypeDescriptionImpl()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

// XTypeDescription
//__________________________________________________________________________________________________
// virtual
TypeClass ConstantTypeDescriptionImpl::getTypeClass()
    throw( RuntimeException )
{
    return TypeClass_CONSTANT;
}
//__________________________________________________________________________________________________
// virtual
OUString ConstantTypeDescriptionImpl::getName()
    throw( RuntimeException )
{
    return _aName;
}

// XConstantTypeDescription
//__________________________________________________________________________________________________
// virtual
Any SAL_CALL ConstantTypeDescriptionImpl::getConstantValue()
    throw( RuntimeException )
{
    return _aValue;
}

}
