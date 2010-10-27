/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef __FRAMEWORK_UIELEMENT_STATUSBAR_HXX_
#define __FRAMEWORK_UIELEMENT_STATUSBAR_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <uielement/statusbarmanager.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
