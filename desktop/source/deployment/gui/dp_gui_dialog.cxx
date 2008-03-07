/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui_dialog.cxx,v $
 *
 *  $Revision: 1.34 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 11:02:59 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "svtools/controldims.hrc"
#include "dp_gui.h"
#include "dp_gui_shared.hxx"
#include "dp_gui_updatedialog.hxx"
#include "dp_gui_updateinstalldialog.hxx"
#include "dp_gui_updatedata.hxx"
#include "dp_identifier.hxx"
#include "rtl/uri.hxx"
#include "osl/thread.hxx"
#include "osl/mutex.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/implbase1.hxx"
#include "ucbhelper/content.hxx"
#include "comphelper/anytostring.hxx"
#include "comphelper/sequence.hxx"
#include "tools/resmgr.hxx"
#include "toolkit/helper/vclunohelper.hxx"
#include "vcl/wintypes.hxx"
#include "vcl/msgbox.hxx"
#include "vcl/threadex.hxx"
#include "svtools/svtools.hrc"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "svtools/fixedhyper.hxx"
#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/container/XChild.hpp"
#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/ui/dialogs/XFilePicker.hpp"
#include "com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp"
#include "com/sun/star/ui/dialogs/XFilterManager.hpp"
#include "com/sun/star/ui/dialogs/XFolderPicker.hpp"
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include "com/sun/star/ui/dialogs/ExecutableDialogResults.hpp"
#include "com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp"
#include "com/sun/star/system/SystemShellExecuteFlags.hpp"
#include "com/sun/star/system/XSystemShellExecute.hpp"
#include "com/sun/star/ucb/XContent.hpp"
#include "com/sun/star/ucb/XContentAccess.hpp"
#include "com/sun/star/ucb/NameClash.hpp"
#include "com/sun/star/ucb/ContentAction.hpp"
#include "com/sun/star/sdbc/XResultSet.hpp"
#include "com/sun/star/sdbc/XRow.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/beans/Optional.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/frame/XDesktop.hpp"
#include "com/sun/star/deployment/thePackageManagerFactory.hpp"
#include "boost/function.hpp"
#include "boost/bind.hpp"
#include <sfx2/sfxdlg.hxx>
#include <map>
#include <vector>
#include <algorithm>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;

namespace css = ::com::sun::star;
using ::rtl::OUString;




namespace dp_gui {

const long ITEM_ID_PACKAGE = 1;
const long ITEM_ID_VERSION = 2;
const long ITEM_ID_STATUS = 3;


//______________________________________________________________________________
DialogImpl::DialogImpl(
    Window * pParent, OUString const & extensionURL,
    Reference<XComponentContext> const & xContext )
    : ModelessDialog( pParent, getResId(RID_DLG_PACKAGE_MANAGER) ),
      m_pUpdateDialog(NULL),
      m_extensionURL(extensionURL),
//    m_bAddingExtensions(false),
      m_xComponentContext( xContext ),
      m_xPkgMgrFac( deployment::thePackageManagerFactory::get(xContext) ),
      m_strAddPackages( getResourceString(RID_STR_ADD_PACKAGES) ),
      m_strAddingPackages( getResourceString(RID_STR_ADDING_PACKAGES) ),
      m_strRemovingPackages( getResourceString(RID_STR_REMOVING_PACKAGES) ),
      m_strEnablingPackages( getResourceString(RID_STR_ENABLING_PACKAGES) ),
      m_strDisablingPackages( getResourceString(RID_STR_DISABLING_PACKAGES) ),
      m_strExportPackage( getResourceString(RID_STR_EXPORT_PACKAGE) ),
      m_strExportPackages( getResourceString(RID_STR_EXPORT_PACKAGES) ),
      m_strExportingPackages( getResourceString(RID_STR_EXPORTING_PACKAGES) )
{

    m_addExtensionQueue.reset(new AddExtensionQueue(this, m_xComponentContext,
        m_xPkgMgrFac->getPackageManager(OUSTR("user"))));
    // If the extensionURL contains  a URL then we were
    //started as a result of an install request triggered by user, for example by
    //double clicking an extension in a file browser. This extension will be installed
    //immediatly and for that time we disable the buttons
//     if (extensionURL.getLength())
//         m_bAddingExtensions = true;

    Reference<css::lang::XMultiServiceFactory> xConfig(
        xContext->getServiceManager()->createInstanceWithContext(
            OUSTR("com.sun.star.configuration.ConfigurationProvider"), m_xComponentContext),
        UNO_QUERY_THROW);
    Any args[1];
    css::beans::PropertyValue val1(
        OUSTR("nodepath"), 0, Any(OUSTR("/org.openoffice.Office.OptionsDialog/Nodes")),
        ::css::beans::PropertyState_DIRECT_VALUE);
    args[0] <<= val1;
    m_xNameAccessNodes = Reference<css::container::XNameAccess>(
        xConfig->createInstanceWithArguments(OUSTR("com.sun.star.configuration.ConfigurationAccess"),
          Sequence<Any>(args, 1)), UNO_QUERY_THROW);

    css::beans::PropertyValue val2(
        OUSTR("nodepath"), 0, Any(OUSTR("/org.openoffice.Office.ExtensionManager/ExtensionRepositories")),
        ::css::beans::PropertyState_DIRECT_VALUE);
    args[0] <<= val2;
    m_xNameAccessRepositories = Reference<css::container::XNameAccess>(
        xConfig->createInstanceWithArguments(OUSTR("com.sun.star.configuration.ConfigurationAccess"),
          Sequence<Any>(args, 1)), UNO_QUERY_THROW);
}

//______________________________________________________________________________
DialogImpl::~DialogImpl()
{
}

//------------------------------------------------------------------------------
::rtl::Reference<DialogImpl> DialogImpl::s_dialog;
::osl::Mutex DialogImpl::s_dialogMutex;
::rtl::Reference<DialogImpl> DialogImpl::s_closingDialog;
::osl::Mutex DialogImpl::s_closingMutex;

//______________________________________________________________________________
::rtl::Reference<DialogImpl> DialogImpl::get(
    Reference<XComponentContext> const & xContext,
    Reference<awt::XWindow> const & xParent,
    OUString const & extensionURL,
    OUString const & defaultView )
{
    {
        ::osl::MutexGuard g(s_dialogMutex);
        if (s_dialog.is())
        {
            //Add extensions. In this case the Extension Manager is already open
            //and someone installs an extension by using the system integration. That is, by
            //double clicking and extension.
            //In case the application message loop does not work yet, which is the case when
            //this service is created in the unpkg process, then the queue thread may be blocked
            //during installation until messages are dispatched.
            s_dialog->m_addExtensionQueue->addExtension(extensionURL);
            return s_dialog;
        }
    }

    Window * pParent = DIALOG_NO_PARENT;
    if (xParent.is())
        pParent = VCLUnoHelper::GetWindow(xParent);
    ::rtl::Reference<DialogImpl> that( new DialogImpl( pParent,
        extensionURL,xContext ) );

    // !!! tab-order relates to creation order:
    that->m_ftPackages.reset(
        new FixedText( that.get(), getResId(RID_FT_PACKAGES) ) );

    // selection box: header bar + treelistbox:
    that->m_selectionBox.reset( new SelectionBoxControl( that.get() ) );
    that->m_treelb.reset(
        new TreeListBoxImpl(
            xContext, that->m_selectionBox.get(), that.get() ) );
    that->m_headerBar.reset(
        new HeaderBar( that->m_selectionBox.get() ) );
    that->m_headerBar->SetEndDragHdl(
        LINK( that.get(), DialogImpl, headbar_dragEnd ) );

    that->m_addButton.reset(
        new ThreadedPushButton( that.get(),
                                &DialogImpl::clickAdd, RID_BTN_ADD ) );
    that->m_removeButton.reset(
        new ThreadedPushButton( that.get(),
                                &DialogImpl::clickRemove, RID_BTN_REMOVE ) );
    that->m_enableButton.reset(
        new ThreadedPushButton( that.get(),
                                &DialogImpl::clickEnableDisable,
                                RID_BTN_ENABLE ) );
    that->m_disableButton.reset(
        new ThreadedPushButton( that.get(),
                                &DialogImpl::clickEnableDisable,
                                RID_BTN_DISABLE ) );
    that->m_exportButton.reset(
        new ThreadedPushButton( that.get(),
                                &DialogImpl::clickExport, RID_BTN_EXPORT ) );
    that->m_checkUpdatesButton.reset(
        new SyncPushButton( that.get(), &DialogImpl::clickCheckUpdates,
                            RID_BTN_CHECK_UPDATES ) );

    that->m_optionsButton.reset(
        new ThreadedPushButton( that.get(), &DialogImpl::clickOptions,
                                RID_BTN_OPTIONS ) );

    that->m_optionsButton->Enable(true);

    that->m_getExtensionsButton.reset(
        new svt::FixedHyperlink( that.get(), getResId( RID_BTN_GET_EXTENSIONS ) ) );

    that->m_getExtensionsButton->SetClickHdl( LINK( that.get(), DialogImpl, hyperlink_clicked ) );
    css::uno::Any aValue = that->m_xNameAccessRepositories->getByName(OUSTR("WebsiteLink"));
    String sURL( aValue.get< OUString >() );
    that->m_getExtensionsButton->SetQuickHelpText( sURL );

    that->m_bottomLine.reset( new FixedLine( that.get() ) );
    that->m_closeButton.reset(
        new OKButton( that.get(), getResId(RID_BTN_CLOSE) ) );
    that->m_helpButton.reset(
        new HelpButton( that.get(), getResId(RID_BTN_HELP) ) );
    if (! office_is_running())
        that->m_helpButton->Disable();

    // free local resources (RID < 256):
    that->FreeResource();

    css::uno::Reference<css::deployment::XPackageManager> xUserContext(
        that->m_xPkgMgrFac->getPackageManager( OUSTR("user") ) );
    css::uno::Reference<css::deployment::XPackageManager> xSharedContext(
        that->m_xPkgMgrFac->getPackageManager( OUSTR("shared") ) );
    that->m_packageManagers.realloc(2);
    that->m_packageManagers[0] = xUserContext;
    that->m_packageManagers[1] = xSharedContext;

    that->m_updatability.reset(
        new Updatability(
            that->m_packageManagers, *that->m_checkUpdatesButton ) );

    // sizes, spacing, position:
    that->m_buttonSize = that->LogicToPixel(
        Size( RSC_CD_PUSHBUTTON_WIDTH, RSC_CD_PUSHBUTTON_HEIGHT ),
        MapMode( MAP_APPFONT ) );
    that->m_textSize = that->LogicToPixel(
        Size( 0, RSC_CD_CHECKBOX_HEIGHT ),
        MapMode( MAP_APPFONT ) );
    that->m_relatedSpace = that->LogicToPixel(
        Size( RSC_SP_CTRL_GROUP_X, RSC_SP_CTRL_GROUP_Y ),
        MapMode( MAP_APPFONT ) );
    that->m_unrelatedSpace = that->LogicToPixel(
        Size( RSC_SP_CTRL_X, RSC_SP_CTRL_Y ),
        MapMode( MAP_APPFONT ) );
    that->m_borderLeftTopSpace = that->LogicToPixel(
        Size( RSC_SP_DLG_INNERBORDER_LEFT, RSC_SP_DLG_INNERBORDER_TOP ),
        MapMode( MAP_APPFONT ) );
    that->m_borderRightBottomSpace = that->LogicToPixel(
        Size( RSC_SP_DLG_INNERBORDER_RIGHT, RSC_SP_DLG_INNERBORDER_BOTTOM ),
        MapMode( MAP_APPFONT ) );
    that->m_ftFontHeight = that->m_textSize.Height() /*!!!that->m_ftPackages->GetTextHeight()*/;
    that->m_descriptionYSpace = that->LogicToPixel(
        Size( 0, RSC_SP_CTRL_DESC_Y ), MapMode( MAP_APPFONT ) ).getHeight();

    // minimum size:
    that->SetMinOutputSizePixel(
        Size( // width:
              that->m_borderLeftTopSpace.getWidth() +
              (3 * that->m_buttonSize.getWidth()) +
              (2 * that->m_unrelatedSpace.getWidth()) +
              that->m_borderRightBottomSpace.getWidth(),
              // height:
              that->m_borderLeftTopSpace.getHeight() +
              that->m_ftFontHeight +
              that->m_descriptionYSpace +
              (5 * that->m_buttonSize.getHeight()) +
              (1 * that->m_textSize.Height()) +
              (4 * that->m_relatedSpace.getHeight()) +
              (2 * that->m_unrelatedSpace.getHeight()) +
              that->m_borderRightBottomSpace.getHeight() ) );

    that->Resize();

    // calculate tabs:
    long selWidth = that->m_treelb->GetSizePixel().getWidth();
    long vscrollWidth =
        Application::GetSettings().GetStyleSettings().GetScrollBarSize();

    long statusWidth = that->m_treelb->GetTextWidth(
        that->m_treelb->m_strEnabled );
    statusWidth = ::std::max(
        statusWidth, that->m_treelb->GetTextWidth(
            that->m_treelb->m_strDisabled ) );
    statusWidth = ::std::max(
        statusWidth, that->m_treelb->GetTextWidth(
            that->m_treelb->m_strUnknown ) );
    String strStatus( getResourceString(RID_STR_PACKAGE_STATUS) );
    statusWidth = ::std::max(
        statusWidth, that->m_treelb->GetTextWidth( strStatus ) );
    statusWidth += 1;

    that->m_headerBar->InsertItem(
        ITEM_ID_PACKAGE, getResourceString(RID_STR_PACKAGE),
        selWidth - statusWidth - statusWidth - vscrollWidth );
    that->m_headerBar->InsertItem( ITEM_ID_VERSION, getResourceString(RID_STR_EXTENSION_VERSION), statusWidth);
    that->m_headerBar->InsertItem( ITEM_ID_STATUS, strStatus, statusWidth );

    long tabs[ 4 ];
    tabs[ 0 ] = 3; // two tabs
    tabs[ 1 ] = 0;
    tabs[ 2 ] = selWidth - statusWidth -statusWidth - vscrollWidth;
    tabs[ 3 ] = selWidth - statusWidth - vscrollWidth;
    that->m_treelb->SetTabs( tabs, MAP_PIXEL );
    that->m_treelb->InitHeaderBar( that->m_headerBar.get() );

    //### add top-level nodes: #########################

    that->m_treelb->SetUpdateMode(FALSE);

    that->m_treelb->addNode(
        0 /* no parent */,
        getResourceString(RID_STR_USER_INSTALLATION),
        OUString() /* no factory URL */,
        xUserContext,
        Reference<deployment::XPackage>(),
        Reference<XCommandEnvironment>(),
        false /* no sort in */ );
    that->m_treelb->addNode(
        0 /* no parent */,
        getResourceString(RID_STR_SHARED_INSTALLATION),
        OUString() /* no factory URL */,
        xSharedContext,
        Reference<deployment::XPackage>(),
        Reference<XCommandEnvironment>(),
        false /* no sort in */ );

    if (office_is_running())
    {
        that->m_xDesktop.set(
            that->m_xComponentContext->getServiceManager()
            ->createInstanceWithContext(
                OUSTR("com.sun.star.frame.Desktop"),
                that->m_xComponentContext ),
            UNO_QUERY_THROW );
        that->m_xDesktop->addTerminateListener( that.get() );
        ::ucbhelper::Content ucb_tdocRoot( OUSTR("vnd.sun.star.tdoc:/"), 0 );
        that->m_xTdocRoot.set( ucb_tdocRoot.get() );

//         // scan for open documents:
//         Reference<sdbc::XResultSet> xResultSet(
//             ucb_tdocRoot.createCursor( Sequence<OUString>(),
//                                        ::ucbhelper::INCLUDE_FOLDERS_ONLY ) );
//         while (xResultSet->next()) {
//             that->contentEvent(
//                 ContentEvent( that->m_xTdocRoot,
//                               ContentAction::INSERTED,
//                               Reference<XContentAccess>(
//                                   xResultSet, UNO_QUERY_THROW )->queryContent(),
//                               that->m_xTdocRoot->getIdentifier() ) );
//         }
//         that->m_xTdocRoot->addContentEventListener( that.get() );
    }

    that->m_treelb->SetUpdateMode(TRUE);

    //##################################################
     that->updateButtonStates();
    that->m_selectionBox->Show();
    that->m_headerBar->Show();
    that->m_treelb->Show();
    that->m_bottomLine->Show();

    // default selection:
    that->m_treelb->GrabFocus();
    SvLBoxEntry * defEntry = that->m_treelb->GetEntry(
        defaultView.equalsIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM("shared") ) ? 1 : 0 );
    if (defEntry != 0)
        that->m_treelb->Select( defEntry );

    //Assuming two threads enter this function shortly after each other and
    //both run to this point, then only one dialog will be kept alive by setting
    //it to s_dialog. The other dialog will be destructed after leaving this function.
    //That also means that an installation request (double-click on oxt file), must not
    //be past into the constructor of DialogImpl, because the dialog may be destructed
    //rigth away and will never show.
    //See also DialogImpl::Close, DialogImpl::destroyDialog, DialogImpl::disposing
    {
        ::osl::MutexGuard g(s_dialogMutex);
        if (! s_dialog.is()) {
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            s_dialog = that;
        }
    }
    s_dialog->m_addExtensionQueue->addExtension(extensionURL);
    return s_dialog;
}

//______________________________________________________________________________
void DialogImpl::Resize()
{
    Size totalSize( GetSizePixel() );
    Size selSize( totalSize.getWidth() -
                  m_borderRightBottomSpace.getWidth() -
                  m_buttonSize.getWidth() -
                  m_unrelatedSpace.getWidth() -
                  m_borderLeftTopSpace.getWidth(),

                  totalSize.getHeight() -
                  m_borderLeftTopSpace.getHeight() -
                  m_ftFontHeight -
                  m_descriptionYSpace -
                  (2 * m_unrelatedSpace.getHeight()) -
                  m_buttonSize.getHeight() -
                  m_textSize.getHeight() -
                  m_relatedSpace.getHeight() -
                  m_borderRightBottomSpace.getHeight() );

    long buttonX =
        m_borderLeftTopSpace.getWidth() + selSize.getWidth() +
        m_unrelatedSpace.getWidth();
    long buttonY = m_borderLeftTopSpace.getHeight() +
        m_ftFontHeight + m_descriptionYSpace;

    m_ftPackages->SetPosSizePixel(
        m_borderLeftTopSpace.getWidth(), m_borderLeftTopSpace.getHeight(),
        selSize.getWidth(), m_ftFontHeight );

    m_selectionBox->SetPosSizePixel(
        m_borderLeftTopSpace.getWidth(), buttonY,
        selSize.getWidth(), selSize.getHeight() );
    long selBarHeight = m_headerBar->GetSizePixel().getHeight();
    Size selOutputSize( m_selectionBox->GetOutputSizePixel() );
    m_headerBar->SetPosSizePixel(
        0, 0, selOutputSize.getWidth(), selBarHeight );
    m_treelb->SetPosSizePixel(
        0, selBarHeight, selOutputSize.getWidth(),
        selOutputSize.getHeight() - selBarHeight );

    m_addButton->SetPosSizePixel(
        buttonX,
        buttonY + (0 * (m_buttonSize.getHeight() + m_relatedSpace.getHeight())),
        m_buttonSize.getWidth(), m_buttonSize.getHeight() );
    m_removeButton->SetPosSizePixel(
        buttonX,
        buttonY + (1 * (m_buttonSize.getHeight() + m_relatedSpace.getHeight())),
        m_buttonSize.getWidth(), m_buttonSize.getHeight() );
    m_enableButton->SetPosSizePixel(
        buttonX,
        buttonY + (2 * (m_buttonSize.getHeight() + m_relatedSpace.getHeight())),
        m_buttonSize.getWidth(), m_buttonSize.getHeight() );
    m_disableButton->SetPosSizePixel(
        buttonX,
        buttonY + (3 * (m_buttonSize.getHeight() + m_relatedSpace.getHeight())),
        m_buttonSize.getWidth(), m_buttonSize.getHeight() );
    m_exportButton->SetPosSizePixel(
        buttonX,
        buttonY + (4 * (m_buttonSize.getHeight() + m_relatedSpace.getHeight())),
        m_buttonSize.getWidth(), m_buttonSize.getHeight() );
    m_checkUpdatesButton->SetPosSizePixel(
        buttonX,
        buttonY + (5 * (m_buttonSize.getHeight() + m_relatedSpace.getHeight())),
        m_buttonSize.getWidth(), m_buttonSize.getHeight() );
    m_optionsButton->SetPosSizePixel(
        buttonX,
        buttonY + (6 * (m_buttonSize.getHeight() + m_relatedSpace.getHeight())),
        m_buttonSize.getWidth(), m_buttonSize.getHeight() );
    m_getExtensionsButton->SetPosSizePixel(
        m_borderLeftTopSpace.getWidth(),
        buttonY + selSize.getHeight() + m_relatedSpace.getHeight(),
        selSize.getWidth(), m_textSize.Height() );
    long bottomY =
        totalSize.getHeight() -
        m_borderRightBottomSpace.getHeight() - m_buttonSize.getHeight();
    m_closeButton->SetPosSizePixel(
        buttonX - m_unrelatedSpace.getWidth() - m_buttonSize.getWidth(),
        bottomY,
        m_buttonSize.getWidth(), m_buttonSize.getHeight() );
    m_helpButton->SetPosSizePixel(
        buttonX, bottomY,
        m_buttonSize.getWidth(), m_buttonSize.getHeight() );
    m_bottomLine->SetPosSizePixel(
        0, bottomY - m_unrelatedSpace.getHeight() - 4,
        totalSize.getWidth(), 8 );
}

//______________________________________________________________________________
IMPL_LINK( DialogImpl, headbar_dragEnd, HeaderBar *, pBar )
{
    if (pBar != 0 && pBar->GetCurItemId() == 0)
        return 0;
    OSL_ASSERT( m_headerBar.get() == pBar );

    if (! m_headerBar->IsItemMode())
    {
        const sal_Int32 TAB_WIDTH_MIN = 10;
        sal_Int32 maxWidth =
            m_headerBar->GetSizePixel().getWidth() - TAB_WIDTH_MIN;

        sal_Int32 packageWith = m_headerBar->GetItemSize( ITEM_ID_PACKAGE );
        if (packageWith < TAB_WIDTH_MIN)
            m_headerBar->SetItemSize( ITEM_ID_PACKAGE, TAB_WIDTH_MIN );
        else if (packageWith > maxWidth)
            m_headerBar->SetItemSize( ITEM_ID_PACKAGE, maxWidth );
        if (m_headerBar->GetItemSize( ITEM_ID_STATUS ) < TAB_WIDTH_MIN)
            m_headerBar->SetItemSize( ITEM_ID_STATUS, TAB_WIDTH_MIN );

        sal_Int32 nPos = 0;
        USHORT nTabs = m_headerBar->GetItemCount();
        OSL_ASSERT( m_treelb->TabCount() == nTabs );
        for ( USHORT i = 1; i < nTabs; ++i ) {
            nPos += m_headerBar->GetItemSize( i );
            m_treelb->SetTab( i, nPos, MAP_PIXEL );
        }
    }
    return 1;
}

//______________________________________________________________________________
IMPL_LINK( DialogImpl, hyperlink_clicked, svt::FixedHyperlink*, EMPTYARG )
{
    OUString sURL;
    try
    {   //throws css::container::NoSuchElementException, css::lang::WrappedTargetException
        Any value = m_xNameAccessRepositories->getByName(OUSTR("WebsiteLink"));
        sURL = value.get<OUString> ();
        openWebBrowser(sURL);
     }
    catch (css::uno::Exception& )
    {
        Any exc( ::cppu::getCaughtException() );
        OUString msg(::comphelper::anyToString(exc));
        errbox( msg );
    }
    return 1;
}


//This event is posted after DialogImpl::Show was called.
//It is used to install the extension when running unopkg gui extension
//We use this event to make sure that the extension manager dialog is showing.
IMPL_LINK( DialogImpl, startInstallExtensions, DialogImpl * , EMPTYARG )
{
    //See also updateButtonState which uses m_arExtensions
    m_addExtensionQueue->addExtension(m_extensionURL);
    m_extensionURL = OUString();
    return 0;
}

void DialogImpl::openWebBrowser(OUString const & sURL) const
{
    try
    {
        Reference< css::system::XSystemShellExecute > xSystemShellExecute(
            m_xComponentContext->getServiceManager()->createInstanceWithContext(
                OUString::createFromAscii( "com.sun.star.system.SystemShellExecute" ),
                m_xComponentContext), UNO_QUERY_THROW);
        //throws css::lang::IllegalArgumentException, css::system::SystemShellExecuteException
        xSystemShellExecute->execute(
            sURL, OUString(), css::system::SystemShellExecuteFlags::DEFAULTS);
    }
    catch (css::uno::Exception& )
    {
        Any exc( ::cppu::getCaughtException() );
        OUString msg(::comphelper::anyToString(exc));
        errbox( msg );
    }
}

::std::vector<dp_gui::UpdateData> DialogImpl::excludeWebsiteDownloads(
        ::std::vector<dp_gui::UpdateData> const & data)
{
    ::std::vector<dp_gui::UpdateData> ret;
    typedef std::vector< dp_gui::UpdateData >::const_iterator cit;
    for (cit i = data.begin(); i < data.end(); i++)
    {
        if (i->sWebsiteURL.getLength() == 0)
            ret.push_back(*i);
    }
    return ret;
}
//______________________________________________________________________________
void DialogImpl::updateButtonStates(
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    m_updatability->start();

    bool allowModification = true;
    bool bEnable = true;
    bool bDisable = true;
    bool bRemove = true;
    bool bExport = true;
    bool bOptions = true;

    sal_Int32 nSelectedPackages = 0;
    Reference<css::deployment::XPackage> xPackage;
    Reference<css::deployment::XPackageManager> xPackageManager;
    for ( SvLBoxEntry * entry = m_treelb->FirstSelected();
          entry != 0; entry = m_treelb->NextSelected(entry) )
    {
        allowModification = allowModification &&
            ! m_treelb->getPackageManager(entry)->isReadOnly();

        xPackage = m_treelb->getPackage(entry);

        if (xPackage.is())
        {
            ++nSelectedPackages;
            if (m_treelb->isFirstLevelChild( entry ))
            {
                //get the package manager for this package which we need to determine if
                //options button is to be shown.
                xPackageManager = m_treelb->getPackageManager(entry);
                switch (getPackageState( xPackage, xCmdEnv ))
                {
                case REGISTERED:
                    bEnable = false;
                    break;
                case NOT_REGISTERED:
                    bDisable = false;
                    break;
                case AMBIGUOUS:
                    break;
                case NOT_AVAILABLE:
                    bEnable = false;
                    bDisable = false;
                    break;
                }
            }
            else {
                // export still possible:
                bEnable = bDisable = bRemove = bOptions = false;
            }
        }
        else { // selected non-package entry:
            bExport = bEnable = bDisable = bRemove = bOptions = false;
        }
    }
//     bEnable &= (allowModification && nSelectedPackages > 0) && !m_bAddingExtensions;
//     bDisable &= (allowModification && nSelectedPackages > 0) && !m_bAddingExtensions;
//     bRemove &= (allowModification && nSelectedPackages > 0) && !m_bAddingExtensions;
    bEnable &= allowModification && nSelectedPackages > 0;
    bDisable &= allowModification && nSelectedPackages > 0;
    bRemove &= allowModification && nSelectedPackages > 0;

    bExport &= (nSelectedPackages > 0);

    if (bOptions)
    {
        //We do not support multiple selection for the Options button
        if (nSelectedPackages == 1 && xPackage->isBundle())
        {
            //check if this package is shared. Then if the same package exist as
            //user then we will not enable the button
            css::beans::Optional<OUString> aId = xPackage->getIdentifier();
            //a bundle must always have an id
            OSL_ASSERT(aId.IsPresent);
            if (xPackageManager->getContext().equals(OUSTR("shared")))
            {
                //get the "user" xpackage manager;
                Reference<css::deployment::XPackageManager> xUserPM;
                for (sal_Int32 i = 0; i < m_packageManagers.getLength(); i++)
                {
                    Reference<css::deployment::XPackageManager> const &  xPM = m_packageManagers[i];
                    if (xPM->getContext().equals(OUSTR("user")))
                    {
                        xUserPM = xPM;
                        break;
                    }
                }
                try {
                    //getDeployedPackage throws an IllegalArgumentException if the package
                    //does not exist
                    xUserPM->getDeployedPackage(
                            aId.Value, OUSTR(""),
                            Reference<css::ucb::XCommandEnvironment>()).is();
                    bOptions = false;
                } catch (css::uno::Exception & ) {
                }
            }
            if (bOptions &&  ! supportsOptions(aId.Value))
                bOptions = false;
        }
        else
        {
            bOptions = false;
        }
    }

    {
        const ::vos::OGuard guard( Application::GetSolarMutex() );
        m_disableButton->Enable( bDisable );
        m_enableButton->Enable( bEnable );
        m_exportButton->Enable( bExport );
        m_optionsButton->Enable( bOptions);
        SvLBoxEntry * currEntry = m_treelb->getCurrentSingleSelectedEntry();

        m_addButton->Enable(
//            !m_bAddingExtensions &&
            allowModification &&
            (currEntry != 0 &&
            m_treelb->GetParent( currEntry ) == 0 /* top-level */) &&
            (nSelectedPackages == 0) );
        m_removeButton->Enable( bRemove );
    }
}

// The function investigates if the extension supports options.
bool DialogImpl::supportsOptions( ::rtl::OUString const & sExtensionId)
{
    bool bOptions = false;
    //iterate over all available nodes
    Sequence<OUString> seqNames = m_xNameAccessNodes->getElementNames();

    for (int i = 0; i < seqNames.getLength(); i++)
    {
        Any anyNode = m_xNameAccessNodes->getByName(seqNames[i]);
        //If we have a node then then it must contain the set of leaves. This is part of OptionsDialog.xcs
        Reference<XInterface> xIntNode = anyNode.get<Reference<XInterface> >();
        Reference<css::container::XNameAccess> xNode(xIntNode, UNO_QUERY_THROW);

        Any anyLeaves = xNode->getByName(OUSTR("Leaves"));
        Reference<XInterface> xIntLeaves = anyLeaves.get<Reference<XInterface> >();
        Reference<css::container::XNameAccess> xLeaves(xIntLeaves, UNO_QUERY_THROW);

        //iterate over all available leaves
        Sequence<OUString> seqLeafNames = xLeaves->getElementNames();
        for (int j = 0; j < seqLeafNames.getLength(); j++)
        {
            Any anyLeaf = xLeaves->getByName(seqLeafNames[j]);
            Reference<XInterface> xIntLeaf = anyLeaf.get<Reference<XInterface> >();
            Reference<css::beans::XPropertySet> xLeaf(xIntLeaf, UNO_QUERY_THROW);
            //investigate the Id property if it matches the extension identifier which
            //has been passed in.
            Any anyValue = xLeaf->getPropertyValue(OUSTR("Id"));

            OUString sId = anyValue.get<OUString>();
            if (sId == sExtensionId)
            {
                bOptions = true;
                break;
            }
        }
        if (bOptions)
            break;
    }
    return bOptions;
}

//______________________________________________________________________________

::std::vector<
    ::std::pair<
        Reference< deployment::XPackage>,
        Reference< deployment::XPackageManager> > >
        DialogImpl::TreeListBoxImpl::getSelectedPackages(bool onlyFirstLevel) const
{
    ::std::vector< ::std::pair< Reference<deployment::XPackage>,
        Reference<deployment::XPackageManager> > > ret;

    for ( SvLBoxEntry * entry = FirstSelected();
          entry != 0; entry = NextSelected(entry) )
    {
        if (onlyFirstLevel && ! isFirstLevelChild( entry ))
            continue;
        Reference<deployment::XPackage> xPackage( getPackage(entry) );
        Reference<deployment::XPackageManager> xPackageManager(getPackageManager(entry));
        OSL_ASSERT(xPackageManager.is());
        if (xPackage.is())
            ret.push_back( ::std::make_pair(xPackage, xPackageManager));
    }
    return ret;
}

namespace {
struct StrAllFiles : public rtl::StaticWithInit<const OUString, StrAllFiles> {
    const OUString operator () () {
        const ::vos::OGuard guard( Application::GetSolarMutex() );
        ::std::auto_ptr<ResMgr> const resmgr(
            ResMgr::CreateResMgr( "fps_office" ) );
        OSL_ASSERT( resmgr.get() != 0 );
        String ret( ResId( STR_FILTERNAME_ALL, *resmgr.get() ) );
        return ret;
    }
};
} // anon namespace

//______________________________________________________________________________
Sequence<OUString> DialogImpl::solarthread_raiseAddPicker(
    Reference<deployment::XPackageManager> const & xPackageManager )
{
    const Any mode( static_cast<sal_Int16>(
                        ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE ) );
    const Reference<ui::dialogs::XFilePicker> xFilePicker(
        m_xComponentContext->getServiceManager()
        ->createInstanceWithArgumentsAndContext(
            OUSTR("com.sun.star.ui.dialogs.FilePicker"),
            Sequence<Any>(&mode, 1), m_xComponentContext ), UNO_QUERY_THROW );
    xFilePicker->setTitle( m_strAddPackages );
    xFilePicker->setMultiSelectionMode(true);

    // collect and set filter list:
    typedef /* sorted */ ::std::map<OUString, OUString> t_string2string;
    t_string2string title2filter;
    const Sequence< Reference<deployment::XPackageTypeInfo> > packageTypes(
        xPackageManager->getSupportedPackageTypes() );
    for ( sal_Int32 pos = 0; pos < packageTypes.getLength(); ++pos ) {
        Reference<deployment::XPackageTypeInfo> const & xPackageType =
            packageTypes[ pos ];
        const OUString filter( xPackageType->getFileFilter() );
        if (filter.getLength() > 0)
        {
            const OUString title( xPackageType->getShortDescription() );
            const ::std::pair<t_string2string::iterator, bool> insertion(
                title2filter.insert( t_string2string::value_type(
                                         title, filter ) ) );
            if (! insertion.second) { // already existing, append extensions:
                ::rtl::OUStringBuffer buf;
                buf.append( insertion.first->second );
                buf.append( static_cast<sal_Unicode>(';') );
                buf.append( filter );
                insertion.first->second = buf.makeStringAndClear();
            }
        }
    }

    const Reference<ui::dialogs::XFilterManager> xFilterManager(
        xFilePicker, UNO_QUERY_THROW );
    // All files at top:
    xFilterManager->appendFilter( StrAllFiles::get(), OUSTR("*.*") );
    // then supported ones:
    t_string2string::const_iterator iPos( title2filter.begin() );
    const t_string2string::const_iterator iEnd( title2filter.end() );
    for ( ; iPos != iEnd; ++iPos ) {
        try {
            xFilterManager->appendFilter( iPos->first, iPos->second );
        }
        catch (lang::IllegalArgumentException & exc) {
            OSL_ENSURE( 0, ::rtl::OUStringToOString(
                            exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
            (void) exc;
        }
    }
    xFilterManager->setCurrentFilter( StrAllFiles::get() );

    if (xFilePicker->execute() != ui::dialogs::ExecutableDialogResults::OK)
        return Sequence<OUString>(); // cancelled

    Sequence<OUString> files( xFilePicker->getFiles() );
    OSL_ASSERT( files.getLength() > 0 );
    return files;
}

//______________________________________________________________________________
void DialogImpl::clickAdd( USHORT )
{
    //Prevent adding of new extension (m_addExtensionQueue) which are caused by
    //calls to "unopkg gui ext", for example, double-clicking and extension.
    ::osl::MutexGuard actionGuard(ActionMutex::get());

    //The top level nodes of the tree contain the respective XPackageManager
    OSL_ASSERT( m_treelb->getSelectedPackages(false).size() == 0 );
    const Reference<deployment::XPackageManager> xPackageManager(
        m_treelb->getPackageManager(m_treelb->getCurrentSingleSelectedEntry() ) );
    OSL_ASSERT(xPackageManager.is() );

    if (! continueActionForSharedExtension(xPackageManager, ADD_ACTION))
        return;

    const Sequence<OUString> files(
        vcl::solarthread::syncExecute(
            boost::bind( &DialogImpl::solarthread_raiseAddPicker, this,
                         xPackageManager ) ) );
    if (files.getLength() == 0)
        return;

    ::rtl::Reference<ProgressCommandEnv> currentCmdEnv(
        new ProgressCommandEnv( m_xComponentContext, this, m_strAddingPackages ) );
    currentCmdEnv->showProgress( files.getLength() );
    Reference<task::XAbortChannel> xAbortChannel(
        xPackageManager->createAbortChannel() );

    for ( sal_Int32 pos = files.getLength() > 1 ? 1 : 0;
          !currentCmdEnv->isAborted() && pos < files.getLength(); ++pos )
    {
        OUString file;
        if (files.getLength() > 1)
            file = makeURL( files[ 0 ], files[ pos ] );
        else
            file = files[ pos ];
        currentCmdEnv->progressSection(
            ::ucbhelper::Content( file, currentCmdEnv.get() ).getPropertyValue(
                OUSTR("Title") ).get<OUString>(), xAbortChannel );
        try {
            Reference<deployment::XPackage> xPackage(
                xPackageManager->addPackage(
                    file, OUString() /* detect media-type */,
                    xAbortChannel, currentCmdEnv.get() ) );
            OSL_ASSERT( xPackage.is() );
        }
        catch (css::ucb::CommandFailedException & )
        {
            //For exampl, we want to add many extensions, and one of them
            //is already installed. Then we'll get a dialog asking if we want to overwrite. If we then press
            //cancel the exception is thrown. This should not prevent us from installing all other
            //extensions.
        }
        catch (CommandAbortedException &) {
            //User clicked the cancel button
            break;
        }
    }
}

//______________________________________________________________________________
void DialogImpl::clickRemove( USHORT )
{
    //Prevent adding of new extension (m_addExtensionQueue) which are caused by
    //calls to "unopkg gui ext", for example, double-clicking and extension.
    ::osl::MutexGuard actionGuard(ActionMutex::get());

    const ::std::vector< ::std::pair< Reference<deployment::XPackage>,
        Reference<deployment::XPackageManager> > > selection(
            m_treelb->getSelectedPackages(true) );
    OSL_ASSERT( selection.size() > 0 );

    //Check if we want to remove a shared extension and notify user if necessary
    for (TreeListBoxImpl::CI_PAIR_PACKAGE i = selection.begin();
         i != selection.end(); i++)
    {
        if (! continueActionForSharedExtension(i->second, REMOVE_ACTION))
        {
            return;
        }
        else
        {
            //We only show the the messagebox once
            if (i->second->getContext().equals(OUSTR("shared")))
                break;
        }
    }
    ::rtl::Reference<ProgressCommandEnv> currentCmdEnv(
        new ProgressCommandEnv( m_xComponentContext, this, m_strRemovingPackages) );
    currentCmdEnv->showProgress( selection.size() );
    for ( TreeListBoxImpl::CI_PAIR_PACKAGE pos = selection.begin();
          !currentCmdEnv->isAborted() && pos != selection.end(); ++pos )
    {
        Reference<task::XAbortChannel> xAbortChannel(
            pos->second->createAbortChannel() );
        OUString id( dp_misc::getIdentifier( pos->first ) );
        currentCmdEnv->progressSection( id, xAbortChannel );
        try {
            pos->second->removePackage(
                id, pos->first->getName(), xAbortChannel,
                currentCmdEnv.get() );
        }
        catch (CommandAbortedException &) {
            break;
        }
    }

    // Check, if there are still updates to be notified via menu bar icon
    css::uno::Sequence< css::uno::Sequence< rtl::OUString > > aItemList;
    UpdateDialog::createNotifyJob( false, aItemList );
}

//______________________________________________________________________________
void DialogImpl::clickEnableDisable( USHORT id )
{
    //Prevent adding of new extension (m_addExtensionQueue) which are caused by
    //calls to "unopkg gui ext", for example, double-clicking and extension.
    ::osl::MutexGuard actionGuard(ActionMutex::get());

    const ::std::vector< ::std::pair< Reference<deployment::XPackage>,
        Reference<deployment::XPackageManager> > > selection(
            m_treelb->getSelectedPackages(true) );
    OSL_ASSERT( selection.size() > 0 );

    //Check if we want to remove a shared extension and notify user if necessary
    for (TreeListBoxImpl::CI_PAIR_PACKAGE i = selection.begin();
         i != selection.end(); i++)
    {
        if (! continueActionForSharedExtension(i->second,
            id == RID_BTN_ENABLE ? ENABLE_ACTION : DISABLE_ACTION))
        {
            return;
        }
        else
        {
            //We only show the the messagebox once
            if (i->second->getContext().equals(OUSTR("shared")))
                break;
        }
    }

    ::rtl::Reference<ProgressCommandEnv> currentCmdEnv(
        new ProgressCommandEnv( m_xComponentContext, this, id == RID_BTN_ENABLE
                                ? m_strEnablingPackages
                                : m_strDisablingPackages ) );
    currentCmdEnv->showProgress( selection.size() );
    for ( TreeListBoxImpl::CI_PAIR_PACKAGE pos = selection.begin();
          !currentCmdEnv->isAborted() && pos != selection.end(); ++pos )
    {
        Reference<deployment::XPackage> const & xPackage = pos->first;
        Reference<task::XAbortChannel> xAbortChannel(
            xPackage->createAbortChannel() );
        currentCmdEnv->progressSection( xPackage->getDisplayName(),
                                        xAbortChannel );
        try {
            if (id == RID_BTN_ENABLE)
                xPackage->registerPackage( xAbortChannel,
                                           currentCmdEnv.get() );
            else
                xPackage->revokePackage( xAbortChannel,
                                         currentCmdEnv.get() );
        }
        catch (CommandAbortedException &) {
            break;
        }
    }
}

bool DialogImpl::solarthread_raiseExportPickers(
    Sequence< Reference<deployment::XPackage> > const & selection,
    OUString & rDestFolder, OUString & rNewTitle, sal_Int32 & rNameClashAction )
{
    rNameClashAction = NameClash::ASK;

    if (selection.getLength() > 1)
    {
        // raise folder picker:
        Reference<ui::dialogs::XFolderPicker> xFolderPicker(
            m_xComponentContext->getServiceManager()
            ->createInstanceWithContext(
                OUSTR("com.sun.star.ui.dialogs.FolderPicker"),
                m_xComponentContext ), UNO_QUERY_THROW );
        xFolderPicker->setTitle( m_strExportPackages );
        if (xFolderPicker->execute() !=ui::dialogs::ExecutableDialogResults::OK)
            return false; // cancelled
        rDestFolder = xFolderPicker->getDirectory();
    }
    else // single item selected
    {
        const Any mode( static_cast<sal_Int16>(
                      ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION ) );
        const Reference<ui::dialogs::XFilePicker> xFilePicker(
            m_xComponentContext->getServiceManager()
            ->createInstanceWithArgumentsAndContext(
                OUSTR("com.sun.star.ui.dialogs.FilePicker"),
                Sequence<Any>(&mode, 1), m_xComponentContext ),
            UNO_QUERY_THROW );
        xFilePicker->setTitle( m_strExportPackage );
        xFilePicker->setMultiSelectionMode(false);

        const Reference<ui::dialogs::XFilePickerControlAccess> xFPControlAccess(
            xFilePicker, UNO_QUERY_THROW );
        xFPControlAccess->setValue(
            ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION,
            0, Any(true));

        ::rtl::Reference<ProgressCommandEnv> currentCmdEnv(
            new ProgressCommandEnv( m_xComponentContext, this, m_strExportPackage) );
        OSL_ASSERT( selection.getLength() == 1 );
        Reference<deployment::XPackage> const & xPackage = selection[ 0 ];

        // set filter:
        const Reference<deployment::XPackageTypeInfo> xPackageType(
            xPackage->getPackageType() );
        bool isLegacyBundle = false;
        bool isBundle = false;
        OUString bundleext(RTL_CONSTASCII_USTRINGPARAM(".oxt"));
        OUString bundlefilterext(RTL_CONSTASCII_USTRINGPARAM("*.oxt"));
        if (xPackageType.is()) {
            const Reference<ui::dialogs::XFilterManager> xFilterManager(
                xFilePicker, UNO_QUERY_THROW );
            try {
                // All files at top:
                xFilterManager->appendFilter(
                    StrAllFiles::get(), OUSTR("*.*") );

                // then package filter:
                if (xPackageType->getMediaType().equals(
                        OUSTR("application/vnd.sun.star.legacy-package-bundle"))) {
                    isLegacyBundle = true;
                    xFilterManager->appendFilter(
                        xPackageType->getShortDescription(),
                        bundlefilterext);
                } else {
                    if (xPackageType->getMediaType().equals(
                            OUSTR("application/vnd.sun.star.package-bundle")))
                        isBundle = true;

                    xFilterManager->appendFilter(
                        xPackageType->getShortDescription(),
                        xPackageType->getFileFilter() );
                }

                xFilterManager->setCurrentFilter(
                    xPackageType->getShortDescription() );
            }
            catch (lang::IllegalArgumentException & exc) {
                OSL_ENSURE( 0, ::rtl::OUStringToOString(
                                exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
                (void) exc;
            }
        }

        // set default selection:
        ::ucbhelper::Content sourceContent(
            xPackage->getURL(), currentCmdEnv.get() );
        OUString defaultname(sourceContent.getPropertyValue(
                                 OUSTR("Title") ).get<OUString>());
        OUString legacyext(RTL_CONSTASCII_USTRINGPARAM(".zip"));
        OUString legacyextlong(RTL_CONSTASCII_USTRINGPARAM(".uno.zip"));
        OUString bundleextold(RTL_CONSTASCII_USTRINGPARAM(".uno.pkg"));
        if (isLegacyBundle && defaultname.lastIndexOf(legacyext)>0) {
            if (defaultname.lastIndexOf(legacyextlong)>0) {
                defaultname = defaultname.replaceAt(
                    defaultname.lastIndexOf(legacyextlong), 8, bundleext);
            } else {
                defaultname = defaultname.replaceAt(
                    defaultname.lastIndexOf(legacyext), 4, bundleext);
            }
        } else if (isBundle && defaultname.lastIndexOf(bundleextold)>0) {
                defaultname = defaultname.replaceAt(
                    defaultname.lastIndexOf(bundleextold), 8, bundleext);
        }
        xFilePicker->setDefaultName(defaultname);

        if (xFilePicker->execute() != ui::dialogs::ExecutableDialogResults::OK)
            return false; // cancelled

        Sequence<OUString> files( xFilePicker->getFiles() );
        OSL_ASSERT( files.getLength() == 1 );
        OUString const & url = files[ 0 ];
        ::ucbhelper::Content childContent( url, currentCmdEnv.get() );
        Reference<container::XChild> xChild( childContent.get(),
                                             UNO_QUERY_THROW );
        ::ucbhelper::Content destFolderContent(
            Reference<XContent>( xChild->getParent(), UNO_QUERY_THROW ),
            currentCmdEnv.get() );
        rDestFolder = destFolderContent.getURL();
        rNewTitle = rtl::Uri::decode( url.copy( url.lastIndexOf( '/' ) + 1 ),
                                      rtl_UriDecodeWithCharset,
                                      RTL_TEXTENCODING_UTF8 );
        // overwrite, because FilePicker has already asked:
        rNameClashAction = NameClash::OVERWRITE;
    }
    return true;
}

//______________________________________________________________________________
void DialogImpl::clickExport( USHORT )
{
    //Prevent adding of new extension (m_addExtensionQueue) which are caused by
    //calls to "unopkg gui ext", for example, double-clicking and extension.
    ::osl::MutexGuard actionGuard(ActionMutex::get());

    const ::std::vector< ::std::pair< Reference<deployment::XPackage>,
        Reference<deployment::XPackageManager> > > selection(
            m_treelb->getSelectedPackages(false) );

    OSL_ASSERT( selection.size() > 0 );

    //create the Sequence<Reference<XPackage> > out of selection for use in
    //syncExecute
    Sequence<Reference<deployment::XPackage> > seqPackages(
        selection.size());
    sal_Int32 j = 0;
    for (TreeListBoxImpl::CI_PAIR_PACKAGE i = selection.begin();
         i != selection.end(); i++, j++)
    {
        seqPackages[j] = i->first;
    }

    OUString destFolder;
    OUString newTitle;
    sal_Int32 nameClashAction = NameClash::ASK;
    using namespace vcl::solarthread;
    if (! syncExecute( boost::bind(
                           &DialogImpl::solarthread_raiseExportPickers, this,
                           seqPackages,
                           inout_by_ref(destFolder),
                           inout_by_ref(newTitle),
                           inout_by_ref(nameClashAction) ) ))
        return;

    ::rtl::Reference<ProgressCommandEnv> currentCmdEnv(
        new ProgressCommandEnv( m_xComponentContext, this, m_strExportingPackages ) );
    currentCmdEnv->showProgress( selection.size() );
    for ( TreeListBoxImpl::CI_PAIR_PACKAGE pos = selection.begin();
          !currentCmdEnv->isAborted() && pos != selection.end(); ++pos )
    {
        Reference<deployment::XPackage> const & xPackage = pos->first;
        currentCmdEnv->progressSection( xPackage->getDisplayName() );
        OSL_ASSERT( destFolder.getLength() > 0 );
        xPackage->exportTo( destFolder, newTitle, nameClashAction,
                            currentCmdEnv.get() );
    }
}

//______________________________________________________________________________
void DialogImpl::clickCheckUpdates( USHORT )
{
    // see checkUpdates.
    checkUpdates(false);
}

//______________________________________________________________________________
void DialogImpl::clickOptions( USHORT )
{
    //Prevent adding of new extension (m_addExtensionQueue) which are caused by
    //calls to "unopkg gui ext", for example, double-clicking and extension.
    ::osl::MutexGuard actionGuard(ActionMutex::get());

    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    if ( pFact )
    {
        rtl::OUString sExtensionId;
        Reference<deployment::XPackage> xPackage(
            m_treelb->getPackage( m_treelb->FirstSelected() ) );
        if ( xPackage.is() )
            sExtensionId = xPackage->getIdentifier().Value;

        const ::vos::OGuard guard( Application::GetSolarMutex() );
        VclAbstractDialog* pDlg = pFact->CreateOptionsDialog( this, sExtensionId, rtl::OUString() );
        pDlg->Execute();
        delete pDlg;
    }
}

//______________________________________________________________________________
void DialogImpl::errbox( OUString const & msg, Window const *  parent) const
{
    const ::vos::OGuard guard( Application::GetSolarMutex() );
    Window const * thisParent = parent ? parent : this;
    const ::std::auto_ptr<ErrorBox> box( new ErrorBox( const_cast<Window*>(thisParent), WB_OK, msg ) );
    box->SetText( GetText() );
    box->Execute();
}

//______________________________________________________________________________
void DialogImpl::checkUpdates( bool selected, bool showUpdateOnly, bool parentVisible )
{
    //Prevent adding of new extension (m_addExtensionQueue) which are caused by
    //calls to "unopkg gui ext", for example, double-clicking and extension.
    ::osl::MutexGuard actionGuard(ActionMutex::get());
    ::vos::OClearableGuard aGuard( Application::GetSolarMutex() );
    //Todo: m_pUpdateDialog can never be != NULL. It is not accessed outside of this
    //function and it will be deleted at the end of this function.
    if ( m_pUpdateDialog != NULL )
    {
        m_pUpdateDialog->ToTop();
    }
    else
    {
        std::vector<UpdateData> data;
        Window * pParent = this;

        if ( showUpdateOnly && !parentVisible )
            pParent = GetParent();

        m_pUpdateDialog = new UpdateDialog( m_xComponentContext, pParent, this,
                                            ( selected ? new SelectedPackageIterator(*m_treelb.get())
                                                       : rtl::Reference<SelectedPackageIterator>()),
                                            ( selected ? Sequence<Reference<deployment::XPackageManager> >()
                                                       : m_packageManagers ),
                                            &data );
        if ( ( m_pUpdateDialog->Execute() == RET_OK ) && !data.empty() )
        {
            m_pUpdateDialog->notifyMenubar( true, false ); // prepare the checking, if there updates to be notified via menu bar icon
           //If there is at least one directly downloadable dialog then we
            //open the install dialog.
            int countWebsiteDownload = 0;
            typedef std::vector< dp_gui::UpdateData >::const_iterator cit;
            for (cit i = data.begin(); i < data.end(); i++)
            {
                if (i->sWebsiteURL.getLength() > 0)
                    countWebsiteDownload ++;
            }

            short nDialogResult = RET_OK;
            if (data.size() - countWebsiteDownload > 0)
            {
                ::std::vector<dp_gui::UpdateData> dataDownload(excludeWebsiteDownloads(data));
                nDialogResult = UpdateInstallDialog( pParent, dataDownload, m_xComponentContext ).Execute();
                m_pUpdateDialog->notifyMenubar( false, true ); // Check, if there are still pending updates to be notified via menu bar icon
            }
            else
                m_pUpdateDialog->notifyMenubar( false, false ); // Check, if there are pending updates to be notified via menu bar icon
            //Now start the webbrowser and navigate to the websites where we get the updates
            if (RET_OK == nDialogResult)
             {
                for (cit i = data.begin(); i < data.end(); i++)
                {
                    if (i->sWebsiteURL.getLength() > 0)
                    {
                        openWebBrowser(i->sWebsiteURL);
                    }
                }
            }
        }
        delete m_pUpdateDialog;
        m_pUpdateDialog = NULL;
    }
}

bool DialogImpl::continueUpdateForSharedExtension(
    Window * pUpdateDialog,
    Reference<css::deployment::XPackageManager> const & xPMgr)
{
    return continueActionForSharedExtension(xPMgr, UPDATE_ACTION, pUpdateDialog);
}


bool DialogImpl::continueActionForSharedExtension(
    Reference<css::deployment::XPackageManager> const & xPMgr, ACTION action,
    Window * pUpdateDialog)
{
    /** The following flags are used to indicate that a warning has already been displayed
        for a particular action.
        We show a warning if a user is going to modify a shared extension. This warning
        shall only appear once during a session.
        Because this function is called from the various button handlers only there is
        no risk of concurrent access.
        ToDo!!! We should move these flags to DialogImpl and make it a real one instance
        service. Currently it is destroyed when the window is being closed.
    */

      static bool bWarningAddSharedDisplayed = false;
      static bool bWarningRemoveSharedDisplayed = false;
      static bool bWarningEnableSharedDisplayed = false;
      static bool bWarningDisableSharedDisplayed = false;
      static bool bWarningUpdateSharedDisplayed = false;

    //If the package manager is readonly then the user cannot modify anything anyway.
    //Then the messagebox need not be displayed. Also the add, remove, disable buttons
    //should not be enabled.
    sal_uInt32 id = 0;
    //The flag is used to determine if the warning for this action was already displayed.
    //We only display the warning once for each action.
    bool bWasAlreadyDisplayed = false;
    switch (action)
    {
    case ADD_ACTION:
        OSL_ASSERT(!pUpdateDialog);
        id = RID_WARNINGBOX_ADD_SHARED_EXTENSION;
        bWasAlreadyDisplayed = bWarningAddSharedDisplayed;
        bWarningAddSharedDisplayed = true;
        break;
    case REMOVE_ACTION:
        OSL_ASSERT(!pUpdateDialog);
        id = RID_WARNINGBOX_REMOVE_SHARED_EXTENSION;
        bWasAlreadyDisplayed = bWarningRemoveSharedDisplayed;
        bWarningRemoveSharedDisplayed = true;
        break;
    case ENABLE_ACTION:
        OSL_ASSERT(!pUpdateDialog);
        id = RID_WARNINGBOX_ENABLE_SHARED_EXTENSION;
        bWasAlreadyDisplayed = bWarningEnableSharedDisplayed;
        bWarningEnableSharedDisplayed = true;
        break;
    case DISABLE_ACTION:
        OSL_ASSERT(!pUpdateDialog);
        id = RID_WARNINGBOX_DISABLE_SHARED_EXTENSION;
        bWasAlreadyDisplayed = bWarningDisableSharedDisplayed;
        bWarningDisableSharedDisplayed = true;
        break;
    case UPDATE_ACTION:
        OSL_ASSERT(pUpdateDialog);
        id = RID_WARNINGBOX_UPDATE_SHARED_EXTENSION;
        bWasAlreadyDisplayed = bWarningUpdateSharedDisplayed;
        bWarningUpdateSharedDisplayed = true;
        break;

    default:
        OSL_ASSERT(0);
    }
    if (! bWasAlreadyDisplayed
        &&xPMgr->getContext().equals(OUSTR("shared"))
        && ! xPMgr->isReadOnly())
    {
        vos::OGuard guard(Application::GetSolarMutex());
        WarningBox box(pUpdateDialog ? pUpdateDialog : this,
            ResId(id, *DeploymentGuiResMgr::get()));
        String msgText = box.GetMessText();
        msgText.SearchAndReplaceAllAscii( "%PRODUCTNAME", BrandName::get() );
        if (RET_OK == box.Execute())
            return true;
        else
            return false;
    }
    return true;
}

//______________________________________________________________________________
void DialogImpl::SyncPushButton::Click()
{
    try {
        (m_dialog->*m_clickCallback)( m_id );
    }
    catch (CommandFailedException &) {
        // already handled by interaction handler
    }
    catch (Exception &) {
        Any exc( ::cppu::getCaughtException() );
        OUString msg;
        deployment::DeploymentException dpExc;
        if ((exc >>= dpExc) &&
            dpExc.Cause.getValueTypeClass() == TypeClass_EXCEPTION) {
            // notify error cause only:
            msg = reinterpret_cast<Exception const *>(
                dpExc.Cause.getValue() )->Message;
        }
        if (msg.getLength() == 0) // fallback for debugging purposes
            msg = ::comphelper::anyToString(exc);
        m_dialog->errbox( msg );
    }
}


//------------------------------------------------------------------------------
namespace {
extern "C" {
void SAL_CALL ThreadedPushButton_callback( void * p )
{
    DialogImpl::ThreadedPushButton * that =
        static_cast<DialogImpl::ThreadedPushButton *>(p);
    that->DialogImpl::SyncPushButton::Click();
}
}
}


//______________________________________________________________________________
void DialogImpl::ThreadedPushButton::Click()
{
    if (m_thread != 0) {
        const ULONG nLockCount = Application::ReleaseSolarMutex();
        //todo deadlock. When the add button is clicked then the file picker is started which
        //runs in the main thread. When I click again on Add, then this function is called again
        //and also from the main thread. That is I join the main thread on itself here. However
        //I could only achieve this on Windows when I debugged the app. Since the file picker is a modal dialog
        //one cannot click again on Add as long as the file picker is open.
        osl_joinWithThread( m_thread );
        if (nLockCount > 0)
            Application::AcquireSolarMutex( nLockCount );
        osl_destroyThread( m_thread );
    }
    m_thread = osl_createSuspendedThread( ThreadedPushButton_callback, this );
    OSL_ASSERT( m_thread != 0 );
    osl_resumeThread( m_thread );
}

//______________________________________________________________________________
DialogImpl::ThreadedPushButton::~ThreadedPushButton()
{
    if (m_thread != 0) {
        const ULONG nLockCount = Application::ReleaseSolarMutex();
        osl_joinWithThread( m_thread );
        if (nLockCount > 0)
            Application::AcquireSolarMutex( nLockCount );
        osl_destroyThread( m_thread );
    }
}

//==============================================================================
ResId DialogImpl::getResId( USHORT id )
{
    const ::vos::OGuard guard( Application::GetSolarMutex() );
    return ResId( id, *DeploymentGuiResMgr::get() );
}

//==============================================================================
String DialogImpl::getResourceString( USHORT id )
{
    // init with non-acquired solar mutex:
    BrandName::get();
    const ::vos::OGuard guard( Application::GetSolarMutex() );
    String ret( ResId( id, *DeploymentGuiResMgr::get() ) );
    if (ret.SearchAscii( "%PRODUCTNAME" ) != STRING_NOTFOUND) {
        ret.SearchAndReplaceAllAscii( "%PRODUCTNAME", BrandName::get() );
    }
    return ret;
}

SelectedPackageIterator::SelectedPackageIterator(
    DialogImpl::TreeListBoxImpl & list):
    m_list(list),
    m_entry(NULL)
{}

SelectedPackageIterator::~SelectedPackageIterator() {}

void SelectedPackageIterator::next(
    Reference<deployment::XPackage> * package,
    Reference<deployment::XPackageManager> * packageManager)
{
    OSL_ASSERT(package != NULL && packageManager != NULL);
    for (;;) {
        m_entry = m_entry == NULL
            ? m_list.FirstSelected() : m_list.NextSelected(m_entry);
        if (m_entry == NULL) {
            package->clear();
            packageManager->clear();
            break;
        }
        if (m_list.isFirstLevelChild(m_entry)) {
            *package = m_list.getPackage(m_entry);
            *packageManager = m_list.getPackageManager(m_entry);
            break;
        }
    }
}

}
