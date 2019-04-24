/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "COMRefCounted.hpp"

class ClassFactory : public COMRefCounted<IClassFactory>
{
public:
    ClassFactory();
    virtual ~ClassFactory() override;

    // IUnknown methods

    HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID riid,
        void **ppvObject) override;

    // IClassFactory methods

    HRESULT STDMETHODCALLTYPE CreateInstance(
        IUnknown *pUnkOuter,
        REFIID riid,
        void **ppvObject) override;

    HRESULT STDMETHODCALLTYPE LockServer(
        BOOL fLock) override;

    // Non-COM methods

    static long GetObjectCount() { return m_nObjCount; }
    static long GetLockCount() { return m_nLockCount; }

private:
    static long m_nObjCount;
    static long m_nLockCount;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
