/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui_dialog.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-20 17:58:35 $
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
#include "rtl/uri.hxx"
#include "osl/thread.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/implbase1.hxx"
#include "ucbhelper/content.hxx"
#include "unotools/configmgr.hxx"
#include "comphelper/anytostring.hxx"
#include "comphelper/sequence.hxx"
#include "tools/resmgr.hxx"
#include "toolkit/helper/vclunohelper.hxx"
#include "vcl/wintypes.hxx"
#include "vcl/msgbox.hxx"
#include "vcl/threadex.hxx"
#include "svtools/svtools.hrc"
#include "com/sun/star/container/XChild.hpp"
#include "com/sun/star/ui/dialogs/XFilePicker.hpp"
#include "com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp"
#include "com/sun/star/ui/dialogs/XFilterManager.hpp"
#include "com/sun/star/ui/dialogs/XFolderPicker.hpp"
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include "com/sun/star/ui/dialogs/ExecutableDialogResults.hpp"
#include "com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp"
#include "com/sun/star/ucb/XContent.hpp"
#include "com/sun/star/ucb/XContentAccess.hpp"
#include "com/sun/star/ucb/NameClash.hpp"
#include "com/sun/star/ucb/ContentAction.hpp"
#include "com/sun/star/sdbc/XResultSet.hpp"
#include "com/sun/star/sdbc/XRow.hpp"
#include "boost/function.hpp"
#include "boost/bind.hpp"
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

namespace {
extern "C" {
void SAL_CALL InstallThread( void * p )
{
    DialogImpl * that =  static_cast<DialogImpl*>(p);
    that->installExtensions();
}
} // extern "C"
} //anon namespace


//______________________________________________________________________________
DialogImpl::DialogImpl(
    Window * pParent, Sequence<OUString> const & arExtensions,
    Reference<XComponentContext> const & xContext )
    : ModelessDialog( pParent, getResId(RID_DLG_PACKAGE_MANAGER) ),
      m_modifiableContext( new ModifiableContext ),
      m_arExtensions(arExtensions),
      m_installThread(0),
      m_bAutoInstallFinished(false),
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
    //If unopkg gui extension1 extension2 ... was used then we install them right away
    if (m_arExtensions.getLength() > 0)
        Application::PostUserEvent(
            LINK( this, DialogImpl, startInstallExtensions ), 0);

}

//______________________________________________________________________________
DialogImpl::~DialogImpl()
{
    if (m_updatability.get() != NULL)
        m_updatability->stop();
}

//------------------------------------------------------------------------------
::rtl::Reference<DialogImpl> DialogImpl::s_dialog;

//______________________________________________________________________________
::rtl::Reference<DialogImpl> DialogImpl::get(
    Reference<XComponentContext> const & xContext,
    Reference<awt::XWindow> const & xParent,
    Sequence<OUString> const & arExtensions,
    OUString const & defaultView )
{
    if (s_dialog.is()) {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
        return s_dialog;
    }

    Window * pParent = DIALOG_NO_PARENT;
    if (xParent.is())
        pParent = VCLUnoHelper::GetWindow(xParent);
    ::rtl::Reference<DialogImpl> that( new DialogImpl( pParent,
        arExtensions,xContext ) );

    // xxx todo: set icon:
//     that->SetIcon( ICON_PACKAGE_MANAGER );

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

    //
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
            xContext, that->m_packageManagers, *that->m_checkUpdatesButton ) );

    // sizes, spacing, position:
    that->m_buttonSize = that->LogicToPixel(
        Size( RSC_CD_PUSHBUTTON_WIDTH, RSC_CD_PUSHBUTTON_HEIGHT ),
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
    that->m_ftFontHeight = that->m_ftPackages->GetTextHeight();
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
              (6 * that->m_buttonSize.getHeight()) +
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

        ::ucb::Content ucb_tdocRoot( OUSTR("vnd.sun.star.tdoc:/"), 0 );
        that->m_xTdocRoot.set( ucb_tdocRoot.get() );

//         // scan for open documents:
//         Reference<sdbc::XResultSet> xResultSet(
//             ucb_tdocRoot.createCursor( Sequence<OUString>(),
//                                        ::ucb::INCLUDE_FOLDERS_ONLY ) );
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

    const ::vos::OGuard guard( Application::GetSolarMutex() );
    if (! s_dialog.is()) {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
        s_dialog = that;
    }
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

//IMPL_STATIC_LINK( DialogImpl, startInstallExtensions, void *, p )

//This event should only be called onse during the lifetime of DialogImpl. It is posted
//in the constructor of DialogImpl.
//It is used to install the extension when running unopkg gui extensions1 extension2 ...
//We use this event to make sure that the extension manager dialog is showing.
//
IMPL_LINK( DialogImpl, startInstallExtensions, DialogImpl * , EMPTYARG )
{
    if (m_installThread != 0)
    {
        OSL_ASSERT(0);
    }
    else
    {
        m_installThread = osl_createSuspendedThread( InstallThread, this );
        OSL_ASSERT(m_installThread != 0 );
        osl_resumeThread(m_installThread );
    }
    return 0;
}

void DialogImpl::installExtensions()
{
    OSL_ASSERT(m_arExtensions.getLength() > 0);
    //Currently unopkg gui ext1 ext2 ... is only supported for user context.
    OUString context(OUSTR("user"));
    Reference<deployment::XPackageManager> xPackageManager(
        m_xPkgMgrFac->getPackageManager( context) );
    OSL_ASSERT( xPackageManager.is() );

    ::rtl::Reference<ProgressCommandEnv> currentCmdEnv(
        new ProgressCommandEnv( context, this, m_strAddingPackages, true) );
    currentCmdEnv->showProgress( m_arExtensions.getLength() );
    Reference<task::XAbortChannel> xAbortChannel(
        xPackageManager->createAbortChannel() );

    for ( sal_Int32 pos = 0;
          !currentCmdEnv->isAborted() && pos < m_arExtensions.getLength(); ++pos )
    {
        OUString file;
        file = m_arExtensions[ pos ];
        currentCmdEnv->progressSection(
            ::ucb::Content( file, currentCmdEnv.get() ).getPropertyValue(
                OUSTR("Title") ).get<OUString>(), xAbortChannel );
        try
        {
            Reference<deployment::XPackage> xPackage(
                xPackageManager->addPackage(
                    file, OUString() /* detect media-type */,
                    xAbortChannel, currentCmdEnv.get() ) );
            OSL_ASSERT( xPackage.is() );
            m_treelb->select(xPackage);
        }
        catch (Exception &) {
            //all exception should be handled by the interaction between xPackageManager and
            //the interaction handler of currentCmdEnv
            continue;
        }

        //catch (
        // todo replace, remove later, currently used to signa name clashes etc.
        //catch (Exception &) {
        //  Any exc( ::cppu::getCaughtException() );
        //  OUString msg;
        //  deployment::DeploymentException dpExc;
        //  if ((exc >>= dpExc) &&
        //      dpExc.Cause.getValueTypeClass() == TypeClass_EXCEPTION) {
        //      // notify error cause only:
        //      msg = reinterpret_cast<Exception const *>(
        //          dpExc.Cause.getValue() )->Message;
        //  }
        //  if (msg.getLength() == 0) // fallback for debugging purposes
        //      msg = ::comphelper::anyToString(exc);
        //  errbox( msg );
        //}
    }

    m_bAutoInstallFinished = true;
    updateButtonStates();
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

    sal_Int32 nSelectedPackages = 0;
    for ( SvLBoxEntry * entry = m_treelb->FirstSelected();
          entry != 0; entry = m_treelb->NextSelected(entry) )
    {
        allowModification = allowModification &&
            m_modifiableContext->isModifiable( m_treelb->getContext( entry ) );

        Reference<deployment::XPackage> xPackage(
            m_treelb->getPackage(entry) );
        if (xPackage.is())
        {
            ++nSelectedPackages;
            if (m_treelb->isFirstLevelChild( entry ))
            {
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
                bEnable = bDisable = bRemove = false;
            }
        }
        else { // selected non-package entry:
            bExport = bEnable = bDisable = bRemove = false;
        }
    }
    //When unopkg gui ext1 ext2 ... was used then the installation starts automatically.
    //Untill this installation has finished the user must not interfere.
    bool bDisableAll = m_arExtensions.getLength() > 0
        && !m_bAutoInstallFinished;

    bEnable &= (allowModification && nSelectedPackages > 0) && !bDisableAll;
    bDisable &= (allowModification && nSelectedPackages > 0) && !bDisableAll;
    bRemove &= (allowModification && nSelectedPackages > 0) && !bDisableAll;
    bExport &= (nSelectedPackages > 0);

    m_disableButton->Enable( bDisable );
    m_enableButton->Enable( bEnable );
    m_exportButton->Enable( bExport );
    SvLBoxEntry * currEntry = m_treelb->getCurrentSingleSelectedEntry();

    m_addButton->Enable(
        !bDisableAll &&
        allowModification &&
        (currEntry != 0 &&
         m_treelb->GetParent( currEntry ) == 0 /* top-level */) &&
        (nSelectedPackages == 0) );
    m_removeButton->Enable( bRemove );
}

//______________________________________________________________________________
Sequence< Reference<deployment::XPackage> >
DialogImpl::TreeListBoxImpl::getSelectedPackages( bool onlyFirstLevel ) const
{
    ::std::vector< Reference<deployment::XPackage> > ret;

    for ( SvLBoxEntry * entry = FirstSelected();
          entry != 0; entry = NextSelected(entry) )
    {
        if (onlyFirstLevel && !isFirstLevelChild( entry ))
            continue;
        Reference<deployment::XPackage> xPackage( getPackage(entry) );
        if (xPackage.is())
            ret.push_back( xPackage );
    }
    return comphelper::containerToSequence(ret);
}

namespace {
struct StrAllFiles : public rtl::StaticWithInit<const OUString, StrAllFiles> {
    const OUString operator () () {
        const ::vos::OGuard guard( Application::GetSolarMutex() );
        ::std::auto_ptr<ResMgr> const resmgr(
            ResMgr::CreateResMgr( "fps_office" LIBRARY_SOLARUPD() ) );
        OSL_ASSERT( resmgr.get() != 0 );
        String ret( ResId( STR_FILTERNAME_ALL, resmgr.get() ) );
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
    OSL_ASSERT( m_treelb->getSelectedPackages().getLength() == 0 );
    OUString context( m_treelb->getContext(
                          m_treelb->getCurrentSingleSelectedEntry() ) );
    OSL_ASSERT( context.getLength() > 0 );
    if (context.getLength() == 0)
        return;

    Reference<deployment::XPackageManager> xPackageManager(
        m_xPkgMgrFac->getPackageManager( context ) );
    OSL_ASSERT( xPackageManager.is() );

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
            ::ucb::Content( file, currentCmdEnv.get() ).getPropertyValue(
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
            break;
        }
    }
}

//______________________________________________________________________________
void DialogImpl::clickRemove( USHORT )
{
    OSL_ASSERT( m_treelb->getSelectedPackages(true).getLength() > 0 );

    typedef ::std::pair<
        Reference<deployment::XPackageManager>, OUString > t_item;
    ::std::vector<t_item> to_be_removed;

    for ( SvLBoxEntry * entry = m_treelb->FirstSelected();
          entry != 0; entry = m_treelb->NextSelected(entry) )
    {
        if (m_treelb->isFirstLevelChild( entry ))
        {
            OUString context(m_treelb->getContext(entry));
            OSL_ASSERT( context.getLength() > 0 );
            if (context.getLength() == 0)
                continue;
            to_be_removed.push_back(
                t_item( m_xPkgMgrFac->getPackageManager(context),
                        m_treelb->getPackage(entry)->getName() ) );
        }
    }

    ::rtl::Reference<ProgressCommandEnv> currentCmdEnv(
        new ProgressCommandEnv( m_xComponentContext, this, m_strRemovingPackages) );
    currentCmdEnv->showProgress( to_be_removed.size() );
    for ( ::std::size_t pos = 0;
          !currentCmdEnv->isAborted() && pos < to_be_removed.size(); ++pos )
    {
        t_item const & item = to_be_removed[ pos ];
        Reference<task::XAbortChannel> xAbortChannel(
            item.first->createAbortChannel() );
        currentCmdEnv->progressSection( item.second, xAbortChannel );
        try {
            item.first->removePackage(
                item.second, xAbortChannel, currentCmdEnv.get() );
        }
        catch (CommandAbortedException &) {
            break;
        }
    }
}

//______________________________________________________________________________
void DialogImpl::clickEnableDisable( USHORT id )
{
    Sequence< Reference<deployment::XPackage> > selection(
        m_treelb->getSelectedPackages(true) );
    OSL_ASSERT( selection.getLength() > 0 );

    ::rtl::Reference<ProgressCommandEnv> currentCmdEnv(
        new ProgressCommandEnv( m_xComponentContext, this, id == RID_BTN_ENABLE
                                ? m_strEnablingPackages
                                : m_strDisablingPackages ) );
    currentCmdEnv->showProgress( selection.getLength() );
    for ( sal_Int32 pos = 0;
          !currentCmdEnv->isAborted() && pos < selection.getLength(); ++pos )
    {
        Reference<deployment::XPackage> const & xPackage = selection[ pos ];
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
        ::ucb::Content sourceContent( xPackage->getURL(), currentCmdEnv.get() );
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
        ::ucb::Content childContent( url, currentCmdEnv.get() );
        Reference<container::XChild> xChild( childContent.get(),
                                             UNO_QUERY_THROW );
        ::ucb::Content destFolderContent(
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
    Sequence< Reference<deployment::XPackage> > selection(
        m_treelb->getSelectedPackages() );
    OSL_ASSERT( selection.getLength() > 0 );
    if (selection.getLength() == 0)
        return;

    OUString destFolder;
    OUString newTitle;
    sal_Int32 nameClashAction = NameClash::ASK;
    using namespace vcl::solarthread;
    if (! syncExecute( boost::bind(
                           &DialogImpl::solarthread_raiseExportPickers, this,
                           selection,
                           inout_by_ref(destFolder),
                           inout_by_ref(newTitle),
                           inout_by_ref(nameClashAction) ) ))
        return;

    ::rtl::Reference<ProgressCommandEnv> currentCmdEnv(
        new ProgressCommandEnv( m_xComponentContext, this, m_strExportingPackages ) );
    currentCmdEnv->showProgress( selection.getLength() );
    for ( sal_Int32 pos = 0;
          !currentCmdEnv->isAborted() && pos < selection.getLength(); ++pos )
    {
        Reference<deployment::XPackage> const & xPackage = selection[ pos ];
        currentCmdEnv->progressSection( xPackage->getDisplayName() );
        OSL_ASSERT( destFolder.getLength() > 0 );
        xPackage->exportTo( destFolder, newTitle, nameClashAction,
                            currentCmdEnv.get() );
    }
}

//______________________________________________________________________________
void DialogImpl::clickCheckUpdates( USHORT )
{
    checkUpdates(false);
}

//______________________________________________________________________________
void DialogImpl::errbox( OUString const & msg )
{
    const ::vos::OGuard guard( Application::GetSolarMutex() );
    ::std::auto_ptr<ErrorBox> box( new ErrorBox( this, WB_OK, msg ) );
    box->SetText( GetText() );
    box->Execute();
}

//______________________________________________________________________________
void DialogImpl::checkUpdates(bool selected)
{
    std::vector<UpdateData> data;
    if (UpdateDialog(
            m_xComponentContext, this, m_modifiableContext,
            (selected
             ? new SelectedPackageIterator(*m_treelb.get())
             : rtl::Reference<SelectedPackageIterator>()),
            (selected
             ? Sequence<Reference<deployment::XPackageManager> >()
             : m_packageManagers),
            &data).Execute() == RET_OK
        && !data.empty())
    {
        UpdateInstallDialog(this, data, m_xComponentContext).Execute();
    }
}

//##############################################################################

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
    return ResId( id, DeploymentGuiResMgr::get() );
}

//==============================================================================
String DialogImpl::getResourceString( USHORT id )
{
    // init with non-acquired solar mutex:
    BrandName::get();
    const ::vos::OGuard guard( Application::GetSolarMutex() );
    String ret( ResId( id, DeploymentGuiResMgr::get() ) );
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
