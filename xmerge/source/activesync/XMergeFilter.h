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

// XMergeFilter.h: interface for the CXMergeFilter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMERGEFILTER_H__25C39F6B_A1D7_408E_8F58_9CBEE9A666CC__INCLUDED_)
#define AFX_XMERGEFILTER_H__25C39F6B_A1D7_408E_8F58_9CBEE9A666CC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000





class CXMergeFilter : public ICeFileFilter
{
protected:
    long m_cRef;

private:
    TCHAR* GetXMergeClassPath();
    TCHAR* GetJavaBaseDir();

    TCHAR* m_szJavaBaseDir;
    TCHAR* m_szClasspath;

    BOOL m_bHaveExcel;
    BOOL m_bHaveWord;


public:
    static const LPTSTR m_pszPSWExportCLSID;
    static const LPTSTR m_pszPSWExportExt;
    static const LPTSTR m_pszPSWExportDesc;
    static const LPTSTR m_pszPSWExportShortDesc;

    static const LPTSTR m_pszPSWImportCLSID;
    static const LPTSTR m_pszPSWImportExt;
    static const LPTSTR m_pszPSWImportDesc;
    static const LPTSTR m_pszPSWImportShortDesc;

    static const LPTSTR m_pszPXLExportCLSID;
    static const LPTSTR m_pszPXLExportExt;
    static const LPTSTR m_pszPXLExportDesc;
    static const LPTSTR m_pszPXLExportShortDesc;

    static const LPTSTR m_pszPXLImportCLSID;
    static const LPTSTR m_pszPXLImportExt;
    static const LPTSTR m_pszPXLImportDesc;
    static const LPTSTR m_pszPXLImportShortDesc;

public:
    CXMergeFilter();
    virtual ~CXMergeFilter();


    /********** IUnknown methods **********/
    STDMETHODIMP            QueryInterface(REFIID iid, void **ppvObject);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

    /********** ICeFileFilter methods *********/
    STDMETHODIMP            FilterOptions(HWND hwndParent);
    STDMETHODIMP            FormatMessage(DWORD dwFlags, DWORD dwMessageId, DWORD dwLanguageId,
                                          LPTSTR lpBuffer, DWORD nSize, va_list *Arguments, DWORD *pcb);
    STDMETHODIMP            NextConvertFile(int nConversion, CFF_CONVERTINFO *pci,
                                            CFF_SOURCEFILE *psf, CFF_DESTINATIONFILE *pdf,
                                            volatile BOOL *pbCancel, CF_ERROR *perr);

};

#endif // !defined(AFX_XMERGEFILTER_H__25C39F6B_A1D7_408E_8F58_9CBEE9A666CC__INCLUDED_)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
