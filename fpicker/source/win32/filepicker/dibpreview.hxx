/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dibpreview.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 10:52:41 $
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

#ifndef _DIBPREVIEW_HXX_
#define _DIBPREVIEW_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _PREVIEWBASE_HXX_
#include "previewbase.hxx"
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

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
