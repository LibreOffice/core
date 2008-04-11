/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: atlwindow.hxx,v $
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
#ifndef _WINDOW_HXX_
#define _WINDOW_HXX_
#include <atlbase.h>
extern CComModule _Module;
#include<atlcom.h>
#include<atlctl.h>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/uno/Reference.h>
#include "../../source/inc/DtObjFactory.hxx"


using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::datatransfer;

struct ThreadData
{
    Reference<XDragSource> source;
    Reference<XTransferable> transferable;
    HANDLE evtThreadReady;
};

class AWindow: public CWindowImpl<AWindow, CWindow,
      CWinTraits<WS_CAPTION |WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0> >
{
    TCHAR m_strName[80];
    Reference<XDropTarget> m_xDropTarget;
    Reference<XDragSource> m_xDragSource;
    BOOL m_isMTA;

    HWND m_hwndEdit;

    CDTransObjFactory m_aDataConverter;

public:
    AWindow(LPCTSTR strName)
    {
        RECT rcPos= {0,0,200,200};
        Create(0, rcPos, strName);
    }
    AWindow(LPCTSTR strName, RECT pos, BOOL mta=FALSE): m_isMTA( mta)
    {
        Create(0, pos, strName);
    }

    ~AWindow()
    {
        if(m_hWnd)
            DestroyWindow();
    }


    BEGIN_MSG_MAP(AWindow)
        MESSAGE_HANDLER( WM_CLOSE, OnClose)
        MESSAGE_HANDLER( WM_CREATE, OnCreate)
        MESSAGE_RANGE_HANDLER( WM_MOUSEFIRST,  WM_MOUSELAST, OnMouseAction)
        MESSAGE_HANDLER( WM_TIMER, OnTimer)
        MESSAGE_HANDLER( WM_SIZE, OnSize)
        MESSAGE_HANDLER( WM_SETFOCUS, OnFocus)

    END_MSG_MAP()

    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnMouseAction(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

};

#endif
