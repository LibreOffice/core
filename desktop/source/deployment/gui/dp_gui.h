/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui.h,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 11:02:04 $
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

#include "dp_gui_cmdenv.h"
#include "dp_gui_updatedata.hxx"
#include "dp_misc.h"
#include "dp_gui_updatability.hxx"
#include "dp_gui_addextensionqueue.hxx"
#include "dp_gui.hrc"
#include "rtl/ref.hxx"
#include "rtl/instance.hxx"
#include "osl/thread.hxx"
#include "cppuhelper/implbase2.hxx"
#include "vcl/svapp.hxx"
#include "vcl/dialog.hxx"
#include "vcl/button.hxx"
#include "vcl/fixed.hxx"
#include "salhelper/simplereferenceobject.hxx"
#include "svtools/svtabbx.hxx"
#include "svtools/headbar.hxx"
#include "com/sun/star/ucb/XContentEventListener.hpp"
#include "osl/mutex.hxx"
#include <list>
#include <memory>
#include <queue>

namespace com { namespace sun { namespace star {
    namespace container {
        class XNameAccess;
    }
    namespace frame {
        class XDesktop;
    }
    namespace awt {
        class XWindow;
    }
    namespace uno {
        class XComponentContext;
    }
    namespace deployment {
        class XPackageManagerFactory;
    }
} } }

namespace svt {
    class FixedHyperlink;
}

namespace dp_gui {

class UpdateDialog;
enum PackageState { REGISTERED, NOT_REGISTERED, AMBIGUOUS, NOT_AVAILABLE };

PackageState getPackageState(
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage> const & xPackage,
    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment> const & xCmdEnv =
    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment>() );


// Use this mutex whenever a button is clicked, such as Add, Update, Options, in order to prevent that
// an extension is installed which was startet asynchronously. For example, an office is running with the
// Extension Manager open and the user installs an extension by double-clicking one in a file browser.
struct ActionMutex : public rtl::Static< ::osl::Mutex, ActionMutex>{};

//==============================================================================
struct DialogImpl :
        public ModelessDialog,
        public ::cppu::WeakImplHelper2< ::com::sun::star::frame::XTerminateListener,
                                        ::com::sun::star::ucb::XContentEventListener >
{
    static ResId getResId( USHORT id );
    static String getResourceString( USHORT id );

    struct SelectionBoxControl : public Control
    {
    public:

        SelectionBoxControl( DialogImpl * dialog );

        long Notify( NotifyEvent & rEvt );

        /* Signals that the dialog (DialogImpl) is about to be destroyed.
            In this case we must prevent to access members of the dialog,
            which is the case in Notify. Notify may be called when the dialog
            is being destructed.
        */
        bool m_bShutDown;
    private:
        DialogImpl * m_dialog;
    };

    struct TreeListBoxImpl : public SvHeaderTabListBox
    {
        typedef ::std::list<
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> > t_nodeList;
        t_nodeList m_nodes;
        SvLBoxEntry * addNode(
            SvLBoxEntry * parentNode,
            String const & displayName,
            ::rtl::OUString const & factoryURL,
            ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager>
            const & xPackageManager,
            ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage> const & xPackage,
            ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment> const & xCmdEnv,
            bool sortIn = true );
        SvLBoxEntry * addPackageNode(
            SvLBoxEntry * parentNode,
            ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage> const & xPackage,
            ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment> const & xCmdEnv);

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> m_context;
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
        String m_strCtxCheckUpdate;
        String m_strCtxOptions;

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
        TreeListBoxImpl(
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> const & context,
            Window * pParent, DialogImpl * dialog );

        SvLBoxEntry * getCurrentSingleSelectedEntry() const;
        bool isFirstLevelChild( SvLBoxEntry * entry ) const;
        ::rtl::OUString getContext( SvLBoxEntry * entry ) const;
        ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage> getPackage(
            SvLBoxEntry * entry ) const;
        void select(::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage> const &   xPackage);

        ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager> getPackageManager(
            SvLBoxEntry * entry ) const;

        /** returns a vector of pairs of packages and the respective package managers.

            Only returns valid packages. That is the top-level nodes StarOffice Extensions and
            my Extensions have no associated XPackage.
         */
        ::std::vector<
            ::std::pair<
                ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage>,
                ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager> > >
        getSelectedPackages(bool onlyFirstLevel) const;
        typedef ::std::vector<
            ::std::pair<
                ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage>,
                ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager> > >::const_iterator
                CI_PAIR_PACKAGE;
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
    DECL_LINK( hyperlink_clicked, svt::FixedHyperlink * );

    // solar thread functions, because of gtk file/folder picker:
    ::com::sun::star::uno::Sequence<rtl::OUString> solarthread_raiseAddPicker(
        ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager>
        const & xPackageManager );
    bool solarthread_raiseExportPickers(
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage> >
        const & selection,
        rtl::OUString & rDestFolder, rtl::OUString & rNewTitle,
        sal_Int32 & rNameClashAction );

    void clickClose( USHORT id );
    void clickAdd( USHORT id );
    void clickRemove( USHORT id );
    void clickEnableDisable( USHORT id );
    void clickExport( USHORT id );
    void clickCheckUpdates( USHORT id );
    void clickOptions( USHORT id );
//  void installExtensions();

    void updateButtonStates(
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment> const & xCmdEnv =
        com::sun::star::uno::Reference<
        com::sun::star::ucb::XCommandEnvironment>() );


    void checkUpdates( bool selected, bool showUpdateOnly = false, bool parentVisible = true );
    void errbox( ::rtl::OUString const & msg, Window const * pParent = NULL ) const;
    bool supportsOptions( ::rtl::OUString const & sExtensionId);
    void openWebBrowser(::rtl::OUString const & sURL) const;

    /** return a vector which only contains information for directly downloadable updates.
    */
    static ::std::vector<dp_gui::UpdateData> excludeWebsiteDownloads(
        ::std::vector<dp_gui::UpdateData> const & data);


    enum ACTION { ADD_ACTION, REMOVE_ACTION, ENABLE_ACTION, DISABLE_ACTION, UPDATE_ACTION };
    /** checks if an some action is done with a shared extension.

        If so, the user will be warned by  message box, reading that all other office instances
        need to be closed. If the user selects the cancel button then this function returns false.
        Otherwise it returns true.
        When the action is UPDATE_ACTION then, and only then, pUpdateDialog must be provided.
     */
    bool continueActionForSharedExtension(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::deployment::XPackageManager> const & xPMgr, ACTION a,
            Window * pUpdateDialog = NULL);

    /** called from update dialog.
    */
    bool continueUpdateForSharedExtension(
        Window * pUpdateDialog,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::deployment::XPackageManager> const & xPMgr);


    UpdateDialog* m_pUpdateDialog;
    /** The extensions which are passed when the Extension Manager dialog is created.
        This is the case when the Extension Manager is started as a result of a
        "system integration operation", such as double clicking on a oxt file.
        This extension is always installed as user extension.
    */
    ::rtl::OUString m_extensionURL;

    /** This flag is used to show that there are extensions being installed and the buttons should
        therefore be disabled. For example, when the Extension Manager was started as a result of
        double-click on an extension in a file browser, then that extension is being immediately
        installed. During that time the buttons and context menus should be disabled.
    */
    bool m_bAddingExtensions;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> m_xComponentContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManagerFactory> m_xPkgMgrFac;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager> >
        m_packageManagers;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDesktop> m_xDesktop;
    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent> m_xTdocRoot;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> m_xNameAccessNodes;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> m_xNameAccessRepositories;

     const String m_strAddPackages;
    const String m_strAddingPackages;
    const String m_strRemovingPackages;
    const String m_strEnablingPackages;
    const String m_strDisablingPackages;
    const String m_strExportPackage;
    const String m_strExportPackages;
    const String m_strExportingPackages;

    Size m_buttonSize;
    Size m_textSize;
    Size m_relatedSpace;
    Size m_unrelatedSpace;
    Size m_borderLeftTopSpace;
    Size m_borderRightBottomSpace;
    long m_ftFontHeight;
    long m_descriptionYSpace;


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
    ::std::auto_ptr<SyncPushButton> m_checkUpdatesButton;
    ::std::auto_ptr<ThreadedPushButton> m_optionsButton;
    ::std::auto_ptr<svt::FixedHyperlink> m_getExtensionsButton;
    ::std::auto_ptr<FixedLine> m_bottomLine;

    ::std::auto_ptr<OKButton> m_closeButton;
    ::std::auto_ptr<HelpButton> m_helpButton;

    ::std::auto_ptr<Updatability> m_updatability;
    ::std::auto_ptr<AddExtensionQueue> m_addExtensionQueue;

    DECL_LINK( destroyDialog, DialogImpl *);
    DECL_LINK( startInstallExtensions, DialogImpl *);
    static ::rtl::Reference<DialogImpl> s_dialog;
    /** This mutex is used to protect s_dialog.
    */
    static ::osl::Mutex s_dialogMutex;

    /** When the user closes the Extension Manager dialog, then the current instance of
        the dialog will be transferred from s_dialog to s_closingDialog. The dialog will
        remain open until all remaining installation request (see m_addExtensionsQueue)
        are processed. See DialogImpl::Close and DialogImpl::destroyDialog in dp_gui_treelb.cxx.
    */
    static ::rtl::Reference<DialogImpl> s_closingDialog;
    /* This mutex is used to protect s_closingDialog
    */
    static ::osl::Mutex s_closingMutex;

    virtual ~DialogImpl();
    DialogImpl(
        Window * pParent,
        ::rtl::OUString const & extensionURL,
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext );
    static ::rtl::Reference<DialogImpl> get(
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> const & xContext,
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow> const & xParent = 0,
        ::rtl::OUString const & extensionURL = ::rtl::OUString(),
        ::rtl::OUString const & view = ::rtl::OUString() );
    // XEventListener
    virtual void SAL_CALL disposing( ::com::sun::star::lang::EventObject const & evt )
        throw (::com::sun::star::uno::RuntimeException);
    // XContentEventListener
    virtual void SAL_CALL contentEvent( ::com::sun::star::ucb::ContentEvent const & evt )
        throw (::com::sun::star::uno::RuntimeException);
    // XTerminateListener
    virtual void SAL_CALL queryTermination( ::com::sun::star::lang::EventObject const & evt )
        throw (::com::sun::star::frame::TerminationVetoException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL notifyTermination(
        ::com::sun::star::lang::EventObject const & evt ) throw (::com::sun::star::uno::RuntimeException);
};

class SelectedPackageIterator: public salhelper::SimpleReferenceObject {
public:
    SelectedPackageIterator(DialogImpl::TreeListBoxImpl & list);

    virtual ~SelectedPackageIterator();

    void next(
        ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage> * package,
        ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager> * packageManager);
        // must only be called with Application::GetSolarMutex() locked and
        // while the DialogImpl::TreeListBoxImpl is still alive

private:
    SelectedPackageIterator(SelectedPackageIterator &); // not defined
    void operator =(SelectedPackageIterator &); // not defined

    DialogImpl::TreeListBoxImpl & m_list;
    SvLBoxEntry * m_entry;
};

} // namespace dp_gui

#endif
