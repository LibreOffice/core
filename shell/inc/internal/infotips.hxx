/*************************************************************************
 *
 *  $RCSfile: infotips.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-04-07 10:52:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef INFOTIPS_HXX_INCLUDED
#define INFOTIPS_HXX_INCLUDED

#include <objidl.h>
#include <shlobj.h>
#include <string>

class CInfoTip : public IQueryInfo, public IPersistFile
{
public:
    CInfoTip(long RefCnt = 1);
    virtual ~CInfoTip();

    //-----------------------------
    // IUnknown methods
    //-----------------------------

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID riid,
            void __RPC_FAR *__RPC_FAR *ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef( void);

    virtual ULONG STDMETHODCALLTYPE Release( void);

    //----------------------------
    // IQueryInfo methods
    //----------------------------

    virtual HRESULT STDMETHODCALLTYPE GetInfoTip(DWORD dwFlags, wchar_t** ppwszTip);

    virtual HRESULT STDMETHODCALLTYPE GetInfoFlags(DWORD *pdwFlags);

    //----------------------------
    // IPersist methods
    //----------------------------

    virtual HRESULT STDMETHODCALLTYPE GetClassID(CLSID* pClassID);

    //----------------------------
    // IPersistFile methods
    //----------------------------

    virtual HRESULT STDMETHODCALLTYPE IsDirty(void);

    virtual HRESULT STDMETHODCALLTYPE Load(
            /* [in] */ LPCOLESTR pszFileName,
            /* [in] */ DWORD dwMode);

    virtual HRESULT STDMETHODCALLTYPE Save(
            /* [unique][in] */ LPCOLESTR pszFileName,
            /* [in] */ BOOL fRemember);

    virtual HRESULT STDMETHODCALLTYPE SaveCompleted(
            /* [unique][in] */ LPCOLESTR pszFileName);

    virtual HRESULT STDMETHODCALLTYPE GetCurFile(
            /* [out] */ LPOLESTR __RPC_FAR *ppszFileName);

private:
    long         m_RefCnt;
    char         m_szFileName[MAX_PATH];
    std::wstring m_FileNameOnly;
};

#endif
