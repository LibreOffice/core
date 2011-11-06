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




#include <limits.h>
#include <vos/diagnose.hxx>

#include <vos/refernce.hxx>

using namespace vos;

ORefCount::~ORefCount()
{

}


OReference::OReference()
{
}

OReference::~OReference()
{
    VOS_ASSERT(m_RefCount.referenced() == 0);
}

IReference::RefCount OReference::acquire()
{
    return (m_RefCount.acquire());
}

IReference::RefCount OReference::release()
{
    RefCount Count = m_RefCount.release();

    if (Count == 0)
        delete this;

    return (Count);
}


