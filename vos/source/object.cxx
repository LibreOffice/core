/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <string.h>

#include <rtl/alloc.h>
#include <rtl/memory.h>

#include <osl/diagnose.h>

#include <vos/object.hxx>

using namespace vos;

/////////////////////////////////////////////////////////////////////////////
// Object super class

VOS_NAMESPACE(OClassInfo, vos) VOS_NAMESPACE(OObject, vos)::__ClassInfo__(VOS_CLASSNAME(OObject, vos), sizeof(VOS_NAMESPACE(OObject, vos)));

OObject::OObject()
{
}

OObject::OObject(const OCreateParam&)
{
}

OObject::~OObject()
{
}

void* OObject::operator new(size_t size)
{
   void* p = rtl_allocateMemory(size);

   OSL_ASSERT(p != NULL);

   return (p);
}

void* OObject::operator new(size_t, void* p)
{
   return (p);
}

void OObject::operator delete(void* p)
{
   rtl_freeMemory(p);
}

const OClassInfo& OObject::classInfo()
{
    return (__ClassInfo__);
}

const OClassInfo& OObject::getClassInfo() const
{
    return (VOS_CLASSINFO(VOS_NAMESPACE(OObject, vos)));
}

sal_Bool OObject::isKindOf(const OClassInfo& rClass) const
{
    OSL_ASSERT(this != NULL);

    const OClassInfo& rClassThis = getClassInfo();

    return (rClassThis.isDerivedFrom(rClass));
}

/////////////////////////////////////////////////////////////////////////////
// Basic class information

OClassInfo::OClassInfo(const sal_Char *pClassName, sal_Int32 ObjectSize,
                       const OClassInfo* pBaseClass, sal_uInt32 Schema,
                       OObject* (SAL_CALL * fnCreateObject)(const OCreateParam&))
{
    m_pClassName  = pClassName;
    m_nObjectSize = ObjectSize;
    m_wSchema     = Schema;

    m_pfnCreateObject = fnCreateObject;

    m_pBaseClass = pBaseClass;
    m_pNextClass = NULL;
}

OObject* OClassInfo::createObject(const OCreateParam& rParam) const
{
    if (m_pfnCreateObject == NULL)
        return NULL;

    OObject* pObject = NULL;
    pObject = (*m_pfnCreateObject)(rParam);

    return (pObject);
}

sal_Bool OClassInfo::isDerivedFrom(const OClassInfo& rClass) const
{
    OSL_ASSERT(this != NULL);

    const OClassInfo* pClassThis = this;

    while (pClassThis != NULL)
    {
        if (pClassThis == &rClass)
            return (sal_True);

        pClassThis = pClassThis->m_pBaseClass;
    }

    return (sal_False);      // walked to the top, no match
}

const OClassInfo* OClassInfo::getClassInfo(const sal_Char* pClassName)
{
    OSL_ASSERT(pClassName != NULL);

    const OClassInfo* pClass = &VOS_CLASSINFO(VOS_NAMESPACE(OObject, vos));

    while (pClass != NULL)
    {
        if (strcmp(pClassName, pClass->m_pClassName) == 0)
            break;

        pClass = pClass->m_pNextClass;
    }

    return (pClass);
}

VOS_CLASSINIT::VOS_CLASSINIT(register OClassInfo* pNewClass)
{
    OSL_ASSERT(pNewClass != NULL);

    OClassInfo* pClassRoot = (OClassInfo*)&VOS_CLASSINFO(VOS_NAMESPACE(OObject, vos));

    pNewClass->m_pNextClass = pClassRoot->m_pNextClass;

    pClassRoot->m_pNextClass = pNewClass;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
