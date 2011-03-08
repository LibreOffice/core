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
#include "precompiled_dtrans.hxx"

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------
#include "..\..\inc\DtObjFactory.hxx"

#include "XTDataObject.hxx"

#include "DOTransferable.hxx"

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::lang;

//------------------------------------------------------------------------
// implementation
//------------------------------------------------------------------------

IDataObjectPtr SAL_CALL CDTransObjFactory::createDataObjFromTransferable(const Reference<XMultiServiceFactory>& aServiceManager,
                                                                       const Reference< XTransferable >& refXTransferable)
{
    return (IDataObjectPtr(new CXTDataObject(aServiceManager, refXTransferable)));
}

Reference< XTransferable > SAL_CALL CDTransObjFactory::createTransferableFromDataObj( const Reference< XMultiServiceFactory >& aServiceManager,
                                                                                     IDataObjectPtr pIDataObject )
{
    CDOTransferable* pTransf = new CDOTransferable(aServiceManager, pIDataObject);
    Reference<XTransferable> refDOTransf(pTransf);

    pTransf->acquire();
    pTransf->initFlavorList();
    pTransf->release();

    return refDOTransf;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
