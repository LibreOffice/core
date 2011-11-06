/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dtrans.hxx"

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------
#include "..\..\inc\DtObjFactory.hxx"

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

