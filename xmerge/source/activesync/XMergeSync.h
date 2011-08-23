/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
