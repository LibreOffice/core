/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DtObjFactory.hxx,v $
 * $Revision: 1.5 $
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


#ifndef _DTOBJFACTORY_HXX_
#define _DTOBJFACTORY_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <cppuhelper/servicefactory.hxx>

#include <systools/win32/comtools.hxx>

//------------------------------------------------------------------------
// deklarations
//------------------------------------------------------------------------

class CDTransObjFactory
{
public:
    virtual IDataObjectPtr SAL_CALL createDataObjFromTransferable( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& aServiceManager,
                                                                   const com::sun::star::uno::Reference< com::sun::star::datatransfer::XTransferable >& refXTransferable );
    virtual com::sun::star::uno::Reference< com::sun::star::datatransfer::XTransferable > SAL_CALL createTransferableFromDataObj(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& aServiceManager, IDataObjectPtr pIDataObject );
};

#endif
