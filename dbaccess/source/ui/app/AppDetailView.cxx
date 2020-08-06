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
#include <helpids.h>
#include <strings.hrc>
#include "AppView.hxx"
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/util/URL.hpp>
#include <core_resource.hxx>
#include <vcl/event.hxx>
#include <vcl/image.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/settings.hxx>
#include "AppDetailPageHelper.hxx"
#include <dbaccess/IController.hxx>
#include <vcl/treelistentry.hxx>
#include <vcl/viewdataentry.hxx>
#include <algorithm>
#include <dbtreelistbox.hxx>
#include <imageprovider.hxx>
#include "AppController.hxx"

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

TaskEntry::TaskEntry( const char* _pAsciiUNOCommand, const char* _pHelpID, const char* pTitleResourceID )
    :sUNOCommand( OUString::createFromAscii( _pAsciiUNOCommand ) )
    ,pHelpID( _pHelpID )
    ,sTitle( DBA_RES(pTitleResourceID) )
{
}

OCreationList::OCreationList( OTasksWindow& _rParent )
    : InterimItemWindow(&_rParent, "dbaccess/ui/creationlistbox.ui", "CreationListBox")
    , m_xTreeView(m_xBuilder->weld_tree_view("treeview"))
    , m_rTaskWindow( _rParent )
    , m_nCursorIndex(-1)
{
    InitControlBase(m_xTreeView.get());

    m_xTreeView->set_help_id(HID_APP_CREATION_LIST);
    m_xTreeView->connect_row_activated(LINK(this, OCreationList, onSelected));
    m_xTreeView->connect_changed(LINK(this, OCreationList, OnEntrySelectHdl));
    m_xTreeView->connect_key_press(LINK(this, OCreationList, KeyInputHdl));
    m_xTreeView->connect_focus_in(LINK(this, OCreationList, FocusInHdl));
    m_xTreeView->connect_focus_out(LINK(this, OCreationList, FocusOutHdl));
}

IMPL_LINK(OCreationList, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return ChildKeyInput(rKEvt);
}

void OCreationList::dispose()
{
    m_xTreeView.reset();
    InterimItemWindow::dispose();
}

void OCreationList::updateHelpText()
{
    const char* pHelpTextId = nullptr;
    int nCurEntry = m_xTreeView->get_selected_index();
    if (nCurEntry != -1)
        pHelpTextId = reinterpret_cast<TaskEntry*>(m_xTreeView->get_id(nCurEntry).toUInt64())->pHelpID;
    m_rTaskWindow.setHelpText(pHelpTextId);
}

IMPL_LINK(OCreationList, onSelected, weld::TreeView&, rTreeView, bool)
{
    int nCurEntry = rTreeView.get_cursor_index();
    assert(nCurEntry != -1 && "OCreationList::onSelected: invalid entry!");
    URL aCommand;
    aCommand.Complete = reinterpret_cast<TaskEntry*>(rTreeView.get_id(nCurEntry).toUInt64())->sUNOCommand;
    m_rTaskWindow.getDetailView()->getBorderWin().getView()->getAppController().executeChecked( aCommand, Sequence< PropertyValue >() );

    return true;
}

void OCreationList::GetFocus()
{
    InterimItemWindow::GetFocus();
    if (!m_xTreeView)
        return;
    FocusInHdl(*m_xTreeView);
}

IMPL_LINK_NOARG(OCreationList, FocusInHdl, weld::Widget&, void)
{
    m_xTreeView->select(m_nCursorIndex != -1 ? m_nCursorIndex : 0);
}

IMPL_LINK_NOARG(OCreationList, FocusOutHdl, weld::Widget&, void)
{
    m_nCursorIndex = m_xTreeView->get_cursor_index();
    m_xTreeView->unselect_all();
}

IMPL_LINK_NOARG(OCreationList, OnEntrySelectHdl, weld::TreeView&, void)
{
    updateHelpText();
}

OTasksWindow::OTasksWindow(vcl::Window* _pParent,OApplicationDetailView* _pDetailView)
    : Window(_pParent,WB_DIALOGCONTROL )
    ,m_aCreation(VclPtr<OCreationList>::Create(*this))
    ,m_aDescription(VclPtr<FixedText>::Create(this))
    ,m_aFL(VclPtr<FixedLine>::Create(this,WB_VERT))
    ,m_aHelpText(VclPtr<FixedText>::Create(this,WB_WORDBREAK))
    ,m_pDetailView(_pDetailView)
{
    m_aHelpText->SetHelpId(HID_APP_HELP_TEXT);
    m_aDescription->SetHelpId(HID_APP_DESCRIPTION_TEXT);
    m_aDescription->SetText(DBA_RES(STR_DESCRIPTION));

    ImplInitSettings();
}

OTasksWindow::~OTasksWindow()
{
    disposeOnce();
}

void OTasksWindow::dispose()
{
    Clear();
    m_aCreation.disposeAndClear();
    m_aDescription.disposeAndClear();
    m_aHelpText.disposeAndClear();
    m_aFL.disposeAndClear();
    m_pDetailView.clear();
    vcl::Window::dispose();
}

void OTasksWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void OTasksWindow::ImplInitSettings()
{
    // FIXME RenderContext
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    vcl::Font aFont = rStyleSettings.GetFieldFont();
    aFont.SetColor( rStyleSettings.GetWindowTextColor() );
    SetPointFont(*this, aFont);

    SetTextColor( rStyleSettings.GetFieldTextColor() );
    SetTextFillColor();
    m_aHelpText->SetTextColor( rStyleSettings.GetFieldTextColor() );
    m_aHelpText->SetTextFillColor();
    m_aDescription->SetTextColor( rStyleSettings.GetFieldTextColor() );
    m_aDescription->SetTextFillColor();

    SetBackground( rStyleSettings.GetFieldColor() );
    m_aHelpText->SetBackground( rStyleSettings.GetFieldColor() );
    m_aDescription->SetBackground( rStyleSettings.GetFieldColor() );
    m_aFL->SetBackground( rStyleSettings.GetFieldColor() );

    aFont = m_aDescription->GetControlFont();
    aFont.SetWeight(WEIGHT_BOLD);
    m_aDescription->SetControlFont(aFont);
}

void OTasksWindow::setHelpText(const char* pId)
{
    if (pId)
        m_aHelpText->SetText(DBA_RES(pId));
    else
        m_aHelpText->SetText(OUString());
}

void OTasksWindow::Resize()
{
    // parent window dimension
    Size aOutputSize( GetOutputSize() );
    long nOutputWidth   = aOutputSize.Width();
    long nOutputHeight  = aOutputSize.Height();

    Size aFLSize = LogicToPixel(Size(2, 6), MapMode(MapUnit::MapAppFont));
    sal_Int32 n6PPT = aFLSize.Height();
    long nHalfOutputWidth = static_cast<long>(nOutputWidth * 0.5);

    m_aCreation->SetPosSizePixel( Point(0, 0), Size(nHalfOutputWidth - n6PPT, nOutputHeight) );
    // i77897 make the m_aHelpText a little bit smaller. (-5)
    sal_Int32 nNewWidth = nOutputWidth - nHalfOutputWidth - aFLSize.Width() - 5;
    m_aDescription->SetPosSizePixel( Point(nHalfOutputWidth + n6PPT, 0), Size(nNewWidth, nOutputHeight) );
    Size aDesc = m_aDescription->CalcMinimumSize();
    m_aHelpText->SetPosSizePixel( Point(nHalfOutputWidth + n6PPT, aDesc.Height() ), Size(nNewWidth, nOutputHeight - aDesc.Height() - n6PPT) );

    m_aFL->SetPosSizePixel( Point(nHalfOutputWidth , 0), Size(aFLSize.Width(), nOutputHeight ) );
}

void OTasksWindow::fillTaskEntryList( const TaskEntryList& _rList )
{
    Clear();

    try
    {
        Reference< XModuleUIConfigurationManagerSupplier > xModuleCfgMgrSupplier =
            theModuleUIConfigurationManagerSupplier::get( getDetailView()->getBorderWin().getView()->getORB() );
        Reference< XUIConfigurationManager > xUIConfigMgr = xModuleCfgMgrSupplier->getUIConfigurationManager(
            "com.sun.star.sdb.OfficeDatabaseDocument"
        );
        Reference< XImageManager > xImageMgr( xUIConfigMgr->getImageManager(), UNO_QUERY );

        // copy the commands so we can use them with the config managers
        Sequence< OUString > aCommands( _rList.size() );
        OUString* pCommands = aCommands.getArray();
        for (auto const& copyTask : _rList)
        {
            *pCommands = copyTask.sUNOCommand;
            ++pCommands;
        }

        Sequence< Reference< XGraphic> > aImages = xImageMgr->getImages(
            ImageType::SIZE_DEFAULT | ImageType::COLOR_NORMAL ,
            aCommands
        );

        const Reference< XGraphic >* pImages( aImages.getConstArray() );

        weld::TreeView& rTreeView = m_aCreation->get_widget();
        size_t nIndex = 0;
        for (auto const& task : _rList)
        {
            OUString sId = OUString::number(reinterpret_cast<sal_uInt64>(new TaskEntry(task)));
            rTreeView.append(sId, task.sTitle);
            rTreeView.set_image(nIndex++, *pImages++);
        }
    }
    catch(Exception&)
    {
    }

    m_aCreation->Show();
    m_aCreation->get_widget().unselect_all();
    m_aHelpText->Show();
    m_aDescription->Show();
    m_aFL->Show();
    m_aCreation->updateHelpText();
    Enable(!_rList.empty());
}

void OTasksWindow::Clear()
{
    weld::TreeView& rTreeView = m_aCreation->get_widget();
    rTreeView.all_foreach([&rTreeView](weld::TreeIter& rEntry){
        TaskEntry* pUserData = reinterpret_cast<TaskEntry*>(rTreeView.get_id(rEntry).toUInt64());
        delete pUserData;
        return false;
    });

    rTreeView.clear();
}

OApplicationDetailView::OApplicationDetailView(OAppBorderWindow& _rParent,PreviewMode _ePreviewMode) : OSplitterView(&_rParent )
    ,m_aHorzSplitter(VclPtr<Splitter>::Create(this))
    ,m_aTasks(VclPtr<dbaui::OTitleWindow>::Create(this, STR_TASKS, WB_BORDER | WB_DIALOGCONTROL))
    ,m_aContainer(VclPtr<dbaui::OTitleWindow>::Create(this, nullptr, WB_BORDER | WB_DIALOGCONTROL))
    ,m_rBorderWin(_rParent)
{
    ImplInitSettings();

    m_pControlHelper = VclPtr<OAppDetailPageHelper>::Create(m_aContainer.get(),m_rBorderWin,_ePreviewMode);
    m_pControlHelper->Show();
    m_aContainer->setChildWindow(m_pControlHelper);

    VclPtrInstance<OTasksWindow> pTasks(m_aTasks.get(),this);
    pTasks->Show();
    pTasks->Disable(m_rBorderWin.getView()->getCommandController().isDataSourceReadOnly());
    m_aTasks->setChildWindow(pTasks);
    m_aTasks->Show();

    m_aContainer->Show();

    const long  nFrameWidth = LogicToPixel(Size(3, 0), MapMode(MapUnit::MapAppFont)).Width();
    m_aHorzSplitter->SetPosSizePixel( Point(0,50), Size(0,nFrameWidth) );
    // now set the components at the base class
    set(m_aContainer.get(),m_aTasks.get());

    m_aHorzSplitter->Show();
    setSplitter(m_aHorzSplitter.get());
}

OApplicationDetailView::~OApplicationDetailView()
{
    disposeOnce();
}

void OApplicationDetailView::dispose()
{
    set(nullptr);
    setSplitter(nullptr);
    m_aHorzSplitter.disposeAndClear();
    m_aTasks.disposeAndClear();
    m_aContainer.disposeAndClear();
    m_pControlHelper.clear();
    OSplitterView::dispose();
}

void OApplicationDetailView::ImplInitSettings()
{
    // FIXME RenderContext
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    vcl::Font aFont = rStyleSettings.GetFieldFont();
    aFont.SetColor( rStyleSettings.GetWindowTextColor() );
    SetPointFont(*this, aFont);

    SetTextColor( rStyleSettings.GetFieldTextColor() );
    SetTextFillColor();

    SetBackground( rStyleSettings.GetFieldColor() );

    m_aHorzSplitter->SetBackground( rStyleSettings.GetDialogColor() );
    m_aHorzSplitter->SetFillColor( rStyleSettings.GetDialogColor() );
    m_aHorzSplitter->SetTextFillColor(rStyleSettings.GetDialogColor() );
}

void OApplicationDetailView::DataChanged( const DataChangedEvent& rDCEvt )
{
    OSplitterView::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
        (rDCEvt.GetType() == DataChangedEventType::DISPLAY) ||
        (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
        ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
        (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void OApplicationDetailView::setTaskExternalMnemonics( MnemonicGenerator const & _rMnemonics )
{
    m_aExternalMnemonics = _rMnemonics;
}

void OApplicationDetailView::createTablesPage(const Reference< XConnection >& _xConnection )
{
    impl_createPage( E_TABLE, _xConnection, nullptr );
}

void OApplicationDetailView::createPage( ElementType _eType,const Reference< XNameAccess >& _xContainer )
{
    impl_createPage( _eType, nullptr, _xContainer );
}

void OApplicationDetailView::impl_createPage( ElementType _eType, const Reference< XConnection >& _rxConnection,
    const Reference< XNameAccess >& _rxNonTableElements )
{
    // get the data for the pane
    const TaskPaneData& rData = impl_getTaskPaneData( _eType );
    getTasksWindow().fillTaskEntryList( rData.aTasks );

    // enable the pane as a whole, depending on the availability of the first command
    OSL_ENSURE( !rData.aTasks.empty(), "OApplicationDetailView::impl_createPage: no tasks at all!?" );
    bool bEnabled = !rData.aTasks.empty()
                && getBorderWin().getView()->getCommandController().isCommandEnabled( rData.aTasks[0].sUNOCommand );
    getTasksWindow().Enable( bEnabled );
    m_aContainer->setTitle(rData.pTitleId);

    // let our helper create the object list
    if ( _eType == E_TABLE )
        m_pControlHelper->createTablesPage( _rxConnection );
    else
        m_pControlHelper->createPage( _eType, _rxNonTableElements );

    // resize for proper window arrangements
    Resize();
}

namespace {

void impl_fillTaskPaneData(ElementType _eType, TaskPaneData& _rData)
{
    TaskEntryList& rList( _rData.aTasks );
    rList.clear(); rList.reserve( 4 );

    switch ( _eType )
    {
    case E_TABLE:
        rList.emplace_back( ".uno:DBNewTable", RID_STR_TABLES_HELP_TEXT_DESIGN, RID_STR_NEW_TABLE );
        rList.emplace_back( ".uno:DBNewTableAutoPilot", RID_STR_TABLES_HELP_TEXT_WIZARD, RID_STR_NEW_TABLE_AUTO );
        rList.emplace_back( ".uno:DBNewView", RID_STR_VIEWS_HELP_TEXT_DESIGN, RID_STR_NEW_VIEW );
        _rData.pTitleId = RID_STR_TABLES_CONTAINER;
        break;

    case E_FORM:
        rList.emplace_back( ".uno:DBNewForm", RID_STR_FORMS_HELP_TEXT, RID_STR_NEW_FORM );
        rList.emplace_back( ".uno:DBNewFormAutoPilot", RID_STR_FORMS_HELP_TEXT_WIZARD, RID_STR_NEW_FORM_AUTO );
        _rData.pTitleId = RID_STR_FORMS_CONTAINER;
        break;

    case E_REPORT:
        rList.emplace_back( ".uno:DBNewReport", RID_STR_REPORT_HELP_TEXT, RID_STR_NEW_REPORT );
        rList.emplace_back( ".uno:DBNewReportAutoPilot", RID_STR_REPORTS_HELP_TEXT_WIZARD, RID_STR_NEW_REPORT_AUTO );
        _rData.pTitleId = RID_STR_REPORTS_CONTAINER;
        break;

    case E_QUERY:
        rList.emplace_back( ".uno:DBNewQuery", RID_STR_QUERIES_HELP_TEXT, RID_STR_NEW_QUERY );
        rList.emplace_back( ".uno:DBNewQueryAutoPilot", RID_STR_QUERIES_HELP_TEXT_WIZARD, RID_STR_NEW_QUERY_AUTO );
        rList.emplace_back( ".uno:DBNewQuerySql", RID_STR_QUERIES_HELP_TEXT_SQL, RID_STR_NEW_QUERY_SQL );
        _rData.pTitleId = RID_STR_QUERIES_CONTAINER;
        break;

    default:
        OSL_FAIL( "OApplicationDetailView::impl_fillTaskPaneData: illegal element type!" );
    }
}

}

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

OUString OApplicationDetailView::getQualifiedName( SvTreeListEntry* _pEntry ) const
{
    return m_pControlHelper->getQualifiedName( _pEntry );
}

bool OApplicationDetailView::isLeaf(SvTreeListEntry const * _pEntry)
{
    return OAppDetailPageHelper::isLeaf(_pEntry);
}

bool OApplicationDetailView::isALeafSelected() const
{
    return m_pControlHelper->isALeafSelected();
}

void OApplicationDetailView::selectAll()
{
    m_pControlHelper->selectAll();
}

void OApplicationDetailView::sortDown()
{
    m_pControlHelper->sortDown();
}

void OApplicationDetailView::sortUp()
{
    m_pControlHelper->sortUp();
}

bool OApplicationDetailView::isFilled() const
{
    return m_pControlHelper->isFilled();
}

ElementType OApplicationDetailView::getElementType() const
{
    return m_pControlHelper->getElementType();
}

void OApplicationDetailView::clearPages(bool _bTaskAlso)
{
    if ( _bTaskAlso )
        getTasksWindow().Clear();
    m_pControlHelper->clearPages();
}

sal_Int32 OApplicationDetailView::getSelectionCount()
{
    return m_pControlHelper->getSelectionCount();
}

sal_Int32 OApplicationDetailView::getElementCount() const
{
    return m_pControlHelper->getElementCount();
}

void OApplicationDetailView::getSelectionElementNames( std::vector< OUString>& _rNames ) const
{
    m_pControlHelper->getSelectionElementNames( _rNames );
}

void OApplicationDetailView::describeCurrentSelectionForControl( const Control& _rControl, Sequence< NamedDatabaseObject >& _out_rSelectedObjects )
{
    m_pControlHelper->describeCurrentSelectionForControl( _rControl, _out_rSelectedObjects );
}

void OApplicationDetailView::describeCurrentSelectionForType( const ElementType _eType, Sequence< NamedDatabaseObject >& _out_rSelectedObjects )
{
    m_pControlHelper->describeCurrentSelectionForType( _eType, _out_rSelectedObjects );
}

void OApplicationDetailView::selectElements(const Sequence< OUString>& _aNames)
{
    m_pControlHelper->selectElements( _aNames );
}

SvTreeListEntry* OApplicationDetailView::getEntry( const Point& _aPoint ) const
{
    return m_pControlHelper->getEntry(_aPoint);
}

bool OApplicationDetailView::isCutAllowed()
{
    return false;
}

bool OApplicationDetailView::isCopyAllowed()
{
    return true;
}

bool OApplicationDetailView::isPasteAllowed() { return true; }

void OApplicationDetailView::copy() { }

void OApplicationDetailView::cut()  { }

void OApplicationDetailView::paste() { }

SvTreeListEntry*  OApplicationDetailView::elementAdded(ElementType _eType,const OUString& _rName, const Any& _rObject )
{
    return m_pControlHelper->elementAdded(_eType,_rName, _rObject );
}

void OApplicationDetailView::elementRemoved(ElementType _eType,const OUString& _rName )
{
    m_pControlHelper->elementRemoved(_eType,_rName );
}

void OApplicationDetailView::elementReplaced(ElementType _eType
                                                    ,const OUString& _rOldName
                                                    ,const OUString& _rNewName )
{
    m_pControlHelper->elementReplaced( _eType, _rOldName, _rNewName );
}

PreviewMode OApplicationDetailView::getPreviewMode() const
{
    return m_pControlHelper->getPreviewMode();
}

bool OApplicationDetailView::isPreviewEnabled() const
{
    return m_pControlHelper->isPreviewEnabled();
}

void OApplicationDetailView::switchPreview(PreviewMode _eMode)
{
    m_pControlHelper->switchPreview(_eMode);
}

void OApplicationDetailView::showPreview(const Reference< XContent >& _xContent)
{
    m_pControlHelper->showPreview(_xContent);
}

void OApplicationDetailView::showPreview(   const OUString& _sDataSourceName,
                                            const OUString& _sName,
                                            bool _bTable)
{
    m_pControlHelper->showPreview(_sDataSourceName,_sName,_bTable);
}

bool OApplicationDetailView::isSortUp() const
{
    return m_pControlHelper->isSortUp();
}

vcl::Window* OApplicationDetailView::getTreeWindow() const
{
    return m_pControlHelper->getCurrentView();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
