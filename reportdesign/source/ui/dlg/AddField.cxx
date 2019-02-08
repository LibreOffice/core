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
#include <svx/dbaexchange.hxx>
#include <svx/svdpagv.hxx>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/i18n/XCollator.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>

#include <vcl/event.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <tools/diagnose_ex.h>
#include <rptui_slotid.hrc>

#include <connectivity/dbtools.hxx>
#include <core_resource.hxx>
#include <helpids.h>
#include <strings.hrc>
#include <strings.hxx>
#include <ColumnInfo.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/property.hxx>
#include <svtools/imgdef.hxx>
#include <vcl/treelistentry.hxx>

namespace rptui
{
const long STD_WIN_SIZE_X = 180;
const long STD_WIN_SIZE_Y = 320;

using namespace ::com::sun::star;
using namespace sdbc;
using namespace sdb;
using namespace uno;
using namespace datatransfer;
using namespace beans;
using namespace lang;
using namespace container;
using namespace ::svx;

class OAddFieldWindowListBox: public SvTreeListBox
{
    VclPtr<OAddFieldWindow>                    m_pTabWin;

public:
    explicit OAddFieldWindowListBox(Window* pParent, OAddFieldWindow* pFieldWindow);
    virtual ~OAddFieldWindowListBox() override;
    virtual void dispose() override;

    sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) override;
    sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

    uno::Sequence< beans::PropertyValue > getSelectedFieldDescriptors();

protected:
    // DragSourceHelper
    virtual void StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;

private:
    using SvTreeListBox::ExecuteDrop;
};

uno::Sequence< beans::PropertyValue > OAddFieldWindowListBox::getSelectedFieldDescriptors()
{
    uno::Sequence< beans::PropertyValue > aArgs(GetSelectionCount());
    sal_Int32 i = 0;
    SvTreeListEntry* pSelected = FirstSelected();
    while( pSelected )
    {
        // build a descriptor for the currently selected field
        svx::ODataAccessDescriptor aDescriptor;
        m_pTabWin->fillDescriptor(pSelected,aDescriptor);
        aArgs[i++].Value <<= aDescriptor.createPropertyValueSequence();
        pSelected = NextSelected(pSelected);
    }
    return aArgs;
}

// class OAddFieldWindowListBox


OAddFieldWindowListBox::OAddFieldWindowListBox(Window *pParent, OAddFieldWindow* pFieldWindow)
    : SvTreeListBox(pParent, WB_TABSTOP|WB_BORDER|WB_SORT)
    , m_pTabWin(pFieldWindow)
{
    SetHelpId( HID_RPT_FIELD_SEL );
    SetSelectionMode(SelectionMode::Multiple);
    SetDragDropMode( DragDropMode::ALL );
    SetHighlightRange( );
}


OAddFieldWindowListBox::~OAddFieldWindowListBox()
{
    disposeOnce();
}

void OAddFieldWindowListBox::dispose()
{
    m_pTabWin.clear();
    SvTreeListBox::dispose();
}

sal_Int8 OAddFieldWindowListBox::AcceptDrop( const AcceptDropEvent& /*rEvt*/ )
{
    return DND_ACTION_NONE;
}


sal_Int8 OAddFieldWindowListBox::ExecuteDrop( const ExecuteDropEvent& /*rEvt*/ )
{
    return DND_ACTION_NONE;
}


void OAddFieldWindowListBox::StartDrag( sal_Int8 /*_nAction*/, const Point& /*_rPosPixel*/ )
{
    if ( GetSelectionCount() < 1 )
        // no drag without a field
        return;

    rtl::Reference<OMultiColumnTransferable> pDataContainer = new OMultiColumnTransferable(getSelectedFieldDescriptors());

    EndSelection();
    pDataContainer->StartDrag( this, DND_ACTION_COPYMOVE | DND_ACTION_LINK );
}

OAddFieldWindow::OAddFieldWindow(vcl::Window* pParent ,const uno::Reference< beans::XPropertySet >& _xRowSet)
    : FloatingWindow(pParent, "FloatingField", "modules/dbreport/ui/floatingfield.ui")
    , ::comphelper::OPropertyChangeListener(m_aMutex)
    , ::comphelper::OContainerListener(m_aMutex)
    , m_xRowSet(_xRowSet)
    , m_pListBox(VclPtr<OAddFieldWindowListBox>::Create(get<vcl::Window>("box"), this))
    , m_nCommandType(0)
    , m_bEscapeProcessing(false)
{
    get(m_aActions, "toolbox");
    m_nSortUpId = m_aActions->GetItemId(0);
    m_nSortDownId = m_aActions->GetItemId(1);
    m_nRemoveSortId = m_aActions->GetItemId(2);
    m_nInsertId = m_aActions->GetItemId(4);
    get(m_aHelpText, "helptext");

    SetHelpId( HID_RPT_FIELD_SEL_WIN );
    SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetFaceColor()) );
    SetMinOutputSizePixel(Size(STD_WIN_SIZE_X,STD_WIN_SIZE_Y));

    m_aActions->SetLineSpacing(true);
    m_aActions->SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetFaceColor()) );

    m_aActions->SetSelectHdl(LINK(this, OAddFieldWindow, OnSortAction));
    setToolBox(m_aActions.get());
    m_aActions->CheckItem(m_nSortUpId);
    m_aActions->EnableItem(m_nInsertId, false);

    m_pListBox->SetDoubleClickHdl(LINK( this, OAddFieldWindow, OnDoubleClickHdl ) );
    m_pListBox->SetSelectHdl(LINK( this, OAddFieldWindow, OnSelectHdl ) );
    m_pListBox->SetDeselectHdl(LINK( this, OAddFieldWindow, OnSelectHdl ) );
    m_pListBox->SetDoubleClickHdl(LINK( this, OAddFieldWindow, OnDoubleClickHdl ) );
    m_pListBox->set_expand(true);
    m_pListBox->set_height_request(m_pListBox->GetTextHeight() * 8);
    m_pListBox->set_width_request(m_pListBox->approximate_char_width() * 40);
    m_pListBox->Show();

    m_aHelpText->SetControlBackground( GetSettings().GetStyleSettings().GetFaceColor() );

    SetSizePixel(Size(STD_WIN_SIZE_X,STD_WIN_SIZE_Y));

    if ( m_xRowSet.is() )
    {
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
}

OAddFieldWindow::~OAddFieldWindow()
{
    disposeOnce();
}

void OAddFieldWindow::dispose()
{
    if ( m_pListBox.get() )
    {
        SvTreeList* pModel = m_pListBox->GetModel();
        sal_uLong nCount = pModel->GetEntryCount();
        for(sal_uLong i = 0; i< nCount;++i)
        {
            delete static_cast<ColumnInfo*>(pModel->GetEntry(i)->GetUserData());
        }
    }
    if (m_pChangeListener.is())
        m_pChangeListener->dispose();
    if ( m_pContainerListener.is() )
        m_pContainerListener->dispose();

    m_aActions.clear();
    m_aHelpText.clear();
    m_pListBox.disposeAndClear();
    FloatingWindow::dispose();
}

void OAddFieldWindow::GetFocus()
{
    if ( m_pListBox.get() )
        m_pListBox->GrabFocus();
    else
        FloatingWindow::GetFocus();
}

uno::Sequence< beans::PropertyValue > OAddFieldWindow::getSelectedFieldDescriptors()
{
    return m_pListBox->getSelectedFieldDescriptors();
}


bool OAddFieldWindow::PreNotify( NotifyEvent& _rNEvt )
{
    if ( MouseNotifyEvent::KEYINPUT == _rNEvt.GetType() )
    {
        const vcl::KeyCode& rKeyCode = _rNEvt.GetKeyEvent()->GetKeyCode();
        if ( ( 0 == rKeyCode.GetModifier() ) && ( KEY_RETURN == rKeyCode.GetCode() ) )
        {
            if ( m_aCreateLink.IsSet() )
            {
                m_aCreateLink.Call(*this);
                return true;
            }
        }
    }

    return FloatingWindow::PreNotify( _rNEvt );
}

void OAddFieldWindow::_propertyChanged( const beans::PropertyChangeEvent& _evt )
{
    OSL_ENSURE( _evt.Source == m_xRowSet, "OAddFieldWindow::_propertyChanged: where did this come from?" );
    Update();
}


namespace
{
    void lcl_addToList( OAddFieldWindowListBox& _rListBox, const uno::Sequence< OUString >& _rEntries )
    {
        const OUString* pEntries = _rEntries.getConstArray();
        sal_Int32 nEntries = _rEntries.getLength();
        for ( sal_Int32 i = 0; i < nEntries; ++i, ++pEntries )
            _rListBox.InsertEntry( *pEntries,nullptr,false,TREELIST_APPEND,new ColumnInfo(*pEntries) );
    }
    void lcl_addToList( OAddFieldWindowListBox& _rListBox, const uno::Reference< container::XNameAccess>& i_xColumns )
    {
        uno::Sequence< OUString > aEntries = i_xColumns->getElementNames();
        const OUString* pEntries = aEntries.getConstArray();
        sal_Int32 nEntries = aEntries.getLength();
        for ( sal_Int32 i = 0; i < nEntries; ++i, ++pEntries )
        {
            uno::Reference< beans::XPropertySet> xColumn(i_xColumns->getByName(*pEntries),UNO_QUERY_THROW);
            OUString sLabel;
            if ( xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_LABEL) )
                xColumn->getPropertyValue(PROPERTY_LABEL) >>= sLabel;
            if ( !sLabel.isEmpty() )
                _rListBox.InsertEntry( sLabel,nullptr,false,TREELIST_APPEND,new ColumnInfo(*pEntries,sLabel) );
            else
                _rListBox.InsertEntry( *pEntries,nullptr,false,TREELIST_APPEND,new ColumnInfo(*pEntries,sLabel) );
        }
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
        m_pListBox->Clear();
        const ToolBox::ImplToolItems::size_type nItemCount = m_aActions->GetItemCount();
        for (ToolBox::ImplToolItems::size_type j = 0; j< nItemCount; ++j)
        {
            m_aActions->EnableItem(m_aActions->GetItemId(j),false);
        }

        OUString aTitle(RptResId(RID_STR_FIELDSELECTION));
        SetText(aTitle);
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
                lcl_addToList( *m_pListBox, m_xColumns );
                uno::Reference< container::XContainer> xContainer(m_xColumns,uno::UNO_QUERY);
                if ( xContainer.is() )
                    m_pContainerListener = new ::comphelper::OContainerListenerAdapter(this,xContainer);
            }

            // add the parameter columns to the list
            uno::Reference< css::sdbc::XRowSet > xRowSet(m_xRowSet,uno::UNO_QUERY);
            Sequence< OUString > aParamNames( getParameterNames( xRowSet ) );
            lcl_addToList( *m_pListBox, aParamNames );

            // set title
            aTitle += " " + m_aCommandName;
            SetText( aTitle );
            if ( !m_aCommandName.isEmpty() )
            {
                for (ToolBox::ImplToolItems::size_type i = 0; i < nItemCount; ++i)
                {
                    m_aActions->EnableItem(m_aActions->GetItemId(i));
                }
            }
            OnSelectHdl(nullptr);
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

void OAddFieldWindow::fillDescriptor(SvTreeListEntry const * _pSelected,svx::ODataAccessDescriptor& _rDescriptor)
{
    if ( _pSelected && m_xColumns.is() )
    {
        uno::Reference<container::XChild> xChild(getConnection(),uno::UNO_QUERY);
        if ( xChild.is( ) )
        {
            uno::Reference<sdb::XDocumentDataSource> xDocument( xChild->getParent(), uno::UNO_QUERY );
            if ( xDocument.is() )
            {
                uno::Reference<frame::XModel> xModel(xDocument->getDatabaseDocument(),uno::UNO_QUERY);
                if ( xModel.is() )
                    _rDescriptor[ DataAccessDescriptorProperty::DatabaseLocation ] <<= xModel->getURL();
            }
        }

        _rDescriptor[ svx::DataAccessDescriptorProperty::Command ]            <<= GetCommand();
        _rDescriptor[ svx::DataAccessDescriptorProperty::CommandType ]        <<= GetCommandType();
        _rDescriptor[ svx::DataAccessDescriptorProperty::EscapeProcessing ]   <<= GetEscapeProcessing();
        _rDescriptor[ svx::DataAccessDescriptorProperty::Connection ]         <<= getConnection();

        ColumnInfo* pInfo = static_cast<ColumnInfo*>(_pSelected->GetUserData());
        _rDescriptor[ svx::DataAccessDescriptorProperty::ColumnName ]         <<= pInfo->sColumnName;
        if ( m_xColumns->hasByName( pInfo->sColumnName ) )
            _rDescriptor[ svx::DataAccessDescriptorProperty::ColumnObject ] = m_xColumns->getByName(pInfo->sColumnName);
    }
}

void OAddFieldWindow::_elementInserted( const container::ContainerEvent& _rEvent )
{
    if ( m_pListBox.get() )
    {
        OUString sName;
        if ( (_rEvent.Accessor >>= sName) && m_xColumns->hasByName(sName) )
        {
            uno::Reference< beans::XPropertySet> xColumn(m_xColumns->getByName(sName),UNO_QUERY_THROW);
            OUString sLabel;
            if ( xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_LABEL) )
                xColumn->getPropertyValue(PROPERTY_LABEL) >>= sLabel;
            if ( !sLabel.isEmpty() )
                m_pListBox->InsertEntry( sLabel,nullptr,false,TREELIST_APPEND,new ColumnInfo(sName,sLabel) );
            else
                m_pListBox->InsertEntry( sName,nullptr,false,TREELIST_APPEND,new ColumnInfo(sName,sLabel) );
        }
    }
}

void OAddFieldWindow::_elementRemoved( const container::ContainerEvent& /*_rEvent*/ )
{
    if ( m_pListBox.get() )
    {
        m_pListBox->Clear();
        if ( m_xColumns.is() )
            lcl_addToList( *m_pListBox, m_xColumns );
    }
}

void OAddFieldWindow::_elementReplaced( const container::ContainerEvent& /*_rEvent*/ )
{
}

IMPL_LINK_NOARG( OAddFieldWindow, OnSelectHdl, SvTreeListBox*, void )
{
    m_aActions->EnableItem(m_nInsertId, ( m_pListBox.get() && m_pListBox->GetSelectionCount() > 0 ));
}

IMPL_LINK_NOARG( OAddFieldWindow, OnDoubleClickHdl, SvTreeListBox*, bool )
{
    m_aCreateLink.Call(*this);

    return false;
}

void OAddFieldWindow::resizeControls(const Size& _rDiff)
{
    // we use large images so we must change them
    if ( _rDiff.Width() || _rDiff.Height() )
    {
        Invalidate();
    }
}

IMPL_LINK_NOARG( OAddFieldWindow, OnSortAction, ToolBox*, void )
{
    const sal_uInt16 nCurItem = m_aActions->GetCurItemId();
    if (nCurItem == m_nInsertId)
        OnDoubleClickHdl(nullptr);
    else
    {
        if (nCurItem == m_nRemoveSortId || !m_aActions->IsItemChecked(nCurItem))
        {
            const ToolBox::ImplToolItems::size_type nItemCount = m_aActions->GetItemCount();
            for (ToolBox::ImplToolItems::size_type j = 0; j< nItemCount; ++j)
            {
                const sal_uInt16 nItemId = m_aActions->GetItemId(j);
                if ( nCurItem != nItemId )
                    m_aActions->CheckItem(nItemId,false);
            }
            SvSortMode eSortMode = SortNone;
            if (nCurItem != m_nRemoveSortId)
            {
                m_aActions->CheckItem(nCurItem,!m_aActions->IsItemChecked(nCurItem));
                if (m_aActions->IsItemChecked(m_nSortUpId))
                    eSortMode = SortAscending;
                else if (m_aActions->IsItemChecked(m_nSortDownId))
                    eSortMode = SortDescending;
            }

            m_pListBox->GetModel()->SetSortMode(eSortMode);
            if (nCurItem == m_nRemoveSortId)
                Update();

            m_pListBox->GetModel()->Resort();
        }
    }
}


} // namespace rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
