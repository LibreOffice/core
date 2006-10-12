/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui.h,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 14:07:30 $
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

#if ! defined INCLUDED_DP_GUI_H
#define INCLUDED_DP_GUI_H

#include "dp_misc.h"
#include "dp_gui_cmdenv.h"
#include "dp_gui.hrc"
#include "rtl/ref.hxx"
#include "osl/thread.h"
#include "cppuhelper/implbase2.hxx"
#include "vcl/svapp.hxx"
#include "vcl/dialog.hxx"
#include "vcl/button.hxx"
#include "vcl/fixed.hxx"
#include "svtools/svtabbx.hxx"
#include "svtools/headbar.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/awt/XWindow.hpp"
#include "com/sun/star/ucb/XContentEventListener.hpp"
#include "com/sun/star/deployment/thePackageManagerFactory.hpp"
#include "com/sun/star/deployment/ui/PackageManagerDialog.hpp"
#include "com/sun/star/frame/XDesktop.hpp"
#include <list>
#include <memory>


namespace css = ::com::sun::star;

namespace dp_gui {

enum PackageState { REGISTERED, NOT_REGISTERED, AMBIGUOUS, NOT_AVAILABLE };

PackageState getPackageState(
    css::uno::Reference<css::deployment::XPackage> const & xPackage,
    css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv =
    css::uno::Reference<css::ucb::XCommandEnvironment>() );

//==============================================================================
struct DialogImpl :
        public ModelessDialog,
        public ::cppu::WeakImplHelper2< css::frame::XTerminateListener,
                                        css::ucb::XContentEventListener >
{
    static ResId getResId( USHORT id );
    static String getResourceString( USHORT id );

    struct SelectionBoxControl : public Control
    {
        SelectionBoxControl( DialogImpl * dialog )
            : Control( dialog, WB_BORDER | WB_TABSTOP ),
              m_dialog(dialog)
            {}
        long Notify( NotifyEvent & rEvt );

    private:
        DialogImpl * m_dialog;
    };

    struct TreeListBoxImpl : public SvHeaderTabListBox
    {
        typedef ::std::list<
            css::uno::Reference<css::uno::XInterface> > t_nodeList;
        t_nodeList m_nodes;
        SvLBoxEntry * addNode(
            SvLBoxEntry * parentNode,
            String const & displayName,
            ::rtl::OUString const & factoryURL,
            css::uno::Reference<css::deployment::XPackageManager>
            const & xPackageManager,
            css::uno::Reference<css::deployment::XPackage> const & xPackage,
            css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
            bool sortIn = true );
        SvLBoxEntry * addPackageNode(
            SvLBoxEntry * parentNode,
            css::uno::Reference<css::deployment::XPackage> const & xPackage,
            css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv);

        DialogImpl * m_dialog;
        SvLBoxEntry * m_currentSelectedEntry;
        bool m_hiContrastMode;
        Timer m_timer;

        String m_strEnabled;
        String m_strDisabled;
        String m_strUnknown;
        String m_strCtxAdd;
        String m_strCtxRemove;
        String m_strCtxEnable;
        String m_strCtxDisable;
        String m_strCtxExport;

        Image m_defaultPackage;
        Image m_defaultPackage_hc;
        Image m_defaultPackageBundle;
        Image m_defaultPackageBundle_hc;

        DECL_LINK( TimerHandler, Timer * );
        virtual void SelectHdl();
        virtual void DeselectHdl();
        virtual void MouseMove( MouseEvent const & evt );
        virtual void KeyInput( KeyEvent const & evt );
        virtual void RequestingChilds( SvLBoxEntry * pEntry );
        using SvListView::Expand;
        virtual BOOL Expand( SvLBoxEntry * pParent );
        virtual void DataChanged( DataChangedEvent const & evt );
        virtual PopupMenu * CreateContextMenu(void);
        virtual void ExcecuteContextMenuAction( USHORT nSelectedPopupEntry );

        virtual ~TreeListBoxImpl();
        TreeListBoxImpl( Window * pParent, DialogImpl * dialog );

        SvLBoxEntry * getCurrentSingleSelectedEntry() const;
        bool isFirstLevelChild( SvLBoxEntry * entry ) const;
        ::rtl::OUString getContext( SvLBoxEntry * entry ) const;
        css::uno::Reference<css::deployment::XPackage> getPackage(
            SvLBoxEntry * entry ) const;
        css::uno::Sequence<css::uno::Reference<css::deployment::XPackage> >
        getSelectedPackages( bool onlyFirstLevel = false ) const;
    };

    class SyncPushButton : public PushButton
    {
    public:
        typedef void (DialogImpl::* t_clickCallback)( USHORT id );
        inline SyncPushButton(
            DialogImpl * dialog, t_clickCallback cb, USHORT id )
            : PushButton( dialog, getResId(id) ),
              m_dialog(dialog), m_clickCallback(cb), m_id(id) {}
        // PushButton
        virtual void Click();
    private:
        DialogImpl * m_dialog;
        t_clickCallback m_clickCallback;
        USHORT m_id;
    };

    class ThreadedPushButton : public SyncPushButton
    {
        oslThread m_thread;
    public:
        virtual ~ThreadedPushButton();
        inline ThreadedPushButton(
            DialogImpl * dialog, t_clickCallback cb, USHORT id )
            : SyncPushButton( dialog, cb, id ), m_thread(0) {}
        // PushButton
        virtual void Click();
    };

    virtual BOOL Close();
    virtual void Resize();
    DECL_LINK( headbar_dragEnd, HeaderBar * );

    // solar thread functions, because of gtk file/folder picker:
    css::uno::Sequence<rtl::OUString> solarthread_raiseAddPicker(
        css::uno::Reference<css::deployment::XPackageManager>
        const & xPackageManager );
    bool solarthread_raiseExportPickers(
        css::uno::Sequence< css::uno::Reference<css::deployment::XPackage> >
        const & selection,
        rtl::OUString & rDestFolder, rtl::OUString & rNewTitle,
        sal_Int32 & rNameClashAction );

    void clickClose( USHORT id );
    void clickAdd( USHORT id );
    void clickRemove( USHORT id );
    void clickEnableDisable( USHORT id );
    void clickExport( USHORT id );

    bool m_allowSharedLayerModification;
    void updateButtonStates(
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv =
        com::sun::star::uno::Reference<
        com::sun::star::ucb::XCommandEnvironment>() );

    void errbox( ::rtl::OUString const & msg );

    css::uno::Reference<css::uno::XComponentContext> m_xComponentContext;
    css::uno::Reference<css::deployment::XPackageManagerFactory> m_xPkgMgrFac;
    css::uno::Reference<css::frame::XDesktop> m_xDesktop;
    css::uno::Reference<css::ucb::XContent> m_xTdocRoot;

    Size m_buttonSize;
    Size m_relatedSpace;
    Size m_unrelatedSpace;
    Size m_borderLeftTopSpace;
    Size m_borderRightBottomSpace;
    long m_ftFontHeight;
    long m_descriptionYSpace;

    const String m_strAddPackages;
    const String m_strAddingPackages;
    const String m_strRemovingPackages;
    const String m_strEnablingPackages;
    const String m_strDisablingPackages;
    const String m_strExportPackage;
    const String m_strExportPackages;
    const String m_strExportingPackages;

    // controls:
    ::std::auto_ptr<FixedText> m_ftPackages;
    ::std::auto_ptr<SelectionBoxControl> m_selectionBox;
    ::std::auto_ptr<HeaderBar> m_headerBar;
    ::std::auto_ptr<TreeListBoxImpl> m_treelb;

    ::std::auto_ptr<ThreadedPushButton> m_addButton;
    ::std::auto_ptr<ThreadedPushButton> m_removeButton;
    ::std::auto_ptr<ThreadedPushButton> m_enableButton;
    ::std::auto_ptr<ThreadedPushButton> m_disableButton;
    ::std::auto_ptr<ThreadedPushButton> m_exportButton;
    ::std::auto_ptr<FixedLine> m_bottomLine;

    ::std::auto_ptr<OKButton> m_closeButton;
    ::std::auto_ptr<HelpButton> m_helpButton;

    DECL_STATIC_LINK( DialogImpl, destroyDialog, void * );
    static ::rtl::Reference<DialogImpl> s_dialog;
    virtual ~DialogImpl();
    DialogImpl(
        Window * pParent,
        css::uno::Reference< css::uno::XComponentContext > const & xContext );
    static ::rtl::Reference<DialogImpl> get(
        css::uno::Reference<css::uno::XComponentContext> const & xContext,
        css::uno::Reference<css::awt::XWindow> const & xParent = 0,
        ::rtl::OUString const & view = ::rtl::OUString() );
    // XEventListener
    virtual void SAL_CALL disposing( css::lang::EventObject const & evt )
        throw (css::uno::RuntimeException);
    // XContentEventListener
    virtual void SAL_CALL contentEvent( css::ucb::ContentEvent const & evt )
        throw (css::uno::RuntimeException);
    // XTerminateListener
    virtual void SAL_CALL queryTermination( css::lang::EventObject const & evt )
        throw (css::frame::TerminationVetoException,
               css::uno::RuntimeException);
    virtual void SAL_CALL notifyTermination(
        css::lang::EventObject const & evt ) throw (css::uno::RuntimeException);
};

} // namespace dp_gui

#endif
