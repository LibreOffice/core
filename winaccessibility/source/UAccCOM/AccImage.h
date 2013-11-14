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

#if !defined(AFX_ACCIMAGE_H__D5E55275_CCD4_497F_8E77_F9B391D2F4A8__INCLUDED_)
#define AFX_ACCIMAGE_H__D5E55275_CCD4_497F_8E77_F9B391D2F4A8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"        // main symbols


#include <com/sun/star/uno/reference.hxx>
#include <com/sun/star/accessibility/XAccessibleImage.hpp>
#include "UNOXWrapper.h"

/**
 * CAccImage implements IAccessibleImage interface.
 */
class ATL_NO_VTABLE CAccImage :
            public CComObjectRoot,
            public CComCoClass<CAccImage,&CLSID_AccImage>,
            public IAccessibleImage,
            public CUNOXWrapper
{
public:
    CAccImage()
    {
            }
    virtual ~CAccImage()
    {
            }
    BEGIN_COM_MAP(CAccImage)
    COM_INTERFACE_ENTRY(IAccessibleImage)
    COM_INTERFACE_ENTRY(IUNOXWrapper)
    COM_INTERFACE_ENTRY_FUNC_BLIND(NULL,_SmartQI)
    END_COM_MAP()

    static HRESULT WINAPI _SmartQI(void* pv,
                                   REFIID iid, void** ppvObject, DWORD)
    {
        return ((CAccImage*)pv)->SmartQI(iid,ppvObject);
    }

    HRESULT SmartQI(REFIID iid, void** ppvObject)
    {
        if( m_pOuterUnknown )
            return OuterQueryInterface(iid,ppvObject);
        return E_FAIL;
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_AccImage)

    // IAccessibleImage
public:
    // IAccessibleImage

    // Gets the description of the image.
    STDMETHOD(get_description)(BSTR * description);

    STDMETHOD(get_imagePosition)( enum IA2CoordinateType coordinateType,
                                  long __RPC_FAR *x,
                                  long __RPC_FAR *y);

    STDMETHOD(get_imageSize)(
        long __RPC_FAR *height,
        long __RPC_FAR *width);

    // Overide of IUNOXWrapper.
    STDMETHOD(put_XInterface)(long pXInterface);

private:

    com::sun::star::uno::Reference<com::sun::star::accessibility::XAccessibleImage> pRXImg;

    inline com::sun::star::accessibility::XAccessibleImage* GetXInterface()
    {
        return pRXImg.get();
    }
};

#endif // !defined(AFX_ACCIMAGE_H__D5E55275_CCD4_497F_8E77_F9B391D2F4A8__INCLUDED_)
