/*************************************************************************
 *
 *  $RCSfile: dp_gui_dialog.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:04:24 $
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

#include "svtools/controldims.hrc"
#include "dp_gui.h"
#include "rtl/uri.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "ucbhelper/content.hxx"
#include "unotools/configmgr.hxx"
#include "comphelper/anytostring.hxx"
#include "tools/isolang.hxx"
#include "toolkit/helper/vclunohelper.hxx"
#include "vcl/wintypes.hxx"
#include "vcl/msgbox.hxx"
#include "com/sun/star/lang/XUnoTunnel.hpp"
#include "com/sun/star/container/XChild.hpp"
#include "com/sun/star/ui/dialogs/XFilePicker.hpp"
#include "com/sun/star/ui/dialogs/XFolderPicker.hpp"
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include "com/sun/star/ui/dialogs/ExecutableDialogResults.hpp"
#include "com/sun/star/ucb/XContent.hpp"
#include "com/sun/star/ucb/XContentAccess.hpp"
#include "com/sun/star/ucb/NameClash.hpp"
#include "com/sun/star/ucb/ContentAction.hpp"
#include "com/sun/star/sdbc/XResultSet.hpp"
#include "com/sun/star/sdbc/XRow.hpp"
#include <vector>
#include <algorithm>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace dp_gui
{

const long ITEM_ID_PACKAGE = 1;
const long ITEM_ID_STATUS = 2;

//______________________________________________________________________________
DialogImpl::~DialogImpl()
{
}

//______________________________________________________________________________
DialogImpl::DialogImpl(
    Window * pParent, Reference<XComponentContext> const & xContext )
    : ModelessDialog( pParent, getResId(RID_DLG_PACKAGE_MANAGER) ),
      m_allowSharedLayerModification( ! office_is_running() ),
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
}

//------------------------------------------------------------------------------
::rtl::Reference<DialogImpl> DialogImpl::s_dialog;

//______________________________________________________________________________
::rtl::Reference<DialogImpl> DialogImpl::get(
    Reference<XComponentContext> const & xContext,
    Reference<awt::XWindow> const & xParent,
    OUString const & defaultView )
{
    if (s_dialog.is())
        return s_dialog;

    Window * pParent = DIALOG_NO_PARENT;
    if (xParent.is())
        pParent = VCLUnoHelper::GetWindow(xParent);
    ::rtl::Reference<DialogImpl> that( new DialogImpl( pParent, xContext ) );

    // XUnoTunnel: hack to set application solar mutex from within gui
    Reference<lang::XUnoTunnel> xUnoTunnel( that->m_xPkgMgrFac, UNO_QUERY );
    if (xUnoTunnel.is())
    {
        char const ar [] = "ResMgrMutexPointer";
        ::vos::IMutex ** ppmutex = reinterpret_cast< ::vos::IMutex ** >(
            xUnoTunnel->getSomething(
                Sequence<sal_Int8>( reinterpret_cast<sal_Int8 const *>(ar),
                                    ARLEN(ar) - 1 ) ) );
        if (ppmutex != 0)
            *ppmutex = &Application::GetSolarMutex();
    }

    // xxx todo: set icon:
//     that->SetIcon( ICON_PACKAGE_MANAGER );

    // !!! tab-order relates to creation order:
    that->m_ftPackages.reset(
        new FixedText( that.get(), getResId(RID_FT_PACKAGES) ) );

    // selection box: header bar + treelistbox:
    that->m_selectionBox.reset(
        new Control( that.get(), WB_BORDER | WB_TABSTOP ) );
    that->m_treelb.reset(
        new TreeListBoxImpl( that->m_selectionBox.get(), that.get() ) );
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

    that->m_bottomLine.reset( new FixedLine( that.get() ) );
    that->m_closeButton.reset(
        new OKButton( that.get(), getResId(RID_BTN_CLOSE) ) );
    that->m_helpButton.reset(
        new HelpButton( that.get(), getResId(RID_BTN_HELP) ) );

    // free local resources (RID < 256):
    that->FreeResource();

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
    sal_Int32 selWidth = that->m_treelb->GetSizePixel().getWidth();
    sal_Int32 vscrollWidth =
        Application::GetSettings().GetStyleSettings().GetScrollBarSize();

    sal_Int32 statusWidth = that->m_treelb->GetTextWidth(
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
        selWidth - statusWidth - vscrollWidth );
    that->m_headerBar->InsertItem( ITEM_ID_STATUS, strStatus, statusWidth );

    sal_Int32 tabs[ 3 ];
    tabs[ 0 ] = 2; // two tabs
    tabs[ 1 ] = statusWidth;
    tabs[ 2 ] = selWidth - statusWidth - vscrollWidth;
    that->m_treelb->SetTabs( tabs, MAP_PIXEL );
    that->m_treelb->InitHeaderBar( that->m_headerBar.get() );

    //### add top-level nodes: #########################

    that->m_treelb->SetUpdateMode(FALSE);

    that->m_treelb->addNode(
        0 /* no parent */,
        getResourceString(RID_STR_USER_INSTALLATION),
        OUString() /* no factory URL */,
        that->m_xPkgMgrFac->getPackageManager( OUSTR("user") ),
        Reference<deployment::XPackage>(),
        false /* no sort */ );
    that->m_treelb->addNode(
        0 /* no parent */,
        getResourceString(RID_STR_SHARED_INSTALLATION),
        OUString() /* no factory URL */,
        that->m_xPkgMgrFac->getPackageManager( OUSTR("shared") ),
        Reference<deployment::XPackage>(),
        false /* no sort */ );

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
    that->m_treelb->GrabFocus();

    // default selection:
    SvLBoxEntry * defEntry = that->m_treelb->GetEntry(
        defaultView.equalsIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM("shared") ) ? 1 : 0 );
    if (defEntry != 0)
        that->m_treelb->Select( defEntry );

    OSL_ASSERT( ! s_dialog.is() );
    s_dialog = that;
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
void DialogImpl::updateButtonStates()
{
    bool allowModification = true;
    bool bEnable = true;
    bool bDisable = true;
    bool bRemove = true;
    bool bExport = true;

    sal_Int32 nSelectedPackages = 0;
    for ( SvLBoxEntry * entry = m_treelb->FirstSelected();
          entry != 0; entry = m_treelb->NextSelected(entry) )
    {
        if (!m_allowSharedLayerModification && allowModification) {
            allowModification &=
                !m_treelb->getContext( entry ).equalsIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM("shared") );
        }

        Reference<deployment::XPackage> xPackage(
            m_treelb->getPackage(entry) );
        if (xPackage.is())
        {
            ++nSelectedPackages;
            if (m_treelb->isFirstLevelChild( entry ))
            {
                switch (getPackageState( xPackage )) {
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

    bEnable &= (allowModification && nSelectedPackages > 0);
    bDisable &= (allowModification && nSelectedPackages > 0);
    bRemove &= (allowModification && nSelectedPackages > 0);
    bExport &= (nSelectedPackages > 0);

    m_disableButton->Enable( bDisable );
    m_enableButton->Enable( bEnable );
    m_exportButton->Enable( bExport );
    SvLBoxEntry * currEntry = m_treelb->GetCurEntry();
    m_addButton->Enable(
        allowModification &&
        (currEntry != 0 &&
         m_treelb->GetParent( currEntry ) == 0 /* top-level */) &&
        (nSelectedPackages == 0) );
    m_removeButton->Enable( bRemove );
}

//______________________________________________________________________________
Sequence< Reference<deployment::XPackage> >
DialogImpl::TreeListBoxImpl::getSelectedPackages( bool onlyFirstLevel )
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
    return Sequence< Reference<deployment::XPackage> >( &ret[ 0 ], ret.size() );
}

//______________________________________________________________________________
void DialogImpl::clickAdd( USHORT )
{
    OSL_ASSERT( m_treelb->getSelectedPackages().getLength() == 0 );
    OUString context( m_treelb->getContext(m_treelb->GetCurEntry()) );
    OSL_ASSERT( context.getLength() > 0 );
    if (context.getLength() == 0)
        return;
    Reference<deployment::XPackageManager> xPackageManager(
        m_xPkgMgrFac->getPackageManager( context ) );
    OSL_ASSERT( xPackageManager.is() );

    Any mode(
        makeAny( static_cast<sal_Int16>(
                     ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE ) ) );
    Reference< ui::dialogs::XFilePicker > xFilePicker(
        m_xComponentContext->getServiceManager()
        ->createInstanceWithArgumentsAndContext(
            OUSTR("com.sun.star.ui.dialogs.FilePicker"),
            Sequence<Any>(&mode, 1), m_xComponentContext ), UNO_QUERY_THROW );
    xFilePicker->setTitle( m_strAddPackages );
    xFilePicker->setMultiSelectionMode(true);
    if (xFilePicker->execute() != ui::dialogs::ExecutableDialogResults::OK)
        return; // cancelled
    Sequence<OUString> files( xFilePicker->getFiles() );
    OSL_ASSERT( files.getLength() > 0 );

    ::rtl::Reference<ProgressCommandEnv> currentCmdEnv(
        new ProgressCommandEnv( this, m_strAddingPackages ) );
    currentCmdEnv->showProgress( files.getLength() );
    Reference<task::XAbortChannel> xAbortChannel(
        xPackageManager->createAbortChannel() );

    for ( sal_Int32 pos = files.getLength() > 1 ? 1 : 0;
          !currentCmdEnv->isAborted() && pos < files.getLength(); ++pos )
    {
        OUString file;
        if (files.getLength() > 1)
            file = make_url( files[ 0 ], files[ pos ] );
        else
            file = files[ pos ];
        currentCmdEnv->progressSection(
            extract_throw<OUString>(
                ::ucb::Content( file, currentCmdEnv.get() ).getPropertyValue(
                    OUSTR("Title") ) ), xAbortChannel );
        try {
            Reference< deployment::XPackage > xPackage(
                xPackageManager->addPackage(
                    file, OUString() /* detect media-type */,
                    xAbortChannel, currentCmdEnv.get() ) );
            OSL_ASSERT( xPackage.is() );
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
        new ProgressCommandEnv( this, m_strRemovingPackages ) );
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
    Sequence<Reference<deployment::XPackage> > selection(
        m_treelb->getSelectedPackages(true) );
    OSL_ASSERT( selection.getLength() > 0 );

    ::rtl::Reference<ProgressCommandEnv> currentCmdEnv(
        new ProgressCommandEnv( this, id == RID_BTN_ENABLE
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

//______________________________________________________________________________
void DialogImpl::clickExport( USHORT )
{
    Sequence< Reference<deployment::XPackage> > selection(
        m_treelb->getSelectedPackages() );
    OSL_ASSERT( selection.getLength() > 0 );
    if (selection.getLength() == 0)
        return;

    OUString destFolder, newTitle;
    sal_Int32 nameClashAction = NameClash::ASK;
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
            return; // cancelled
        destFolder = xFolderPicker->getDirectory();
    }
    else // single item selected
    {
        Any mode(
            makeAny(
                static_cast<sal_Int16>(
                    ui::dialogs::TemplateDescription::FILESAVE_SIMPLE ) ) );
        Reference<ui::dialogs::XFilePicker> xFilePicker(
            m_xComponentContext->getServiceManager()
            ->createInstanceWithArgumentsAndContext(
                OUSTR("com.sun.star.ui.dialogs.FilePicker"),
                Sequence<Any>(&mode, 1), m_xComponentContext ),
            UNO_QUERY_THROW );
        xFilePicker->setTitle( m_strExportPackage );
        xFilePicker->setMultiSelectionMode(false);

        ::rtl::Reference<ProgressCommandEnv> currentCmdEnv(
            new ProgressCommandEnv( this, m_strExportPackage ) );
        OSL_ASSERT( selection.getLength() == 1 );
        Reference<deployment::XPackage> const & xPackage = selection[ 0 ];
        ::ucb::Content sourceContent( xPackage->getURL(), currentCmdEnv.get() );
        xFilePicker->setDefaultName(
            extract_throw<OUString>( sourceContent.getPropertyValue(
                                         OUSTR("Title") ) ) );
        if (xFilePicker->execute() != ui::dialogs::ExecutableDialogResults::OK)
            return; // cancelled

        Sequence<OUString> files( xFilePicker->getFiles() );
        OSL_ASSERT( files.getLength() == 1 );
        OUString const & url = files[ 0 ];
        ::ucb::Content childContent( url, currentCmdEnv.get() );
        Reference<container::XChild> xChild( childContent.get(),
                                             UNO_QUERY_THROW );
        ::ucb::Content destFolderContent(
            Reference<XContent>( xChild->getParent(), UNO_QUERY_THROW ),
            currentCmdEnv.get() );
        destFolder = destFolderContent.getURL();
        newTitle = ::rtl::Uri::decode( url.copy( url.lastIndexOf( '/' ) + 1 ),
                                       rtl_UriDecodeWithCharset,
                                       RTL_TEXTENCODING_UTF8 );
        nameClashAction = NameClash::OVERWRITE; /* overwrite, because FilePicker
                                                   has already asked */
    }

    ::rtl::Reference<ProgressCommandEnv> currentCmdEnv(
        new ProgressCommandEnv( this, m_strExportingPackages ) );
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
void DialogImpl::errbox( Any const & exc )
{
    OUString msg;
    deployment::DeploymentException dpExc;
    if ((exc >>= dpExc) &&
        dpExc.Cause.getValueTypeClass() == TypeClass_EXCEPTION) {
        msg = reinterpret_cast<Exception const *>(
            dpExc.Cause.getValue() )->Message;
    }
    if (msg.getLength() == 0) // fallback for debugging purposes
        msg = ::comphelper::anyToString(exc);

    ::vos::OGuard guard( Application::GetSolarMutex() );
    ::std::auto_ptr<ErrorBox> box( new ErrorBox( this, WB_OK, msg ) );
    box->SetText( GetText() );
    box->Execute();
}

//##############################################################################

//______________________________________________________________________________
void DialogImpl::SyncPushButton::Click()
{
    try {
        (m_dialog->*m_clickCallback)( m_id );
    }
    catch (CommandFailedException &) {
        // already handled by UUI handler
    }
    catch (Exception &) {
        Any exc( ::cppu::getCaughtException() );
        m_dialog->errbox( exc );
    }
}

//------------------------------------------------------------------------------
extern "C"
{
static void SAL_CALL ThreadedPushButton_callback( void * p )
{
    DialogImpl::ThreadedPushButton * that =
        static_cast<DialogImpl::ThreadedPushButton *>(p);
    that->DialogImpl::SyncPushButton::Click();
}
}

//______________________________________________________________________________
void DialogImpl::ThreadedPushButton::Click()
{
    if (m_thread != 0) {
        osl_joinWithThread( m_thread );
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
        osl_joinWithThread( m_thread );
        osl_destroyThread( m_thread );
    }
}

//==============================================================================
static ResMgr * getResMgr()
{
    static ResMgr * s_pResMgr = 0;
    if (s_pResMgr == 0) {
        s_pResMgr = ResMgr::CreateResMgr( "deploymentgui" LIBRARY_SOLARUPD() );
        OSL_ASSERT( s_pResMgr != 0 );
    }
    return s_pResMgr;
}

//==============================================================================
ResId DialogImpl::getResId( USHORT id )
{
    ::vos::OGuard guard( Application::GetSolarMutex() );
    return ResId( id, getResMgr() );
}

//==============================================================================
String DialogImpl::getResourceString( USHORT id )
{
    ::vos::OGuard guard( Application::GetSolarMutex() );
    String ret( ResId( id, getResMgr() ) );
    if (ret.SearchAscii( "%PRODUCTNAME" ) != STRING_NOTFOUND)
    {
        static String s_brandName;
        if (s_brandName.Len() == 0)
        {
            OUString brandName(
                extract_throw<OUString>(
                    ::utl::ConfigManager::GetDirectConfigProperty(
                        ::utl::ConfigManager::PRODUCTNAME ) ) );
            s_brandName = brandName;
        }
        ret.SearchAndReplaceAllAscii( "%PRODUCTNAME", s_brandName );
    }
    return ret;
}

}
