/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: source.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-24 10:48:56 $
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

#ifndef _SOURCE_HXX_
#define _SOURCE_HXX_

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDRAGSOURCE_HPP_
#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDRAGSOURCECONTEXT_HPP_
#include <com/sun/star/datatransfer/dnd/XDragSourceContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _OSL_MUTEX_H_
#include <osl/mutex.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase3.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#include "../../inc/DtObjFactory.hxx"
#include "globals.hxx"
#include <oleidl.h>

#include <systools/win32/comtools.hxx>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace cppu;
using namespace osl;
using namespace rtl;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::dnd;



class SourceContext;
// RIGHT MOUSE BUTTON drag and drop not supportet currently.
// ALT modifier is considered to effect a user selection of effects
class DragSource:
      public MutexDummy,
      public WeakComponentImplHelper3<XDragSource, XInitialization, XServiceInfo>,
      public IDropSource

{
    Reference<XMultiServiceFactory> m_serviceFactory;
    HWND m_hAppWindow;

    // The mouse button that set off the drag and drop operation
    short m_MouseButton;
    // Converts XTransferable objects to IDataObject objects.
    CDTransObjFactory m_aDataConverter;

    DragSource();
    DragSource(const DragSource&);
    DragSource &operator= ( const DragSource&);

    // First starting a new drag and drop thread if
    // the last one has finished
    void StartDragImpl(
        const DragGestureEvent& trigger,
        sal_Int8 sourceActions,
        sal_Int32 cursor,
        sal_Int32 image,
        const Reference<XTransferable >& trans,
        const Reference<XDragSourceListener >& listener);

public:
    long m_RunningDndOperationCount;

public:
    // only valid for one dnd operation
    // the thread ID of the thread which created the window
    DWORD m_threadIdWindow;
    // The context notifies the XDragSourceListener s
    Reference<XDragSourceContext>   m_currentContext;

    // the wrapper for the Transferable ( startDrag)
    IDataObjectPtr m_spDataObject;

    sal_Int8 m_sourceActions;

public:
    DragSource(const Reference<XMultiServiceFactory>& sf);
    virtual ~DragSource();

  // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments )
        throw(Exception, RuntimeException);


    // XDragSource
    virtual sal_Bool SAL_CALL isDragImageSupported(  ) throw(RuntimeException);
    virtual sal_Int32 SAL_CALL getDefaultCursor( sal_Int8 dragAction )
        throw( IllegalArgumentException, RuntimeException);
    virtual void SAL_CALL startDrag( const DragGestureEvent& trigger,
                                     sal_Int8 sourceActions,
                                     sal_Int32 cursor,
                                     sal_Int32 image,
                                     const Reference<XTransferable >& trans,
                                     const Reference<XDragSourceListener >& listener )
                throw( RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (RuntimeException);



    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef( );

    virtual ULONG STDMETHODCALLTYPE Release( );


    // IDropSource
    virtual HRESULT STDMETHODCALLTYPE QueryContinueDrag(
        /* [in] */ BOOL fEscapePressed,
        /* [in] */ DWORD grfKeyState);

    virtual HRESULT STDMETHODCALLTYPE GiveFeedback(
        /* [in] */ DWORD dwEffect);

};


#endif
