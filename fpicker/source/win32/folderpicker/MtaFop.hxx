/*************************************************************************
 *
 *  $RCSfile: MtaFop.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-06 15:57:42 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _MTAFOP_HXX_
#define _MTAFOP_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include <utility>
#include <objidl.h>
#include <shlobj.h>

#ifndef _WINIMPLHELPER_HXX_
#include "..\misc\WinImplHelper.hxx"
#endif

//----------------------------------------------------------------
// a simple helper class used to provide a buffer for different
// Win32 file and directory functions
//----------------------------------------------------------------

class CAutoPathBuff
{
public:
    CAutoPathBuff( size_t size = 0 )
    {
        if (0 == size)
            size = 32000; // max path length under Win2000

        pBuff = new sal_Unicode[size];

        OSL_POSTCOND(pBuff,"Could not allocate path buffer");
    }

    ~CAutoPathBuff( )
    {
        delete [] pBuff;
    }

    operator sal_Unicode*( )
    {
        OSL_PRECOND( pBuff, \
            "No path buffer allocated" );
        return pBuff;
    }

    sal_Unicode* get( )
    {
        OSL_PRECOND( pBuff, \
            "No path buffer allocated" );
        return pBuff;
    }

private:
    sal_Unicode* pBuff;
};

//--------------------------------------------------------
// the Mta-Ole clipboard class is for internal use only!
// only one instance of this class should be created, the
// user has to ensure this!
// the class is not thread-safe because it will be used
// only from within the clipboard service and the methods
// of the clipboard service are already synchronized
//--------------------------------------------------------

class CMtaFolderPicker
{
public:
    CMtaFolderPicker( sal_uInt32 Flags );
    virtual ~CMtaFolderPicker( );

    // shell functions
    sal_Bool SAL_CALL browseForFolder( );

    virtual void  SAL_CALL setDisplayDirectory( const rtl::OUString& aDirectory );
    virtual rtl::OUString  SAL_CALL getDisplayDirectory( );
    virtual rtl::OUString SAL_CALL getDirectory( );

    virtual void SAL_CALL setDescription( const rtl::OUString& aDescription );

    virtual void SAL_CALL setTitle( const rtl::OUString& aTitle );
    rtl::OUString  SAL_CALL getTitle( );

    //-----------------------------------------------------
    // XCancellable
    //-----------------------------------------------------

    virtual void SAL_CALL cancel( );

protected:
    void SAL_CALL enableOk( sal_Bool bEnable );
    void SAL_CALL setSelection( const rtl::OUString& aDirectory );
    void SAL_CALL setStatusText( const rtl::OUString& aStatusText );

    virtual void SAL_CALL onInitialized( );
    virtual void SAL_CALL onSelChanged( const rtl::OUString& aNewPath );
    virtual sal_uInt32 SAL_CALL onValidateFailed( sal_Unicode* lpInvalidPath );

private:
    // helper functions
    LPITEMIDLIST  SAL_CALL getItemIdListFromPath( const rtl::OUString& aDirectory );
    rtl::OUString SAL_CALL getPathFromItemIdList( LPCITEMIDLIST lpItemIdList );
    void SAL_CALL releaseItemIdList( LPITEMIDLIST lpItemIdList );

    unsigned int run( );

    // create a hidden windows which serves as an request
    // target; so we guarantee synchronization
    sal_Bool SAL_CALL createStaRequestWindow( );

    //---------------------------------------------------------------
    // message handler functions; remeber these functions are called
    // from a different thread context!
    //---------------------------------------------------------------

    sal_Bool SAL_CALL onBrowseForFolder( );

    static LRESULT CALLBACK StaWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    static unsigned int WINAPI StaThreadProc( LPVOID pParam );

    static int CALLBACK FolderPickerCallback( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData );

protected:
    HWND m_hwnd;

private:
    ATOM SAL_CALL RegisterStaRequestWindowClass( );
    void SAL_CALL UnregisterStaRequestWindowClass( );

private:
    HANDLE                      m_hStaThread;
    unsigned                    m_uStaThreadId;
    HANDLE                      m_hEvtThrdReady;
    HWND                        m_hwndStaRequestWnd;
    rtl::OUString               m_dialogTitle;
    rtl::OUString               m_Description;
    rtl::OUString               m_displayDir;
    rtl::OUString               m_SelectedDir;
    BROWSEINFOW                 m_bi;
    CAutoPathBuff               m_pathBuff;
    HINSTANCE                   m_hInstance;

    // the request window class has to be registered only
    // once per process, so multiple instance of this class
    // share the registered window class
    static ATOM       s_ClassAtom;
    static osl::Mutex s_Mutex;
    static sal_Int32  s_StaRequestWndRegisterCount;

// prevent copy and assignment
private:
    CMtaFolderPicker( const CMtaFolderPicker& );
    CMtaFolderPicker& operator=( const CMtaFolderPicker& );
};

#endif
