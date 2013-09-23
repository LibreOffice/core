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

#include "SelectionBrowseBox.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include "QueryDesignView.hxx"
#include "querycontroller.hxx"
#include "QueryTableView.hxx"
#include "browserids.hxx"
#include <comphelper/extract.hxx>
#include <comphelper/stl_types.hxx>
#include <comphelper/string.hxx>
#include <comphelper/types.hxx>
#include "TableFieldInfo.hxx"
#include "dbu_qry.hrc"
#include "dbaccess_helpid.hrc"
#include <com/sun/star/container/XNameAccess.hpp>
#include "dbustrings.hrc"
#include "QTableWindow.hxx"
#include <vcl/msgbox.hxx>
#include "QueryDesignFieldUndoAct.hxx"
#include "sqlmessage.hxx"
#include "UITools.hxx"
#include <osl/diagnose.h>
#include "svtools/treelistentry.hxx"

using namespace ::svt;
using namespace ::dbaui;
using namespace ::connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::accessibility;

#define g_strOne OUString("1")
#define g_strZero OUString("0")

#define DEFAULT_QUERY_COLS  20
#define DEFAULT_SIZE        GetTextWidth(g_strZero) * 30
#define CHECKBOX_SIZE       10
#define HANDLE_ID            0
#define HANDLE_COLUMN_WITDH 70
#define SORT_COLUMN_NONE    0xFFFFFFFF

namespace
{
    sal_Bool isFieldNameAsterix(const OUString& _sFieldName )
    {
        sal_Bool bAsterix = !(!_sFieldName.isEmpty() && _sFieldName.toChar() != '*');
        if ( !bAsterix )
        {
            String sName = _sFieldName;
            xub_StrLen nTokenCount = comphelper::string::getTokenCount(sName, '.');
            if (    (nTokenCount == 2 && sName.GetToken(1,'.').GetChar(0) == '*' )
                ||  (nTokenCount == 3 && sName.GetToken(2,'.').GetChar(0) == '*' ) )
            {
                bAsterix = sal_True;
            }
        }
        return bAsterix;
    }
    sal_Bool lcl_SupportsCoreSQLGrammar(const Reference< XConnection>& _xConnection)
    {
        sal_Bool bSupportsCoreGrammar = sal_False;
        if ( _xConnection.is() )
        {
            try
            {
                Reference< XDatabaseMetaData >  xMetaData = _xConnection->getMetaData();
                bSupportsCoreGrammar = xMetaData.is() && xMetaData->supportsCoreSQLGrammar();
            }
            catch(Exception&)
            {
            }
        }
        return bSupportsCoreGrammar;
    }
}

DBG_NAME(OSelectionBrowseBox)
OSelectionBrowseBox::OSelectionBrowseBox( Window* pParent )
                   :EditBrowseBox( pParent,EBBF_NOROWPICTURE, WB_3DLOOK, BROWSER_COLUMNSELECTION | BROWSER_KEEPSELECTION |  BROWSER_HIDESELECT |
                                  BROWSER_HIDECURSOR | BROWSER_HLINESFULL | BROWSER_VLINESFULL )
                   ,m_aFunctionStrings(ModuleRes(STR_QUERY_FUNCTIONS))
                   ,m_nVisibleCount(0)
                   ,m_nLastSortColumn(SORT_COLUMN_NONE)
                   ,m_bOrderByUnRelated(sal_True)
                   ,m_bGroupByUnRelated(sal_True)
                   ,m_bStopTimer(sal_False)
                   ,m_bWasEditing(sal_False)
                   ,m_bDisableErrorBox(sal_False)
                   ,m_bInUndoMode(sal_False)
{
    DBG_CTOR(OSelectionBrowseBox,NULL);
    SetHelpId(HID_CTL_QRYDGNCRIT);

    m_nMode =       BROWSER_COLUMNSELECTION | BROWSER_HIDESELECT
                |   BROWSER_KEEPSELECTION   | BROWSER_HIDECURSOR
                |   BROWSER_HLINESFULL      | BROWSER_VLINESFULL
                |   BROWSER_HEADERBAR_NEW   ;

    m_pTextCell     = new Edit(&GetDataWindow(), 0);
    m_pVisibleCell  = new CheckBoxControl(&GetDataWindow());
    m_pTableCell    = new ListBoxControl(&GetDataWindow());     m_pTableCell->SetDropDownLineCount( 20 );
    m_pFieldCell    = new ComboBoxControl(&GetDataWindow());    m_pFieldCell->SetDropDownLineCount( 20 );
    m_pOrderCell    = new ListBoxControl(&GetDataWindow());
    m_pFunctionCell = new ListBoxControl(&GetDataWindow());     m_pFunctionCell->SetDropDownLineCount( 20 );

    m_pVisibleCell->SetHelpId(HID_QRYDGN_ROW_VISIBLE);
    m_pTableCell->SetHelpId(HID_QRYDGN_ROW_TABLE);
    m_pFieldCell->SetHelpId(HID_QRYDGN_ROW_FIELD);
    m_pOrderCell->SetHelpId(HID_QRYDGN_ROW_ORDER);
    m_pFunctionCell->SetHelpId(HID_QRYDGN_ROW_FUNCTION);

    // switch off triState of ::com::sun::star::form::CheckBox
    m_pVisibleCell->GetBox().EnableTriState( sal_False );

    Font aTitleFont = OutputDevice::GetDefaultFont( DEFAULTFONT_SANS_UNICODE,Window::GetSettings().GetLanguageTag().getLanguageType(),DEFAULTFONT_FLAGS_ONLYONE);
    aTitleFont.SetSize(Size(0, 6));
    SetTitleFont(aTitleFont);

    String aTxt(ModuleRes(STR_QUERY_SORTTEXT));
    xub_StrLen nCount = comphelper::string::getTokenCount(aTxt, ';');
    xub_StrLen nIdx = 0;
    for (; nIdx < nCount; nIdx++)
        m_pOrderCell->InsertEntry(aTxt.GetToken(nIdx));

    for(long i=0;i < BROW_ROW_CNT;i++)
        m_bVisibleRow.push_back(sal_True);

    m_bVisibleRow[BROW_FUNCTION_ROW] = sal_False;   // first hide

    m_timerInvalidate.SetTimeout(200);
    m_timerInvalidate.SetTimeoutHdl(LINK(this, OSelectionBrowseBox, OnInvalidateTimer));
    m_timerInvalidate.Start();
}

OSelectionBrowseBox::~OSelectionBrowseBox()
{
    DBG_DTOR(OSelectionBrowseBox,NULL);

    delete m_pTextCell;
    delete m_pVisibleCell;
    delete m_pFieldCell;
    delete m_pTableCell;
    delete m_pOrderCell;
    delete m_pFunctionCell;
}

void OSelectionBrowseBox::initialize()
{
    Reference< XConnection> xConnection = static_cast<OQueryController&>(getDesignView()->getController()).getConnection();
    if(xConnection.is())
    {
        const IParseContext& rContext = static_cast<OQueryController&>(getDesignView()->getController()).getParser().getContext();
        IParseContext::InternationalKeyCode eFunctions[] = { IParseContext::KEY_AVG,IParseContext::KEY_COUNT,IParseContext::KEY_MAX
            ,IParseContext::KEY_MIN,IParseContext::KEY_SUM
            ,IParseContext::KEY_EVERY
            ,IParseContext::KEY_ANY
            ,IParseContext::KEY_SOME
            ,IParseContext::KEY_STDDEV_POP
            ,IParseContext::KEY_STDDEV_SAMP
            ,IParseContext::KEY_VAR_SAMP
            ,IParseContext::KEY_VAR_POP
            ,IParseContext::KEY_COLLECT
            ,IParseContext::KEY_FUSION
            ,IParseContext::KEY_INTERSECTION
        };

        String sGroup = m_aFunctionStrings.GetToken(comphelper::string::getTokenCount(m_aFunctionStrings, ';') - 1);
        m_aFunctionStrings = m_aFunctionStrings.GetToken(0);

        for (size_t i = 0; i < sizeof (eFunctions) / sizeof (eFunctions[0]); ++i)
        {
            m_aFunctionStrings += String(RTL_CONSTASCII_USTRINGPARAM(";"));
            m_aFunctionStrings += OStringToOUString(rContext.getIntlKeywordAscii(eFunctions[i]),
                RTL_TEXTENCODING_UTF8);
        }
        m_aFunctionStrings += String(RTL_CONSTASCII_USTRINGPARAM(";"));
        m_aFunctionStrings += sGroup;

        // Aggregate functions in general available only with Core SQL
        // We slip in a few optionals one, too.
        if ( lcl_SupportsCoreSQLGrammar(xConnection) )
        {
            xub_StrLen nCount = comphelper::string::getTokenCount(m_aFunctionStrings, ';');
            for (xub_StrLen nIdx = 0; nIdx < nCount; nIdx++)
                m_pFunctionCell->InsertEntry(m_aFunctionStrings.GetToken(nIdx));
        }
        else // else only COUNT(*) and COUNT("table".*)
        {
            m_pFunctionCell->InsertEntry(m_aFunctionStrings.GetToken(0));
            m_pFunctionCell->InsertEntry(m_aFunctionStrings.GetToken(2)); // 2 -> COUNT
        }
        try
        {
            Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
            if ( xMetaData.is() )
            {
                m_bOrderByUnRelated = xMetaData->supportsOrderByUnrelated();
                m_bGroupByUnRelated = xMetaData->supportsGroupByUnrelated();
            }
        }
        catch(Exception&)
        {
        }
    }

    Init();
}

OQueryDesignView* OSelectionBrowseBox::getDesignView()
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OSL_ENSURE(static_cast<const OQueryDesignView*>(GetParent()),"Parent isn't an OQueryDesignView!");
    return static_cast<OQueryDesignView*>(GetParent());
}

OQueryDesignView* OSelectionBrowseBox::getDesignView() const
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OSL_ENSURE(static_cast<const OQueryDesignView*>(GetParent()),"Parent isn't an OQueryDesignView!");
    return static_cast<OQueryDesignView*>(GetParent());
}

namespace
{
    class OSelectionBrwBoxHeader : public ::svt::EditBrowserHeader
    {
        OSelectionBrowseBox* m_pBrowseBox;
    protected:
        virtual void Select();
    public:
        OSelectionBrwBoxHeader(OSelectionBrowseBox* pParent);
    };
    OSelectionBrwBoxHeader::OSelectionBrwBoxHeader(OSelectionBrowseBox* pParent)
        : ::svt::EditBrowserHeader(pParent,WB_BUTTONSTYLE|WB_DRAG)
        ,m_pBrowseBox(pParent)
    {
    }

    void OSelectionBrwBoxHeader::Select()
    {
        EditBrowserHeader::Select();
        m_pBrowseBox->GrabFocus();

        BrowserMode nMode = m_pBrowseBox->GetMode();
        if ( 0 == m_pBrowseBox->GetSelectColumnCount() )
        {
            m_pBrowseBox->DeactivateCell();
            // we are in the right mode if a row hase been selected row
            if ( BROWSER_HIDESELECT == ( nMode & BROWSER_HIDESELECT ) )
            {
                nMode &= ~BROWSER_HIDESELECT;
                nMode |= BROWSER_MULTISELECTION;
                m_pBrowseBox->SetMode( nMode );
            }
        }
        m_pBrowseBox->SelectColumnId( GetCurItemId() );
        m_pBrowseBox->DeactivateCell();
    }
}

BrowserHeader* OSelectionBrowseBox::imp_CreateHeaderBar(BrowseBox* /*pParent*/)
{
    return new OSelectionBrwBoxHeader(this);
}

void OSelectionBrowseBox::ColumnMoved( sal_uInt16 nColId,sal_Bool _bCreateUndo )
{
    EditBrowseBox::ColumnMoved( nColId );
    // swap the two columns
    sal_uInt16 nNewPos = GetColumnPos( nColId );
    OTableFields& rFields = getFields();
    if ( rFields.size() > sal_uInt16(nNewPos-1) )
    {
        sal_uInt16 nOldPos = 0;
        OTableFields::iterator aEnd = rFields.end();
        OTableFields::iterator aIter = rFields.begin();
        for (; aIter != aEnd && ( (*aIter)->GetColumnId() != nColId ); ++aIter,++nOldPos)
            ;

        OSL_ENSURE( (nNewPos-1) != nOldPos && nOldPos < rFields.size(),"Old and new position are equal!");
        if ( aIter != aEnd )
        {
            OTableFieldDescRef pOldEntry = rFields[nOldPos];
            rFields.erase(rFields.begin() + nOldPos);
            rFields.insert(rFields.begin() + nNewPos - 1,pOldEntry);

            // create the undo action
            if ( !m_bInUndoMode && _bCreateUndo )
            {
                OTabFieldMovedUndoAct* pUndoAct = new OTabFieldMovedUndoAct(this);
                pUndoAct->SetColumnPosition( nOldPos + 1);
                pUndoAct->SetTabFieldDescr(pOldEntry);

                getDesignView()->getController().addUndoActionAndInvalidate(pUndoAct);
            }
        }
    }
    else
        OSL_FAIL("Invalid column id!");
}

void OSelectionBrowseBox::Init()
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);

    EditBrowseBox::Init();

    // set the header bar
    BrowserHeader* pNewHeaderBar = CreateHeaderBar(this);
    pNewHeaderBar->SetMouseTransparent(sal_False);

    SetHeaderBar(pNewHeaderBar);
    SetMode(m_nMode);

    Font    aFont( GetDataWindow().GetFont() );
    aFont.SetWeight( WEIGHT_NORMAL );
    GetDataWindow().SetFont( aFont );

    Size aHeight;
    const Control* pControls[] = { m_pTextCell,m_pVisibleCell,m_pTableCell,m_pFieldCell };

    for (sal_Size i = 0; i < sizeof (pControls) / sizeof (pControls[0]); ++i)
    {
        const Size aTemp(pControls[i]->GetOptimalSize());
        if ( aTemp.Height() > aHeight.Height() )
            aHeight.Height() = aTemp.Height();
    }
    SetDataRowHeight(aHeight.Height());
    SetTitleLines(1);
    // get number of visible rows
    for(long i=0;i<BROW_ROW_CNT;i++)
    {
        if(m_bVisibleRow[i])
            m_nVisibleCount++;
    }
    RowInserted(0, m_nVisibleCount, sal_False);
    try
    {
        Reference< XConnection> xConnection = static_cast<OQueryController&>(getDesignView()->getController()).getConnection();
        if(xConnection.is())
        {
            Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
            m_nMaxColumns = xMetaData.is() ? xMetaData->getMaxColumnsInSelect() : 0;

        }
        else
            m_nMaxColumns = 0;
    }
    catch(const SQLException&)
    {
        OSL_FAIL("Catched Exception when asking for database metadata options!");
        m_nMaxColumns = 0;
    }
}

void OSelectionBrowseBox::PreFill()
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    SetUpdateMode(sal_False);

    if (GetCurRow() != 0)
        GoToRow(0);

    static_cast< OQueryController& >( getDesignView()->getController() ).clearFields();

    DeactivateCell();

    RemoveColumns();
    InsertHandleColumn( HANDLE_COLUMN_WITDH );
    SetUpdateMode(sal_True);
}

void OSelectionBrowseBox::ClearAll()
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    SetUpdateMode(sal_False);

    OTableFields::reverse_iterator aIter = getFields().rbegin();
    for ( ;aIter != getFields().rend(); ++aIter )
    {
        if ( !(*aIter)->IsEmpty() )
        {
            RemoveField( (*aIter)->GetColumnId() );
            aIter = getFields().rbegin();
        }
    }
    m_nLastSortColumn = SORT_COLUMN_NONE;
    SetUpdateMode(sal_True);
}

void OSelectionBrowseBox::SetReadOnly(sal_Bool bRO)
{
    if (bRO)
    {
        DeactivateCell();
        m_nMode &= ~BROWSER_HIDECURSOR;
        SetMode(m_nMode);
    }
    else
    {
        m_nMode |= BROWSER_HIDECURSOR;
        SetMode(m_nMode);
        ActivateCell();
    }
}

CellController* OSelectionBrowseBox::GetController(long nRow, sal_uInt16 nColId)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    if ( nColId > getFields().size() )
        return NULL;
    OTableFieldDescRef pEntry = getFields()[nColId-1];
    OSL_ENSURE(pEntry.is(), "OSelectionBrowseBox::GetController : keine FieldDescription !");

    if (!pEntry.is())
        return NULL;

    if (static_cast<OQueryController&>(getDesignView()->getController()).isReadOnly())
        return NULL;

    long nCellIndex = GetRealRow(nRow);
    switch (nCellIndex)
    {
        case BROW_FIELD_ROW:
            return new ComboBoxCellController(m_pFieldCell);
        case BROW_TABLE_ROW:
            return new ListBoxCellController(m_pTableCell);
        case BROW_VIS_ROW:
            return new CheckBoxCellController(m_pVisibleCell);
        case BROW_ORDER_ROW:
            return new ListBoxCellController(m_pOrderCell);
        case BROW_FUNCTION_ROW:
            return new ListBoxCellController(m_pFunctionCell);
        default:
            return new EditCellController(m_pTextCell);
    }
}

void OSelectionBrowseBox::InitController(CellControllerRef& /*rController*/, long nRow, sal_uInt16 nColId)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OSL_ENSURE(nColId != BROWSER_INVALIDID,"An Invalid Id was set!");
    if ( nColId == BROWSER_INVALIDID )
        return;
    sal_uInt16 nPos = GetColumnPos(nColId);
    if ( nPos == 0 || nPos == BROWSER_INVALIDID || nPos > getFields().size() )
        return;
    OTableFieldDescRef pEntry = getFields()[nPos-1];
    OSL_ENSURE(pEntry.is(), "OSelectionBrowseBox::InitController : keine FieldDescription !");
    long nCellIndex = GetRealRow(nRow);

    switch (nCellIndex)
    {
        case BROW_FIELD_ROW:
        {
            m_pFieldCell->Clear();
            m_pFieldCell->SetText(String());

            OUString aField(pEntry->GetField());
            OUString aTable(pEntry->GetAlias());

            getDesignView()->fillValidFields(aTable, m_pFieldCell);

            // replace with alias.*
            if (aField.trim() == "*")
            {
                aField = aTable + ".*";
            }
            m_pFieldCell->SetText(aField);
        }   break;
        case BROW_TABLE_ROW:
        {
            m_pTableCell->Clear();
            enableControl(pEntry,m_pTableCell);
            if ( !pEntry->isCondition() )
            {
                OJoinTableView::OTableWindowMap* pTabWinList = getDesignView()->getTableView()->GetTabWinMap();
                if (pTabWinList)
                {
                    OJoinTableView::OTableWindowMap::iterator aIter = pTabWinList->begin();
                    OJoinTableView::OTableWindowMap::iterator aEnd = pTabWinList->end();

                    for(;aIter != aEnd;++aIter)
                        m_pTableCell->InsertEntry(static_cast<OQueryTableWindow*>(aIter->second)->GetAliasName());

                    m_pTableCell->InsertEntry(String(ModuleRes(STR_QUERY_NOTABLE)), 0);
                    if (!pEntry->GetAlias().isEmpty())
                        m_pTableCell->SelectEntry(pEntry->GetAlias());
                    else
                        m_pTableCell->SelectEntry(String(ModuleRes(STR_QUERY_NOTABLE)));
                }
            }
        }   break;
        case BROW_VIS_ROW:
        {
            m_pVisibleCell->GetBox().Check(pEntry->IsVisible());
            m_pVisibleCell->GetBox().SaveValue();

            enableControl(pEntry,m_pTextCell);

            if(!pEntry->IsVisible() && pEntry->GetOrderDir() != ORDER_NONE && !m_bOrderByUnRelated)
            {
               // a column has to visible in order to show up in ORDER BY
                pEntry->SetVisible(sal_True);
                m_pVisibleCell->GetBox().Check(pEntry->IsVisible());
                m_pVisibleCell->GetBox().SaveValue();
                m_pVisibleCell->GetBox().Disable();
                m_pVisibleCell->GetBox().EnableInput(sal_False);
                String aMessage(ModuleRes(STR_QRY_ORDERBY_UNRELATED));
                OQueryDesignView* paDView = getDesignView();
                InfoBox(paDView, aMessage).Execute();
            }
        }   break;
        case BROW_ORDER_ROW:
            m_pOrderCell->SelectEntryPos(
                sal::static_int_cast< sal_uInt16 >(pEntry->GetOrderDir()));
            enableControl(pEntry,m_pOrderCell);
            break;
        case BROW_COLUMNALIAS_ROW:
            setTextCellContext(pEntry,pEntry->GetFieldAlias(),HID_QRYDGN_ROW_ALIAS);
            break;
        case BROW_FUNCTION_ROW:
            setFunctionCell(pEntry);
            break;
        default:
        {
            sal_uInt16  nIdx = sal_uInt16(nCellIndex - BROW_CRIT1_ROW);
            setTextCellContext(pEntry,pEntry->GetCriteria( nIdx ),HID_QRYDGN_ROW_CRIT);
        }
    }
    Controller()->ClearModified();
}

void OSelectionBrowseBox::notifyTableFieldChanged(const String& _sOldAlias,const String& _sAlias,sal_Bool& _bListAction,sal_uInt16 _nColumnId)
{
    appendUndoAction(_sOldAlias,_sAlias,BROW_TABLE_ROW,_bListAction);
    if ( m_bVisibleRow[BROW_TABLE_ROW] )
        RowModified(GetBrowseRow(BROW_TABLE_ROW), _nColumnId);
}

void OSelectionBrowseBox::notifyFunctionFieldChanged(const String& _sOldFunctionName,const String& _sFunctionName,sal_Bool& _bListAction,sal_uInt16 _nColumnId)
{
    appendUndoAction(_sOldFunctionName,_sFunctionName,BROW_FUNCTION_ROW,_bListAction);
    if ( !m_bVisibleRow[BROW_FUNCTION_ROW] )
        SetRowVisible(BROW_FUNCTION_ROW, sal_True);
    RowModified(GetBrowseRow(BROW_FUNCTION_ROW), _nColumnId);
}

void OSelectionBrowseBox::clearEntryFunctionField(const String& _sFieldName,OTableFieldDescRef& _pEntry,sal_Bool& _bListAction,sal_uInt16 _nColumnId)
{
    if ( isFieldNameAsterix( _sFieldName ) && (!_pEntry->isNoneFunction() || _pEntry->IsGroupBy()) )
    {
        OUString sFunctionName;
        GetFunctionName(SQL_TOKEN_COUNT,sFunctionName);
        String sOldLocalizedFunctionName = _pEntry->GetFunction();
        if ( !sOldLocalizedFunctionName.Equals(sFunctionName) || _pEntry->IsGroupBy() )
        {
            // append undo action for the function field
            _pEntry->SetFunctionType(FKT_NONE);
            _pEntry->SetFunction(OUString());
            _pEntry->SetGroupBy(sal_False);
            notifyFunctionFieldChanged(sOldLocalizedFunctionName,_pEntry->GetFunction(),_bListAction,_nColumnId);
        }
    }
}

sal_Bool OSelectionBrowseBox::fillColumnRef(const OSQLParseNode* _pColumnRef, const Reference< XConnection >& _rxConnection, OTableFieldDescRef& _pEntry, sal_Bool& _bListAction )
{
    OSL_ENSURE(_pColumnRef,"No valid parsenode!");
    OUString sColumnName,sTableRange;
    OSQLParseTreeIterator::getColumnRange(_pColumnRef,_rxConnection,sColumnName,sTableRange);
    return fillColumnRef(sColumnName,sTableRange,_rxConnection->getMetaData(),_pEntry,_bListAction);
}

sal_Bool OSelectionBrowseBox::fillColumnRef(const OUString& _sColumnName,const OUString& _sTableRange,const Reference<XDatabaseMetaData>& _xMetaData,OTableFieldDescRef& _pEntry,sal_Bool& _bListAction)
{
    sal_Bool bError = sal_False;
    ::comphelper::UStringMixEqual bCase(_xMetaData->supportsMixedCaseQuotedIdentifiers());
    // check if the table name is the same
    if ( !_sTableRange.isEmpty() && (bCase(_pEntry->GetTable(),_sTableRange) || bCase(_pEntry->GetAlias(),_sTableRange)) )
    { // a table was already inserted and the tables contains that column name

        if ( !_pEntry->GetTabWindow() )
        { // fill tab window
            OUString sOldAlias = _pEntry->GetAlias();
            if ( !fillEntryTable(_pEntry,_pEntry->GetTable()) )
                fillEntryTable(_pEntry,_pEntry->GetAlias()); // only when the first failed
            if ( !bCase(sOldAlias,_pEntry->GetAlias()) )
                notifyTableFieldChanged(sOldAlias,_pEntry->GetAlias(),_bListAction,GetCurColumnId());
        }
    }
    // check if the table window
    OQueryTableWindow* pEntryTab = static_cast<OQueryTableWindow*>(_pEntry->GetTabWindow());
    if ( !pEntryTab ) // no table found with this name so we have to travel through all tables
    {
        OJoinTableView::OTableWindowMap* pTabWinList = getDesignView()->getTableView()->GetTabWinMap();
        if ( pTabWinList )
        {
            sal_uInt16 nTabCount = 0;
            if ( !static_cast<OQueryTableView*>(getDesignView()->getTableView())->FindTableFromField(_sColumnName,_pEntry,nTabCount) ) // error occurred: column not in table window
            {
                String sErrorMsg(ModuleRes(RID_STR_FIELD_DOESNT_EXIST));
                sErrorMsg.SearchAndReplaceAscii("$name$",_sColumnName);
                OSQLErrorBox( this, sErrorMsg ).Execute();
                bError = sal_True;
            }
            else
            {
                pEntryTab = static_cast<OQueryTableWindow*>(_pEntry->GetTabWindow());
                notifyTableFieldChanged(String(),_pEntry->GetAlias(),_bListAction,GetCurColumnId());
            }
        }
    }
    if ( pEntryTab ) // here we got a valid table
        _pEntry->SetField(_sColumnName);

    return bError;
}

sal_Bool OSelectionBrowseBox::saveField(String& _sFieldName ,OTableFieldDescRef& _pEntry, sal_Bool& _bListAction)
{
    sal_Bool bError = sal_False;

    OQueryController& rController = static_cast<OQueryController&>(getDesignView()->getController());

    // first look if the name can be found in our tables
    sal_uInt16 nTabCount = 0;
    String sOldAlias = _pEntry->GetAlias();
    if ( static_cast<OQueryTableView*>(getDesignView()->getTableView())->FindTableFromField(_sFieldName,_pEntry,nTabCount) )
    {
        // append undo action for the alias name
        _pEntry->SetField(_sFieldName);
        notifyTableFieldChanged(sOldAlias,_pEntry->GetAlias(),_bListAction,GetCurColumnId());
        clearEntryFunctionField(_sFieldName,_pEntry,_bListAction,_pEntry->GetColumnId());
        return bError;
    }

    Reference<XConnection> xConnection( rController.getConnection() );
    Reference< XDatabaseMetaData > xMetaData;
    if ( xConnection.is() )
        xMetaData = xConnection->getMetaData();
    OSL_ENSURE( xMetaData.is(), "OSelectionBrowseBox::saveField: invalid connection/meta data!" );
    if ( !xMetaData.is() )
        return sal_True;

    OUString sErrorMsg;
    // second test if the name can be set as select columns in a pseudo statement
    // we have to look which entries  we should quote

    const OUString sFieldAlias = _pEntry->GetFieldAlias();
    ::connectivity::OSQLParser& rParser( rController.getParser() );
    {
        // automatically add parentheses around subqueries
        OSQLParseNode *pParseNode = NULL;
        OUString devnull;
        pParseNode = rParser.parseTree( devnull, _sFieldName, true );
        if (pParseNode == NULL)
            pParseNode = rParser.parseTree( devnull, _sFieldName, false );
        if (pParseNode != NULL && SQL_ISRULE(pParseNode, select_statement))
            _sFieldName = OUString("(") + _sFieldName + ")";
    }

    OSQLParseNode* pParseNode = NULL;
    {
        // 4 passes in trying to interprete the field name
        // - don't quote the field name, parse internationally
        // - don't quote the field name, parse en-US
        // - quote the field name, parse internationally
        // - quote the field name, parse en-US
        size_t nPass = 4;
        OUString sQuotedFullFieldName(::dbtools::quoteName( xMetaData->getIdentifierQuoteString(), _sFieldName ));
        OUString sFullFieldName(_sFieldName);

        if  ( _pEntry->isAggreateFunction() )
        {
            OSL_ENSURE(!_pEntry->GetFunction().isEmpty(),"Functionname darf hier nicht leer sein! ;-(");
            sQuotedFullFieldName = _pEntry->GetFunction() + "(" + sQuotedFullFieldName + ")";
            sFullFieldName = _pEntry->GetFunction() + "(" + sFullFieldName + ")";
        }

        do
        {
            bool bQuote = ( nPass <= 2 );
            bool bInternational = ( nPass % 2 ) == 0;

            OUString sSql;
            if ( bQuote )
                sSql += sQuotedFullFieldName;
            else
                sSql += sFullFieldName;

            sSql = OUString("SELECT ") + sSql;
            if ( !sFieldAlias.isEmpty() )
            { // always quote the alias name: there cannot be a function in it
                sSql += OUString(" ");
                sSql += ::dbtools::quoteName( xMetaData->getIdentifierQuoteString(), sFieldAlias );
            }
            sSql += OUString(" FROM x");

            pParseNode = rParser.parseTree( sErrorMsg, sSql, bInternational );
        }
        while ( ( pParseNode == NULL ) && ( --nPass > 0 ) );
    }

    if ( pParseNode == NULL )
    {
        // something different which we have to check
        String sErrorMessage( ModuleRes( STR_QRY_COLUMN_NOT_FOUND ) );
        sErrorMessage.SearchAndReplaceAscii("$name$",_sFieldName);
        OSQLErrorBox( this, sErrorMessage ).Execute();

        return sal_True;
    }

    // we got a valid select column
    // find what type of column has be inserted
    ::connectivity::OSQLParseNode* pSelection = pParseNode->getChild(2);
    if ( SQL_ISRULE(pSelection,selection) ) // we found the asterix
    {
        _pEntry->SetField(_sFieldName);
        clearEntryFunctionField(_sFieldName,_pEntry,_bListAction,_pEntry->GetColumnId());
    }
    else // travel through the select column parse node
    {
        ::comphelper::UStringMixEqual bCase(xMetaData->supportsMixedCaseQuotedIdentifiers());

        OTableFieldDescRef aSelEntry = _pEntry;
        sal_uInt16 nColumnId = aSelEntry->GetColumnId();

        sal_uInt32 nCount = pSelection->count();
        for (sal_uInt32 i = 0; i < nCount; ++i)
        {
            if ( i > 0 ) // may we have to append more than one field
            {
                sal_uInt16 nColumnPostion;
                aSelEntry = FindFirstFreeCol(nColumnPostion);
                if ( !aSelEntry.is() )
                {
                    AppendNewCol(1);
                    aSelEntry = FindFirstFreeCol(nColumnPostion);
                }
                ++nColumnPostion;
                nColumnId = GetColumnId(nColumnPostion);
            }

            ::connectivity::OSQLParseNode* pChild = pSelection->getChild( i );
            OSL_ENSURE(SQL_ISRULE(pChild,derived_column), "No derived column found!");
            // get the column alias
            OUString sColumnAlias = OSQLParseTreeIterator::getColumnAlias(pChild);
            if ( !sColumnAlias.isEmpty() ) // we found an as clause
            {
                String aSelectionAlias = aSelEntry->GetFieldAlias();
                aSelEntry->SetFieldAlias( sColumnAlias );
                // append undo
                appendUndoAction(aSelectionAlias,aSelEntry->GetFieldAlias(),BROW_COLUMNALIAS_ROW,_bListAction);
                if ( m_bVisibleRow[BROW_COLUMNALIAS_ROW] )
                    RowModified(GetBrowseRow(BROW_COLUMNALIAS_ROW), nColumnId);
            }

            ::connectivity::OSQLParseNode* pColumnRef = pChild->getChild(0);
            if (
                    pColumnRef->getKnownRuleID() != OSQLParseNode::subquery &&
                    pColumnRef->count() == 3 &&
                    SQL_ISPUNCTUATION(pColumnRef->getChild(0),"(") &&
                    SQL_ISPUNCTUATION(pColumnRef->getChild(2),")")
                )
                pColumnRef = pColumnRef->getChild(1);

            if ( SQL_ISRULE(pColumnRef,column_ref) ) // we found a valid column name or more column names
            {
                // look if we can find the corresponding table
                bError = fillColumnRef( pColumnRef, xConnection, aSelEntry, _bListAction );

                // we found a simple column so we must clear the function fields but only when the column name is '*'
                // and the function is different to count
                clearEntryFunctionField(_sFieldName,aSelEntry,_bListAction,nColumnId);
            }
            // do we have a aggregate function and only a function?
            else if ( SQL_ISRULE(pColumnRef,general_set_fct) )
            {
                OUString sLocalizedFunctionName;
                if ( GetFunctionName(pColumnRef->getChild(0)->getTokenID(),sLocalizedFunctionName) )
                {
                    String sOldLocalizedFunctionName = aSelEntry->GetFunction();
                    aSelEntry->SetFunction(sLocalizedFunctionName);
                    sal_uInt32 nFunCount = pColumnRef->count() - 1;
                    sal_Int32 nFunctionType = FKT_AGGREGATE;
                    sal_Bool bQuote = sal_False;
                    // may be there exists only one parameter which is a column, fill all information into our fields
                    if ( nFunCount == 4 && SQL_ISRULE(pColumnRef->getChild(3),column_ref) )
                        bError = fillColumnRef( pColumnRef->getChild(3), xConnection, aSelEntry, _bListAction );
                    else if ( nFunCount == 3 ) // we have a COUNT(*) here, so take the first table
                        bError = fillColumnRef( OUString("*"), OUString(), xMetaData, aSelEntry, _bListAction );
                    else
                    {
                        nFunctionType |= FKT_NUMERIC;
                        bQuote = sal_True;
                        aSelEntry->SetDataType(DataType::DOUBLE);
                        aSelEntry->SetFieldType(TAB_NORMAL_FIELD);
                    }

                    // now parse the parameters
                    OUString sParameters;
                    for(sal_uInt32 function = 2; function < nFunCount; ++function) // we only want to parse the parameters of the function
                        pColumnRef->getChild(function)->parseNodeToStr( sParameters, xConnection, &rParser.getContext(), sal_True, bQuote );

                    aSelEntry->SetFunctionType(nFunctionType);
                    aSelEntry->SetField(sParameters);
                    if ( aSelEntry->IsGroupBy() )
                    {
                        sOldLocalizedFunctionName = m_aFunctionStrings.GetToken(comphelper::string::getTokenCount(m_aFunctionStrings, ';')-1);
                        aSelEntry->SetGroupBy(sal_False);
                    }

                    // append undo action
                    notifyFunctionFieldChanged(sOldLocalizedFunctionName,sLocalizedFunctionName,_bListAction, nColumnId);
                }
                else
                    OSL_FAIL("Unsupported function inserted!");

            }
            else
            {
                // so we first clear the function field
                clearEntryFunctionField(_sFieldName,aSelEntry,_bListAction,nColumnId);
                OUString sFunction;
                pColumnRef->parseNodeToStr( sFunction,
                                            xConnection,
                                            &rController.getParser().getContext(),
                                            sal_True,
                                            sal_True); // quote is to true because we need quoted elements inside the function

                getDesignView()->fillFunctionInfo(pColumnRef,sFunction,aSelEntry);

                if( SQL_ISRULEOR3(pColumnRef, position_exp, extract_exp, fold) ||
                    SQL_ISRULEOR3(pColumnRef, char_substring_fct, length_exp, char_value_fct) )
                    // a calculation has been found ( can be calc and function )
                {
                    // now parse the whole statement
                    sal_uInt32 nFunCount = pColumnRef->count();
                    OUString sParameters;
                    for(sal_uInt32 function = 0; function < nFunCount; ++function)
                        pColumnRef->getChild(function)->parseNodeToStr( sParameters, xConnection, &rParser.getContext(), sal_True, sal_True );

                    sOldAlias = aSelEntry->GetAlias();
                    sal_Int32 nNewFunctionType = aSelEntry->GetFunctionType() | FKT_NUMERIC | FKT_OTHER;
                    aSelEntry->SetFunctionType(nNewFunctionType);
                    aSelEntry->SetField(sParameters);
                }
                else
                {
                    aSelEntry->SetFieldAlias(sColumnAlias);
                    if ( SQL_ISRULE(pColumnRef,set_fct_spec) )
                        aSelEntry->SetFunctionType(/*FKT_NUMERIC | */FKT_OTHER);
                    else
                        aSelEntry->SetFunctionType(FKT_NUMERIC | FKT_OTHER);
                }

                aSelEntry->SetAlias(OUString());
                notifyTableFieldChanged(sOldAlias,aSelEntry->GetAlias(),_bListAction, nColumnId);
            }

            if ( i > 0 && !InsertField(aSelEntry,BROWSER_INVALIDID,sal_True,sal_False).is() ) // may we have to append more than one field
            { // the field could not be inserted
                String sErrorMessage( ModuleRes( RID_STR_FIELD_DOESNT_EXIST ) );
                sErrorMessage.SearchAndReplaceAscii("$name$",aSelEntry->GetField());
                OSQLErrorBox( this, sErrorMessage ).Execute();
                bError = sal_True;
            }
        }
    }
    delete pParseNode;

    return bError;
}

sal_Bool OSelectionBrowseBox::SaveModified()
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OQueryController& rController = static_cast<OQueryController&>(getDesignView()->getController());
    OTableFieldDescRef pEntry = NULL;
    sal_uInt16 nCurrentColumnPos = GetColumnPos(GetCurColumnId());
    if(getFields().size() > static_cast<sal_uInt16>(nCurrentColumnPos - 1))
        pEntry = getEntry(nCurrentColumnPos - 1);

    sal_Bool bWasEmpty = pEntry.is() ? pEntry->IsEmpty() : sal_False;
    sal_Bool bError         = sal_False;
    sal_Bool bListAction    = sal_False;

    if (pEntry.is() && Controller().Is() && Controller()->IsModified())
    {
        // for the Undo-action
        String strOldCellContents,sNewValue;
        long nRow = GetRealRow(GetCurRow());
        sal_Bool bAppendRow = sal_False;
        switch (nRow)
        {
            case BROW_VIS_ROW:
                {
                    sal_Bool bOldValue = m_pVisibleCell->GetBox().GetSavedValue() != STATE_NOCHECK;
                    strOldCellContents = bOldValue ? g_strOne : g_strZero;
                    sNewValue          = !bOldValue ? g_strOne : g_strZero;
                }
                if((m_bOrderByUnRelated || pEntry->GetOrderDir() == ORDER_NONE) &&
                   (m_bGroupByUnRelated || !pEntry->IsGroupBy()))
                {
                    pEntry->SetVisible(m_pVisibleCell->GetBox().IsChecked());
                }
                else
                {
                    pEntry->SetVisible(sal_True);
                    m_pVisibleCell->GetBox().Check();
                }
                break;

            case BROW_FIELD_ROW:
            {
                String aFieldName(m_pFieldCell->GetText());
                try
                {
                    if (!aFieldName.Len())
                    {
                        OTableFieldDescRef pNewEntry = new OTableFieldDesc();
                        pNewEntry->SetColumnId( pEntry->GetColumnId() );
                        ::std::replace(getFields().begin(),getFields().end(),pEntry,pNewEntry);
                        sal_uInt16 nCol = GetCurColumnId();
                        for (int i = 0; i < m_nVisibleCount; i++)   // redraw column
                            RowModified(i,nCol);
                    }
                    else
                    {
                        strOldCellContents = pEntry->GetField();
                        bListAction = sal_True;
                        if ( !m_bInUndoMode )
                            rController.GetUndoManager().EnterListAction(String(),String());

                        sal_uInt16 nPos = m_pFieldCell->GetEntryPos(aFieldName);
                        String aAliasName = pEntry->GetAlias();
                        if ( nPos != COMBOBOX_ENTRY_NOTFOUND && !aAliasName.Len() && comphelper::string::getTokenCount(aFieldName, '.') > 1 )
                        { // special case, we have a table field so we must cut the table name
                            String sTableAlias = aFieldName.GetToken(0,'.');
                            pEntry->SetAlias(sTableAlias);
                            String sColumnName = aFieldName.Copy(sTableAlias.Len()+1,aFieldName.Len() - sTableAlias.Len() -1);
                            Reference<XConnection> xConnection = rController.getConnection();
                            if ( !xConnection.is() )
                                return sal_False;
                            bError = fillColumnRef( sColumnName, sTableAlias, xConnection->getMetaData(), pEntry, bListAction );
                        }
                        else
                            bError = sal_True;

                        if ( bError )
                            bError = saveField(aFieldName,pEntry,bListAction);
                    }
                }
                catch(Exception&)
                {
                    bError = sal_True;
                }
                if ( bError )
                {
                    sNewValue = aFieldName;
                    if ( !m_bInUndoMode )
                        static_cast<OQueryController&>(getDesignView()->getController()).GetUndoManager().LeaveListAction();
                    bListAction = sal_False;
                }
                else
                    sNewValue = pEntry->GetField();
                rController.InvalidateFeature( ID_BROWSER_QUERY_EXECUTE );
            }
            break;

            case BROW_TABLE_ROW:
            {
                String aAliasName = m_pTableCell->GetSelectEntry();
                strOldCellContents = pEntry->GetAlias();
                if ( m_pTableCell->GetSelectEntryPos() != 0 )
                {
                    pEntry->SetAlias(aAliasName);
                    // we have to set the table name as well as the table window
                    OJoinTableView::OTableWindowMap* pTabWinList = getDesignView()->getTableView()->GetTabWinMap();
                    if (pTabWinList)
                    {
                        OJoinTableView::OTableWindowMapIterator aIter = pTabWinList->find(aAliasName);
                        if(aIter != pTabWinList->end())
                        {
                            OQueryTableWindow* pEntryTab = static_cast<OQueryTableWindow*>(aIter->second);
                            if (pEntryTab)
                            {
                                pEntry->SetTable(pEntryTab->GetTableName());
                                pEntry->SetTabWindow(pEntryTab);
                            }
                        }
                    }
                }
                else
                {
                    pEntry->SetAlias(OUString());
                    pEntry->SetTable(OUString());
                    pEntry->SetTabWindow(NULL);
                }
                sNewValue = pEntry->GetAlias();

            }   break;

            case BROW_ORDER_ROW:
            {
                strOldCellContents = OUString::number((sal_uInt16)pEntry->GetOrderDir());
                sal_uInt16 nIdx = m_pOrderCell->GetSelectEntryPos();
                if (nIdx == sal_uInt16(-1))
                    nIdx = 0;
                pEntry->SetOrderDir(EOrderDir(nIdx));
                if(!m_bOrderByUnRelated)
                {
                    pEntry->SetVisible(sal_True);
                    m_pVisibleCell->GetBox().Check();
                    RowModified(GetBrowseRow(BROW_VIS_ROW), GetCurColumnId());
                }
                sNewValue = OUString::number((sal_uInt16)pEntry->GetOrderDir());
            }   break;

            case BROW_COLUMNALIAS_ROW:
                strOldCellContents = pEntry->GetFieldAlias();
                pEntry->SetFieldAlias(m_pTextCell->GetText());
                sNewValue = pEntry->GetFieldAlias();
                break;
            case BROW_FUNCTION_ROW:
                {
                    strOldCellContents = pEntry->GetFunction();
                    sal_uInt16 nPos = m_pFunctionCell->GetSelectEntryPos();
                    // these functions are only available in CORE
                    String sFunctionName        = m_pFunctionCell->GetEntry(nPos);
                    OUString sGroupFunctionName   = m_aFunctionStrings.GetToken(comphelper::string::getTokenCount(m_aFunctionStrings, ';')-1);
                    sal_Bool bGroupBy = sal_False;
                    if ( sGroupFunctionName.equals(sFunctionName) ) // check if the function name is GROUP
                    {
                        bGroupBy = sal_True;

                        if ( !m_bGroupByUnRelated && !pEntry->IsVisible() )
                        {
                            // we have to change the visblie flag, so we must append also an undo action
                            pEntry->SetVisible(sal_True);
                            m_pVisibleCell->GetBox().Check();
                            appendUndoAction(g_strZero,g_strOne,BROW_VIS_ROW,bListAction);
                            RowModified(GetBrowseRow(BROW_VIS_ROW), GetCurColumnId());
                        }

                        pEntry->SetFunction(String());
                        pEntry->SetFunctionType(pEntry->GetFunctionType() & ~FKT_AGGREGATE );
                    }
                    else if ( nPos ) // we found an aggregate function
                    {
                        pEntry->SetFunctionType(pEntry->GetFunctionType() | FKT_AGGREGATE );
                        pEntry->SetFunction(sFunctionName);
                    }
                    else
                    {
                        sFunctionName = String();
                        pEntry->SetFunction(String());
                        pEntry->SetFunctionType(pEntry->GetFunctionType() & ~FKT_AGGREGATE );
                    }

                    pEntry->SetGroupBy(bGroupBy);

                    sNewValue = sFunctionName;
                }
                break;
            default:
            {
                Reference< XConnection> xConnection = static_cast<OQueryController&>(getDesignView()->getController()).getConnection();
                if(!xConnection.is())
                    break;

                sal_uInt16  nIdx = sal_uInt16(nRow - BROW_CRIT1_ROW);
                String aText = comphelper::string::stripStart(m_pTextCell->GetText(), ' ');

                OUString aCrit;
                if(aText.Len())
                {
                    OUString aErrorMsg;
                    Reference<XPropertySet> xColumn;
                    OSQLParseNode* pParseNode = getDesignView()->getPredicateTreeFromEntry(pEntry,aText,aErrorMsg,xColumn);

                    if (pParseNode)
                    {
                        pParseNode->parseNodeToPredicateStr(aCrit,
                                                            xConnection,
                                                            static_cast<OQueryController&>(getDesignView()->getController()).getNumberFormatter(),
                                                            xColumn,
                                                            pEntry->GetAlias(),
                                                            getDesignView()->getLocale(),
                                                            static_cast<sal_Char>(getDesignView()->getDecimalSeparator().toChar()),
                                                            &(static_cast<OQueryController&>(getDesignView()->getController()).getParser().getContext()));
                        delete pParseNode;
                    }
                    else
                    {
                        if(xColumn.is())
                        {
                            sal_Int32 nType = 0;
                            xColumn->getPropertyValue(PROPERTY_TYPE) >>= nType;
                            switch(nType)
                            {
                                case DataType::CHAR:
                                case DataType::VARCHAR:
                                case DataType::LONGVARCHAR:
                                case DataType::CLOB:
                                    if(aText.GetChar(0) != '\'' || aText.GetChar(aText.Len() -1) != '\'')
                                    {
                                        aText.SearchAndReplaceAll(OUString("'"), OUString("''"));
                                        String aTmp(OUString("'"));
                                        (aTmp += aText) += OUString("'");
                                        aText = aTmp;
                                    }
                                    break;
                                default:
                                    ;
                            }
                            ::connectivity::OSQLParser& rParser = static_cast<OQueryController&>(getDesignView()->getController()).getParser();
                            pParseNode = rParser.predicateTree(aErrorMsg,
                                                                aText,
                                                                static_cast<OQueryController&>(getDesignView()->getController()).getNumberFormatter(),
                                                                xColumn);
                            if (pParseNode)
                            {
                                pParseNode->parseNodeToPredicateStr(aCrit,
                                                                    xConnection,
                                                                    static_cast<OQueryController&>(getDesignView()->getController()).getNumberFormatter(),
                                                                    xColumn,
                                                                    pEntry->GetAlias(),
                                                                    getDesignView()->getLocale(),
                                                                    static_cast<sal_Char>(getDesignView()->getDecimalSeparator().toChar()),
                                                                    &(static_cast<OQueryController&>(getDesignView()->getController()).getParser().getContext()));
                                delete pParseNode;
                            }
                            else
                            {
                                if ( !m_bDisableErrorBox )
                                {
                                    OSQLWarningBox( this, aErrorMsg ).Execute();
                                }
                                bError = sal_True;
                            }
                        }
                        else
                        {
                            if ( !m_bDisableErrorBox )
                            {
                                OSQLWarningBox( this, aErrorMsg ).Execute();
                            }
                            bError = sal_True;
                        }
                    }
                }
                strOldCellContents = pEntry->GetCriteria(nIdx);
                pEntry->SetCriteria(nIdx, aCrit);
                sNewValue = pEntry->GetCriteria(nIdx);
                if(!aCrit.isEmpty() && nRow >= (GetRowCount()-1))
                    bAppendRow = sal_True;
            }
        }
        if(!bError && Controller())
            Controller()->ClearModified();

        RowModified(GetCurRow(), GetCurColumnId());

        if ( bAppendRow )
        {
            RowInserted( GetRowCount()-1, 1, sal_True );
            m_bVisibleRow.push_back(sal_True);
            ++m_nVisibleCount;
        }

        if(!bError)
        {
            // and now the undo-action for the total
            appendUndoAction(strOldCellContents,sNewValue,nRow);

        }
    }

    // did I store data in a FieldDescription which was empty before and which is not empty anymore after the changes?
    if ( pEntry.is() && bWasEmpty && !pEntry->IsEmpty() && !bError )
    {
        // Default to visible
        pEntry->SetVisible(sal_True);
        appendUndoAction(g_strZero,g_strOne,BROW_VIS_ROW,bListAction);
        RowModified(BROW_VIS_ROW, GetCurColumnId());

        // if required add empty columns
        sal_uInt16 nDummy;
        CheckFreeColumns(nDummy);
    }

    if ( bListAction && !m_bInUndoMode )
        static_cast<OQueryController&>(getDesignView()->getController()).GetUndoManager().LeaveListAction();

    return pEntry != NULL && !bError;
}

sal_Bool OSelectionBrowseBox::SeekRow(long nRow)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    sal_Bool bRet = sal_False;

    m_nSeekRow = nRow;
    if (nRow < m_nVisibleCount )
        bRet = sal_True;

    return bRet;
}

void OSelectionBrowseBox::PaintCell(OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColumnId) const
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    rDev.SetClipRegion(Region(rRect));

    OTableFieldDescRef pEntry = NULL;
    sal_uInt16 nPos = GetColumnPos(nColumnId);
    if(getFields().size() > sal_uInt16(nPos - 1))
        pEntry = getFields()[nPos - 1];

    if (!pEntry.is())
        return;

    long nRow = GetRealRow(m_nSeekRow);
    if (nRow == BROW_VIS_ROW)
        PaintTristate(rDev, rRect, pEntry->IsVisible() ? STATE_CHECK : STATE_NOCHECK);
    else
        rDev.DrawText(rRect, GetCellText(nRow, nColumnId),TEXT_DRAW_VCENTER);

    rDev.SetClipRegion( );
}

void OSelectionBrowseBox::PaintStatusCell(OutputDevice& rDev, const Rectangle& rRect) const
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    Rectangle aRect(rRect);
    aRect.TopLeft().Y() -= 2;
    String  aLabel(ModuleRes(STR_QUERY_HANDLETEXT));

   // from BROW_CRIT2_ROW onwards all rows are shown "or"
    xub_StrLen nToken = (xub_StrLen) (m_nSeekRow >= GetBrowseRow(BROW_CRIT2_ROW))
                                ?
            xub_StrLen(BROW_CRIT2_ROW) : xub_StrLen(GetRealRow(m_nSeekRow));
    rDev.DrawText(aRect, aLabel.GetToken(nToken),TEXT_DRAW_VCENTER);
}

void OSelectionBrowseBox::RemoveColumn(sal_uInt16 _nColumnId)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OQueryController& rController = static_cast<OQueryController&>(getDesignView()->getController());

    sal_uInt16 nPos = GetColumnPos(_nColumnId);
    // the control should always have exactly one more column: the HandleColumn
    OSL_ENSURE((nPos == 0) || (nPos <= getFields().size()), "OSelectionBrowseBox::RemoveColumn : invalid parameter nColId");
    // ColId is synonymous to Position, and the condition should be valid

    sal_uInt16 nCurCol = GetCurColumnId();
    long nCurrentRow = GetCurRow();

    DeactivateCell();

    getFields().erase( getFields().begin() + (nPos - 1) );
    OTableFieldDescRef pEntry = new OTableFieldDesc();
    pEntry->SetColumnId(_nColumnId);
    getFields().push_back(pEntry);

    EditBrowseBox::RemoveColumn( _nColumnId );
    InsertDataColumn( _nColumnId , String(), DEFAULT_SIZE, HIB_STDSTYLE, HEADERBAR_APPEND);

    // Neuzeichnen
    Rectangle aInvalidRect = GetInvalidRect( _nColumnId );
    Invalidate( aInvalidRect );

    ActivateCell( nCurrentRow, nCurCol );

    rController.setModified( sal_True );

    invalidateUndoRedo();
}

void OSelectionBrowseBox::RemoveField(sal_uInt16 nColumnId )
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OQueryController& rController = static_cast<OQueryController&>(getDesignView()->getController());

    sal_uInt16 nPos = GetColumnPos(nColumnId);
    OSL_ENSURE(getFields().size() > sal_uInt16(nPos-1),"ID is to great!");

    OTableFieldDescRef pDesc = getEntry((sal_uInt32)(nPos - 1)) ;
    pDesc->SetColWidth( (sal_uInt16)GetColumnWidth(nColumnId) );    // was not stored this before

    // trigger UndoAction
    if ( !m_bInUndoMode )
    {
        OTabFieldDelUndoAct* pUndoAction = new OTabFieldDelUndoAct( this );
        pUndoAction->SetTabFieldDescr(pDesc);
        pUndoAction->SetColumnPosition(nPos);
        rController.addUndoActionAndInvalidate( pUndoAction );
    }

    RemoveColumn(nColumnId);

    invalidateUndoRedo();
}

void OSelectionBrowseBox::adjustSelectionMode( sal_Bool _bClickedOntoHeader, sal_Bool _bClickedOntoHandleCol )
{
    // if a Header has been selected it should be shown otherwise not
    if ( _bClickedOntoHeader )
    {
        if (0 == GetSelectColumnCount() )
            // I am in the correct mode if a selected column exists
            if ( BROWSER_HIDESELECT == ( m_nMode & BROWSER_HIDESELECT ) )
            {
                m_nMode &= ~BROWSER_HIDESELECT;
                m_nMode |= BROWSER_MULTISELECTION;
                SetMode( m_nMode );
            }
    }
    else if ( BROWSER_HIDESELECT != ( m_nMode & BROWSER_HIDESELECT ) )
    {
        if ( GetSelectColumnCount() != 0 )
            SetNoSelection();

        if ( _bClickedOntoHandleCol )
        {
            m_nMode |= BROWSER_HIDESELECT;
            m_nMode &= ~BROWSER_MULTISELECTION;
            SetMode( m_nMode );
        }
    }
}

void OSelectionBrowseBox::MouseButtonDown(const BrowserMouseEvent& rEvt)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    if( rEvt.IsLeft() )
    {
        sal_Bool bOnHandle = HANDLE_ID == rEvt.GetColumnId();
        sal_Bool bOnHeader = ( rEvt.GetRow() < 0 ) && !bOnHandle;
        adjustSelectionMode( bOnHeader, bOnHandle );
    }
    EditBrowseBox::MouseButtonDown(rEvt);
}

void OSelectionBrowseBox::MouseButtonUp(const BrowserMouseEvent& rEvt)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    EditBrowseBox::MouseButtonUp( rEvt );
    static_cast<OQueryController&>(getDesignView()->getController()).InvalidateFeature( ID_BROWSER_QUERY_EXECUTE );
}

void OSelectionBrowseBox::KeyInput( const KeyEvent& rEvt )
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    if (IsColumnSelected(GetCurColumnId()))
    {
        if (rEvt.GetKeyCode().GetCode() == KEY_DELETE &&    // Delete rows
            !rEvt.GetKeyCode().IsShift() &&
            !rEvt.GetKeyCode().IsMod1())
        {
            RemoveField(GetCurColumnId());
            return;
        }
    }
    EditBrowseBox::KeyInput(rEvt);
}

sal_Int8 OSelectionBrowseBox::AcceptDrop( const BrowserAcceptDropEvent& rEvt )
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    sal_Int8 nDropAction = DND_ACTION_NONE;
    if  ( rEvt.GetRow() >= -1 )
    {
        if ( IsEditing() )
        {
            // allow the asterix again
            m_bDisableErrorBox = sal_True;
            SaveModified();
            m_bDisableErrorBox = sal_False;
            DeactivateCell();
        }
        // check if the format is already supported, if not deactivate the current cell and try again
        if ( OJoinExchObj::isFormatAvailable(GetDataFlavors()) )
            nDropAction = DND_ACTION_LINK;
    }

    return nDropAction;
}

sal_Int8 OSelectionBrowseBox::ExecuteDrop( const BrowserExecuteDropEvent& _rEvt )
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);

    TransferableDataHelper aDropped(_rEvt.maDropEvent.Transferable);
    if (!OJoinExchObj::isFormatAvailable(aDropped.GetDataFlavorExVector()))
    {
        OSL_FAIL("OSelectionBrowseBox::ExecuteDrop: this should never have passed AcceptDrop!");
        return DND_ACTION_NONE;
    }

    OTableFieldDesc aInfo;
    // insert the field at the selected position
    OJoinExchangeData jxdSource = OJoinExchObj::GetSourceDescription(_rEvt.maDropEvent.Transferable);
    InsertField(jxdSource);

    return DND_ACTION_LINK;
}

OTableFieldDescRef OSelectionBrowseBox::AppendNewCol( sal_uInt16 nCnt)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    // one or more can be created, but the first one will is not returned
    sal_uInt32 nCount = getFields().size();
    for (sal_uInt16 i=0 ; i<nCnt ; i++)
    {
        OTableFieldDescRef pEmptyEntry = new OTableFieldDesc();
        getFields().push_back(pEmptyEntry);
        sal_uInt16 nColumnId = sal::static_int_cast< sal_uInt16 >(getFields().size());
        pEmptyEntry->SetColumnId( nColumnId );

        InsertDataColumn( nColumnId , String(), DEFAULT_SIZE, HIB_STDSTYLE, HEADERBAR_APPEND);
    }

    return getFields()[nCount];
}

void OSelectionBrowseBox::DeleteFields(const String& rAliasName)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    if (!getFields().empty())
    {
        sal_uInt16 nColId = GetCurColumnId();
        sal_uInt32 nRow = GetCurRow();

        sal_Bool bWasEditing = IsEditing();
        if (bWasEditing)
            DeactivateCell();

        OTableFields::reverse_iterator aIter = getFields().rbegin();
        OTableFieldDescRef pEntry = NULL;
        for(sal_uInt16 nPos=sal::static_int_cast< sal_uInt16 >(getFields().size());aIter != getFields().rend();++aIter,--nPos)
        {
            pEntry = *aIter;
            if ( pEntry->GetAlias().equals( rAliasName ) )
            {
                RemoveField( GetColumnId( nPos ) );
                break;
            }
        }

        if (bWasEditing)
            ActivateCell(nRow , nColId);
    }
}

void OSelectionBrowseBox::SetColWidth(sal_uInt16 nColId, long nNewWidth)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    sal_Bool bWasEditing = IsEditing();
    if (bWasEditing)
        DeactivateCell();

    // create the BaseClass
    SetColumnWidth(nColId, nNewWidth);

    // tell it the FieldDescription
    OTableFieldDescRef pEntry = getEntry(GetColumnPos(nColId) - 1);
    if (pEntry.is())
        pEntry->SetColWidth(sal_uInt16(GetColumnWidth(nColId)));

    if (bWasEditing)
        ActivateCell(GetCurRow(), GetCurColumnId());
}

Rectangle OSelectionBrowseBox::GetInvalidRect( sal_uInt16 nColId )
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    // The rectangle is the full output area of the window
    Rectangle aInvalidRect( Point(0,0), GetOutputSizePixel() );

    // now update the left side
    Rectangle aFieldRect(GetCellRect( 0, nColId )); // used instead of GetFieldRectPixel
    aInvalidRect.Left() = aFieldRect.Left();

    return aInvalidRect;
}

void OSelectionBrowseBox::InsertColumn(OTableFieldDescRef pEntry, sal_uInt16& _nColumnPosition)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    // the control should have exactly one more column: the HandleColumn
    OSL_ENSURE(_nColumnPosition == BROWSER_INVALIDID || (_nColumnPosition <= (long)getFields().size()), "OSelectionBrowseBox::InsertColumn : invalid parameter nColId.");
     // -1 means at the end. Count means at the end, others denotes a correct position

    sal_uInt16 nCurCol = GetCurColumnId();
    long nCurrentRow = GetCurRow();

    DeactivateCell();

    // remember the column id of the current positon
    sal_uInt16 nColumnId = GetColumnId(_nColumnPosition);
    // put at the end of the list if to small or to big,
    if ((_nColumnPosition == BROWSER_INVALIDID) || (_nColumnPosition >= getFields().size()))   // append the field
    {
        if (FindFirstFreeCol(_nColumnPosition) == NULL)  // no more free columns
        {
            AppendNewCol(1);
            _nColumnPosition = sal::static_int_cast< sal_uInt16 >(
                getFields().size());
        }
        else
            ++_nColumnPosition; // within the list
        nColumnId = GetColumnId(_nColumnPosition);
        pEntry->SetColumnId( nColumnId );
        getFields()[ _nColumnPosition - 1] = pEntry;
    }

    // check if the column ids are identical, if not we have to move
    if ( pEntry->GetColumnId() != nColumnId )
    {
        sal_uInt16 nOldPosition = GetColumnPos(pEntry->GetColumnId());
        OSL_ENSURE( nOldPosition != 0,"Old position was 0. Not possible!");
        SetColumnPos(pEntry->GetColumnId(),_nColumnPosition);
        // we have to delete an empty field for the fields list, because the columns must have equal length
        if ( nOldPosition > 0 && nOldPosition <= getFields().size() )
            getFields()[nOldPosition - 1] = pEntry;

        ColumnMoved(pEntry->GetColumnId(),sal_False);
    }

    if ( pEntry->GetFunctionType() & (FKT_AGGREGATE) )
    {
        OUString sFunctionName = pEntry->GetFunction();
        if ( GetFunctionName(sal_uInt32(-1),sFunctionName) )
            pEntry->SetFunction(sFunctionName);
    }

    nColumnId = pEntry->GetColumnId();

    SetColWidth(nColumnId,getDesignView()->getColWidth(GetColumnPos(nColumnId)-1));
    // redraw
    Rectangle aInvalidRect = GetInvalidRect( nColumnId );
    Invalidate( aInvalidRect );

    ActivateCell( nCurrentRow, nCurCol );
    static_cast<OQueryController&>(getDesignView()->getController()).setModified( sal_True );

    invalidateUndoRedo();
}

OTableFieldDescRef OSelectionBrowseBox::InsertField(const OJoinExchangeData& jxdSource, sal_uInt16 _nColumnPosition, sal_Bool bVis, sal_Bool bActivate)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OQueryTableWindow* pSourceWin = static_cast<OQueryTableWindow*>(jxdSource.pListBox->GetTabWin());
    if (!pSourceWin)
        return NULL;

    // name and position of the selected field
    String aFieldName = jxdSource.pListBox->GetEntryText(jxdSource.pEntry);
    sal_uInt32 nFieldIndex = jxdSource.pListBox->GetModel()->GetAbsPos(jxdSource.pEntry);
    OTableFieldInfo* pInf = static_cast<OTableFieldInfo*>(jxdSource.pEntry->GetUserData());

    // construct DragInfo, such that I use the other InsertField
    OTableFieldDescRef aInfo = new OTableFieldDesc(pSourceWin->GetTableName(),aFieldName);
    aInfo->SetTabWindow(pSourceWin);
    aInfo->SetFieldIndex(nFieldIndex);
    aInfo->SetFieldType(pInf->GetKeyType());
    aInfo->SetAlias(pSourceWin->GetAliasName());

    aInfo->SetDataType(pInf->GetDataType());
    aInfo->SetVisible(bVis);

    return InsertField(aInfo, _nColumnPosition, bVis, bActivate);
}

OTableFieldDescRef OSelectionBrowseBox::InsertField(const OTableFieldDescRef& _rInfo, sal_uInt16 _nColumnPosition, sal_Bool bVis, sal_Bool bActivate)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);

    if(m_nMaxColumns && m_nMaxColumns <= FieldsCount())
        return NULL;
    if (bActivate)
        SaveModified();

    // new column description
    OTableFieldDescRef pEntry = _rInfo;
    pEntry->SetVisible(bVis);

    // insert column
    InsertColumn( pEntry, _nColumnPosition );

    if ( !m_bInUndoMode )
    {
        // trigger UndoAction
        OTabFieldCreateUndoAct* pUndoAction = new OTabFieldCreateUndoAct( this );
        pUndoAction->SetTabFieldDescr( pEntry );
        pUndoAction->SetColumnPosition(_nColumnPosition);
        getDesignView()->getController().addUndoActionAndInvalidate( pUndoAction );
    }

    return pEntry;
}

sal_uInt16 OSelectionBrowseBox::FieldsCount()
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OTableFields::iterator aIter = getFields().begin();
    sal_uInt16 nCount = 0;

    while (aIter != getFields().end())
    {
        if ((*aIter).is() && !(*aIter)->IsEmpty())
            ++nCount;
        ++aIter;
    }

    return nCount;
}

OTableFieldDescRef OSelectionBrowseBox::FindFirstFreeCol(sal_uInt16& _rColumnPosition )
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OTableFields::iterator aIter = getFields().begin();
    OTableFields::iterator aEnd  = getFields().end();

    _rColumnPosition = BROWSER_INVALIDID;

    while ( aIter != aEnd )
    {
        ++_rColumnPosition;
        OTableFieldDescRef pEntry = (*aIter);
        if ( pEntry.is() && pEntry->IsEmpty() )
            return pEntry;
        ++aIter;
    }

    return NULL;
}

void OSelectionBrowseBox::CheckFreeColumns(sal_uInt16& _rColumnPosition)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    if (FindFirstFreeCol(_rColumnPosition) == NULL)
    {
        // it is full, append a Packen column
        AppendNewCol(DEFAULT_QUERY_COLS);
        OSL_VERIFY(FindFirstFreeCol(_rColumnPosition).is());
    }
}

void OSelectionBrowseBox::AddGroupBy( const OTableFieldDescRef& rInfo , sal_uInt32 /*_nCurrentPos*/)
{
    Reference< XConnection> xConnection = static_cast<OQueryController&>(getDesignView()->getController()).getConnection();
    if(!xConnection.is())
        return;
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OSL_ENSURE(!rInfo->IsEmpty(),"AddGroupBy:: OTableFieldDescRef sollte nicht Empty sein!");
    OTableFieldDescRef pEntry;
    const Reference<XDatabaseMetaData> xMeta = xConnection->getMetaData();
    const ::comphelper::UStringMixEqual bCase(xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers());
    //sal_Bool bAppend = sal_False;

    OTableFields& rFields = getFields();
    OTableFields::iterator aIter = rFields.begin();
    OTableFields::iterator aEnd = rFields.end();
    for(;aIter != aEnd;++aIter)
    {
        pEntry = *aIter;
        OSL_ENSURE(pEntry.is(),"OTableFieldDescRef was null!");

        const OUString   aField = pEntry->GetField();
        const OUString   aAlias = pEntry->GetAlias();

        if (bCase(aField,rInfo->GetField()) &&
            bCase(aAlias,rInfo->GetAlias()) &&
            pEntry->GetFunctionType() == rInfo->GetFunctionType() &&
            pEntry->GetFunction() == rInfo->GetFunction())
        {
            if ( pEntry->isNumericOrAggreateFunction() && rInfo->IsGroupBy() )
            {
                pEntry->SetGroupBy(sal_False);
                aIter = rFields.end();
                break;
            }
            else
            {
                if ( !pEntry->IsGroupBy() && !pEntry->HasCriteria() ) // here we have a where condition which is no having clause
                {
                    pEntry->SetGroupBy(rInfo->IsGroupBy());
                    if(!m_bGroupByUnRelated && pEntry->IsGroupBy())
                        pEntry->SetVisible(sal_True);
                    break;
                }
            }

        }
    }

    if (aIter == rFields.end())
    {
        OTableFieldDescRef pTmp = InsertField(rInfo, BROWSER_INVALIDID, sal_False, sal_False );
        if ( (pTmp->isNumericOrAggreateFunction() && rInfo->IsGroupBy()) ) // the GroupBy is inherited from rInfo
            pTmp->SetGroupBy(sal_False);
    }
}

void OSelectionBrowseBox::DuplicateConditionLevel( const sal_uInt16 nLevel)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    const sal_uInt16 nNewLevel = nLevel +1;
    OTableFields& rFields = getFields();
    OTableFields::iterator aIter = rFields.begin();
    OTableFields::iterator aEnd = rFields.end();
    for(;aIter != aEnd;++aIter)
    {
        OTableFieldDescRef pEntry = *aIter;

        OUString sValue = pEntry->GetCriteria(nLevel);
        if ( !sValue.isEmpty() )
        {
            pEntry->SetCriteria( nNewLevel, sValue);
            if ( nNewLevel == (m_nVisibleCount-BROW_CRIT1_ROW-1) )
            {
                RowInserted( GetRowCount()-1, 1, sal_True );
                m_bVisibleRow.push_back(sal_True);
                ++m_nVisibleCount;
            }
            m_bVisibleRow[BROW_CRIT1_ROW + nNewLevel] = sal_True;
        }
    }
}

void OSelectionBrowseBox::AddCondition( const OTableFieldDescRef& rInfo, const String& rValue, const sal_uInt16 nLevel,bool _bAddOrOnOneLine )
{
    Reference< XConnection> xConnection = static_cast<OQueryController&>(getDesignView()->getController()).getConnection();
    if(!xConnection.is())
        return;
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OSL_ENSURE(rInfo.is() && !rInfo->IsEmpty(),"AddCondition:: OTableFieldDescRef sollte nicht Empty sein!");

    OTableFieldDescRef pLastEntry;
    Reference<XDatabaseMetaData> xMeta = xConnection->getMetaData();
    ::comphelper::UStringMixEqual bCase(xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers());

    OTableFields& rFields = getFields();
    OTableFields::iterator aIter = rFields.begin();
    OTableFields::iterator aEnd = rFields.end();
    for(;aIter != aEnd;++aIter)
    {
        OTableFieldDescRef pEntry = *aIter;
        const OUString   aField = pEntry->GetField();
        const OUString   aAlias = pEntry->GetAlias();

        if (bCase(aField,rInfo->GetField()) &&
            bCase(aAlias,rInfo->GetAlias()) &&
            pEntry->GetFunctionType() == rInfo->GetFunctionType() &&
            pEntry->GetFunction() == rInfo->GetFunction() &&
            pEntry->IsGroupBy() == rInfo->IsGroupBy() )
        {
            if ( pEntry->isNumericOrAggreateFunction() && rInfo->IsGroupBy() )
                pEntry->SetGroupBy(sal_False);
            else
            {
                if(!m_bGroupByUnRelated && pEntry->IsGroupBy())
                    pEntry->SetVisible(sal_True);
            }
            if (pEntry->GetCriteria(nLevel).isEmpty() )
            {
                pEntry->SetCriteria( nLevel, rValue);
                if(nLevel == (m_nVisibleCount-BROW_CRIT1_ROW-1))
                {
                    RowInserted( GetRowCount()-1, 1, sal_True );
                    m_bVisibleRow.push_back(sal_True);
                    ++m_nVisibleCount;
                }
                m_bVisibleRow[BROW_CRIT1_ROW + nLevel] = sal_True;
                break;
            }
            if ( _bAddOrOnOneLine )
            {
                pLastEntry = pEntry;
            }
        }
    }
    if ( pLastEntry.is() )
    {
        String sCriteria = rValue;
        String sOldCriteria = pLastEntry->GetCriteria( nLevel );
        if ( sOldCriteria.Len() )
        {
            sCriteria = String(RTL_CONSTASCII_USTRINGPARAM("( "));
            sCriteria += sOldCriteria;
            sCriteria += String(RTL_CONSTASCII_USTRINGPARAM(" OR "));
            sCriteria += rValue;
            sCriteria += String(RTL_CONSTASCII_USTRINGPARAM(" )"));
        }
        pLastEntry->SetCriteria( nLevel, sCriteria);
        if(nLevel == (m_nVisibleCount-BROW_CRIT1_ROW-1))
        {
            RowInserted( GetRowCount()-1, 1, sal_True );
            m_bVisibleRow.push_back(sal_True);
            ++m_nVisibleCount;
        }
        m_bVisibleRow[BROW_CRIT1_ROW + nLevel] = sal_True;
    }

    else if (aIter == getFields().end())
    {
        OTableFieldDescRef pTmp = InsertField(rInfo, BROWSER_INVALIDID, sal_False, sal_False );
        if ( pTmp->isNumericOrAggreateFunction() && rInfo->IsGroupBy() ) // the GroupBy was inherited from rInfo
            pTmp->SetGroupBy(sal_False);
        if ( pTmp.is() )
        {
            pTmp->SetCriteria( nLevel, rValue);
            if(nLevel == (m_nVisibleCount-BROW_CRIT1_ROW-1))
            {
                RowInserted( GetRowCount()-1, 1, sal_True );
                m_bVisibleRow.push_back(sal_True);
                ++m_nVisibleCount;
            }
        }
    }
}

void OSelectionBrowseBox::AddOrder( const OTableFieldDescRef& rInfo, const EOrderDir eDir, sal_uInt32 _nCurrentPos)
{
    if (_nCurrentPos == 0)
        m_nLastSortColumn = SORT_COLUMN_NONE;

    Reference< XConnection> xConnection = static_cast<OQueryController&>(getDesignView()->getController()).getConnection();
    if(!xConnection.is())
        return;
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OSL_ENSURE(!rInfo->IsEmpty(),"AddOrder:: OTableFieldDescRef should not be Empty!");
    OTableFieldDescRef pEntry;
    Reference<XDatabaseMetaData> xMeta = xConnection->getMetaData();
    ::comphelper::UStringMixEqual bCase(xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers());

    sal_Bool bAppend = sal_False;
    OTableFields& rFields = getFields();
    OTableFields::iterator aIter = rFields.begin();
    OTableFields::iterator aEnd = rFields.end();
    for(;aIter != aEnd;++aIter)
    {
        pEntry = *aIter;
        OUString aField = pEntry->GetField();
        OUString aAlias = pEntry->GetAlias();

        if (bCase(aField,rInfo->GetField()) &&
            bCase(aAlias,rInfo->GetAlias()))
        {
            sal_uInt32 nPos = aIter - rFields.begin();
            bAppend = (m_nLastSortColumn != SORT_COLUMN_NONE) && (nPos <= m_nLastSortColumn);
            if ( bAppend )
                aIter = rFields.end();
            else
            {
                if ( !m_bOrderByUnRelated )
                    pEntry->SetVisible(sal_True);
                pEntry->SetOrderDir( eDir );
                m_nLastSortColumn = nPos;
            }
            break;
        }
    }

    if (aIter == rFields.end())
    {
        OTableFieldDescRef pTmp = InsertField(rInfo, BROWSER_INVALIDID, sal_False, sal_False );
        if(pTmp.is())
        {
            m_nLastSortColumn = pTmp->GetColumnId() - 1;
            if ( !m_bOrderByUnRelated && !bAppend )
                pTmp->SetVisible(sal_True);
            pTmp->SetOrderDir( eDir );
        }
    }
}

void OSelectionBrowseBox::ArrangeControls(sal_uInt16& nX, sal_uInt16 nY)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    EditBrowseBox::ArrangeControls(nX, nY);
}

sal_Bool OSelectionBrowseBox::Save()
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    sal_Bool bRet = sal_True;
    if (IsModified())
        bRet = SaveModified();
    return bRet;
}

void OSelectionBrowseBox::CellModified()
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    long nRow = GetRealRow(GetCurRow());
    switch (nRow)
    {
        case BROW_VIS_ROW:
            {
                OTableFieldDescRef  pEntry = getEntry(GetColumnPos(GetCurColumnId()) - 1);

                sal_uInt16 nIdx = m_pOrderCell->GetSelectEntryPos();
                if(!m_bOrderByUnRelated && nIdx > 0 &&
                    nIdx != sal_uInt16(-1)          &&
                    !pEntry->IsEmpty()              &&
                    pEntry->GetOrderDir() != ORDER_NONE)
                {
                    m_pVisibleCell->GetBox().Check();
                    pEntry->SetVisible(sal_True);
                }
                else
                    pEntry->SetVisible(m_pVisibleCell->GetBox().IsChecked());
            }
            break;
    }
    static_cast<OQueryController&>(getDesignView()->getController()).setModified( sal_True );
}

void OSelectionBrowseBox::Fill()
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OSL_ENSURE(ColCount() >= 1, "OSelectionBrowseBox::Fill : please call only after inserting the handle column !");

    sal_uInt16 nColCount = ColCount() - 1;
    if (nColCount < DEFAULT_QUERY_COLS)
        AppendNewCol(DEFAULT_QUERY_COLS - nColCount);
}

Size OSelectionBrowseBox::CalcOptimalSize( const Size& _rAvailable )
{
    Size aReturn( _rAvailable.Width(), GetTitleHeight() );

    aReturn.Height() += ( m_nVisibleCount ? m_nVisibleCount : 15 ) * GetDataRowHeight();
    aReturn.Height() += 40; // just some space

    return aReturn;
}

void OSelectionBrowseBox::Command(const CommandEvent& rEvt)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    switch (rEvt.GetCommand())
    {
        case COMMAND_CONTEXTMENU:
        {
            Point aMenuPos( rEvt.GetMousePosPixel() );

            if (!rEvt.IsMouseEvent())
            {
                if  ( 1 == GetSelectColumnCount() )
                {
                    sal_uInt16 nSelId = GetColumnId(
                        sal::static_int_cast< sal_uInt16 >(
                            FirstSelectedColumn() ) );
                    ::Rectangle aColRect( GetFieldRectPixel( 0, nSelId, sal_False ) );

                    aMenuPos = aColRect.TopCenter();
                }
                else
                {
                    EditBrowseBox::Command(rEvt);
                    return;
                }
            }

            sal_uInt16 nColId = GetColumnId(GetColumnAtXPosPixel( aMenuPos.X() ));
            long   nRow = GetRowAtYPosPixel( aMenuPos.Y() );

            if (nRow < 0 && nColId > HANDLE_ID )
            {
                if ( !IsColumnSelected( nColId ) )
                {
                    adjustSelectionMode( sal_True /* clicked onto a header */ , sal_False /* not onto the handle col */ );
                    SelectColumnId( nColId );
                }

                if (!static_cast<OQueryController&>(getDesignView()->getController()).isReadOnly())
                {
                    PopupMenu aContextMenu( ModuleRes( RID_QUERYCOLPOPUPMENU ) );
                    switch (aContextMenu.Execute(this, aMenuPos))
                    {
                        case SID_DELETE:
                            RemoveField(nColId);
                            break;

                        case ID_BROWSER_COLWIDTH:
                            adjustBrowseBoxColumnWidth( this, nColId );
                            break;
                    }
                }
            }
            else if(nRow >= 0 && nColId <= HANDLE_ID)
            {
                if (!static_cast<OQueryController&>(getDesignView()->getController()).isReadOnly())
                {
                    PopupMenu aContextMenu(ModuleRes(RID_QUERYFUNCTION_POPUPMENU));
                    aContextMenu.CheckItem( ID_QUERY_FUNCTION, m_bVisibleRow[BROW_FUNCTION_ROW]);
                    aContextMenu.CheckItem( ID_QUERY_TABLENAME, m_bVisibleRow[BROW_TABLE_ROW]);
                    aContextMenu.CheckItem( ID_QUERY_ALIASNAME, m_bVisibleRow[BROW_COLUMNALIAS_ROW]);
                    aContextMenu.CheckItem( ID_QUERY_DISTINCT, static_cast<OQueryController&>(getDesignView()->getController()).isDistinct());

                    switch (aContextMenu.Execute(this, aMenuPos))
                    {
                        case ID_QUERY_FUNCTION:
                            SetRowVisible(BROW_FUNCTION_ROW, !IsRowVisible(BROW_FUNCTION_ROW));
                            static_cast<OQueryController&>(getDesignView()->getController()).InvalidateFeature( SID_QUERY_VIEW_FUNCTIONS );
                            break;
                        case ID_QUERY_TABLENAME:
                            SetRowVisible(BROW_TABLE_ROW, !IsRowVisible(BROW_TABLE_ROW));
                            static_cast<OQueryController&>(getDesignView()->getController()).InvalidateFeature( SID_QUERY_VIEW_TABLES );
                            break;
                        case ID_QUERY_ALIASNAME:
                            SetRowVisible(BROW_COLUMNALIAS_ROW, !IsRowVisible(BROW_COLUMNALIAS_ROW));
                            static_cast<OQueryController&>(getDesignView()->getController()).InvalidateFeature( SID_QUERY_VIEW_ALIASES );
                            break;
                        case ID_QUERY_DISTINCT:
                            static_cast<OQueryController&>(getDesignView()->getController()).setDistinct(!static_cast<OQueryController&>(getDesignView()->getController()).isDistinct());
                            static_cast<OQueryController&>(getDesignView()->getController()).setModified( sal_True );
                            static_cast<OQueryController&>(getDesignView()->getController()).InvalidateFeature( SID_QUERY_DISTINCT_VALUES );
                            break;
                    }

                    static_cast<OQueryController&>(getDesignView()->getController()).setModified( sal_True );
                }
            }
            else
            {
                EditBrowseBox::Command(rEvt);
                return;
            }
        }
        default:
            EditBrowseBox::Command(rEvt);
    }
}

sal_Bool OSelectionBrowseBox::IsRowVisible(sal_uInt16 _nWhich) const
{
    OSL_ENSURE(_nWhich<(m_bVisibleRow.size()), "OSelectionBrowseBox::IsRowVisible : invalid parameter !");
    return m_bVisibleRow[_nWhich];
}

void OSelectionBrowseBox::SetRowVisible(sal_uInt16 _nWhich, sal_Bool _bVis)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OSL_ENSURE(_nWhich<m_bVisibleRow.size(), "OSelectionBrowseBox::SetRowVisible : invalid parameter !");

    sal_Bool bWasEditing = IsEditing();
    if (bWasEditing)
        DeactivateCell();

    // do this before removing or inserting rows, as this triggers ActivateCell-calls, which rely on m_bVisibleRow
    m_bVisibleRow[_nWhich] = !m_bVisibleRow[_nWhich];

    long nId = GetBrowseRow(_nWhich);
    if (_bVis)
    {
        RowInserted(nId,1);
        ++m_nVisibleCount;
    }
    else
    {
        RowRemoved(nId,1);
        --m_nVisibleCount;
    }

    if (bWasEditing)
        ActivateCell();
}

long OSelectionBrowseBox::GetBrowseRow(long nRowId) const
{
    sal_uInt16 nCount(0);
    for(sal_uInt16 i = 0 ; i < nRowId ; ++i)
    {
        if ( m_bVisibleRow[i] )
            ++nCount;
    }
    return nCount;
}

long OSelectionBrowseBox::GetRealRow(long nRowId) const
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    long nErg=0,i;
    const long nCount = m_bVisibleRow.size();
    for(i=0;i < nCount; ++i)
    {
        if(m_bVisibleRow[i])
        {
            if(nErg++ == nRowId)
                break;
        }
    }
    OSL_ENSURE(nErg <= long(m_bVisibleRow.size()),"nErg kann nicht groesser als BROW_ROW_CNT sein!");
    return i;
}

static const long nVisibleRowMask[] =
                    {
                            0x0001,
                            0x0002,
                            0x0004,
                            0x0008,
                            0x0010,
                            0x0020,
                            0x0040,
                            0x0080,
                            0x0100,
                            0x0200,
                            0x0400,
                            0x0800
                    };
sal_Int32 OSelectionBrowseBox::GetNoneVisibleRows() const
{
    sal_Int32 nErg(0);
    // only the first 11 row are interesting
    sal_Int32 nSize = sizeof(nVisibleRowMask) / sizeof(nVisibleRowMask[0]);
    for(sal_Int32 i=0;i<nSize;i++)
    {
        if(!m_bVisibleRow[i])
            nErg |= nVisibleRowMask[i];
    }
    return nErg;
}

void OSelectionBrowseBox::SetNoneVisbleRow(long nRows)
{
    // only the first 11 row are interesting
    sal_Int32 nSize = sizeof(nVisibleRowMask) / sizeof(nVisibleRowMask[0]);
    for(sal_Int32 i=0;i< nSize;i++)
        m_bVisibleRow[i] = !(nRows & nVisibleRowMask[i]);
}

OUString OSelectionBrowseBox::GetCellText(long nRow, sal_uInt16 nColId) const
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);

    sal_uInt16 nPos = GetColumnPos(nColId);

    OTableFieldDescRef pEntry = getFields()[nPos-1];
    OSL_ENSURE(pEntry != NULL, "OSelectionBrowseBox::GetCellText : invalid column id, prepare for GPF ... ");
    if ( pEntry->IsEmpty() )
        return OUString();

    String aText;
    switch (nRow)
    {
        case BROW_TABLE_ROW:
            aText = pEntry->GetAlias();
            break;
        case BROW_FIELD_ROW:
        {
            String aField = pEntry->GetField();
            if (aField.GetChar(0) == '*')                   // * durch alias.* ersetzen
            {
                aField = pEntry->GetAlias();
                if(aField.Len())
                    aField += '.';
                aField += '*';
            }
            aText = aField;
        }   break;
        case BROW_ORDER_ROW:
            if (pEntry->GetOrderDir() != ORDER_NONE)
                aText = String(ModuleRes(STR_QUERY_SORTTEXT) ).GetToken(sal::static_int_cast< sal_uInt16 >(pEntry->GetOrderDir()));
            break;
        case BROW_VIS_ROW:
            break;
        case BROW_COLUMNALIAS_ROW:
            aText = pEntry->GetFieldAlias();
            break;
        case BROW_FUNCTION_ROW:
            // we always show the group function at first
            if ( pEntry->IsGroupBy() )
                aText = m_aFunctionStrings.GetToken(comphelper::string::getTokenCount(m_aFunctionStrings, ';')-1);
            else if ( pEntry->isNumericOrAggreateFunction() )
                aText = pEntry->GetFunction();
            break;
        default:
            aText = pEntry->GetCriteria(sal_uInt16(nRow - BROW_CRIT1_ROW));
    }
    return aText;
}

sal_Bool OSelectionBrowseBox::GetFunctionName(sal_uInt32 _nFunctionTokenId, OUString& rFkt)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    sal_Bool bErg=sal_True;
    switch(_nFunctionTokenId)
    {
        case SQL_TOKEN_COUNT:
            rFkt = (m_pFunctionCell->GetEntryCount() < 3) ? m_pFunctionCell->GetEntry(1) : m_pFunctionCell->GetEntry(2);
            break;
        case SQL_TOKEN_AVG:
            rFkt = m_pFunctionCell->GetEntry(1);
            break;
        case SQL_TOKEN_MAX:
            rFkt = m_pFunctionCell->GetEntry(3);
            break;
        case SQL_TOKEN_MIN:
            rFkt = m_pFunctionCell->GetEntry(4);
            break;
        case SQL_TOKEN_SUM:
            rFkt = m_pFunctionCell->GetEntry(5);
            break;
        case SQL_TOKEN_EVERY:
            rFkt = m_pFunctionCell->GetEntry(6);
            break;
        case SQL_TOKEN_ANY:
            rFkt = m_pFunctionCell->GetEntry(7);
            break;
        case SQL_TOKEN_SOME:
            rFkt = m_pFunctionCell->GetEntry(8);
            break;
        case SQL_TOKEN_STDDEV_POP:
            rFkt = m_pFunctionCell->GetEntry(9);
            break;
        case SQL_TOKEN_STDDEV_SAMP:
            rFkt = m_pFunctionCell->GetEntry(10);
            break;
        case SQL_TOKEN_VAR_SAMP:
            rFkt = m_pFunctionCell->GetEntry(11);
            break;
        case SQL_TOKEN_VAR_POP:
            rFkt = m_pFunctionCell->GetEntry(12);
            break;
        case SQL_TOKEN_COLLECT:
            rFkt = m_pFunctionCell->GetEntry(13);
            break;
        case SQL_TOKEN_FUSION:
            rFkt = m_pFunctionCell->GetEntry(14);
            break;
        case SQL_TOKEN_INTERSECTION:
            rFkt = m_pFunctionCell->GetEntry(15);
            break;
        default:
            {
                xub_StrLen nCount = comphelper::string::getTokenCount(m_aFunctionStrings, ';');
                xub_StrLen i;
                for ( i = 0; i < nCount-1; i++) // grouping is not counted
                {
                    if(rFkt.equalsIgnoreAsciiCase(m_aFunctionStrings.GetToken(i)))
                    {
                        rFkt = m_aFunctionStrings.GetToken(i);
                        break;
                    }
                }
                if(i == nCount-1)
                    bErg = sal_False;
            }
    }

    return bErg;
}

String OSelectionBrowseBox::GetCellContents(sal_Int32 nCellIndex, sal_uInt16 nColId)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    if ( GetCurColumnId() == nColId && !m_bInUndoMode )
        SaveModified();

    sal_uInt16 nPos = GetColumnPos(nColId);
    OTableFieldDescRef pEntry = getFields()[nPos - 1];
    OSL_ENSURE(pEntry != NULL, "OSelectionBrowseBox::GetCellContents : invalid column id, prepare for GPF ... ");

    switch (nCellIndex)
    {
        case BROW_VIS_ROW :
            return pEntry->IsVisible() ? g_strOne : g_strZero;
        case BROW_ORDER_ROW:
        {
            sal_uInt16 nIdx = m_pOrderCell->GetSelectEntryPos();
            if (nIdx == sal_uInt16(-1))
                nIdx = 0;
            return OUString(nIdx);
        }
        default:
            return GetCellText(nCellIndex, nColId);
    }
}

void OSelectionBrowseBox::SetCellContents(sal_Int32 nRow, sal_uInt16 nColId, const String& strNewText)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    sal_Bool bWasEditing = IsEditing() && (GetCurColumnId() == nColId) && IsRowVisible(static_cast<sal_uInt16>(nRow)) && (GetCurRow() == static_cast<sal_uInt16>(GetBrowseRow(nRow)));
    if (bWasEditing)
        DeactivateCell();

    sal_uInt16 nPos = GetColumnPos(nColId);
    OTableFieldDescRef pEntry = getEntry(nPos - 1);
    OSL_ENSURE(pEntry != NULL, "OSelectionBrowseBox::SetCellContents : invalid column id, prepare for GPF ... ");

    switch (nRow)
    {
        case BROW_VIS_ROW:
            pEntry->SetVisible(strNewText.Equals(g_strOne));
            break;
        case BROW_FIELD_ROW:
            pEntry->SetField(strNewText);
            break;
        case BROW_TABLE_ROW:
            pEntry->SetAlias(strNewText);
            break;
        case BROW_ORDER_ROW:
        {
            sal_uInt16 nIdx = (sal_uInt16)strNewText.ToInt32();
            pEntry->SetOrderDir(EOrderDir(nIdx));
        }   break;
        case BROW_COLUMNALIAS_ROW:
            pEntry->SetFieldAlias(strNewText);
            break;
        case BROW_FUNCTION_ROW:
        {
            OUString sGroupFunctionName = m_aFunctionStrings.GetToken(comphelper::string::getTokenCount(m_aFunctionStrings, ';')-1);
            pEntry->SetFunction(strNewText);
            // first reset this two member
            sal_Int32 nFunctionType = pEntry->GetFunctionType();
            nFunctionType &= ~FKT_AGGREGATE;
            pEntry->SetFunctionType(nFunctionType);
            if ( pEntry->IsGroupBy() && !sGroupFunctionName.equalsIgnoreAsciiCase(strNewText) )
                pEntry->SetGroupBy(sal_False);

            if ( sGroupFunctionName.equalsIgnoreAsciiCase(strNewText) )
                pEntry->SetGroupBy(sal_True);
            else if ( strNewText.Len() )
            {
                nFunctionType |= FKT_AGGREGATE;
                pEntry->SetFunctionType(nFunctionType);
            }
        }   break;
        default:
            pEntry->SetCriteria(sal_uInt16(nRow - BROW_CRIT1_ROW), strNewText);
    }

    long nCellIndex = GetRealRow(nRow);
    if(IsRowVisible(static_cast<sal_uInt16>(nRow)))
        RowModified(nCellIndex, nColId);

    // the appropriate field-description is now empty -> set Visible to sal_False (now it is consistent to normal empty rows)
    if (pEntry->IsEmpty())
        pEntry->SetVisible(sal_False);

    if (bWasEditing)
        ActivateCell(nCellIndex, nColId);

    static_cast<OQueryController&>(getDesignView()->getController()).setModified( sal_True );
}

sal_uInt32 OSelectionBrowseBox::GetTotalCellWidth(long nRow, sal_uInt16 nColId) const
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);

    long nRowId = GetRealRow(nRow);
    if (nRowId == BROW_VIS_ROW)
        return CHECKBOX_SIZE;
    else
        return  GetDataWindow().GetTextWidth(GetCellText(nRowId, nColId));
}

void OSelectionBrowseBox::ColumnResized(sal_uInt16 nColId)
{
    if (static_cast<OQueryController&>(getDesignView()->getController()).isReadOnly())
        return;
    // The resizing of columns can't be suppressed (BrowseBox doesn't support that) so we have to do this
    // fake. It's not _that_ bad : the user may change column widths while in read-only mode to see all details
    // but the changes aren't permanent ...

    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    sal_uInt16 nPos = GetColumnPos(nColId);
    OSL_ENSURE(nPos <= getFields().size(),"ColumnResized:: nColId sollte nicht groesser als List::count sein!");
    OTableFieldDescRef pEntry = getEntry(nPos-1);
    OSL_ENSURE(pEntry.is(), "OSelectionBrowseBox::ColumnResized : keine FieldDescription !");
    static_cast<OQueryController&>(getDesignView()->getController()).setModified( sal_True );
    EditBrowseBox::ColumnResized(nColId);

    if ( pEntry.is())
    {
        if ( !m_bInUndoMode )
        {
            // create the undo action
            OTabFieldSizedUndoAct* pUndo = new OTabFieldSizedUndoAct(this);
            pUndo->SetColumnPosition( nPos );
            pUndo->SetOriginalWidth(pEntry->GetColWidth());
            getDesignView()->getController().addUndoActionAndInvalidate(pUndo);
        }
        pEntry->SetColWidth(sal_uInt16(GetColumnWidth(nColId)));
    }
}

sal_uInt32 OSelectionBrowseBox::GetTotalCellWidth(long nRowId, sal_uInt16 nColId)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    sal_uInt16 nPos = GetColumnPos(nColId);
    OSL_ENSURE((nPos == 0) || (nPos <= getFields().size()), "OSelectionBrowseBox::GetTotalCellWidth : invalid parameter nColId");

    OTableFieldDescRef pEntry = getFields()[nPos-1];
    OSL_ENSURE(pEntry.is(), "OSelectionBrowseBox::GetTotalCellWidth : invalid FieldDescription !");

    long nRow = GetRealRow(nRowId);
    String strText(GetCellText(nRow, nColId));
    return GetDataWindow().LogicToPixel(Size(GetDataWindow().GetTextWidth(strText),0)).Width();
}

sal_uInt16 OSelectionBrowseBox::GetDefaultColumnWidth(const String& /*rName*/) const
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    // the base class makes it dependent on the text. I have no column headers, therefore I
    // like to have a new Default-value
    return static_cast<sal_uInt16>(DEFAULT_SIZE);
}

sal_Bool OSelectionBrowseBox::isCutAllowed()
{
    sal_Bool bCutAllowed = sal_False;
    long nRow = GetRealRow(GetCurRow());
    switch (nRow)
    {
        case BROW_VIS_ROW:
        case BROW_ORDER_ROW:
        case BROW_TABLE_ROW:
        case BROW_FUNCTION_ROW:
            break;
        case BROW_FIELD_ROW:
            bCutAllowed = !m_pFieldCell->GetSelected().isEmpty();
            break;
        default:
            bCutAllowed = !m_pTextCell->GetSelected().isEmpty();
            break;
    }
    return bCutAllowed;
}

void OSelectionBrowseBox::cut()
{
    long nRow = GetRealRow(GetCurRow());
    switch (nRow)
    {
        case BROW_FIELD_ROW:
            m_pFieldCell->Cut();
            m_pFieldCell->SetModifyFlag();
            break;
        default:
            m_pTextCell->Cut();
            m_pTextCell->SetModifyFlag();
    }
    SaveModified();
    RowModified(GetBrowseRow(nRow), GetCurColumnId());

    invalidateUndoRedo();
}

void OSelectionBrowseBox::paste()
{
    long nRow = GetRealRow(GetCurRow());
    switch (nRow)
    {
        case BROW_FIELD_ROW:
            m_pFieldCell->Paste();
            m_pFieldCell->SetModifyFlag();
            break;
        default:
            m_pTextCell->Paste();
            m_pTextCell->SetModifyFlag();
    }
    RowModified(GetBrowseRow(nRow), GetCurColumnId());
    invalidateUndoRedo();
}

sal_Bool OSelectionBrowseBox::isPasteAllowed()
{
    sal_Bool bPasteAllowed = sal_True;
    long nRow = GetRealRow(GetCurRow());
    switch (nRow)
    {
        case BROW_VIS_ROW:
        case BROW_ORDER_ROW:
        case BROW_TABLE_ROW:
        case BROW_FUNCTION_ROW:
            bPasteAllowed = sal_False;
            break;
    }
    return bPasteAllowed;
}

sal_Bool OSelectionBrowseBox::isCopyAllowed()
{
    return isCutAllowed();
}

void OSelectionBrowseBox::copy()
{
    long nRow = GetRealRow(GetCurRow());
    switch (nRow)
    {
        case BROW_FIELD_ROW:
            m_pFieldCell->Copy();
            break;
        default:
            m_pTextCell->Copy();
    }
}

void OSelectionBrowseBox::appendUndoAction(const String& _rOldValue,const String& _rNewValue,sal_Int32 _nRow,sal_Bool& _bListAction)
{
    if ( !m_bInUndoMode && !_rNewValue.Equals(_rOldValue) )
    {
        if ( !_bListAction )
        {
            _bListAction = sal_True;
            static_cast<OQueryController&>(getDesignView()->getController()).GetUndoManager().EnterListAction(String(),String());
        }
        appendUndoAction(_rOldValue,_rNewValue,_nRow);
    }
}

void OSelectionBrowseBox::appendUndoAction(const String& _rOldValue,const String& _rNewValue,sal_Int32 _nRow)
{
    if ( !m_bInUndoMode && !_rNewValue.Equals(_rOldValue) )
    {
        OTabFieldCellModifiedUndoAct* pUndoAct = new OTabFieldCellModifiedUndoAct(this);
        pUndoAct->SetCellIndex(_nRow);
        OSL_ENSURE(GetColumnPos(GetCurColumnId()) != BROWSER_INVALIDID,"Current position isn't valid!");
        pUndoAct->SetColumnPosition( GetColumnPos(GetCurColumnId()) );
        pUndoAct->SetCellContents(_rOldValue);
        getDesignView()->getController().addUndoActionAndInvalidate(pUndoAct);
    }
}

IMPL_LINK_NOARG(OSelectionBrowseBox, OnInvalidateTimer)
{
    static_cast<OQueryController&>(getDesignView()->getController()).InvalidateFeature(SID_CUT);
    static_cast<OQueryController&>(getDesignView()->getController()).InvalidateFeature(SID_COPY);
    static_cast<OQueryController&>(getDesignView()->getController()).InvalidateFeature(SID_PASTE);
    if(!m_bStopTimer)
        m_timerInvalidate.Start();
    return 0L;
}

void OSelectionBrowseBox::stopTimer()
{
    m_bStopTimer = sal_True;
    if (m_timerInvalidate.IsActive())
        m_timerInvalidate.Stop();
}

void OSelectionBrowseBox::startTimer()
{
    m_bStopTimer = sal_False;
    if (!m_timerInvalidate.IsActive())
        m_timerInvalidate.Start();
}

OTableFields& OSelectionBrowseBox::getFields() const
{
    OQueryController& rController = static_cast<OQueryController&>(getDesignView()->getController());
    return rController.getTableFieldDesc();
}

void OSelectionBrowseBox::enableControl(const OTableFieldDescRef& _rEntry,Window* _pControl)
{
    sal_Bool bEnable = !_rEntry->isCondition();
    _pControl->Enable(bEnable);
    _pControl->EnableInput(bEnable);
}

void OSelectionBrowseBox::setTextCellContext(const OTableFieldDescRef& _rEntry,const String& _sText,const OString& _sHelpId)
{
    m_pTextCell->SetText(_sText);
    m_pTextCell->ClearModifyFlag();
    if (!m_pTextCell->HasFocus())
        m_pTextCell->GrabFocus();

    enableControl(_rEntry,m_pTextCell);

    if (m_pTextCell->GetHelpId() != _sHelpId)
        // as TextCell is used in various contexts I will delete the cached HelpText
        m_pTextCell->SetHelpText(String());
    m_pTextCell->SetHelpId(_sHelpId);
}

void OSelectionBrowseBox::invalidateUndoRedo()
{
    OQueryController& rController = static_cast<OQueryController&>(getDesignView()->getController());
    rController.InvalidateFeature( ID_BROWSER_UNDO );
    rController.InvalidateFeature( ID_BROWSER_REDO );
    rController.InvalidateFeature( ID_BROWSER_QUERY_EXECUTE );
}

OTableFieldDescRef OSelectionBrowseBox::getEntry(OTableFields::size_type _nPos)
{
    // we have to check if we need a new entry at this position
    OTableFields& aFields = getFields();
    OSL_ENSURE(aFields.size() > _nPos,"ColID is to great!");

    OTableFieldDescRef pEntry = aFields[_nPos];
    OSL_ENSURE(pEntry.is(),"Invalid entry!");
    if ( !pEntry.is() )
    {
        pEntry = new OTableFieldDesc();
        pEntry->SetColumnId(
            GetColumnId(sal::static_int_cast< sal_uInt16 >(_nPos+1)));
        aFields[_nPos] = pEntry;
    }
    return pEntry;
}

void OSelectionBrowseBox::GetFocus()
{
    if(!IsEditing() && !m_bWasEditing)
        ActivateCell();
    EditBrowseBox::GetFocus();
}

void OSelectionBrowseBox::DeactivateCell(sal_Bool _bUpdate)
{
    m_bWasEditing = sal_True;
    EditBrowseBox::DeactivateCell(_bUpdate);
    m_bWasEditing = sal_False;
}

OUString OSelectionBrowseBox::GetRowDescription( sal_Int32 _nRow ) const
{
    String  aLabel(ModuleRes(STR_QUERY_HANDLETEXT));

    // from BROW_CRIT2_ROW onwards all rows are shown as "or"
    xub_StrLen nToken = (xub_StrLen) (_nRow >= GetBrowseRow(BROW_CRIT2_ROW))
                                ?
            xub_StrLen(BROW_CRIT2_ROW) : xub_StrLen(GetRealRow(_nRow));
    return OUString(aLabel.GetToken(nToken));
}

OUString OSelectionBrowseBox::GetAccessibleObjectName( ::svt::AccessibleBrowseBoxObjType _eObjType,sal_Int32 _nPosition) const
{
    OUString sRetText;
    switch( _eObjType )
    {
        case ::svt::BBTYPE_ROWHEADERCELL:
            sRetText = GetRowDescription(_nPosition);
            break;
        default:
            sRetText = EditBrowseBox::GetAccessibleObjectDescription(_eObjType,_nPosition);
    }
    return sRetText;
}

sal_Bool OSelectionBrowseBox::fillEntryTable(OTableFieldDescRef& _pEntry,const OUString& _sTableName)
{
    sal_Bool bRet = sal_False;
    OJoinTableView::OTableWindowMap* pTabWinList = getDesignView()->getTableView()->GetTabWinMap();
    if (pTabWinList)
    {
        OJoinTableView::OTableWindowMapIterator aIter = pTabWinList->find(_sTableName);
        if(aIter != pTabWinList->end())
        {
            OQueryTableWindow* pEntryTab = static_cast<OQueryTableWindow*>(aIter->second);
            if (pEntryTab)
            {
                _pEntry->SetTable(pEntryTab->GetTableName());
                _pEntry->SetTabWindow(pEntryTab);
                bRet = sal_True;
            }
        }
    }
    return bRet;
}

void OSelectionBrowseBox::setFunctionCell(OTableFieldDescRef& _pEntry)
{
    Reference< XConnection> xConnection = static_cast<OQueryController&>(getDesignView()->getController()).getConnection();
    if ( xConnection.is() )
    {
        // Aggregate functions in general only available with Core SQL
        if ( lcl_SupportsCoreSQLGrammar(xConnection) )
        {
            // if we have an asterix, no other function than count is allowed
            m_pFunctionCell->Clear();
            m_pFunctionCell->InsertEntry(m_aFunctionStrings.GetToken(0));
            if ( isFieldNameAsterix(_pEntry->GetField()) )
                m_pFunctionCell->InsertEntry(m_aFunctionStrings.GetToken(2)); // 2 -> COUNT
            else
            {
                xub_StrLen nCount = comphelper::string::getTokenCount(m_aFunctionStrings, ';');
                if ( _pEntry->isNumeric() )
                    --nCount;
                for (xub_StrLen nIdx = 1; nIdx < nCount; nIdx++)
                    m_pFunctionCell->InsertEntry(m_aFunctionStrings.GetToken(nIdx));
            }

            if ( _pEntry->IsGroupBy() )
            {
                OSL_ENSURE(!_pEntry->isNumeric(),"Not allowed to combine group by and numeric values!");
                m_pFunctionCell->SelectEntry(m_pFunctionCell->GetEntry(m_pFunctionCell->GetEntryCount() - 1));
            }
            else if ( m_pFunctionCell->GetEntryPos(String(_pEntry->GetFunction())) != COMBOBOX_ENTRY_NOTFOUND )
                m_pFunctionCell->SelectEntry(String(_pEntry->GetFunction()));
            else
                m_pFunctionCell->SelectEntryPos(0);

            enableControl(_pEntry,m_pFunctionCell);
        }
        else
        {
            // only COUNT(*) and COUNT("table".*) allowed
            sal_Bool bCountRemoved = !isFieldNameAsterix(_pEntry->GetField());
            if ( bCountRemoved )
                m_pFunctionCell->RemoveEntry(1);

            if ( !bCountRemoved && m_pFunctionCell->GetEntryCount() < 2)
                m_pFunctionCell->InsertEntry(m_aFunctionStrings.GetToken(2)); // 2 -> COUNT

            if(m_pFunctionCell->GetEntryPos(String(_pEntry->GetFunction())) != COMBOBOX_ENTRY_NOTFOUND)
                m_pFunctionCell->SelectEntry(_pEntry->GetFunction());
            else
                m_pFunctionCell->SelectEntryPos(0);
        }
    }
}

Reference< XAccessible > OSelectionBrowseBox::CreateAccessibleCell( sal_Int32 _nRow, sal_uInt16 _nColumnPos )
{
    OTableFieldDescRef pEntry = NULL;
    if(getFields().size() > sal_uInt16(_nColumnPos - 1))
        pEntry = getFields()[_nColumnPos - 1];

    if ( _nRow == BROW_VIS_ROW && pEntry.is() )
        return EditBrowseBox::CreateAccessibleCheckBoxCell( _nRow, _nColumnPos,pEntry->IsVisible() ? STATE_CHECK : STATE_NOCHECK );

    return EditBrowseBox::CreateAccessibleCell( _nRow, _nColumnPos );
}

bool OSelectionBrowseBox::HasFieldByAliasName(const OUString& rFieldName, OTableFieldDescRef& rInfo) const
{
    OTableFields& aFields = getFields();
    OTableFields::iterator aIter = aFields.begin();
    OTableFields::iterator aEnd  = aFields.end();

    for(;aIter != aEnd;++aIter)
    {
        if ( (*aIter)->GetFieldAlias() == rFieldName )
        {
            *rInfo = *(*aIter);
            break;
        }
    }
    return aIter != aEnd;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
