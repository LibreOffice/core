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

#ifndef _DIBPREVIEW_HXX_
#define _DIBPREVIEW_HXX_

#include "previewbase.hxx"
#include <osl/mutex.hxx>

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

//---------------------------------------------
// A very simple wrapper for a window that does
// display DIBs.
// Maybe it would be better and more extensible
// to create another class that is responsible
// for rendering a specific image format into
// the area of the window, but for our purpose
// it's enough to go the simple way - KISS.
//---------------------------------------------

class CDIBPreview : public PreviewBase
{
public:

    // ctor
    CDIBPreview(HINSTANCE instance,HWND parent,sal_Bool bShowWindow = sal_False);

    // dtor
    virtual ~CDIBPreview( );

    // preview interface implementation

    virtual sal_Int32 SAL_CALL getTargetColorDepth()
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getAvailableWidth()
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getAvailableHeight()
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setImage(sal_Int16 aImageFormat, const ::com::sun::star::uno::Any& aImage)
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL setShowState(sal_Bool bShowState)
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL getShowState()
        throw (::com::sun::star::uno::RuntimeException);

    virtual HWND SAL_CALL getWindowHandle() const;

private:
    virtual void SAL_CALL onPaint( HWND hWnd, HDC hDC );

    ATOM SAL_CALL RegisterDibPreviewWindowClass( );
    void SAL_CALL UnregisterDibPreviewWindowClass( );

    static LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

private:
    HINSTANCE  m_Instance;
    HWND       m_Hwnd;
    com::sun::star::uno::Sequence<sal_Int8> m_Image;
    osl::Mutex  m_PaintLock;

    // the preview window class has to be registered only
    // once per process, so multiple instance of this class
    // share the registered window class
    static ATOM       s_ClassAtom;
    static osl::Mutex s_Mutex;
    static sal_Int32  s_RegisterDibPreviewWndCount;

// prevent copy and assignment
private:
    CDIBPreview(const CDIBPreview&);
    CDIBPreview& operator=(const CDIBPreview&);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
