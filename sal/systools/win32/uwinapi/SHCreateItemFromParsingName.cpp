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


#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning(disable:4740)
#endif

#include "macros.h"

static HRESULT WINAPI SHCreateItemFromParsingName_Failure (PCWSTR pszPath, IBindCtx *pbc, REFIID riid, void **ppv)
{
    return E_NOTIMPL;
}

extern HRESULT (WINAPI *pSHCreateItemFromParsingName)(PCWSTR pszPath, IBindCtx *pbc, REFIID riid, void **ppv);

static HRESULT WINAPI SHCreateItemFromParsingName_Thunk (PCWSTR pszPath, IBindCtx *pbc, REFIID riid, void **ppv)
{
    ResolveThunk_TRYLOAD( (FARPROC*)&pSHCreateItemFromParsingName, "shell32.dll", "SHCreateItemFromParsingName", 0, (FARPROC)SHCreateItemFromParsingName_Failure);
    return pSHCreateItemFromParsingName(pszPath, pbc, riid, ppv);
}

extern "C" HRESULT WINAPI SHCreateItemFromParsingName (PCWSTR pszPath, IBindCtx *pbc, REFIID riid, void **ppv)
{
    return pSHCreateItemFromParsingName(pszPath, pbc, riid, ppv);
}

HRESULT (WINAPI *pSHCreateItemFromParsingName)(PCWSTR pszPath, IBindCtx *pbc, REFIID riid, void **ppv) = SHCreateItemFromParsingName_Thunk;

