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



#ifndef CLASSFACTORY_HXX_INCLUDED
#define CLASSFACTORY_HXX_INCLUDED

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <objidl.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

class CClassFactory : public IClassFactory
{
public:
    CClassFactory(const CLSID& clsid);
    virtual ~CClassFactory();

    //-----------------------------
    // IUnknown methods
    //-----------------------------

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID riid,
            void __RPC_FAR *__RPC_FAR *ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef(void);

    virtual ULONG STDMETHODCALLTYPE Release(void);

    //-----------------------------
    // IClassFactory methods
    //-----------------------------

    virtual HRESULT STDMETHODCALLTYPE CreateInstance(
            IUnknown __RPC_FAR *pUnkOuter,
            REFIID riid,
            void __RPC_FAR *__RPC_FAR *ppvObject);

    virtual HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock);

    static bool IsLocked();

private:
    long  m_RefCnt;
    CLSID m_Clsid;

    static long  s_ServerLocks;
};

#endif
