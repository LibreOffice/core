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

#ifndef _CLIPLISTENER_HXX
#define _CLIPLISTENER_HXX

#include "svtools/svtdllapi.h"
#include <tools/link.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>

class Window;


class SVT_DLLPUBLIC TransferableClipboardListener : public ::cppu::WeakImplHelper1<
                            ::com::sun::star::datatransfer::clipboard::XClipboardListener >
{
    Link    aLink;

public:
            // Link is called with a TransferableDataHelper pointer
            TransferableClipboardListener( const Link& rCallback );
            ~TransferableClipboardListener();

    void    AddRemoveListener( Window* pWin, sal_Bool bAdd );
    void    ClearCallbackLink();

            // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
                                            throw(::com::sun::star::uno::RuntimeException);
            // XClipboardListener
    virtual void SAL_CALL changedContents( const ::com::sun::star::datatransfer::clipboard::ClipboardEvent& event )
                                            throw(::com::sun::star::uno::RuntimeException);
};

#endif

