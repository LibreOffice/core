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
#include <vcl/svapp.hxx>
#include "AppDetailPageHelper.hxx"
#include <dbaccess/IController.hxx>
#include <algorithm>
#include <dbtreelistbox.hxx>
#include "AppController.hxx"

using namespace ::dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using ::com::sun::star::util::URL;
using ::com::sun::star::sdb::application::NamedDatabaseObject;

TaskEntry::TaskEntry( const char* _pAsciiUNOCommand, TranslateId _pHelpID, TranslateId pTitleResourceID, bool _bHideWhenDisabled )
    :sUNOCommand( OUString::createFromAscii( _pAsciiUNOCommand ) )
    ,pHelpID( _pHelpID )
    ,sTitle( DBA_RES(pTitleResourceID) )
    ,bHideWhenDisabled( _bHideWhenDisabled )
{
}

void OTasksWindow::updateHelpText()
{
    TranslateId pHelpTextId;
    int nCurEntry = m_xTreeView->get_selected_index();
    if (nCurEntry != -1)
        pHelpTextId = weld::fromId<TaskEntry*>(m_xTreeView->get_id(nCurEntry))->pHelpID;
    setHelpText(pHelpTextId);
}

IMPL_LINK(OTasksWindow, onSelected, weld::TreeView&, rTreeView, bool)
{
    m_nCursorIndex = rTreeView.get_cursor_index();
    if (m_nCursorIndex != -1)
    {
        URL aCommand;
        aCommand.Complete = weld::fromId<TaskEntry*>(rTreeView.get_id(m_nCursorIndex))->sUNOCommand;
        getDetailView()->getBorderWin().getView()->getAppController().executeChecked( aCommand, Sequence< PropertyValue >() );
    }
    return true;
}

void OTasksWindow::GrabFocus()
{
    if (!m_xTreeView)
        return;
    m_xTreeView->grab_focus();
}

bool OTasksWindow::HasChildPathFocus() const
{
    return m_xTreeView && m_xTreeView->has_focus();
}

IMPL_LINK_NOARG(OTasksWindow, FocusInHdl, weld::Widget&, void)
{
    m_xTreeView->select(m_nCursorIndex != -1 ? m_nCursorIndex : 0);
}

IMPL_LINK_NOARG(OTasksWindow, FocusOutHdl, weld::Widget&, void)
{
    m_nCursorIndex = m_xTreeView->get_cursor_index();
    m_xTreeView->unselect_all();
}

IMPL_LINK_NOARG(OTasksWindow, OnEntrySelectHdl, weld::TreeView&, void)
{
    m_nCursorIndex = m_xTreeView->get_cursor_index();
    updateHelpText();
}

OTasksWindow::OTasksWindow(weld::Container* pParent, OApplicationDetailView* pDetailView)
    : OChildWindow(pParent, "dbaccess/ui/taskwindow.ui", "TaskWindow")
    , m_xTreeView(m_xBuilder->weld_tree_view("treeview"))
    , m_xDescription(m_xBuilder->weld_label("description"))
    , m_xHelpText(m_xBuilder->weld_text_view("helptext"))
    , m_pDetailView(pDetailView)
    , m_nCursorIndex(-1)
{
    m_xContainer->set_stack_background();

    m_xTreeView->set_help_id(HID_APP_CREATION_LIST);
    m_xTreeView->connect_row_activated(LINK(this, OTasksWindow, onSelected));
    m_xTreeView->connect_changed(LINK(this, OTasksWindow, OnEntrySelectHdl));
    m_xTreeView->connect_focus_in(LINK(this, OTasksWindow, FocusInHdl));
    m_xTreeView->connect_focus_out(LINK(this, OTasksWindow, FocusOutHdl));
    // an arbitrary small size it's allowed to shrink to
    m_xTreeView->set_size_request(42, 42);

    m_xHelpText->set_help_id(HID_APP_HELP_TEXT);
    m_xDescription->set_help_id(HID_APP_DESCRIPTION_TEXT);
}

OTasksWindow::~OTasksWindow()
{
    Clear();
}

void OTasksWindow::setHelpText(TranslateId pId)
{
    if (pId)
        m_xHelpText->set_text(DBA_RES(pId));
    else
        m_xHelpText->set_text(OUString());
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

        size_t nIndex = 0;
        for (auto const& task : _rList)
        {
            OUString sId = weld::toId(new TaskEntry(task));
            m_xTreeView->append(sId, task.sTitle);
            m_xTreeView->set_image(nIndex++, *pImages++);
        }
    }
    catch(Exception&)
    {
    }

    m_xTreeView->unselect_all();
    updateHelpText();
    Enable(!_rList.empty());
}

void OTasksWindow::Clear()
{
    m_xTreeView->all_foreach([this](weld::TreeIter& rEntry){
        TaskEntry* pUserData = weld::fromId<TaskEntry*>(m_xTreeView->get_id(rEntry));
        delete pUserData;
        return false;
    });

    m_xTreeView->clear();
}

OApplicationDetailView::OApplicationDetailView(weld::Container* pParent, OAppBorderWindow& rBorder,
                                               PreviewMode ePreviewMode)
    : m_xBuilder(Application::CreateBuilder(pParent, "dbaccess/ui/appdetailwindow.ui"))
    , m_xContainer(m_xBuilder->weld_container("AppDetailWindow"))
    , m_xHorzSplitter(m_xBuilder->weld_paned("splitter"))
    , m_xTasksParent(m_xBuilder->weld_container("tasks"))
    , m_xContainerParent(m_xBuilder->weld_container("container"))
    , m_xTasks(new dbaui::OTitleWindow(m_xTasksParent.get(), STR_TASKS))
    , m_xTitleContainer(new dbaui::OTitleWindow(m_xContainerParent.get(), TranslateId()))
    , m_rBorderWin(rBorder)
{
    m_xControlHelper = std::make_shared<OAppDetailPageHelper>(m_xTitleContainer->getChildContainer(), m_rBorderWin, ePreviewMode);
    m_xTitleContainer->setChildWindow(m_xControlHelper);

    std::shared_ptr<OChildWindow> xTasks = std::make_shared<OTasksWindow>(m_xTasks->getChildContainer(), this);
    xTasks->Enable(!m_rBorderWin.getView()->getCommandController().isDataSourceReadOnly());
    m_xTasks->setChildWindow(xTasks);
}

OApplicationDetailView::~OApplicationDetailView()
{
}

void OApplicationDetailView::setTaskExternalMnemonics( MnemonicGenerator const & rMnemonics )
{
    m_aExternalMnemonics = rMnemonics;
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
    m_xTitleContainer->setTitle(rData.pTitleId);

    // let our helper create the object list
    if ( _eType == E_TABLE )
        GetControlHelper()->createTablesPage( _rxConnection );
    else
        GetControlHelper()->createPage( _eType, _rxNonTableElements );
}

void OApplicationDetailView::impl_fillTaskPaneData(ElementType _eType, TaskPaneData& _rData) const
{
    TaskEntryList& rList( _rData.aTasks );
    rList.clear(); rList.reserve( 4 );

    switch ( _eType )
    {
    case E_TABLE:
        rList.emplace_back( ".uno:DBNewTable", RID_STR_TABLES_HELP_TEXT_DESIGN, RID_STR_NEW_TABLE );
        rList.emplace_back( ".uno:DBNewTableAutoPilot", RID_STR_TABLES_HELP_TEXT_WIZARD, RID_STR_NEW_TABLE_AUTO );
        rList.emplace_back( ".uno:DBNewView", RID_STR_VIEWS_HELP_TEXT_DESIGN, RID_STR_NEW_VIEW, true );
        _rData.pTitleId = RID_STR_TABLES_CONTAINER;
        break;

    case E_FORM:
        rList.emplace_back( ".uno:DBNewForm", RID_STR_FORMS_HELP_TEXT, RID_STR_NEW_FORM );
        rList.emplace_back( ".uno:DBNewFormAutoPilot", RID_STR_FORMS_HELP_TEXT_WIZARD, RID_STR_NEW_FORM_AUTO );
        _rData.pTitleId = RID_STR_FORMS_CONTAINER;
        break;

    case E_REPORT:
        rList.emplace_back( ".uno:DBNewReport", RID_STR_REPORT_HELP_TEXT, RID_STR_NEW_REPORT, true );
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

    // remove the entries which are not enabled currently
    for (TaskEntryList::iterator pTask = rList.begin(); pTask != rList.end();)
    {
        if  (   pTask->bHideWhenDisabled
            &&  !getBorderWin().getView()->getCommandController().isCommandEnabled( pTask->sUNOCommand )
            )
            pTask = rList.erase( pTask );
        else
        {
            ++pTask;
        }
    }
}

const TaskPaneData& OApplicationDetailView::impl_getTaskPaneData( ElementType _eType )
{
    if ( m_aTaskPaneData.empty() )
        m_aTaskPaneData.resize( size_t(E_ELEMENT_TYPE_COUNT) );
    OSL_ENSURE( ( _eType >= 0 ) && ( _eType < E_ELEMENT_TYPE_COUNT ), "OApplicationDetailView::impl_getTaskPaneData: illegal element type!" );
    TaskPaneData& rData = m_aTaskPaneData[ _eType ];

    //oj: do not check, otherwise extensions will only be visible after a reload.
    impl_fillTaskPaneData( _eType, rData );

    return rData;
}

OUString OApplicationDetailView::getQualifiedName(const weld::TreeIter* _pEntry) const
{
    return GetControlHelper()->getQualifiedName( _pEntry );
}

bool OApplicationDetailView::isLeaf(const weld::TreeView& rTreeView, const weld::TreeIter& rEntry)
{
    return OAppDetailPageHelper::isLeaf(rTreeView, rEntry);
}

bool OApplicationDetailView::isALeafSelected() const
{
    return GetControlHelper()->isALeafSelected();
}

void OApplicationDetailView::selectAll()
{
    GetControlHelper()->selectAll();
}

void OApplicationDetailView::sortDown()
{
    GetControlHelper()->sortDown();
}

void OApplicationDetailView::sortUp()
{
    GetControlHelper()->sortUp();
}

bool OApplicationDetailView::isFilled() const
{
    return GetControlHelper()->isFilled();
}

ElementType OApplicationDetailView::getElementType() const
{
    return GetControlHelper()->getElementType();
}

void OApplicationDetailView::clearPages(bool _bTaskAlso)
{
    if ( _bTaskAlso )
        getTasksWindow().Clear();
    GetControlHelper()->clearPages();
}

sal_Int32 OApplicationDetailView::getSelectionCount()
{
    return GetControlHelper()->getSelectionCount();
}

sal_Int32 OApplicationDetailView::getElementCount() const
{
    return GetControlHelper()->getElementCount();
}

void OApplicationDetailView::getSelectionElementNames( std::vector< OUString>& _rNames ) const
{
    GetControlHelper()->getSelectionElementNames( _rNames );
}

void OApplicationDetailView::describeCurrentSelectionForControl(const weld::TreeView& rControl, Sequence< NamedDatabaseObject >& out_rSelectedObjects)
{
    GetControlHelper()->describeCurrentSelectionForControl(rControl, out_rSelectedObjects);
}

void OApplicationDetailView::describeCurrentSelectionForType( const ElementType _eType, Sequence< NamedDatabaseObject >& _out_rSelectedObjects )
{
    GetControlHelper()->describeCurrentSelectionForType( _eType, _out_rSelectedObjects );
}

vcl::Window* OApplicationDetailView::getMenuParent() const
{
    return GetControlHelper()->getMenuParent();
}

void OApplicationDetailView::adjustMenuPosition(const weld::TreeView& rControl, ::Point& rPos) const
{
    return GetControlHelper()->adjustMenuPosition(rControl, rPos);
}

void OApplicationDetailView::selectElements(const Sequence< OUString>& _aNames)
{
    GetControlHelper()->selectElements( _aNames );
}

std::unique_ptr<weld::TreeIter> OApplicationDetailView::getEntry(const Point& rPoint) const
{
    return GetControlHelper()->getEntry(rPoint);
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

std::unique_ptr<weld::TreeIter> OApplicationDetailView::elementAdded(ElementType _eType,const OUString& _rName, const Any& _rObject )
{
    return GetControlHelper()->elementAdded(_eType, _rName, _rObject);
}

void OApplicationDetailView::elementRemoved(ElementType _eType,const OUString& _rName )
{
    GetControlHelper()->elementRemoved(_eType,_rName );
}

void OApplicationDetailView::elementReplaced(ElementType _eType
                                                    ,const OUString& _rOldName
                                                    ,const OUString& _rNewName )
{
    GetControlHelper()->elementReplaced( _eType, _rOldName, _rNewName );
}

PreviewMode OApplicationDetailView::getPreviewMode() const
{
    return GetControlHelper()->getPreviewMode();
}

bool OApplicationDetailView::isPreviewEnabled() const
{
    return GetControlHelper()->isPreviewEnabled();
}

void OApplicationDetailView::switchPreview(PreviewMode _eMode)
{
    GetControlHelper()->switchPreview(_eMode);
}

void OApplicationDetailView::showPreview(const Reference< XContent >& _xContent)
{
    GetControlHelper()->showPreview(_xContent);
}

void OApplicationDetailView::showPreview(   const OUString& _sDataSourceName,
                                            const OUString& _sName,
                                            bool _bTable)
{
    GetControlHelper()->showPreview(_sDataSourceName,_sName,_bTable);
}

bool OApplicationDetailView::isSortUp() const
{
    return GetControlHelper()->isSortUp();
}

TreeListBox* OApplicationDetailView::getTreeWindow() const
{
    DBTreeViewBase* pCurrent = GetControlHelper()->getCurrentView();
    if (!pCurrent)
        return nullptr;
    return &pCurrent->getListBox();
}

OAppDetailPageHelper* OApplicationDetailView::GetControlHelper()
{
    return static_cast<OAppDetailPageHelper*>(m_xControlHelper.get());
}

const OAppDetailPageHelper* OApplicationDetailView::GetControlHelper() const
{
    return static_cast<const OAppDetailPageHelper*>(m_xControlHelper.get());
}

bool OApplicationDetailView::HasChildPathFocus() const
{
    return m_xHorzSplitter->has_focus() ||
           m_xTasks->HasChildPathFocus() ||
           m_xTitleContainer->HasChildPathFocus();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
