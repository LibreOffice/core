/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tbxctrl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:31:02 $
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
#ifndef _SFXTBXCTRL_HXX
#define _SFXTBXCTRL_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif
#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _FLOATWIN_HXX //autogen
#include <vcl/floatwin.hxx>
#endif
#include <sfx2/ctrlitem.hxx>
#include <sfx2/sfxstatuslistener.hxx>
#ifndef _SVTOOLS_TOOLBOXCONTROLLER_HXX
#include <svtools/toolboxcontroller.hxx>
#endif
#ifndef _SVTOOLS_FRAMESTATUSLISTENER_HXX
#include <svtools/framestatuslistener.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XDOCKABLEWINDOWLISTENER_HPP_
#include <com/sun/star/awt/XDockableWindowListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XDOCKABLEWINDOW_HPP_
#include <com/sun/star/awt/XDockableWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSUBTOOLBARCONTROLLER_HPP_
#include <com/sun/star/frame/XSubToolbarController.hpp>
#endif

//------------------------------------------------------------------

class SfxUnoToolBoxControl;
class SfxToolBoxControl;
class SfxBindings;
class SfxModule;
class SfxUnoControllerItem;

svt::ToolboxController* SAL_CALL SfxToolBoxControllerFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, ToolBox* pToolbox, unsigned short nID, const ::rtl::OUString& aCommandURL );

//typedef SfxToolBoxControl* (*SfxToolBoxControlCtor)( USHORT nId, ToolBox &rTbx, SfxBindings & );
typedef SfxToolBoxControl* (*SfxToolBoxControlCtor)( USHORT nSlotId, USHORT nId, ToolBox& rBox );

struct SfxTbxCtrlFactory
{
    SfxToolBoxControlCtor   pCtor;
    TypeId                  nTypeId;
    USHORT                  nSlotId;

    SfxTbxCtrlFactory( SfxToolBoxControlCtor pTheCtor,
            TypeId nTheTypeId, USHORT nTheSlotId ):
        pCtor(pTheCtor),
        nTypeId(nTheTypeId),
        nSlotId(nTheSlotId)
    {}
};

//------------------------------------------------------------------

enum SfxPopupWindowType
{
    SFX_POPUPWINDOW_NONE,
    SFX_POPUPWINDOW_ONCLICK,
    SFX_POPUPWINDOW_ONTIMEOUT,
    SFX_POPUPWINDOW_ONCLICKANDMOVE,
    SFX_POPUPWINDOW_ONTIMEOUTANDMOVE,
    SFX_POPUPWINDOW_CONTEXTMENU
};

//------------------------------------------------------------------

class SfxFrameStatusListener : public svt::FrameStatusListener
{
    public:
        SfxFrameStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rServiceManager,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
                                SfxStatusListenerInterface* pCallee );
        virtual ~SfxFrameStatusListener();

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event )
            throw ( ::com::sun::star::uno::RuntimeException );

    private:
        SfxStatusListenerInterface* m_pCallee;
};

//------------------------------------------------------------------

/*  FloatingWindows, die aus ToolBoxen abgerissen werden k"onnen, sollten
    dieser Klasse abgeleitet werden. Da sie ebenfalls von SfxControllerItem
    abgeleitet ist, erhalten ihre Instanzen auch die StateChanged Aufrufe.
*/

class SFX2_DLLPUBLIC SfxPopupWindow: public FloatingWindow, public SfxStatusListenerInterface
{
friend class SfxToolBox_Impl;
    BOOL                                                                             m_bFloating;
    ULONG                                                                            m_nEventId;
    BOOL                                                                             m_bCascading;
    Link                                                                             m_aDeleteLink;
    USHORT                                                                           m_nId;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >              m_xFrame;
    SfxFrameStatusListener*                                                          m_pStatusListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >           m_xStatusListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceManager;

private:
    SfxFrameStatusListener* GetOrCreateStatusListener();

    SAL_DLLPRIVATE SfxPopupWindow(SfxPopupWindow &); // not defined
    SAL_DLLPRIVATE void operator =(SfxPopupWindow &); // not defined

//#if 0 // _SOLAR__PRIVATE
    DECL_DLLPRIVATE_LINK( Delete, void * );
//#endif

protected:
    virtual void            PopupModeEnd();
    virtual BOOL            Close();
    virtual void            DeleteFloatingWindow();

    USHORT                  GetId() const { return m_nId; }
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& GetFrame() const { return m_xFrame; }
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& GetServiceManager() const { return m_xServiceManager; }

    void                    BindListener();
    void                    UnbindListener();
    void                    AddStatusListener( const rtl::OUString& rCommandURL );
    void                    RemoveStatusListener( const rtl::OUString& rCommandURL );
    void                    UpdateStatus( const rtl::OUString& rCommandURL );

    // SfxStatusListenerInterface
    using FloatingWindow::StateChanged;
    virtual void            StateChanged( USHORT nSID, SfxItemState eState,
                                          const SfxPoolItem* pState );

public:
                            SfxPopupWindow( USHORT nId,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                                            WinBits nBits );
                            SfxPopupWindow( USHORT nId,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                                            const ResId &rId );
                            SfxPopupWindow( USHORT nId,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                                            Window* pParentWindow,
                                            const ResId &rId );
                            SfxPopupWindow( USHORT nId,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                                            Window* pParentWindow,
                                            WinBits nBits );
                            ~SfxPopupWindow();

    virtual SfxPopupWindow* Clone() const;
    virtual void            MouseMove( const MouseEvent& rMEvt );

    void                    StartCascading();
    void                    EndCascading();
    SAL_DLLPRIVATE void SetDeleteLink_Impl( const Link& rLink )
                            {
                                m_aDeleteLink = rLink;
                            }
};

//------------------------------------------------------------------
//------------------------------------------------------------------

#define SFX_DECL_TOOLBOX_CONTROL() \
        static SfxToolBoxControl* CreateImpl( USHORT nSlotId, USHORT nId, ToolBox &rTbx ); \
        static void RegisterControl(USHORT nSlotId = 0, SfxModule *pMod=NULL)

/*  F"ur spezielle ToolBox-Controls, z.B. eine Font-Auswahl-Box oder
    aus ToolBoxen abrei"sbare FloatingWindows mu"s passend zur Item-Subclass
    eine Subclass von SfxTooBoxControl implementiert werden.

    Diese Klasse mu"s in SfxApplication::Init() mit der statischen Methode
    RegisterControl() registriert werden. Der SFx erzeugt dann automatisch
    diese Controls in ToolBoxen, wenn die dazugeh"origen Slots von dem
    angegebenen Typ sind.
 */

struct SfxToolBoxControl_Impl;
class SFX2_DLLPUBLIC SfxToolBoxControl:
                         public ::com::sun::star::awt::XDockableWindowListener,
                         public ::com::sun::star::frame::XSubToolbarController,
                         public svt::ToolboxController

{
friend class SfxToolbox;
friend class SfxToolBox_Impl;
friend class SfxToolboxCustomizer;
friend class SfxPopupWindow;
friend struct SfxTbxCtrlFactory;

    SfxToolBoxControl_Impl*    pImpl;

protected:
    DECL_LINK( PopupModeEndHdl, void * );
    DECL_LINK( ClosePopupWindow, SfxPopupWindow * );

    // old SfxToolBoxControl methods
    virtual void               StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual void               Select( BOOL bMod1 = FALSE );
    virtual void               Select( USHORT nModifier );

    virtual void               DoubleClick();
    virtual void               Click();
    virtual SfxPopupWindowType GetPopupWindowType() const;
    virtual SfxPopupWindow*    CreatePopupWindow();
    virtual SfxPopupWindow*    CreatePopupWindowCascading();
    virtual Window*            CreateItemWindow( Window *pParent );

    // Must be called by subclass to set a new popup window instance
    void                       SetPopupWindow( SfxPopupWindow* pWindow );

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void               SAL_CALL acquire() throw();
    virtual void               SAL_CALL release() throw();

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent ) throw( ::com::sun::star::uno::RuntimeException );

    // XComponent
    virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);

    // new controller API
    // XStatusListener
    virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event )
        throw ( ::com::sun::star::uno::RuntimeException );

    // XToolbarController
    virtual void SAL_CALL execute( sal_Int16 KeyModifier )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL click()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL doubleClick()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createPopupWindow()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createItemWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& rParent )
        throw (::com::sun::star::uno::RuntimeException);

    // XSubToolbarController
    virtual ::sal_Bool SAL_CALL opensSubToolbar(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getSubToolbarName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL functionSelected( const ::rtl::OUString& aCommand ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL updateImage(  ) throw (::com::sun::star::uno::RuntimeException);

    //  XDockableWindowListener
    virtual void SAL_CALL startDocking( const ::com::sun::star::awt::DockingEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::DockingData SAL_CALL docking( const ::com::sun::star::awt::DockingEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endDocking( const ::com::sun::star::awt::EndDockingEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL prepareToggleFloatingMode( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL toggleFloatingMode( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closed( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endPopupMode( const ::com::sun::star::awt::EndPopupModeEvent& e ) throw (::com::sun::star::uno::RuntimeException);

    // helper methods
    void    createAndPositionSubToolBar( const ::rtl::OUString& rSubToolBarResName );
    ::Size  getPersistentFloatingSize( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame, const ::rtl::OUString& rSubToolBarResName );

public:
                               SFX_DECL_TOOLBOX_CONTROL();

                               SfxToolBoxControl( USHORT nSlotID, USHORT nId, ToolBox& rBox, BOOL bShowStrings = FALSE );
    virtual                    ~SfxToolBoxControl();

    ToolBox&                   GetToolBox() const;
    unsigned short             GetId() const;
    unsigned short             GetSlotId() const;

    void                       Dispatch( const ::rtl::OUString& aCommand,
                                         ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs );
    static void                Dispatch( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& rDispatchProvider,
                                         const rtl::OUString& rCommand,
                                         ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs );

    static SfxItemState        GetItemState( const SfxPoolItem* pState );
    static SfxToolBoxControl*  CreateControl( USHORT nSlotId, USHORT nTbxId, ToolBox *pBox, SfxModule *pMod );
    static void                RegisterToolBoxControl( SfxModule*, SfxTbxCtrlFactory*);
};

#define SFX_IMPL_TOOLBOX_CONTROL(Class, nItemClass) \
        SfxToolBoxControl* __EXPORT Class::CreateImpl( USHORT nSlotId, USHORT nId, ToolBox &rTbx ) \
               { return new Class( nSlotId, nId, rTbx ); } \
        void Class::RegisterControl(USHORT nSlotId, SfxModule *pMod) \
               { SfxToolBoxControl::RegisterToolBoxControl( pMod, new SfxTbxCtrlFactory( \
                    Class::CreateImpl, TYPE(nItemClass), nSlotId ) ); }

#define SFX_IMPL_TOOLBOX_CONTROL_ARG(Class, nItemClass, Arg) \
        SfxToolBoxControl* __EXPORT Class::CreateImpl( USHORT nSlotId, USHORT nId, ToolBox &rTbx ) \
               { return new Class( nSlotId, nId, rTbx, Arg); } \
        void Class::RegisterControl(USHORT nSlotId, SfxModule *pMod) \
               { SfxToolBoxControl::RegisterToolBoxControl( pMod, new SfxTbxCtrlFactory( \
                    Class::CreateImpl, TYPE(nItemClass), nSlotId ) ); }

//=========================================================================

//#if 0 // _SOLAR__PRIVATE

class SfxDragButton_Impl : public FixedImage
{
public:

                    SfxDragButton_Impl( Window *pParent );
    virtual void    Command ( const CommandEvent& rCEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
};

class SfxDragToolBoxControl_Impl : public SfxToolBoxControl
/*  [Beschreibung]

*/

{
public:
                            SFX_DECL_TOOLBOX_CONTROL();
                            SfxDragToolBoxControl_Impl( USHORT nId, ToolBox& rBox );
    virtual Window*         CreateItemWindow( Window *pParent );
    using SfxToolBoxControl::Select;
    virtual void            Select( BOOL bMod1 = FALSE );
};

//------------------------------------------------------------------------

class SfxAppToolBoxControl_Impl : public SfxToolBoxControl

/*  [Beschreibung]

    Interne Hilfsklasse f"ur um das Popup-Menu <AppMenu_Impl> unter Neu
    im SDT zu starten.
*/

{
public:
                            SFX_DECL_TOOLBOX_CONTROL();
                            SfxAppToolBoxControl_Impl( USHORT nSlotId, USHORT nId, ToolBox& rBox );
                            ~SfxAppToolBoxControl_Impl();
    void                    SetImage( const String& rFacName );

                            struct ExecuteInfo
                            {
                                ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >     xDispatch;
                                ::com::sun::star::util::URL                                                aTargetURL;
                                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >  aArgs;
                            };

                            DECL_STATIC_LINK( SfxAppToolBoxControl_Impl, ExecuteHdl_Impl, ExecuteInfo* );

protected:
    virtual void            Click();
    using SfxToolBoxControl::Select;
    virtual void            Select( BOOL );
    virtual void            StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual SfxPopupWindow* CreatePopupWindow();
                            DECL_LINK( Activate, Menu * ); // Needed to support high contrast images
private:
    String                  aLastURL;
    BOOL                    bBigImages;
    PopupMenu*              pMenu;
    BOOL                    m_bWasHiContrastMode;
    BOOL                    m_bShowMenuImages;
};

class SfxHistoryToolBoxControl_Impl : public SfxToolBoxControl
{
    Timer                   aTimer;

private:
                            DECL_LINK( Timeout, Timer * );

protected:
    virtual void            Click( );
    using SfxToolBoxControl::Select;
    virtual void            Select( BOOL );

public:
                            SFX_DECL_TOOLBOX_CONTROL();
                            SfxHistoryToolBoxControl_Impl( USHORT nId, ToolBox& rBox );
};

class SfxReloadToolBoxControl_Impl : public SfxToolBoxControl
{
    protected:
        using SfxToolBoxControl::Select;
        virtual void Select( USHORT nSelectModifier );

    public:
    SFX_DECL_TOOLBOX_CONTROL();
        SfxReloadToolBoxControl_Impl( USHORT nSlotId, USHORT nId, ToolBox& rBox );
};

class SfxPopupMenuManager;
class SfxAddonsToolBoxControl_Impl : public SfxToolBoxControl

/*  [Description]

    Internal helper class to provide the addons popup menu through the addons
    toolbox button.
*/

{
    BOOL        bBigImages;
    PopupMenu*  pMenu;
    BOOL        m_bWasHiContrastMode;
    BOOL        m_bShowMenuImages;

protected:
    virtual void            Click();
    using SfxToolBoxControl::Select;
    virtual void            Select( BOOL );
    virtual void            StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
                            DECL_LINK( Activate, Menu * ); // Needed to support high contrast images
public:
                            SFX_DECL_TOOLBOX_CONTROL();
                            SfxAddonsToolBoxControl_Impl( USHORT nSlotId, USHORT nId, ToolBox& rBox );
                            ~SfxAddonsToolBoxControl_Impl();

                            void RefreshMenuImages( Menu* pMenu );
};

//#endif

#endif
