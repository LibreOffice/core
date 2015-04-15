/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_EMBEDSERV_SOURCE_INC_IIPAOBJ_HXX
#define INCLUDED_EMBEDSERV_SOURCE_INC_IIPAOBJ_HXX
#ifdef _MSC_VER
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
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

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
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
