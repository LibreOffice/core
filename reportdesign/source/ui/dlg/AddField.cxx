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
#include <AddField.hxx>
#include <UITools.hxx>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>

#include <utility>
#include <vcl/svapp.hxx>

#include <comphelper/diagnose_ex.hxx>

#include <connectivity/dbtools.hxx>
#include <core_resource.hxx>
#include <helpids.h>
#include <strings.hrc>
#include <strings.hxx>

#include <comphelper/sequence.hxx>

namespace rptui
{

using namespace ::com::sun::star;
using namespace sdbc;
using namespace sdb;
using namespace uno;
using namespace beans;
using namespace lang;
using namespace container;
using namespace ::svx;

IMPL_LINK(OAddFieldWindow, DragBeginHdl, bool&, rUnsetDragIcon, bool)
{
    rUnsetDragIcon = false;
    if (m_xListBox->get_selected_index() == -1)
    {
        // no drag without a field
        return true;
    }

    m_xHelper->setDescriptors(getSelectedFieldDescriptors());
    return false;
}

OAddFieldWindow::OAddFieldWindow(weld::Window* pParent, uno::Reference< beans::XPropertySet > xRowSet)
    : GenericDialogController(pParent, "modules/dbreport/ui/floatingfield.ui", "FloatingField")
    , ::comphelper::OPropertyChangeListener(m_aMutex)
    , ::comphelper::OContainerListener(m_aMutex)
    , m_xRowSet(std::move(xRowSet))
    , m_xActions(m_xBuilder->weld_toolbar("toolbox"))
    , m_xListBox(m_xBuilder->weld_tree_view("treeview"))
    , m_nCommandType(0)
    , m_bEscapeProcessing(false)
{
    m_xListBox->set_help_id(HID_RPT_FIELD_SEL);
    m_xListBox->set_selection_mode(SelectionMode::Multiple);
    m_xHelper.set(new svx::OMultiColumnTransferable);
    rtl::Reference<TransferDataContainer> xHelper(m_xHelper);
    m_xListBox->enable_drag_source(xHelper, DND_ACTION_COPYMOVE | DND_ACTION_LINK);
    m_xListBox->connect_drag_begin(LINK(this, OAddFieldWindow, DragBeginHdl));

    m_xDialog->connect_container_focus_changed(LINK(this, OAddFieldWindow, FocusChangeHdl));

    m_xDialog->set_help_id(HID_RPT_FIELD_SEL_WIN);

    m_xActions->connect_clicked(LINK(this, OAddFieldWindow, OnSortAction));
    m_xActions->set_item_active("up", true);
    m_xListBox->make_sorted();
    m_xActions->set_item_sensitive("insert", false);

    m_xListBox->connect_row_activated(LINK( this, OAddFieldWindow, OnDoubleClickHdl ) );
    m_xListBox->connect_changed(LINK( this, OAddFieldWindow, OnSelectHdl ) );
    m_xListBox->set_size_request(m_xListBox->get_approximate_digit_width() * 45, m_xListBox->get_height_rows(8));

    if (!m_xRowSet.is())
        return;

    try
    {
        // be notified when the settings of report definition change
        m_pChangeListener = new ::comphelper::OPropertyChangeMultiplexer( this, m_xRowSet );
        m_pChangeListener->addProperty( PROPERTY_COMMAND );
        m_pChangeListener->addProperty( PROPERTY_COMMANDTYPE );
        m_pChangeListener->addProperty( PROPERTY_ESCAPEPROCESSING );
        m_pChangeListener->addProperty( PROPERTY_FILTER );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("reportdesign");
    }
}

OAddFieldWindow::~OAddFieldWindow()
{
    m_aListBoxData.clear();
    if (m_pChangeListener.is())
        m_pChangeListener->dispose();
    if ( m_pContainerListener.is() )
        m_pContainerListener->dispose();
}

IMPL_LINK_NOARG(OAddFieldWindow, FocusChangeHdl, weld::Container&, void)
{
    if (m_xDialog->has_toplevel_focus())
        m_xListBox->grab_focus();
}

uno::Sequence< beans::PropertyValue > OAddFieldWindow::getSelectedFieldDescriptors()
{
    std::vector<beans::PropertyValue> aArgs;

    m_xListBox->selected_foreach([this, &aArgs](weld::TreeIter& rEntry){
        // build a descriptor for the currently selected field
        svx::ODataAccessDescriptor aDescriptor;
        fillDescriptor(rEntry, aDescriptor);
        aArgs.push_back(beans::PropertyValue());
        aArgs.back().Value <<= aDescriptor.createPropertyValueSequence();

        return false;
    });

    return comphelper::containerToSequence(aArgs);
}

void OAddFieldWindow::_propertyChanged( const beans::PropertyChangeEvent& _evt )
{
    OSL_ENSURE( _evt.Source == m_xRowSet, "OAddFieldWindow::_propertyChanged: where did this come from?" );
    Update();
}

void OAddFieldWindow::addToList(const uno::Sequence< OUString >& rEntries)
{
    for (const OUString& rEntry : rEntries)
    {
        m_aListBoxData.emplace_back(new ColumnInfo(rEntry));
        OUString sId(weld::toId(m_aListBoxData.back().get()));
        m_xListBox->append(sId, rEntry);
    }
}

void OAddFieldWindow::addToList(const uno::Reference< container::XNameAccess>& i_xColumns)
{
    const uno::Sequence< OUString > aEntries = i_xColumns->getElementNames();
    for ( const OUString& rEntry : aEntries )
    {
        uno::Reference< beans::XPropertySet> xColumn(i_xColumns->getByName(rEntry),UNO_QUERY_THROW);
        OUString sLabel;
        if ( xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_LABEL) )
            xColumn->getPropertyValue(PROPERTY_LABEL) >>= sLabel;
        m_aListBoxData.emplace_back(new ColumnInfo(rEntry, sLabel));
        OUString sId(weld::toId(m_aListBoxData.back().get()));
        if ( !sLabel.isEmpty() )
            m_xListBox->append(sId, sLabel);
        else
            m_xListBox->append(sId, rEntry);
    }
}

void OAddFieldWindow::Update()
{
    SolarMutexGuard aSolarGuard;

    if ( m_pContainerListener.is() )
        m_pContainerListener->dispose();
    m_pContainerListener = nullptr;
    m_xColumns.clear();

    try
    {
        // ListBox loeschen
        m_xListBox->clear();
        m_aListBoxData.clear();
        const OUString aIds[] = { "up", "down" };
        for (size_t j = 0; j< std::size(aIds); ++j)
            m_xActions->set_item_sensitive(aIds[j], false);

        OUString aTitle(RptResId(RID_STR_FIELDSELECTION));
        m_xDialog->set_title(aTitle);
        if ( m_xRowSet.is() )
        {
            OUString sCommand( m_aCommandName );
            sal_Int32       nCommandType( m_nCommandType );
            bool        bEscapeProcessing( m_bEscapeProcessing );
            OUString sFilter( m_sFilter );

            OSL_VERIFY( m_xRowSet->getPropertyValue( PROPERTY_COMMAND ) >>= sCommand );
            OSL_VERIFY( m_xRowSet->getPropertyValue( PROPERTY_COMMANDTYPE ) >>= nCommandType );
            OSL_VERIFY( m_xRowSet->getPropertyValue( PROPERTY_ESCAPEPROCESSING ) >>= bEscapeProcessing );
            OSL_VERIFY( m_xRowSet->getPropertyValue( PROPERTY_FILTER ) >>= sFilter );

            m_aCommandName  = sCommand;
            m_nCommandType  = nCommandType;
            m_bEscapeProcessing = bEscapeProcessing;
            m_sFilter = sFilter;

            // add the columns to the list
            uno::Reference< sdbc::XConnection> xCon = getConnection();
            if ( xCon.is() && !m_aCommandName.isEmpty() )
                m_xColumns = dbtools::getFieldsByCommandDescriptor( xCon, GetCommandType(), GetCommand(), m_xHoldAlive );
            if ( m_xColumns.is() )
            {
                addToList(m_xColumns);
                uno::Reference< container::XContainer> xContainer(m_xColumns,uno::UNO_QUERY);
                if ( xContainer.is() )
                    m_pContainerListener = new ::comphelper::OContainerListenerAdapter(this,xContainer);
            }

            // add the parameter columns to the list
            uno::Reference< css::sdbc::XRowSet > xRowSet(m_xRowSet,uno::UNO_QUERY);
            Sequence< OUString > aParamNames( getParameterNames( xRowSet ) );
            addToList(aParamNames);

            // set title
            aTitle += " " + m_aCommandName;
            m_xDialog->set_title(aTitle);
            if ( !m_aCommandName.isEmpty() )
            {
                for (size_t i = 0; i < std::size(aIds); ++i)
                    m_xActions->set_item_sensitive(aIds[i], true);
            }
            OnSelectHdl(*m_xListBox);
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("reportdesign");
    }
}

uno::Reference< sdbc::XConnection> OAddFieldWindow::getConnection() const
{
    return uno::Reference< sdbc::XConnection>(m_xRowSet->getPropertyValue( PROPERTY_ACTIVECONNECTION ),uno::UNO_QUERY);
}

void OAddFieldWindow::fillDescriptor(const weld::TreeIter& rSelected, svx::ODataAccessDescriptor& rDescriptor)
{
    if (!m_xColumns.is())
        return;

    uno::Reference<container::XChild> xChild(getConnection(),uno::UNO_QUERY);
    if ( xChild.is( ) )
    {
        uno::Reference<sdb::XDocumentDataSource> xDocument( xChild->getParent(), uno::UNO_QUERY );
        if ( xDocument.is() )
        {
            uno::Reference<frame::XModel> xModel(xDocument->getDatabaseDocument(),uno::UNO_QUERY);
            if ( xModel.is() )
                rDescriptor[ DataAccessDescriptorProperty::DatabaseLocation ] <<= xModel->getURL();
        }
    }

    rDescriptor[ svx::DataAccessDescriptorProperty::Command ]            <<= GetCommand();
    rDescriptor[ svx::DataAccessDescriptorProperty::CommandType ]        <<= GetCommandType();
    rDescriptor[ svx::DataAccessDescriptorProperty::EscapeProcessing ]   <<= m_bEscapeProcessing;
    rDescriptor[ svx::DataAccessDescriptorProperty::Connection ]         <<= getConnection();

    ColumnInfo* pInfo = weld::fromId<ColumnInfo*>(m_xListBox->get_id(rSelected));
    rDescriptor[ svx::DataAccessDescriptorProperty::ColumnName ]         <<= pInfo->sColumnName;
    if ( m_xColumns->hasByName( pInfo->sColumnName ) )
        rDescriptor[ svx::DataAccessDescriptorProperty::ColumnObject ] = m_xColumns->getByName(pInfo->sColumnName);
}

void OAddFieldWindow::_elementInserted( const container::ContainerEvent& _rEvent )
{
    OUString sName;
    if ( !((_rEvent.Accessor >>= sName) && m_xColumns->hasByName(sName)) )
        return;

    uno::Reference< beans::XPropertySet> xColumn(m_xColumns->getByName(sName),UNO_QUERY_THROW);
    OUString sLabel;
    if ( xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_LABEL) )
        xColumn->getPropertyValue(PROPERTY_LABEL) >>= sLabel;
    m_aListBoxData.emplace_back(new ColumnInfo(sName, sLabel));
    OUString sId(weld::toId(m_aListBoxData.back().get()));
    if (!sLabel.isEmpty())
        m_xListBox->append(sId, sLabel);
    else
        m_xListBox->append(sId, sName);
}

void OAddFieldWindow::_elementRemoved( const container::ContainerEvent& /*_rEvent*/ )
{
    m_xListBox->clear();
    m_aListBoxData.clear();
    if ( m_xColumns.is() )
        addToList(m_xColumns);
}

void OAddFieldWindow::_elementReplaced( const container::ContainerEvent& /*_rEvent*/ )
{
}

IMPL_LINK_NOARG( OAddFieldWindow, OnSelectHdl, weld::TreeView&, void )
{
    m_xActions->set_item_sensitive("insert", m_xListBox->get_selected_index() != -1);
}

IMPL_LINK_NOARG( OAddFieldWindow, OnDoubleClickHdl, weld::TreeView&, bool )
{
    m_aCreateLink.Call(*this);
    return true;
}

IMPL_LINK(OAddFieldWindow, OnSortAction, const OUString&, rCurItem, void)
{
    if (rCurItem == "insert")
    {
        OnDoubleClickHdl(*m_xListBox);
        return;
    }

    const OUString aIds[] = { "up", "down" };

    if (rCurItem == "delete")
    {
        for (size_t j = 0; j< std::size(aIds); ++j)
            m_xActions->set_item_active(aIds[j], false);

        m_xListBox->make_unsorted();
        Update();
        return;
    }

    for (size_t j = 0; j< std::size(aIds); ++j)
        m_xActions->set_item_active(aIds[j], rCurItem == aIds[j]);

    m_xListBox->make_sorted();
    if (m_xActions->get_item_active("down"))
        m_xListBox->set_sort_order(false);
}

} // namespace rptui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
