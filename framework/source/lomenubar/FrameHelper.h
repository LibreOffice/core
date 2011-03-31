/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * A LibreOffice extension to send the menubar structure through DBusMenu
 *
 * Copyright 2011 Canonical, Ltd.
 * Authors:
 *     Alberto Ruiz <alberto.ruiz@codethink.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the the GNU Lesser General Public License version 3, as published by the Free
 * Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR PURPOSE.  See the applicable
 * version of the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with this program. If not, see <http://www.gnu.org/licenses/>
 *
 */

#ifndef __FRAME_HELPER_H__
#define __FRAME_HELPER_H__

#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/XMenu.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>

#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/menuitem.h>

using com::sun::star::awt::KeyEvent;
using com::sun::star::awt::XMenu;
using com::sun::star::beans::PropertyValue;
using com::sun::star::container::XNameAccess;
using com::sun::star::frame::FrameActionEvent;
using com::sun::star::frame::XFrame;
using com::sun::star::frame::XFramesSupplier;
using com::sun::star::frame::XFrameActionListener;
using com::sun::star::frame::XStatusListener;
using com::sun::star::frame::XDispatchProvider;
using com::sun::star::frame::XModuleManager;
using com::sun::star::frame::XModel;
using com::sun::star::lang::EventObject;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::lang::XMultiComponentFactory;
using com::sun::star::uno::Any;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::RuntimeException;
using com::sun::star::util::XURLTransformer;
using com::sun::star::ui::XAcceleratorConfiguration;
using rtl::OUString;

/* This class is a helper in charge of closing the dbusmenu server when a frame is closed,
 * and also allows the menuitem callbacks to dispatch commands.
 */

class FrameHelper : public cppu::WeakImplHelper1 < XFrameActionListener >
{
  private:
    Reference < XFrame >                xFrame;
    Reference < XMultiServiceFactory >  xMSF;
    Reference < XNameAccess >           xUICommands;
    DbusmenuServer                     *server;
    DbusmenuMenuitem                   *root;
    gboolean                            watcher_set;
    guint                               watcher;
    XStatusListener                    *xSL;
    Reference < XURLTransformer >       xTrans;
    Reference < XDispatchProvider >     xdp;
    GHashTable                         *commandsInfo;
    gboolean                            blockDetach;

    //These object/methods are used to recreate dynamic popupmenus
    Reference < XMultiComponentFactory > xPCF;
    Reference < XModuleManager>          xMM;
    Sequence  < Any >                    args;

    gboolean                             isSpecialSubmenu (OUString command);

    //This is to build the shortcut database
    Reference< XAcceleratorConfiguration > docAccelConf;
    Reference< XAcceleratorConfiguration > modAccelConf;
    Reference< XAcceleratorConfiguration > globAccelConf;

    void getAcceleratorConfigurations (Reference < XModel >,
                                       Reference < XModuleManager>);

    KeyEvent findShortcutForCommand  (OUString);

  public:
    FrameHelper(const Reference< XMultiServiceFactory >&,
                const Reference< XFrame >&,
                DbusmenuServer*);

    virtual ~FrameHelper();
    virtual void SAL_CALL frameAction(const FrameActionEvent& action)
      throw (RuntimeException);

    virtual void SAL_CALL disposing(const EventObject& aEvent)
      throw (RuntimeException);

    //Setters
    void setRootItem         (DbusmenuMenuitem *);
    void setRegistrarWatcher (guint watcher);
    void setServer           (DbusmenuServer *);

    //Getters
    Reference < XFrame > getFrame               ();
    unsigned long        getXID                 ();
    GHashTable*          getCommandsInfo        ();
    XStatusListener*     getStatusListener      ();
    ::rtl::OUString      getLabelFromCommandURL (::rtl::OUString);

    //Menu Related actions
    void dispatchCommand     (OUString);
    void rebuildMenu         (Reference < XMenu >, DbusmenuMenuitem*);
    void rebuildMenuFromRoot ();
    void populateWindowList  (DbusmenuMenuitem *);
};
#endif // __FRAME_HELPER_H__
