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

#include "svx/fmgridif.hxx"
#include "fmitems.hxx"
#include "fmprop.hrc"
#include "svx/fmtools.hxx"
#include "svx/fmresids.hrc"
#include "fmservs.hxx"
#include "fmurl.hxx"
#include "formcontrolfactory.hxx"
#include "gridcell.hxx"
#include "gridcols.hxx"
#include "svx/dbaexchange.hxx"
#include "svx/dialmgr.hxx"
#include "svx/dialogs.hrc"
#include "svx/fmgridcl.hxx"
#include "svx/svxdlg.hxx"
#include "svx/svxids.hrc"

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
#include <comphelper/extract.hxx>
#include <comphelper/numbers.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/property.hxx>
#include <comphelper/string.hxx>
#include <connectivity/dbtools.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/eitem.hxx>
#include <svtools/fmtfield.hxx>
#include <svl/numuno.hxx>
#include <tools/multisel.hxx>
#include <tools/shl.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/help.hxx>
#include <vcl/image.hxx>
#include <vcl/longcurr.hxx>
#include <vcl/menu.hxx>

#include <math.h>

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

//==============================================================================
//------------------------------------------------------------------------------
OUString FieldServiceFromId(sal_Int32 nID)
{
    switch (nID)
    {
        case SID_FM_EDIT            : return FM_COL_TEXTFIELD;
        case SID_FM_COMBOBOX        : return FM_COL_COMBOBOX;
        case SID_FM_LISTBOX         : return FM_COL_LISTBOX;
        case SID_FM_CHECKBOX        : return FM_COL_CHECKBOX;
        case SID_FM_DATEFIELD       : return FM_COL_DATEFIELD;
        case SID_FM_TIMEFIELD       : return FM_COL_TIMEFIELD;
        case SID_FM_NUMERICFIELD    : return FM_COL_NUMERICFIELD;
        case SID_FM_CURRENCYFIELD   : return FM_COL_CURRENCYFIELD;
        case SID_FM_PATTERNFIELD    : return FM_COL_PATTERNFIELD;
        case SID_FM_FORMATTEDFIELD  : return FM_COL_FORMATTEDFIELD;
    }
    return OUString();
}

//==============================================================================
struct FmGridHeaderData
{
    ODataAccessDescriptor   aDropData;
    Point                   aDropPosPixel;
    sal_Int8                nDropAction;
    Reference< XInterface > xDroppedStatement;
    Reference< XInterface > xDroppedResultSet;
};

//==============================================================================
//------------------------------------------------------------------------------
const sal_Int16 nChangeTypeOffset = 1000;
void SetMenuItem(const ImageList& rList, sal_uInt16 nID, Menu* pMenu, Menu& rNewMenu, sal_Bool bDesignMode = sal_True, sal_Int16 nOffset = nChangeTypeOffset)
{
    pMenu->SetItemImage(nID, rList.GetImage(nID));
    pMenu->EnableItem(nID, bDesignMode);
    rNewMenu.InsertItem(nID + nOffset, pMenu->GetItemText(nID));
    rNewMenu.SetItemImage(nID + nOffset, rList.GetImage(nID));
    rNewMenu.SetHelpId(nID + nOffset, pMenu->GetHelpId(nID));
    rNewMenu.EnableItem(nID + nOffset, bDesignMode);
}

//------------------------------------------------------------------------------
FmGridHeader::FmGridHeader( BrowseBox* pParent, WinBits nWinBits)
        :EditBrowserHeader(pParent, nWinBits)
        ,DropTargetHelper(this)
        ,m_pImpl(new FmGridHeaderData)
{
}

//------------------------------------------------------------------------------
FmGridHeader::~FmGridHeader()
{
    delete m_pImpl;
}

//------------------------------------------------------------------------------
sal_uInt16 FmGridHeader::GetModelColumnPos(sal_uInt16 nId) const
{
    return static_cast<FmGridControl*>(GetParent())->GetModelColumnPos(nId);
}
//---------------------------------------------------------------------------------------
void FmGridHeader::notifyColumnSelect(sal_uInt16 nColumnId)
{
    sal_uInt16 nPos = GetModelColumnPos(nColumnId);
    Reference< XIndexAccess >  xColumns(((FmGridControl*)GetParent())->GetPeer()->getColumns(), UNO_QUERY);
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
//------------------------------------------------------------------------------
void FmGridHeader::Select()
{
    EditBrowserHeader::Select();
    notifyColumnSelect(GetCurItemId());
}

//------------------------------------------------------------------------------
void FmGridHeader::RequestHelp( const HelpEvent& rHEvt )
{
    sal_uInt16 nItemId = GetItemId( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );
    if ( nItemId )
    {
        if ( rHEvt.GetMode() & (HELPMODE_QUICK | HELPMODE_BALLOON) )
        {
            Rectangle aItemRect = GetItemRect( nItemId );
            Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
            aItemRect.Left()   = aPt.X();
            aItemRect.Top()    = aPt.Y();
            aPt = OutputToScreenPixel( aItemRect.BottomRight() );
            aItemRect.Right()  = aPt.X();
            aItemRect.Bottom() = aPt.Y();

            sal_uInt16 nPos = GetModelColumnPos(nItemId);
            Reference< ::com::sun::star::container::XIndexContainer >  xColumns(static_cast<FmGridControl*>(GetParent())->GetPeer()->getColumns());
            try
            {
                Reference< ::com::sun::star::beans::XPropertySet >  xColumn(xColumns->getByIndex(nPos),UNO_QUERY);
                OUString aHelpText;
                xColumn->getPropertyValue(FM_PROP_HELPTEXT) >>= aHelpText;
                if ( aHelpText.isEmpty() )
                    xColumn->getPropertyValue(FM_PROP_DESCRIPTION) >>= aHelpText;
                if ( !aHelpText.isEmpty() )
                {
                    if ( rHEvt.GetMode() & HELPMODE_BALLOON )
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

//------------------------------------------------------------------------------
sal_Int8 FmGridHeader::AcceptDrop( const AcceptDropEvent& rEvt )
{
    // drop allowed in design mode only
    if (!static_cast<FmGridControl*>(GetParent())->IsDesignMode())
        return DND_ACTION_NONE;

    // search for recognized formats
    const DataFlavorExVector& rFlavors = GetDataFlavorExVector();
    if (OColumnTransferable::canExtractColumnDescriptor(rFlavors, CTF_COLUMN_DESCRIPTOR | CTF_FIELD_DESCRIPTOR))
        return rEvt.mnAction;

    return DND_ACTION_NONE;
}

//------------------------------------------------------------------------------
sal_Int8 FmGridHeader::ExecuteDrop( const ExecuteDropEvent& _rEvt )
{
    if (!static_cast<FmGridControl*>(GetParent())->IsDesignMode())
        return DND_ACTION_NONE;

    TransferableDataHelper aDroppedData(_rEvt.maDropEvent.Transferable);

    // check the formats
    sal_Bool bColumnDescriptor  = OColumnTransferable::canExtractColumnDescriptor(aDroppedData.GetDataFlavorExVector(), CTF_COLUMN_DESCRIPTOR);
    sal_Bool bFieldDescriptor   = OColumnTransferable::canExtractColumnDescriptor(aDroppedData.GetDataFlavorExVector(), CTF_FIELD_DESCRIPTOR);
    if (!bColumnDescriptor && !bFieldDescriptor)
    {
        OSL_FAIL("FmGridHeader::ExecuteDrop: should never have reached this (no extractable format)!");
        return DND_ACTION_NONE;
    }

    // extract the descriptor
    OUString sDatasouce, sCommand, sFieldName,sDatabaseLocation,sConnnectionResource;
    sal_Int32       nCommandType = CommandType::COMMAND;
    Reference< XPreparedStatement >     xStatement;
    Reference< XResultSet >             xResultSet;
    Reference< XPropertySet >           xField;
    Reference< XConnection >            xConnection;

    ODataAccessDescriptor aColumn = OColumnTransferable::extractColumnDescriptor(aDroppedData);
    if (aColumn.has(daDataSource))  aColumn[daDataSource]   >>= sDatasouce;
    if (aColumn.has(daDatabaseLocation))    aColumn[daDatabaseLocation] >>= sDatabaseLocation;
    if (aColumn.has(daConnectionResource))  aColumn[daConnectionResource] >>= sConnnectionResource;
    if (aColumn.has(daCommand))     aColumn[daCommand]      >>= sCommand;
    if (aColumn.has(daCommandType)) aColumn[daCommandType]  >>= nCommandType;
    if (aColumn.has(daColumnName))  aColumn[daColumnName]   >>= sFieldName;
    if (aColumn.has(daColumnObject))aColumn[daColumnObject] >>= xField;
    if (aColumn.has(daConnection))  aColumn[daConnection]   >>= xConnection;

    if  (   sFieldName.isEmpty()
        ||  sCommand.isEmpty()
        ||  (   sDatasouce.isEmpty()
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
                OUString sSignificantSource( sDatasouce.isEmpty() ? sDatabaseLocation : sDatasouce );
                xConnection = OStaticDataAccessTools().getConnection_withFeedback(sSignificantSource, OUString(), OUString(),
                                  static_cast<FmGridControl*>(GetParent())->getContext() );
            }
            catch(NoSuchElementException&)
            {   // allowed, means sDatasouce isn't a valid data source name ....
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
        m_pImpl->aDropData[daConnection] <<= xConnection;
        m_pImpl->aDropData[daColumnObject] <<= xField;

        m_pImpl->nDropAction = _rEvt.mnAction;
        m_pImpl->aDropPosPixel = _rEvt.maPosPixel;
        m_pImpl->xDroppedStatement = xStatement;
        m_pImpl->xDroppedResultSet = xResultSet;

        PostUserEvent(LINK(this, FmGridHeader, OnAsyncExecuteDrop));
    }
    catch (Exception&)
    {
        OSL_FAIL("FmGridHeader::ExecuteDrop: caught an exception while creatin' the column !");
        ::comphelper::disposeComponent(xStatement);
        return sal_False;
    }

    return DND_ACTION_LINK;
}

//------------------------------------------------------------------------------
IMPL_LINK( FmGridHeader, OnAsyncExecuteDrop, void*, /*NOTINTERESTEDIN*/ )
{
    OUString             sCommand, sFieldName,sURL;
    sal_Int32                   nCommandType = CommandType::COMMAND;
    Reference< XPropertySet >   xField;
    Reference< XConnection >    xConnection;

    OUString sDatasouce = m_pImpl->aDropData.getDataSource();
    if ( sDatasouce.isEmpty() && m_pImpl->aDropData.has(daConnectionResource) )
        m_pImpl->aDropData[daConnectionResource]    >>= sURL;
    m_pImpl->aDropData[daCommand]       >>= sCommand;
    m_pImpl->aDropData[daCommandType]   >>= nCommandType;
    m_pImpl->aDropData[daColumnName]    >>= sFieldName;
    m_pImpl->aDropData[daConnection]    >>= xConnection;
    m_pImpl->aDropData[daColumnObject]  >>= xField;

    try
    {
        // need number formats
        Reference< XNumberFormatsSupplier > xSupplier = OStaticDataAccessTools().getNumberFormats(xConnection, sal_True);
        Reference< XNumberFormats >  xNumberFormats;
        if (xSupplier.is())
            xNumberFormats = xSupplier->getNumberFormats();
        if (!xNumberFormats.is())
        {
            ::comphelper::disposeComponent(m_pImpl->xDroppedResultSet);
            ::comphelper::disposeComponent(m_pImpl->xDroppedStatement);
            return 0L;
        }

        // Vom Feld werden nun zwei Informationen benoetigt:
        // a.) Name des Feldes fuer Label und ControlSource
        // b.) FormatKey, um festzustellen, welches Feld erzeugt werden soll
        sal_Int32 nDataType = 0;
        xField->getPropertyValue(FM_PROP_FIELDTYPE) >>= nDataType;
        // diese Datentypen koennen im Gridcontrol nicht verarbeitet werden
        switch (nDataType)
        {
            case DataType::BLOB:
            case DataType::LONGVARBINARY:
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::OTHER:
                ::comphelper::disposeComponent(m_pImpl->xDroppedResultSet);
                ::comphelper::disposeComponent(m_pImpl->xDroppedStatement);
                return 0L;
        }

        // Erstellen der Column
        Reference< XIndexContainer >  xCols(static_cast<FmGridControl*>(GetParent())->GetPeer()->getColumns());
        Reference< XGridColumnFactory >  xFactory(xCols, UNO_QUERY);

        sal_uInt16 nColId = GetItemId(m_pImpl->aDropPosPixel);
        // EinfuegePosition, immer vor der aktuellen Spalte
        sal_uInt16 nPos = GetModelColumnPos(nColId);
        Reference< XPropertySet >  xCol, xSecondCol;

        // Create Column based on type, default textfield
        std::vector<sal_uInt16> aPossibleTypes;
        switch (nDataType)
        {
            case DataType::BIT:
            case DataType::BOOLEAN:
                aPossibleTypes.push_back(SID_FM_CHECKBOX);
                break;
            case DataType::TINYINT:
            case DataType::SMALLINT:
            case DataType::INTEGER:
                aPossibleTypes.push_back(SID_FM_NUMERICFIELD);
                aPossibleTypes.push_back(SID_FM_FORMATTEDFIELD);
                break;
            case DataType::REAL:
            case DataType::DOUBLE:
            case DataType::NUMERIC:
            case DataType::DECIMAL:
                aPossibleTypes.push_back(SID_FM_FORMATTEDFIELD);
                aPossibleTypes.push_back(SID_FM_NUMERICFIELD);
                break;
            case DataType::TIMESTAMP:
                aPossibleTypes.push_back(SID_FM_TWOFIELDS_DATE_N_TIME);
                aPossibleTypes.push_back(SID_FM_DATEFIELD);
                aPossibleTypes.push_back(SID_FM_TIMEFIELD);
                aPossibleTypes.push_back(SID_FM_FORMATTEDFIELD);
                break;
            case DataType::DATE:
                aPossibleTypes.push_back(SID_FM_DATEFIELD);
                aPossibleTypes.push_back(SID_FM_FORMATTEDFIELD);
                break;
            case DataType::TIME:
                aPossibleTypes.push_back(SID_FM_TIMEFIELD);
                aPossibleTypes.push_back(SID_FM_FORMATTEDFIELD);
                break;
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::LONGVARCHAR:
            default:
                aPossibleTypes.push_back(SID_FM_EDIT);
                aPossibleTypes.push_back(SID_FM_FORMATTEDFIELD);
                break;
        }
        // if it's a currency field, a a "currency field" option
        try
        {
            if  (   ::comphelper::hasProperty(FM_PROP_ISCURRENCY, xField)
                &&  ::comphelper::getBOOL(xField->getPropertyValue(FM_PROP_ISCURRENCY)))
                aPossibleTypes.insert(aPossibleTypes.begin(), SID_FM_CURRENCYFIELD);
        }
        catch(Exception&)
        {
            OSL_FAIL("FmGridHeader::ExecuteDrop: Exception occurred!");
        }

        sal_Bool bDateNTimeCol = sal_False;
        if (!aPossibleTypes.empty())
        {
            sal_Int32 nPreferedType = aPossibleTypes[0];
            if ((m_pImpl->nDropAction == DND_ACTION_LINK) && (aPossibleTypes.size() > 1))
            {
                ImageList aImageList( SVX_RES(RID_SVXIMGLIST_FMEXPL) );

                PopupMenu aInsertMenu(SVX_RES(RID_SVXMNU_COLS));
                PopupMenu aTypeMenu;
                PopupMenu* pMenu = aInsertMenu.GetPopupMenu(SID_FM_INSERTCOL);
                for (std::vector<sal_uInt16>::const_iterator iter = aPossibleTypes.begin(); iter != aPossibleTypes.end(); ++iter)
                    SetMenuItem(aImageList, *iter, pMenu, aTypeMenu, sal_True, 0);
                nPreferedType = aTypeMenu.Execute(this, m_pImpl->aDropPosPixel);
            }

            bDateNTimeCol = nPreferedType == SID_FM_TWOFIELDS_DATE_N_TIME;
            sal_uInt16 nColCount = bDateNTimeCol ? 2 : 1;
            OUString sFieldService;
            while (nColCount--)
            {
                if (bDateNTimeCol)
                    nPreferedType = nColCount ? SID_FM_DATEFIELD : SID_FM_TIMEFIELD;

                sFieldService = FieldServiceFromId(nPreferedType);
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
            return 0L;
        }

        if (bDateNTimeCol)
        {
            OUString sTimePostfix(SVX_RESSTR(RID_STR_POSTFIX_TIME));
            xCol->setPropertyValue(FM_PROP_LABEL, makeAny( OUString( sFieldName + sTimePostfix ) ) );

            OUString sDatePostfix(SVX_RESSTR( RID_STR_POSTFIX_DATE));
            xSecondCol->setPropertyValue(FM_PROP_LABEL, makeAny( OUString( sFieldName + sDatePostfix ) ) );
        }
        else
            xCol->setPropertyValue(FM_PROP_LABEL, makeAny(sFieldName));

        FormControlFactory aControlFactory;
        aControlFactory.initializeControlModel( DocumentClassification::classifyHostDocument( xCols ), xCol );
        aControlFactory.initializeFieldDependentProperties( xField, xCol, xNumberFormats );

        xCol->setPropertyValue(FM_PROP_CONTROLSOURCE, makeAny(sFieldName));
        if ( xSecondCol.is() )
            xSecondCol->setPropertyValue(FM_PROP_CONTROLSOURCE, makeAny(sFieldName));

        if (bDateNTimeCol)
        {
            String sRealName,sPurePostfix;

            OUString aPostfix[] = {
                SVX_RESSTR(RID_STR_POSTFIX_DATE),
                SVX_RESSTR(RID_STR_POSTFIX_TIME)
            };

            for ( size_t i=0; i<2; ++i )
            {
                sPurePostfix = comphelper::string::stripStart(aPostfix[i], ' ');
                sPurePostfix = comphelper::string::stripStart(sPurePostfix, '(');
                sPurePostfix = comphelper::string::stripEnd(sPurePostfix, ')');
                sRealName = sFieldName;
                sRealName += '_';
                sRealName += sPurePostfix;
                if (i)
                    xSecondCol->setPropertyValue(FM_PROP_NAME, makeAny(OUString(sRealName)));
                else
                    xCol->setPropertyValue(FM_PROP_NAME, makeAny(OUString(sRealName)));
            }
        }
        else
            xCol->setPropertyValue(FM_PROP_NAME, makeAny(sFieldName));

        // jetzt einfuegen
        Any aElement;
        aElement <<= xCol;
        xCols->insertByIndex(nPos, aElement);

        if (bDateNTimeCol)
        {
            aElement <<= xSecondCol;
            xCols->insertByIndex(nPos == (sal_uInt16)-1 ? nPos : ++nPos, aElement);
        }

        // ist die component::Form an die Datenbankangebunden?
        Reference< XFormComponent >  xFormCp(xCols, UNO_QUERY);
        Reference< XPropertySet >  xForm(xFormCp->getParent(), UNO_QUERY);
        if (xForm.is())
        {
            if (::comphelper::getString(xForm->getPropertyValue(FM_PROP_DATASOURCE)).isEmpty())
            {
                if ( !sDatasouce.isEmpty() )
                    xForm->setPropertyValue(FM_PROP_DATASOURCE, makeAny(sDatasouce));
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
                        aCommandType <<= (sal_Int32)CommandType::TABLE;
                        break;
                    case CommandType::QUERY:
                        aCommandType <<= (sal_Int32)CommandType::QUERY;
                        break;
                    default:
                        aCommandType <<= (sal_Int32)CommandType::COMMAND;
                        xForm->setPropertyValue(FM_PROP_ESCAPE_PROCESSING, bool2any((sal_Bool)(2 == nCommandType)));
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
        return 0L;
    }

    ::comphelper::disposeComponent(m_pImpl->xDroppedResultSet);
    ::comphelper::disposeComponent(m_pImpl->xDroppedStatement);
    return 1L;
}

//------------------------------------------------------------------------------
void FmGridHeader::PreExecuteColumnContextMenu(sal_uInt16 nColId, PopupMenu& rMenu)
{
    sal_Bool bDesignMode = static_cast<FmGridControl*>(GetParent())->IsDesignMode();

    Reference< ::com::sun::star::container::XIndexContainer >  xCols(static_cast<FmGridControl*>(GetParent())->GetPeer()->getColumns());
    // Aufbau des Insert Menues
    // mark the column if nColId != HEADERBAR_ITEM_NOTFOUND
    if(nColId > 0)
    {
        sal_uInt16 nPos2 = GetModelColumnPos(nColId);

        Reference< ::com::sun::star::container::XIndexContainer >  xColumns(static_cast<FmGridControl*>(GetParent())->GetPeer()->getColumns());
        Reference< ::com::sun::star::beans::XPropertySet> xColumn;
        ::cppu::extractInterface(xColumn, xColumns->getByIndex(nPos2));
        Reference< ::com::sun::star::view::XSelectionSupplier >  xSelSupplier(xColumns, UNO_QUERY);
        if (xSelSupplier.is())
            xSelSupplier->select(makeAny(xColumn));
    }

    // EinfuegePosition, immer vor der aktuellen Spalte
    sal_uInt16 nPos = GetModelColumnPos(nColId);
    sal_Bool bMarked = nColId && static_cast<FmGridControl*>(GetParent())->isColumnMarked(nColId);

    ImageList aImageList( SVX_RES(RID_SVXIMGLIST_FMEXPL) );
    PopupMenu* pControlMenu = new PopupMenu;

    PopupMenu* pMenu = rMenu.GetPopupMenu(SID_FM_INSERTCOL);
    if (pMenu)
    {
        SetMenuItem(aImageList, SID_FM_EDIT, pMenu, *pControlMenu, bDesignMode);
        SetMenuItem(aImageList, SID_FM_CHECKBOX, pMenu, *pControlMenu, bDesignMode);
        SetMenuItem(aImageList, SID_FM_COMBOBOX, pMenu, *pControlMenu, bDesignMode);
        SetMenuItem(aImageList, SID_FM_LISTBOX, pMenu, *pControlMenu, bDesignMode);
        SetMenuItem(aImageList, SID_FM_DATEFIELD, pMenu, *pControlMenu, bDesignMode);
        SetMenuItem(aImageList, SID_FM_TIMEFIELD, pMenu, *pControlMenu, bDesignMode);
        SetMenuItem(aImageList, SID_FM_NUMERICFIELD, pMenu, *pControlMenu, bDesignMode);
        SetMenuItem(aImageList, SID_FM_CURRENCYFIELD, pMenu, *pControlMenu, bDesignMode);
        SetMenuItem(aImageList, SID_FM_PATTERNFIELD, pMenu, *pControlMenu, bDesignMode);
        SetMenuItem(aImageList, SID_FM_FORMATTEDFIELD, pMenu, *pControlMenu, bDesignMode);
    }

    if (pMenu && xCols.is() && nColId)
    {
        Reference< ::com::sun::star::beans::XPropertySet > xSet;
        ::cppu::extractInterface(xSet, xCols->getByIndex(nPos));
        sal_Int16 nClassId;
        xSet->getPropertyValue(FM_PROP_CLASSID) >>= nClassId;

        Reference< ::com::sun::star::io::XPersistObject >  xServiceQuestion(xSet, UNO_QUERY);
        sal_Int32 nColType = xServiceQuestion.is() ? getColumnTypeByModelName(xServiceQuestion->getServiceName()) : 0;
        if (nColType == TYPE_TEXTFIELD)
        {   // edit fields and formatted fields have the same service name, thus getColumnTypeByModelName returns TYPE_TEXTFIELD
            // in both cases. And as columns don't have an ::com::sun::star::lang::XServiceInfo interface, we have to distinguish both
            // types via the existence of special properties
            Reference< ::com::sun::star::beans::XPropertySet >  xProps(xSet, UNO_QUERY);
            if (xProps.is())
            {
                Reference< ::com::sun::star::beans::XPropertySetInfo >  xPropsInfo = xProps->getPropertySetInfo();
                if (xPropsInfo.is() && xPropsInfo->hasPropertyByName(FM_PROP_FORMATSSUPPLIER))
                    nColType = TYPE_FORMATTEDFIELD;
            }
        }

        pControlMenu->EnableItem(SID_FM_EDIT + nChangeTypeOffset, bDesignMode && (nColType != TYPE_TEXTFIELD));
        pControlMenu->EnableItem(SID_FM_COMBOBOX + nChangeTypeOffset, bDesignMode && (nColType != TYPE_COMBOBOX));
        pControlMenu->EnableItem(SID_FM_LISTBOX + nChangeTypeOffset, bDesignMode && (nColType != TYPE_LISTBOX));
        pControlMenu->EnableItem(SID_FM_CHECKBOX + nChangeTypeOffset, bDesignMode && (nColType != TYPE_CHECKBOX));
        pControlMenu->EnableItem(SID_FM_DATEFIELD + nChangeTypeOffset, bDesignMode && (nColType != TYPE_DATEFIELD));
        pControlMenu->EnableItem(SID_FM_NUMERICFIELD + nChangeTypeOffset, bDesignMode && (nColType != TYPE_NUMERICFIELD));
        pControlMenu->EnableItem(SID_FM_TIMEFIELD + nChangeTypeOffset, bDesignMode && (nColType != TYPE_TIMEFIELD));
        pControlMenu->EnableItem(SID_FM_CURRENCYFIELD + nChangeTypeOffset, bDesignMode && (nColType != TYPE_CURRENCYFIELD));
        pControlMenu->EnableItem(SID_FM_PATTERNFIELD + nChangeTypeOffset, bDesignMode && (nColType != TYPE_PATTERNFIELD));
        pControlMenu->EnableItem(SID_FM_FORMATTEDFIELD + nChangeTypeOffset, bDesignMode && (nColType != TYPE_FORMATTEDFIELD));
        rMenu.SetPopupMenu(SID_FM_CHANGECOL, pControlMenu);
    }

    rMenu.EnableItem(SID_FM_INSERTCOL, bDesignMode && xCols.is());
    rMenu.EnableItem(SID_FM_DELETECOL, bDesignMode && bMarked && xCols.is());
    rMenu.EnableItem(SID_FM_CHANGECOL, bDesignMode && bMarked && xCols.is());
    rMenu.EnableItem(SID_FM_SHOW_PROPERTY_BROWSER, bDesignMode && bMarked && xCols.is());

    PopupMenu* pShowColsMenu = rMenu.GetPopupMenu(SID_FM_SHOWCOLS);
    sal_uInt16 nHiddenCols = 0;
    if (pShowColsMenu)
    {
        if (xCols.is())
        {
            // check for hidden cols
            Reference< ::com::sun::star::beans::XPropertySet >  xCurCol;
            Any aHidden,aName;
            for (sal_uInt16 i=0; i<xCols->getCount(); ++i)
            {
                ::cppu::extractInterface(xCurCol, xCols->getByIndex(i));
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
                            0, OString(), nHiddenCols);
                            // the ID is arbitrary, but should be unique within the whole menu
                    }
                    ++nHiddenCols;
                }
            }
        }
        pShowColsMenu->EnableItem(SID_FM_SHOWCOLS_MORE, xCols.is() && (nHiddenCols > 16));
        pShowColsMenu->EnableItem(SID_FM_SHOWALLCOLS, xCols.is() && (nHiddenCols > 0));
    }

    // allow the 'hide column' item ?
    sal_Bool bAllowHide = bMarked;                                          // a column is marked
    bAllowHide = bAllowHide || (!bDesignMode && (nPos != (sal_uInt16)-1));  // OR we are in alive mode and have hit a column
    bAllowHide = bAllowHide && xCols.is();                              // AND we have a column container
    bAllowHide = bAllowHide && (xCols->getCount()-nHiddenCols > 1);     // AND there are at least two visible columns
    rMenu.EnableItem(SID_FM_HIDECOL,  bAllowHide);

    sal_Bool bChecked = sal_False;
    if (bMarked)
    {

        SfxViewFrame* pCurrentFrame = SfxViewFrame::Current();
        SfxItemState eState = SFX_ITEM_UNKNOWN;
        // ask the bindings of the current view frame (which should be the one we're residing in) for the state
        if (pCurrentFrame)
        {
            SfxPoolItem* pItem = NULL;
            eState = pCurrentFrame->GetBindings().QueryState(SID_FM_CTL_PROPERTIES, pItem);

            if (eState >= SFX_ITEM_AVAILABLE && pItem )
            {
                bChecked = pItem->ISA(SfxBoolItem) && ((SfxBoolItem*)pItem)->GetValue();
                rMenu.CheckItem(SID_FM_SHOW_PROPERTY_BROWSER,bChecked);
            }
            delete pItem;
        }
    }
}

enum InspectorAction { eOpenInspector, eCloseInspector, eUpdateInspector, eNone };

//------------------------------------------------------------------------------
void FmGridHeader::PostExecuteColumnContextMenu(sal_uInt16 nColId, const PopupMenu& rMenu, sal_uInt16 nExecutionResult)
{
    Reference< ::com::sun::star::container::XIndexContainer >  xCols(static_cast<FmGridControl*>(GetParent())->GetPeer()->getColumns());
    sal_uInt16 nPos = GetModelColumnPos(nColId);

    // remove and delet the menu we inserted in PreExecuteColumnContextMenu
    PopupMenu* pControlMenu = rMenu.GetPopupMenu(SID_FM_CHANGECOL);
    delete pControlMenu;

    OUString aFieldType;
    sal_Bool    bReplace = sal_False;
    InspectorAction eInspectorAction = eNone;
    Reference< XPropertySet > xColumnToInspect;
    switch (nExecutionResult)
    {
        case SID_FM_DELETECOL:
        {
            Reference< XInterface >  xCol;
            ::cppu::extractInterface(xCol, xCols->getByIndex(nPos));
            xCols->removeByIndex(nPos);
            ::comphelper::disposeComponent(xCol);
        }   break;
        case SID_FM_SHOW_PROPERTY_BROWSER:
            eInspectorAction = rMenu.IsItemChecked( SID_FM_SHOW_PROPERTY_BROWSER ) ? eOpenInspector : eCloseInspector;
            xColumnToInspect.set( xCols->getByIndex( nPos ), UNO_QUERY );
            break;
        case SID_FM_EDIT + nChangeTypeOffset:
            bReplace = sal_True;
        case SID_FM_EDIT:
            aFieldType = FM_COL_TEXTFIELD;
            break;
        case SID_FM_COMBOBOX + nChangeTypeOffset:
            bReplace = sal_True;
        case SID_FM_COMBOBOX:
            aFieldType = FM_COL_COMBOBOX;
            break;
        case SID_FM_LISTBOX + nChangeTypeOffset:
            bReplace = sal_True;
        case SID_FM_LISTBOX:
            aFieldType = FM_COL_LISTBOX;
            break;
        case SID_FM_CHECKBOX + nChangeTypeOffset:
            bReplace = sal_True;
        case SID_FM_CHECKBOX:
            aFieldType = FM_COL_CHECKBOX;
            break;
        case SID_FM_DATEFIELD + nChangeTypeOffset:
            bReplace = sal_True;
        case SID_FM_DATEFIELD:
            aFieldType = FM_COL_DATEFIELD;
            break;
        case SID_FM_TIMEFIELD + nChangeTypeOffset:
            bReplace = sal_True;
        case SID_FM_TIMEFIELD:
            aFieldType = FM_COL_TIMEFIELD;
            break;
        case SID_FM_NUMERICFIELD + nChangeTypeOffset:
            bReplace = sal_True;
        case SID_FM_NUMERICFIELD:
            aFieldType = FM_COL_NUMERICFIELD;
            break;
        case SID_FM_CURRENCYFIELD + nChangeTypeOffset:
            bReplace = sal_True;
        case SID_FM_CURRENCYFIELD:
            aFieldType = FM_COL_CURRENCYFIELD;
            break;
        case SID_FM_PATTERNFIELD + nChangeTypeOffset:
            bReplace = sal_True;
        case SID_FM_PATTERNFIELD:
            aFieldType = FM_COL_PATTERNFIELD;
            break;
        case SID_FM_FORMATTEDFIELD + nChangeTypeOffset:
            bReplace = sal_True;
        case SID_FM_FORMATTEDFIELD:
            aFieldType = FM_COL_FORMATTEDFIELD;
            break;
        case SID_FM_HIDECOL:
        {
            Reference< ::com::sun::star::beans::XPropertySet >  xCurCol;
            ::cppu::extractInterface(xCurCol, xCols->getByIndex(nPos));
            xCurCol->setPropertyValue(FM_PROP_HIDDEN, makeAny((sal_Bool)sal_True));
        }
        break;
        case SID_FM_SHOWCOLS_MORE:
        {
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            if(pFact)
            {
                AbstractFmShowColsDialog* pDlg = pFact->CreateFmShowColsDialog(NULL);
                DBG_ASSERT(pDlg, "Dialogdiet fail!");
                pDlg->SetColumns(xCols);
                pDlg->Execute();
                delete pDlg;
            }

        }
        break;
        case SID_FM_SHOWALLCOLS:
        {
            // just iterate through all the cols ...
            Reference< ::com::sun::star::beans::XPropertySet >  xCurCol;
            for (sal_uInt16 i=0; i<xCols->getCount(); ++i)
            {
                ::cppu::extractInterface(xCurCol, xCols->getByIndex(i));
                xCurCol->setPropertyValue(FM_PROP_HIDDEN, makeAny((sal_Bool)sal_False));
            }
            // TODO : there must be a more clever way to do this ....
            // with the above the view is updated after every single model update ...
        }
        break;
        default:
            if (nExecutionResult>0 && nExecutionResult<=16)
            {   // it was a "show column/<colname>" command (there are at most 16 such items)
                // search the nExecutionResult'th hidden col
                Reference< ::com::sun::star::beans::XPropertySet >  xCurCol;
                for (sal_uInt16 i=0; i<xCols->getCount() && nExecutionResult; ++i)
                {
                    ::cppu::extractInterface(xCurCol, xCols->getByIndex(i));
                    Any aHidden = xCurCol->getPropertyValue(FM_PROP_HIDDEN);
                    if (::comphelper::getBOOL(aHidden))
                        if (!--nExecutionResult)
                        {
                            xCurCol->setPropertyValue(FM_PROP_HIDDEN, makeAny((sal_Bool)sal_False));
                            break;
                        }
                }
            }
            break;
    }

    if ( !aFieldType.isEmpty() )
    {
        try
        {
            Reference< XGridColumnFactory > xFactory( xCols, UNO_QUERY_THROW );
            Reference< XPropertySet > xNewCol( xFactory->createColumn( aFieldType ), UNO_SET_THROW );

            if ( bReplace )
            {
                // ein paar Properties hinueberretten
                Reference< XPropertySet > xReplaced( xCols->getByIndex( nPos ), UNO_QUERY );

                OStaticDataAccessTools().TransferFormComponentProperties(
                    xReplaced, xNewCol, Application::GetSettings().GetUILanguageTag().getLocale() );

                xCols->replaceByIndex( nPos, makeAny( xNewCol ) );
                ::comphelper::disposeComponent( xReplaced );

                eInspectorAction = eUpdateInspector;
                xColumnToInspect = xNewCol;
            }
            else
            {
                FormControlFactory factory;

                OUString sLabel = factory.getDefaultUniqueName_ByComponentType(
                    Reference< XNameAccess >( xCols, UNO_QUERY_THROW ), xNewCol );
                xNewCol->setPropertyValue( FM_PROP_LABEL, makeAny( sLabel ) );
                xNewCol->setPropertyValue( FM_PROP_NAME, makeAny( sLabel ) );

                factory.initializeControlModel( DocumentClassification::classifyHostDocument( xCols ), xNewCol );

                xCols->insertByIndex( nPos, makeAny( xNewCol ) );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
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
            FmInterfaceItem aIFaceItem( SID_FM_SHOW_PROPERTY_BROWSER, xColumnToInspect );
            SfxBoolItem aShowItem( SID_FM_SHOW_PROPERTIES, eInspectorAction == eCloseInspector ? sal_False : sal_True );

            pCurrentFrame->GetBindings().GetDispatcher()->Execute( SID_FM_SHOW_PROPERTY_BROWSER, SFX_CALLMODE_ASYNCHRON,
                                      &aIFaceItem, &aShowItem, 0L );
        }
    }
}

//------------------------------------------------------------------------------
void FmGridHeader::triggerColumnContextMenu( const ::Point& _rPreferredPos )
{
    // the affected col
    sal_uInt16 nColId = GetItemId( _rPreferredPos );

    // the menu
    PopupMenu aContextMenu( SVX_RES( RID_SVXMNU_COLS ) );

    // let derivees modify the menu
    PreExecuteColumnContextMenu( nColId, aContextMenu );
    aContextMenu.RemoveDisabledEntries( sal_True, sal_True );

    // execute the menu
    sal_uInt16 nResult = aContextMenu.Execute( this, _rPreferredPos );

    // let derivees handle the result
    PostExecuteColumnContextMenu( nColId, aContextMenu, nResult );
}

//------------------------------------------------------------------------------
void FmGridHeader::Command(const CommandEvent& rEvt)
{
    switch (rEvt.GetCommand())
    {
        case COMMAND_CONTEXTMENU:
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

//------------------------------------------------------------------------------
FmGridControl::FmGridControl(
                const Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
                Window* pParent,
                FmXGridPeer* _pPeer,
                WinBits nBits)
        :DbGridControl(_rxContext, pParent, nBits)
        ,m_pPeer(_pPeer)
        ,m_nCurrentSelectedColumn(-1)
        ,m_nMarkedColumnId(BROWSER_INVALIDID)
        ,m_bSelecting(sal_False)
        ,m_bInColumnMove(sal_False)
{
    EnableInteractiveRowHeight( );
}

//------------------------------------------------------------------------------
void FmGridControl::Command(const CommandEvent& _rEvt)
{
    if ( COMMAND_CONTEXTMENU == _rEvt.GetCommand() )
    {
        FmGridHeader* pMyHeader = static_cast< FmGridHeader* >( GetHeaderBar() );
        if ( pMyHeader && !_rEvt.IsMouseEvent() )
        {   // context menu requested by keyboard
            if  ( 1 == GetSelectColumnCount() || IsDesignMode() )
            {
                sal_uInt16 nSelId = GetColumnId(
                    sal::static_int_cast< sal_uInt16 >( FirstSelectedColumn() ) );
                ::Rectangle aColRect( GetFieldRectPixel( 0, nSelId, sal_False ) );

                Point aRelativePos( pMyHeader->ScreenToOutputPixel( OutputToScreenPixel( aColRect.TopCenter() ) ) );
                pMyHeader->triggerColumnContextMenu( aRelativePos, FmGridHeader::AccessControl() );

                // handled
                return;
            }
        }
    }

    DbGridControl::Command( _rEvt );
}

// ::com::sun::star::beans::XPropertyChangeListener
//------------------------------------------------------------------------------
void FmGridControl::propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt)
{
    if (evt.PropertyName == FM_PROP_ROWCOUNT)
    {
        // if we're not in the main thread call AdjustRows asynchronously
        implAdjustInSolarThread(sal_True);
        return;
    }

    const DbGridRowRef& xRow = GetCurrentRow();
    // waehrend Positionierung wird kein abgleich  der Properties vorgenommen
    Reference<XPropertySet> xSet(evt.Source,UNO_QUERY);
    if (xRow.Is() && (::cppu::any2bool(xSet->getPropertyValue(FM_PROP_ISNEW))|| CompareBookmark(getDataSource()->getBookmark(), xRow->GetBookmark())))
    {
        if (evt.PropertyName == FM_PROP_ISMODIFIED)
        {
            // modified or clean ?
            GridRowStatus eStatus = ::comphelper::getBOOL(evt.NewValue) ? GRS_MODIFIED : GRS_CLEAN;
            if (eStatus != xRow->GetStatus())
            {
                xRow->SetStatus(eStatus);
                SolarMutexGuard aGuard;
                RowModified(GetCurrentPos());
            }
        }
    }
}

//------------------------------------------------------------------------------
void FmGridControl::SetDesignMode(sal_Bool bMode)
{
    sal_Bool bOldMode = IsDesignMode();
    DbGridControl::SetDesignMode(bMode);
    if (bOldMode != bMode)
    {
        if (!bMode)
        {
            // selection aufheben
            markColumn(USHRT_MAX);
        }
        else
        {
            Reference< ::com::sun::star::container::XIndexContainer >  xColumns(GetPeer()->getColumns());
            Reference< ::com::sun::star::view::XSelectionSupplier >  xSelSupplier(xColumns, UNO_QUERY);
            if (xSelSupplier.is())
            {
                Any aSelection = xSelSupplier->getSelection();
                Reference< ::com::sun::star::beans::XPropertySet >  xColumn;
                if (aSelection.getValueType().getTypeClass() == TypeClass_INTERFACE)
                    ::cppu::extractInterface(xColumn, aSelection);
                Reference< XInterface >  xCurrent;
                for (sal_uInt16 i=0; i<xColumns->getCount(); ++i)
                {
                    ::cppu::extractInterface(xCurrent, xColumns->getByIndex(i));
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

//------------------------------------------------------------------------------
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
    if (GetEmptyRow().Is() && IsRowSelected(GetRowCount() - 1))
        nSelectedRows -= 1;

    // nothing to do
    if (nSelectedRows <= 0)
        return;

    // try to confirm the delete
    Reference< ::com::sun::star::frame::XDispatchProvider >  xDispatcher = (::com::sun::star::frame::XDispatchProvider*)GetPeer();
    if (xDispatcher.is())
    {
        ::com::sun::star::util::URL aUrl;
        aUrl.Complete = FMURL_CONFIRM_DELETION;
        // #100312# ------------
        Reference< ::com::sun::star::util::XURLTransformer > xTransformer(
            ::com::sun::star::util::URLTransformer::create(::comphelper::getProcessComponentContext()) );
        xTransformer->parseStrict( aUrl );

        Reference< ::com::sun::star::frame::XDispatch >  xDispatch = xDispatcher->queryDispatch(aUrl, OUString(), 0);
        Reference< ::com::sun::star::form::XConfirmDeleteListener >  xConfirm(xDispatch, UNO_QUERY);
        if (xConfirm.is())
        {
            ::com::sun::star::sdb::RowChangeEvent aEvent;
            aEvent.Source = (Reference< XInterface > )(*getDataSource());
            aEvent.Rows = nSelectedRows;
            aEvent.Action = ::com::sun::star::sdb::RowChangeAction::DELETE;
            if (!xConfirm->confirmDelete(aEvent))
                return;
        }
    }

    const MultiSelection* pRowSelection = GetSelection();
    if ( pRowSelection && pRowSelection->IsAllSelected() )
    {
        BeginCursorAction();
        CursorWrapper* pCursor = getDataSource();
        Reference< XResultSetUpdate >  xUpdateCursor((Reference< XInterface >)*pCursor, UNO_QUERY);
        try
        {
            pCursor->beforeFirst();
            while( pCursor->next() )
                xUpdateCursor->deleteRow();

            SetUpdateMode(sal_False);
            SetNoSelection();

            xUpdateCursor->moveToInsertRow();
        }
        catch(const Exception&)
        {
            OSL_FAIL("Exception caught while deleting rows!");
        }
        // An den DatenCursor anpassen
        AdjustDataSource(sal_True);
        EndCursorAction();
        SetUpdateMode(sal_True);
    }
    else
    {
        Reference< ::com::sun::star::sdbcx::XDeleteRows >  xDeleteThem((Reference< XInterface >)*getDataSource(), UNO_QUERY);

        // colect the bookmarks of the selected rows
        Sequence < Any> aBookmarks = getSelectionBookmarks();

        // determine the next row to position after deletion
        Any aBookmark;
        sal_Bool bNewPos = sal_False;
        // if the current row isn't selected we take the row as row after deletion
        OSL_ENSURE( GetCurrentRow().Is(), "FmGridControl::DeleteSelectedRows: no current row here?" );
            // crash reports suggest it can happen we don't have a current row - how?
            // #154303# / 2008-04-23 / frank.schoenheit@sun.com
        if ( !IsRowSelected( GetCurrentPos() ) && !IsCurrentAppending() && GetCurrentRow().Is() )
        {
            aBookmark = GetCurrentRow()->GetBookmark();
            bNewPos   = sal_True;
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
                    GetSeekRow()->SetState(m_pSeekCursor, sal_True);

                    bNewPos = sal_True;
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
                    GetSeekRow()->SetState(m_pSeekCursor, sal_True);

                    bNewPos = sal_True;
                    aBookmark = m_pSeekCursor->getBookmark();
                }
            }
        }

        // Sind alle Zeilen Selectiert
        // Zweite bedingung falls keine einguegeZeile existiert
        sal_Bool bAllSelected = GetTotalCount() == nSelectedRows || GetRowCount() == nSelectedRows;

        BeginCursorAction();

        // now delete the row
        Sequence <sal_Int32> aDeletedRows;
        SetUpdateMode( sal_False );
        try
        {
            aDeletedRows = xDeleteThem->deleteRows(aBookmarks);
        }
        catch(SQLException&)
        {
        }
        SetUpdateMode( sal_True );

        // how many rows are deleted?
        sal_Int32 nDeletedRows = 0;
        const sal_Int32* pSuccess = aDeletedRows.getConstArray();
        for (sal_Int32 i = 0; i < aDeletedRows.getLength(); i++)
        {
            if (pSuccess[i])
                ++nDeletedRows;
        }

        // sind Zeilen geloescht worden?
        if (nDeletedRows)
        {
            SetUpdateMode(sal_False);
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
                            Reference< XResultSetUpdate >  xUpdateCursor((Reference< XInterface >)*m_pDataCursor, UNO_QUERY);
                            xUpdateCursor->moveToInsertRow();
                        }
                    }
                    else
                    {
                        Reference< ::com::sun::star::beans::XPropertySet >  xSet((Reference< XInterface >)*m_pDataCursor, UNO_QUERY);

                        sal_Int32 nRecordCount(0);
                        xSet->getPropertyValue(FM_PROP_ROWCOUNT) >>= nRecordCount;
                        if ( m_pDataCursor->rowDeleted() )
                            --nRecordCount;

                        // there are no rows left and we have an insert row
                        if (!nRecordCount && GetEmptyRow().Is())
                        {
                            Reference< XResultSetUpdate >  xUpdateCursor((Reference< XInterface >)*m_pDataCursor, UNO_QUERY);
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

            // An den DatenCursor anpassen
            AdjustDataSource(sal_True);

            // es konnten nicht alle Zeilen geloescht werden
            // da nie nicht geloeschten wieder selektieren
            if (nDeletedRows < nSelectedRows)
            {
                // waren alle selektiert
                if (bAllSelected)
                {
                    SelectAll();
                    if (IsInsertionRow(GetRowCount() - 1))  // einfuegeZeile nicht
                        SelectRow(GetRowCount() - 1, sal_False);
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
            SetUpdateMode(sal_True);
        }
        else // Zeile konnte nicht geloescht werden
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
//------------------------------------------------------------------------------
void FmGridControl::positioned(const ::com::sun::star::lang::EventObject& /*rEvent*/)
{
    SAL_INFO("svx.fmcmop", "FmGridControl::positioned");
    // position on the data source (force it to be done in the main thread)
    implAdjustInSolarThread(sal_False);
}

//------------------------------------------------------------------------------
sal_Bool FmGridControl::commit()
{
    // Commit nur ausfuehren, wenn nicht bereits ein Update vom ::com::sun::star::form::component::GridControl ausgefuehrt
    // wird
    if (!IsUpdating())
    {
        if (Controller().Is() && Controller()->IsModified())
        {
            if (!SaveModified())
                return sal_False;
        }
    }
    return sal_True;
}

//------------------------------------------------------------------------------
void FmGridControl::inserted(const ::com::sun::star::lang::EventObject& /*rEvent*/)
{
    const DbGridRowRef& xRow = GetCurrentRow();
    if (!xRow.Is())
        return;

    // Zeile ist eingefuegt worden, dann den status und mode zuruecksetzen
    xRow->SetState(m_pDataCursor, sal_False);
    xRow->SetNew(sal_False);

}

//------------------------------------------------------------------------------
BrowserHeader* FmGridControl::imp_CreateHeaderBar(BrowseBox* pParent)
{
    DBG_ASSERT( pParent == this, "FmGridControl::imp_CreateHeaderBar: parent?" );
    return new FmGridHeader( pParent );
}

//------------------------------------------------------------------------------
void FmGridControl::markColumn(sal_uInt16 nId)
{
    if (GetHeaderBar() && m_nMarkedColumnId != nId)
    {
        // deselektieren
        if (m_nMarkedColumnId != BROWSER_INVALIDID)
        {
            HeaderBarItemBits aBits = GetHeaderBar()->GetItemBits(m_nMarkedColumnId) & ~HIB_FLAT;
            GetHeaderBar()->SetItemBits(m_nMarkedColumnId, aBits);
        }


        if (nId != BROWSER_INVALIDID)
        {
            HeaderBarItemBits aBits = GetHeaderBar()->GetItemBits(nId) | HIB_FLAT;
            GetHeaderBar()->SetItemBits(nId, aBits);
        }
        m_nMarkedColumnId = nId;
    }
}

//------------------------------------------------------------------------------
sal_Bool FmGridControl::isColumnMarked(sal_uInt16 nId) const
{
    return m_nMarkedColumnId == nId;
}

//------------------------------------------------------------------------------
long FmGridControl::QueryMinimumRowHeight()
{
    long nMinimalLogicHeight = 20; // 0.2 cm
    long nMinimalPixelHeight = LogicToPixel( Point( 0, nMinimalLogicHeight ), MAP_10TH_MM ).Y();
    return CalcZoom( nMinimalPixelHeight );
}

//------------------------------------------------------------------------------
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
            Any aProperty = makeAny( (sal_Int32)PixelToLogic( Point( 0, nUnzoomedPixelHeight ), MAP_10TH_MM ).Y() );
            xModel->setPropertyValue( FM_PROP_ROWHEIGHT, aProperty );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FmGridControl::RowHeightChanged: caught an exception!" );
        }
    }
}

//------------------------------------------------------------------------------
void FmGridControl::ColumnResized(sal_uInt16 nId)
{
    DbGridControl::ColumnResized(nId);

    // Wert ans model uebergeben
    DbGridColumn* pCol = DbGridControl::GetColumns().at( GetModelColumnPos(nId) );
    Reference< ::com::sun::star::beans::XPropertySet >  xColModel(pCol->getModel());
    if (xColModel.is())
    {
        Any aWidth;
        sal_Int32 nColumnWidth = GetColumnWidth(nId);
        nColumnWidth = CalcReverseZoom(nColumnWidth);
        // Umrechnen in 10THMM
        aWidth <<= (sal_Int32)PixelToLogic(Point(nColumnWidth,0),MAP_10TH_MM).X();
        xColModel->setPropertyValue(FM_PROP_WIDTH, aWidth);
    }
}

//------------------------------------------------------------------------------
void FmGridControl::CellModified()
{
    DbGridControl::CellModified();
    GetPeer()->CellModified();
}

//------------------------------------------------------------------------------
void FmGridControl::BeginCursorAction()
{
    DbGridControl::BeginCursorAction();
    m_pPeer->stopCursorListening();
}

//------------------------------------------------------------------------------
void FmGridControl::EndCursorAction()
{
    m_pPeer->startCursorListening();
    DbGridControl::EndCursorAction();
}

//------------------------------------------------------------------------------
void FmGridControl::ColumnMoved(sal_uInt16 nId)
{
    m_bInColumnMove = sal_True;

    DbGridControl::ColumnMoved(nId);
    Reference< ::com::sun::star::container::XIndexContainer >  xColumns(GetPeer()->getColumns());

    if (xColumns.is())
    {
        // suchen der Spalte und verschieben im Model
        // ColumnPos holen
        DbGridColumn* pCol = DbGridControl::GetColumns().at( GetModelColumnPos(nId) );
        Reference< ::com::sun::star::beans::XPropertySet >  xCol;

        // Einfuegen muß sich an den Column Positionen orientieren
        sal_Int32 i;
        Reference< XInterface > xCurrent;
        for (i = 0; !xCol.is() && i < xColumns->getCount(); i++)
        {
            ::cppu::extractInterface(xCurrent, xColumns->getByIndex(i));
            if (xCurrent == pCol->getModel())
            {
                xCol = pCol->getModel();
                break;
            }
        }

        DBG_ASSERT(i < xColumns->getCount(), "Falscher ::com::sun::star::sdbcx::Index");
        xColumns->removeByIndex(i);
        Any aElement;
        aElement <<= xCol;
        xColumns->insertByIndex(GetModelColumnPos(nId), aElement);
        pCol->setModel(xCol);
        // if the column which is shown here is selected ...
        if ( isColumnSelected(nId,pCol) )
            markColumn(nId); // ... -> mark it
    }

    m_bInColumnMove = sal_False;
}

//------------------------------------------------------------------------------
void FmGridControl::InitColumnsByModels(const Reference< ::com::sun::star::container::XIndexContainer >& xColumns)
{
    // Spalten wieder neu setzen
    // wenn es nur eine HandleColumn gibt, dann nicht
    if (GetModelColCount())
    {
        RemoveColumns();
        InsertHandleColumn();
    }

    if (!xColumns.is())
        return;

    SetUpdateMode(sal_False);

    // Einfuegen mu� sich an den Column Positionen orientieren
    sal_Int32 i;
    Any aWidth;
    for (i = 0; i < xColumns->getCount(); ++i)
    {
        Reference< ::com::sun::star::beans::XPropertySet > xCol;
        ::cppu::extractInterface(xCol, xColumns->getByIndex(i));

        OUString aName(
            comphelper::getString(xCol->getPropertyValue(FM_PROP_LABEL)));

        aWidth = xCol->getPropertyValue(FM_PROP_WIDTH);
        sal_Int32 nWidth = 0;
        if (aWidth >>= nWidth)
            nWidth = LogicToPixel(Point(nWidth,0),MAP_10TH_MM).X();

        AppendColumn(aName, (sal_uInt16)nWidth);
        DbGridColumn* pCol = DbGridControl::GetColumns().at( i );
        pCol->setModel(xCol);
    }

    // und jetzt noch die hidden columns rausnehmen
    // (wir haben das nicht gleich in der oberen Schleife gemacht, da wir dann Probleme mit den
    // IDs der Spalten bekommen haetten : AppendColumn vergibt die automatisch, die Spalte _nach_
    // einer versteckten braucht aber eine um eine erhoehte ID ....
    Any aHidden;
    for (i = 0; i < xColumns->getCount(); ++i)
    {
        Reference< ::com::sun::star::beans::XPropertySet > xCol;
        ::cppu::extractInterface(xCol, xColumns->getByIndex(i));
        aHidden = xCol->getPropertyValue(FM_PROP_HIDDEN);
        if (::comphelper::getBOOL(aHidden))
            HideColumn(GetColumnIdFromModelPos((sal_uInt16)i));
    }

    SetUpdateMode(sal_True);
}

//------------------------------------------------------------------------------
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

        sal_Bool bIllegalType = sal_False;
        switch ( nDataType )
        {
            case DataType::BLOB:
            case DataType::LONGVARBINARY:
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::OTHER:
                bIllegalType = sal_True;
                break;
        }

        if ( bIllegalType )
        {
            _pColumn->SetObject( (sal_Int16)nFieldPos );
            return;
        }
    }

    // the control type is determined by the ColumnServiceName
    static OUString s_sPropColumnServiceName( "ColumnServiceName" );
    if ( !::comphelper::hasProperty( s_sPropColumnServiceName, _rxColumnModel ) )
        return;

    _pColumn->setModel( _rxColumnModel );

    OUString sColumnServiceName;
    _rxColumnModel->getPropertyValue( s_sPropColumnServiceName ) >>= sColumnServiceName;

    sal_Int32 nTypeId = getColumnTypeByModelName( sColumnServiceName );
    _pColumn->CreateControl( nFieldPos, xField, nTypeId );
}

//------------------------------------------------------------------------------
void FmGridControl::InitColumnsByFields(const Reference< ::com::sun::star::container::XIndexAccess >& _rxFields)
{
    if ( !_rxFields.is() )
        return;

    // Spalten initialisieren
    Reference< XIndexContainer > xColumns( GetPeer()->getColumns() );
    Reference< XNameAccess > xFieldsAsNames( _rxFields, UNO_QUERY );

    // Einfuegen muss sich an den Column Positionen orientieren
    for (sal_Int32 i = 0; i < xColumns->getCount(); i++)
    {
        DbGridColumn* pCol = GetColumns().at( i );
        OSL_ENSURE(pCol,"No grid column!");
        if ( pCol )
        {
            Reference< XPropertySet > xColumnModel;
            ::cppu::extractInterface( xColumnModel, xColumns->getByIndex( i ) );

            InitColumnByField( pCol, xColumnModel, xFieldsAsNames, _rxFields );
        }
    }
}

//------------------------------------------------------------------------------
void FmGridControl::HideColumn(sal_uInt16 nId)
{
    DbGridControl::HideColumn(nId);

    sal_uInt16 nPos = GetModelColumnPos(nId);
    if (nPos == (sal_uInt16)-1)
        return;

    DbGridColumn* pColumn = GetColumns().at( nPos );
    if (pColumn->IsHidden())
        GetPeer()->columnHidden(pColumn);

    if (nId == m_nMarkedColumnId)
        m_nMarkedColumnId = (sal_uInt16)-1;
}
// -----------------------------------------------------------------------------
sal_Bool FmGridControl::isColumnSelected(sal_uInt16 /*nId*/,DbGridColumn* _pColumn)
{
    OSL_ENSURE(_pColumn,"Column can not be null!");
    sal_Bool bSelected = sal_False;
    // if the column which is shown here is selected ...
    Reference< ::com::sun::star::view::XSelectionSupplier >  xSelSupplier(GetPeer()->getColumns(), UNO_QUERY);
    if ( xSelSupplier.is() )
    {
        Reference< ::com::sun::star::beans::XPropertySet >  xColumn;
        xSelSupplier->getSelection() >>= xColumn;
        bSelected = (xColumn.get() == _pColumn->getModel().get());
    }
    return bSelected;
}

//------------------------------------------------------------------------------
void FmGridControl::ShowColumn(sal_uInt16 nId)
{
    DbGridControl::ShowColumn(nId);

    sal_uInt16 nPos = GetModelColumnPos(nId);
    if (nPos == (sal_uInt16)-1)
        return;

    DbGridColumn* pColumn = GetColumns().at( nPos );
    if (!pColumn->IsHidden())
        GetPeer()->columnVisible(pColumn);

    // if the column which is shown here is selected ...
    if ( isColumnSelected(nId,pColumn) )
        markColumn(nId); // ... -> mark it
}

//------------------------------------------------------------------------------
sal_Bool FmGridControl::selectBookmarks(const Sequence< Any >& _rBookmarks)
{
    SolarMutexGuard aGuard;
        // need to lock the SolarMutex so that no paint call disturbs us ...

    if ( !m_pSeekCursor )
    {
        OSL_FAIL( "FmGridControl::selectBookmarks: no seek cursor!" );
        return sal_False;
    }

    const Any* pBookmark = _rBookmarks.getConstArray();
    const Any* pBookmarkEnd = pBookmark + _rBookmarks.getLength();

    SetNoSelection();

    sal_Bool bAllSuccessfull = sal_True;
    try
    {
        for (; pBookmark != pBookmarkEnd; ++pBookmark)
        {
            // move the seek cursor to the row given
            if (m_pSeekCursor->moveToBookmark(*pBookmark))
                SelectRow( m_pSeekCursor->getRow() - 1);
            else
                bAllSuccessfull = sal_False;
        }
    }
    catch(Exception&)
    {
        OSL_FAIL("FmGridControl::selectBookmarks: could not move to one of the bookmarks!");
        return sal_False;
    }

    return bAllSuccessfull;
}

//------------------------------------------------------------------------------
Sequence< Any> FmGridControl::getSelectionBookmarks()
{
    // lock our update so no paint-triggered seeks interfere ...
    SetUpdateMode(sal_False);

    sal_Int32 nSelectedRows = GetSelectRowCount(), i = 0;
    Sequence< Any> aBookmarks(nSelectedRows);
    if ( nSelectedRows )
    {
        Any* pBookmarks = (Any*)aBookmarks.getArray();

        // (I'm not sure if the problem isn't deeper : The szenario : a large table displayed by a grid with a
        // thread-safe cursor (dBase). On loading the sdb-cursor started a counting thread. While this counting progress
        // was running, I tried do delete 3 records from within the grid. Deletion caused a SeekCursor, which did a
        // m_pSeekCursor->moveRelative and a m_pSeekCursor->getPosition.
        // Unfortunally the first call caused a propertyChanged(RECORDCOUNT) which resulted in a repaint of the
        // navigation bar and the grid. The latter itself will result in SeekRow calls. So after (successfully) returning
        // from the moveRelative the getPosition returns an invalid value. And so the SeekCursor fails.
        // In the consequence ALL parts of code where two calls to the seek cursor are done, while the second call _relys_ on
        // the first one, should be secured against recursion, with a broad-minded interpretion of "recursion" : if any of these
        // code parts is executed, no other should be accessible. But this sounds very difficult to achieve ....
        // )

        // The next problem caused by the same behaviuor (SeekCursor causes a propertyChanged) : when adjusting rows we implicitly
        // change our selection. So a "FirstSelected(); SeekCursor(); NextSelected();" may produce unpredictable results.
        // That's why we _first_ collect the indicies of the selected rows and _then_ their bookmarks.
        long nIdx = FirstSelectedRow();
        while (nIdx >= 0)
        {
            // (we misuse the bookmarks array for this ...)
            pBookmarks[i++] <<= (sal_Int32)nIdx;
            nIdx = NextSelectedRow();
        }
        DBG_ASSERT(i == nSelectedRows, "FmGridControl::DeleteSelectedRows : could not collect the row indicies !");

        for (i=0; i<nSelectedRows; ++i)
        {
            nIdx = ::comphelper::getINT32(pBookmarks[i]);
            if (IsInsertionRow(nIdx))
            {
                // leerzeile nicht loeschen
                aBookmarks.realloc(--nSelectedRows);
                SelectRow(nIdx,sal_False);          // selection aufheben fuer leerzeile
                break;
            }

            // Zunaechst den DatenCursor auf den selektierten Satz pos.
            if (SeekCursor(nIdx))
            {
                GetSeekRow()->SetState(m_pSeekCursor, sal_True);

                pBookmarks[i] = m_pSeekCursor->getBookmark();
            }
    #ifdef DBG_UTIL
            else
                OSL_FAIL("FmGridControl::DeleteSelectedRows : a bookmark could not be determined !");
    #endif
        }
    }
    SetUpdateMode(sal_True);

    // if one of the SeekCursor-calls failed ....
    aBookmarks.realloc(i);

    // (the alternative : while collecting the bookmarks lock our propertyChanged, this should resolve both our problems.
    // but this would be incompatible as we need a locking flag, then ...)

    return aBookmarks;
}
// -----------------------------------------------------------------------------
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
                        SAL_WARN("svx.form",
                                "exception caught: " << e.Message);
                    }
                }
            }
        }
        return sRetText;
    }
}
// Object data and state ------------------------------------------------------
OUString FmGridControl::GetAccessibleObjectName( ::svt::AccessibleBrowseBoxObjType _eObjType,sal_Int32 _nPosition ) const
{
    OUString sRetText;
    switch( _eObjType )
    {
        case ::svt::BBTYPE_BROWSEBOX:
            if ( GetPeer() )
            {
                Reference<XPropertySet> xProp(GetPeer()->getColumns(),UNO_QUERY);
                if ( xProp.is() )
                    xProp->getPropertyValue(FM_PROP_NAME) >>= sRetText;
            }
            break;
        case ::svt::BBTYPE_COLUMNHEADERCELL:
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
// -----------------------------------------------------------------------------

OUString FmGridControl::GetAccessibleObjectDescription( ::svt::AccessibleBrowseBoxObjType _eObjType,sal_Int32 _nPosition ) const
{
    OUString sRetText;
    switch( _eObjType )
    {
        case ::svt::BBTYPE_BROWSEBOX:
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
        case ::svt::BBTYPE_COLUMNHEADERCELL:
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
// -----------------------------------------------------------------------------
void FmGridControl::Select()
{
    DbGridControl::Select();
    // ... betrifft das unsere Spalten ?
    const MultiSelection* pColumnSelection = GetColumnSelection();

    sal_uInt16 nSelectedColumn =
        pColumnSelection && pColumnSelection->GetSelectCount()
            ? sal::static_int_cast< sal_uInt16 >(
                ((MultiSelection*)pColumnSelection)->FirstSelected())
            : SAL_MAX_UINT16;
    // die HandleColumn wird nicht selektiert
    switch (nSelectedColumn)
    {
        case SAL_MAX_UINT16: break; // no selection
        case  0 : nSelectedColumn = SAL_MAX_UINT16; break;
                    // handle col can't be seledted
        default :
            // get the model col pos instead of the view col pos
            nSelectedColumn = GetModelColumnPos(GetColumnIdFromViewPos(nSelectedColumn - 1));
            break;
    }

    if (nSelectedColumn != m_nCurrentSelectedColumn)
    {
        // VOR dem Aufruf des select am SelectionSupplier !
        m_nCurrentSelectedColumn = nSelectedColumn;

        if (!m_bSelecting)
        {
            m_bSelecting = sal_True;

            try
            {
                Reference< XIndexAccess >  xColumns(GetPeer()->getColumns(), UNO_QUERY);
                Reference< XSelectionSupplier >  xSelSupplier(xColumns, UNO_QUERY);
                if (xSelSupplier.is())
                {
                    if (nSelectedColumn != SAL_MAX_UINT16)
                    {
                        Reference< XPropertySet >  xColumn;
                        ::cppu::extractInterface(xColumn,xColumns->getByIndex(nSelectedColumn));
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


            m_bSelecting = sal_False;
        }
    }
}
// -----------------------------------------------------------------------------
sal_Int32 FmGridControl::GetSelectedColumn() const
{
    return m_nCurrentSelectedColumn;
}
// -----------------------------------------------------------------------------
void FmGridControl::KeyInput( const KeyEvent& rKEvt )
{
    sal_Bool bDone = sal_False;
    const KeyCode& rKeyCode = rKEvt.GetKeyCode();
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
                bDone = sal_True;
                break;
            case KEY_DELETE:
                if ( GetSelectColumnCount() && GetPeer() && m_nCurrentSelectedColumn >= 0 )
                {
                    Reference< ::com::sun::star::container::XIndexContainer >  xCols(GetPeer()->getColumns());
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
                bDone = sal_True;
                break;
        }
    }
    if ( !bDone )
        DbGridControl::KeyInput( rKEvt );
}
// -----------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
