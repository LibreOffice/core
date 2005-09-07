/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stbitem.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:20:14 $
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
#ifndef _SFXSTBITEM_HXX
#define _SFXSTBITEM_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif
#ifndef _SVTOOLS_STATUSBARCONTROLLER_HXX
#include <svtools/statusbarcontroller.hxx>
#endif

//------------------------------------------------------------------

class SfxModule;
class StatusBar;
class SfxStatusBarControl;
class SfxBindings;

svt::StatusbarController* SAL_CALL SfxStatusBarControllerFactory(
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
    StatusBar* pStatusBar,
    unsigned short nID,
    const ::rtl::OUString& aCommandURL );
typedef SfxStatusBarControl* (*SfxStatusBarControlCtor)( USHORT nSlotId, USHORT nId, StatusBar &rStb );

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

class CommandEvent;
class MouseEvent;
class UserDrawEvent;

class SFX2_DLLPUBLIC SfxStatusBarControl: public svt::StatusbarController
{
    USHORT          nSlotId;
    USHORT          nId;
    StatusBar*      pBar;

protected:
    // new controller API
    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void               SAL_CALL acquire() throw();
    virtual void               SAL_CALL release() throw();

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent ) throw( ::com::sun::star::uno::RuntimeException );

    // XComponent
    virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);

    // XStatusListener
    virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event )
        throw ( ::com::sun::star::uno::RuntimeException );

    // XStatusbarController
    virtual ::sal_Bool SAL_CALL mouseButtonDown( const ::com::sun::star::awt::MouseEvent& aMouseEvent ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL mouseMove( const ::com::sun::star::awt::MouseEvent& aMouseEvent ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL mouseButtonUp( const ::com::sun::star::awt::MouseEvent& aMouseEvent ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL command( const ::com::sun::star::awt::Point& aPos,
                                    ::sal_Int32 nCommand,
                                    ::sal_Bool bMouseEvent,
                                    const ::com::sun::star::uno::Any& aData ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL paint( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& xGraphics,
                                    const ::com::sun::star::awt::Rectangle& rOutputRectangle,
                                    ::sal_Int32 nItemId, ::sal_Int32 nStyle ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL click() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL doubleClick() throw (::com::sun::star::uno::RuntimeException);

    // Old sfx2 interface
    virtual void    StateChanged( USHORT nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );
    virtual void    Click();
    virtual void    DoubleClick();
    virtual void    Command( const CommandEvent& rCEvt );
    virtual BOOL    MouseButtonDown( const MouseEvent & );
    virtual BOOL    MouseMove( const MouseEvent & );
    virtual BOOL    MouseButtonUp( const MouseEvent & );
    virtual void    Paint( const UserDrawEvent &rUDEvt );

    static USHORT   convertAwtToVCLMouseButtons( sal_Int16 nAwtMouseButtons );

public:
                    SfxStatusBarControl( USHORT nSlotID, USHORT nId, StatusBar& rBar );
    virtual         ~SfxStatusBarControl();

    USHORT          GetSlotId() const { return nSlotId; }
    USHORT          GetId() const { return nId; }
    StatusBar&      GetStatusBar() const { return *pBar; }
    void            CaptureMouse();
    void            ReleaseMouse();

    static SfxStatusBarControl* CreateControl( USHORT nSlotID, USHORT nId, StatusBar *pBar, SfxModule* );
};

//------------------------------------------------------------------

#define SFX_DECL_STATUSBAR_CONTROL() \
        static SfxStatusBarControl* CreateImpl( USHORT nSlotId, USHORT nId, StatusBar &rStb ); \
        static void RegisterControl(USHORT nSlotId = 0, SfxModule *pMod=NULL)

#define SFX_IMPL_STATUSBAR_CONTROL(Class, nItemClass) \
        SfxStatusBarControl* __EXPORT Class::CreateImpl( USHORT nSlotId, USHORT nId, StatusBar &rStb ) \
               { return new Class( nSlotId, nId, rStb ); } \
        void Class::RegisterControl(USHORT nSlotId, SfxModule *pMod) \
               { SFX_APP()->RegisterStatusBarControl( pMod, new SfxStbCtrlFactory( \
                    Class::CreateImpl, TYPE(nItemClass), nSlotId ) ); }


#endif
