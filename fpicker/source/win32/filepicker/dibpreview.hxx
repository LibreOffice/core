/*************************************************************************
 *
 *  $RCSfile: dibpreview.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: tra $ $Date: 2001-07-11 09:20:28 $
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

#ifndef _DIBPREVIEW_HXX_
#define _DIBPREVIEW_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include <windows.h>

//---------------------------------------------
// A very simple wrapper for a window that does
// display DIBs.
// Maybe it would be better and more extensible
// to create another class that is responsible
// for rendering a specific image format into
// the area of the window, but for our purpose
// it's enough to go the simple way - KISS.
//---------------------------------------------

class CDIBPreview
{
public:
    // clients can create instances only
    // through the static create method
    CDIBPreview(
        sal_Int32 x,
        sal_Int32 y,
        sal_Int32 cx,
        sal_Int32 cy,
        HWND aParent,
        HINSTANCE hInstance,
        sal_Bool bShow = sal_True );

    // dtor
    ~CDIBPreview( );

    // window size information

    void SAL_CALL setWidth( sal_Int32 cx_new );

    sal_Int32 SAL_CALL getWidth( ) const;

    void SAL_CALL setHeight( sal_Int32 cy_new );

    sal_Int32 SAL_CALL getHeight( ) const;

    // shows the preview window
    // possible values see SHOW_STATE
    sal_Bool SAL_CALL show( sal_Bool bShow );

    // returns true when the preview window is
    // visible and
    // false if not
    sal_Bool SAL_CALL isVisible( ) const;

    // returns the currently supported color
    // resolution of the preview window
    int SAL_CALL getColorDepth( );

    void SAL_CALL setImage( const ::com::sun::star::uno::Sequence< sal_Int8 >& pImageData );

private:
    virtual void SAL_CALL onPaint( HWND hWnd, HDC hDC );

    ATOM SAL_CALL RegisterDibPreviewWindowClass( );
    void SAL_CALL UnregisterDibPreviewWindowClass( );

    static LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

private:
    HWND                                        m_hwnd;
    HINSTANCE                                   m_hInstance;
    ::com::sun::star::uno::Sequence< sal_Int8 > m_ImageData;
    sal_Int32                                   m_cx;
    sal_Int32                                   m_cy;
    sal_Bool                                    m_bWndClassRegistered;

    // the preview window class has to be registered only
    // once per process, so multiple instance of this class
    // share the registered window class
    static ATOM       s_ClassAtom;
    static osl::Mutex s_Mutex;
    static sal_Int32  s_RegisterDibPreviewWndCount;

// prevent copy and assignment
private:
    CDIBPreview( const CDIBPreview& );
    CDIBPreview& operator=( const CDIBPreview& );
};


#endif