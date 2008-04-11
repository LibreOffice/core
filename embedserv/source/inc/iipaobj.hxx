/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: iipaobj.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _IIPAOBJ_HXX_
#define _IIPAOBJ_HXX_
#if defined(_MSC_VER) && (_MSC_VER > 1310)
#pragma warning(disable : 4917 4555)
#endif

#include "stdafx.h"
#include <oleidl.h>

#include <osl/interlck.h>
#include <rtl/ref.hxx>
class EmbedDocument_Impl;
class DocumentHolder;

class CIIAObj
    : public IOleInPlaceActiveObject
{

public:

    CIIAObj( DocumentHolder * );
    virtual ~CIIAObj();

    /* IUnknown methods */
    STDMETHODIMP QueryInterface(REFIID, LPVOID FAR * ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    /* IOleInPlaceActiveObject methods */
    STDMETHODIMP GetWindow(HWND *);
    STDMETHODIMP ContextSensitiveHelp(BOOL);
    STDMETHODIMP TranslateAccelerator(LPMSG);
    STDMETHODIMP OnFrameWindowActivate(BOOL);
    STDMETHODIMP OnDocWindowActivate(BOOL);
    STDMETHODIMP ResizeBorder(LPCRECT, LPOLEINPLACEUIWINDOW
                              , BOOL);
    STDMETHODIMP EnableModeless(BOOL);


private:

    oslInterlockedCount                 m_refCount;
    ::rtl::Reference< DocumentHolder >  m_rDocHolder;
};


#endif