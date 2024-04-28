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

#include <browserids.hxx>
#include <com/sun/star/sdbc/XRow.hpp>
#include <connectivity/dbexception.hxx>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <JoinController.hxx>
#include <TableWindowData.hxx>
#include <TableWindow.hxx>
#include <TableConnectionData.hxx>
#include <adtabdlg.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <osl/diagnose.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbc;
using namespace ::dbtools;
using namespace ::comphelper;

namespace dbaui
{

// AddTableDialogContext
class AddTableDialogContext : public IAddTableDialogContext
{
    OJoinController& m_rController;

public:
    explicit AddTableDialogContext( OJoinController& _rController )
        :m_rController( _rController )
    {
    }

    virtual ~AddTableDialogContext() {}

    // IAddTableDialogContext
    virtual css::uno::Reference< css::sdbc::XConnection >
                    getConnection() const override;
    virtual bool    allowViews() const override;
    virtual bool    allowQueries() const override;
    virtual bool    allowAddition() const override;
    virtual void    addTableWindow( const OUString& _rQualifiedTableName, const OUString& _rAliasName ) override;
    virtual void    onWindowClosing() override;

private:
    OJoinTableView* getTableView() const;
};

Reference< XConnection > AddTableDialogContext::getConnection() const
{
    return m_rController.getConnection();
}

bool AddTableDialogContext::allowViews() const
{
    return m_rController.allowViews();
}

bool AddTableDialogContext::allowQueries() const
{
    return m_rController.allowQueries();
}

bool AddTableDialogContext::allowAddition() const
{
    return m_rController.getJoinView()->getTableView()->IsAddAllowed();
}

void AddTableDialogContext::addTableWindow( const OUString& _rQualifiedTableName, const OUString& _rAliasName )
{
    getTableView()->AddTabWin( _rQualifiedTableName, _rAliasName, true );
}

void AddTableDialogContext::onWindowClosing()
{
    if (!m_rController.getView())
        return;
    m_rController.InvalidateFeature( ID_BROWSER_ADDTABLE );
    m_rController.getView()->GrabFocus();
}

OJoinTableView* AddTableDialogContext::getTableView() const
{
    if ( m_rController.getJoinView() )
        return m_rController.getJoinView()->getTableView();
    return nullptr;
}

// OJoinController

OJoinController::OJoinController(const Reference< XComponentContext >& _rM)
    : OJoinController_BASE(_rM)
{
}

OJoinController::~OJoinController()
{
}

OJoinDesignView* OJoinController::getJoinView()
{
    return static_cast< OJoinDesignView* >( getView() );
}

void OJoinController::disposing()
{
    if (m_xAddTableDialog)
    {
        m_xAddTableDialog->response(RET_CLOSE);
        m_xAddTableDialog.reset();
    }

    OJoinController_BASE::disposing();

    clearView();

    m_vTableConnectionData.clear();
    m_vTableData.clear();
}

void OJoinController::reconnect( bool _bUI )
{
    OJoinController_BASE::reconnect( _bUI );
    if ( isConnected() && m_xAddTableDialog )
        m_xAddTableDialog->Update();
}

void OJoinController::impl_onModifyChanged()
{
    OJoinController_BASE::impl_onModifyChanged();
    InvalidateFeature( SID_RELATION_ADD_RELATION );
}

void OJoinController::SaveTabWinPosSize(OTableWindow const * pTabWin, tools::Long nOffsetX, tools::Long nOffsetY)
{
    // the data for the window
    const TTableWindowData::value_type& pData = pTabWin->GetData();
    OSL_ENSURE(pData != nullptr, "SaveTabWinPosSize : TabWin has no data !");

    // set Position & Size of data anew (with current window parameters)
    Point aPos = pTabWin->GetPosPixel();
    aPos.AdjustX(nOffsetX );
    aPos.AdjustY(nOffsetY );
    pData->SetPosition(aPos);
    pData->SetSize(pTabWin->GetSizePixel());

}

FeatureState OJoinController::GetState(sal_uInt16 _nId) const
{
    FeatureState aReturn;
        // (disabled automatically)
    aReturn.bEnabled = true;

    switch (_nId)
    {
        case ID_BROWSER_EDITDOC:
            aReturn.bChecked = isEditable();
            break;
        case ID_BROWSER_ADDTABLE:
            aReturn.bEnabled = ( getView() != nullptr )
                            && const_cast< OJoinController* >( this )->getJoinView()->getTableView()->IsAddAllowed();
            aReturn.bChecked = aReturn.bEnabled && m_xAddTableDialog;
            if ( aReturn.bEnabled )
                aReturn.sTitle = OAddTableDlg::getDialogTitleForContext( impl_getDialogContext() );
            break;

        default:
            aReturn = OJoinController_BASE::GetState(_nId);
    }
    return aReturn;
}

AddTableDialogContext& OJoinController::impl_getDialogContext() const
{
    if (!m_pDialogContext)
    {
        OJoinController* pNonConstThis = const_cast< OJoinController* >( this );
        pNonConstThis->m_pDialogContext.reset( new AddTableDialogContext( *pNonConstThis ) );
    }
    assert(m_pDialogContext && "always exists at this point");
    return *m_pDialogContext;
}

void OJoinController::Execute(sal_uInt16 _nId, const Sequence< PropertyValue >& aArgs)
{
    switch(_nId)
    {
        case ID_BROWSER_EDITDOC:
            if(isEditable())
            { // the state should be changed to not editable
                switch (saveModified())
                {
                    case RET_CANCEL:
                        // don't change anything here so return
                        return;
                    case RET_NO:
                        reset();
                        setModified(false);     // and we are not modified yet
                        break;
                    default:
                        break;
                }
            }
            setEditable(!isEditable());
            getJoinView()->setReadOnly(!isEditable());
            InvalidateAll();
            return;
        case ID_BROWSER_ADDTABLE:
            if (m_xAddTableDialog)
            {
                m_xAddTableDialog->response(RET_CLOSE);
                getView()->GrabFocus();
            }
            else
            {
                runDialogAsync();
            }
            break;
        default:
            OJoinController_BASE::Execute(_nId,aArgs);
    }
    InvalidateFeature(_nId);
}

void OJoinController::runDialogAsync()
{
    assert(!m_xAddTableDialog);
    m_xAddTableDialog = std::make_shared<OAddTableDlg>(getFrameWeld(), impl_getDialogContext());
    {
        weld::WaitObject aWaitCursor(getFrameWeld());
        m_xAddTableDialog->Update();
    }
    weld::DialogController::runAsync(m_xAddTableDialog, [this](sal_Int32 /*nResult*/){
        m_xAddTableDialog->OnClose();
        m_xAddTableDialog.reset();
    });
}

void OJoinController::SaveTabWinsPosSize( OJoinTableView::OTableWindowMap* pTabWinList, tools::Long nOffsetX, tools::Long nOffsetY )
{
    // Deletion and recreation of the old implementation with the current model is not correct anymore:
    // The TabWins have a pointer to their data, but they are managed by me. When I delete the old ones, the TabWins suddenly have a pointer to objects, which no longer exist.
    // If the TabWins had a SetData, I could save that effort... but they don't, further I also would still have to set information anew, which actually didn't change.
    // So I don't delete the TabWinDatas, but only update them.
    OSL_ENSURE(m_vTableData.size() == pTabWinList->size(),
        "OJoinController::SaveTabWinsPosSize : inconsistent state : should have as many TabWinDatas as TabWins !");

    for (auto const& tabWin : *pTabWinList)
        SaveTabWinPosSize(tabWin.second, nOffsetX, nOffsetY);
}

void OJoinController::removeConnectionData(const TTableConnectionData::value_type& _pData)
{
    std::erase(m_vTableConnectionData, _pData);
}

void OJoinController::describeSupportedFeatures()
{
    OJoinController_BASE::describeSupportedFeatures();
    implDescribeSupportedFeature( ".uno:Redo",      ID_BROWSER_REDO,    CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:Save",      ID_BROWSER_SAVEDOC, CommandGroup::DOCUMENT );
    implDescribeSupportedFeature( ".uno:Undo",      ID_BROWSER_UNDO,    CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:AddTable",  ID_BROWSER_ADDTABLE,CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:EditDoc",   ID_BROWSER_EDITDOC, CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:GetUndoStrings", SID_GETUNDOSTRINGS );
    implDescribeSupportedFeature( ".uno:GetRedoStrings", SID_GETREDOSTRINGS );
}

sal_Bool SAL_CALL OJoinController::suspend(sal_Bool _bSuspend)
{
    if ( getBroadcastHelper().bInDispose || getBroadcastHelper().bDisposed )
        return true;

    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );
    if ( getView() && getView()->IsInModalMode() )
        return false;
    bool bCheck = true;
    if ( _bSuspend )
    {
        bCheck = saveModified() != RET_CANCEL;
        if ( bCheck )
            OSingleDocumentController::suspend(_bSuspend);
    }
    return bCheck;
}

void OJoinController::loadTableWindows( const ::comphelper::NamedValueCollection& i_rViewSettings )
{
    m_vTableData.clear();

    m_aMinimumTableViewSize = Point();

    for (auto& table : i_rViewSettings.getOrDefault("Tables", Sequence<PropertyValue>()))
    {
        ::comphelper::NamedValueCollection aSingleTableData(table.Value);
        loadTableWindow( aSingleTableData );
    }
    if ( m_aMinimumTableViewSize != Point() )
    {
        getJoinView()->getScrollHelper()->resetRange( m_aMinimumTableViewSize );
    }
}

void OJoinController::loadTableWindow( const ::comphelper::NamedValueCollection& i_rTableWindowSettings )
{
    sal_Int32 nX = -1, nY = -1, nHeight = -1, nWidth = -1;

    OUString sComposedName,sTableName,sWindowName;
    bool bShowAll = false;

    sComposedName = i_rTableWindowSettings.getOrDefault( "ComposedName", sComposedName );
    sTableName = i_rTableWindowSettings.getOrDefault( "TableName", sTableName );
    sWindowName = i_rTableWindowSettings.getOrDefault( "WindowName", sWindowName );
    nY = i_rTableWindowSettings.getOrDefault( "WindowTop", nY );
    nX = i_rTableWindowSettings.getOrDefault( "WindowLeft", nX );
    nWidth = i_rTableWindowSettings.getOrDefault( "WindowWidth", nWidth );
    nHeight = i_rTableWindowSettings.getOrDefault( "WindowHeight", nHeight );
    bShowAll = i_rTableWindowSettings.getOrDefault( "ShowAll", bShowAll );

    TTableWindowData::value_type pData = createTableWindowData(sComposedName,sTableName,sWindowName);
    if ( pData )
    {
        pData->SetPosition(Point(nX,nY));
        pData->SetSize( Size( nWidth, nHeight ) );
        pData->ShowAll(bShowAll);
        m_vTableData.push_back(pData);
        if ( m_aMinimumTableViewSize.X() < (nX+nWidth) )
            m_aMinimumTableViewSize.setX( nX+nWidth );
        if ( m_aMinimumTableViewSize.Y() < (nY+nHeight) )
            m_aMinimumTableViewSize.setY( nY+nHeight );
    }
}

void OJoinController::saveTableWindows( ::comphelper::NamedValueCollection& o_rViewSettings ) const
{
    if ( m_vTableData.empty() )
        return;

    ::comphelper::NamedValueCollection aAllTablesData;

    sal_Int32 i = 1;
    for (auto const& elem : m_vTableData)
    {
        ::comphelper::NamedValueCollection aWindowData;
        aWindowData.put( "ComposedName", elem->GetComposedName() );
        aWindowData.put( "TableName", elem->GetTableName() );
        aWindowData.put( "WindowName", elem->GetWinName() );
        aWindowData.put( "WindowTop", static_cast<sal_Int32>(elem->GetPosition().Y()) );
        aWindowData.put( "WindowLeft", static_cast<sal_Int32>(elem->GetPosition().X()) );
        aWindowData.put( "WindowWidth", static_cast<sal_Int32>(elem->GetSize().Width()) );
        aWindowData.put( "WindowHeight", static_cast<sal_Int32>(elem->GetSize().Height()) );
        aWindowData.put( "ShowAll", elem->IsShowAll() );

        const OUString sTableName( "Table" + OUString::number( i++ ) );
        aAllTablesData.put( sTableName, aWindowData.getPropertyValues() );
    }

    o_rViewSettings.put( "Tables", aAllTablesData.getPropertyValues() );
}

TTableWindowData::value_type OJoinController::createTableWindowData(const OUString& _sComposedName,const OUString& _sTableName,const OUString& _sWindowName)
{
    OJoinDesignView* pView = getJoinView();
    if( pView )
        return pView->getTableView()->createTableWindowData(_sComposedName,_sTableName,_sWindowName);
    OSL_FAIL("We should never ever reach this point!");

    return TTableWindowData::value_type();
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
