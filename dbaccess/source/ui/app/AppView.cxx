/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AppView.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:11:43 $
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
#include "precompiled_dbaccess.hxx"
#ifndef DBAUI_APPVIEW_HXX
#include "AppView.hxx"
#endif
#ifndef _DBU_APP_HRC_
#include "dbu_app.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBAUI_IAPPELEMENTNOTIFICATION_HXX
#include "IAppElementNotification.hxx"
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARD_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef DBAUI_APPDETAILVIEW_HXX
#include "AppDetailView.hxx"
#endif
#ifndef _DBAUI_TABLETREE_HXX_
#include "tabletree.hxx"
#endif
#ifndef DBAUI_APPSWAPWINDOW_HXX
#include "AppSwapWindow.hxx"
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef DBAUI_APPSWAPWINDOW_HXX
#include "AppSwapWindow.hxx"
#endif
#ifndef DBAUI_TITLE_WINDOW_HXX
#include "AppTitleWindow.hxx"
#endif
#ifndef _DBAUI_DSNTYPES_HXX_
#include "dsntypes.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef DBAUI_ICONTROLLER_HXX
#include "IController.hxx"
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif


using namespace ::dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;

//==================================================================
// class OAppBorderWindow
DBG_NAME(OAppBorderWindow)
//==================================================================
OAppBorderWindow::OAppBorderWindow(OApplicationView* _pParent,PreviewMode _ePreviewMode) : Window(_pParent,WB_DIALOGCONTROL)
    ,m_pPanel(NULL)
    ,m_pDetailView(NULL)
    ,m_pView(_pParent)
{
    DBG_CTOR(OAppBorderWindow,NULL);

    SetBorderStyle(WINDOW_BORDER_MONO);

    m_pPanel = new OTitleWindow(this,STR_DATABASE,WB_BORDER | WB_DIALOGCONTROL ,FALSE);
    m_pPanel->SetBorderStyle(WINDOW_BORDER_MONO);
    OApplicationSwapWindow* pSwap = new OApplicationSwapWindow( m_pPanel, *this );
    pSwap->Show();
    pSwap->SetUniqueId(UID_APP_SWAP_VIEW);

    m_pPanel->setChildWindow(pSwap);
    m_pPanel->SetUniqueId(UID_APP_DATABASE_VIEW);
    m_pPanel->Show();

    m_pDetailView = new OApplicationDetailView(*this,_ePreviewMode);
    m_pDetailView->Show();

    ImplInitSettings();
}
// -----------------------------------------------------------------------------
OAppBorderWindow::~OAppBorderWindow()
{
    //////////////////////////////////////////////////////////////////////
    // Childs zerstoeren
    if ( m_pPanel )
    {
        m_pPanel->Hide();
        ::std::auto_ptr<Window> aTemp(m_pPanel);
        m_pPanel = NULL;
    }
    if ( m_pDetailView )
    {
        m_pDetailView->Hide();
        ::std::auto_ptr<Window> aTemp(m_pDetailView);
        m_pDetailView = NULL;
    }

    DBG_DTOR(OAppBorderWindow,NULL);
}
// -----------------------------------------------------------------------------
void OAppBorderWindow::GetFocus()
{
    if ( m_pPanel )
        m_pPanel->GrabFocus();
}
// -----------------------------------------------------------------------------
void OAppBorderWindow::Resize()
{
    //////////////////////////////////////////////////////////////////////
    // Abmessungen parent window
    Size aOutputSize( GetOutputSize() );
    long nOutputWidth   = aOutputSize.Width();
    long nOutputHeight  = aOutputSize.Height();
    long nX = 0;

    Size aFLSize = LogicToPixel( Size( 3, 8 ), MAP_APPFONT );
    if ( m_pPanel )
    {
        OApplicationSwapWindow* pSwap = getPanel();
        if ( pSwap )
        {
            if ( pSwap->GetEntryCount() != 0 )
                nX = pSwap->GetBoundingBox( pSwap->GetEntry(0) ).GetWidth() + aFLSize.Height();
        }
        nX = ::std::max(m_pPanel->GetWidthPixel() ,nX);
        m_pPanel->SetPosSizePixel(Point(0,0),Size(nX,nOutputHeight));
    }

    if ( m_pDetailView )
        m_pDetailView->SetPosSizePixel(Point(nX + aFLSize.Width(),0),Size(nOutputWidth - nX - aFLSize.Width(),nOutputHeight));
}
// -----------------------------------------------------------------------------
void OAppBorderWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
        (rDCEvt.GetType() == DATACHANGED_DISPLAY) ||
        (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
        ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
        (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        ImplInitSettings();
        Invalidate();
    }
}
// -----------------------------------------------------------------------------
void OAppBorderWindow::ImplInitSettings()
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    if( true )
    {
        Font aFont;
        aFont = rStyleSettings.GetFieldFont();
        aFont.SetColor( rStyleSettings.GetWindowTextColor() );
        SetPointFont( aFont );
    }

    if( true )
    {
        SetTextColor( rStyleSettings.GetFieldTextColor() );
        SetTextFillColor();
    }

    if( true )
        SetBackground( rStyleSettings.GetDialogColor() );

    /*SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetDialogColor() ) );
    SetFillColor( Application::GetSettings().GetStyleSettings().GetDialogColor() );
    SetTextFillColor( Application::GetSettings().GetStyleSettings().GetDialogColor() );*/
}
// -----------------------------------------------------------------------------
OApplicationView* OAppBorderWindow::getView() const
{
    return m_pView;
}

// -----------------------------------------------------------------------------
OApplicationSwapWindow* OAppBorderWindow::getPanel() const
{
    return static_cast< OApplicationSwapWindow* >( m_pPanel->getChildWindow() );
}

// -----------------------------------------------------------------------------
OApplicationDetailView* OAppBorderWindow::getDetailView() const
{
    return m_pDetailView;
}

//==================================================================
// class OApplicationView
//==================================================================
DBG_NAME(OApplicationView);
//------------------------------------------------------------------------------
OApplicationView::OApplicationView( Window* pParent
                                    ,const Reference< XMultiServiceFactory >& _rxOrb
                                    ,IController* _pIController
                                    ,IApplicationElementNotification* _pController
                                    ,IControlActionListener* _pActonListener
                                    ,IContainerFoundListener* _pContainerListener
                                    ,IViewChangeListener* _pViewChangeListener
                                    ,const Reference< XController>& _xController
                                    ,PreviewMode _ePreviewMode
                                   ) :
    ODataView( pParent ,_pIController,_rxOrb,WB_DIALOGCONTROL )
    ,m_xController(_xController)
    ,m_pElementNotification( _pController )
    ,m_pActonListener(_pActonListener)
    ,m_pContainerListener(_pContainerListener)
    ,m_pViewChangeListener(_pViewChangeListener)
    ,m_eChildFocus(NONE)
{
    DBG_CTOR(OApplicationView,NULL);

    try
    {
        m_aLocale = SvtSysLocale().GetLocaleData().getLocale();
    }
    catch(Exception&)
    {
    }

    m_pWin = new OAppBorderWindow(this,_ePreviewMode);
    m_pWin->SetUniqueId(UID_APP_VIEW_BORDER_WIN);
    m_pWin->Show();

    ImplInitSettings();
}

//------------------------------------------------------------------------------
OApplicationView::~OApplicationView()
{
    DBG_DTOR(OApplicationView,NULL);

    {
        stopComponentListening(m_xObject);
        m_pWin->Hide();
        ::std::auto_ptr<Window> aTemp(m_pWin);
        m_pWin = NULL;
    }
    m_pElementNotification = NULL;
}
// -----------------------------------------------------------------------------
void OApplicationView::createIconAutoMnemonics( MnemonicGenerator& _rMnemonics )
{
    if ( m_pWin && m_pWin->getPanel() )
        m_pWin->getPanel()->createIconAutoMnemonics( _rMnemonics );
}

// -----------------------------------------------------------------------------
void OApplicationView::setTaskExternalMnemonics( MnemonicGenerator& _rMnemonics )
{
    if ( m_pWin && m_pWin->getDetailView() )
        m_pWin->getDetailView()->setTaskExternalMnemonics( _rMnemonics );
}

// -----------------------------------------------------------------------------
void OApplicationView::DataChanged( const DataChangedEvent& rDCEvt )
{
    ODataView::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
        (rDCEvt.GetType() == DATACHANGED_DISPLAY) ||
        (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
        ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
        (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        ImplInitSettings();
        Invalidate();
    }
}
//------------------------------------------------------------------------------
void OApplicationView::resizeDocumentView(Rectangle& _rPlayground)
{
    if ( m_pWin && !_rPlayground.IsEmpty() )
    {
        Size aFLSize = LogicToPixel( Size( 3, 3 ), MAP_APPFONT );
        _rPlayground.Move( aFLSize.A(),aFLSize.B() );
        Size aOldSize = _rPlayground.GetSize();
        _rPlayground.SetSize( Size(aOldSize.A() - 2*aFLSize.A(), aOldSize.B() - 2*aFLSize.B()) );

        m_pWin->SetPosSizePixel(_rPlayground.TopLeft() , _rPlayground.GetSize() );
    }
    // just for completeness: there is no space left, we occupied it all ...
    _rPlayground.SetPos( _rPlayground.BottomRight() );
    _rPlayground.SetSize( Size( 0, 0 ) );
}
//------------------------------------------------------------------------------
long OApplicationView::PreNotify( NotifyEvent& rNEvt )
{
    switch(rNEvt.GetType())
    {
        case EVENT_GETFOCUS:
            if( m_pWin && getPanel() && getPanel()->HasChildPathFocus() )
                m_eChildFocus = PANELSWAP;
            else if ( m_pWin && getDetailView() && getDetailView()->HasChildPathFocus() )
                m_eChildFocus = DETAIL;
            else
                m_eChildFocus = NONE;
            break;
        case EVENT_KEYINPUT:
        {
            const KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
            // give the pane the chance to intercept mnemonic accelerators
            // #i34790# - 2004-09-30 - fs@openoffice.org
            if ( getPanel() && getPanel()->interceptKeyInput( *pKeyEvent ) )
                return 1L;
            // and ditto the detail view
            // #i72799# - 2006-12-20 / frank.schoenheit@sun.com
            if ( getDetailView() && getDetailView()->interceptKeyInput( *pKeyEvent ) )
                return 1L;
        }
        break;
    }

    return ODataView::PreNotify(rNEvt);
}
// -----------------------------------------------------------------------------
IClipboardTest* OApplicationView::getActiveChild() const
{
    IClipboardTest* pTest = NULL;
    if ( DETAIL == m_eChildFocus )
        pTest = getDetailView();
    return pTest;
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationView::isCopyAllowed()
{
    IClipboardTest* pTest = getActiveChild();
    return pTest && pTest->isCopyAllowed();
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationView::isCutAllowed()
{
    IClipboardTest* pTest = getActiveChild();
    return pTest && pTest->isCutAllowed();
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationView::isPasteAllowed()
{
    IClipboardTest* pTest = getActiveChild();
    return pTest && pTest->isPasteAllowed();
}
// -----------------------------------------------------------------------------
void OApplicationView::copy()
{
    IClipboardTest* pTest = getActiveChild();
    if ( pTest )
        pTest->copy();
}
// -----------------------------------------------------------------------------
void OApplicationView::cut()
{
    IClipboardTest* pTest = getActiveChild();
    if ( pTest )
        pTest->cut();
}
// -----------------------------------------------------------------------------
void OApplicationView::paste()
{
    IClipboardTest* pTest = getActiveChild();
    if ( pTest )
        pTest->paste();
}
// -----------------------------------------------------------------------------
::rtl::OUString OApplicationView::getQualifiedName( SvLBoxEntry* _pEntry ) const
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->getQualifiedName( _pEntry );
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationView::isLeaf(SvLBoxEntry* _pEntry) const
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->isLeaf(_pEntry);
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationView::isALeafSelected() const
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->isALeafSelected();
}
// -----------------------------------------------------------------------------
void OApplicationView::selectAll()
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    getDetailView()->selectAll();
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationView::isSortUp() const
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->isSortUp();
}
// -----------------------------------------------------------------------------
void OApplicationView::sortDown()
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    getDetailView()->sortDown();
}
// -----------------------------------------------------------------------------
void OApplicationView::sortUp()
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    getDetailView()->sortUp();
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationView::isFilled() const
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->isFilled();
}
// -----------------------------------------------------------------------------
ElementType OApplicationView::getElementType() const
{
    OSL_ENSURE(m_pWin && getDetailView() && getPanel(),"Detail view is NULL! -> GPF");
    return getDetailView()->HasChildPathFocus() ? getDetailView()->getElementType() : getPanel()->getElementType();
}
// -----------------------------------------------------------------------------
sal_Int32 OApplicationView::getSelectionCount()
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->getSelectionCount();
}
// -----------------------------------------------------------------------------
sal_Int32 OApplicationView::getElementCount()
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->getElementCount();
}
// -----------------------------------------------------------------------------
void OApplicationView::getSelectionElementNames( ::std::vector< ::rtl::OUString>& _rNames ) const
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    getDetailView()->getSelectionElementNames( _rNames );
}
// -----------------------------------------------------------------------------
void OApplicationView::selectElements(const Sequence< ::rtl::OUString>& _aNames)
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    getDetailView()->selectElements( _aNames );
}
// -----------------------------------------------------------------------------
SvLBoxEntry* OApplicationView::elementAdded(ElementType eType,const ::rtl::OUString& _rName, const Any& _rObject )
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->elementAdded(eType,_rName,_rObject);
}
// -----------------------------------------------------------------------------
void OApplicationView::elementRemoved(ElementType eType,const ::rtl::OUString& _rName )
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    getDetailView()->elementRemoved(eType,_rName);
}
// -----------------------------------------------------------------------------
void OApplicationView::elementReplaced(ElementType _eType
                                                    ,const ::rtl::OUString& _rOldName
                                                    ,const ::rtl::OUString& _rNewName )
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    getDetailView()->elementReplaced(_eType, _rOldName, _rNewName );
}
// -----------------------------------------------------------------------------
void OApplicationView::clearPages(sal_Bool _bTaskAlso)
{
    OSL_ENSURE(m_pWin && getDetailView() && getPanel(),"Detail view is NULL! -> GPF");
    getPanel()->clearSelection();
    getDetailView()->clearPages(_bTaskAlso);
}
// -----------------------------------------------------------------------------
void OApplicationView::selectContainer(ElementType _eType)
{
    OSL_ENSURE(m_pWin && getPanel(),"Detail view is NULL! -> GPF");
    WaitObject aWO(this);
    getPanel()->selectContainer(_eType);
}
// -----------------------------------------------------------------------------
SvLBoxEntry* OApplicationView::getEntry( const Point& _aPosPixel ) const
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->getEntry(_aPosPixel);
}
// -----------------------------------------------------------------------------
PreviewMode OApplicationView::getPreviewMode()
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->getPreviewMode();
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationView::isPreviewEnabled()
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->isPreviewEnabled();
}
// -----------------------------------------------------------------------------
void OApplicationView::switchPreview(PreviewMode _eMode)
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    getDetailView()->switchPreview(_eMode);
}
// -----------------------------------------------------------------------------
void OApplicationView::showPreview(const Reference< XContent >& _xContent)
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    stopComponentListening(m_xObject);
    m_xObject = NULL;
    getDetailView()->showPreview(_xContent);
}
// -----------------------------------------------------------------------------
void OApplicationView::showPreview( const ::rtl::OUString& _sDataSourceName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
                                    const ::rtl::OUString& _sName,
                                    sal_Bool _bTable)
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    if ( isPreviewEnabled() )
    {
        stopComponentListening(m_xObject);
        m_xObject = NULL;
        try
        {
            Reference<XNameAccess> xNameAccess;
            if ( _bTable )
            {
                Reference<XTablesSupplier> xSup(_xConnection,UNO_QUERY);
                if ( xSup.is() )
                    xNameAccess.set(xSup->getTables(),UNO_QUERY);
            }
            else
            {
                Reference<XQueriesSupplier> xSup(_xConnection,UNO_QUERY);
                if ( xSup.is() )
                    xNameAccess.set(xSup->getQueries(),UNO_QUERY);
            }
            if ( xNameAccess.is() && xNameAccess->hasByName(_sName) )
                m_xObject.set(xNameAccess->getByName(_sName),UNO_QUERY);
        }
        catch(Exception)
        {
            OSL_ENSURE(0,"Exception caught!");
        }
        if ( m_xObject.is() )
            startComponentListening(m_xObject);
        getDetailView()->showPreview(_sDataSourceName,_sName,_bTable);
    }
}
// -----------------------------------------------------------------------------
void OApplicationView::GetFocus()
{
    if ( m_eChildFocus == NONE && m_pWin )
    {
        m_pWin->GrabFocus();
    }
}
// -----------------------------------------------------------------------------
void OApplicationView::_disposing( const ::com::sun::star::lang::EventObject& /*_rSource*/ )
{
    if ( m_pWin && getDetailView() )
        showPreview(NULL);
}
// -----------------------------------------------------------------------------
void OApplicationView::ImplInitSettings()
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    if( true )
    {
        Font aFont;
        aFont = rStyleSettings.GetFieldFont();
        aFont.SetColor( rStyleSettings.GetWindowTextColor() );
        SetPointFont( aFont );
    }

    if( true )
    {
        SetTextColor( rStyleSettings.GetFieldTextColor() );
        SetTextFillColor();
    }

    if( true )
        SetBackground( rStyleSettings.GetFieldColor() );
    /*SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetDialogColor() ) );
    SetFillColor( Application::GetSettings().GetStyleSettings().GetDialogColor() );
    SetTextFillColor( Application::GetSettings().GetStyleSettings().GetDialogColor() );*/
}
//-----------------------------------------------------------------------------
