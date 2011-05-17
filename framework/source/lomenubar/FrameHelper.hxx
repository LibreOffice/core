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

#ifndef __FRAME_HELPER_HXX__
#define __FRAME_HELPER_HXX__

#include <vector>

#include <boost/scoped_ptr.hpp>

#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/XMenu.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <cppuhelper/implbase1.hxx>

//#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#include <libdbusmenu-glib/server.h>
#pragma GCC diagnostic error "-Wignored-qualifiers"
//#pragma GCC diagnostic pop
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


namespace framework { namespace lomenubar
{
    class DispatchRegistry;
}}

/* This class is a helper in charge of closing the dbusmenu server when a frame is closed,
 * and also allows the menuitem callbacks to dispatch commands.
 */

class FrameHelper : public cppu::WeakImplHelper1 < XFrameActionListener >
{
  private:
    const Reference < XStatusListener > m_xStatusListener;
    ::boost::scoped_ptr< ::framework::lomenubar::DispatchRegistry> m_pDispatchRegistry;
    const Reference < XMultiServiceFactory > m_xMSF;
    const Reference < XURLTransformer > m_xTrans;
    const Reference < XModuleManager> m_xMM;
    const Reference < XMultiComponentFactory > m_xPCF;
    const Reference < XFrame > m_xFrame;
    const Reference < XDispatchProvider > m_xdp;
    const Sequence < Any > m_args;
    Reference < XNameAccess >           m_xUICommands;
    DbusmenuServer                     *m_server;
    DbusmenuMenuitem                   *m_root;
    gboolean                            m_watcher_set;
    guint                               m_watcher;
    GHashTable                         *m_commandsInfo;
    //This variable prevents the helper from being disconnected from the frame
    //for special cases of component dettaching like print preview
    gboolean                            m_blockDetach;


    gboolean                             isSpecialSubmenu (OUString command);

    //This is to build the shortcut database
    Reference< XAcceleratorConfiguration > m_docAccelConf;
    Reference< XAcceleratorConfiguration > m_modAccelConf;
    Reference< XAcceleratorConfiguration > m_globAccelConf;

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
    ::rtl::OUString      getLabelFromCommandURL (::rtl::OUString);

    //Menu Related actions
    void dispatchCommand     (OUString);
    void rebuildMenu         (Reference < XMenu >, DbusmenuMenuitem*);
    void rebuildMenuFromRoot ();
    void populateWindowList  (DbusmenuMenuitem *);
};
#endif // __FRAME_HELPER_HXX__
