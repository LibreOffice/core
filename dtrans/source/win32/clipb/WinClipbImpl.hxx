/*************************************************************************
 *
 *  $RCSfile: WinClipbImpl.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: tra $ $Date: 2001-03-16 16:32:44 $
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


#ifndef _WINCLIPBIMPL_HXX_
#define _WINCLIPBIMPL_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDLISTENER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDOWNER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#endif

#ifndef _MTAOLECLIPB_HXX_
#include "..\..\inc\MtaOleClipb.hxx"
#endif

#include <windows.h>
#include <comdef.h>

class CWinClipboard;
class CXNotifyingDataObject;

//---------------------------------------------------
// impl class to avoid deadlocks between XTDataObject
// and the clipboard implementation
//---------------------------------------------------

class CWinClipbImpl
{
public:
    ~CWinClipbImpl( );

protected:
    CWinClipbImpl( const ::rtl::OUString& aClipboardName, CWinClipboard* theWinClipboard );

    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > SAL_CALL getContents(  )
        throw( ::com::sun::star::uno::RuntimeException );

    void SAL_CALL setContents(
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTransferable,
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner )
        throw( ::com::sun::star::uno::RuntimeException );

    ::rtl::OUString SAL_CALL getName(  ) throw( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------
    // XClipboardEx
    //------------------------------------------------

    sal_Int8 SAL_CALL getRenderingCapabilities(  ) throw( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------
    // XFlushableClipboard
    //------------------------------------------------

    void SAL_CALL flushClipboard( ) throw( com::sun::star::uno::RuntimeException );

    //------------------------------------------------
    // XComponent
    //------------------------------------------------

    void SAL_CALL dispose( ) throw( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------
    // member functions
    //------------------------------------------------

    void SAL_CALL registerClipboardViewer( );
    void SAL_CALL unregisterClipboardViewer( );

    static void WINAPI onClipboardContentChanged( void );

private:
    void SAL_CALL onReleaseDataObject( CXNotifyingDataObject* theCaller );

private:
    ::rtl::OUString         m_itsName;
    CMtaOleClipboard        m_MtaOleClipboard;
    CWinClipboard*          m_pWinClipboard;
    CXNotifyingDataObject*  m_pCurrentClipContent;
    osl::Mutex              m_aMutex;
    static CWinClipbImpl*   s_pCWinClipbImpl;

private:
    CWinClipbImpl( const CWinClipbImpl& );
    CWinClipbImpl& operator=( const CWinClipbImpl& );

    friend class CWinClipboard;
    friend class CXNotifyingDataObject;
};

#endif