/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
// XMergeFactory.h: interface for the CXMergeFactory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMERGEFACTORY_H__3150043C_57FB_4BC8_9104_379506FA6B9F__INCLUDED_)
#define AFX_XMERGEFACTORY_H__3150043C_57FB_4BC8_9104_379506FA6B9F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CXMergeFactory : public IClassFactory
{
private:
    LONG m_cRef; 
    virtual ~CXMergeFactory() {};

public:
    CXMergeFactory() : m_cRef(1) {};	// Set reference count when first created
    

    /********** IUnknown methods **********/
    STDMETHODIMP			QueryInterface(REFIID riid, void **ppvObject);
    STDMETHODIMP_(ULONG)	AddRef(void);
    STDMETHODIMP_(ULONG)	Release();


    /********** IUnknown methods **********/
    STDMETHODIMP			CreateInstance(IUnknown* pUnkOuter, REFIID riid, void **ppvObject);
    STDMETHODIMP			LockServer(BOOL fLock);
};

#endif // !defined(AFX_XMERGEFACTORY_H__3150043C_57FB_4BC8_9104_379506FA6B9F__INCLUDED_)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
