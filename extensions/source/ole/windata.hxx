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
#ifndef INCLUDED_EXTENSIONS_SOURCE_OLE_WINDATA_HXX
#define INCLUDED_EXTENSIONS_SOURCE_OLE_WINDATA_HXX

#include <oleidl.h>

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#pragma clang diagnostic ignored "-Wextra"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wint-to-pointer-cast"
#pragma clang diagnostic ignored "-Winvalid-noreturn"
#pragma clang diagnostic ignored "-Wmicrosoft"
#pragma clang diagnostic ignored "-Wnon-pod-varargs"
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include <atlbase.h>

#if defined __clang__
#pragma clang diagnostic pop
#endif

#include <osl/diagnose.h>

//Wrapper for VARDESC
class VarDesc
{
    VARDESC* operator = (const VarDesc*);
    VarDesc(const VarDesc&);
// Construction
public:
    CComPtr< ITypeInfo > m_pTypeInfo;
    VARDESC* m_pVarDesc;

    explicit VarDesc(ITypeInfo* pTypeInfo) :
      m_pTypeInfo(pTypeInfo),
      m_pVarDesc(nullptr)
   {
       OSL_ASSERT(pTypeInfo);
   }
   ~VarDesc()
   {
      if (m_pVarDesc != nullptr)
      {
         m_pTypeInfo->ReleaseVarDesc(m_pVarDesc);
      }
   }

   VARDESC* operator->()
   {
      return m_pVarDesc;
   }

   VARDESC** operator&()
   {
      return &m_pVarDesc;
   }

    operator VARDESC* ()
    {
        return m_pVarDesc;
    }
};

//Wrapper for FUNCDESC structure
class FuncDesc
{
    FUNCDESC* operator = (const FuncDesc &);
    FuncDesc(const FuncDesc&);
    CComPtr<ITypeInfo> m_pTypeInfo;
    FUNCDESC * m_pFuncDesc;

public:

    explicit FuncDesc(ITypeInfo * pTypeInfo) :
        m_pTypeInfo(pTypeInfo),
        m_pFuncDesc(nullptr)
        {
            OSL_ASSERT(pTypeInfo);
        }
    ~FuncDesc()
    {
        ReleaseFUNCDESC();
    }

    FUNCDESC* operator -> ()
    {
        return m_pFuncDesc;
    }

    FUNCDESC** operator & ()
    {
        return & m_pFuncDesc;
    }

    operator FUNCDESC* ()
    {
        return m_pFuncDesc;
    }

    FUNCDESC* operator = (FUNCDESC* pDesc)
    {
        ReleaseFUNCDESC();
        m_pFuncDesc = pDesc;
        return m_pFuncDesc;
    }
    FUNCDESC* Detach()
    {
        FUNCDESC* pDesc = m_pFuncDesc;
        m_pFuncDesc = nullptr;
        return pDesc;
    }

    void ReleaseFUNCDESC()
    {
        if (m_pFuncDesc != nullptr)
        {
            m_pTypeInfo->ReleaseFuncDesc(m_pFuncDesc);
        }
        m_pFuncDesc = nullptr;
    }
};
//Wrapper for EXCEPINFO structure
class ExcepInfo : public EXCEPINFO
{
    EXCEPINFO* operator = (const ExcepInfo& );
    ExcepInfo(const ExcepInfo &);
public:
   ExcepInfo()
   {
      memset(this, 0, sizeof(ExcepInfo));
   }
   ~ExcepInfo()
   {
      if (bstrSource != nullptr)
         ::SysFreeString(bstrSource);
      if (bstrDescription != nullptr)
        ::SysFreeString(bstrDescription);
      if (bstrHelpFile != nullptr)
        ::SysFreeString(bstrHelpFile);
   }
};

//Wrapper for TYPEATTR
class TypeAttr
{
    TYPEATTR* operator = (const TypeAttr &);
    TypeAttr(const TypeAttr &);
public:
    CComPtr< ITypeInfo > m_pTypeInfo;
    TYPEATTR* m_pTypeAttr;

    explicit TypeAttr(ITypeInfo* pTypeInfo) :
      m_pTypeInfo( pTypeInfo ),
      m_pTypeAttr( nullptr )
   {
       OSL_ASSERT(pTypeInfo);
   }
   ~TypeAttr() throw()
   {
        if (m_pTypeAttr != nullptr)
        {
            m_pTypeInfo->ReleaseTypeAttr(m_pTypeAttr);
        }
   }

   TYPEATTR** operator&() throw()
   {
      return &m_pTypeAttr;
   }

   TYPEATTR* operator->() throw()
   {
      return m_pTypeAttr;
   }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
