/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WinClipbImpl.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:01:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
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

#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

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
    osl::Mutex              m_ClipContentMutex;

    static osl::Mutex       s_aMutex;
    static CWinClipbImpl*   s_pCWinClipbImpl;

private:
    CWinClipbImpl( const CWinClipbImpl& );
    CWinClipbImpl& operator=( const CWinClipbImpl& );

    friend class CWinClipboard;
    friend class CXNotifyingDataObject;
};

#endif
