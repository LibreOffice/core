/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: idroptarget.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:02:34 $
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
#ifndef _IDROPTARGET_HXX_
#define _IDROPTARGET_HXX_

#include "target.hxx"

class IDropTargetImpl: public IDropTarget
{
    LONG m_nRefCount;
    // Calls to IDropTarget functions are delegated to a DropTarget.
    DropTarget& m_rDropTarget;

    virtual ~IDropTargetImpl(); // delete is only called by IUnknown::Release
    IDropTargetImpl( const IDropTargetImpl& );
    IDropTargetImpl& operator=( const IDropTargetImpl& );
public:
    IDropTargetImpl( DropTarget& pTarget);

       // IDropTarget
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef( );

    virtual ULONG STDMETHODCALLTYPE Release( );

    virtual HRESULT STDMETHODCALLTYPE DragEnter(
            /* [unique][in] */ IDataObject __RPC_FAR *pDataObj,
            /* [in] */ DWORD grfKeyState,
            /* [in] */ POINTL pt,
            /* [out][in] */ DWORD __RPC_FAR *pdwEffect);

    virtual HRESULT STDMETHODCALLTYPE DragOver(
            /* [in] */ DWORD grfKeyState,
            /* [in] */ POINTL pt,
            /* [out][in] */ DWORD __RPC_FAR *pdwEffect);

    virtual HRESULT STDMETHODCALLTYPE DragLeave( ) ;

    virtual HRESULT STDMETHODCALLTYPE Drop(
            /* [unique][in] */ IDataObject __RPC_FAR *pDataObj,
            /* [in] */ DWORD grfKeyState,
            /* [in] */ POINTL pt,
            /* [out][in] */ DWORD __RPC_FAR *pdwEffect);


};
#endif
