/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

// SOActionsApproval.h: Definition of the SOActionsApproval class

#pragma once

#ifdef _MSC_VER
#pragma once
#endif

#include "resource.h"
#include <ExDispID.h>
#include <ExDisp.h>
#include <shlguid.h>

#include <atlctl.h>

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif
#include <so_activex.h>
#if defined __clang__
#pragma clang diagnostic pop
#endif

// SOActionsApproval

class SOActionsApproval :
    public IDispatchImpl<ISOActionsApproval, &IID_ISOActionsApproval, &LIBID_SO_ACTIVEXLib>,
    public ISupportErrorInfo,
    public CComObjectRoot,
    public CComCoClass<SOActionsApproval,&CLSID_SOActionsApproval>
{
public:
    SOActionsApproval() {}
    virtual ~SOActionsApproval() {}

BEGIN_COM_MAP(SOActionsApproval)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISOActionsApproval)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
END_COM_MAP()
#if defined __clang__
#pragma clang diagnostic pop
#endif
DECLARE_NOT_AGGREGATABLE(SOActionsApproval)
// Remove the comment from the line above if you don't want your object to
// support aggregation.

DECLARE_REGISTRY_RESOURCEID(IDR_SODOCUMENTEVENTLISTENER)

// ISupportsErrorInfo
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid) override;

// ISOActionsApproval
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE approveAction(
            /* [in] */ long nActionID,
            /* [retval][out] */ boolean *pbApproval) override
        {
            // only PreventClose is approved
            USES_CONVERSION;
            *pbApproval = ( nActionID == 1 );

            return S_OK;
        }

        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Bridge_implementedInterfaces(
            /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pVal) override
        {
            *pVal = SafeArrayCreateVector( VT_BSTR, 0, 1 );

            if( !*pVal )
                return E_FAIL;

            LONG ix = 0;
            CComBSTR aInterface( OLESTR( "com.sun.star.embed.XActionsApproval" ) );
            SafeArrayPutElement( *pVal, &ix, aInterface );

            return S_OK;
        }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
