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


#include "tabwin.hxx"
#include "svx/fmtools.hxx"
#include "fmservs.hxx"
#include "stringlistresource.hxx"

#include <svx/svxids.hrc>
#include <svx/dbaexchange.hxx>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/util/XLocalizedAliases.hpp>
#include <comphelper/processfactory.hxx>

#include "fmhelp.hrc"
#include <svx/fmshell.hxx>
#include "fmshimp.hxx"
#include <svx/fmpage.hxx>

#include "fmpgeimp.hxx"

#include "fmprop.hrc"

#include "svx/fmresids.hrc"
#include <svx/dialmgr.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/dispatch.hxx>
#include <comphelper/property.hxx>
#include <sfx2/frame.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include "svtools/treelistentry.hxx"
#include <vcl/settings.hxx>

const long STD_WIN_SIZE_X = 120;
const long STD_WIN_SIZE_Y = 150;

const long LISTBOX_BORDER = 2;

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

namespace {

struct ColumnInfo
{
    OUString sColumnName;
    OUString sLabel;
    ColumnInfo(const OUString& i_sColumnName,const OUString& i_sLabel)
        : sColumnName(i_sColumnName)
        , sLabel(i_sLabel)
    {
    }
};

}

static void lcl_addToList( SvTreeListBox& _rListBox, const uno::Reference< container::XNameAccess>& i_xColumns )
{
    uno::Sequence< OUString > aEntries = i_xColumns->getElementNames();
    const OUString* pEntries = aEntries.getConstArray();
    sal_Int32 nEntries = aEntries.getLength();
    for ( sal_Int32 i = 0; i < nEntries; ++i, ++pEntries )
    {
        uno::Reference< beans::XPropertySet> xColumn(i_xColumns->getByName(*pEntries),UNO_QUERY_THROW);
        OUString sLabel;
        if ( xColumn->getPropertySetInfo()->hasPropertyByName(FM_PROP_LABEL) )
            xColumn->getPropertyValue(FM_PROP_LABEL) >>= sLabel;
        if ( !sLabel.isEmpty() )
            _rListBox.InsertEntry( sLabel, NULL, false, TREELIST_APPEND, new ColumnInfo(*pEntries,sLabel) );
        else
            _rListBox.InsertEntry( *pEntries, NULL, false, TREELIST_APPEND, new ColumnInfo(*pEntries,sLabel) );
    }
}

FmFieldWinListBox::FmFieldWinListBox( FmFieldWin* pParent )
    :SvTreeListBox( pParent, WB_HASBUTTONS|WB_BORDER )
    ,pTabWin( pParent )
{
    SetHelpId( HID_FIELD_SEL );

    SetHighlightRange( );
}

FmFieldWinListBox::~FmFieldWinListBox()
{
    disposeOnce();
}

void FmFieldWinListBox::dispose()
{
    pTabWin.clear();
    SvTreeListBox::dispose();
}


sal_Int8 FmFieldWinListBox::AcceptDrop( const AcceptDropEvent& /*rEvt*/ )
{
    return DND_ACTION_NONE;
}


sal_Int8 FmFieldWinListBox::ExecuteDrop( const ExecuteDropEvent& /*rEvt*/ )
{
    return DND_ACTION_NONE;
}


bool FmFieldWinListBox::DoubleClickHdl()
{
    if ( pTabWin->createSelectionControls() )
        return true;

    return SvTreeListBox::DoubleClickHdl();
}


void FmFieldWinListBox::StartDrag( sal_Int8 /*_nAction*/, const Point& /*_rPosPixel*/ )
{
    SvTreeListEntry* pSelected = FirstSelected();
    if (!pSelected)
        // no drag without a field
        return;

    svx::ODataAccessDescriptor aDescriptor;
    aDescriptor[ daDataSource ] <<= pTabWin->GetDatabaseName();
    aDescriptor[ daConnection ] <<= pTabWin->GetConnection().getTyped();
    aDescriptor[ daCommand ]    <<= pTabWin->GetObjectName();
    aDescriptor[ daCommandType ]<<= pTabWin->GetObjectType();
    ColumnInfo* pInfo = static_cast<ColumnInfo*>(pSelected->GetUserData());
    aDescriptor[ daColumnName ] <<= pInfo->sColumnName;

    TransferableHelper* pTransferColumn = new OColumnTransferable(
        aDescriptor, ColumnTransferFormatFlags::FIELD_DESCRIPTOR | ColumnTransferFormatFlags::CONTROL_EXCHANGE | ColumnTransferFormatFlags::COLUMN_DESCRIPTOR
    );
    Reference< XTransferable> xEnsureDelete = pTransferColumn;
    EndSelection();
    pTransferColumn->StartDrag( this, DND_ACTION_COPY );
}

FmFieldWinData::FmFieldWinData()
{
}


FmFieldWinData::~FmFieldWinData()
{
}

FmFieldWin::FmFieldWin(SfxBindings* _pBindings, SfxChildWindow* _pMgr, vcl::Window* _pParent)
            :SfxFloatingWindow(_pBindings, _pMgr, _pParent, WinBits(WB_STDMODELESS|WB_SIZEABLE))
            ,SfxControllerItem(SID_FM_FIELDS_CONTROL, *_pBindings)
            ,::comphelper::OPropertyChangeListener(m_aMutex)
            ,pData(new FmFieldWinData)
            ,m_nObjectType(0)
            ,m_pChangeListener(NULL)
{
    SetHelpId( HID_FIELD_SEL_WIN );

    SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetFaceColor()) );
    pListBox = VclPtr<FmFieldWinListBox>::Create( this );
    pListBox->Show();
    UpdateContent(NULL);
    SetSizePixel(Size(STD_WIN_SIZE_X,STD_WIN_SIZE_Y));
}


FmFieldWin::~FmFieldWin()
{
    disposeOnce();
}

void FmFieldWin::dispose()
{
    if (m_pChangeListener)
    {
        m_pChangeListener->dispose();
        m_pChangeListener->release();
        //  delete m_pChangeListener;
    }
    pListBox.disposeAndClear();
    delete pData;
    ::SfxControllerItem::dispose();
    SfxFloatingWindow::dispose();
}


void FmFieldWin::GetFocus()
{
    if ( pListBox )
        pListBox->GrabFocus();
    else
        SfxFloatingWindow::GetFocus();
}


bool FmFieldWin::createSelectionControls( )
{
    SvTreeListEntry* pSelected = pListBox->FirstSelected();
    if ( pSelected )
    {
        // build a descriptor for the currently selected field
        ODataAccessDescriptor aDescr;
        aDescr.setDataSource(GetDatabaseName());

        aDescr[ daConnection ]  <<= GetConnection().getTyped();

        aDescr[ daCommand ]     <<= GetObjectName();
        aDescr[ daCommandType ] <<= GetObjectType();
        ColumnInfo* pInfo = static_cast<ColumnInfo*>(pSelected->GetUserData());
        aDescr[ daColumnName ]  <<= pInfo->sColumnName;//OUString( pListBox->GetEntryText( pSelected) );

        // transfer this to the SFX world
        SfxUnoAnyItem aDescriptorItem( SID_FM_DATACCESS_DESCRIPTOR, makeAny( aDescr.createPropertyValueSequence() ) );
        const SfxPoolItem* pArgs[] =
        {
            &aDescriptorItem, NULL
        };

        // execute the create slot
        GetBindings().Execute( SID_FM_CREATE_FIELDCONTROL, pArgs );
    }

    return NULL != pSelected;
}


bool FmFieldWin::PreNotify( NotifyEvent& _rNEvt )
{
    if ( MouseNotifyEvent::KEYINPUT == _rNEvt.GetType() )
    {
        const vcl::KeyCode& rKeyCode = _rNEvt.GetKeyEvent()->GetKeyCode();
        if ( ( 0 == rKeyCode.GetModifier() ) && ( KEY_RETURN == rKeyCode.GetCode() ) )
        {
            if ( createSelectionControls() )
                return true;
        }
    }

    return SfxFloatingWindow::PreNotify( _rNEvt );
}


bool FmFieldWin::Close()
{
    return SfxFloatingWindow::Close();
}


void FmFieldWin::_propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xForm(evt.Source, ::com::sun::star::uno::UNO_QUERY);
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
        UpdateContent(NULL);
}


void FmFieldWin::UpdateContent(FmFormShell* pShell)
{
    pListBox->Clear();
    OUString aTitle(SVX_RESSTR(RID_STR_FIELDSELECTION));
    SetText( aTitle );

    if (!pShell || !pShell->GetImpl())
        return;

    Reference< XForm >  xForm = pShell->GetImpl()->getCurrentForm();
    if ( xForm.is() )
        UpdateContent( xForm );
}


void FmFieldWin::UpdateContent(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > & xForm)
{
    try
    {
        // delete ListBox
        pListBox->Clear();
        OUString aTitle(SVX_RES(RID_STR_FIELDSELECTION));
        SetText(aTitle);

        if (!xForm.is())
            return;

        Reference< XPreparedStatement >  xStatement;
        Reference< XPropertySet >  xSet(xForm, UNO_QUERY);

        m_aObjectName   = ::comphelper::getString(xSet->getPropertyValue(FM_PROP_COMMAND));
        m_aDatabaseName = ::comphelper::getString(xSet->getPropertyValue(FM_PROP_DATASOURCE));
        m_nObjectType   = ::comphelper::getINT32(xSet->getPropertyValue(FM_PROP_COMMANDTYPE));

        // get the connection of the form
        m_aConnection.reset(
            connectRowset( Reference< XRowSet >( xForm, UNO_QUERY ), ::comphelper::getProcessComponentContext(), true ),
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
                lcl_addToList(*pListBox,xColumns);
        }

        // set prefix
        OUString  aPrefix;
        StringListResource aPrefixes( SVX_RES( RID_RSC_TABWIN_PREFIX ) );

        switch (m_nObjectType)
        {
            case CommandType::TABLE:
                aPrefix = aPrefixes[0];
                break;
            case CommandType::QUERY:
                aPrefix = aPrefixes[1];
                break;
            default:
                aPrefix = aPrefixes[2];
                break;
        }

        // listen for changes at ControlSource in PropertySet
        if (m_pChangeListener)
        {
            m_pChangeListener->dispose();
            m_pChangeListener->release();
        }
        m_pChangeListener = new ::comphelper::OPropertyChangeMultiplexer(this, xSet);
        m_pChangeListener->acquire();
        m_pChangeListener->addProperty(FM_PROP_DATASOURCE);
        m_pChangeListener->addProperty(FM_PROP_COMMAND);
        m_pChangeListener->addProperty(FM_PROP_COMMANDTYPE);

        // Titel setzen
        aTitle = aTitle + " " + aPrefix + " " + OUString(m_aObjectName.getStr());
        SetText( aTitle );
    }
    catch( const Exception& )
    {
        OSL_FAIL( "FmTabWin::UpdateContent: caught an exception!" );
    }
}


void FmFieldWin::Resize()
{
    SfxFloatingWindow::Resize();

    Size aOutputSize( GetOutputSizePixel() );



    // adapt size of ::com::sun::star::form::ListBox
    Point aLBPos( LISTBOX_BORDER, LISTBOX_BORDER );
    Size aLBSize( aOutputSize );
    aLBSize.Width() -= (2*LISTBOX_BORDER);
    aLBSize.Height() -= (2*LISTBOX_BORDER);

    pListBox->SetPosSizePixel( aLBPos, aLBSize );
}


void FmFieldWin::FillInfo( SfxChildWinInfo& rInfo ) const
{
    rInfo.bVisible = false;
}


SFX_IMPL_FLOATINGWINDOW(FmFieldWinMgr, SID_FM_ADD_FIELD)


FmFieldWinMgr::FmFieldWinMgr(vcl::Window* _pParent, sal_uInt16 _nId,
               SfxBindings* _pBindings, SfxChildWinInfo* _pInfo)
              :SfxChildWindow(_pParent, _nId)
{
    SetWindow( VclPtr<FmFieldWin>::Create(_pBindings, this, _pParent) );
    SetHideNotDelete(true);
    static_cast<SfxFloatingWindow*>(GetWindow())->Initialize( _pInfo );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
