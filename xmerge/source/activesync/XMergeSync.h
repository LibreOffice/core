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
// XMergeSyncModule.h: interface for the CXMergeSyncModule class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMERGESYNCMODULE_H__0788DA0C_4DCB_4876_9722_F9EAF1EB5462__INCLUDED_)
#define AFX_XMERGESYNCMODULE_H__0788DA0C_4DCB_4876_9722_F9EAF1EB5462__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Used to keep track of the dll

class CXMergeSyncModule
{
protected:
    long  m_lLocks;
    long  m_lObjs;

public:
    long GetLockCount();
    long LockServer(BOOL fLock);
    HINSTANCE m_hInst;
    CXMergeSyncModule();
    virtual ~CXMergeSyncModule();
};

#endif // !defined(AFX_XMERGESYNCMODULE_H__0788DA0C_4DCB_4876_9722_F9EAF1EB5462__INCLUDED_)
