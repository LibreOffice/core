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

#include <svx/fmgridif.hxx>
#include <fmprop.hxx>
#include <svx/fmtools.hxx>
#include <fmservs.hxx>
#include <fmurl.hxx>
#include <formcontrolfactory.hxx>
#include <gridcell.hxx>
#include <gridcols.hxx>
#include <svx/dbaexchange.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/fmgridcl.hxx>
#include <svx/svxdlg.hxx>
#include <svx/svxids.hrc>
#include <bitmaps.hlst>
#include <sdbdatacolumn.hxx>

#include <com/sun/star/form/XConfirmDeleteListener.hpp>
#include <com/sun/star/form/XFormComponent.hpp>
#include <com/sun/star/form/XGridColumnFactory.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/RowChangeAction.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XDeleteRows.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/property.hxx>
#include <comphelper/string.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/eitem.hxx>
#include <vcl/fmtfield.hxx>
#include <svl/numuno.hxx>
#include <tools/debug.hxx>
#include <tools/multisel.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/help.hxx>
#include <vcl/image.hxx>
#include <vcl/longcurr.hxx>
#include <vcl/menu.hxx>
#include <vcl/settings.hxx>
#include <sal/log.hxx>

#include <math.h>
#include <memory>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::cppu;
using namespace ::svxform;
using namespace ::svx;
using namespace ::dbtools;

struct FmGridHeaderData
{
    ODataAccessDescriptor   aDropData;
    Point                   aDropPosPixel;
    sal_Int8                nDropAction;
    Reference< XInterface > xDroppedStatement;
    Reference< XInterface > xDroppedResultSet;
};

static void SetMenuItem(const OUString& rImgID, const OString &rID, Menu& rMenu, bool bDesignMode)
{
    Image aImage(StockImage::Yes, rImgID);
    sal_uInt16 nID = rMenu.GetItemId(rID);
    rMenu.SetItemImage(nID, aImage);
    rMenu.EnableItem(nID, bDesignMode);
}

FmGridHeader::FmGridHeader( BrowseBox* pParent, WinBits nWinBits)
        :EditBrowserHeader(pParent, nWinBits)
        ,DropTargetHelper(this)
        ,m_pImpl(new FmGridHeaderData)
{
}

FmGridHeader::~FmGridHeader()
{
    disposeOnce();
}

void FmGridHeader::dispose()
{
    m_pImpl.reset();
    DropTargetHelper::dispose();
    svt::EditBrowserHeader::dispose();
}

sal_uInt16 FmGridHeader::GetModelColumnPos(sal_uInt16 nId) const
{
    return static_cast<FmGridControl*>(GetParent())->GetModelColumnPos(nId);
}

void FmGridHeader::notifyColumnSelect(sal_uInt16 nColumnId)
{
    sal_uInt16 nPos = GetModelColumnPos(nColumnId);
    Reference< XIndexAccess >  xColumns(static_cast<FmGridControl*>(GetParent())->GetPeer()->getColumns(), UNO_QUERY);
    if ( nPos < xColumns->getCount() )
    {
        Reference< XSelectionSupplier >  xSelSupplier(xColumns, UNO_QUERY);
        if ( xSelSupplier.is() )
        {
            Reference< XPropertySet >  xColumn;
            xColumns->getByIndex(nPos) >>= xColumn;
            xSelSupplier->select(makeAny(xColumn));
        }
    }
}

void FmGridHeader::Select()
{
    EditBrowserHeader::Select();
    notifyColumnSelect(GetCurItemId());
}

void FmGridHeader::RequestHelp( const HelpEvent& rHEvt )
{
    sal_uInt16 nItemId = GetItemId( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );
    if ( nItemId )
    {
        if ( rHEvt.GetMode() & (HelpEventMode::QUICK | HelpEventMode::BALLOON) )
        {
            tools::Rectangle aItemRect = GetItemRect( nItemId );
            Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
            aItemRect.SetLeft( aPt.X() );
            aItemRect.SetTop( aPt.Y() );
            aPt = OutputToScreenPixel( aItemRect.BottomRight() );
            aItemRect.SetRight( aPt.X() );
            aItemRect.SetBottom( aPt.Y() );

            sal_uInt16 nPos = GetModelColumnPos(nItemId);
            Reference< css::container::XIndexContainer >  xColumns(static_cast<FmGridControl*>(GetParent())->GetPeer()->getColumns());
            try
            {
                Reference< css::beans::XPropertySet >  xColumn(xColumns->getByIndex(nPos),UNO_QUERY);
                OUString aHelpText;
                xColumn->getPropertyValue(FM_PROP_HELPTEXT) >>= aHelpText;
                if ( aHelpText.isEmpty() )
                    xColumn->getPropertyValue(FM_PROP_DESCRIPTION) >>= aHelpText;
                if ( !aHelpText.isEmpty() )
                {
                    if ( rHEvt.GetMode() & HelpEventMode::BALLOON )
                        Help::ShowBalloon( this, aItemRect.Center(), aItemRect, aHelpText );
                    else
                        Help::ShowQuickHelp( this, aItemRect, aHelpText );
                    return;
                }
            }
            catch(Exception&)
            {
                return;
            }
        }
    }
    EditBrowserHeader::RequestHelp( rHEvt );
}

sal_Int8 FmGridHeader::AcceptDrop( const AcceptDropEvent& rEvt )
{
    // drop allowed in design mode only
    if (!static_cast<FmGridControl*>(GetParent())->IsDesignMode())
        return DND_ACTION_NONE;

    // search for recognized formats
    const DataFlavorExVector& rFlavors = GetDataFlavorExVector();
    if (OColumnTransferable::canExtractColumnDescriptor(rFlavors, ColumnTransferFormatFlags::COLUMN_DESCRIPTOR | ColumnTransferFormatFlags::FIELD_DESCRIPTOR))
        return rEvt.mnAction;

    return DND_ACTION_NONE;
}

sal_Int8 FmGridHeader::ExecuteDrop( const ExecuteDropEvent& _rEvt )
{
    if (!static_cast<FmGridControl*>(GetParent())->IsDesignMode())
        return DND_ACTION_NONE;

    TransferableDataHelper aDroppedData(_rEvt.maDropEvent.Transferable);

    // check the formats
    bool bColumnDescriptor  = OColumnTransferable::canExtractColumnDescriptor(aDroppedData.GetDataFlavorExVector(), ColumnTransferFormatFlags::COLUMN_DESCRIPTOR);
    bool bFieldDescriptor   = OColumnTransferable::canExtractColumnDescriptor(aDroppedData.GetDataFlavorExVector(), ColumnTransferFormatFlags::FIELD_DESCRIPTOR);
    if (!bColumnDescriptor && !bFieldDescriptor)
    {
        OSL_FAIL("FmGridHeader::ExecuteDrop: should never have reached this (no extractable format)!");
        return DND_ACTION_NONE;
    }

    // extract the descriptor
    OUString sDatasource, sCommand, sFieldName,sDatabaseLocation;
    sal_Int32       nCommandType = CommandType::COMMAND;
    Reference< XPreparedStatement >     xStatement;
    Reference< XResultSet >             xResultSet;
    Reference< XPropertySet >           xField;
    Reference< XConnection >            xConnection;

    ODataAccessDescriptor aColumn = OColumnTransferable::extractColumnDescriptor(aDroppedData);
    if (aColumn.has(DataAccessDescriptorProperty::DataSource))  aColumn[DataAccessDescriptorProperty::DataSource]   >>= sDatasource;
    if (aColumn.has(DataAccessDescriptorProperty::DatabaseLocation))    aColumn[DataAccessDescriptorProperty::DatabaseLocation] >>= sDatabaseLocation;
    if (aColumn.has(DataAccessDescriptorProperty::Command))     aColumn[DataAccessDescriptorProperty::Command]      >>= sCommand;
    if (aColumn.has(DataAccessDescriptorProperty::CommandType)) aColumn[DataAccessDescriptorProperty::CommandType]  >>= nCommandType;
    if (aColumn.has(DataAccessDescriptorProperty::ColumnName))  aColumn[DataAccessDescriptorProperty::ColumnName]   >>= sFieldName;
    if (aColumn.has(DataAccessDescriptorProperty::ColumnObject))aColumn[DataAccessDescriptorProperty::ColumnObject] >>= xField;
    if (aColumn.has(DataAccessDescriptorProperty::Connection))  aColumn[DataAccessDescriptorProperty::Connection]   >>= xConnection;

    if  (   sFieldName.isEmpty()
        ||  sCommand.isEmpty()
        ||  (   sDatasource.isEmpty()
            &&  sDatabaseLocation.isEmpty()
            &&  !xConnection.is()
            )
        )
    {
        OSL_FAIL( "FmGridHeader::ExecuteDrop: somebody started a nonsense drag operation!!" );
        return DND_ACTION_NONE;
    }

    try
    {
        // need a connection
        if (!xConnection.is())
        {   // the transferable did not contain the connection -> build an own one
            try
            {
                OUString sSignificantSource( sDatasource.isEmpty() ? sDatabaseLocation : sDatasource );
                xConnection = getConnection_withFeedback(sSignificantSource, OUString(), OUString(),
                                  static_cast<FmGridControl*>(GetParent())->getContext() );
            }
            catch(NoSuchElementException&)
            {   // allowed, means sDatasource isn't a valid data source name ....
            }
            catch(Exception&)
            {
                OSL_FAIL("FmGridHeader::ExecuteDrop: could not retrieve the database access object !");
            }

            if (!xConnection.is())
            {
                OSL_FAIL("FmGridHeader::ExecuteDrop: could not retrieve the database access object !");
                return DND_ACTION_NONE;
            }
        }

        // try to obtain the column object
        if (!xField.is())
        {
#ifdef DBG_UTIL
            Reference< XServiceInfo >  xServiceInfo(xConnection, UNO_QUERY);
            DBG_ASSERT(xServiceInfo.is() && xServiceInfo->supportsService(SRV_SDB_CONNECTION), "FmGridHeader::ExecuteDrop: invalid connection (no database access connection !)");
#endif

            Reference< XNameAccess > xFields;
            switch (nCommandType)
            {
                case CommandType::TABLE:
                {
                    Reference< XTablesSupplier > xSupplyTables(xConnection, UNO_QUERY);
                    Reference< XColumnsSupplier >  xSupplyColumns;
                    xSupplyTables->getTables()->getByName(sCommand) >>= xSupplyColumns;
                    xFields = xSupplyColumns->getColumns();
                }
                break;
                case CommandType::QUERY:
                {
                    Reference< XQueriesSupplier > xSupplyQueries(xConnection, UNO_QUERY);
                    Reference< XColumnsSupplier > xSupplyColumns;
                    xSupplyQueries->getQueries()->getByName(sCommand) >>= xSupplyColumns;
                    xFields  = xSupplyColumns->getColumns();
                }
                break;
                default:
                {
                    xStatement = xConnection->prepareStatement(sCommand);
                    // not interested in any results

                    Reference< XPropertySet > xStatProps(xStatement,UNO_QUERY);
                    xStatProps->setPropertyValue("MaxRows", makeAny(sal_Int32(0)));

                    xResultSet = xStatement->executeQuery();
                    Reference< XColumnsSupplier >  xSupplyCols(xResultSet, UNO_QUERY);
                    if (xSupplyCols.is())
                        xFields = xSupplyCols->getColumns();
                }
            }

            if (xFields.is() && xFields->hasByName(sFieldName))
                xFields->getByName(sFieldName) >>= xField;

            if (!xField.is())
            {
                ::comphelper::disposeComponent(xStatement);
                return DND_ACTION_NONE;
            }
        }

        // do the drop asynchronously
        // (85957 - UI actions within the drop are not allowed, but we want to open a popup menu)
        m_pImpl->aDropData = aColumn;
        m_pImpl->aDropData[DataAccessDescriptorProperty::Connection] <<= xConnection;
        m_pImpl->aDropData[DataAccessDescriptorProperty::ColumnObject] <<= xField;

        m_pImpl->nDropAction = _rEvt.mnAction;
        m_pImpl->aDropPosPixel = _rEvt.maPosPixel;
        m_pImpl->xDroppedStatement = xStatement;
        m_pImpl->xDroppedResultSet = xResultSet;

        PostUserEvent(LINK(this, FmGridHeader, OnAsyncExecuteDrop), nullptr, true);
    }
    catch (Exception&)
    {
        OSL_FAIL("FmGridHeader::ExecuteDrop: caught an exception while creatin' the column !");
        ::comphelper::disposeComponent(xStatement);
        return DND_ACTION_NONE;
    }

    return DND_ACTION_LINK;
}

IMPL_LINK_NOARG( FmGridHeader, OnAsyncExecuteDrop, void*, void )
{
    OUString             sCommand, sFieldName,sURL;
    sal_Int32                   nCommandType = CommandType::COMMAND;
    Reference< XPropertySet >   xField;
    Reference< XConnection >    xConnection;

    OUString sDatasource = m_pImpl->aDropData.getDataSource();
    if ( sDatasource.isEmpty() && m_pImpl->aDropData.has(DataAccessDescriptorProperty::ConnectionResource) )
        m_pImpl->aDropData[DataAccessDescriptorProperty::ConnectionResource]    >>= sURL;
    m_pImpl->aDropData[DataAccessDescriptorProperty::Command]       >>= sCommand;
    m_pImpl->aDropData[DataAccessDescriptorProperty::CommandType]   >>= nCommandType;
    m_pImpl->aDropData[DataAccessDescriptorProperty::ColumnName]    >>= sFieldName;
    m_pImpl->aDropData[DataAccessDescriptorProperty::Connection]    >>= xConnection;
    m_pImpl->aDropData[DataAccessDescriptorProperty::ColumnObject]  >>= xField;

    try
    {
        // need number formats
        Reference< XNumberFormatsSupplier > xSupplier = getNumberFormats(xConnection, true);
        Reference< XNumberFormats >  xNumberFormats;
        if (xSupplier.is())
            xNumberFormats = xSupplier->getNumberFormats();
        if (!xNumberFormats.is())
        {
            ::comphelper::disposeComponent(m_pImpl->xDroppedResultSet);
            ::comphelper::disposeComponent(m_pImpl->xDroppedStatement);
            return;
        }

        // The field now needs two pieces of information:
        // a.) Name of the field for label and ControlSource
        // b.) FormatKey, to determine which field is to be created
        sal_Int32 nDataType = 0;
        xField->getPropertyValue(FM_PROP_FIELDTYPE) >>= nDataType;
        // these datatypes can not be processed in Gridcontrol
        switch (nDataType)
        {
            case DataType::BLOB:
            case DataType::LONGVARBINARY:
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::OTHER:
                ::comphelper::disposeComponent(m_pImpl->xDroppedResultSet);
                ::comphelper::disposeComponent(m_pImpl->xDroppedStatement);
                return;
        }

        // Creating the column
        Reference< XIndexContainer >  xCols(static_cast<FmGridControl*>(GetParent())->GetPeer()->getColumns());
        Reference< XGridColumnFactory >  xFactory(xCols, UNO_QUERY);

        sal_uInt16 nColId = GetItemId(m_pImpl->aDropPosPixel);
        // insert position, always before the current column
        sal_uInt16 nPos = GetModelColumnPos(nColId);
        Reference< XPropertySet >  xCol, xSecondCol;

        // Create Column based on type, default textfield
        std::vector<OString> aPossibleTypes;
        std::vector<OUString> aImgResId;
        switch (nDataType)
        {
            case DataType::BIT:
            case DataType::BOOLEAN:
                aPossibleTypes.emplace_back(FM_COL_CHECKBOX);
                aImgResId.emplace_back(RID_SVXBMP_CHECKBOX);
                break;
            case DataType::TINYINT:
            case DataType::SMALLINT:
            case DataType::INTEGER:
                aPossibleTypes.emplace_back(FM_COL_NUMERICFIELD);
                aImgResId.emplace_back(RID_SVXBMP_NUMERICFIELD);
                aPossibleTypes.emplace_back(FM_COL_FORMATTEDFIELD);
                aImgResId.emplace_back(RID_SVXBMP_FORMATTEDFIELD);
                break;
            case DataType::REAL:
            case DataType::DOUBLE:
            case DataType::NUMERIC:
            case DataType::DECIMAL:
                aPossibleTypes.emplace_back(FM_COL_FORMATTEDFIELD);
                aImgResId.emplace_back(RID_SVXBMP_FORMATTEDFIELD);
                aPossibleTypes.emplace_back(FM_COL_NUMERICFIELD);
                aImgResId.emplace_back(RID_SVXBMP_NUMERICFIELD);
                break;
            case DataType::TIMESTAMP:
                aPossibleTypes.emplace_back("dateandtimefield");
                aImgResId.emplace_back(RID_SVXBMP_DATE_N_TIME_FIELDS);
                aPossibleTypes.emplace_back(FM_COL_DATEFIELD);
                aImgResId.emplace_back(RID_SVXBMP_DATEFIELD);
                aPossibleTypes.emplace_back(FM_COL_TIMEFIELD);
                aImgResId.emplace_back(RID_SVXBMP_TIMEFIELD);
                aPossibleTypes.emplace_back(FM_COL_FORMATTEDFIELD);
                aImgResId.emplace_back(RID_SVXBMP_FORMATTEDFIELD);
                break;
            case DataType::DATE:
                aPossibleTypes.emplace_back(FM_COL_DATEFIELD);
                aImgResId.emplace_back(RID_SVXBMP_DATEFIELD);
                aPossibleTypes.emplace_back(FM_COL_FORMATTEDFIELD);
                aImgResId.emplace_back(RID_SVXBMP_FORMATTEDFIELD);
                break;
            case DataType::TIME:
                aPossibleTypes.emplace_back(FM_COL_TIMEFIELD);
                aImgResId.emplace_back(RID_SVXBMP_TIMEFIELD);
                aPossibleTypes.emplace_back(FM_COL_FORMATTEDFIELD);
                aImgResId.emplace_back(RID_SVXBMP_FORMATTEDFIELD);
                break;
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::LONGVARCHAR:
            default:
                aPossibleTypes.emplace_back(FM_COL_TEXTFIELD);
                aImgResId.emplace_back(RID_SVXBMP_EDITBOX);
                aPossibleTypes.emplace_back(FM_COL_FORMATTEDFIELD);
                aImgResId.emplace_back(RID_SVXBMP_FORMATTEDFIELD);
                break;
        }
        // if it's a currency field, a "currency field" option
        try
        {
            if  (   ::comphelper::hasProperty(FM_PROP_ISCURRENCY, xField)
                &&  ::comphelper::getBOOL(xField->getPropertyValue(FM_PROP_ISCURRENCY)))
            {
                aPossibleTypes.insert(aPossibleTypes.begin(), FM_COL_CURRENCYFIELD);
                aImgResId.insert(aImgResId.begin(), RID_SVXBMP_CURRENCYFIELD);
            }
        }
        catch (const Exception&)
        {
            OSL_FAIL("FmGridHeader::ExecuteDrop: Exception occurred!");
        }

        assert(aPossibleTypes.size() == aImgResId.size());

        bool bDateNTimeCol = false;
        if (!aPossibleTypes.empty())
        {
            OString sPreferredType = aPossibleTypes[0];
            if ((m_pImpl->nDropAction == DND_ACTION_LINK) && (aPossibleTypes.size() > 1))
            {
                VclBuilder aBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "svx/ui/colsmenu.ui", "");
                VclPtr<PopupMenu> aInsertMenu(aBuilder.get_menu("menu"));
                PopupMenu* pTypeMenu = aInsertMenu->GetPopupMenu(aInsertMenu->GetItemId("insert"));
                pTypeMenu->ShowItem(pTypeMenu->GetItemId("dateandtimefield"));
                std::vector<OString>::const_iterator iter;
                std::vector<OUString>::const_iterator imgiter;
                for (iter = aPossibleTypes.begin(), imgiter = aImgResId.begin();
                     iter != aPossibleTypes.end(); ++iter, ++imgiter)
                {
                    SetMenuItem(*imgiter, *iter, *pTypeMenu, true);
                }
                if (pTypeMenu->Execute(this, m_pImpl->aDropPosPixel))
                    sPreferredType = pTypeMenu->GetCurItemIdent();
            }

            bDateNTimeCol = sPreferredType == "dateandtimefield";
            sal_uInt16 nColCount = bDateNTimeCol ? 2 : 1;
            OUString sFieldService;
            while (nColCount--)
            {
                if (bDateNTimeCol)
                    sPreferredType = nColCount ? FM_COL_DATEFIELD : FM_COL_TIMEFIELD;

                sFieldService = OUString::fromUtf8(sPreferredType);
                Reference< XPropertySet >  xThisRoundCol;
                if ( !sFieldService.isEmpty() )
                    xThisRoundCol = xFactory->createColumn(sFieldService);
                if (nColCount)
                    xSecondCol = xThisRoundCol;
                else
                    xCol = xThisRoundCol;
            }
        }

        if (!xCol.is() || (bDateNTimeCol && !xSecondCol.is()))
        {
            ::comphelper::disposeComponent(xCol);   // in case only the creation of the second column failed
            ::comphelper::disposeComponent(m_pImpl->xDroppedResultSet);
            ::comphelper::disposeComponent(m_pImpl->xDroppedStatement);
            return;
        }

        if (bDateNTimeCol)
        {
            OUString sTimePostfix(SvxResId(RID_STR_POSTFIX_TIME));
            xCol->setPropertyValue(FM_PROP_LABEL, makeAny( OUString( sFieldName + sTimePostfix ) ) );

            OUString sDatePostfix(SvxResId( RID_STR_POSTFIX_DATE));
            xSecondCol->setPropertyValue(FM_PROP_LABEL, makeAny( OUString( sFieldName + sDatePostfix ) ) );
        }
        else
            xCol->setPropertyValue(FM_PROP_LABEL, makeAny(sFieldName));

        // insert now
        Any aElement;
        aElement <<= xCol;

        xCols->insertByIndex(nPos, aElement);

        FormControlFactory aControlFactory;
        aControlFactory.initializeControlModel( DocumentClassification::classifyHostDocument( xCols ), xCol );
        FormControlFactory::initializeFieldDependentProperties( xField, xCol, xNumberFormats );

        xCol->setPropertyValue(FM_PROP_CONTROLSOURCE, makeAny(sFieldName));
        if ( xSecondCol.is() )
            xSecondCol->setPropertyValue(FM_PROP_CONTROLSOURCE, makeAny(sFieldName));

        if (bDateNTimeCol)
        {
            OUString aPostfix[] = {
                SvxResId(RID_STR_POSTFIX_DATE),
                SvxResId(RID_STR_POSTFIX_TIME)
            };

            for ( size_t i=0; i<2; ++i )
            {
                OUString sPurePostfix = comphelper::string::stripStart(aPostfix[i], ' ');
                sPurePostfix = comphelper::string::stripStart(sPurePostfix, '(');
                sPurePostfix = comphelper::string::stripEnd(sPurePostfix, ')');
                OUString sRealName = sFieldName + "_" + sPurePostfix;
                if (i)
                    xSecondCol->setPropertyValue(FM_PROP_NAME, makeAny(sRealName));
                else
                    xCol->setPropertyValue(FM_PROP_NAME, makeAny(sRealName));
            }
        }
        else
            xCol->setPropertyValue(FM_PROP_NAME, makeAny(sFieldName));

        if (bDateNTimeCol)
        {
            aElement <<= xSecondCol;
            xCols->insertByIndex(nPos == sal_uInt16(-1) ? nPos : ++nPos, aElement);
        }

        // is the component::Form tied to the database?
        Reference< XFormComponent >  xFormCp(xCols, UNO_QUERY);
        Reference< XPropertySet >  xForm(xFormCp->getParent(), UNO_QUERY);
        if (xForm.is())
        {
            if (::comphelper::getString(xForm->getPropertyValue(FM_PROP_DATASOURCE)).isEmpty())
            {
                if ( !sDatasource.isEmpty() )
                    xForm->setPropertyValue(FM_PROP_DATASOURCE, makeAny(sDatasource));
                else
                    xForm->setPropertyValue(FM_PROP_URL, makeAny(sURL));
            }

            if (::comphelper::getString(xForm->getPropertyValue(FM_PROP_COMMAND)).isEmpty())
            {
                xForm->setPropertyValue(FM_PROP_COMMAND, makeAny(sCommand));
                Any aCommandType;
                switch (nCommandType)
                {
                    case CommandType::TABLE:
                        aCommandType <<= sal_Int32(CommandType::TABLE);
                        break;
                    case CommandType::QUERY:
                        aCommandType <<= sal_Int32(CommandType::QUERY);
                        break;
                    default:
                        aCommandType <<= sal_Int32(CommandType::COMMAND);
                        xForm->setPropertyValue(FM_PROP_ESCAPE_PROCESSING, css::uno::Any(2 == nCommandType));
                        break;
                }
                xForm->setPropertyValue(FM_PROP_COMMANDTYPE, aCommandType);
            }
        }
    }
    catch (Exception&)
    {
        OSL_FAIL("FmGridHeader::OnAsyncExecuteDrop: caught an exception while creatin' the column !");
        ::comphelper::disposeComponent(m_pImpl->xDroppedResultSet);
        ::comphelper::disposeComponent(m_pImpl->xDroppedStatement);
        return;
    }

    ::comphelper::disposeComponent(m_pImpl->xDroppedResultSet);
    ::comphelper::disposeComponent(m_pImpl->xDroppedStatement);
}

void FmGridHeader::PreExecuteColumnContextMenu(sal_uInt16 nColId, PopupMenu& rMenu)
{
    bool bDesignMode = static_cast<FmGridControl*>(GetParent())->IsDesignMode();

    Reference< css::container::XIndexContainer >  xCols(static_cast<FmGridControl*>(GetParent())->GetPeer()->getColumns());
    // Building of the Insert Menu
    // mark the column if nColId != HEADERBAR_ITEM_NOTFOUND
    if(nColId > 0)
    {
        sal_uInt16 nPos2 = GetModelColumnPos(nColId);

        Reference< css::container::XIndexContainer >  xColumns(static_cast<FmGridControl*>(GetParent())->GetPeer()->getColumns());
        Reference< css::beans::XPropertySet>          xColumn( xColumns->getByIndex(nPos2), css::uno::UNO_QUERY);
        Reference< css::view::XSelectionSupplier >    xSelSupplier(xColumns, UNO_QUERY);
        if (xSelSupplier.is())
            xSelSupplier->select(makeAny(xColumn));
    }

    // insert position, always before the current column
    sal_uInt16 nPos = GetModelColumnPos(nColId);
    bool bMarked = nColId && static_cast<FmGridControl*>(GetParent())->isColumnMarked(nColId);

    PopupMenu* pMenu = rMenu.GetPopupMenu(rMenu.GetItemId("insert"));
    if (pMenu)
    {
        SetMenuItem(RID_SVXBMP_EDITBOX, FM_COL_TEXTFIELD, *pMenu, bDesignMode);
        SetMenuItem(RID_SVXBMP_CHECKBOX, FM_COL_CHECKBOX, *pMenu, bDesignMode);
        SetMenuItem(RID_SVXBMP_COMBOBOX, FM_COL_COMBOBOX, *pMenu, bDesignMode);
        SetMenuItem(RID_SVXBMP_LISTBOX, FM_COL_LISTBOX, *pMenu, bDesignMode);
        SetMenuItem(RID_SVXBMP_DATEFIELD, FM_COL_DATEFIELD, *pMenu, bDesignMode);
        SetMenuItem(RID_SVXBMP_TIMEFIELD, FM_COL_TIMEFIELD, *pMenu, bDesignMode);
        SetMenuItem(RID_SVXBMP_NUMERICFIELD, FM_COL_NUMERICFIELD, *pMenu, bDesignMode);
        SetMenuItem(RID_SVXBMP_CURRENCYFIELD, FM_COL_CURRENCYFIELD, *pMenu, bDesignMode);
        SetMenuItem(RID_SVXBMP_PATTERNFIELD, FM_COL_PATTERNFIELD, *pMenu, bDesignMode);
        SetMenuItem(RID_SVXBMP_FORMATTEDFIELD, FM_COL_FORMATTEDFIELD, *pMenu, bDesignMode);
    }

    if (pMenu && xCols.is() && nColId)
    {
        Reference< css::beans::XPropertySet > xPropSet( xCols->getByIndex(nPos), css::uno::UNO_QUERY);

        Reference< css::io::XPersistObject >  xServiceQuestion(xPropSet, UNO_QUERY);
        sal_Int32 nColType = xServiceQuestion.is() ? getColumnTypeByModelName(xServiceQuestion->getServiceName()) : 0;
        if (nColType == TYPE_TEXTFIELD)
        {   // edit fields and formatted fields have the same service name, thus getColumnTypeByModelName returns TYPE_TEXTFIELD
            // in both cases. And as columns don't have an css::lang::XServiceInfo interface, we have to distinguish both
            // types via the existence of special properties
            if (xPropSet.is())
            {
                Reference< css::beans::XPropertySetInfo >  xPropsInfo = xPropSet->getPropertySetInfo();
                if (xPropsInfo.is() && xPropsInfo->hasPropertyByName(FM_PROP_FORMATSSUPPLIER))
                    nColType = TYPE_FORMATTEDFIELD;
            }
        }

        PopupMenu* pControlMenu = rMenu.GetPopupMenu(rMenu.GetItemId("change"));
        if (pControlMenu)
        {
            SetMenuItem(RID_SVXBMP_EDITBOX, FM_COL_TEXTFIELD"1", *pControlMenu, bDesignMode && (nColType != TYPE_TEXTFIELD));
            SetMenuItem(RID_SVXBMP_CHECKBOX, FM_COL_CHECKBOX"1", *pControlMenu, bDesignMode && (nColType != TYPE_CHECKBOX));
            SetMenuItem(RID_SVXBMP_COMBOBOX, FM_COL_COMBOBOX"1", *pControlMenu, bDesignMode && (nColType != TYPE_COMBOBOX));
            SetMenuItem(RID_SVXBMP_LISTBOX, FM_COL_LISTBOX"1", *pControlMenu, bDesignMode && (nColType != TYPE_LISTBOX));
            SetMenuItem(RID_SVXBMP_DATEFIELD, FM_COL_DATEFIELD"1", *pControlMenu, bDesignMode && (nColType != TYPE_DATEFIELD));
            SetMenuItem(RID_SVXBMP_TIMEFIELD, FM_COL_TIMEFIELD"1", *pControlMenu, bDesignMode && (nColType != TYPE_TIMEFIELD));
            SetMenuItem(RID_SVXBMP_NUMERICFIELD, FM_COL_NUMERICFIELD"1", *pControlMenu, bDesignMode && (nColType != TYPE_NUMERICFIELD));
            SetMenuItem(RID_SVXBMP_CURRENCYFIELD, FM_COL_CURRENCYFIELD"1", *pControlMenu, bDesignMode && (nColType != TYPE_CURRENCYFIELD));
            SetMenuItem(RID_SVXBMP_PATTERNFIELD, FM_COL_PATTERNFIELD"1", *pControlMenu, bDesignMode && (nColType != TYPE_PATTERNFIELD));
            SetMenuItem(RID_SVXBMP_FORMATTEDFIELD, FM_COL_FORMATTEDFIELD"1", *pControlMenu, bDesignMode && (nColType != TYPE_FORMATTEDFIELD));
        }
        rMenu.EnableItem(rMenu.GetItemId("change"), bDesignMode && bMarked && xCols.is());
    }
    else
        rMenu.EnableItem(rMenu.GetItemId("change"), false);

    rMenu.EnableItem(rMenu.GetItemId("insert"), bDesignMode && xCols.is());
    rMenu.EnableItem(rMenu.GetItemId("delete"), bDesignMode && bMarked && xCols.is());
    rMenu.EnableItem(rMenu.GetItemId("column"), bDesignMode && bMarked && xCols.is());

    PopupMenu* pShowColsMenu = rMenu.GetPopupMenu(rMenu.GetItemId("show"));
    sal_uInt16 nHiddenCols = 0;
    if (pShowColsMenu)
    {
        if (xCols.is())
        {
            // check for hidden cols
            Reference< css::beans::XPropertySet >  xCurCol;
            Any aHidden,aName;
            for (sal_Int32 i=0; i<xCols->getCount(); ++i)
            {
                xCurCol.set(xCols->getByIndex(i), css::uno::UNO_QUERY);
                DBG_ASSERT(xCurCol.is(), "FmGridHeader::PreExecuteColumnContextMenu : the Peer has invalid columns !");
                aHidden = xCurCol->getPropertyValue(FM_PROP_HIDDEN);
                DBG_ASSERT(aHidden.getValueType().getTypeClass() == TypeClass_BOOLEAN,
                    "FmGridHeader::PreExecuteColumnContextMenu : the property 'hidden' should be boolean !");
                if (::comphelper::getBOOL(aHidden))
                {
                    // put the column name into the 'show col' menu
                    if (nHiddenCols < 16)
                    {   // (only the first 16 items to keep the menu rather small)
                        aName = xCurCol->getPropertyValue(FM_PROP_LABEL);
                        pShowColsMenu->InsertItem(nHiddenCols + 1, ::comphelper::getString(aName),
                            MenuItemBits::NONE, OString(), nHiddenCols);
                            // the ID is arbitrary, but should be unique within the whole menu
                    }
                    ++nHiddenCols;
                }
            }
        }
        pShowColsMenu->EnableItem(pShowColsMenu->GetItemId("more"), xCols.is() && (nHiddenCols > 16));
        pShowColsMenu->EnableItem(pShowColsMenu->GetItemId("all"), xCols.is() && (nHiddenCols > 0));
    }

    // allow the 'hide column' item ?
    bool bAllowHide = bMarked;                                          // a column is marked
    bAllowHide = bAllowHide || (!bDesignMode && (nPos != sal_uInt16(-1)));  // OR we are in alive mode and have hit a column
    bAllowHide = bAllowHide && xCols.is();                              // AND we have a column container
    bAllowHide = bAllowHide && (xCols->getCount()-nHiddenCols > 1);     // AND there are at least two visible columns
    rMenu.EnableItem(rMenu.GetItemId("hide"), bAllowHide);

    if (bMarked)
    {

        SfxViewFrame* pCurrentFrame = SfxViewFrame::Current();
        SfxItemState eState = SfxItemState::UNKNOWN;
        // ask the bindings of the current view frame (which should be the one we're residing in) for the state
        if (pCurrentFrame)
        {
            std::unique_ptr<SfxPoolItem> pItem;
            eState = pCurrentFrame->GetBindings().QueryState(SID_FM_CTL_PROPERTIES, pItem);

            if (eState >= SfxItemState::DEFAULT && pItem != nullptr)
            {
                bool bChecked = dynamic_cast<const SfxBoolItem*>( pItem.get()) != nullptr && static_cast<SfxBoolItem*>(pItem.get())->GetValue();
                rMenu.CheckItem("column", bChecked);
            }
        }
    }
}

enum InspectorAction { eOpenInspector, eCloseInspector, eUpdateInspector, eNone };

void FmGridHeader::PostExecuteColumnContextMenu(sal_uInt16 nColId, const PopupMenu& rMenu, sal_uInt16 nExecutionResult)
{
    Reference< css::container::XIndexContainer >  xCols(static_cast<FmGridControl*>(GetParent())->GetPeer()->getColumns());
    sal_uInt16 nPos = GetModelColumnPos(nColId);

    OUString aFieldType;
    bool    bReplace = false;
    InspectorAction eInspectorAction = eNone;

    OString sExecutionResult = rMenu.GetCurItemIdent();
    if (sExecutionResult.isEmpty())
    {
        PopupMenu* pMenu = rMenu.GetPopupMenu(rMenu.GetItemId("insert"));
        if (pMenu)
            sExecutionResult = pMenu->GetCurItemIdent();
    }
    if (sExecutionResult.isEmpty())
    {
        PopupMenu* pMenu = rMenu.GetPopupMenu(rMenu.GetItemId("change"));
        if (pMenu)
            sExecutionResult = pMenu->GetCurItemIdent();
    }
    if (sExecutionResult.isEmpty())
    {
        PopupMenu* pMenu = rMenu.GetPopupMenu(rMenu.GetItemId("show"));
        if (pMenu)
            sExecutionResult = pMenu->GetCurItemIdent();
    }

    if (sExecutionResult == "delete")
    {
        Reference< XInterface > xCol(
            xCols->getByIndex(nPos), css::uno::UNO_QUERY);
        xCols->removeByIndex(nPos);
        ::comphelper::disposeComponent(xCol);
    }
    else if (sExecutionResult == "hide")
    {
        Reference< css::beans::XPropertySet > xCurCol( xCols->getByIndex(nPos), css::uno::UNO_QUERY);
        xCurCol->setPropertyValue(FM_PROP_HIDDEN, makeAny(true));
    }
    else if (sExecutionResult == "column")
    {
        eInspectorAction = rMenu.IsItemChecked(rMenu.GetItemId("column")) ? eOpenInspector : eCloseInspector;
    }
    else if (sExecutionResult.startsWith(FM_COL_TEXTFIELD))
    {
        if (sExecutionResult != FM_COL_TEXTFIELD)
            bReplace = true;
        aFieldType = FM_COL_TEXTFIELD;
    }
    else if (sExecutionResult.startsWith(FM_COL_COMBOBOX))
    {
        if (sExecutionResult != FM_COL_COMBOBOX)
            bReplace = true;
        aFieldType = FM_COL_COMBOBOX;
    }
    else if (sExecutionResult.startsWith(FM_COL_LISTBOX))
    {
        if (sExecutionResult != FM_COL_LISTBOX)
            bReplace = true;
        aFieldType = FM_COL_LISTBOX;
    }
    else if (sExecutionResult.startsWith(FM_COL_CHECKBOX))
    {
        if (sExecutionResult != FM_COL_CHECKBOX)
            bReplace = true;
        aFieldType = FM_COL_CHECKBOX;
    }
    else if (sExecutionResult.startsWith(FM_COL_DATEFIELD))
    {
        if (sExecutionResult != FM_COL_DATEFIELD)
            bReplace = true;
        aFieldType = FM_COL_DATEFIELD;
    }
    else if (sExecutionResult.startsWith(FM_COL_TIMEFIELD))
    {
        if (sExecutionResult != FM_COL_TIMEFIELD)
            bReplace = true;
        aFieldType = FM_COL_TIMEFIELD;
    }
    else if (sExecutionResult.startsWith(FM_COL_NUMERICFIELD))
    {
        if (sExecutionResult != FM_COL_NUMERICFIELD)
            bReplace = true;
        aFieldType = FM_COL_NUMERICFIELD;
    }
    else if (sExecutionResult.startsWith(FM_COL_CURRENCYFIELD))
    {
        if (sExecutionResult != FM_COL_CURRENCYFIELD)
            bReplace = true;
        aFieldType = FM_COL_CURRENCYFIELD;
    }
    else if (sExecutionResult.startsWith(FM_COL_PATTERNFIELD))
    {
        if (sExecutionResult != FM_COL_PATTERNFIELD)
            bReplace = true;
        aFieldType = FM_COL_PATTERNFIELD;
    }
    else if (sExecutionResult.startsWith(FM_COL_FORMATTEDFIELD))
    {
        if (sExecutionResult != FM_COL_FORMATTEDFIELD)
            bReplace = true;
        aFieldType = FM_COL_FORMATTEDFIELD;
    }
    else if (sExecutionResult == "more")
    {
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractFmShowColsDialog> pDlg(pFact->CreateFmShowColsDialog(GetFrameWeld()));
        pDlg->SetColumns(xCols);
        pDlg->Execute();
    }
    else if (sExecutionResult == "all")
    {
        // just iterate through all the cols ...
        Reference< css::beans::XPropertySet >  xCurCol;
        for (sal_Int32 i=0; i<xCols->getCount(); ++i)
        {
            xCurCol.set(xCols->getByIndex(i), css::uno::UNO_QUERY);
            xCurCol->setPropertyValue(FM_PROP_HIDDEN, makeAny(false));
        }
        // TODO : there must be a more clever way to do this ....
        // with the above the view is updated after every single model update ...
    }
    else if (nExecutionResult>0 && nExecutionResult<=16)
    {   // it was a "show column/<colname>" command (there are at most 16 such items)
        // search the nExecutionResult'th hidden col
        Reference< css::beans::XPropertySet >  xCurCol;
        for (sal_Int32 i=0; i<xCols->getCount() && nExecutionResult; ++i)
        {
            xCurCol.set(xCols->getByIndex(i), css::uno::UNO_QUERY);
            Any aHidden = xCurCol->getPropertyValue(FM_PROP_HIDDEN);
            if (::comphelper::getBOOL(aHidden))
                if (!--nExecutionResult)
                {
                    xCurCol->setPropertyValue(FM_PROP_HIDDEN, makeAny(false));
                    break;
                }
        }
    }

    if ( !aFieldType.isEmpty() )
    {
        try
        {
            Reference< XGridColumnFactory > xFactory( xCols, UNO_QUERY_THROW );
            Reference< XPropertySet > xNewCol( xFactory->createColumn( aFieldType ), UNO_SET_THROW );

            if ( bReplace )
            {
                // rescue over a few properties
                Reference< XPropertySet > xReplaced( xCols->getByIndex( nPos ), UNO_QUERY );

                TransferFormComponentProperties(
                    xReplaced, xNewCol, Application::GetSettings().GetUILanguageTag().getLocale() );

                xCols->replaceByIndex( nPos, makeAny( xNewCol ) );
                ::comphelper::disposeComponent( xReplaced );

                eInspectorAction = eUpdateInspector;
            }
            else
            {
                FormControlFactory factory;

                OUString sLabel = FormControlFactory::getDefaultUniqueName_ByComponentType(
                    Reference< XNameAccess >( xCols, UNO_QUERY_THROW ), xNewCol );
                xNewCol->setPropertyValue( FM_PROP_LABEL, makeAny( sLabel ) );
                xNewCol->setPropertyValue( FM_PROP_NAME, makeAny( sLabel ) );

                factory.initializeControlModel( DocumentClassification::classifyHostDocument( xCols ), xNewCol );

                xCols->insertByIndex( nPos, makeAny( xNewCol ) );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }
    }

    SfxViewFrame* pCurrentFrame = SfxViewFrame::Current();
    OSL_ENSURE( pCurrentFrame, "FmGridHeader::PostExecuteColumnContextMenu: no view frame -> no bindings -> no property browser!" );
    if ( pCurrentFrame )
    {
        if ( eInspectorAction == eUpdateInspector )
        {
            if ( !pCurrentFrame->HasChildWindow( SID_FM_SHOW_PROPERTIES ) )
                eInspectorAction = eNone;
        }

        if ( eInspectorAction != eNone )
        {
            SfxBoolItem aShowItem( SID_FM_SHOW_PROPERTIES, eInspectorAction != eCloseInspector );

            pCurrentFrame->GetBindings().GetDispatcher()->ExecuteList(
                    SID_FM_SHOW_PROPERTY_BROWSER, SfxCallMode::ASYNCHRON,
                    { &aShowItem });
        }
    }
}

void FmGridHeader::triggerColumnContextMenu( const ::Point& _rPreferredPos )
{
    // the affected col
    sal_uInt16 nColId = GetItemId( _rPreferredPos );

    // the menu
    VclBuilder aBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "svx/ui/colsmenu.ui", "");
    VclPtr<PopupMenu> aContextMenu(aBuilder.get_menu("menu"));

    // let derivatives modify the menu
    PreExecuteColumnContextMenu( nColId, *aContextMenu );
    aContextMenu->RemoveDisabledEntries( true, true );

    // execute the menu
    sal_uInt16 nResult = aContextMenu->Execute( this, _rPreferredPos );

    // let derivatives handle the result
    PostExecuteColumnContextMenu( nColId, *aContextMenu, nResult );
}

void FmGridHeader::Command(const CommandEvent& rEvt)
{
    switch (rEvt.GetCommand())
    {
        case CommandEventId::ContextMenu:
        {
            if (!rEvt.IsMouseEvent())
                return;

            triggerColumnContextMenu( rEvt.GetMousePosPixel() );
        }
        break;
        default:
            EditBrowserHeader::Command(rEvt);
    }
}

FmGridControl::FmGridControl(
                const Reference< css::uno::XComponentContext >& _rxContext,
                vcl::Window* pParent,
                FmXGridPeer* _pPeer,
                WinBits nBits)
        :DbGridControl(_rxContext, pParent, nBits)
        ,m_pPeer(_pPeer)
        ,m_nCurrentSelectedColumn(-1)
        ,m_nMarkedColumnId(BROWSER_INVALIDID)
        ,m_bSelecting(false)
        ,m_bInColumnMove(false)
{
    EnableInteractiveRowHeight( );
}

void FmGridControl::Command(const CommandEvent& _rEvt)
{
    if ( CommandEventId::ContextMenu == _rEvt.GetCommand() )
    {
        FmGridHeader* pMyHeader = static_cast< FmGridHeader* >( GetHeaderBar() );
        if ( pMyHeader && !_rEvt.IsMouseEvent() )
        {   // context menu requested by keyboard
            if  ( 1 == GetSelectColumnCount() || IsDesignMode() )
            {
                sal_uInt16 nSelId = GetColumnId(
                    sal::static_int_cast< sal_uInt16 >( FirstSelectedColumn() ) );
                ::tools::Rectangle aColRect( GetFieldRectPixel( 0, nSelId, false ) );

                Point aRelativePos( pMyHeader->ScreenToOutputPixel( OutputToScreenPixel( aColRect.TopCenter() ) ) );
                pMyHeader->triggerColumnContextMenu(aRelativePos);

                // handled
                return;
            }
        }
    }

    DbGridControl::Command( _rEvt );
}

// css::beans::XPropertyChangeListener
void FmGridControl::propertyChange(const css::beans::PropertyChangeEvent& evt)
{
    if (evt.PropertyName == FM_PROP_ROWCOUNT)
    {
        // if we're not in the main thread call AdjustRows asynchronously
        implAdjustInSolarThread(true);
        return;
    }

    const DbGridRowRef& xRow = GetCurrentRow();
    // no adjustment of the properties is carried out during positioning
    Reference<XPropertySet> xSet(evt.Source,UNO_QUERY);
    if (xRow.is() && (::cppu::any2bool(xSet->getPropertyValue(FM_PROP_ISNEW))|| CompareBookmark(getDataSource()->getBookmark(), xRow->GetBookmark())))
    {
        if (evt.PropertyName == FM_PROP_ISMODIFIED)
        {
            // modified or clean ?
            GridRowStatus eStatus = ::comphelper::getBOOL(evt.NewValue) ? GridRowStatus::Modified : GridRowStatus::Clean;
            if (eStatus != xRow->GetStatus())
            {
                xRow->SetStatus(eStatus);
                SolarMutexGuard aGuard;
                RowModified(GetCurrentPos());
            }
        }
    }
}

void FmGridControl::SetDesignMode(bool bMode)
{
    bool bOldMode = IsDesignMode();
    DbGridControl::SetDesignMode(bMode);
    if (bOldMode != bMode)
    {
        if (!bMode)
        {
            // cancel selection
            markColumn(USHRT_MAX);
        }
        else
        {
            Reference< css::container::XIndexContainer >  xColumns(GetPeer()->getColumns());
            Reference< css::view::XSelectionSupplier >  xSelSupplier(xColumns, UNO_QUERY);
            if (xSelSupplier.is())
            {
                Any aSelection = xSelSupplier->getSelection();
                Reference< css::beans::XPropertySet >  xColumn;
                if (aSelection.getValueType().getTypeClass() == TypeClass_INTERFACE)
                    xColumn.set(aSelection, css::uno::UNO_QUERY);
                Reference< XInterface >  xCurrent;
                for (sal_Int32 i=0; i<xColumns->getCount(); ++i)
                {
                    xCurrent.set(xColumns->getByIndex(i), css::uno::UNO_QUERY);
                    if (xCurrent == xColumn)
                    {
                        markColumn(GetColumnIdFromModelPos(i));
                        break;
                    }
                }
            }
        }
    }
}

void FmGridControl::DeleteSelectedRows()
{
    if (!m_pSeekCursor)
        return;

    // how many rows are selected?
    sal_Int32 nSelectedRows = GetSelectRowCount();

    // the current line should be deleted but it is currently in edit mode
    if ( IsCurrentAppending() )
        return;
    // is the insert row selected
    if (GetEmptyRow().is() && IsRowSelected(GetRowCount() - 1))
        nSelectedRows -= 1;

    // nothing to do
    if (nSelectedRows <= 0)
        return;

    // try to confirm the delete
    Reference< css::frame::XDispatchProvider >  xDispatcher = static_cast<css::frame::XDispatchProvider*>(GetPeer());
    if (xDispatcher.is())
    {
        css::util::URL aUrl;
        aUrl.Complete = FMURL_CONFIRM_DELETION;
        Reference< css::util::XURLTransformer > xTransformer(
            css::util::URLTransformer::create(::comphelper::getProcessComponentContext()) );
        xTransformer->parseStrict( aUrl );

        Reference< css::frame::XDispatch >  xDispatch = xDispatcher->queryDispatch(aUrl, OUString(), 0);
        Reference< css::form::XConfirmDeleteListener >  xConfirm(xDispatch, UNO_QUERY);
        if (xConfirm.is())
        {
            css::sdb::RowChangeEvent aEvent;
            aEvent.Source = Reference< XInterface >(*getDataSource());
            aEvent.Rows = nSelectedRows;
            aEvent.Action = css::sdb::RowChangeAction::DELETE;
            if (!xConfirm->confirmDelete(aEvent))
                return;
        }
    }

    const MultiSelection* pRowSelection = GetSelection();
    if ( pRowSelection && pRowSelection->IsAllSelected() )
    {
        BeginCursorAction();
        CursorWrapper* pCursor = getDataSource();
        Reference< XResultSetUpdate >  xUpdateCursor(Reference< XInterface >(*pCursor), UNO_QUERY);
        try
        {
            pCursor->beforeFirst();
            while( pCursor->next() )
                xUpdateCursor->deleteRow();

            SetUpdateMode(false);
            SetNoSelection();

            xUpdateCursor->moveToInsertRow();
        }
        catch(const Exception&)
        {
            OSL_FAIL("Exception caught while deleting rows!");
        }
        // adapt to the data cursor
        AdjustDataSource(true);
        EndCursorAction();
        SetUpdateMode(true);
    }
    else
    {
        Reference< css::sdbcx::XDeleteRows >  xDeleteThem(Reference< XInterface >(*getDataSource()), UNO_QUERY);

        // collect the bookmarks of the selected rows
        Sequence < Any> aBookmarks = getSelectionBookmarks();

        // determine the next row to position after deletion
        Any aBookmark;
        bool bNewPos = false;
        // if the current row isn't selected we take the row as row after deletion
        OSL_ENSURE( GetCurrentRow().is(), "FmGridControl::DeleteSelectedRows: no current row here?" );
            // crash reports suggest it can happen we don't have a current row - how?
            // #154303# / 2008-04-23 / frank.schoenheit@sun.com
        if ( !IsRowSelected( GetCurrentPos() ) && !IsCurrentAppending() && GetCurrentRow().is() )
        {
            aBookmark = GetCurrentRow()->GetBookmark();
            bNewPos   = true;
        }
        else
        {
            // we look for the first row after the selected block for selection
            long nIdx = LastSelectedRow() + 1;
            if (nIdx < GetRowCount() - 1)
            {
                // there is a next row to position on
                if (SeekCursor(nIdx))
                {
                    GetSeekRow()->SetState(m_pSeekCursor.get(), true);

                    bNewPos = true;
                    // if it's not the row for inserting we keep the bookmark
                    if (!IsInsertionRow(nIdx))
                        aBookmark = m_pSeekCursor->getBookmark();
                }
            }
            else
            {
                // we look for the first row before the selected block for selection after deletion
                nIdx = FirstSelectedRow() - 1;
                if (nIdx >= 0 && SeekCursor(nIdx))
                {
                    GetSeekRow()->SetState(m_pSeekCursor.get(), true);

                    bNewPos = true;
                    aBookmark = m_pSeekCursor->getBookmark();
                }
            }
        }

        // Are all rows selected?
        // Second condition if no insertion line exists
        bool bAllSelected = GetTotalCount() == nSelectedRows || GetRowCount() == nSelectedRows;

        BeginCursorAction();

        // now delete the row
        Sequence<sal_Int32> aDeletedRows;
        SetUpdateMode( false );
        try
        {
            aDeletedRows = xDeleteThem->deleteRows(aBookmarks);
        }
        catch(SQLException&)
        {
        }
        SetUpdateMode( true );

        // how many rows are deleted?
        sal_Int32 nDeletedRows = 0;
        const sal_Int32* pSuccess = aDeletedRows.getConstArray();
        for (sal_Int32 i = 0; i < aDeletedRows.getLength(); i++)
        {
            if (pSuccess[i])
                ++nDeletedRows;
        }

        // have rows been deleted?
        if (nDeletedRows)
        {
            SetUpdateMode(false);
            SetNoSelection();
            try
            {
                // did we delete all the rows than try to move to the next possible row
                if (nDeletedRows == aDeletedRows.getLength())
                {
                    // there exists a new position to move on
                    if (bNewPos)
                    {
                        if (aBookmark.hasValue())
                            getDataSource()->moveToBookmark(aBookmark);
                        // no valid bookmark so move to the insert row
                        else
                        {
                            Reference< XResultSetUpdate >  xUpdateCursor(Reference< XInterface >(*m_pDataCursor), UNO_QUERY);
                            xUpdateCursor->moveToInsertRow();
                        }
                    }
                    else
                    {
                        Reference< css::beans::XPropertySet >  xSet(Reference< XInterface >(*m_pDataCursor), UNO_QUERY);

                        sal_Int32 nRecordCount(0);
                        xSet->getPropertyValue(FM_PROP_ROWCOUNT) >>= nRecordCount;
                        if ( m_pDataCursor->rowDeleted() )
                            --nRecordCount;

                        // there are no rows left and we have an insert row
                        if (!nRecordCount && GetEmptyRow().is())
                        {
                            Reference< XResultSetUpdate >  xUpdateCursor(Reference< XInterface >(*m_pDataCursor), UNO_QUERY);
                            xUpdateCursor->moveToInsertRow();
                        }
                        else if (nRecordCount)
                            // move to the first row
                            getDataSource()->first();
                    }
                }
                // not all the rows where deleted, so move to the first row which remained in the resultset
                else
                {
                    for (sal_Int32 i = 0; i < aDeletedRows.getLength(); i++)
                    {
                        if (!pSuccess[i])
                        {
                            getDataSource()->moveToBookmark(aBookmarks.getConstArray()[i]);
                            break;
                        }
                    }
                }
            }
            catch(const Exception&)
            {
                try
                {
                    // positioning went wrong so try to move to the first row
                    getDataSource()->first();
                }
                catch(const Exception&)
                {
                }
            }

            // adapt to the data cursor
            AdjustDataSource(true);

            // not all rows could be deleted;
            // never select again there the ones that could not be deleted
            if (nDeletedRows < nSelectedRows)
            {
                // were all selected
                if (bAllSelected)
                {
                    SelectAll();
                    if (IsInsertionRow(GetRowCount() - 1))  // not the insertion row
                        SelectRow(GetRowCount() - 1, false);
                }
                else
                {
                    // select the remaining rows
                    for (sal_Int32 i = 0; i < aDeletedRows.getLength(); i++)
                    {
                        try
                        {
                            if (!pSuccess[i])
                            {
                                m_pSeekCursor->moveToBookmark(m_pDataCursor->getBookmark());
                                SetSeekPos(m_pSeekCursor->getRow() - 1);
                                SelectRow(GetSeekPos());
                            }
                        }
                        catch(const Exception&)
                        {
                            // keep the seekpos in all cases
                            SetSeekPos(m_pSeekCursor->getRow() - 1);
                        }
                    }
                }
            }

            EndCursorAction();
            SetUpdateMode(true);
        }
        else // row could not be deleted
        {
            EndCursorAction();
            try
            {
                // currentrow is the insert row?
                if (!IsCurrentAppending())
                    getDataSource()->refreshRow();
            }
            catch(const Exception&)
            {
            }
        }
    }

    // if there is no selection anymore we can start editing
    if (!GetSelectRowCount())
        ActivateCell();
}

// XCurrentRecordListener
void FmGridControl::positioned()
{
    SAL_INFO("svx.fmcomp", "FmGridControl::positioned");
    // position on the data source (force it to be done in the main thread)
    implAdjustInSolarThread(false);
}

bool FmGridControl::commit()
{
    // execute commit only if an update is not already executed by the
    // css::form::component::GridControl
    if (!IsUpdating())
    {
        if (Controller().is() && Controller()->IsModified())
        {
            if (!SaveModified())
                return false;
        }
    }
    return true;
}

void FmGridControl::inserted()
{
    const DbGridRowRef& xRow = GetCurrentRow();
    if (!xRow.is())
        return;

    // line has been inserted, then reset the status and mode
    xRow->SetState(m_pDataCursor.get(), false);
    xRow->SetNew(false);

}

VclPtr<BrowserHeader> FmGridControl::imp_CreateHeaderBar(BrowseBox* pParent)
{
    DBG_ASSERT( pParent == this, "FmGridControl::imp_CreateHeaderBar: parent?" );
    return VclPtr<FmGridHeader>::Create( pParent );
}

void FmGridControl::markColumn(sal_uInt16 nId)
{
    if (GetHeaderBar() && m_nMarkedColumnId != nId)
    {
        // deselect
        if (m_nMarkedColumnId != BROWSER_INVALIDID)
        {
            HeaderBarItemBits aBits = GetHeaderBar()->GetItemBits(m_nMarkedColumnId) & ~HeaderBarItemBits::FLAT;
            GetHeaderBar()->SetItemBits(m_nMarkedColumnId, aBits);
        }


        if (nId != BROWSER_INVALIDID)
        {
            HeaderBarItemBits aBits = GetHeaderBar()->GetItemBits(nId) | HeaderBarItemBits::FLAT;
            GetHeaderBar()->SetItemBits(nId, aBits);
        }
        m_nMarkedColumnId = nId;
    }
}

bool FmGridControl::isColumnMarked(sal_uInt16 nId) const
{
    return m_nMarkedColumnId == nId;
}

long FmGridControl::QueryMinimumRowHeight()
{
    long const nMinimalLogicHeight = 20; // 0.2 cm
    long nMinimalPixelHeight = LogicToPixel(Point(0, nMinimalLogicHeight), MapMode(MapUnit::Map10thMM)).Y();
    return CalcZoom( nMinimalPixelHeight );
}

void FmGridControl::RowHeightChanged()
{
    DbGridControl::RowHeightChanged();

    Reference< XPropertySet > xModel( GetPeer()->getColumns(), UNO_QUERY );
    DBG_ASSERT( xModel.is(), "FmGridControl::RowHeightChanged: no model!" );
    if ( xModel.is() )
    {
        try
        {
            sal_Int32 nUnzoomedPixelHeight = CalcReverseZoom( GetDataRowHeight() );
            Any aProperty = makeAny( static_cast<sal_Int32>(PixelToLogic( Point(0, nUnzoomedPixelHeight), MapMode(MapUnit::Map10thMM)).Y()) );
            xModel->setPropertyValue( FM_PROP_ROWHEIGHT, aProperty );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FmGridControl::RowHeightChanged: caught an exception!" );
        }
    }
}

void FmGridControl::ColumnResized(sal_uInt16 nId)
{
    DbGridControl::ColumnResized(nId);

    // transfer value to the model
    DbGridColumn* pCol = DbGridControl::GetColumns()[ GetModelColumnPos(nId) ].get();
    const Reference< css::beans::XPropertySet >&  xColModel(pCol->getModel());
    if (xColModel.is())
    {
        Any aWidth;
        sal_Int32 nColumnWidth = GetColumnWidth(nId);
        nColumnWidth = CalcReverseZoom(nColumnWidth);
        // convert to 10THMM
        aWidth <<= static_cast<sal_Int32>(PixelToLogic(Point(nColumnWidth, 0), MapMode(MapUnit::Map10thMM)).X());
        xColModel->setPropertyValue(FM_PROP_WIDTH, aWidth);
    }
}

void FmGridControl::CellModified()
{
    DbGridControl::CellModified();
    GetPeer()->CellModified();
}

void FmGridControl::BeginCursorAction()
{
    DbGridControl::BeginCursorAction();
    m_pPeer->stopCursorListening();
}

void FmGridControl::EndCursorAction()
{
    m_pPeer->startCursorListening();
    DbGridControl::EndCursorAction();
}

void FmGridControl::ColumnMoved(sal_uInt16 nId)
{
    m_bInColumnMove = true;

    DbGridControl::ColumnMoved(nId);
    Reference< css::container::XIndexContainer >  xColumns(GetPeer()->getColumns());

    if (xColumns.is())
    {
        // locate the column and move in the model;
        // get ColumnPos
        DbGridColumn* pCol = DbGridControl::GetColumns()[ GetModelColumnPos(nId) ].get();
        Reference< css::beans::XPropertySet >  xCol;

        // inserting must be based on the column positions
        sal_Int32 i;
        Reference< XInterface > xCurrent;
        for (i = 0; !xCol.is() && i < xColumns->getCount(); i++)
        {
            xCurrent.set(xColumns->getByIndex(i), css::uno::UNO_QUERY);
            if (xCurrent == pCol->getModel())
            {
                xCol = pCol->getModel();
                break;
            }
        }

        DBG_ASSERT(i < xColumns->getCount(), "Wrong css::sdbcx::Index");
        xColumns->removeByIndex(i);
        Any aElement;
        aElement <<= xCol;
        xColumns->insertByIndex(GetModelColumnPos(nId), aElement);
        pCol->setModel(xCol);
        // if the column which is shown here is selected ...
        if ( isColumnSelected(pCol) )
            markColumn(nId); // ... -> mark it
    }

    m_bInColumnMove = false;
}

void FmGridControl::InitColumnsByModels(const Reference< css::container::XIndexContainer >& xColumns)
{
    // reset columns;
    // if there is only one HandleColumn, then don't
    if (GetModelColCount())
    {
        RemoveColumns();
        InsertHandleColumn();
    }

    if (!xColumns.is())
        return;

    SetUpdateMode(false);

    // inserting must be based on the column positions
    sal_Int32 i;
    Any aWidth;
    for (i = 0; i < xColumns->getCount(); ++i)
    {
        Reference< css::beans::XPropertySet > xCol(
            xColumns->getByIndex(i), css::uno::UNO_QUERY);

        OUString aName(
            comphelper::getString(xCol->getPropertyValue(FM_PROP_LABEL)));

        aWidth = xCol->getPropertyValue(FM_PROP_WIDTH);
        sal_Int32 nWidth = 0;
        if (aWidth >>= nWidth)
            nWidth = LogicToPixel(Point(nWidth, 0), MapMode(MapUnit::Map10thMM)).X();

        AppendColumn(aName, static_cast<sal_uInt16>(nWidth));
        DbGridColumn* pCol = DbGridControl::GetColumns()[ i ].get();
        pCol->setModel(xCol);
    }

    // and now remove the hidden columns as well
    // (we did not already make it in the upper loop, since we would then have gotten
    // problems with the IDs of the columns: AppendColumn allocates them automatically,
    // but the column _after_ a hidden one needs an ID increased by one ...)
    Any aHidden;
    for (i = 0; i < xColumns->getCount(); ++i)
    {
        Reference< css::beans::XPropertySet > xCol( xColumns->getByIndex(i), css::uno::UNO_QUERY);
        aHidden = xCol->getPropertyValue(FM_PROP_HIDDEN);
        if (::comphelper::getBOOL(aHidden))
            HideColumn(GetColumnIdFromModelPos(static_cast<sal_uInt16>(i)));
    }

    SetUpdateMode(true);
}

void FmGridControl::InitColumnByField(
    DbGridColumn* _pColumn, const Reference< XPropertySet >& _rxColumnModel,
    const Reference< XNameAccess >& _rxFieldsByNames, const Reference< XIndexAccess >& _rxFieldsByIndex )
{
    DBG_ASSERT( _rxFieldsByNames == _rxFieldsByIndex, "FmGridControl::InitColumnByField: invalid container interfaces!" );

    // lookup the column which belongs to the control source
    OUString sFieldName;
    _rxColumnModel->getPropertyValue( FM_PROP_CONTROLSOURCE ) >>= sFieldName;
    Reference< XPropertySet > xField;
    _rxColumnModel->getPropertyValue( FM_PROP_BOUNDFIELD ) >>= xField;


    if ( !xField.is() && /*sFieldName.getLength() && */_rxFieldsByNames->hasByName( sFieldName ) ) // #i93452# do not check for name length
        _rxFieldsByNames->getByName( sFieldName ) >>= xField;

    // determine the position of this column
    sal_Int32 nFieldPos = -1;
    if ( xField.is() )
    {
        Reference< XPropertySet > xCheck;
        sal_Int32 nFieldCount = _rxFieldsByIndex->getCount();
        for ( sal_Int32 i = 0; i < nFieldCount; ++i)
        {
            _rxFieldsByIndex->getByIndex( i ) >>= xCheck;
            if ( xField.get() == xCheck.get() )
            {
                nFieldPos = i;
                break;
            }
        }
    }

    if ( xField.is() && ( nFieldPos >= 0 ) )
    {
        // some data types are not allowed
        sal_Int32 nDataType = DataType::OTHER;
        xField->getPropertyValue( FM_PROP_FIELDTYPE ) >>= nDataType;

        bool bIllegalType = false;
        switch ( nDataType )
        {
            case DataType::BLOB:
            case DataType::LONGVARBINARY:
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::OTHER:
                bIllegalType = true;
                break;
        }

        if ( bIllegalType )
        {
            _pColumn->SetObject( static_cast<sal_Int16>(nFieldPos) );
            return;
        }
    }

    // the control type is determined by the ColumnServiceName
    static const char s_sPropColumnServiceName[] = "ColumnServiceName";
    if ( !::comphelper::hasProperty( s_sPropColumnServiceName, _rxColumnModel ) )
        return;

    _pColumn->setModel( _rxColumnModel );

    OUString sColumnServiceName;
    _rxColumnModel->getPropertyValue( s_sPropColumnServiceName ) >>= sColumnServiceName;

    sal_Int32 nTypeId = getColumnTypeByModelName( sColumnServiceName );
    _pColumn->CreateControl( nFieldPos, xField, nTypeId );
}

void FmGridControl::InitColumnsByFields(const Reference< css::container::XIndexAccess >& _rxFields)
{
    if ( !_rxFields.is() )
        return;

    // initialize columns
    Reference< XIndexContainer > xColumns( GetPeer()->getColumns() );
    Reference< XNameAccess > xFieldsAsNames( _rxFields, UNO_QUERY );

    // inserting must be based on the column positions
    for (sal_Int32 i = 0; i < xColumns->getCount(); i++)
    {
        DbGridColumn* pCol = GetColumns()[ i ].get();
        OSL_ENSURE(pCol,"No grid column!");
        if ( pCol )
        {
            Reference< XPropertySet > xColumnModel(
                xColumns->getByIndex( i ), css::uno::UNO_QUERY);

            InitColumnByField( pCol, xColumnModel, xFieldsAsNames, _rxFields );
        }
    }
}

void FmGridControl::HideColumn(sal_uInt16 nId)
{
    DbGridControl::HideColumn(nId);

    sal_uInt16 nPos = GetModelColumnPos(nId);
    if (nPos == sal_uInt16(-1))
        return;

    DbGridColumn* pColumn = GetColumns()[ nPos ].get();
    if (pColumn->IsHidden())
        GetPeer()->columnHidden(pColumn);

    if (nId == m_nMarkedColumnId)
        m_nMarkedColumnId = sal_uInt16(-1);
}

bool FmGridControl::isColumnSelected(DbGridColumn const * _pColumn)
{
    OSL_ENSURE(_pColumn,"Column can not be null!");
    bool bSelected = false;
    // if the column which is shown here is selected ...
    Reference< css::view::XSelectionSupplier >  xSelSupplier(GetPeer()->getColumns(), UNO_QUERY);
    if ( xSelSupplier.is() )
    {
        Reference< css::beans::XPropertySet >  xColumn;
        xSelSupplier->getSelection() >>= xColumn;
        bSelected = (xColumn.get() == _pColumn->getModel().get());
    }
    return bSelected;
}

void FmGridControl::ShowColumn(sal_uInt16 nId)
{
    DbGridControl::ShowColumn(nId);

    sal_uInt16 nPos = GetModelColumnPos(nId);
    if (nPos == sal_uInt16(-1))
        return;

    DbGridColumn* pColumn = GetColumns()[ nPos ].get();
    if (!pColumn->IsHidden())
        GetPeer()->columnVisible(pColumn);

    // if the column which is shown here is selected ...
    if ( isColumnSelected(pColumn) )
        markColumn(nId); // ... -> mark it
}

bool FmGridControl::selectBookmarks(const Sequence< Any >& _rBookmarks)
{
    SolarMutexGuard aGuard;
        // need to lock the SolarMutex so that no paint call disturbs us ...

    if ( !m_pSeekCursor )
    {
        OSL_FAIL( "FmGridControl::selectBookmarks: no seek cursor!" );
        return false;
    }

    const Any* pBookmark = _rBookmarks.getConstArray();
    const Any* pBookmarkEnd = pBookmark + _rBookmarks.getLength();

    SetNoSelection();

    bool bAllSuccessfull = true;
    try
    {
        for (; pBookmark != pBookmarkEnd; ++pBookmark)
        {
            // move the seek cursor to the row given
            if (m_pSeekCursor->moveToBookmark(*pBookmark))
                SelectRow( m_pSeekCursor->getRow() - 1);
            else
                bAllSuccessfull = false;
        }
    }
    catch(Exception&)
    {
        OSL_FAIL("FmGridControl::selectBookmarks: could not move to one of the bookmarks!");
        return false;
    }

    return bAllSuccessfull;
}

Sequence< Any> FmGridControl::getSelectionBookmarks()
{
    // lock our update so no paint-triggered seeks interfere ...
    SetUpdateMode(false);

    sal_Int32 nSelectedRows = GetSelectRowCount(), i = 0;
    Sequence< Any> aBookmarks(nSelectedRows);
    if ( nSelectedRows )
    {
        Any* pBookmarks = aBookmarks.getArray();

        // (I'm not sure if the problem isn't deeper: The scenario: a large table displayed by a grid with a
        // thread-safe cursor (dBase). On loading the sdb-cursor started a counting thread. While this counting progress
        // was running, I tried do delete 3 records from within the grid. Deletion caused a SeekCursor, which did a
        // m_pSeekCursor->moveRelative and a m_pSeekCursor->getPosition.
        // Unfortunately the first call caused a propertyChanged(RECORDCOUNT) which resulted in a repaint of the
        // navigation bar and the grid. The latter itself will result in SeekRow calls. So after (successfully) returning
        // from the moveRelative the getPosition returns an invalid value. And so the SeekCursor fails.
        // In the consequence ALL parts of code where two calls to the seek cursor are done, while the second call _relies_ on
        // the first one, should be secured against recursion, with a broad-minded interpretation of "recursion": if any of these
        // code parts is executed, no other should be accessible. But this sounds very difficult to achieve...
        // )

        // The next problem caused by the same behavior (SeekCursor causes a propertyChanged): when adjusting rows we implicitly
        // change our selection. So a "FirstSelected(); SeekCursor(); NextSelected();" may produce unpredictable results.
        // That's why we _first_ collect the indices of the selected rows and _then_ their bookmarks.
        long nIdx = FirstSelectedRow();
        while (nIdx != BROWSER_ENDOFSELECTION)
        {
            // (we misuse the bookmarks array for this ...)
            pBookmarks[i++] <<= static_cast<sal_Int32>(nIdx);
            nIdx = NextSelectedRow();
        }
        DBG_ASSERT(i == nSelectedRows, "FmGridControl::DeleteSelectedRows : could not collect the row indices !");

        for (i=0; i<nSelectedRows; ++i)
        {
            nIdx = ::comphelper::getINT32(pBookmarks[i]);
            if (IsInsertionRow(nIdx))
            {
                // do not delete empty row
                aBookmarks.realloc(--nSelectedRows);
                SelectRow(nIdx, false);          // cancel selection for empty row
                break;
            }

            // first, position the data cursor on the selected block
            if (SeekCursor(nIdx))
            {
                GetSeekRow()->SetState(m_pSeekCursor.get(), true);

                pBookmarks[i] = m_pSeekCursor->getBookmark();
            }
    #ifdef DBG_UTIL
            else
                OSL_FAIL("FmGridControl::DeleteSelectedRows : a bookmark could not be determined !");
    #endif
        }
    }
    SetUpdateMode(true);

    // if one of the SeekCursor-calls failed ....
    aBookmarks.realloc(i);

    // (the alternative : while collecting the bookmarks lock our propertyChanged, this should resolve both our problems.
    // but this would be incompatible as we need a locking flag, then ...)

    return aBookmarks;
}

namespace
{
    OUString getColumnPropertyFromPeer(FmXGridPeer* _pPeer,sal_Int32 _nPosition,const OUString& _sPropName)
    {
        OUString sRetText;
        if ( _pPeer && _nPosition != -1)
        {
            Reference<XIndexContainer> xIndex = _pPeer->getColumns();
            if ( xIndex.is() && xIndex->getCount() > _nPosition )
            {
                Reference<XPropertySet> xProp;
                xIndex->getByIndex( _nPosition ) >>= xProp;
                if ( xProp.is() )
                {
                    try {
                        xProp->getPropertyValue( _sPropName ) >>= sRetText;
                    } catch (UnknownPropertyException const& e) {
                        SAL_WARN("svx.fmcomp", e);
                    }
                }
            }
        }
        return sRetText;
    }
}

// Object data and state
OUString FmGridControl::GetAccessibleObjectName( ::vcl::AccessibleBrowseBoxObjType _eObjType,sal_Int32 _nPosition ) const
{
    OUString sRetText;
    switch( _eObjType )
    {
        case ::vcl::BBTYPE_BROWSEBOX:
            if ( GetPeer() )
            {
                Reference<XPropertySet> xProp(GetPeer()->getColumns(),UNO_QUERY);
                if ( xProp.is() )
                    xProp->getPropertyValue(FM_PROP_NAME) >>= sRetText;
            }
            break;
        case ::vcl::BBTYPE_COLUMNHEADERCELL:
            sRetText = getColumnPropertyFromPeer(
                GetPeer(),
                GetModelColumnPos(
                    sal::static_int_cast< sal_uInt16 >(_nPosition)),
                FM_PROP_LABEL);
            break;
        default:
            sRetText = DbGridControl::GetAccessibleObjectName(_eObjType,_nPosition);
    }
    return sRetText;
}

OUString FmGridControl::GetAccessibleObjectDescription( ::vcl::AccessibleBrowseBoxObjType _eObjType,sal_Int32 _nPosition ) const
{
    OUString sRetText;
    switch( _eObjType )
    {
        case ::vcl::BBTYPE_BROWSEBOX:
            if ( GetPeer() )
            {
                Reference<XPropertySet> xProp(GetPeer()->getColumns(),UNO_QUERY);
                if ( xProp.is() )
                {
                    xProp->getPropertyValue(FM_PROP_HELPTEXT) >>= sRetText;
                    if ( sRetText.isEmpty() )
                        xProp->getPropertyValue(FM_PROP_DESCRIPTION) >>= sRetText;
                }
            }
            break;
        case ::vcl::BBTYPE_COLUMNHEADERCELL:
            sRetText = getColumnPropertyFromPeer(
                GetPeer(),
                GetModelColumnPos(
                    sal::static_int_cast< sal_uInt16 >(_nPosition)),
                FM_PROP_HELPTEXT);
            if ( sRetText.isEmpty() )
                sRetText = getColumnPropertyFromPeer(
                            GetPeer(),
                            GetModelColumnPos(
                                sal::static_int_cast< sal_uInt16 >(_nPosition)),
                            FM_PROP_DESCRIPTION);

            break;
        default:
            sRetText = DbGridControl::GetAccessibleObjectDescription(_eObjType,_nPosition);
    }
    return sRetText;
}

void FmGridControl::Select()
{
    DbGridControl::Select();
    // ... does it affect our columns?
    const MultiSelection* pColumnSelection = GetColumnSelection();

    sal_uInt16 nSelectedColumn =
        pColumnSelection && pColumnSelection->GetSelectCount()
            ? sal::static_int_cast< sal_uInt16 >(
                const_cast<MultiSelection*>(pColumnSelection)->FirstSelected())
            : SAL_MAX_UINT16;
    // the HandleColumn is not selected
    switch (nSelectedColumn)
    {
        case SAL_MAX_UINT16: break; // no selection
        case  0 : nSelectedColumn = SAL_MAX_UINT16; break;
                    // handle col can't be selected
        default :
            // get the model col pos instead of the view col pos
            nSelectedColumn = GetModelColumnPos(GetColumnIdFromViewPos(nSelectedColumn - 1));
            break;
    }

    if (nSelectedColumn != m_nCurrentSelectedColumn)
    {
        // BEFORE calling the select at the SelectionSupplier!
        m_nCurrentSelectedColumn = nSelectedColumn;

        if (!m_bSelecting)
        {
            m_bSelecting = true;

            try
            {
                Reference< XIndexAccess >  xColumns(GetPeer()->getColumns(), UNO_QUERY);
                Reference< XSelectionSupplier >  xSelSupplier(xColumns, UNO_QUERY);
                if (xSelSupplier.is())
                {
                    if (nSelectedColumn != SAL_MAX_UINT16)
                    {
                        Reference< XPropertySet >  xColumn(
                            xColumns->getByIndex(nSelectedColumn),
                            css::uno::UNO_QUERY);
                        xSelSupplier->select(makeAny(xColumn));
                    }
                    else
                    {
                        xSelSupplier->select(Any());
                    }
                }
            }
            catch(Exception&)
            {
            }


            m_bSelecting = false;
        }
    }
}


void FmGridControl::KeyInput( const KeyEvent& rKEvt )
{
    bool bDone = false;
    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();
    if (    IsDesignMode()
        &&  !rKeyCode.IsShift()
        &&  !rKeyCode.IsMod1()
        &&  !rKeyCode.IsMod2()
        &&  GetParent() )
    {
        switch ( rKeyCode.GetCode() )
        {
            case KEY_ESCAPE:
                GetParent()->GrabFocus();
                bDone = true;
                break;
            case KEY_DELETE:
                if ( GetSelectColumnCount() && GetPeer() && m_nCurrentSelectedColumn >= 0 )
                {
                    Reference< css::container::XIndexContainer >  xCols(GetPeer()->getColumns());
                    if ( xCols.is() )
                    {
                        try
                        {
                            if ( m_nCurrentSelectedColumn < xCols->getCount() )
                            {
                                Reference< XInterface >  xCol;
                                xCols->getByIndex(m_nCurrentSelectedColumn) >>= xCol;
                                xCols->removeByIndex(m_nCurrentSelectedColumn);
                                ::comphelper::disposeComponent(xCol);
                            }
                        }
                        catch(const Exception&)
                        {
                            OSL_FAIL("exception occurred while deleting a column");
                        }
                    }
                }
                bDone = true;
                break;
        }
    }
    if ( !bDone )
        DbGridControl::KeyInput( rKEvt );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
