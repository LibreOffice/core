/*************************************************************************
 *
 *  $RCSfile: atlwindow.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jl $ $Date: 2001-07-19 11:14:24 $
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
#ifndef _WINDOW_HXX_
#define _WINDOW_HXX_
#include <atlbase.h>
extern CComModule _Module;
#include<atlcom.h>
#include<atlctl.h>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/uno/Reference.h>
#include "../../source/inc/DtObjFactory.hxx"


using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::lang;
#define WM_SOURCE_INIT WM_APP+100
#define WM_SOURCE_STARTDRAG WM_APP+101


struct StartDragData
{
    Reference<XDragSource> source;
    Reference<XTransferable> transferable;
};

struct InitializationData
{
    Reference<XInitialization> xInit;
    HWND hWnd;
};

class AWindow: public CWindowImpl<AWindow, CWindow,
      CWinTraits<WS_CAPTION |WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0> >
{
    TCHAR m_strName[80];
    Reference<XDropTarget> m_xDropTarget;
    Reference<XDragSource> m_xDragSource;
    BOOL m_isMTA;
    BOOL m_bInitInMTA;
    HWND m_hwndEdit;
    // Id of the MTA thread to which we post messages.
    DWORD m_idMTAThread;
    CDTransObjFactory m_aDataConverter;

public:
    AWindow(LPCTSTR strName, DWORD idMTAThread): m_idMTAThread( idMTAThread)
    {
        RECT rcPos= {0,0,200,200};
        Create(0, rcPos, strName);
    }
    AWindow(LPCTSTR strName, DWORD idMTAThread, RECT pos, BOOL mta=FALSE,
            BOOL initInMTA=FALSE):
        m_isMTA( mta), m_idMTAThread( idMTAThread), m_bInitInMTA( initInMTA)
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
