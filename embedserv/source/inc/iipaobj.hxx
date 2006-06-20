/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: iipaobj.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:42:33 $
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

#ifndef _IIPAOBJ_HXX_
#define _IIPAOBJ_HXX_

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
