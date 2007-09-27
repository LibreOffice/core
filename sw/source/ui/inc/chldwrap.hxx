/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chldwrap.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:54:58 $
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
#ifndef _CHLDWRAP_HXX
#define _CHLDWRAP_HXX

#ifndef _SFX_CHILDWIN_HXX //autogen
#include <sfx2/childwin.hxx>
#endif

#ifndef _SV_TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif

class SwDocShell;

class SwChildWinWrapper : public SfxChildWindow
{
    Timer       m_aUpdateTimer;
    SwDocShell* m_pDocSh;

    DECL_LINK( UpdateHdl, void* );

    // Implementation in fldtdlg.cxx
protected:
    SwChildWinWrapper(Window *pParentWindow, USHORT nId);

public:
    virtual BOOL    ReInitDlg(SwDocShell *pDocSh);

    inline SwDocShell*  GetOldDocShell()                    { return m_pDocSh; }
    inline void         SetOldDocShell(SwDocShell *pDcSh)   { m_pDocSh = pDcSh; }
};


#endif

