/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: statusbar.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 14:31:48 $
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

#ifndef __FRAMEWORK_UIELEMENT_STATUSBAR_HXX_
#define __FRAMEWORK_UIELEMENT_STATUSBAR_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_UIELEMENT_STATUSBARMANAGER_HXX_
#include <uielement/statusbarmanager.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <vcl/status.hxx>

namespace framework
{

class FrameworkStatusBar : public StatusBar
{
    public:

        FrameworkStatusBar( Window*           pParent,
                            WinBits           nWinBits );
        virtual ~FrameworkStatusBar();

        void         SetStatusBarManager( StatusBarManager* pStatusBarManager );

        virtual void StateChanged( StateChangedType nType );
        virtual void DataChanged( const DataChangedEvent& rDCEvt );
        virtual void UserDraw(const UserDrawEvent& rUDEvt);
        virtual void Command( const CommandEvent &rEvt );
        virtual void MouseMove( const MouseEvent& rMEvt );
        virtual void MouseButtonDown( const MouseEvent& rMEvt );
        virtual void MouseButtonUp( const MouseEvent& rMEvt );

    private:
        StatusBarManager*       m_pMgr;
        sal_Bool                m_bShow : 1,
                                m_bLock : 1;
};

}

#endif // __FRAMEWORK_UIELEMENT_STATUSBAR_HXX_
