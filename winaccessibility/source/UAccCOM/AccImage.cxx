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

#include "stdafx.h"
#include "UAccCOM2.h"
#include "AccImage.h"
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>

using namespace com::sun::star::accessibility;
using namespace com::sun::star::uno;

/**
   * Get description.
   * @param description Variant to get description.
   * @return Result.
*/
STDMETHODIMP CAccImage::get_description(BSTR * description)
{

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if (description == NULL)
        return E_INVALIDARG;
    if( !pRXImg.is() )
        return E_FAIL;

    ::rtl::OUString ouStr = GetXInterface()->getAccessibleImageDescription();
    SAFE_SYSFREESTRING(*description);
    *description = SysAllocString((OLECHAR*)ouStr.getStr());

    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

STDMETHODIMP CAccImage::get_imagePosition(
    /* [in] */ enum IA2CoordinateType,
    /* [out] */ long __RPC_FAR *,
    /* [retval][out] */ long __RPC_FAR *)
{
    return E_NOTIMPL;
}

STDMETHODIMP CAccImage::get_imageSize(
    /* [out] */ long __RPC_FAR *,
    /* [retval][out] */ long __RPC_FAR *)
{
    return E_NOTIMPL;
}

/**
   * Put UNO interface.
   * @param pXInterface UNO interface.
   * @return Result.
*/
STDMETHODIMP CAccImage::put_XInterface(long pXInterface)
{


    ENTER_PROTECTED_BLOCK

    CUNOXWrapper::put_XInterface(pXInterface);
    //special query.
    if(pUNOInterface == NULL)
        return E_FAIL;

    Reference<XAccessibleContext> pRContext = pUNOInterface->getAccessibleContext();
    if( !pRContext.is() )
    {
        return E_FAIL;
    }
    Reference<XAccessibleImage> pRXI(pRContext,UNO_QUERY);
    if( !pRXI.is() )
        pRXImg = NULL;
    else
        pRXImg = pRXI.get();
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}
