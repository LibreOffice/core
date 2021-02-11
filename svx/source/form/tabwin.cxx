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


#include <tabwin.hxx>
#include <fmservs.hxx>

#include <svx/strings.hrc>
#include <svx/svxids.hrc>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/types.hxx>

#include <helpids.h>
#include <svx/fmshell.hxx>
#include <fmshimp.hxx>

#include <fmprop.hxx>

#include <svx/dialmgr.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/frame.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <tools/diagnose_ex.h>
#include <tabwin.hrc>

const tools::Long STD_WIN_SIZE_X = 120;
const tools::Long STD_WIN_SIZE_Y = 150;

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star;
using namespace ::svxform;
using namespace ::svx;
using namespace ::dbtools;

struct ColumnInfo
{
    OUString sColumnName;
    explicit ColumnInfo(const OUString& i_sColumnName)
        : sColumnName(i_sColumnName)
    {
    }
};

void FmFieldWin::addToList(const uno::Reference< container::XNameAccess>& i_xColumns )
{
    const uno::Sequence< OUString > aEntries = i_xColumns->getElementNames();
    for ( const OUString& rEntry : aEntries )
    {
        uno::Reference< beans::XPropertySet> xColumn(i_xColumns->getByName(rEntry),UNO_QUERY_THROW);
        OUString sLabel;
        if ( xColumn->getPropertySetInfo()->hasPropertyByName(FM_PROP_LABEL) )
            xColumn->getPropertyValue(FM_PROP_LABEL) >>= sLabel;
        m_aListBoxData.emplace_back(new ColumnInfo(rEntry));
        OUString sId(OUString::number(reinterpret_cast<sal_Int64>(m_aListBoxData.back().get())));
        if ( !sLabel.isEmpty() )
            m_xListBox->append(sId, sLabel);
        else
            m_xListBox->append(sId, rEntry);
    }
}

IMPL_LINK(FmFieldWin, DragBeginHdl, bool&, rUnsetDragIcon, bool)
{
    rUnsetDragIcon = false;

    ColumnInfo* pSelected = reinterpret_cast<ColumnInfo*>(m_xListBox->get_selected_id().toInt64());
    if (!pSelected)
    {
        // no drag without a field
        return true;
    }

    svx::ODataAccessDescriptor aDescriptor;
    aDescriptor[ DataAccessDescriptorProperty::DataSource ] <<= GetDatabaseName();
    aDescriptor[ DataAccessDescriptorProperty::Connection ] <<= GetConnection().getTyped();
    aDescriptor[ DataAccessDescriptorProperty::Command ]    <<= GetObjectName();
    aDescriptor[ DataAccessDescriptorProperty::CommandType ]<<= GetObjectType();
    aDescriptor[ DataAccessDescriptorProperty::ColumnName ] <<= pSelected->sColumnName;

    m_xHelper->setDescriptor(aDescriptor);

    return false;
}

FmFieldWin::FmFieldWin(SfxBindings* _pBindings, SfxChildWindow* _pMgr, weld::Window* _pParent)
    : SfxModelessDialogController(_pBindings, _pMgr, _pParent, "svx/ui/formfielddialog.ui", "FormFieldDialog")
    , SfxControllerItem(SID_FM_FIELDS_CONTROL, *_pBindings)
    , comphelper::OPropertyChangeListener(m_aMutex)
    , m_xListBox(m_xBuilder->weld_tree_view("treeview"))
    , m_nObjectType(0)
{
    m_xDialog->set_help_id(HID_FIELD_SEL_WIN);
    m_xListBox->set_help_id(HID_FIELD_SEL);

    m_xListBox->connect_row_activated(LINK(this, FmFieldWin, RowActivatedHdl));
    m_xHelper.set(new OColumnTransferable(
        ColumnTransferFormatFlags::FIELD_DESCRIPTOR | ColumnTransferFormatFlags::CONTROL_EXCHANGE | ColumnTransferFormatFlags::COLUMN_DESCRIPTOR
    ));
    rtl::Reference<TransferDataContainer> xHelper(m_xHelper);
    m_xListBox->enable_drag_source(xHelper, DND_ACTION_COPY);
    m_xListBox->connect_drag_begin(LINK(this, FmFieldWin, DragBeginHdl));

    UpdateContent(nullptr);
    m_xDialog->set_size_request(STD_WIN_SIZE_X, STD_WIN_SIZE_Y);
}

FmFieldWin::~FmFieldWin()
{
    if (m_xChangeListener.is())
    {
        m_xChangeListener->dispose();
        m_xChangeListener.clear();
    }
    ::SfxControllerItem::dispose();
}

IMPL_LINK_NOARG(FmFieldWin, RowActivatedHdl, weld::TreeView&, bool)
{
    return createSelectionControls();
}

bool FmFieldWin::createSelectionControls()
{
    ColumnInfo* pSelected = reinterpret_cast<ColumnInfo*>(m_xListBox->get_selected_id().toInt64());
    if (pSelected)
    {
        // build a descriptor for the currently selected field
        ODataAccessDescriptor aDescr;
        aDescr.setDataSource(GetDatabaseName());

        aDescr[ DataAccessDescriptorProperty::Connection ]  <<= GetConnection().getTyped();

        aDescr[ DataAccessDescriptorProperty::Command ]     <<= GetObjectName();
        aDescr[ DataAccessDescriptorProperty::CommandType ] <<= GetObjectType();
        aDescr[ DataAccessDescriptorProperty::ColumnName ]  <<= pSelected->sColumnName;

        // transfer this to the SFX world
        SfxUnoAnyItem aDescriptorItem( SID_FM_DATACCESS_DESCRIPTOR, makeAny( aDescr.createPropertyValueSequence() ) );
        const SfxPoolItem* pArgs[] =
        {
            &aDescriptorItem, nullptr
        };

        // execute the create slot
        GetBindings().Execute( SID_FM_CREATE_FIELDCONTROL, pArgs );
    }

    return nullptr != pSelected;
}

void FmFieldWin::_propertyChanged(const css::beans::PropertyChangeEvent& evt)
{
    css::uno::Reference< css::form::XForm >  xForm(evt.Source, css::uno::UNO_QUERY);
    UpdateContent(xForm);
}

void FmFieldWin::StateChanged(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState)
{
    if (!pState  || SID_FM_FIELDS_CONTROL != nSID)
        return;

    if (eState >= SfxItemState::DEFAULT)
    {
        FmFormShell* pShell = dynamic_cast<FmFormShell*>( static_cast<const SfxObjectItem*>(pState)->GetShell() );
        UpdateContent(pShell);
    }
    else
        UpdateContent(nullptr);
}

void FmFieldWin::UpdateContent(FmFormShell const * pShell)
{
    m_xListBox->clear();
    m_aListBoxData.clear();
    OUString aTitle(SvxResId(RID_STR_FIELDSELECTION));
    m_xDialog->set_title(aTitle);

    if (!pShell || !pShell->GetImpl())
        return;

    Reference<XForm> const xForm = pShell->GetImpl()->getCurrentForm_Lock();
    if ( xForm.is() )
        UpdateContent( xForm );
}

void FmFieldWin::UpdateContent(const css::uno::Reference< css::form::XForm > & xForm)
{
    try
    {
        // delete ListBox
        m_xListBox->clear();
        m_aListBoxData.clear();
        OUString aTitle(SvxResId(RID_STR_FIELDSELECTION));
        m_xDialog->set_title(aTitle);

        if (!xForm.is())
            return;

        Reference< XPropertySet >  xSet(xForm, UNO_QUERY);

        m_aObjectName   = ::comphelper::getString(xSet->getPropertyValue(FM_PROP_COMMAND));
        m_aDatabaseName = ::comphelper::getString(xSet->getPropertyValue(FM_PROP_DATASOURCE));
        m_nObjectType   = ::comphelper::getINT32(xSet->getPropertyValue(FM_PROP_COMMANDTYPE));

        // get the connection of the form
        m_aConnection.reset(
            connectRowset( Reference< XRowSet >( xForm, UNO_QUERY ), ::comphelper::getProcessComponentContext(), nullptr ),
            SharedConnection::NoTakeOwnership
        );
        // TODO: When incompatible changes (such as extending the "virtualdbtools" interface by ensureRowSetConnection)
        // are allowed, again, we should change this: dbtools should consistently use SharedConnection all over
        // the place, and connectRowset should be replaced with ensureRowSetConnection

        // get the fields of the object

        if ( m_aConnection.is() && !m_aObjectName.isEmpty() )
        {
            Reference< XComponent > xKeepFieldsAlive;
            Reference< XNameAccess > xColumns = getFieldsByCommandDescriptor( m_aConnection, m_nObjectType, m_aObjectName,xKeepFieldsAlive );
            if ( xColumns.is() )
                addToList(xColumns);
        }

        // set prefix
        OUString  aPrefix;

        switch (m_nObjectType)
        {
            case CommandType::TABLE:
                aPrefix = SvxResId(RID_RSC_TABWIN_PREFIX[0]);
                break;
            case CommandType::QUERY:
                aPrefix = SvxResId(RID_RSC_TABWIN_PREFIX[1]);
                break;
            default:
                aPrefix = SvxResId(RID_RSC_TABWIN_PREFIX[2]);
                break;
        }

        // listen for changes at ControlSource in PropertySet
        if (m_xChangeListener.is())
        {
            m_xChangeListener->dispose();
            m_xChangeListener.clear();
        }
        m_xChangeListener = new ::comphelper::OPropertyChangeMultiplexer(this, xSet);
        m_xChangeListener->addProperty(FM_PROP_DATASOURCE);
        m_xChangeListener->addProperty(FM_PROP_COMMAND);
        m_xChangeListener->addProperty(FM_PROP_COMMANDTYPE);

        // set title
        aTitle += " " + aPrefix + " " + m_aObjectName;
        m_xDialog->set_title(aTitle);
    }
    catch( const Exception& )
    {
        TOOLS_WARN_EXCEPTION( "svx", "FmTabWin::UpdateContent" );
    }
}

void FmFieldWin::FillInfo( SfxChildWinInfo& rInfo ) const
{
    rInfo.bVisible = false;
}

SFX_IMPL_MODELESSDIALOGCONTOLLER(FmFieldWinMgr, SID_FM_ADD_FIELD)

FmFieldWinMgr::FmFieldWinMgr(vcl::Window* _pParent, sal_uInt16 _nId,
               SfxBindings* _pBindings, SfxChildWinInfo const * _pInfo)
              :SfxChildWindow(_pParent, _nId)
{
    auto xDlg = std::make_shared<FmFieldWin>(_pBindings, this, _pParent->GetFrameWeld());
    SetController(xDlg);
    SetHideNotDelete(true);
    xDlg->Initialize(_pInfo);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
