/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "AppDetailView.hxx"
#include <osl/diagnose.h>
#include "dbaccess_helpid.hrc"
#include "dbu_app.hrc"
#include "AppView.hxx"
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/ModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/util/URL.hpp>
#include "listviewitems.hxx"
#include <vcl/image.hxx>
#include <vcl/mnemonic.hxx>
#include "browserids.hxx"
#include "AppDetailPageHelper.hxx"
#include <vcl/svapp.hxx>
#include "callbacks.hxx"
#include "IController.hxx"
#include "moduledbu.hxx"
#include <svtools/localresaccess.hxx>
#include <algorithm>
#include "dbtreelistbox.hxx"
#include "IApplicationController.hxx"
#include "imageprovider.hxx"
#include "comphelper/processfactory.hxx"

using namespace ::dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using ::com::sun::star::util::URL;
using ::com::sun::star::sdb::application::NamedDatabaseObject;

#define SPACEBETWEENENTRIES     4

// -----------------------------------------------------------------------------
TaskEntry::TaskEntry( const sal_Char* _pAsciiUNOCommand, sal_uInt16 _nHelpID, sal_uInt16 _nTitleResourceID, bool _bHideWhenDisabled )
    :sUNOCommand( ::rtl::OUString::createFromAscii( _pAsciiUNOCommand ) )
    ,nHelpID( _nHelpID )
    ,sTitle( ModuleRes( _nTitleResourceID ) )
    ,bHideWhenDisabled( _bHideWhenDisabled )
{
}

// -----------------------------------------------------------------------------
OCreationList::OCreationList( OTasksWindow& _rParent )
    :SvTreeListBox( &_rParent, WB_TABSTOP | WB_HASBUTTONSATROOT | WB_HASBUTTONS )
    ,m_rTaskWindow( _rParent )
    ,m_pMouseDownEntry( NULL )
    ,m_pLastActiveEntry( NULL )
{
    sal_uInt16 nSize = SPACEBETWEENENTRIES;
    SetSpaceBetweenEntries(nSize);
    SetSelectionMode( NO_SELECTION );
    SetExtendedWinBits( EWB_NO_AUTO_CURENTRY );
    SetNodeDefaultImages( );
    EnableEntryMnemonics();
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
            DrawSelectionBackground( GetBoundingRect( _pEntry ), bIsMouseDownEntry ? 1 : 2, sal_False, sal_True, sal_False );

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
void OCreationList::SelectSearchEntry( const void* _pEntry )
{
    SvLBoxEntry* pEntry = const_cast< SvLBoxEntry* >( static_cast< const SvLBoxEntry* >( _pEntry ) );
    OSL_ENSURE( pEntry, "OCreationList::SelectSearchEntry: invalid entry!" );

    if ( pEntry )
        setCurrentEntryInvalidate( pEntry );

    if ( !HasChildPathFocus() )
        GrabFocus();
}

// -----------------------------------------------------------------------------
void OCreationList::ExecuteSearchEntry( const void* _pEntry ) const
{
    SvLBoxEntry* pEntry = const_cast< SvLBoxEntry* >( static_cast< const SvLBoxEntry* >( _pEntry ) );
    OSL_ENSURE( pEntry, "OCreationList::ExecuteSearchEntry: invalid entry!" );
    OSL_ENSURE( pEntry == GetCurEntry(), "OCreationList::ExecuteSearchEntry: SelectSearchEntry should have been called before!" );

    if ( pEntry )
        onSelected( pEntry );
}

// -----------------------------------------------------------------------------
Rectangle OCreationList::GetFocusRect( SvLBoxEntry* _pEntry, long _nLine )
{
    Rectangle aRect = SvTreeListBox::GetFocusRect( _pEntry, _nLine );
    aRect.Left() = 0;

    // try to let the focus rect start before the bitmap item - this looks better
    SvLBoxItem* pBitmapItem = _pEntry->GetFirstItem( SV_ITEM_ID_LBOXCONTEXTBMP );
    SvLBoxTab* pTab = pBitmapItem ? GetTab( _pEntry, pBitmapItem ) : NULL;
    SvViewDataItem* pItemData = pBitmapItem ? GetViewDataItem( _pEntry, pBitmapItem ) : NULL;
    OSL_ENSURE( pTab && pItemData, "OCreationList::GetFocusRect: could not find the first bitmap item!" );
    if ( pTab && pItemData )
        aRect.Left() = pTab->GetPos() - pItemData->aSize.Width() / 2;

    // inflate the rectangle a little bit - looks better, too
    aRect.Left() = ::std::max< long >( 0, aRect.Left() - 2 );
    aRect.Right() = ::std::min< long >( GetOutputSizePixel().Width() - 1, aRect.Right() + 2 );

    return aRect;
}
// -----------------------------------------------------------------------------
void OCreationList::StartDrag( sal_Int8 /*_nAction*/, const Point& /*_rPosPixel*/ )
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

    OSL_ENSURE( !m_pMouseDownEntry, "OCreationList::MouseButtonDown: I missed some mouse event!" );
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
            OSL_ENSURE( IsMouseCaptured(), "OCreationList::MouseMove: inconsistence (1)!" );
            if ( pEntry == m_pMouseDownEntry )
            {
                setCurrentEntryInvalidate( m_pMouseDownEntry );
            }
            else
            {
                OSL_ENSURE( ( GetCurEntry() == m_pMouseDownEntry ) || !GetCurEntry(),
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
        OSL_ENSURE( IsMouseCaptured(), "OCreationList::MouseButtonUp: hmmm .... no mouse captured, but an active entry?" );
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
        {
            InvalidateEntry( GetCurEntry() );
            CallEventListeners( VCLEVENT_LISTBOX_SELECT, GetCurEntry() );
        }
        updateHelpText();
        return true;
    }
    return false;
}
// -----------------------------------------------------------------------------
void OCreationList::updateHelpText()
{
    sal_uInt16 nHelpTextId = 0;
    if ( GetCurEntry() )
        nHelpTextId = reinterpret_cast< TaskEntry* >( GetCurEntry()->GetUserData() )->nHelpID;
    m_rTaskWindow.setHelpText( nHelpTextId );
}
// -----------------------------------------------------------------------------
void OCreationList::onSelected( SvLBoxEntry* _pEntry ) const
{
    OSL_ENSURE( _pEntry, "OCreationList::onSelected: invalid entry!" );
    URL aCommand;
    aCommand.Complete = reinterpret_cast< TaskEntry* >( _pEntry->GetUserData() )->sUNOCommand;
    m_rTaskWindow.getDetailView()->getBorderWin().getView()->getAppController().executeChecked( aCommand, Sequence< PropertyValue >() );
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
        {
            InvalidateEntry( pNewCurrent );
            CallEventListeners( VCLEVENT_LISTBOX_SELECT, pNewCurrent );
        }
        updateHelpText();
    }
}
// -----------------------------------------------------------------------------
DBG_NAME(OTasksWindow)
OTasksWindow::OTasksWindow(Window* _pParent,OApplicationDetailView* _pDetailView)
    : Window(_pParent,WB_DIALOGCONTROL )
    ,m_aCreation(*this)
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

    ImageProvider aImageProvider;
    Image aFolderImage = aImageProvider.getFolderImage( DatabaseObject::FORM );
    m_aCreation.SetDefaultCollapsedEntryBmp( aFolderImage );
    m_aCreation.SetDefaultExpandedEntryBmp( aFolderImage );

    ImplInitSettings(sal_True,sal_True,sal_True);
}
// -----------------------------------------------------------------------------
OTasksWindow::~OTasksWindow()
{
    DBG_DTOR(OTasksWindow,NULL);
    Clear();
}
// -----------------------------------------------------------------------
void OTasksWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    DBG_CHKTHIS(OTasksWindow,NULL);
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings( sal_True, sal_True, sal_True );
        Invalidate();
    }
}
//  -----------------------------------------------------------------------------
void OTasksWindow::ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground )
{
    DBG_CHKTHIS(OTasksWindow,NULL);
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    if( bFont )
    {
        Font aFont;
        aFont = rStyleSettings.GetFieldFont();
        aFont.SetColor( rStyleSettings.GetWindowTextColor() );
        SetPointFont( aFont );
    }

    if( bForeground || bFont )
    {
        SetTextColor( rStyleSettings.GetFieldTextColor() );
        SetTextFillColor();
        m_aHelpText.SetTextColor( rStyleSettings.GetFieldTextColor() );
        m_aHelpText.SetTextFillColor();
        m_aDescription.SetTextColor( rStyleSettings.GetFieldTextColor() );
        m_aDescription.SetTextFillColor();
    }

    if( bBackground )
    {
        SetBackground( rStyleSettings.GetFieldColor() );
        m_aHelpText.SetBackground( rStyleSettings.GetFieldColor() );
        m_aDescription.SetBackground( rStyleSettings.GetFieldColor() );
        m_aFL.SetBackground( rStyleSettings.GetFieldColor() );
    }

    Font aFont = m_aDescription.GetControlFont();
    aFont.SetWeight(WEIGHT_BOLD);
    m_aDescription.SetControlFont(aFont);
}
// -----------------------------------------------------------------------------
void OTasksWindow::setHelpText(sal_uInt16 _nId)
{
    DBG_CHKTHIS(OTasksWindow,NULL);
    if ( _nId )
    {
        String sText = ModuleRes(_nId);

        m_aHelpText.SetText(sText);
    }
    else
    {
        m_aHelpText.SetText(String());
}
}
// -----------------------------------------------------------------------------
IMPL_LINK(OTasksWindow, OnEntrySelectHdl, SvTreeListBox*, /*_pTreeBox*/)
{
    DBG_CHKTHIS(OTasksWindow,NULL);
    SvLBoxEntry* pEntry = m_aCreation.GetHdlEntry();
    if ( pEntry )
        m_aHelpText.SetText( ModuleRes( reinterpret_cast< TaskEntry* >( pEntry->GetUserData() )->nHelpID ) );
    return 1L;
}
// -----------------------------------------------------------------------------
void OTasksWindow::Resize()
{
    DBG_CHKTHIS(OTasksWindow,NULL);

    // parent window dimension
    Size aOutputSize( GetOutputSize() );
    long nOutputWidth   = aOutputSize.Width();
    long nOutputHeight  = aOutputSize.Height();

    Size aFLSize = LogicToPixel( Size( 2, 6 ), MAP_APPFONT );
    sal_Int32 n6PPT = aFLSize.Height();
    long nHalfOutputWidth = static_cast<long>(nOutputWidth * 0.5);

    m_aCreation.SetPosSizePixel( Point(0, 0), Size(nHalfOutputWidth - n6PPT, nOutputHeight) );
    // i77897 make the m_aHelpText a little bit smaller. (-5)
    sal_Int32 nNewWidth = nOutputWidth - nHalfOutputWidth - aFLSize.Width() - 5;
    m_aDescription.SetPosSizePixel( Point(nHalfOutputWidth + n6PPT, 0), Size(nNewWidth, nOutputHeight) );
    Size aDesc = m_aDescription.CalcMinimumSize();
    m_aHelpText.SetPosSizePixel( Point(nHalfOutputWidth + n6PPT, aDesc.Height() ), Size(nNewWidth, nOutputHeight - aDesc.Height() - n6PPT) );

    m_aFL.SetPosSizePixel( Point(nHalfOutputWidth , 0), Size(aFLSize.Width(), nOutputHeight ) );
}
// -----------------------------------------------------------------------------
void OTasksWindow::fillTaskEntryList( const TaskEntryList& _rList )
{
    DBG_CHKTHIS(OTasksWindow,NULL);
    Clear();

    try
    {
        Reference< XModuleUIConfigurationManagerSupplier > xModuleCfgMgrSupplier(
            ModuleUIConfigurationManagerSupplier::create(comphelper::getComponentContext(getDetailView()->getBorderWin().getView()->getORB())) );
        Reference< XUIConfigurationManager > xUIConfigMgr = xModuleCfgMgrSupplier->getUIConfigurationManager(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.OfficeDatabaseDocument" ) )
        );
        Reference< XImageManager > xImageMgr( xUIConfigMgr->getImageManager(), UNO_QUERY );

        // copy the commands so we can use them with the config managers
        Sequence< ::rtl::OUString > aCommands( _rList.size() );
        ::rtl::OUString* pCommands = aCommands.getArray();
        TaskEntryList::const_iterator aEnd = _rList.end();
        for ( TaskEntryList::const_iterator pCopyTask = _rList.begin(); pCopyTask != aEnd; ++pCopyTask, ++pCommands )
            *pCommands = pCopyTask->sUNOCommand;

        Sequence< Reference< XGraphic> > aImages = xImageMgr->getImages(
            ImageType::SIZE_DEFAULT | ImageType::COLOR_NORMAL ,
            aCommands
        );

        const Reference< XGraphic >* pImages( aImages.getConstArray() );

        for ( TaskEntryList::const_iterator pTask = _rList.begin(); pTask != aEnd; ++pTask, ++pImages )
        {
            SvLBoxEntry* pEntry = m_aCreation.InsertEntry( pTask->sTitle );
            pEntry->SetUserData( reinterpret_cast< void* >( new TaskEntry( *pTask ) ) );

            Image aImage = Image( *pImages );
            m_aCreation.SetExpandedEntryBmp(  pEntry, aImage );
            m_aCreation.SetCollapsedEntryBmp( pEntry, aImage );
        }
    }
    catch(Exception&)
    {
    }

    m_aCreation.Show();
    m_aCreation.SelectAll(sal_False);
    m_aHelpText.Show();
    m_aDescription.Show();
    m_aFL.Show();
    m_aCreation.updateHelpText();
    Enable(!_rList.empty());
}
// -----------------------------------------------------------------------------
void OTasksWindow::Clear()
{
    DBG_CHKTHIS(OTasksWindow,NULL);
    m_aCreation.resetLastActive();
    SvLBoxEntry* pEntry = m_aCreation.First();
    while ( pEntry )
    {
        delete reinterpret_cast< TaskEntry* >( pEntry->GetUserData() );
        pEntry = m_aCreation.Next(pEntry);
    }
    m_aCreation.Clear();
}
//==================================================================
// class OApplicationDetailView
//==================================================================
DBG_NAME(OApplicationDetailView)
OApplicationDetailView::OApplicationDetailView(OAppBorderWindow& _rParent,PreviewMode _ePreviewMode) : OSplitterView(&_rParent,sal_False )
    ,m_aHorzSplitter(this)
    ,m_aTasks(this,STR_TASKS,WB_BORDER | WB_DIALOGCONTROL )
    ,m_aContainer(this,0,WB_BORDER | WB_DIALOGCONTROL )
    ,m_rBorderWin(_rParent)
{
    DBG_CTOR(OApplicationDetailView,NULL);
    SetUniqueId(UID_APP_DETAIL_VIEW);
    ImplInitSettings( sal_True, sal_True, sal_True );

    m_pControlHelper = new OAppDetailPageHelper(&m_aContainer,m_rBorderWin,_ePreviewMode);
    m_pControlHelper->Show();
    m_aContainer.setChildWindow(m_pControlHelper);

    OTasksWindow* pTasks = new OTasksWindow(&m_aTasks,this);
    pTasks->Show();
    pTasks->Disable(m_rBorderWin.getView()->getCommandController().isDataSourceReadOnly());
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
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    if( bFont )
    {
        Font aFont;
        aFont = rStyleSettings.GetFieldFont();
        aFont.SetColor( rStyleSettings.GetWindowTextColor() );
        SetPointFont( aFont );
    }

    if( bForeground || bFont )
    {
        SetTextColor( rStyleSettings.GetFieldTextColor() );
        SetTextFillColor();
    }

    if( bBackground )
        SetBackground( rStyleSettings.GetFieldColor() );

    m_aHorzSplitter.SetBackground( rStyleSettings.GetDialogColor() );
    m_aHorzSplitter.SetFillColor( rStyleSettings.GetDialogColor() );
    m_aHorzSplitter.SetTextFillColor(rStyleSettings.GetDialogColor() );
}
// -----------------------------------------------------------------------
void OApplicationDetailView::DataChanged( const DataChangedEvent& rDCEvt )
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    OSplitterView::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
        (rDCEvt.GetType() == DATACHANGED_DISPLAY) ||
        (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
        ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
        (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
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
void OApplicationDetailView::setTaskExternalMnemonics( MnemonicGenerator& _rMnemonics )
{
    m_aExternalMnemonics = _rMnemonics;
}

// -----------------------------------------------------------------------------
bool OApplicationDetailView::interceptKeyInput( const KeyEvent& _rEvent )
{
    const KeyCode& rKeyCode = _rEvent.GetKeyCode();
    if ( rKeyCode.GetModifier() == KEY_MOD2 )
        return getTasksWindow().HandleKeyInput( _rEvent );

    // not handled
    return false;
}

// -----------------------------------------------------------------------------
void OApplicationDetailView::createTablesPage(const Reference< XConnection >& _xConnection )
{
    impl_createPage( E_TABLE, _xConnection, NULL );
}

// -----------------------------------------------------------------------------
void OApplicationDetailView::createPage( ElementType _eType,const Reference< XNameAccess >& _xContainer )
{
    impl_createPage( _eType, NULL, _xContainer );
}

// -----------------------------------------------------------------------------
void OApplicationDetailView::impl_createPage( ElementType _eType, const Reference< XConnection >& _rxConnection,
    const Reference< XNameAccess >& _rxNonTableElements )
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);

    // get the data for the pane
    const TaskPaneData& rData = impl_getTaskPaneData( _eType );
    getTasksWindow().fillTaskEntryList( rData.aTasks );

    // enable the pane as a whole, depending on the availability of the first command
    OSL_ENSURE( !rData.aTasks.empty(), "OApplicationDetailView::impl_createPage: no tasks at all!?" );
    bool bEnabled = rData.aTasks.empty()
                ?   false
                :   getBorderWin().getView()->getCommandController().isCommandEnabled( rData.aTasks[0].sUNOCommand );
    getTasksWindow().Enable( bEnabled );
    m_aContainer.setTitle( rData.nTitleId );

    // let our helper create the object list
    if ( _eType == E_TABLE )
        m_pControlHelper->createTablesPage( _rxConnection );
    else
        m_pControlHelper->createPage( _eType, _rxNonTableElements );

    // resize for proper window arrangements
    Resize();
}

// -----------------------------------------------------------------------------
const TaskPaneData& OApplicationDetailView::impl_getTaskPaneData( ElementType _eType )
{
    if ( m_aTaskPaneData.empty() )
        m_aTaskPaneData.resize( ELEMENT_COUNT );
    OSL_ENSURE( ( _eType >= 0 ) && ( _eType < E_ELEMENT_TYPE_COUNT ), "OApplicationDetailView::impl_getTaskPaneData: illegal element type!" );
    TaskPaneData& rData = m_aTaskPaneData[ _eType ];

    //oj: do not check, otherwise extensions will only be visible after a reload.
    impl_fillTaskPaneData( _eType, rData );

    return rData;
}

// -----------------------------------------------------------------------------
void OApplicationDetailView::impl_fillTaskPaneData( ElementType _eType, TaskPaneData& _rData ) const
{
    TaskEntryList& rList( _rData.aTasks );
    rList.clear(); rList.reserve( 4 );

    switch ( _eType )
    {
    case E_TABLE:
        rList.push_back( TaskEntry( ".uno:DBNewTable", RID_STR_TABLES_HELP_TEXT_DESIGN, RID_STR_NEW_TABLE ) );
        rList.push_back( TaskEntry( ".uno:DBNewTableAutoPilot", RID_STR_TABLES_HELP_TEXT_WIZARD, RID_STR_NEW_TABLE_AUTO ) );
        rList.push_back( TaskEntry( ".uno:DBNewView", RID_STR_VIEWS_HELP_TEXT_DESIGN, RID_STR_NEW_VIEW, true ) );
        _rData.nTitleId = RID_STR_TABLES_CONTAINER;
        break;

    case E_FORM:
        rList.push_back( TaskEntry( ".uno:DBNewForm", RID_STR_FORMS_HELP_TEXT, RID_STR_NEW_FORM ) );
        rList.push_back( TaskEntry( ".uno:DBNewFormAutoPilot", RID_STR_FORMS_HELP_TEXT_WIZARD, RID_STR_NEW_FORM_AUTO ) );
        _rData.nTitleId = RID_STR_FORMS_CONTAINER;
        break;

    case E_REPORT:
        rList.push_back( TaskEntry( ".uno:DBNewReport", RID_STR_REPORT_HELP_TEXT, RID_STR_NEW_REPORT, true ) );
        rList.push_back( TaskEntry( ".uno:DBNewReportAutoPilot", RID_STR_REPORTS_HELP_TEXT_WIZARD, RID_STR_NEW_REPORT_AUTO ) );
        _rData.nTitleId = RID_STR_REPORTS_CONTAINER;
        break;

    case E_QUERY:
        rList.push_back( TaskEntry( ".uno:DBNewQuery", RID_STR_QUERIES_HELP_TEXT, RID_STR_NEW_QUERY ) );
        rList.push_back( TaskEntry( ".uno:DBNewQueryAutoPilot", RID_STR_QUERIES_HELP_TEXT_WIZARD, RID_STR_NEW_QUERY_AUTO ) );
        rList.push_back( TaskEntry( ".uno:DBNewQuerySql", RID_STR_QUERIES_HELP_TEXT_SQL, RID_STR_NEW_QUERY_SQL ) );
        _rData.nTitleId = RID_STR_QUERIES_CONTAINER;
        break;

    default:
        OSL_FAIL( "OApplicationDetailView::impl_fillTaskPaneData: illegal element type!" );
    }

    MnemonicGenerator aAllMnemonics( m_aExternalMnemonics );

    // remove the entries which are not enabled currently
    for (   TaskEntryList::iterator pTask = rList.begin();
            pTask != rList.end();
        )
    {
        if  (   pTask->bHideWhenDisabled
            &&  !getBorderWin().getView()->getCommandController().isCommandEnabled( pTask->sUNOCommand )
            )
            pTask = rList.erase( pTask );
        else
        {
            aAllMnemonics.RegisterMnemonic( pTask->sTitle );
            ++pTask;
        }
    }

    // for the remaining entries, assign mnemonics
    for (   TaskEntryList::iterator pTask = rList.begin();
            pTask != rList.end();
            ++pTask
        )
    {
        aAllMnemonics.CreateMnemonic( pTask->sTitle );
        // don't do this for now, until our task window really supports mnemonics
    }
}

// -----------------------------------------------------------------------------
::rtl::OUString OApplicationDetailView::getQualifiedName( SvLBoxEntry* _pEntry ) const
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    return m_pControlHelper->getQualifiedName( _pEntry );
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
        getTasksWindow().Clear();
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
void OApplicationDetailView::getSelectionElementNames( ::std::vector< ::rtl::OUString>& _rNames ) const
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    m_pControlHelper->getSelectionElementNames( _rNames );
}
// -----------------------------------------------------------------------------
void OApplicationDetailView::describeCurrentSelectionForControl( const Control& _rControl, Sequence< NamedDatabaseObject >& _out_rSelectedObjects )
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    m_pControlHelper->describeCurrentSelectionForControl( _rControl, _out_rSelectedObjects );
}
// -----------------------------------------------------------------------------
void OApplicationDetailView::describeCurrentSelectionForType( const ElementType _eType, Sequence< NamedDatabaseObject >& _out_rSelectedObjects )
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    m_pControlHelper->describeCurrentSelectionForType( _eType, _out_rSelectedObjects );
}
// -----------------------------------------------------------------------------
void OApplicationDetailView::selectElements(const Sequence< ::rtl::OUString>& _aNames)
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    m_pControlHelper->selectElements( _aNames );
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
SvLBoxEntry*  OApplicationDetailView::elementAdded(ElementType _eType,const ::rtl::OUString& _rName, const Any& _rObject )
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    return m_pControlHelper->elementAdded(_eType,_rName, _rObject );
}
// -----------------------------------------------------------------------------
void OApplicationDetailView::elementRemoved(ElementType _eType,const ::rtl::OUString& _rName )
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    m_pControlHelper->elementRemoved(_eType,_rName );
}
// -----------------------------------------------------------------------------
void OApplicationDetailView::elementReplaced(ElementType _eType
                                                    ,const ::rtl::OUString& _rOldName
                                                    ,const ::rtl::OUString& _rNewName )
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    m_pControlHelper->elementReplaced( _eType, _rOldName, _rNewName );
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
                                            const ::rtl::OUString& _sName,
                                            sal_Bool _bTable)
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    m_pControlHelper->showPreview(_sDataSourceName,_sName,_bTable);
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationDetailView::isSortUp() const
{
    DBG_CHKTHIS(OApplicationDetailView,NULL);
    return m_pControlHelper->isSortUp();
}
// -----------------------------------------------------------------------------
Window* OApplicationDetailView::getTreeWindow() const
{
    return m_pControlHelper->getCurrentView();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
