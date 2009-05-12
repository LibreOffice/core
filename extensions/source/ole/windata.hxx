/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: windata.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef AUTOMATION_BRIDGE_WINDATA_HXX
#define AUTOMATION_BRIDGE_WINDATA_HXX

#pragma warning (push,1)
#pragma warning (disable:4668)
#pragma warning (disable:4548)
#include "oleidl.h"

#include <atlbase.h>
#pragma warning (pop)
#include "osl/diagnose.h"

namespace ole_adapter
{
//Wrapper for VARDESC
class VarDesc
{
    VARDESC* operator = (const VarDesc*);
    VarDesc(const VarDesc&);
// Construction
public:
    CComPtr< ITypeInfo > m_pTypeInfo;
    VARDESC* m_pVarDesc;

    VarDesc(ITypeInfo* pTypeInfo) :
      m_pVarDesc(NULL),
      m_pTypeInfo(pTypeInfo)
   {
       OSL_ASSERT(pTypeInfo);
   }
   ~VarDesc()
   {
      if (m_pVarDesc != NULL)
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

    FuncDesc(ITypeInfo * pTypeInfo) :
        m_pFuncDesc(NULL),
        m_pTypeInfo(pTypeInfo)
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
        m_pFuncDesc = NULL;
        return pDesc;
    }

    void ReleaseFUNCDESC()
    {
        if (m_pFuncDesc != NULL)
        {
            m_pTypeInfo->ReleaseFuncDesc(m_pFuncDesc);
        }
        m_pFuncDesc = NULL;
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
         if (bstrSource != NULL)
         ::SysFreeString(bstrSource);
      if (bstrDescription != NULL)
        ::SysFreeString(bstrDescription);
      if (bstrHelpFile != NULL)
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

    TypeAttr(ITypeInfo* pTypeInfo) :
      m_pTypeAttr( NULL ),
      m_pTypeInfo( pTypeInfo )
   {
       OSL_ASSERT(pTypeInfo);
   }
   ~TypeAttr() throw()
   {
        if (m_pTypeAttr != NULL)
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



}

#endif
