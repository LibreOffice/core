/*************************************************************************
 *
 *  $RCSfile: stbitem.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:24 $
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
#ifndef _SFXSTBITEM_HXX
#define _SFXSTBITEM_HXX

#ifndef _SFXCTRLITEM_HXX
#include "ctrlitem.hxx"
#endif

class SvStream;
class SvStream;
class SfxModule;

//------------------------------------------------------------------

class StatusBar;
class SfxStatusBarControl;
class SfxBindings;

typedef SfxStatusBarControl* (*SfxStatusBarControlCtor)( USHORT nId, StatusBar &rStb, SfxBindings & );

struct SfxStbCtrlFactory
{
    SfxStatusBarControlCtor pCtor;
    TypeId                  nTypeId;
    USHORT                  nSlotId;

    SfxStbCtrlFactory( SfxStatusBarControlCtor pTheCtor,
            TypeId nTheTypeId, USHORT nTheSlotId ):
        pCtor(pTheCtor),
        nTypeId(nTheTypeId),
        nSlotId(nTheSlotId)
    {}
};

//------------------------------------------------------------------

class  CommandEvent;
class  MouseEvent;
class UserDrawEvent;

class SfxStatusBarControl: public SfxControllerItem
{
friend class SfxStatusBar_Impl;

    StatusBar*      pBar;

protected:
    virtual void    StateChanged( USHORT nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );
    virtual void    Click();
    virtual void    DoubleClick();
    virtual void    Command( const CommandEvent& rCEvt );
    virtual BOOL    MouseButtonDown( const MouseEvent & );
    virtual BOOL    MouseMove( const MouseEvent & );
    virtual BOOL    MouseButtonUp( const MouseEvent & );
    virtual void    Paint( const UserDrawEvent &rUDEvt );

public:
                    SfxStatusBarControl( USHORT nId, StatusBar& rBar, SfxBindings & );
    virtual         ~SfxStatusBarControl();

    StatusBar&      GetStatusBar() const { return *pBar; }
    void            CaptureMouse();
    void            ReleaseMouse();

    static SfxStatusBarControl* CreateControl( USHORT nId, StatusBar *pBar, SfxBindings&, SfxModule* );
};

//------------------------------------------------------------------

#define SFX_DECL_STATUSBAR_CONTROL() \
        static SfxStatusBarControl* CreateImpl( USHORT nId, StatusBar &rStb, SfxBindings &rBindings ); \
        static void RegisterControl(USHORT nSlotId = 0, SfxModule *pMod=NULL)

#define SFX_IMPL_STATUSBAR_CONTROL(Class, nItemClass) \
        SfxStatusBarControl* __EXPORT Class::CreateImpl( USHORT nId, StatusBar &rStb, SfxBindings &rBindings ) \
               { return new Class(nId, rStb, rBindings); } \
        void Class::RegisterControl(USHORT nSlotId, SfxModule *pMod) \
               { SFX_APP()->RegisterStatusBarControl( pMod, new SfxStbCtrlFactory( \
                    Class::CreateImpl, TYPE(nItemClass), nSlotId ) ); }


#endif

