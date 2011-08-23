/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _UIVWIMP_HXX
#define _UIVWIMP_HXX

#ifndef _SWVIEW_HXX //autogen
#include <view.hxx>
#endif

#include <bf_sfx2/objsh.hxx>

#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDLISTENER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>	// helper for implementations
#endif
#ifndef _SWUNODEF_HXX
#include <swunodef.hxx>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_ 
#include <cppuhelper/weakref.hxx>
#endif
namespace com{ namespace sun{ namespace star {
    namespace frame {
        class XDispatchProviderInterceptor;
    }
    namespace lang {
        class XUnoTunnel;
    }
}}}
namespace binfilter {

class SwXTextView;
class SfxRequest;

class SwView_Impl
{
    STAR_REFERENCE( frame::XDispatchProviderInterceptor )	xDisProvInterceptor;
    STAR_REFERENCE( view::XSelectionSupplier ) 				*pxXTextView;		// UNO object
    ::com::sun::star::uno::WeakReference< ::com::sun::star::lang::XUnoTunnel > xTransferable;
    
    SfxObjectShellRef           xTmpSelDocSh;
    SvEmbeddedObjectRef         aEmbeddedObjRef;
    
    SwView* pView;
    ShellModes 					eShellMode;

public:
    SwView_Impl(SwView* pShell);
    ~SwView_Impl();

    void							SetShellMode(ShellModes eSet);

    ::com::sun::star::view::XSelectionSupplier* GetUNOObject();
    SwXTextView*					GetUNOObject_Impl();
    void                            Invalidate();

    ShellModes						GetShellMode() {return eShellMode;}


    void 							AddClipboardListener();

    SfxObjectShellRef &             GetTmpSelectionDoc()    { return xTmpSelDocSh; }
    SvEmbeddedObjectRef &           GetEmbeddedObjRef()     { return (SvEmbeddedObjectRef&)(long&)aEmbeddedObjRef; }

};
} //namespace binfilter
#endif

