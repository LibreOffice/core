/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DtObjFactory.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 17:00:41 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dtrans.hxx"

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _DTOBJFACTORY_HXX_
#include "..\..\inc\DtObjFactory.hxx"
#endif

#ifndef _TWRAPPERDATAOBJECT_HXX_
#include "XTDataObject.hxx"
#endif

#ifndef _DOWRAPPERTRANSFERABLE_HXX_
#include "DOTransferable.hxx"
#endif

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

