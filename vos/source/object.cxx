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



#include <string.h>

#include <rtl/alloc.h>
#include <rtl/memory.h>

#include <vos/diagnose.hxx>

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

   VOS_ASSERT(p != NULL);

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
    VOS_ASSERT(this != NULL);

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
    VOS_ASSERT(this != NULL);

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
    VOS_ASSERT(pClassName != NULL);

    const OClassInfo* pClass = &VOS_CLASSINFO(VOS_NAMESPACE(OObject, vos));

    while (pClass != NULL)
    {
        if (strcmp(pClassName, pClass->m_pClassName) == 0)
            break;

        pClass = pClass->m_pNextClass;
    }

    return (pClass);
}

VOS_CLASSINIT::VOS_CLASSINIT(OClassInfo* pNewClass)
{
    VOS_ASSERT(pNewClass != NULL);

    OClassInfo* pClassRoot = (OClassInfo*)&VOS_CLASSINFO(VOS_NAMESPACE(OObject, vos));

    pNewClass->m_pNextClass = pClassRoot->m_pNextClass;

    pClassRoot->m_pNextClass = pNewClass;
}
