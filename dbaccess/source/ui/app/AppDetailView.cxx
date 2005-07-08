/*************************************************************************
 *
 *  $RCSfile: AppDetailView.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-08 10:37:44 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
 *  Contributor(s): Ocke Janssen
 *
 *
 ************************************************************************/
#ifndef DBAUI_APPDETAILVIEW_HXX
#include "AppDetailView.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef _DBU_APP_HRC_
#include "dbu_app.hrc"
#endif
#ifndef DBAUI_APPVIEW_HXX
#include "AppView.hxx"
#endif
#ifndef DBAUI_IAPPELEMENTNOTIFICATION_HXX
#include "IAppElementNotification.hxx"
#endif
#ifndef _COM_SUN_STAR_UI_XUICONFIGURATIONMANAGER_HPP_
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XMODULEUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XIMAGEMANAGER_HPP_
#include <com/sun/star/ui/XImageManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_IMAGETYPE_HPP_
#include <com/sun/star/ui/ImageType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XVIEWSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHIC_HPP_
#include <com/sun/star/graphic/XGraphic.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _DBAUI_LISTVIEWITEMS_HXX_
#include "listviewitems.hxx"
#endif
#ifndef _IMAGE_HXX //autogen
#include <vcl/image.hxx>
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef DBAUI_APPDETAILPAGEHELPER_HXX
#include "AppDetailPageHelper.hxx"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _DBACCESS_UI_CALLBACKS_HXX_
#include "callbacks.hxx"
#endif
#ifndef DBAUI_ICONTROLLER_HXX
#include "IController.hxx"
#endif
#ifndef _SVTOOLS_LOCALRESACCESS_HXX_
#include <svtools/localresaccess.hxx>
#endif


using namespace ::dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::container;

#define SPACEBETWEENENTRIES     4

OCreationList::OCreationList(OTasksWindow* _pParent)
    :SvTreeListBox( _pParent, WB_TABSTOP | WB_HASBUTTONSATROOT | WB_HASBUTTONS )
    ,m_pTaskWindow(_pParent)
    ,m_pMouseDownEntry( NULL )
    ,m_pLastActiveEntry( NULL )
{
    USHORT nSize = SPACEBETWEENENTRIES;
    SetSpaceBetweenEntries(nSize);
    SetSelectionMode( NO_SELECTION );
    SetExtendedWinBits( EWB_NO_AUTO_CURENTRY );
}
// -----------------------------------------------------------------------------
void OCreationList::Paint( const Rectangle& _rRect )
{
    if ( m_pMouseDownEntry )
        m_aOriginalFont = GetFont();

    m_aOriginalBackgroundColor = GetBackground().GetColor();
    SvTreeListBox::Paint( _rRect );
    SetBackground( m_aOriginalBackgroundColor );

    if ( m_pMouseDownEntry )
        Control::SetFont( m_aOriginalFont );
}
// -----------------------------------------------------------------------------
void OCreationList::PreparePaint( SvLBoxEntry* _pEntry )
{
    Wallpaper aEntryBackground( m_aOriginalBackgroundColor );
    if ( _pEntry )
    {
        if ( _pEntry == GetCurEntry() )
        {
            // draw a selection background
            bool bIsMouseDownEntry = ( _pEntry == m_pMouseDownEntry );
            DrawSelectionBackground( GetBoundingRect( _pEntry ), bIsMouseDownEntry ? 1 : 2, FALSE, TRUE, FALSE );

            if ( bIsMouseDownEntry )
            {
                Font aFont( GetFont() );
                aFont.SetColor( GetSettings().GetStyleSettings().GetHighlightTextColor() );
                Control::SetFont( aFont );
            }

            // and temporary set a transparent background, for all the other
            // paint operations the SvTreeListBox is going to do
            aEntryBackground = Wallpaper( Color( COL_TRANSPARENT ) );
        }
    }

    SetBackground( aEntryBackground );
}
// -----------------------------------------------------------------------------
Rectangle OCreationList::GetFocusRect( SvLBoxEntry* _pEntry, long _nLine )
{
    Rectangle aRect = SvTreeListBox::GetFocusRect( _pEntry, _nLine );
    aRect.Left() = 0;

    // try to let the focus rect start before the bitmap item - this looks better
    SvLBoxItem* pBitmapItem = _pEntry->GetFirstItem( SV_ITEM_ID_LBOXCONTEXTBMP );
    USHORT nTabPos = 0;
    SvLBoxTab* pTab = pBitmapItem ? GetTab( _pEntry, pBitmapItem ) : NULL;
    SvViewDataItem* pItemData = pBitmapItem ? GetViewDataItem( _pEntry, pBitmapItem ) : NULL;
    DBG_ASSERT( pTab && pItemData, "OCreationList::GetFocusRect: could not find the first bitmap item!" );
    if ( pTab && pItemData )
        aRect.Left() = pTab->GetPos() - pItemData->aSize.Width() / 2;

    // inflate the rectangle a little bit - looks better, too
    aRect.Left() = ::std::max< long >( 0, aRect.Left() - 2 );
    aRect.Right() = ::std::min< long >( GetOutputSizePixel().Width() - 1, aRect.Right() + 2 );

    return aRect;
}
// -----------------------------------------------------------------------------
void OCreationList::StartDrag( sal_Int8 _nAction, const Point& _rPosPixel )
{
    // don't give this to the base class, it does a ReleaseMouse as very first action
    // Though I think this is a bug (it should ReleaseMouse only if it is going to do
    // something with the drag-event), I hesitate to fix it in the current state,
    // since I don't overlook the consequences, and we're close to 2.0 ...)
}
// -----------------------------------------------------------------------------
void OCreationList::ModelHasCleared()
{
    SvTreeListBox::ModelHasCleared();
    m_pLastActiveEntry = NULL;
    m_pMouseDownEntry = NULL;
}
// -----------------------------------------------------------------------------
void OCreationList::GetFocus()
{
    SvTreeListBox::GetFocus();
    if ( !GetCurEntry() )
        setCurrentEntryInvalidate( m_pLastActiveEntry ? m_pLastActiveEntry : GetFirstEntryInView() );
}
// -----------------------------------------------------------------------------
void OCreationList::LoseFocus()
{
    SvTreeListBox::LoseFocus();
    m_pLastActiveEntry = GetCurEntry();
    setCurrentEntryInvalidate( NULL );
}
// -----------------------------------------------------------------------------
void OCreationList::MouseButtonDown( const MouseEvent& rMEvt )
{
    SvTreeListBox::MouseButtonDown( rMEvt );

    DBG_ASSERT( !m_pMouseDownEntry, "OCreationList::MouseButtonDown: I missed some mouse event!" );
    m_pMouseDownEntry = GetCurEntry();
    if ( m_pMouseDownEntry )
    {
        InvalidateEntry( m_pMouseDownEntry );
        CaptureMouse();
    }
}
// -----------------------------------------------------------------------------
void OCreationList::MouseMove( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeaveWindow() )
    {
        setCurrentEntryInvalidate( NULL );
    }
    else if ( !rMEvt.IsSynthetic() )
    {
        SvLBoxEntry* pEntry = GetEntry( rMEvt.GetPosPixel() );

        if ( m_pMouseDownEntry )
        {
            // we're currently in a "mouse down" phase
            DBG_ASSERT( IsMouseCaptured(), "OCreationList::MouseMove: inconsistence (1)!" );
            if ( pEntry == m_pMouseDownEntry )
            {
                setCurrentEntryInvalidate( m_pMouseDownEntry );
            }
            else
            {
                DBG_ASSERT( ( GetCurEntry() == m_pMouseDownEntry ) || !GetCurEntry(),
                    "OCreationList::MouseMove: inconsistence (2)!" );
                setCurrentEntryInvalidate( NULL );
            }
        }
        else
        {
            // the user is simply hovering with the mouse
            if ( setCurrentEntryInvalidate( pEntry ) )
            {
                if ( !m_pMouseDownEntry )
                    updateHelpText();
            }
        }
    }

    SvTreeListBox::MouseMove(rMEvt);
}
// -----------------------------------------------------------------------------
void OCreationList::MouseButtonUp( const MouseEvent& rMEvt )
{
    SvLBoxEntry* pEntry = GetEntry( rMEvt.GetPosPixel() );
    bool bExecute = false;
    // Was the mouse released over the active entry?
    // (i.e. the entry which was under the mouse when the button went down)
    if ( pEntry && ( m_pMouseDownEntry == pEntry ) )
    {
        if ( !rMEvt.IsShift() && !rMEvt.IsMod1() && !rMEvt.IsMod2() && rMEvt.IsLeft() && rMEvt.GetClicks() == 1 )
            bExecute = true;
    }

    if ( m_pMouseDownEntry )
    {
        DBG_ASSERT( IsMouseCaptured(), "OCreationList::MouseButtonUp: hmmm .... no mouse captured, but an active entry?" );
        ReleaseMouse();

        InvalidateEntry( m_pMouseDownEntry );
        m_pMouseDownEntry = NULL;
    }

    SvTreeListBox::MouseButtonUp( rMEvt );

    if ( bExecute )
        onSelected( pEntry );
}
// -----------------------------------------------------------------------------
bool OCreationList::setCurrentEntryInvalidate( SvLBoxEntry* _pEntry )
{
    if ( GetCurEntry() != _pEntry )
    {
        if ( GetCurEntry() )
            InvalidateEntry( GetCurEntry() );
        SetCurEntry( _pEntry );
        if ( GetCurEntry() )
            InvalidateEntry( GetCurEntry() );
        return true;
    }
    return false;
}
// -----------------------------------------------------------------------------
void OCreationList::updateHelpText()
{
    USHORT nHelpTextId = 0;
    if ( GetCurEntry() )
        nHelpTextId = reinterpret_cast< TResourcePair* >( GetCurEntry()->GetUserData() )->second;
    m_pTaskWindow->setHelpText( nHelpTextId );
}
// -----------------------------------------------------------------------------
void OCreationList::onSelected( SvLBoxEntry* _pEntry ) const
{
    DBG_ASSERT( _pEntry, "OCreationList::onSelected: invalid entry!" );
    m_pTaskWindow->getDetailView()->onCreationClick( reinterpret_cast< TResourcePair* >( _pEntry->GetUserData() )->first );
}
// -----------------------------------------------------------------------------
void OCreationList::KeyInput( const KeyEvent& rKEvt )
{
    const KeyCode& rCode = rKEvt.GetKeyCode();
    if ( !rCode.IsMod1() && !rCode.IsMod2() && !rCode.IsShift() )
    {
        if ( rCode.GetCode() == KEY_RETURN )
        {
            SvLBoxEntry* pEntry = GetCurEntry() ? GetCurEntry() : FirstSelected();
            if ( pEntry )
                onSelected( pEntry );
            return;
        }
    }
    SvLBoxEntry* pOldCurrent = GetCurEntry();
    SvTreeListBox::KeyInput(rKEvt);
    SvLBoxEntry* pNewCurrent = GetCurEntry();

    if ( pOldCurrent != pNewCurrent )
    {
        if ( pOldCurrent )
            InvalidateEntry( pOldCurrent );
        if ( pNewCurrent )
            InvalidateEntry( pNewCurrent );
        updateHelpText();
    }
}
// -----------------------------------------------------------------------------
DBG_NAME(OTasksWindow)
OTasksWindow::OTasksWindow(Window* _pParent,OApplicationDetailView* _pDetailView)
    : Window(_pParent,WB_DIALOGCONTROL )
    ,m_aCreation(this)
    ,m_aDescription(this)
    ,m_aHelpText(this,WB_WORDBREAK)
    ,m_aFL(this,WB_VERT)
    ,m_pDetailView(_pDetailView)
{
    DBG_CTOR(OTasksWindow,NULL);
    SetUniqueId(UID_APP_TASKS_WINDOW);
    m_aCreation.SetHelpId(HID_APP_CREATION_LIST);
    m_aCreation.SetSelectHdl(LINK(this, OTasksWindow, OnEntrySelectHdl));
    m_aHelpText.SetHelpId(HID_APP_HELP_TEXT);
    m_aDescription.SetHelpId(HID_APP_DESCRIPTION_TEXT);
    m_aDescription.SetText(ModuleRes(STR_DESCRIPTION));
    Font aFont = m_aDescription.GetControlFont();
    aFont.SetWeight(WEIGHT_BOLD);
    m_aDescription.SetControlFont(aFont);
}
// -----------------------------------------------------------------------------
OTasksWindow::~OTasksWindow()
{
    DBG_DTOR(OTasksWindow,NULL);
    Clear();
}
// -----------------------------------------------------------------------------
void OTasksWindow::setHelpText(USHORT _nId)
{
    DBG_CHKTHIS(OTasksWindow,NULL);
    if ( _nId )
        m_aHelpText.SetText(ModuleRes(_nId));
    else
        m_aHelpText.SetText(String());
}
// -----------------------------------------------------------------------------
IMPL_LINK(OTasksWindow, OnEntrySelectHdl, SvTreeListBox*, _pTreeBox)
{
    DBG_CHKTHIS(OTasksWindow,NULL);
    SvLBoxEntry* pEntry = m_aCreation.GetHdlEntry();
    if ( pEntry )
        m_aHelpText.SetText(ModuleRes(reinterpret_cast<TResourcePair*>(pEntry->GetUserData())->second));
    return 1L;
}
// -----------------------------------------------------------------------------
void OTasksWindow::Resize()
{
    DBG_CHKTHIS(OTasksWindow,NULL);
    //////////////////////////////////////////////////////////////////////
    // Abmessungen parent window
    Size aOutputSize( GetOutputSize() );
    long nOutputWidth   = aOutputSize.Width();
    long nOutputHeight  = aOutputSize.Height();

    Size aFLSize = LogicToPixel( Size( 2, 6 ), MAP_APPFONT );
    sal_Int32 n6PPT = aFLSize.Height();
    long nHalfOutputWidth = static_cast<long>(nOutputWidth * 0.5);

    m_aCreation.SetPosSizePixel( Point(0, 0), Size(nHalfOutputWidth - n6PPT, nOutputHeight) );
    m_aDescription.SetPosSizePixel( Point(nHalfOutputWidth + n6PPT, 0), Size(nOutputWidth - nHalfOutputWidth - aFLSize.Width(), nOutputHeight) );
    Size aDesc = m_aDescription.CalcMinimumSize();
    m_aHelpText.SetPosSizePixel( Point(nHalfOutputWidth + n6PPT, aDesc.Height() ), Size(nOutputWidth - nHalfOutputWidth - aFLSize.Width(), nOutputHeight - aDesc.Height() - n6PPT) );

    m_aFL.SetPosSizePixel( Point(nHalfOutputWidth , 0), Size(aFLSize.Width(), nOutputHeight ) );
}
// -----------------------------------------------------------------------------
void OTasksWindow::fillCreationNew( const TResourceStruct& _rList )
{
    DBG_CHKTHIS(OTasksWindow,NULL);
    Clear();

    try
    {
        Reference<XModuleUIConfigurationManagerSupplier> xModuleCfgMgrSupplier(getDetailView()->getBorderWin()->getView()->getORB()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ui.ModuleUIConfigurationManagerSupplier"))),UNO_QUERY);
        Reference<XUIConfigurationManager> xUIConfigMgr = xModuleCfgMgrSupplier->getUIConfigurationManager(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.OfficeDatabaseDocument")));
        Reference<XImageManager> xImageMgr(xUIConfigMgr->getImageManager(),UNO_QUERY);

        m_aHelpTextIds.reserve(_rList.size());
        Sequence< ::rtl::OUString> aSeq(_rList.size());
        sal_Int32 i = 0;
        TResourceStruct::const_iterator aEnd = _rList.end();
        for (TResourceStruct::const_iterator aIter = _rList.begin(); aIter != aEnd; ++aIter,++i)
        {
            aSeq[i] = aIter->second.first;
        }

        i = 0;

        Sequence< Reference< XGraphic> > aImages = xImageMgr->getImages( ImageType::SIZE_DEFAULT | ImageType::COLOR_NORMAL, aSeq );
        Sequence< Reference< XGraphic> > aHCImages = xImageMgr->getImages( ImageType::SIZE_DEFAULT | ImageType::COLOR_HIGHCONTRAST, aSeq );

        for (TResourceStruct::const_iterator aIter = _rList.begin(); aIter != aEnd; ++aIter,++i)
        {
            SvLBoxEntry* pEntry = m_aCreation.InsertEntry( aIter->first );
            pEntry->SetUserData(reinterpret_cast<void*>(new TResourcePair(aIter->second)));

            Image aImage = Image( aImages[i] );
            m_aCreation.SetExpandedEntryBmp( pEntry, aImage, BMP_COLOR_NORMAL );
            m_aCreation.SetCollapsedEntryBmp( pEntry, aImage, BMP_COLOR_NORMAL );

            Image aHCImage = Image( aHCImages[i] );
            m_aCreation.SetExpandedEntryBmp( pEntry, aHCImage, BMP_COLOR_HIGHCONTRAST );
            m_aCreation.SetCollapsedEntryBmp( pEntry, aHCImage, BMP_COLOR_HIGHCONTRAST );
        }
    }
    catch(Exception&)
    {
    }

    m_aCreation.Show();
    m_aCreation.SelectAll(FALSE);
    m_aHelpText.Show();
    m_aDescription.Show();
    m_aFL.Show();
    m_aCreation.updateHelpText();
}
// -----------------------------------------------------------------------------
void OTasksWindow::Clear()
{
    DBG_CHKTHIS(OTasksWindow,NULL);
    m_aCreation.resetLastActive();
    SvLBoxEntry* pEntry = m_aCreation.First();
    while ( pEntry )
    {
        delete reinterpret_cast<TResourcePair*>(pEntry->GetUserData());
        pEntry = m_aCreation.Next(pEntry);
    }
    m_aCreation.Clear();
}
//==================================================================
// class OApplicationDetailView
//==================================================================
DBG_NAME(OApplicationDetailView)
OApplicationDetailView::OApplicationDetailView(OAppBorderWindow* _pParent) : OSplitterView(_pParent,sal_False )
    ,m_aHorzSplitter(this)
    ,m_aTasks(this,STR_TASKS,WB_BORDER | WB_DIALOGCONTROL )
    ,m_aContainer(this,0,WB_BORDER | WB_DIALOGCONTROL )
    ,m_pBorderWin(_pParent)
{
    DBG_CTOR(OApplicationDetailView,NULL);
    SetUniqueId(UID_APP_DETAIL_VIEW);
    ImplInitSettings( sal_True, sal_True, sal_True );

    m_pControlHelper = new OAppDetailPageHelper(&m_aContainer,_pParent);
    m_pControlHelper->Show();
    m_aContainer.setChildWindow(m_pControlHelper);

    OTasksWindow* pTasks = new OTasksWindow(&m_aTasks,this);
    pTasks->Show();
    pTasks->Disable(_pParent->getView()->getCommandController()->isDataSourceReadOnly());
    m_aTasks.setChildWindow(pTasks);
    m_aTasks.SetUniqueId(UID_APP_TASKS_VIEW);
    m_aTasks.Show();

    m_aContainer.SetUniqueId(UID_APP_CONTAINER_VIEW);
    m_aContainer.Show();

    const long  nFrameWidth = LogicToPixel( Size( 3, 0 ), MAP_APPFONT ).Width();
    m_aHorzSplitter.SetPosSizePixel( Point(0,50), Size(0,nFrameWidth) );
    // now set the components at the base class
    set(&m_aContainer,&m_aTasks);

    m_aHorzSplitter.Show();
    m_aHorzSplitter.SetUniqueId(UID_APP_VIEW_HORZ_SPLIT);
    setSplitter(&m_aHorzSplitter);
}
// -----------------------------------------------------------------------------
OApplicationDetailView::~OApplicationDetailView()
{
    DBG_DTOR(OApplicationDetailView,NULL);
    set(NULL,NULL);
    setSplitter(NULL);
    m_pControlHelper = NULL;
}
//  -----------------------------------------------------------------------------
void OApplicationDetailView::ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground )
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetDialogColor() ) );
    m_aHorzSplitter.SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetDialogColor() ) );
    m_aHorzSplitter.SetFillColor( GetSettings().GetStyleSettings().GetDialogColor() );
    m_aHorzSplitter.SetTextFillColor( GetSettings().GetStyleSettings().GetDialogColor() );
}
// -----------------------------------------------------------------------
void OApplicationDetailView::DataChanged( const DataChangedEvent& rDCEvt )
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    OSplitterView::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings( sal_True, sal_True, sal_True );
        Invalidate();
    }
}
// -----------------------------------------------------------------------------
void OApplicationDetailView::GetFocus()
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    OSplitterView::GetFocus();
}
// -----------------------------------------------------------------------------
void OApplicationDetailView::createTablesPage(const Reference< XConnection>& _xConnection)
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    TResourceStruct aList;
    aList.reserve(4);
    aList.push_back( TResourceStruct::value_type(ModuleRes(RID_STR_NEW_TABLE),TResourcePair(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewTable")),RID_STR_TABLES_HELP_TEXT_DESIGN)));
    aList.push_back( TResourceStruct::value_type(ModuleRes(RID_STR_NEW_TABLE_AUTO),TResourcePair(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewTableAutoPilot")),RID_STR_TABLES_HELP_TEXT_WIZARD)));

    ::com::sun::star::util::URL aUrl;
    aUrl.Complete = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewView"));
    if ( getBorderWin()->getView()->getCommandController()->isCommandEnabled(aUrl) )
        aList.push_back( TResourceStruct::value_type(ModuleRes(RID_STR_NEW_VIEW),TResourcePair(aUrl.Complete,RID_STR_VIEWS_HELP_TEXT_DESIGN)));
    //  aList.push_back( TResourceStruct::value_type(ModuleRes(RID_STR_NEW_VIEW_AUTO),TResourcePair(ID_NEW_VIEW_DESIGN_AUTO_PILOT,RID_STR_VIEWS_HELP_TEXT_WIZARD)));

    static_cast<OTasksWindow*>(m_aTasks.getChildWindow())->fillCreationNew( aList );
    static_cast<OTasksWindow*>(m_aTasks.getChildWindow())->Enable(static_cast<OAppBorderWindow*>(GetParent())->getView()->getCommandController()->isCommandEnabled(ID_NEW_TABLE_DESIGN));

    m_pControlHelper->createTablesPage(_xConnection);
    m_aContainer.setTitle(RID_STR_TABLES_CONTAINER);
    Resize();
}
// -----------------------------------------------------------------------------
void OApplicationDetailView::createPage(ElementType _eType,const Reference< XNameAccess >& _xContainer)
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    USHORT nTitleId = 0;
    TResourceStruct aList;
    aList.reserve(4);
    switch(_eType )
    {
        case E_FORM:
            {
                aList.push_back( TResourceStruct::value_type(ModuleRes(RID_STR_NEW_FORM),TResourcePair(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewForm")), RID_STR_FORMS_HELP_TEXT)));
                aList.push_back( TResourceStruct::value_type(ModuleRes(RID_STR_NEW_FORM_AUTO),TResourcePair(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewFormAutoPilot")),RID_STR_FORMS_HELP_TEXT_WIZARD)));

                nTitleId = RID_STR_FORMS_CONTAINER;
                static_cast<OTasksWindow*>(m_aTasks.getChildWindow())->Enable(static_cast<OAppBorderWindow*>(GetParent())->getView()->getCommandController()->isCommandEnabled(SID_APP_NEW_FORM));
            }
            break;
        case E_REPORT:
            {
                aList.push_back( TResourceStruct::value_type(ModuleRes(RID_STR_NEW_REPORT_AUTO),TResourcePair(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewReportAutoPilot")),RID_STR_REPORTS_HELP_TEXT_WIZARD)));

                nTitleId = RID_STR_REPORTS_CONTAINER;
                static_cast<OTasksWindow*>(m_aTasks.getChildWindow())->Enable(static_cast<OAppBorderWindow*>(GetParent())->getView()->getCommandController()->isCommandEnabled(ID_DOCUMENT_CREATE_REPWIZ));
            }
            break;
        case E_QUERY:
            {
                aList.push_back( TResourceStruct::value_type(ModuleRes(RID_STR_NEW_QUERY),TResourcePair(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewQuery")),RID_STR_QUERIES_HELP_TEXT)));
                aList.push_back( TResourceStruct::value_type(ModuleRes(RID_STR_NEW_QUERY_AUTO),TResourcePair(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewQueryAutoPilot")),RID_STR_QUERIES_HELP_TEXT_WIZARD)));
                aList.push_back( TResourceStruct::value_type(ModuleRes(RID_STR_NEW_QUERY_SQL),TResourcePair(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewQuerySql")),RID_STR_QUERIES_HELP_TEXT_SQL)));

                nTitleId = RID_STR_QUERIES_CONTAINER;
                static_cast<OTasksWindow*>(m_aTasks.getChildWindow())->Enable(static_cast<OAppBorderWindow*>(GetParent())->getView()->getCommandController()->isCommandEnabled(ID_NEW_QUERY_DESIGN));
            }
            break;
        default:
            OSL_ENSURE(0,"Illegal call!");
    }

    static_cast<OTasksWindow*>(m_aTasks.getChildWindow())->fillCreationNew( aList );
    m_pControlHelper->createPage(_eType,_xContainer);
    m_aContainer.setTitle(nTitleId);
    Resize();
}
// -----------------------------------------------------------------------------
::rtl::OUString OApplicationDetailView::getQualifiedName(SvLBoxEntry* _pEntry,const Reference< XDatabaseMetaData>& _xMetaData) const
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    return m_pControlHelper->getQualifiedName(_pEntry,_xMetaData);
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationDetailView::isLeaf(SvLBoxEntry* _pEntry) const
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    return m_pControlHelper->isLeaf(_pEntry);
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationDetailView::isALeafSelected() const
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    return m_pControlHelper->isALeafSelected();
}
// -----------------------------------------------------------------------------
void OApplicationDetailView::selectAll()
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    m_pControlHelper->selectAll();
}
// -----------------------------------------------------------------------------
void OApplicationDetailView::sortDown()
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    m_pControlHelper->sortDown();
}
// -----------------------------------------------------------------------------
void OApplicationDetailView::sortUp()
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    m_pControlHelper->sortUp();
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationDetailView::isFilled() const
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    return m_pControlHelper->isFilled();
}
// -----------------------------------------------------------------------------
ElementType OApplicationDetailView::getElementType() const
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    return m_pControlHelper->getElementType();
}
// -----------------------------------------------------------------------------
void OApplicationDetailView::clearPages(sal_Bool _bTaskAlso)
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    if ( _bTaskAlso )
        static_cast<OTasksWindow*>(m_aTasks.getChildWindow())->Clear();
    m_pControlHelper->clearPages();
}
// -----------------------------------------------------------------------------
sal_Int32 OApplicationDetailView::getSelectionCount()
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    return m_pControlHelper->getSelectionCount();
}
// -----------------------------------------------------------------------------
sal_Int32 OApplicationDetailView::getElementCount()
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    return m_pControlHelper->getElementCount();
}
// -----------------------------------------------------------------------------
void OApplicationDetailView::getSelectionElementNames(::std::vector< ::rtl::OUString>& _rNames,const Reference< XDatabaseMetaData>& _xMetaData) const
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    m_pControlHelper->getSelectionElementNames(_rNames,_xMetaData);
}
// -----------------------------------------------------------------------------
SvLBoxEntry* OApplicationDetailView::getEntry( const Point& _aPoint ) const
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    return m_pControlHelper->getEntry(_aPoint);
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationDetailView::isCutAllowed()
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    return m_pControlHelper->isCutAllowed();
}
sal_Bool OApplicationDetailView::isCopyAllowed()
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    return m_pControlHelper->isCopyAllowed();
}
sal_Bool OApplicationDetailView::isPasteAllowed()   { DBG_CHKTHIS(OApplicationDetailView,NULL);return m_pControlHelper->isPasteAllowed(); }
void OApplicationDetailView::copy() { DBG_CHKTHIS(OApplicationDetailView,NULL);m_pControlHelper->copy(); }
void OApplicationDetailView::cut()  { DBG_CHKTHIS(OApplicationDetailView,NULL);m_pControlHelper->cut(); }
void OApplicationDetailView::paste()
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    m_pControlHelper->paste();
}
// -----------------------------------------------------------------------------
void OApplicationDetailView::onCreationClick( const ::rtl::OUString& _sCommand)
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    static_cast<OAppBorderWindow*>(GetParent())->getView()->getElementNotification()->onCreationClick(_sCommand);
}
// -----------------------------------------------------------------------------
SvLBoxEntry*  OApplicationDetailView::elementAdded(ElementType _eType,const ::rtl::OUString& _rName, const Any& _rObject, const Reference< XConnection >& _rxConn )
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    return m_pControlHelper->elementAdded(_eType,_rName, _rObject, _rxConn );
}
// -----------------------------------------------------------------------------
void OApplicationDetailView::elementRemoved(ElementType _eType,const ::rtl::OUString& _rName, const Reference< XConnection >& _rxConn )
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    m_pControlHelper->elementRemoved(_eType,_rName, _rxConn );
}
// -----------------------------------------------------------------------------
void OApplicationDetailView::elementReplaced(ElementType _eType
                                                    ,const ::rtl::OUString& _rOldName
                                                    ,const ::rtl::OUString& _rNewName
                                                    ,const Reference< XConnection >& _rxConn )
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    m_pControlHelper->elementReplaced(_eType, _rOldName,_rNewName,_rxConn );
}
// -----------------------------------------------------------------------------
PreviewMode OApplicationDetailView::getPreviewMode()
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    return m_pControlHelper->getPreviewMode();
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationDetailView::isPreviewEnabled()
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    return m_pControlHelper->isPreviewEnabled();
}
// -----------------------------------------------------------------------------
void OApplicationDetailView::switchPreview()
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    m_pControlHelper->switchPreview();
}
// -----------------------------------------------------------------------------
void OApplicationDetailView::switchPreview(PreviewMode _eMode)
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    m_pControlHelper->switchPreview(_eMode);
}
// -----------------------------------------------------------------------------
void OApplicationDetailView::showPreview(const Reference< XContent >& _xContent)
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    m_pControlHelper->showPreview(_xContent);
}
// -----------------------------------------------------------------------------
void OApplicationDetailView::showPreview(   const ::rtl::OUString& _sDataSourceName,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
                                            const ::rtl::OUString& _sName,
                                            sal_Bool _bTable)
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    m_pControlHelper->showPreview(_sDataSourceName,_xConnection,_sName,_bTable);
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationDetailView::isSortUp() const
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    return m_pControlHelper->isSortUp();
}
// -----------------------------------------------------------------------------
void OApplicationDetailView::disableControls(sal_Bool _bDisable)
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    static_cast<OTasksWindow*>(m_aTasks.getChildWindow())->Enable(!_bDisable);
    m_pControlHelper->disableControls(_bDisable);
}
// -----------------------------------------------------------------------------
