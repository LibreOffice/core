/*************************************************************************
 *
 *  $RCSfile: propsheets.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-04-07 10:53:32 $
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

#ifndef PROPSHEETS_HXX_INCLUDED
#define PROPSHEETS_HXX_INCLUDED

#ifndef METAINFO_HXX_INCLUDED
#include "internal/metainfo.hxx"
#endif

#include <shlobj.h>
#include <string>
#include <memory>

class CPropertySheet : public IShellExtInit, public IShellPropSheetExt
{
public:
    CPropertySheet(long RefCnt = 1);

    virtual ~CPropertySheet();

    //-----------------------------
    // IUnknown methods
    //-----------------------------

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID riid,
            void __RPC_FAR *__RPC_FAR *ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef( void);

    virtual ULONG STDMETHODCALLTYPE Release( void);

    //-----------------------------
    // IShellExtInit
    //-----------------------------

    virtual HRESULT STDMETHODCALLTYPE Initialize(
        LPCITEMIDLIST pidlFolder, LPDATAOBJECT lpdobj, HKEY hkeyProgID);

    //-----------------------------
    // IShellPropSheetExt
    //-----------------------------

    virtual HRESULT STDMETHODCALLTYPE AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);

    virtual HRESULT STDMETHODCALLTYPE ReplacePage(
        UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam);

private:
    // Windows callback functions
    static UINT CALLBACK PropPageSummaryCallback(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);
    static BOOL CALLBACK PropPageSummaryProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK PropPageStatisticsProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);


    // Helper functions
    void InitPropPageSummary(HWND hwnd, LPPROPSHEETPAGE lppsp);
    void InitPropPageStatistics(HWND hwnd, LPPROPSHEETPAGE lppsp);

private:
    long m_RefCnt;
    char m_szFileName[MAX_PATH];
    std::auto_ptr<COpenOfficeMetaInformation> m_pMetaInfo;
};

#endif
