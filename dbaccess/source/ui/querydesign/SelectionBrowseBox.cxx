/*************************************************************************
 *
 *  $RCSfile: SelectionBrowseBox.cxx,v $
 *
 *  $Revision: 1.51 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-18 15:05:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBAUI_QUERYDESIGN_OSELECTIONBROWSEBOX_HXX
#include "SelectionBrowseBox.hxx"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef DBAUI_QUERYDESIGNVIEW_HXX
#include "QueryDesignView.hxx"
#endif
#ifndef DBAUI_QUERYCONTROLLER_HXX
#include "querycontroller.hxx"
#endif
#ifndef DBAUI_QUERYTABLEVIEW_HXX
#include "QueryTableView.hxx"
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef DBAUI_TABLEFIELDINFO_HXX
#include "TableFieldInfo.hxx"
#endif
#ifndef _DBU_QRY_HRC_
#include "dbu_qry.hrc"
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_SYSTEM_HXX
#include <vcl/system.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef DBAUI_QUERY_TABLEWINDOW_HXX
#include "QTableWindow.hxx"
#endif
#ifndef DBAUI_QUERYTABLEVIEW_HXX
#include "QueryTableView.hxx"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#include <rope>
#ifndef DBAUI_QUERYDESIGNFIELDUNDOACT_HXX
#include "QueryDesignFieldUndoAct.hxx"
#endif
#ifndef _SVX_DBEXCH_HRC
#include <svx/dbexch.hrc>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif

using namespace ::svt;
using namespace ::dbaui;
using namespace ::connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::accessibility;

const String g_strOne = String::CreateFromAscii("1");
const String g_strZero = String::CreateFromAscii("0");

#define DEFAULT_QUERY_COLS  20
#define DEFAULT_SIZE        GetTextWidth(g_strZero) * 30
#define CHECKBOX_SIZE       10
#define HANDLE_ID            0
#define HANDLE_COLUMN_WITDH 70

#define SQL_ISRULEOR2(pParseNode, e1,e2)    ((pParseNode)->isRule() && (\
                                            (pParseNode)->getRuleID() == OSQLParser::RuleID(OSQLParseNode::e1) || \
                                            (pParseNode)->getRuleID() == OSQLParser::RuleID(OSQLParseNode::e2)))


// -----------------------------------------------------------------------------
namespace
{
    sal_Bool isFieldNameAsterix(const ::rtl::OUString& _sFieldName )
    {
        sal_Bool bAsterix;
        if ( !(bAsterix = !(_sFieldName.getLength() && _sFieldName.toChar() != '*')) )
        {
            String sName = _sFieldName;
            xub_StrLen nTokenCount = sName.GetTokenCount('.');
            if (    (nTokenCount == 2 && sName.GetToken(1,'.').GetChar(0) == '*' )
                ||  (nTokenCount == 3 && sName.GetToken(2,'.').GetChar(0) == '*' ) )
            {
                bAsterix = sal_True;
            }
        }
        return bAsterix;
    }
}


DBG_NAME(OSelectionBrowseBox);
//------------------------------------------------------------------------------
OSelectionBrowseBox::OSelectionBrowseBox( Window* pParent )
                   :EditBrowseBox( pParent,EBBF_NOROWPICTURE, WB_3DLOOK, BROWSER_COLUMNSELECTION | BROWSER_KEEPSELECTION |  BROWSER_HIDESELECT |
                                  BROWSER_HIDECURSOR | BROWSER_HLINESFULL | BROWSER_VLINESFULL )
                   ,m_nVisibleCount(0)
                   ,m_aFunctionStrings(ModuleRes(STR_QUERY_FUNCTIONS))
                   ,m_bOrderByUnRelated(sal_True)
                   ,m_bGroupByUnRelated(sal_True)
                   ,m_bStopTimer(sal_False)
                   ,m_bWasEditing(sal_False)
{
    DBG_CTOR(OSelectionBrowseBox,NULL);
    SetHelpId(HID_CTL_QRYDGNCRIT);

    m_nMode =       BROWSER_COLUMNSELECTION | BROWSER_HIDESELECT
                |   BROWSER_KEEPSELECTION   | BROWSER_HIDECURSOR
                |   BROWSER_HLINESFULL      | BROWSER_VLINESFULL
                |   BROWSER_HEADERBAR_NEW   ;

    m_pTextCell     = new Edit(&GetDataWindow(), 0);
    //  m_pTextCell->EnableSpecialCheck(sal_False);
    m_pVisibleCell  = new CheckBoxControl(&GetDataWindow());
    m_pTableCell    = new ListBoxControl(&GetDataWindow());
    m_pFieldCell    = new ComboBoxControl(&GetDataWindow());
    m_pOrderCell    = new ListBoxControl(&GetDataWindow());
    m_pFunctionCell = new ListBoxControl(&GetDataWindow());

    m_pVisibleCell->SetHelpId(HID_QRYDGN_ROW_VISIBLE);
    m_pTableCell->SetHelpId(HID_QRYDGN_ROW_TABLE);
    m_pFieldCell->SetHelpId(HID_QRYDGN_ROW_FIELD);
    m_pOrderCell->SetHelpId(HID_QRYDGN_ROW_ORDER);
    m_pFunctionCell->SetHelpId(HID_QRYDGN_ROW_FUNCTION);

    //////////////////////////////////////////////////////////////////////
    // TriState der ::com::sun::star::form::CheckBox abschalten
    m_pVisibleCell->GetBox().EnableTriState( sal_False );

//  m_pEmptyEntry = new OTableFieldDesc();
//  m_pEmptyEntry->SetColWidth(DEFAULT_SIZE);

    Font aTitleFont = OutputDevice::GetDefaultFont( DEFAULTFONT_SANS_UNICODE,Window::GetSettings().GetLanguage(),DEFAULTFONT_FLAGS_ONLYONE);
    aTitleFont.SetSize(Size(0, 6));
    SetTitleFont(aTitleFont);

    String aTxt(ModuleRes(STR_QUERY_SORTTEXT));
    xub_StrLen nCount = aTxt.GetTokenCount();
    xub_StrLen nIdx = 0;
    for (; nIdx < nCount; nIdx++)
        m_pOrderCell->InsertEntry(aTxt.GetToken(nIdx));

    for(long i=0;i < BROW_ROW_CNT;i++)
        m_bVisibleRow.push_back(sal_True);

    m_bVisibleRow[BROW_FUNCTION_ROW] = sal_False;   // zuerst ausblenden

    m_timerInvalidate.SetTimeout(200);
    m_timerInvalidate.SetTimeoutHdl(LINK(this, OSelectionBrowseBox, OnInvalidateTimer));
    m_timerInvalidate.Start();
}

//------------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
void OSelectionBrowseBox::initialize()
{
    Reference< XConnection> xConnection = static_cast<OQueryController*>(getDesignView()->getController())->getConnection();
    try
    {
        if(xConnection.is())
        {

            Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
            // Diese Funktionen stehen nur unter CORE zur Verf¸gung
            if(xMetaData->supportsCoreSQLGrammar())
            {
                xub_StrLen nCount   = m_aFunctionStrings.GetTokenCount();
                for (xub_StrLen nIdx = 0; nIdx < nCount; nIdx++)
                    m_pFunctionCell->InsertEntry(m_aFunctionStrings.GetToken(nIdx));
            }
            else // sonst nur COUNT(*)
            {
                m_pFunctionCell->InsertEntry(m_aFunctionStrings.GetToken(0));
                m_pFunctionCell->InsertEntry(m_aFunctionStrings.GetToken(2)); // 2 -> COUNT
            }
            m_bOrderByUnRelated = xMetaData->supportsOrderByUnrelated();
            m_bGroupByUnRelated = xMetaData->supportsGroupByUnrelated();
        }
    }
    catch(const SQLException&)
    {
        OSL_ENSURE(0,"Catched Exception when asking for database metadata options!");
    }

    Init();
}
//==============================================================================
OQueryDesignView* OSelectionBrowseBox::getDesignView()
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OSL_ENSURE(static_cast<const OQueryDesignView*>(GetParent()),"Parent isn't an OQueryDesignView!");
    return static_cast<OQueryDesignView*>(GetParent());
}
// -----------------------------------------------------------------------------
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
            // wenn es schon eine selektierte Spalte gibt, bin ich schon im richtigen Modus
            if ( BROWSER_HIDESELECT == ( nMode & BROWSER_HIDESELECT ) )
            {
                nMode &= ~BROWSER_HIDESELECT;
                nMode |= BROWSER_MULTISELECTION;
                m_pBrowseBox->SetMode( nMode );
            }
        }
        m_pBrowseBox->SelectColumnId( GetCurItemId() );
    }
}

// -----------------------------------------------------------------------------
BrowserHeader* OSelectionBrowseBox::imp_CreateHeaderBar(BrowseBox* pParent)
{
    return new OSelectionBrwBoxHeader(this);
}
// -----------------------------------------------------------------------------
void OSelectionBrowseBox::ColumnMoved( USHORT nColId,BOOL _bCreateUndo )
{
    EditBrowseBox::ColumnMoved( nColId );
    // swap the two columns
    USHORT nNewPos = GetColumnPos( nColId );
    OTableFields& rFields = getFields();
    if ( rFields.size() > USHORT(nNewPos-1) )
    {
        USHORT nOldPos = 0;
        OTableFields::iterator aEnd = rFields.end();
        OTableFields::iterator aIter = rFields.begin();
        for (; aIter != aEnd && ( (*aIter)->GetColumnId() != nColId ); ++aIter,++nOldPos)
            ;

        OSL_ENSURE( (nNewPos-1) != nOldPos,"Old and new position are equal!");
        OTableFieldDescRef pOldEntry = rFields[nOldPos];
        rFields.erase(rFields.begin() + nOldPos);
        rFields.insert(rFields.begin() + nNewPos - 1,pOldEntry);

        // create the undo action
        if ( _bCreateUndo )
        {
            OTabFieldMovedUndoAct* pUndoAct = new OTabFieldMovedUndoAct(this);
            pUndoAct->SetColumnPosition( nOldPos + 1);
            pUndoAct->SetTabFieldDescr(pOldEntry);

            getDesignView()->getController()->addUndoActionAndInvalidate(pUndoAct);
        }
    }
    else
        OSL_ENSURE(0,"Invalid column id!");
}
//------------------------------------------------------------------------------
void OSelectionBrowseBox::Init()
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);

    EditBrowseBox::Init();

    // set the header bar
    BrowserHeader* pHeader = CreateHeaderBar(this);
    pHeader->SetMouseTransparent(sal_False);

    SetHeaderBar(pHeader);
    SetMode(m_nMode);


    SetMapMode( MapMode(MAP_TWIP) );
    GetDataWindow().SetMapMode( GetMapMode() );

    Font    aFont( GetDataWindow().GetFont() );
    aFont.SetWeight( WEIGHT_NORMAL );
    GetDataWindow().SetFont( aFont );

    //xxx richtige Zeilenhoehe fuer EditEng ???
    long nLSize = GetDataWindow().GetTextHeight() + 4;
    SetDataRowHeight(nLSize);
    SetTitleLines(1);
    // Anzahl der sichtbaren Zeilen ermitteln
    for(long i=0;i<BROW_ROW_CNT;i++)
    {
        if(m_bVisibleRow[i])
            m_nVisibleCount++;
    }
    RowInserted(0, m_nVisibleCount, sal_False);
    try
    {
        Reference< XConnection> xConnection = static_cast<OQueryController*>(getDesignView()->getController())->getConnection();
        if(xConnection.is())
        {
            Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
            m_nMaxColumns = xMetaData->getMaxColumnsInSelect();

        }
        else
            m_nMaxColumns = 0;
    }
    catch(const SQLException&)
    {
        OSL_ENSURE(0,"Catched Exception when asking for database metadata options!");
        m_nMaxColumns = 0;
    }
}

//------------------------------------------------------------------------------
void OSelectionBrowseBox::PreFill()
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    SetUpdateMode(sal_False);

    if (GetCurRow() != 0)
        GoToRow(0);


    OQueryController* pController = static_cast<OQueryController*>(static_cast<OQueryController*>(getDesignView()->getController()));

    pController->clearFields();

    sal_uInt16 nCurCol  = GetCurColumnId();
    long nCurRow        = GetCurRow();
    DeactivateCell();

    RemoveColumns();
    InsertHandleColumn( HANDLE_COLUMN_WITDH );
    SetUpdateMode(sal_True);
}
//------------------------------------------------------------------------------
void OSelectionBrowseBox::ClearAll()
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    SetUpdateMode(sal_False);

    OQueryController* pController = static_cast<OQueryController*>(static_cast<OQueryController*>(getDesignView()->getController()));

    OTableFields::reverse_iterator aIter = getFields().rbegin();
    for(sal_uInt16 nId=getFields().size();aIter != getFields().rend();++aIter,--nId)
    {
        if(!(*aIter)->IsEmpty())
            RemoveField(GetColumnId(nId));
    }
    SetUpdateMode(sal_True);
}
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
CellController* OSelectionBrowseBox::GetController(long nRow, sal_uInt16 nColId)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OQueryController* pController = static_cast<OQueryController*>(static_cast<OQueryController*>(getDesignView()->getController()));

    OTableFieldDescRef pEntry = getFields()[nColId-1];
    DBG_ASSERT(pEntry.isValid(), "OSelectionBrowseBox::GetController : keine FieldDescription !");

    if (!pEntry.isValid())
        return NULL;

    if (static_cast<OQueryController*>(getDesignView()->getController())->isReadOnly())
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

//------------------------------------------------------------------------------
void OSelectionBrowseBox::InitController(CellControllerRef& rController, long nRow, sal_uInt16 nColId)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OQueryController* pController = static_cast<OQueryController*>(static_cast<OQueryController*>(getDesignView()->getController()));


    OTableFieldDescRef pEntry = getFields()[GetColumnPos(nColId)-1];
    DBG_ASSERT(pEntry.isValid(), "OSelectionBrowseBox::InitController : keine FieldDescription !");
    long nCellIndex = GetRealRow(nRow);

    switch (nCellIndex)
    {
        case BROW_FIELD_ROW:
        {
            m_pFieldCell->Clear();
            m_pFieldCell->SetText(String());

            String aField(pEntry->GetField());
            String aTable(pEntry->GetAlias());

            getDesignView()->fillValidFields(aTable, m_pFieldCell);

            // * durch alias.* ersetzen
            if ((aField.GetChar(0) == '*') && aTable.Len())
            {
                aField = aTable;
                aField.AppendAscii(".*");
            }
            m_pFieldCell->SetText(aField);
        }   break;
        case BROW_TABLE_ROW:
        {
            m_pTableCell->Clear();
            enableControl(pEntry,m_pTableCell);
            if ( !pEntry->isCondition() && !pEntry->isNumeric() )
            {
                OJoinTableView::OTableWindowMap* pTabWinList = getDesignView()->getTableView()->GetTabWinMap();
                if (pTabWinList)
                {
                    OJoinTableView::OTableWindowMap::iterator aIter = pTabWinList->begin();
                    for(;aIter != pTabWinList->end();++aIter)
                        m_pTableCell->InsertEntry(static_cast<OQueryTableWindow*>(aIter->second)->GetAliasName());

                    m_pTableCell->InsertEntry(String(ModuleRes(STR_QUERY_NOTABLE)), 0);
                    if (pEntry->GetAlias().getLength())
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
                // Spalte muss sichtbar sein, um im ORDER BY aufzutauchen
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
            m_pOrderCell->SelectEntryPos(pEntry->GetOrderDir());
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
            sal_uInt16  nIdx = nCellIndex-BROW_CRIT1_ROW;
            setTextCellContext(pEntry,pEntry->GetCriteria( nIdx ),HID_QRYDGN_ROW_CRIT);
        }
    }
    Controller()->ClearModified();
}
// -----------------------------------------------------------------------------
void OSelectionBrowseBox::notifyTableFieldChanged(const String& _sOldAlias,const String& _sAlias,sal_Bool& _bListAction,USHORT _nColumnId)
{
    appendUndoAction(_sOldAlias,_sAlias,BROW_TABLE_ROW,_bListAction);
    if ( m_bVisibleRow[BROW_TABLE_ROW] )
        RowModified(GetBrowseRow(BROW_TABLE_ROW), _nColumnId);
}
// -----------------------------------------------------------------------------
void OSelectionBrowseBox::notifyFunctionFieldChanged(const String& _sOldFunctionName,const String& _sFunctionName,sal_Bool& _bListAction,USHORT _nColumnId)
{
    appendUndoAction(_sOldFunctionName,_sFunctionName,BROW_FUNCTION_ROW,_bListAction);
    if ( !m_bVisibleRow[BROW_FUNCTION_ROW] )
        SetRowVisible(BROW_FUNCTION_ROW, sal_True);
    RowModified(GetBrowseRow(BROW_FUNCTION_ROW), _nColumnId);
}
// -----------------------------------------------------------------------------
void OSelectionBrowseBox::clearEntryFunctionField(const String& _sFieldName,OTableFieldDescRef& _pEntry,sal_Bool& _bListAction,USHORT _nColumnId)
{
    if ( isFieldNameAsterix( _sFieldName ) && (!_pEntry->isNoneFunction() || _pEntry->IsGroupBy()) )
    {
        String sFunctionName;
        GetFunctionName(SQL_TOKEN_COUNT,sFunctionName);
        String sOldLocalizedFunctionName = _pEntry->GetFunction();
        if ( !sOldLocalizedFunctionName.Equals(sFunctionName) || _pEntry->IsGroupBy() )
        {
            // append undo action for the function field
            ::rtl::OUString sOldLocalizedFunctionName = _pEntry->GetFunction();
            _pEntry->SetFunctionType(FKT_NONE);
            _pEntry->SetFunction(::rtl::OUString());
            _pEntry->SetGroupBy(sal_False);
            notifyFunctionFieldChanged(sOldLocalizedFunctionName,_pEntry->GetFunction(),_bListAction,_nColumnId);
        }
    }
}
// -----------------------------------------------------------------------------
sal_Bool OSelectionBrowseBox::fillColumnRef(const OSQLParseNode* _pColumnRef,const Reference<XDatabaseMetaData>& _xMetaData,OTableFieldDescRef& _pEntry,sal_Bool& _bListAction)
{
    OSL_ENSURE(_pColumnRef,"No valid parsenode!");
    ::rtl::OUString sColumnName,sTableRange;
    OSQLParseTreeIterator::getColumnRange(_pColumnRef,_xMetaData,sColumnName,sTableRange);
    return fillColumnRef(sColumnName,sTableRange,_xMetaData,_pEntry,_bListAction);
}
// -----------------------------------------------------------------------------
sal_Bool OSelectionBrowseBox::fillColumnRef(const ::rtl::OUString& _sColumnName,const ::rtl::OUString& _sTableRange,const Reference<XDatabaseMetaData>& _xMetaData,OTableFieldDescRef& _pEntry,sal_Bool& _bListAction)
{
    sal_Bool bError = sal_False;
    ::comphelper::UStringMixEqual bCase(_xMetaData->storesMixedCaseQuotedIdentifiers());
    // check if the table name is the same
    if ( _sTableRange.getLength() && (bCase(_pEntry->GetTable(),_sTableRange) || bCase(_pEntry->GetAlias(),_sTableRange)) )
    { // a table was already inserted and the tables contains that column name

        if ( !_pEntry->GetTabWindow() )
        { // fill tab window
            ::rtl::OUString sOldAlias = _pEntry->GetAlias();
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
            if ( !static_cast<OQueryTableView*>(getDesignView()->getTableView())->FindTableFromField(_sColumnName,_pEntry,nTabCount) ) // error occured: column not in table window
            {
                String sTitle(ModuleRes(STR_STAT_WARNING));
                String sErrorMsg(ModuleRes(RID_STR_FIELD_DOESNT_EXIST));
                sErrorMsg.SearchAndReplaceAscii("$name$",_sColumnName);
                OSQLMessageBox aDlg(this,sTitle,sErrorMsg,WB_OK | WB_DEF_OK,OSQLMessageBox::Warning);
                aDlg.Execute();
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
// -----------------------------------------------------------------------------
sal_Bool OSelectionBrowseBox::saveField(const String& _sFieldName,OTableFieldDescRef& _pEntry,sal_Bool& _bListAction)
{
    sal_Bool bError = sal_False;

    OQueryController* pController = static_cast<OQueryController*>(static_cast<OQueryController*>(getDesignView()->getController()));

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

    ::rtl::OUString sErrorMsg;
    // second test if the name can be set as select columns in a pseudo statement
    // we have to look which entries  we should quote

    ::rtl::OUString sSql;
    sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SELECT "));
    sSql += _sFieldName;
    sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" FROM x"));

    ::connectivity::OSQLParser* pParser = pController->getParser();
    OSQLParseNode* pParseNode = pParser->parseTree(sErrorMsg, sSql,sal_False);

    if ( pParseNode ) // we got a valid select column
    {
        // find what type of column has be inserted
        ::connectivity::OSQLParseNode* pSelection = pParseNode->getChild(2);
        if ( SQL_ISRULE(pSelection,selection) ) // we found the asterix
        {
            _pEntry->SetField(_sFieldName);
            clearEntryFunctionField(_sFieldName,_pEntry,_bListAction,_pEntry->GetColumnId());
        } // travel through the select column parse node
        else
        {
            Reference<XConnection> xConnection = pController->getConnection();
            if ( !xConnection.is() )
                return sal_True;
            Reference<XDatabaseMetaData> xMetaData = xConnection->getMetaData();
            ::comphelper::UStringMixEqual bCase(xMetaData->storesMixedCaseQuotedIdentifiers());

            OTableFieldDescRef aSelEntry = _pEntry;
            USHORT nColumnId = aSelEntry->GetColumnId();

            sal_uInt32 nCount = pSelection->count();
            for (sal_uInt32 i = 0; i < nCount; ++i)
            {
                if ( i > 0 ) // may we have to append more than one field
                {
                    USHORT nColumnPostion;
                    aSelEntry = FindFirstFreeCol(nColumnPostion);
                    if ( !aSelEntry.isValid() )
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
                ::rtl::OUString sColumnAlias = OSQLParseTreeIterator::getColumnAlias(pChild);
                if ( sColumnAlias.getLength() ) // we found an as clause
                {
                    String sOldAlias = aSelEntry->GetFieldAlias();
                    aSelEntry->SetFieldAlias( sColumnAlias );
                    // append undo
                    appendUndoAction(sOldAlias,aSelEntry->GetFieldAlias(),BROW_COLUMNALIAS_ROW,_bListAction);
                    if ( m_bVisibleRow[BROW_COLUMNALIAS_ROW] )
                        RowModified(GetBrowseRow(BROW_COLUMNALIAS_ROW), nColumnId);
                }

                ::connectivity::OSQLParseNode* pColumnRef = pChild->getChild(0);
                if ( SQL_ISRULE(pColumnRef,column_ref) ) // we found a valid column name or more column names
                {
                    // look if we can find the corresponding table
                    bError = fillColumnRef(pColumnRef,xMetaData,aSelEntry,_bListAction);

                    // we found a simple column so we must clear the function fields but only when the column name is '*'
                    // and the function is different to count
                    clearEntryFunctionField(_sFieldName,aSelEntry,_bListAction,nColumnId);
                }
                else // no column, but may be a function or a calculation
                {
                    // first check if we have a aggregate function and only a function
                    if ( SQL_ISRULE(pColumnRef,general_set_fct) )
                    {
                        String sLocalizedFunctionName;
                        if ( GetFunctionName(pColumnRef->getChild(0)->getTokenID(),sLocalizedFunctionName) )
                        {
                            String sOldLocalizedFunctionName = aSelEntry->GetFunction();
                            aSelEntry->SetFunction(sLocalizedFunctionName);
                            sal_uInt32 nFunCount = pColumnRef->count() - 1;
                            sal_Int32 nFunctionType = FKT_AGGREGATE;
                            // may be there exists only one parameter which is a column, fill all information into our fields
                            if ( nFunCount == 4 && SQL_ISRULE(pColumnRef->getChild(3),column_ref) )
                                bError = fillColumnRef(pColumnRef->getChild(3),xMetaData,aSelEntry,_bListAction);
                            else if ( nFunCount == 3 ) // we have a COUNT(*) here, so take the first table
                                bError = fillColumnRef(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("*")),::rtl::OUString(),xMetaData,aSelEntry,_bListAction);
                            else
                                nFunctionType |= FKT_NUMERIC;

                            // now parse the parameters
                            ::rtl::OUString sParameters;
                            for(sal_uInt32 i = 2; i < nFunCount; ++i) // we only want to parse the parameters of the function
                                pColumnRef->getChild(i)->parseNodeToStr(sParameters,xMetaData,&pParser->getContext(),sal_True,sal_False);

                            aSelEntry->SetFunctionType(nFunctionType);
                            aSelEntry->SetField(sParameters);
                            if ( aSelEntry->IsGroupBy() )
                            {
                                sOldLocalizedFunctionName = m_aFunctionStrings.GetToken(m_aFunctionStrings.GetTokenCount()-1);
                                aSelEntry->SetGroupBy(sal_False);
                            }

                            // append undo action
                            notifyFunctionFieldChanged(sOldLocalizedFunctionName,sLocalizedFunctionName,_bListAction, nColumnId);
                        }
                        else
                            OSL_ENSURE(0,"Unsupported function inserted!");

                    }
                    else // a calculation has been found ( can be calc and function )
                    {
                        // append the whole text as field name
                        // so we first clear the function field
                        clearEntryFunctionField(_sFieldName,aSelEntry,_bListAction,nColumnId);
                        sal_Bool bQuote = sal_False;
                        if (    SQL_ISRULEOR2(pColumnRef,length_exp,char_value_fct)
                            ||  SQL_ISRULEOR2(pColumnRef,position_exp,extract_exp) )
                        {
                            bQuote = sal_True;
                            sal_Int32 nDataType = DataType::DOUBLE;
                            OSQLParseNode* pFunctionName = pColumnRef->getChild(0);
                            if ( !SQL_ISPUNCTUATION(pFunctionName,"{") )
                            {
                                if ( SQL_ISRULEOR2(pColumnRef,length_exp,char_value_fct) )
                                    pFunctionName = pFunctionName->getChild(0);

                                if ( pFunctionName )
                                {
                                    ::rtl::OUString sFunctionName = pFunctionName->getTokenValue();
                                    if ( !sFunctionName.getLength() )
                                        sFunctionName = ::rtl::OStringToOUString(OSQLParser::TokenIDToStr(pFunctionName->getTokenID()),RTL_TEXTENCODING_UTF8);

                                    nDataType = OSQLParser::getFunctionReturnType(
                                                        sFunctionName
                                                        ,&pController->getParser()->getContext());
                                    aSelEntry->SetDataType(nDataType);
                                }
                            }
                        }

                        // now parse the hole statement
                        sal_uInt32 nFunCount = pColumnRef->count();
                        ::rtl::OUString sParameters;
                        for(sal_uInt32 i = 0; i < nFunCount; ++i)
                            pColumnRef->getChild(i)->parseNodeToStr(sParameters,xMetaData,&pParser->getContext(),sal_True,bQuote);

                        ::rtl::OUString sOldAlias = aSelEntry->GetAlias();
                        aSelEntry->SetAlias(::rtl::OUString());

                        sal_Int32 nNewFunctionType = aSelEntry->GetFunctionType() | FKT_NUMERIC;
                        aSelEntry->SetFunctionType(nNewFunctionType);


                        aSelEntry->SetFieldType(TAB_NORMAL_FIELD);

                        aSelEntry->SetTabWindow(NULL);

                        aSelEntry->SetField(sParameters);
                        notifyTableFieldChanged(sOldAlias,aSelEntry->GetAlias(),_bListAction, nColumnId);
                    }
                }
                if ( i > 0 && InsertField(aSelEntry,BROWSER_INVALIDID,sal_True,sal_False).isEmpty() ) // may we have to append more than one field
                { // the field could not be isnerted
                    String sTitle(ModuleRes(STR_STAT_WARNING));
                    String sErrorMsg(ModuleRes(RID_STR_FIELD_DOESNT_EXIST));
                    sErrorMsg.SearchAndReplaceAscii("$name$",aSelEntry->GetField());
                    OSQLMessageBox aDlg(this,sTitle,sErrorMsg,WB_OK | WB_DEF_OK,OSQLMessageBox::Warning);
                    aDlg.Execute();
                    bError = sal_True;
                }
            }
        }
        delete pParseNode;
    }
    else // something different which we have to check (may be a select statement)
    {
        String sTitle(ModuleRes(STR_STAT_WARNING));
        String sErrorMsg(ModuleRes(STR_QRY_COLUMN_NOT_FOUND));
        sErrorMsg.SearchAndReplaceAscii("$name$",_sFieldName);
        OSQLMessageBox aDlg(this,sTitle,sErrorMsg,WB_OK | WB_DEF_OK,OSQLMessageBox::Warning);
        aDlg.Execute();
        bError = sal_True;
    }

    return bError;
}
//------------------------------------------------------------------------------
sal_Bool OSelectionBrowseBox::SaveModified()
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OTableFieldDescRef pEntry = NULL;
    OQueryController* pController = static_cast<OQueryController*>(getDesignView()->getController());
    USHORT nPos = GetColumnPos(GetCurColumnId());
    if(getFields().size() > static_cast<USHORT>(nPos - 1))
        pEntry = getEntry(nPos - 1);

    sal_Bool bWasEmpty = pEntry.isValid() ? pEntry->IsEmpty() : sal_False;
    sal_Bool bError         = sal_False;
    sal_Bool bListAction    = sal_False;

    if (pEntry.isValid() && Controller().Is() && Controller()->IsModified())
    {
        // fuer die Undo-Action
        String strOldCellContents,sNewValue;
        long nRow = GetRealRow(GetCurRow());
        switch (nRow)
        {
            case BROW_VIS_ROW:
                {
                    sal_Bool bOldValue = m_pVisibleCell->GetBox().GetSavedValue();
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
                if (!aFieldName.Len())
                {
                    OTableFieldDescRef pNewEntry = new OTableFieldDesc();
                    pNewEntry->SetColumnId( pEntry->GetColumnId() );
                    ::std::replace(getFields().begin(),getFields().end(),pEntry,pNewEntry);
                    sal_uInt16 nCol = GetCurColumnId();
                    for (int i = 0; i < m_nVisibleCount; i++)   // Spalte neu zeichnen
                        RowModified(i,nCol);
                }
                else
                {
                    strOldCellContents = pEntry->GetField();
                    bListAction = sal_True;
                    static_cast<OQueryController*>(getDesignView()->getController())->getUndoMgr()->EnterListAction(String(),String());

                    USHORT nPos = m_pFieldCell->GetEntryPos(aFieldName);
                    if ( nPos != COMBOBOX_ENTRY_NOTFOUND && aFieldName.GetTokenCount('.') > 1 )
                    { // special case, we have a table field so we must cut the table name
                        String sTableAlias = aFieldName.GetToken(0,'.');
                        pEntry->SetAlias(sTableAlias);
                        String sColumnName = aFieldName.GetToken(1,'.');
                        Reference<XConnection> xConnection = pController->getConnection();
                        if ( !xConnection.is() )
                            return sal_False;
                        Reference<XDatabaseMetaData> xMetaData = xConnection->getMetaData();
                        bError = fillColumnRef(sColumnName,sTableAlias,xMetaData,pEntry,bListAction);
                    }
                    else
                        bError = sal_True;

                    if ( bError )
                        bError = saveField(aFieldName,pEntry,bListAction);
                }
                if ( bError )
                {
                    sNewValue = aFieldName;
                    static_cast<OQueryController*>(getDesignView()->getController())->getUndoMgr()->LeaveListAction();
                    bListAction = sal_False;
                }
                else
                    sNewValue = pEntry->GetField();
                pController->InvalidateFeature( ID_BROWSER_QUERY_EXECUTE );
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
                                pEntry->SetDatabase(pEntryTab->GetComposedName());
                                pEntry->SetTable(pEntryTab->GetTableName());
                                pEntry->SetTabWindow(pEntryTab);
                            }
                        }
                    }
                }
                else
                {
                    pEntry->SetAlias(::rtl::OUString());
                    pEntry->SetDatabase(::rtl::OUString());
                    pEntry->SetTable(::rtl::OUString());
                    pEntry->SetTabWindow(NULL);
                }
                sNewValue = pEntry->GetAlias();

            }   break;

            case BROW_ORDER_ROW:
            {
                strOldCellContents = String::CreateFromInt32((sal_uInt16)pEntry->GetOrderDir());
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
                sNewValue = String::CreateFromInt32((sal_uInt16)pEntry->GetOrderDir());
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
                    // Diese Funktionen stehen nur unter CORE zur Verf¸gung
                    String sFunctionName        = m_pFunctionCell->GetEntry(nPos);
                    String sGroupFunctionName   = m_aFunctionStrings.GetToken(m_aFunctionStrings.GetTokenCount()-1);
                    sal_Bool bGroupBy = sal_False;
                    if ( sGroupFunctionName.Equals(sFunctionName) ) // check if the function name is GROUP
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
                Reference< XConnection> xConnection = static_cast<OQueryController*>(getDesignView()->getController())->getConnection();
                if(!xConnection.is())
                    break;

                sal_uInt16  nIdx = nRow - BROW_CRIT1_ROW;
                String aText = m_pTextCell->GetText();

                aText.EraseLeadingChars();
                ::rtl::OUString aCrit;
                if(aText.Len())
                {
                    ::rtl::OUString aErrorMsg;
                    Reference<XPropertySet> xColumn;
                    OSQLParseNode* pParseNode = getDesignView()->getPredicateTreeFromEntry(pEntry,aText,aErrorMsg,xColumn);

                    if (pParseNode)
                    {
                        pParseNode->parseNodeToPredicateStr(aCrit,
                                                            xConnection->getMetaData(),
                                                            static_cast<OQueryController*>(getDesignView()->getController())->getNumberFormatter(),
                                                            xColumn,
                                                            getDesignView()->getLocale(),
                                                            static_cast<sal_Char>(getDesignView()->getDecimalSeparator().toChar()),
                                                            &(static_cast<OQueryController*>(getDesignView()->getController())->getParser()->getContext()));
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
                                    if(aText.GetChar(0) != '\'' || aText.GetChar(aText.Len() -1) != '\'')
                                    {
                                        aText.SearchAndReplaceAll(String::CreateFromAscii("'"),String::CreateFromAscii("''"));
                                        String aTmp(String::CreateFromAscii("'"));
                                        (aTmp += aText) += String::CreateFromAscii("'");
                                        aText = aTmp;
                                    }
                                    break;
                                default:
                                    ;
                            }
                            ::connectivity::OSQLParser* pParser = static_cast<OQueryController*>(getDesignView()->getController())->getParser();
                            pParseNode = pParser->predicateTree(aErrorMsg,
                                                                aText,
                                                                static_cast<OQueryController*>(getDesignView()->getController())->getNumberFormatter(),
                                                                xColumn);
                            if (pParseNode)
                            {
                                pParseNode->parseNodeToPredicateStr(aCrit,
                                                                    xConnection->getMetaData(),
                                                                    static_cast<OQueryController*>(getDesignView()->getController())->getNumberFormatter(),
                                                                    xColumn,
                                                                    getDesignView()->getLocale(),
                                                                    static_cast<sal_Char>(getDesignView()->getDecimalSeparator().toChar()),
                                                                    &(static_cast<OQueryController*>(getDesignView()->getController())->getParser()->getContext()));
                                delete pParseNode;
                            }
                            else
                            {
                                String sTitle(ModuleRes(STR_STAT_WARNING));
                                OSQLMessageBox aDlg(this,sTitle,aErrorMsg,WB_OK | WB_DEF_OK,OSQLMessageBox::Warning);
                                aDlg.Execute();
                                bError = sal_True;
                            }
                        }
                        else
                        {
                            String sTitle(ModuleRes(STR_STAT_WARNING));
                            OSQLMessageBox aDlg(this,sTitle,aErrorMsg,WB_OK | WB_DEF_OK,OSQLMessageBox::Warning);
                            aDlg.Execute();
                            bError = sal_True;
                        }
                    }
                    //  }
                }
                strOldCellContents = pEntry->GetCriteria(nIdx);
                pEntry->SetCriteria(nIdx, aCrit);
                sNewValue = pEntry->GetCriteria(nIdx);
                if(aCrit.getLength() && nRow >= (GetRowCount()-1))
                {
                    RowInserted( GetRowCount()-1, 1, TRUE );
                    m_bVisibleRow.push_back(sal_True);
                    ++m_nVisibleCount;
                }
            }
        }
        if(!bError && Controller())
            Controller()->ClearModified();

        RowModified(GetCurRow(), GetCurColumnId());

        if(!bError)
        {
            // und noch die Undo-Action fuer das Ganze
            appendUndoAction(strOldCellContents,sNewValue,nRow);

        }
    }

    // habe ich Daten in einer FieldDescription gespeichert, die vorher leer war und es nach den Aenderungen nicht mehr ist ?
    if ( pEntry.isValid() && bWasEmpty && !pEntry->IsEmpty() && !bError )
    {
        // Default auf sichtbar
        pEntry->SetVisible(sal_True);
        appendUndoAction(g_strZero,g_strOne,BROW_VIS_ROW,bListAction);
        RowModified(BROW_VIS_ROW, GetCurColumnId());

        // wenn noetig neue freie Spalten anlegen
        USHORT nDummy;
        CheckFreeColumns(nDummy);
    }

    if ( bListAction )
        static_cast<OQueryController*>(getDesignView()->getController())->getUndoMgr()->LeaveListAction();

    return pEntry != NULL && !bError;
}

//------------------------------------------------------------------------------
sal_Bool OSelectionBrowseBox::SeekRow(long nRow)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    sal_Bool bRet = sal_False;

    m_nSeekRow = nRow;
    if (nRow < m_nVisibleCount )
        bRet = sal_True;

    return bRet;
}

//------------------------------------------------------------------------------
void OSelectionBrowseBox::PaintCell(OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColumnId) const
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    rDev.SetClipRegion( rRect );

    OTableFieldDescRef pEntry = NULL;
    USHORT nPos = GetColumnPos(nColumnId);
    if(getFields().size() > sal_uInt16(nPos - 1))
        pEntry = getFields()[nPos - 1];

    if (!pEntry.isValid())
        return;

    long nRow = GetRealRow(m_nSeekRow);
    if (nRow == BROW_VIS_ROW)
        PaintTristate(rDev, rRect, pEntry->IsVisible() ? STATE_CHECK : STATE_NOCHECK);
    else
        rDev.DrawText(rRect.TopLeft(), GetCellText(nRow, nColumnId));

    rDev.SetClipRegion( );
}

//------------------------------------------------------------------------------
void OSelectionBrowseBox::PaintStatusCell(OutputDevice& rDev, const Rectangle& rRect) const
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    Point   aPos(rRect.TopLeft());
    aPos.Y() -= 2;
    String  aLabel(ModuleRes(STR_QUERY_HANDLETEXT));

    // ab BROW_CRIT2_ROW werden alle Zeilen mit "oder" angegeben
    xub_StrLen nToken = (xub_StrLen) (m_nSeekRow >= GetBrowseRow(BROW_CRIT2_ROW))
                                ?
            xub_StrLen(BROW_CRIT2_ROW) : xub_StrLen(GetRealRow(m_nSeekRow));
    rDev.DrawText(aPos, aLabel.GetToken(nToken));
}

//------------------------------------------------------------------------------
void OSelectionBrowseBox::RemoveColumn(USHORT _nColumnId)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OQueryController* pController = static_cast<OQueryController*>(getDesignView()->getController());

    USHORT nPos = GetColumnPos(_nColumnId);
        // das Control sollte immer genau eine Spalte mehr haben, naemlich die HandleColumn
    DBG_ASSERT((nPos == 0) || (nPos <= getFields().size()), "OSelectionBrowseBox::RemoveColumn : invalid parameter nColId");
        // ColId ist bei mir gleichbedeutend mit Position, und da sollte die Bedingung natuerlich zutreffen

    sal_uInt16 nCurCol = GetCurColumnId();
    long nCurRow = GetCurRow();

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

    ActivateCell( nCurRow, nCurCol );

    pController->setModified();

    invalidateUndoRedo();
}

//------------------------------------------------------------------------------
void OSelectionBrowseBox::RemoveField(sal_uInt16 nColumnId, sal_Bool bActivate)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OQueryController* pController = static_cast<OQueryController*>(getDesignView()->getController());

    USHORT nPos = GetColumnPos(nColumnId);
    OSL_ENSURE(getFields().size() > sal_uInt16(nPos-1),"ID is to great!");

    OTableFieldDescRef pDesc = getEntry((sal_uInt32)(nPos - 1)) ;
    pDesc->SetColWidth( (sal_uInt16)GetColumnWidth(nColumnId) );    // hat er sich vorher leider nicht gemerkt

    // UndoAction erzeugen
    OTabFieldDelUndoAct* pUndoAction = new OTabFieldDelUndoAct( this );
    pUndoAction->SetTabFieldDescr(pDesc);
    pUndoAction->SetColumnPosition(nPos);
    pController->addUndoActionAndInvalidate( pUndoAction );

    RemoveColumn(nColumnId);

    invalidateUndoRedo();
}

//------------------------------------------------------------------------------
void OSelectionBrowseBox::adjustSelectionMode( sal_Bool _bClickedOntoHeader, sal_Bool _bClickedOntoHandleCol )
{
    // wenn ein Header selectiert wird, muﬂ die selection angezeigt werden, sonst nicht)
    if ( _bClickedOntoHeader )
    {
        if (0 == GetSelectColumnCount() )
            // wenn es schon eine selektierte Spalte gibt, bin ich schon im richtigen Modus
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

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
void OSelectionBrowseBox::MouseButtonUp(const BrowserMouseEvent& rEvt)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    EditBrowseBox::MouseButtonUp( rEvt );
    static_cast<OQueryController*>(getDesignView()->getController())->InvalidateFeature( ID_BROWSER_QUERY_EXECUTE );
}

//------------------------------------------------------------------------------
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


//------------------------------------------------------------------------------
sal_Int8 OSelectionBrowseBox::AcceptDrop( const BrowserAcceptDropEvent& rEvt )
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    sal_Int8 nDropAction = DND_ACTION_NONE;
    if  (   (rEvt.GetColumnId() >= 0)
        &&  (rEvt.GetRow() >= -1)
        )
    {
        if ( IsEditing() )
        {
            // #100271# OJ allow the asterix again
            SaveModified();
            DeactivateCell();
        }
        // check if the format is already supported, if not deactivate the current cell and try again
        if ( OJoinExchObj::isFormatAvailable(GetDataFlavors()) )
            nDropAction = DND_ACTION_LINK;
    }

    return nDropAction;
}

//------------------------------------------------------------------------------
sal_Int8 OSelectionBrowseBox::ExecuteDrop( const BrowserExecuteDropEvent& _rEvt )
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);

    TransferableDataHelper aDropped(_rEvt.maDropEvent.Transferable);
    if (!OJoinExchObj::isFormatAvailable(aDropped.GetDataFlavorExVector()))
    {
        DBG_ERROR("OSelectionBrowseBox::ExecuteDrop: this should never have passed AcceptDrop!");
        return DND_ACTION_NONE;
    }

    OTableFieldDesc aInfo;
    // Einfuegen des Feldes an der gewuenschten Position
    OJoinExchangeData jxdSource = OJoinExchObj::GetSourceDescription(_rEvt.maDropEvent.Transferable);
    InsertField(jxdSource);

    return DND_ACTION_LINK;
}

//------------------------------------------------------------------------------
OTableFieldDescRef OSelectionBrowseBox::AppendNewCol( sal_uInt16 nCnt)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    // es koennen mehrere angelegt werden, aber der Erste
    // wird returnt
    sal_uInt32 nCount = getFields().size();
    for (sal_uInt16 i=0 ; i<nCnt ; i++)
    {
        OTableFieldDescRef pEmptyEntry = new OTableFieldDesc();
        getFields().push_back(pEmptyEntry);
        USHORT nColumnId = getFields().size();
        pEmptyEntry->SetColumnId( nColumnId );

        InsertDataColumn( nColumnId , String(), DEFAULT_SIZE, HIB_STDSTYLE, HEADERBAR_APPEND);
    }

    return getFields()[nCount];
}

//------------------------------------------------------------------------------
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
        for(USHORT nPos=getFields().size();aIter != getFields().rend();++aIter,--nPos)
        {
            pEntry = *aIter;
            if (pEntry->GetAlias() == ::rtl::OUString(rAliasName) )
                RemoveField(GetColumnId(nPos), sal_False);
        }

        if (bWasEditing)
            ActivateCell(nRow , nColId);
    }
}

//------------------------------------------------------------------------------
void OSelectionBrowseBox::SetColWidth()
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    // Spaltenbreite ermitteln
    sal_uInt16 nColCount = ColCount();
    sal_uInt32 nColWidth = 0;

    for( sal_uInt16 nColId=1; nColId<=nColCount; nColId++ )
    {
        nColWidth = static_cast<OQueryController*>(getDesignView()->getController())->getColWidth( nColId );

        if( nColWidth == 0)
            nColWidth = DEFAULT_SIZE;
        SetColumnWidth( nColId, nColWidth );
    }
}

//------------------------------------------------------------------------------
void OSelectionBrowseBox::SetColWidth(sal_uInt16 nColId, long nNewWidth)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    sal_Bool bWasEditing = IsEditing();
    if (bWasEditing)
        DeactivateCell();

    // die Basisklasse machen lassen
    SetColumnWidth(nColId, nNewWidth);

    // der FieldDescription Bescheid sagen
    OTableFieldDescRef pEntry = getEntry(GetColumnPos(nColId) - 1);
    if (pEntry.isValid())
        pEntry->SetColWidth(sal_uInt16(GetColumnWidth(nColId)));

    if (bWasEditing)
        ActivateCell(GetCurRow(), GetCurColumnId());
}

//------------------------------------------------------------------------------
Rectangle OSelectionBrowseBox::GetInvalidRect( sal_uInt16 nColId )
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    //////////////////////////////////////////////////////////////////////
    // Rechteck ist erst einmal der gesamte Outputbereich des Fensters
    Rectangle aInvalidRect( Point(0,0), GetOutputSizePixel() );

    //////////////////////////////////////////////////////////////////////
    // Dann wird die linke Seite angepasst
    Rectangle aFieldRect(GetCellRect( 0, nColId )); // used instead of GetFieldRectPixel
    aInvalidRect.Left() = aFieldRect.Left();

    return aInvalidRect;
}

//------------------------------------------------------------------------------
void OSelectionBrowseBox::InsertColumn(OTableFieldDescRef pEntry, USHORT& _nColumnPostion)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OQueryController* pController = static_cast<OQueryController*>(getDesignView()->getController());
        // das Control sollte immer genau eine Spalte mehr haben, naemlich die HandleColumn
    DBG_ASSERT(_nColumnPostion == BROWSER_INVALIDID || (_nColumnPostion <= (long)getFields().size()), "OSelectionBrowseBox::InsertColumn : invalid parameter nColId.");
        // -1 heisst ganz hinten, Count heisst ganz hinten, der Rest bezeichnet eine richtige Position

    sal_uInt16 nCurCol = GetCurColumnId();
    long nCurRow = GetCurRow();

    DeactivateCell();

    // remember the column id of the current positon
    USHORT nColumnId = GetColumnId(_nColumnPostion);
    // Wenn zu klein oder zu gross, auf Ende der Liste setzen
    if ((_nColumnPostion == BROWSER_INVALIDID) || (_nColumnPostion >= getFields().size()))   // Anhaengen des Feldes
    {
        if (FindFirstFreeCol(_nColumnPostion) == NULL)  // keine freie Column mehr
        {
            AppendNewCol(1);
            _nColumnPostion = getFields().size();
        }
        else
            ++_nColumnPostion; // innerhalb der vorgegebenen Liste
        nColumnId = GetColumnId(_nColumnPostion);
        pEntry->SetColumnId( nColumnId );
        getFields()[ _nColumnPostion - 1] = pEntry;
    }

    // check if the column ids are identical, if not we have to move
    if ( pEntry->GetColumnId() != nColumnId )
    {
        USHORT nOldPosition = GetColumnPos(pEntry->GetColumnId());
        OSL_ENSURE( nOldPosition != 0,"Old position was 0. Not possible!");
        SetColumnPos(pEntry->GetColumnId(),_nColumnPostion);
        // we have to delete an empty field for the fields list, because the columns must have equal length
        if ( nOldPosition > 0 && nOldPosition <= getFields().size() )
            getFields()[nOldPosition - 1] = pEntry;

        ColumnMoved(pEntry->GetColumnId(),FALSE);
    }

    // Neuzeichnen
    Rectangle aInvalidRect = GetInvalidRect( pEntry->GetColumnId() );
    Invalidate( aInvalidRect );

    ActivateCell( nCurRow, nCurCol );
    static_cast<OQueryController*>(getDesignView()->getController())->setModified();

    invalidateUndoRedo();
}

//------------------------------------------------------------------------------
OTableFieldDescRef OSelectionBrowseBox::InsertField(const OJoinExchangeData& jxdSource, USHORT _nColumnPostion, sal_Bool bVis, sal_Bool bActivate)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OQueryTableWindow* pSourceWin = static_cast<OQueryTableWindow*>(jxdSource.pListBox->GetTabWin());
    if (!pSourceWin)
        return NULL;

    // Namen/Position des selektierten Feldes
    String aFieldName = jxdSource.pListBox->GetEntryText(jxdSource.pEntry);
    sal_uInt32 nFieldIndex = jxdSource.pListBox->GetModel()->GetAbsPos(jxdSource.pEntry);
    OTableFieldInfo* pInf = static_cast<OTableFieldInfo*>(jxdSource.pEntry->GetUserData());

    // eine DragInfo aufbauen, damit ich mich auf das andere InsertField zurueckziehen kann
    OTableFieldDescRef aInfo = new OTableFieldDesc(pSourceWin->GetTableName(),aFieldName);
    aInfo->SetTabWindow(pSourceWin);
    aInfo->SetFieldIndex(nFieldIndex);
    aInfo->SetFieldType(pInf->GetKeyType());
    aInfo->SetAlias(pSourceWin->GetAliasName());
    aInfo->SetDatabase(pSourceWin->GetComposedName());

    aInfo->SetDataType(pInf->GetDataType());
    aInfo->SetVisible(bVis);

    return InsertField(aInfo, _nColumnPostion, bVis, bActivate);
}

//------------------------------------------------------------------------------
OTableFieldDescRef OSelectionBrowseBox::InsertField(const OTableFieldDescRef& _rInfo, USHORT _nColumnPostion, sal_Bool bVis, sal_Bool bActivate)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);

    if(m_nMaxColumns && m_nMaxColumns <= FieldsCount())
        return NULL;
    if (bActivate)
        SaveModified();

    // Neue Spaltenbeschreibung
    OTableFieldDescRef pEntry = _rInfo;
    pEntry->SetVisible(bVis);
    sal_uInt32 nColWidth;
    if( getDesignView()->getColWidth(_rInfo->GetAlias(), _rInfo->GetField(), nColWidth) )
        pEntry->SetColWidth( (sal_uInt16)nColWidth );
    else
        pEntry->SetColWidth( (sal_uInt16)DEFAULT_SIZE );

    // Spalte einfuegen
    InsertColumn( pEntry, _nColumnPostion );

    // UndoAction erzeugen
    OTabFieldCreateUndoAct* pUndoAction = new OTabFieldCreateUndoAct( this );
    pUndoAction->SetTabFieldDescr( pEntry );
    pUndoAction->SetColumnPosition(_nColumnPostion);
    getDesignView()->getController()->addUndoActionAndInvalidate( pUndoAction );

    return pEntry;
}

//------------------------------------------------------------------------------
sal_uInt16 OSelectionBrowseBox::FieldsCount()
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OQueryController* pController = static_cast<OQueryController*>(getDesignView()->getController());

    OTableFields::iterator aIter = getFields().begin();
    sal_uInt16 nCount = 0;

    while (aIter != getFields().end())
    {
        if ((*aIter).isValid() && !(*aIter)->IsEmpty())
            ++nCount;
        ++aIter;
    }

    return nCount;
}

//------------------------------------------------------------------------------
OTableFieldDescRef OSelectionBrowseBox::FindFirstFreeCol(USHORT& _rColumnPosition )
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    OQueryController* pController = static_cast<OQueryController*>(getDesignView()->getController());

    OTableFields::iterator aIter = getFields().begin();
    OTableFields::iterator aEnd  = getFields().end();

    _rColumnPosition = BROWSER_INVALIDID;

    while ( aIter != aEnd )
    {
        ++_rColumnPosition;
        OTableFieldDescRef pEntry = (*aIter);
        if ( pEntry.isValid() && pEntry->IsEmpty() )
            return pEntry;
        ++aIter;
    }

    return NULL;
}

//------------------------------------------------------------------------------
void OSelectionBrowseBox::CheckFreeColumns(USHORT& _rColumnPosition)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    if (FindFirstFreeCol(_rColumnPosition) == NULL)
    {
        // es ist voll, also einen Packen Spalten anhaengen
        AppendNewCol(DEFAULT_QUERY_COLS);
        OSL_VERIFY(FindFirstFreeCol(_rColumnPosition).isValid());
    }
}
//------------------------------------------------------------------------------
void OSelectionBrowseBox::AddGroupBy( const OTableFieldDescRef& rInfo )
{
    Reference< XConnection> xConnection = static_cast<OQueryController*>(getDesignView()->getController())->getConnection();
    if(!xConnection.is())
        return;
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    DBG_ASSERT(!rInfo->IsEmpty(),"AddGroupBy:: OTableFieldDescRef sollte nicht Empty sein!");
    OTableFieldDescRef pEntry;
    ::comphelper::UStringMixEqual bCase(xConnection->getMetaData()->storesMixedCaseQuotedIdentifiers());
    OTableFields::iterator aIter = getFields().begin();
    for(;aIter != getFields().end();++aIter)
    {
        pEntry = *aIter;
        OSL_ENSURE(pEntry.isValid(),"OTableFieldDescRef was null!");

        ::rtl::OUString aField = pEntry->GetField();
        ::rtl::OUString aAlias = pEntry->GetAlias();

        if (bCase(aField,rInfo->GetField()) &&
            bCase(aAlias,rInfo->GetAlias()) &&
            pEntry->GetFunctionType() == rInfo->GetFunctionType())
        {
            if ( pEntry->isNumericOrAggreateFunction() && rInfo->IsGroupBy() )
                pEntry->SetGroupBy(sal_False);
            else
            {
                pEntry->SetGroupBy(rInfo->IsGroupBy());
                if(!m_bGroupByUnRelated && pEntry->IsGroupBy())
                    pEntry->SetVisible(sal_True);
            }
            break;
        }
    }

    if (aIter == getFields().end())
    {
        OTableFieldDescRef pTmp = InsertField(rInfo, BROWSER_INVALIDID, sal_False, sal_False );
        if ( (pTmp->isNumericOrAggreateFunction() && rInfo->IsGroupBy()) ) // das GroupBy wird bereits von rInfo "ubernommen
            pTmp->SetGroupBy(sal_False);
    }
}
//------------------------------------------------------------------------------
void OSelectionBrowseBox::AddCondition( const OTableFieldDescRef& rInfo, const String& rValue, const sal_uInt16 nLevel, const char* pOp )
{
    Reference< XConnection> xConnection = static_cast<OQueryController*>(getDesignView()->getController())->getConnection();
    if(!xConnection.is())
        return;
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    DBG_ASSERT(rInfo.isValid() && !rInfo->IsEmpty(),"AddCondition:: OTableFieldDescRef sollte nicht Empty sein!");

    OTableFieldDescRef pEntry;
    ::comphelper::UStringMixEqual bCase(xConnection->getMetaData()->storesMixedCaseQuotedIdentifiers());

    OTableFields::iterator aIter = getFields().begin();
    for(;aIter != getFields().end();++aIter)
    {
        pEntry = *aIter;
        ::rtl::OUString aField = pEntry->GetField();
        ::rtl::OUString aAlias = pEntry->GetAlias();

        if (bCase(aField,rInfo->GetField()) &&
            bCase(aAlias,rInfo->GetAlias()) &&
            pEntry->GetFunctionType() == rInfo->GetFunctionType())
        {
            if ( pEntry->isNumericOrAggreateFunction() && rInfo->IsGroupBy() )
                pEntry->SetGroupBy(sal_False);
            else
            {
                pEntry->SetGroupBy(rInfo->IsGroupBy());
                if(!m_bGroupByUnRelated && pEntry->IsGroupBy())
                    pEntry->SetVisible(sal_True);
            }
            if (!pEntry->GetCriteria(nLevel).getLength())
            {
                pEntry->SetCriteria( nLevel, rValue);
                if(nLevel == (m_nVisibleCount-BROW_CRIT1_ROW-1))
                {
                    RowInserted( GetRowCount()-1, 1, TRUE );
                    m_bVisibleRow.push_back(sal_True);
                    ++m_nVisibleCount;
                }
                break;
            }
        }
    }

    if (aIter == getFields().end())
    {
        OTableFieldDescRef pTmp = InsertField(rInfo, BROWSER_INVALIDID, sal_False, sal_False );
        if ( pTmp->isNumericOrAggreateFunction() && rInfo->IsGroupBy() ) // das GroupBy wird bereits von rInfo "ubernommen
            pTmp->SetGroupBy(sal_False);
        if ( pTmp.isValid() )
        {
            pTmp->SetCriteria( nLevel, rValue);
            if(nLevel == (m_nVisibleCount-BROW_CRIT1_ROW-1))
            {
                RowInserted( GetRowCount()-1, 1, TRUE );
                m_bVisibleRow.push_back(sal_True);
                ++m_nVisibleCount;
            }
        }
    }
}

//------------------------------------------------------------------------------
void OSelectionBrowseBox::AddOrder( const OTableFieldDescRef& rInfo, const EOrderDir eDir, sal_uInt16& nPos )
{
    Reference< XConnection> xConnection = static_cast<OQueryController*>(getDesignView()->getController())->getConnection();
    if(!xConnection.is())
        return;
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    DBG_ASSERT(!rInfo->IsEmpty(),"AddOrder:: OTableFieldDescRef sollte nicht Empty sein!");
    // nPos merkt sich die Spalte in die Sortierung eingetragen wird,
    // da weitere Sortierungen nur dahinter abgelegt werden duerfen
    OTableFieldDescRef pEntry;
    ::comphelper::UStringMixEqual bCase(xConnection->getMetaData()->storesMixedCaseQuotedIdentifiers());

    OTableFields::iterator aIter = getFields().begin();
    for(;aIter != getFields().end();++aIter)
    {
        pEntry = *aIter;
        ::rtl::OUString aField = pEntry->GetField();
        ::rtl::OUString aAlias = pEntry->GetAlias();

        if (bCase(aField,rInfo->GetField()) &&
            bCase(aAlias,rInfo->GetAlias()))
        {
            if(!m_bOrderByUnRelated)
                pEntry->SetVisible(sal_True);
            pEntry->SetOrderDir( eDir );
            nPos = getFields().end() - aIter;
            break;
        }
    }

    if (aIter == getFields().end())
    {
        OTableFieldDescRef pTmp = InsertField(rInfo, BROWSER_INVALIDID, sal_False, sal_False );
        if(pTmp.isValid())
        {
            if(!m_bOrderByUnRelated)
                pTmp->SetVisible(sal_True);
            pTmp->SetOrderDir( eDir );
        }

        nPos = (sal_uInt16)( getFields().size()-1 );
    }
}

//------------------------------------------------------------------------------
void OSelectionBrowseBox::ArrangeControls(sal_uInt16& nX, sal_uInt16 nY)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    EditBrowseBox::ArrangeControls(nX, nY);
}

//------------------------------------------------------------------------------
sal_Bool OSelectionBrowseBox::Save()
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    sal_Bool bRet = sal_True;
    if (IsModified())
        bRet = SaveModified();
    return bRet;
}

//------------------------------------------------------------------------------
void OSelectionBrowseBox::CellModified()
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    long nRow = GetRealRow(GetCurRow());
    switch (nRow)
    {
        case BROW_VIS_ROW:
            {
                OTableFieldDescRef  pEntry = getEntry(GetColumnPos(GetCurColumnId()) - 1);

                sal_Bool bOldValue = pEntry->IsVisible();

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
    static_cast<OQueryController*>(getDesignView()->getController())->setModified();
}

//------------------------------------------------------------------------------
void OSelectionBrowseBox::Fill()
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    DBG_ASSERT(ColCount() >= 1, "OSelectionBrowseBox::Fill : please call only after inserting the handle column !");

    sal_uInt16 nColCount = ColCount() - 1;
    if (nColCount < DEFAULT_QUERY_COLS)
        AppendNewCol(DEFAULT_QUERY_COLS - nColCount);
}

//------------------------------------------------------------------------------
Size OSelectionBrowseBox::CalcOptimalSize( const Size& _rAvailable )
{
    Size aReturn( _rAvailable.Width(), GetTitleHeight() );

    aReturn.Height() += ( m_nVisibleCount ? m_nVisibleCount : 15 ) * GetDataRowHeight();
    aReturn.Height() += 40; // just some space

    return aReturn;
}

//------------------------------------------------------------------------------
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
                    sal_uInt16 nSelId = GetColumnId( FirstSelectedColumn() );
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

                if (!static_cast<OQueryController*>(getDesignView()->getController())->isReadOnly())
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
                if (!static_cast<OQueryController*>(getDesignView()->getController())->isReadOnly())
                {
                    PopupMenu aContextMenu(ModuleRes(RID_QUERYFUNCTION_POPUPMENU));
                    aContextMenu.CheckItem( ID_QUERY_FUNCTION, m_bVisibleRow[BROW_FUNCTION_ROW]);
                    aContextMenu.CheckItem( ID_QUERY_TABLENAME, m_bVisibleRow[BROW_TABLE_ROW]);
                    aContextMenu.CheckItem( ID_QUERY_ALIASNAME, m_bVisibleRow[BROW_COLUMNALIAS_ROW]);
                    aContextMenu.CheckItem( ID_QUERY_DISTINCT, static_cast<OQueryController*>(getDesignView()->getController())->isDistinct());

                    switch (aContextMenu.Execute(this, aMenuPos))
                    {
                        case ID_QUERY_FUNCTION:
                            SetRowVisible(BROW_FUNCTION_ROW, !IsRowVisible(BROW_FUNCTION_ROW));
                            static_cast<OQueryController*>(getDesignView()->getController())->InvalidateFeature( ID_BROWSER_QUERY_VIEW_FUNCTIONS );
                            break;
                        case ID_QUERY_TABLENAME:
                            SetRowVisible(BROW_TABLE_ROW, !IsRowVisible(BROW_TABLE_ROW));
                            static_cast<OQueryController*>(getDesignView()->getController())->InvalidateFeature( ID_BROWSER_QUERY_VIEW_TABLES );
                            break;
                        case ID_QUERY_ALIASNAME:
                            SetRowVisible(BROW_COLUMNALIAS_ROW, !IsRowVisible(BROW_COLUMNALIAS_ROW));
                            static_cast<OQueryController*>(getDesignView()->getController())->InvalidateFeature( ID_BROWSER_QUERY_VIEW_ALIASES );
                            break;
                        case ID_QUERY_DISTINCT:
                            static_cast<OQueryController*>(getDesignView()->getController())->setDistinct(!static_cast<OQueryController*>(getDesignView()->getController())->isDistinct());
                            static_cast<OQueryController*>(getDesignView()->getController())->setModified();
                            static_cast<OQueryController*>(getDesignView()->getController())->InvalidateFeature( ID_BROWSER_QUERY_DISTINCT_VALUES );
                            break;
                    }

                    static_cast<OQueryController*>(getDesignView()->getController())->setModified();
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

//------------------------------------------------------------------------------
sal_Bool OSelectionBrowseBox::IsRowVisible(sal_uInt16 _nWhich) const
{
    DBG_ASSERT(_nWhich>=0 && _nWhich<(m_bVisibleRow.size()), "OSelectionBrowseBox::IsRowVisible : invalid parameter !");
    return m_bVisibleRow[_nWhich];
}

//------------------------------------------------------------------------------
void OSelectionBrowseBox::SetRowVisible(sal_uInt16 _nWhich, sal_Bool _bVis)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    DBG_ASSERT(_nWhich>=0 && _nWhich<m_bVisibleRow.size(), "OSelectionBrowseBox::SetRowVisible : invalid parameter !");

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

//------------------------------------------------------------------------------
long OSelectionBrowseBox::GetBrowseRow(long nRowId) const
{
    USHORT nCount(0);
    for(USHORT i = 0 ; i < nRowId ; ++i)
    {
        if ( m_bVisibleRow[i] )
            ++nCount;
    }
    return nCount;
}
//------------------------------------------------------------------------------
long OSelectionBrowseBox::GetRealRow(long nRowId) const
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    long nErg=0,i;
    long nCount = m_bVisibleRow.size();
    for(i=0;i < nCount; ++i)
    {
        if(m_bVisibleRow[i])
        {
            if(nErg++ == nRowId)
                break;
        }
    }
    DBG_ASSERT(nErg <= long(m_bVisibleRow.size()),"nErg kann nicht groesser als BROW_ROW_CNT sein!");
    return i;
}
static long nVisibleRowMask[] =
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
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
void OSelectionBrowseBox::SetNoneVisbleRow(long nRows)
{
    // only the first 11 row are interesting
    sal_Int32 nSize = sizeof(nVisibleRowMask) / sizeof(nVisibleRowMask[0]);
    for(sal_Int32 i=0;i< nSize;i++)
        m_bVisibleRow[i] = !(nRows & nVisibleRowMask[i]);
}
//------------------------------------------------------------------------------
String OSelectionBrowseBox::GetCellText(long nRow, sal_uInt16 nColId) const
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);

    USHORT nPos = GetColumnPos(nColId);

    OTableFieldDescRef pEntry = getFields()[nPos-1];
    DBG_ASSERT(pEntry != NULL, "OSelectionBrowseBox::GetCellText : invalid column id, prepare for GPF ... ");
    if ( pEntry->IsEmpty() )
        return String();

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
                aText = String(ModuleRes(STR_QUERY_SORTTEXT) ).GetToken(pEntry->GetOrderDir());
            break;
        case BROW_VIS_ROW:
            break;
        case BROW_COLUMNALIAS_ROW:
            aText = pEntry->GetFieldAlias();
            break;
        case BROW_FUNCTION_ROW:
            // we always show the group function at first
            if ( pEntry->IsGroupBy() )
                aText = m_aFunctionStrings.GetToken(m_aFunctionStrings.GetTokenCount()-1);
            else if ( pEntry->isNumericOrAggreateFunction() )
                aText = pEntry->GetFunction();
            break;
        default:
            aText = pEntry->GetCriteria(nRow - BROW_CRIT1_ROW);
    }
    return aText;
}
//------------------------------------------------------------------------------
sal_Bool OSelectionBrowseBox::GetFunctionName(sal_uInt32 _nFunctionTokenId,String& rFkt)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    sal_Bool bErg=sal_True;
    String aText;
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
        default:
            {
                xub_StrLen nCount = m_aFunctionStrings.GetTokenCount();
                for (xub_StrLen i = 0; i < nCount-1; i++) // Gruppierung wird nicht mit gez"ahlt
                {
                    if(rFkt.EqualsIgnoreCaseAscii(m_aFunctionStrings.GetToken(i)))
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
//------------------------------------------------------------------------------
String OSelectionBrowseBox::GetCellContents(sal_Int32 nCellIndex, USHORT nColId)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    //  DBG_ASSERT(nCellIndex < (GetRowCount()-1),"CellIndex ist zu gross");
    if ( GetCurColumnId() == nColId )
        SaveModified();

    USHORT nPos = GetColumnPos(nColId);
    OTableFieldDescRef pEntry = getFields()[nPos - 1];
    DBG_ASSERT(pEntry != NULL, "OSelectionBrowseBox::GetCellContents : invalid column id, prepare for GPF ... ");

    switch (nCellIndex)
    {
        case BROW_VIS_ROW :
            return pEntry->IsVisible() ? g_strOne : g_strZero;
        case BROW_ORDER_ROW:
        {
            sal_uInt16 nIdx = m_pOrderCell->GetSelectEntryPos();
            if (nIdx == sal_uInt16(-1))
                nIdx = 0;
            return String(nIdx);
        }
        default:
            return GetCellText(nCellIndex, nColId);
    }
}

//------------------------------------------------------------------------------
void OSelectionBrowseBox::SetCellContents(sal_Int32 nRow, USHORT nColId, const String& strNewText)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    sal_Bool bWasEditing = IsEditing() && (GetCurColumnId() == nColId) && IsRowVisible(static_cast<USHORT>(nRow)) && (GetCurRow() == static_cast<USHORT>(GetBrowseRow(nRow)));
    if (bWasEditing)
        DeactivateCell();

    USHORT nPos = GetColumnPos(nColId);
    OTableFieldDescRef pEntry = getEntry(nPos - 1);
    DBG_ASSERT(pEntry != NULL, "OSelectionBrowseBox::SetCellContents : invalid column id, prepare for GPF ... ");


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
            String sOldFunctionName   = pEntry->GetFunction();
            String sGroupFunctionName = m_aFunctionStrings.GetToken(m_aFunctionStrings.GetTokenCount()-1);
            pEntry->SetFunction(strNewText);
            // first reset this two member
            sal_Int32 nFunctionType = pEntry->GetFunctionType();
            nFunctionType &= ~FKT_AGGREGATE;
            pEntry->SetFunctionType(nFunctionType);
            if ( pEntry->IsGroupBy() && !sGroupFunctionName.EqualsIgnoreCaseAscii(strNewText) )
                pEntry->SetGroupBy(sal_False);


            if ( sGroupFunctionName.EqualsIgnoreCaseAscii(strNewText) )
                pEntry->SetGroupBy(sal_True);
            else if ( strNewText.Len() )
            {
                nFunctionType |= FKT_AGGREGATE;
                pEntry->SetFunctionType(nFunctionType);
            }
        }   break;
        default:
            pEntry->SetCriteria(nRow - BROW_CRIT1_ROW, strNewText);
    }

    long nCellIndex = GetRealRow(nRow);
    if(IsRowVisible(static_cast<USHORT>(nRow)))
        RowModified(nCellIndex, nColId);

    // die entsprechende Feld-Beschreibung ist jetzt leer -> Visible auf sal_False (damit das konsistent mit normalen leeren Spalten ist)
    if (pEntry->IsEmpty())
        pEntry->SetVisible(sal_False);

    if (bWasEditing)
        ActivateCell(nCellIndex, nColId);

    static_cast<OQueryController*>(getDesignView()->getController())->setModified();
}
//------------------------------------------------------------------------------
sal_uInt32 OSelectionBrowseBox::GetTotalCellWidth(long nRow, sal_uInt16 nColId) const
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);

    long nRowId = GetRealRow(nRow);
    if (nRowId == BROW_VIS_ROW)
        return CHECKBOX_SIZE;
    else
        return  GetDataWindow().GetTextWidth(GetCellText(nRowId, nColId));
}

//------------------------------------------------------------------------------
void OSelectionBrowseBox::ColumnResized(sal_uInt16 nColId)
{
    if (static_cast<OQueryController*>(getDesignView()->getController())->isReadOnly())
        return;
    // The resizing of columns can't be suppressed (BrowseBox doesn't support that) so we have to do this
    // fake. It's not _that_ bad : the user may change column widths while in read-only mode to see all details
    // but the changes aren't permanent ...

    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    USHORT nPos = GetColumnPos(nColId);
    DBG_ASSERT(nPos <= getFields().size(),"ColumnResized:: nColId sollte nicht groesser als List::count sein!");
    OTableFieldDescRef pEntry = getEntry(nPos-1);
    DBG_ASSERT(pEntry.isValid(), "OSelectionBrowseBox::ColumnResized : keine FieldDescription !");
    static_cast<OQueryController*>(getDesignView()->getController())->setModified();
    EditBrowseBox::ColumnResized(nColId);

    if (pEntry.isValid())
    {
        // create the undo action
        OTabFieldSizedUndoAct* pUndo = new OTabFieldSizedUndoAct(this);
        pUndo->SetColumnPosition( nPos );
        pUndo->SetOriginalWidth(pEntry->GetColWidth());
        getDesignView()->getController()->addUndoActionAndInvalidate(pUndo);
        pEntry->SetColWidth(sal_uInt16(GetColumnWidth(nColId)));
    }
}

//------------------------------------------------------------------------------
sal_uInt32 OSelectionBrowseBox::GetTotalCellWidth(long nRowId, sal_uInt16 nColId)
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    USHORT nPos = GetColumnPos(nColId);
    DBG_ASSERT((nPos == 0) || (nPos <= getFields().size()), "OSelectionBrowseBox::GetTotalCellWidth : invalid parameter nColId");

    OTableFieldDescRef pEntry = getFields()[nPos-1];
    DBG_ASSERT(pEntry.isValid(), "OSelectionBrowseBox::GetTotalCellWidth : invalid FieldDescription !");

    long nRow = GetRealRow(nRowId);
    String strText(GetCellText(nRow, nColId));
    return GetDataWindow().LogicToPixel(Size(GetDataWindow().GetTextWidth(strText),0)).Width();
}

//------------------------------------------------------------------------------
sal_uInt16 OSelectionBrowseBox::GetDefaultColumnWidth(const String& rName) const
{
    DBG_CHKTHIS(OSelectionBrowseBox,NULL);
    // die Baissklasse macht das von dem Text abhaengig, ich habe aber keine Spaltenueberschriften, daher haette ich
    // hier gern einen anderen Default-Wert
    return DEFAULT_SIZE;
}
//------------------------------------------------------------------------------
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
            bCutAllowed = m_pFieldCell->GetSelected().Len() != 0;
            break;
        default:
            bCutAllowed = m_pTextCell->GetSelected().Len() != 0;
            break;
    }
    return bCutAllowed;
}
// -----------------------------------------------------------------------------
void OSelectionBrowseBox::cut()
{
    String sOldValue = GetCellContents(GetRealRow(GetCurRow()),GetCurColumnId());
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
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
sal_Bool OSelectionBrowseBox::isCopyAllowed()
{
    return isCutAllowed();
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
void OSelectionBrowseBox::appendUndoAction(const String& _rOldValue,const String& _rNewValue,sal_Int32 _nRow,sal_Bool& _bListAction)
{
    if ( !_rNewValue.Equals(_rOldValue) )
    {
        if ( !_bListAction )
        {
            _bListAction = sal_True;
            static_cast<OQueryController*>(getDesignView()->getController())->getUndoMgr()->EnterListAction(String(),String());
        }
        appendUndoAction(_rOldValue,_rNewValue,_nRow);
    }
}
// -----------------------------------------------------------------------------
void OSelectionBrowseBox::appendUndoAction(const String& _rOldValue,const String& _rNewValue,sal_Int32 _nRow)
{
    if ( !_rNewValue.Equals(_rOldValue) )
    {
        OTabFieldCellModifiedUndoAct* pUndoAct = new OTabFieldCellModifiedUndoAct(this);
        pUndoAct->SetCellIndex(_nRow);
        OSL_ENSURE(GetColumnPos(GetCurColumnId()) != BROWSER_INVALIDID,"Current position isn't valid!");
        pUndoAct->SetColumnPosition( GetColumnPos(GetCurColumnId()) );
        pUndoAct->SetCellContents(_rOldValue);
        getDesignView()->getController()->addUndoActionAndInvalidate(pUndoAct);
    }
}
// -----------------------------------------------------------------------------
IMPL_LINK(OSelectionBrowseBox, OnInvalidateTimer, void*, EMPTYARG)
{
    static_cast<OQueryController*>(getDesignView()->getController())->InvalidateFeature(SID_CUT);
    static_cast<OQueryController*>(getDesignView()->getController())->InvalidateFeature(SID_COPY);
    static_cast<OQueryController*>(getDesignView()->getController())->InvalidateFeature(SID_PASTE);
    if(!m_bStopTimer)
        m_timerInvalidate.Start();
    return 0L;
}
// -----------------------------------------------------------------------------
void OSelectionBrowseBox::stopTimer()
{
    m_bStopTimer = sal_True;
    if (m_timerInvalidate.IsActive())
        m_timerInvalidate.Stop();
}
// -----------------------------------------------------------------------------
void OSelectionBrowseBox::startTimer()
{
    m_bStopTimer = sal_False;
    if (!m_timerInvalidate.IsActive())
        m_timerInvalidate.Start();
}
// -----------------------------------------------------------------------------
OTableFields& OSelectionBrowseBox::getFields() const
{
    OQueryController* pController = static_cast<OQueryController*>(getDesignView()->getController());
    return pController->getTableFieldDesc();
}
// -----------------------------------------------------------------------------
void OSelectionBrowseBox::enableControl(const OTableFieldDescRef& _rEntry,Window* _pControl)
{
    BOOL bEnable = !_rEntry->isCondition();
    _pControl->Enable(bEnable);
    _pControl->EnableInput(bEnable);
}
// -----------------------------------------------------------------------------
void OSelectionBrowseBox::setTextCellContext(const OTableFieldDescRef& _rEntry,const String& _sText,ULONG _nHelpId)
{
    m_pTextCell->SetText(_sText);
    m_pTextCell->ClearModifyFlag();
    if (!m_pTextCell->HasFocus())
        m_pTextCell->GrabFocus();

    enableControl(_rEntry,m_pTextCell);

    if (m_pTextCell->GetHelpId() != _nHelpId)
        // da TextCell in verschiedenen Kontexten verwendet wird, muss ich den gecachten HelpText loeschen
        m_pTextCell->SetHelpText(String());
    m_pTextCell->SetHelpId(_nHelpId);
}
// -----------------------------------------------------------------------------
void OSelectionBrowseBox::invalidateUndoRedo()
{
    OQueryController* pController = static_cast<OQueryController*>(static_cast<OQueryController*>(getDesignView()->getController()));
    pController->InvalidateFeature( ID_BROWSER_UNDO );
    pController->InvalidateFeature( ID_BROWSER_REDO );
    pController->InvalidateFeature( ID_BROWSER_QUERY_EXECUTE );
}
// -----------------------------------------------------------------------------
OTableFieldDescRef OSelectionBrowseBox::getEntry(OTableFields::size_type _nPos)
{
    // we have to check if we need a new entry at this position
    OTableFields& aFields = getFields();
    OSL_ENSURE(aFields.size() > _nPos,"ColID is to great!");

    OTableFieldDescRef pEntry = aFields[_nPos];
    OSL_ENSURE(pEntry.isValid(),"Invalid entry!");
    if ( !pEntry.isValid() )
    {
        pEntry = new OTableFieldDesc();
        pEntry->SetColumnId(GetColumnId(_nPos+1));
        aFields[_nPos] = pEntry;
    }
    return pEntry;
}
// -----------------------------------------------------------------------------
void OSelectionBrowseBox::GetFocus()
{
    if(!IsEditing() && !m_bWasEditing)
        ActivateCell();
    EditBrowseBox::GetFocus();
}
// -----------------------------------------------------------------------------
void OSelectionBrowseBox::DeactivateCell(sal_Bool _bUpdate)
{
    m_bWasEditing = sal_True;
    EditBrowseBox::DeactivateCell(_bUpdate);
    m_bWasEditing = sal_False;
}
// -----------------------------------------------------------------------------
::rtl::OUString OSelectionBrowseBox::GetRowDescription( sal_Int32 _nRow ) const
{
    String  aLabel(ModuleRes(STR_QUERY_HANDLETEXT));

    // ab BROW_CRIT2_ROW werden alle Zeilen mit "oder" angegeben
    xub_StrLen nToken = (xub_StrLen) (_nRow >= GetBrowseRow(BROW_CRIT2_ROW))
                                ?
            xub_StrLen(BROW_CRIT2_ROW) : xub_StrLen(GetRealRow(_nRow));
    return ::rtl::OUString(aLabel.GetToken(nToken));
}
// -----------------------------------------------------------------------------
::rtl::OUString OSelectionBrowseBox::GetAccessibleName( ::svt::AccessibleBrowseBoxObjType _eObjType,sal_Int32 _nPosition) const
{
    ::rtl::OUString sRetText;
    switch( _eObjType )
    {
        case ::svt::BBTYPE_ROWHEADERCELL:
            sRetText = GetRowDescription(_nPosition);
            break;
        default:
            sRetText = EditBrowseBox::GetAccessibleDescription(_eObjType,_nPosition);
    }
    return sRetText;
}
// -----------------------------------------------------------------------------
sal_Bool OSelectionBrowseBox::fillEntryTable(OTableFieldDescRef& _pEntry,const ::rtl::OUString& _sTableName)
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
                _pEntry->SetDatabase(pEntryTab->GetComposedName());
                _pEntry->SetTable(pEntryTab->GetTableName());
                _pEntry->SetTabWindow(pEntryTab);
                bRet = sal_True;
            }
        }
    }
    return bRet;
}
// -----------------------------------------------------------------------------
void OSelectionBrowseBox::setFunctionCell(OTableFieldDescRef& _pEntry)
{
    Reference< XConnection> xConnection = static_cast<OQueryController*>(getDesignView()->getController())->getConnection();
    if ( xConnection.is() )
    {
        Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();

        // Diese Funktionen stehen nur unter CORE zur Verf¸gung
        if ( xMetaData->supportsCoreSQLGrammar() )
        {
            // if we have an asterix, no other function than count is allowed
            m_pFunctionCell->Clear();
            m_pFunctionCell->InsertEntry(m_aFunctionStrings.GetToken(0));
            if ( isFieldNameAsterix(_pEntry->GetField()) )
                m_pFunctionCell->InsertEntry(m_aFunctionStrings.GetToken(2)); // 2 -> COUNT
            else
            {
                xub_StrLen nCount = m_aFunctionStrings.GetTokenCount();
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
            // nur COUNT(*) erlaubt
            sal_Bool bCountRemoved = sal_False;
            if ( bCountRemoved = !isFieldNameAsterix(_pEntry->GetField()) )
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
// -----------------------------------------------------------------------------
Reference< XAccessible > OSelectionBrowseBox::CreateAccessibleCell( sal_Int32 _nRow, sal_uInt16 _nColumnPos )
{
    OTableFieldDescRef pEntry = NULL;
    if(getFields().size() > sal_uInt16(_nColumnPos - 1))
        pEntry = getFields()[_nColumnPos - 1];

    if ( _nRow == BROW_VIS_ROW && pEntry.isValid() )
        return EditBrowseBox::CreateAccessibleCheckBoxCell( _nRow, _nColumnPos,pEntry->IsVisible() ? STATE_CHECK : STATE_NOCHECK );

    return EditBrowseBox::CreateAccessibleCell( _nRow, _nColumnPos );
}
// -----------------------------------------------------------------------------
