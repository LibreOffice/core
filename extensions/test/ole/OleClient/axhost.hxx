/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: axhost.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:48:54 $
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

#ifndef  __AXHOST_HXX
#define __AXHOST_HXX

#include <atlbase.h>
extern CComModule _Module;
//#include <atlwin.h>
#include <atlcom.h>
#include <atlhost.h>


class HostWin: public CWindowImpl<HostWin, CWindow,
               CWinTraits< WS_CAPTION|WS_POPUPWINDOW|WS_VISIBLE, 0> >
{
    CComBSTR controlName;
    CComPtr<IUnknown> spControl;
public:
    HostWin(LPWSTR progid);

    ~HostWin();


    BEGIN_MSG_MAP(HostWin)
        MESSAGE_HANDLER( WM_CREATE, OnCreate)
    END_MSG_MAP()

    IUnknown* GetHostedControl(){
        return spControl;
    }

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};
#endif


