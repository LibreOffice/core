/*************************************************************************
 *
 *  $RCSfile: uivwimp.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:32:37 $
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
#ifndef _UIVWIMP_HXX
#define _UIVWIMP_HXX

#ifndef _SWVIEW_HXX //autogen
#include <view.hxx>
#endif

#include <sfx2/objsh.hxx>

#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDLISTENER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx> // helper for implementations
#endif
#ifndef _SWUNODEF_HXX
#include <swunodef.hxx>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

class SwXTextView;
class SfxRequest;
class SwTransferable;

namespace com{ namespace sun{ namespace star {
    namespace frame {
        class XDispatchProviderInterceptor;
    }
    namespace lang {
        class XUnoTunnel;
    }
}}}

/* -----------------------------29.05.00 08:22--------------------------------

 ---------------------------------------------------------------------------*/
class SwScannerEventListener : public ::cppu::WeakImplHelper1<
    STAR_NMSPC::lang::XEventListener >
{
    SwView* pView;

public:

    SwScannerEventListener( SwView& rView ) : pView( &rView )  {}
    virtual ~SwScannerEventListener();

    // XEventListener
    virtual void SAL_CALL disposing(
                    const ::com::sun::star::lang::EventObject& rEventObject ) throw(::com::sun::star::uno::RuntimeException);

    void ViewDestroyed() { pView = 0; }
};

// --------------------------- Clipboard EventListener ------------------

class SwClipboardChangeListener : public ::cppu::WeakImplHelper1<
    CLIP_NMSPC::XClipboardListener >
{
    SwView* pView;

    // XEventListener
    virtual void SAL_CALL disposing( const STAR_NMSPC::lang::EventObject& rEventObject )
        throw ( com::sun::star::uno::RuntimeException );

    // XClipboardListener
    virtual void SAL_CALL changedContents( const CLIP_NMSPC::ClipboardEvent& rEventObject )
        throw ( com::sun::star::uno::RuntimeException );

public:
    SwClipboardChangeListener( SwView& rView ) : pView( &rView ) {}
    virtual ~SwClipboardChangeListener();

    void ViewDestroyed() { pView = 0; }

    void AddRemoveListener( BOOL bAdd );
};


/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
class SwView_Impl
{
    STAR_REFERENCE( lang::XEventListener )  xScanEvtLstnr;
    STAR_REFERENCE( lang::XEventListener )  xClipEvtLstnr;
    STAR_REFERENCE( frame::XDispatchProviderInterceptor )   xDisProvInterceptor;
    STAR_REFERENCE( view::XSelectionSupplier )              *pxXTextView;       // UNO object
    com::sun::star::uno::WeakReference< com::sun::star::lang::XUnoTunnel > xTransferable;

    // temporary document for printing text of selection / multi selection
    // in PDF export.
    SfxObjectShellRef           xTmpSelDocSh;
    SvEmbeddedObjectRef         aEmbeddedObjRef;

    SwView* pView;
    SwScannerEventListener*     pScanEvtLstnr;
    SwClipboardChangeListener*  pClipEvtLstnr;
    ShellModes                  eShellMode;

public:
    SwView_Impl(SwView* pShell);
    ~SwView_Impl();

    void                            SetShellMode(ShellModes eSet);

    ::com::sun::star::view::XSelectionSupplier* GetUNOObject();
    SwXTextView*                    GetUNOObject_Impl();
    void                            Invalidate();

    ShellModes                      GetShellMode() {return eShellMode;}

    void                            ExecuteScan(SfxRequest& rReq);
    SwScannerEventListener&         GetScannerEventListener();

    void                            AddClipboardListener();

    SfxObjectShellRef &             GetTmpSelectionDoc()    { return xTmpSelDocSh; }
    SvEmbeddedObjectRef &           GetEmbeddedObjRef()     { return (SvEmbeddedObjectRef&)(long&)aEmbeddedObjRef; }

    void                            AddTransferable(SwTransferable& rTransferable);
};
#endif

