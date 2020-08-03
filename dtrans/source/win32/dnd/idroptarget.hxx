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
#pragma once

#include "target.hxx"

class IDropTargetImpl: public IDropTarget
{
    LONG m_nRefCount;
    // Calls to IDropTarget functions are delegated to a DropTarget.
    DropTarget& m_rDropTarget;

    virtual ~IDropTargetImpl(); // delete is only called by IUnknown::Release
    IDropTargetImpl( const IDropTargetImpl& );
    IDropTargetImpl& operator=( const IDropTargetImpl& );
public:
    explicit IDropTargetImpl(DropTarget& rTarget);

       // IDropTarget
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject) override;

    virtual ULONG STDMETHODCALLTYPE AddRef( ) override;

    virtual ULONG STDMETHODCALLTYPE Release( ) override;

    virtual HRESULT STDMETHODCALLTYPE DragEnter(
            /* [unique][in] */ IDataObject __RPC_FAR *pDataObj,
            /* [in] */ DWORD grfKeyState,
            /* [in] */ POINTL pt,
            /* [out][in] */ DWORD __RPC_FAR *pdwEffect) override;

    virtual HRESULT STDMETHODCALLTYPE DragOver(
            /* [in] */ DWORD grfKeyState,
            /* [in] */ POINTL pt,
            /* [out][in] */ DWORD __RPC_FAR *pdwEffect) override;

    virtual HRESULT STDMETHODCALLTYPE DragLeave( ) override;

    virtual HRESULT STDMETHODCALLTYPE Drop(
            /* [unique][in] */ IDataObject __RPC_FAR *pDataObj,
            /* [in] */ DWORD grfKeyState,
            /* [in] */ POINTL pt,
            /* [out][in] */ DWORD __RPC_FAR *pdwEffect) override;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
