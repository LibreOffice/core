/*************************************************************************
 *
 *  $RCSfile: QueryDesignView.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-20 08:13:25 $
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
#ifndef DBAUI_QUERYDESIGNVIEW_HXX
#include "QueryDesignView.hxx"
#endif
#ifndef DBAUI_QUERYTABLEVIEW_HXX
#include "QueryTableView.hxx"
#endif
#ifndef DBAUI_QUERY_TABLEWINDOW_HXX
#include "QTableWindow.hxx"
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef DBAUI_QUERYCONTROLLER_HXX
#include "querycontroller.hxx"
#endif
#ifndef _SV_SPLIT_HXX
#include <vcl/split.hxx>
#endif
#ifndef _UNDO_HXX
#include <svtools/undo.hxx>
#endif
#ifndef DBAUI_QYDLGTAB_HXX
#include "adtabdlg.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef DBAUI_QUERYDESIGN_OSELECTIONBROWSEBOX_HXX
#include "SelectionBrowseBox.hxx"
#endif
#ifndef DBAUI_TABLEFIELDDATA_HXX
#include "TableFieldData.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_XLOCALEDATA_HPP_
#include <com/sun/star/i18n/XLocaleData.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef DBAUI_QUERYTABLECONNECTION_HXX
#include "QTableConnection.hxx"
#endif
#ifndef DBAUI_CONNECTIONLINE_HXX
#include "ConnectionLine.hxx"
#endif
#ifndef DBAUI_CONNECTIONLINEDATA_HXX
#include "ConnectionLineData.hxx"
#endif
#ifndef DBAUI_QTABLECONNECTIONDATA_HXX
#include "QTableConnectionData.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif

using namespace ::dbaui;
using namespace ::utl;
using namespace ::connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

OQueryDesignView::OQueryDesignView(Window* _pParent, OQueryController* _pController,const Reference< XMultiServiceFactory >& _rFactory)
    :OQueryView(_pParent,_pController,_rFactory)
    ,m_aSplitter( this )
{
    try
    {
        Any aValue = ConfigManager::GetDirectConfigProperty(ConfigManager::LOCALE);
        m_aLocale.Language = ::comphelper::getString(aValue);
        Reference< XLocaleData> xLocaleData = Reference<XLocaleData>(_rFactory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.i18n.LocaleData")),UNO_QUERY);
        LocaleDataItem aData = xLocaleData->getLocaleItem(m_aLocale);
        m_sDecimalSep = aData.decimalSeparator;
    }
    catch(Exception&)
    {
    }

    m_pSelectionBox = new OSelectionBrowseBox(this);
    m_pSelectionBox->SetNoneVisbleRow(static_cast<OQueryController*>(getController())->getVisibleRows());
    m_pSelectionBox->Show();
    // Splitter einrichten
    m_aSplitter.SetSplitHdl(LINK(this, OQueryDesignView,SplitHdl));
    m_aSplitter.Show();

}
// -----------------------------------------------------------------------------
OQueryDesignView::~OQueryDesignView()
{
    delete m_pSelectionBox;
}
//------------------------------------------------------------------------------
IMPL_LINK( OQueryDesignView, SplitHdl, void*, p )
{
    if (!static_cast<OQueryController*>(getController())->isReadOnly())
    {
        long nTest = m_aSplitter.GetPosPixel().Y();
        m_aSplitter.SetPosPixel( Point( m_aSplitter.GetPosPixel().X(),m_aSplitter.GetSplitPosPixel() ) );
        if(!static_cast<OQueryController*>(getController())->isReadOnly())
        {
            static_cast<OQueryController*>(getController())->setSplitPos(m_aSplitter.GetSplitPosPixel());
            static_cast<OQueryController*>(getController())->setModified();
        }
        Resize();
    }
    return 0L;
}
// -------------------------------------------------------------------------
void OQueryDesignView::Construct(const Reference< ::com::sun::star::awt::XControlModel >& xModel)
{
    m_pTableView    = new OQueryTableView(m_pScrollWindow,this);
    OQueryView::Construct(xModel); // initialize m_xMe
}
// -----------------------------------------------------------------------------
void OQueryDesignView::initialize()
{
    if(static_cast<OQueryController*>(getController())->getSplitPos() != -1)
    {
        m_aSplitter.SetPosPixel( Point( m_aSplitter.GetPosPixel().X(),static_cast<OQueryController*>(getController())->getSplitPos() ) );
        m_aSplitter.SetSplitPosPixel(static_cast<OQueryController*>(getController())->getSplitPos());
    }
    m_pSelectionBox->initialize();
    m_pSelectionBox->ClearAll();
    m_pSelectionBox->SetReadOnly(static_cast<OQueryController*>(getController())->isReadOnly());
    m_pSelectionBox->Fill();
}
// -------------------------------------------------------------------------
void OQueryDesignView::resizeControl(Rectangle& _rRect)
{
    Window::Resize();

    Size aSize = GetOutputSizePixel();

    sal_Int32 nSplitPos = static_cast<OQueryController*>(getController())->getSplitPos();
    if( nSplitPos == -1 || nSplitPos >= aSize.Height())
    {
        nSplitPos = sal_Int32(aSize.Height()*0.5);
        static_cast<OQueryController*>(getController())->setSplitPos(nSplitPos);
    }

    Size aToolBoxSize;
    ToolBox* pToolBox = getToolBox();
    if(pToolBox)
        aToolBoxSize = pToolBox->GetOutputSizePixel();
    Point aTopLeft(_rRect.TopLeft());
    aTopLeft.Y() += aToolBoxSize.Height();

    Point   aSplitPos(0,0);
    Size    aSplitSize(0,0);

    aSplitPos       = m_aSplitter.GetPosPixel();
    aSplitPos.Y()   = nSplitPos;
    aSplitSize      = m_aSplitter.GetOutputSizePixel();
    aSplitSize.Width() = aSize.Width();

    if( ( aSplitPos.Y() + aSplitSize.Height() ) > ( aSize.Height() ))
        aSplitPos.Y() = aSize.Height() - aSplitSize.Height();

    if( aSplitPos.Y() <= 0)
        aSplitPos.Y() = LogicToPixel( Size(0,sal_Int32(aSize.Height() * 0.2) ), MAP_APPFONT ).Height();

    Size aTableView(aSize.Width(),aSplitPos.Y()-aToolBoxSize.Height());
    m_pScrollWindow->SetPosSizePixel(aTopLeft,aTableView);

    Point aPos(0,aSplitPos.Y()+aSplitSize.Height());
    m_pSelectionBox->SetPosSizePixel(aPos,Size( aSize.Width(), aSize.Height() - aSplitSize.Height() - aSplitPos.Y() ));

    //set the size of the splitter
    m_aSplitter.SetPosSizePixel( aSplitPos, Size( aSize.Width(), aSplitSize.Height()) );
    m_aSplitter.SetDragRectPixel(   Rectangle( Point( 0, 0 ), aSize) );

    aToolBoxSize.Width() += _rRect.getWidth();
    _rRect.SetSize(aToolBoxSize);
}
// -----------------------------------------------------------------------------
void OQueryDesignView::setReadOnly(sal_Bool _bReadOnly)
{
    m_pSelectionBox->SetReadOnly(_bReadOnly);
}
// -----------------------------------------------------------------------------
void OQueryDesignView::clear()
{
    SfxUndoManager* pUndoMgr = static_cast<OQueryController*>(getController())->getUndoMgr();
    m_pSelectionBox->ClearAll(); // clear the whole selection
    m_pSelectionBox->Fill();    // fill with empty the fields
    m_pTableView->ClearAll();
}
// -----------------------------------------------------------------------------
void OQueryDesignView::setStatement(const ::rtl::OUString& _rsStatement)
{
}
// -----------------------------------------------------------------------------
void OQueryDesignView::copy()
{
}
// -----------------------------------------------------------------------------
BOOL OQueryDesignView::IsAddAllowed()
{
    return m_pTableView->IsAddAllowed();
}
// -----------------------------------------------------------------------------
sal_Bool OQueryDesignView::isCutAllowed()
{
    return sal_False;
}
// -----------------------------------------------------------------------------
void OQueryDesignView::cut()
{
    static_cast<OQueryController*>(getController())->setModified(sal_True);
}
// -----------------------------------------------------------------------------
void OQueryDesignView::paste()
{
    static_cast<OQueryController*>(getController())->setModified(sal_True);
}
// -----------------------------------------------------------------------------
void OQueryDesignView::TableDeleted(const ::rtl::OUString& rAliasName)
{
    // Nachricht, dass Tabelle aus dem Fenster gel"oscht wurde
    DeleteFields(rAliasName);
    static_cast<OQueryController*>(getController())->InvalidateFeature(ID_BROWSER_ADDTABLE);    // view nochmal bescheid sagen
}
//------------------------------------------------------------------------------
void OQueryDesignView::DeleteFields( const ::rtl::OUString& rAliasName )
{
    m_pSelectionBox->DeleteFields( rAliasName );
}
// -----------------------------------------------------------------------------
void OQueryDesignView::SaveTabWinUIConfig(OQueryTableWindow* pWin)
{
    static_cast<OQueryController*>(getController())->SaveTabWinPosSize(pWin, m_pScrollWindow->GetHScrollBar()->GetThumbPos(), m_pScrollWindow->GetVScrollBar()->GetThumbPos());
}
// -----------------------------------------------------------------------------
sal_Bool OQueryDesignView::InsertField( const OTableFieldDesc& rInfo, sal_Bool bVis, sal_Bool bActivate)
{
    if(!m_pSelectionBox->InsertField( rInfo, -1,bVis, bActivate ))
    {
        ErrorBox aBox(this, ModuleRes( ERR_QRY_TOO_MANY_COLUMNS));
        aBox.Execute();
        return sal_False;
    }
    return sal_True;
}
// -----------------------------------------------------------------------------
sal_Bool OQueryDesignView::getColWidth( const ::rtl::OUString& rAliasName, const ::rtl::OUString& rFieldName, sal_uInt32& nWidth )
{
    ::std::vector<OTableFieldDesc*>::iterator aIter = static_cast<OQueryController*>(getController())->getTableFieldDesc()->begin();
    for(;aIter != static_cast<OQueryController*>(getController())->getTableFieldDesc()->end();++aIter)
    {
        if( rAliasName == (*aIter)->GetFieldAlias())
        {
            if( rFieldName == (*aIter)->GetField())
            {
                nWidth = (*aIter)->GetColWidth();
                return sal_True;
            }
        }
    }

    return sal_False;
}
//------------------------------------------------------------------------------
void OQueryDesignView::fillValidFields(const ::rtl::OUString& sAliasName, ComboBox* pFieldList)
{
    DBG_ASSERT(pFieldList != NULL, "OQueryDesignView::FillValidFields : What the hell do you think I can do with a NULL-ptr ? This will crash !");
    pFieldList->Clear();

    sal_Bool bAllTables = sAliasName.getLength() == 0;

    OJoinTableView::OTableWindowMap* pTabWins = m_pTableView->GetTabWinMap();
    ::rtl::OUString strCurrentPrefix;
    ::std::vector< ::rtl::OUString> aFields;
    OJoinTableView::OTableWindowMap::iterator aIter = pTabWins->begin();
    for(;aIter != pTabWins->end();++aIter)
    {
        OQueryTableWindow* pCurrentWin = static_cast<OQueryTableWindow*>(aIter->second);
        if (bAllTables || (pCurrentWin->GetAliasName() == sAliasName))
        {
            strCurrentPrefix = pCurrentWin->GetAliasName();
            strCurrentPrefix += ::rtl::OUString('.');

            pCurrentWin->EnumValidFields(aFields);

            ::std::vector< ::rtl::OUString>::iterator aStrIter = aFields.begin();
            for(;aStrIter != aFields.end();++aStrIter)
            {
                if (bAllTables || aStrIter->toChar() == '*')
                    pFieldList->InsertEntry(::rtl::OUString(strCurrentPrefix) += *aStrIter);
                else
                    pFieldList->InsertEntry(*aStrIter);
            }

            if (!bAllTables)
                // das heisst, dass ich in diesen Block kam, weil der Tabellenname genau der gesuchte war, also bin ich fertig
                // (dadurch verhindere ich auch das doppelte Einfuegen von Feldern, wenn eine Tabelle mehrmals als TabWin vorkommt)
                break;
        }
    }
}
// -----------------------------------------------------------------------------
long OQueryDesignView::PreNotify(NotifyEvent& rNEvt)
{
    BOOL bHandled = FALSE;
    switch (rNEvt.GetType())
    {
        case EVENT_KEYINPUT:
        {
            const KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
            const KeyCode& rCode = pKeyEvent->GetKeyCode();
            if (rCode.IsMod1() || rCode.IsMod2())
                break;
            if (rCode.GetCode() != KEY_F6)
                break;

            if (m_pTableView && m_pTableView->HasChildPathFocus())
            {
                if (m_pSelectionBox)
                {
                    m_pSelectionBox->GrabFocus();
                    bHandled = TRUE;
                }
            }
            else if (m_pSelectionBox && m_pSelectionBox->HasChildPathFocus())
            {
                if (m_pTableView)
                {
                    m_pTableView->GrabTabWinFocus();
                    bHandled = TRUE;
                }
            }
        }
        break;
        case EVENT_GETFOCUS:
        {
            // set focus if noone has the focus
            if (m_pTableView && !m_pTableView->HasChildPathFocus() &&
                m_pSelectionBox && !m_pSelectionBox->HasChildPathFocus())
            {
                m_pTableView->GrabTabWinFocus();
                bHandled = TRUE;
            }

        }
        break;
    }

    if (!bHandled)
        return OQueryView::PreNotify(rNEvt);
    return 1L;
}
// -----------------------------------------------------------------------------
sal_Bool OQueryDesignView::HasFields()
{
    ::std::vector<OTableFieldDesc*>* pList = static_cast<OQueryController*>(getController())->getTableFieldDesc();
    ::std::vector<OTableFieldDesc*>::iterator aIter = pList->begin();
    ::rtl::OUString aFieldName;
    for(;aIter != pList->end();++aIter)
    {
        aFieldName = (*aIter)->GetField();
        if (aFieldName.getLength() != 0 && (*aIter)->IsVisible())
            return sal_True;
    }
    return sal_False;
}
static const ::rtl::OUString C_AND = ::rtl::OUString::createFromAscii(" AND ");
static const ::rtl::OUString C_OR  = ::rtl::OUString::createFromAscii(" OR ");
//------------------------------------------------------------------------------
extern ::rtl::OUString ConvertAlias(const ::rtl::OUString& rName);
//------------------------------------------------------------------------------
::rtl::OUString OQueryDesignView::BuildTable(const OQueryTableWindow* pEntryTab)
{
    ::rtl::OUString aDBName(pEntryTab->GetComposedName());

    Reference< XConnection> xConnection = static_cast<OQueryController*>(getController())->getConnection();
    if(!xConnection.is())
        return aDBName;

    Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
    ::rtl::OUString aCatalog,aSchema,aTable,aComposedName;
    ::dbtools::qualifiedNameComponents(xMetaData,aDBName,aCatalog,aSchema,aTable);
    ::dbtools::composeTableName(xMetaData,aCatalog,aSchema,aTable,aComposedName,sal_True);

    ::rtl::OUString aQuote = xMetaData->getIdentifierQuoteString();
    ::rtl::OUString aTableListStr(aComposedName);
    aTableListStr += ::rtl::OUString(' ');
    aTableListStr += ::dbtools::quoteName(aQuote, ConvertAlias(pEntryTab->GetAliasName())).getStr();
    return aTableListStr;
}
//------------------------------------------------------------------------------
::rtl::OUString OQueryDesignView::BuildJoin(OQueryTableWindow* pLh, OQueryTableWindow* pRh, OQueryTableConnectionData* pData)
{
    return BuildJoin(BuildTable(pLh),BuildTable(pRh),pData);
}
//------------------------------------------------------------------------------
::rtl::OUString OQueryDesignView::BuildJoin(const ::rtl::OUString &rLh, OQueryTableWindow* pRh, OQueryTableConnectionData* pData)
{
    return BuildJoin(rLh,BuildTable(pRh),pData);
}
//------------------------------------------------------------------------------
::rtl::OUString OQueryDesignView::BuildJoin(OQueryTableWindow* pLh, const ::rtl::OUString &rRh, OQueryTableConnectionData* pData)
{
    return BuildJoin(BuildTable(pLh),rRh,pData);
}
//------------------------------------------------------------------------------
::rtl::OUString OQueryDesignView::BuildJoin(const ::rtl::OUString& rLh, const ::rtl::OUString& rRh, OQueryTableConnectionData* pData)
{

    String aErg(rLh);
    switch(pData->GetJoinType())
    {
        case LEFT_JOIN:
            aErg.AppendAscii(" LEFT OUTER ");
            break;
        case RIGHT_JOIN:
            aErg.AppendAscii(" RIGHT OUTER ");
            break;
        case INNER_JOIN:
            DBG_ERROR("OQueryDesignView::BuildJoin: This should not happen!");
            //aErg.AppendAscii(" INNER ");
            break;
        default:
            aErg.AppendAscii(" FULL OUTER ");
            break;
    }
    aErg.AppendAscii("JOIN ");
    aErg += String(rRh);
    aErg.AppendAscii(" ON ");
    aErg += String(BuildJoinCriteria(pData->GetConnLineDataList(),pData));

    return aErg;
}
//------------------------------------------------------------------------------
::rtl::OUString OQueryDesignView::BuildJoinCriteria(::std::vector<OConnectionLineData*>* pLineDataList,OQueryTableConnectionData* pData)
{
    ::rtl::OUString aCondition;
    Reference< XConnection> xConnection = static_cast<OQueryController*>(getController())->getConnection();
    if(!xConnection.is())
        return aCondition;

    ::std::vector<OConnectionLineData*>::iterator aIter = pLineDataList->begin();
    Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
    ::rtl::OUString aQuote = xMetaData->getIdentifierQuoteString();

    for(;aIter != pLineDataList->end();++aIter)
    {
        OConnectionLineData* pLineData = *aIter;
        if(pLineData->IsValid())
        {
            if(aCondition.getLength())
                aCondition += C_AND;
            aCondition += ::dbtools::quoteName(aQuote, ConvertAlias( pData->GetAliasName(JTCS_FROM) )).getStr();
            aCondition += ::rtl::OUString('.');
            aCondition += ::dbtools::quoteName(aQuote, pLineData->GetFieldName(JTCS_FROM) ).getStr();
            aCondition += ::rtl::OUString::createFromAscii(" = ");
            aCondition += ::dbtools::quoteName(aQuote, ConvertAlias( pData->GetAliasName(JTCS_TO) )).getStr();
            aCondition += ::rtl::OUString('.');
            aCondition += ::dbtools::quoteName(aQuote, pLineData->GetFieldName(JTCS_TO) ).getStr();
        }
    }

    return aCondition;
}
//------------------------------------------------------------------------------
::rtl::OUString OQueryDesignView::GenerateSelectList(::std::vector<OTableFieldDesc*>*   _pFieldList,sal_Bool bAlias)
{

    ::rtl::OUString aTmpStr,aFieldListStr;

    sal_Bool bAsterix = sal_False;
    int nVis = 0;
    ::std::vector<OTableFieldDesc*>::iterator aIter = _pFieldList->begin();
    for(;aIter != _pFieldList->end();++aIter)
    {
        OTableFieldDesc* pEntryField = *aIter;
        if(pEntryField->IsVisible())
        {
            if(pEntryField->GetField().toChar() == '*')
                bAsterix = sal_True;
            nVis++;
        }
    }
    if(nVis == 1)
        bAsterix = sal_False;

    Reference< XConnection> xConnection = static_cast<OQueryController*>(getController())->getConnection();
    if(!xConnection.is())
        return aFieldListStr;

    Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
    ::rtl::OUString aQuote = xMetaData->getIdentifierQuoteString();

    aIter = _pFieldList->begin();
    for(;aIter != _pFieldList->end();++aIter)
    {
        OTableFieldDesc* pEntryField = *aIter;
        ::rtl::OUString rFieldName = pEntryField->GetField();
        if (rFieldName.getLength() && pEntryField->IsVisible())
        {
            aTmpStr = ::rtl::OUString();
            ::rtl::OUString rAlias = pEntryField->GetAlias();
            ::rtl::OUString rFieldAlias = pEntryField->GetFieldAlias();
            if(bAlias && rAlias.getLength() || bAsterix)
            {
                aTmpStr += ::dbtools::quoteName(aQuote,ConvertAlias(rAlias));
                aTmpStr += ::rtl::OUString('.');
            }

            if(pEntryField->GetTable().getLength() && rFieldName.toChar() != '*')
                aTmpStr += ::dbtools::quoteName(aQuote, rFieldName).getStr();
            else
                aTmpStr += rFieldName;

            if(pEntryField->GetFunctionType() == FKT_AGGREGATE)
            {
                DBG_ASSERT(pEntryField->GetFunction().getLength(),"Functionname darf hier nicht leer sein! ;-(");
                ::rtl::OUString aTmpStr2(pEntryField->GetFunction());
                aTmpStr2 +=  ::rtl::OUString('(');
                aTmpStr2 += aTmpStr;
                aTmpStr2 +=  ::rtl::OUString(')');
                aTmpStr = aTmpStr2;
            }

            if(rFieldAlias.getLength() && rFieldName.toChar() != '*' )
            {
                aTmpStr += ::rtl::OUString::createFromAscii(" AS ");
                aTmpStr += ::dbtools::quoteName(aQuote, rFieldAlias);
            }
            aFieldListStr += aTmpStr;
            aFieldListStr += ::rtl::OUString::createFromAscii(", ");
        }
    }
    if(aFieldListStr.getLength())
        aFieldListStr = aFieldListStr.replaceAt(aFieldListStr.getLength()-2,2, ::rtl::OUString() );
    return aFieldListStr;
}
//------------------------------------------------------------------------------
void OQueryDesignView::JoinCycle(OQueryTableConnection* pEntryConn,::rtl::OUString &rJoin,OQueryTableWindow* pEntryTabTo)
{
    OQueryTableConnectionData* pData = static_cast<OQueryTableConnectionData*>(pEntryConn->GetData());
    if(pData->GetJoinType() == INNER_JOIN)
        return;
    if(pEntryTabTo->ExistsAVisitedConn()) // then we have a cycle
    {
        sal_Bool bBrace = sal_False;
        if(rJoin.getLength() && rJoin.lastIndexOf(')') == (rJoin.getLength()-1))
        {
            bBrace = sal_True;
            rJoin = rJoin.replaceAt(rJoin.getLength()-1,1,::rtl::OUString(' '));
        }
        (rJoin += C_AND) += BuildJoinCriteria(pData->GetConnLineDataList(),pData);
        if(bBrace)
            rJoin += ::rtl::OUString(')');
        pEntryConn->SetVisited(sal_True);
    }
}

//------------------------------------------------------------------------------
void OQueryDesignView::GenerateInnerJoinCriterias(::rtl::OUString& _rJoinCrit,const ::std::vector<OTableConnection*>* _pConnList)
{
    ::std::vector<OTableConnection*>::const_iterator aIter = _pConnList->begin();
    for(;aIter != _pConnList->end();++aIter)
    {
        const OQueryTableConnection* pEntryConn = static_cast<const OQueryTableConnection*>(*aIter);
        OQueryTableConnectionData* pEntryConnData = static_cast<OQueryTableConnectionData*>(pEntryConn->GetData());
        if(pEntryConnData->GetJoinType() == INNER_JOIN)
        {
            if(_rJoinCrit.getLength())
                _rJoinCrit += C_AND;
            _rJoinCrit += BuildJoinCriteria(pEntryConnData->GetConnLineDataList(),pEntryConnData);
        }
    }
}
//------------------------------------------------------------------------------
void OQueryDesignView::GetNextJoin(OQueryTableConnection* pEntryConn,::rtl::OUString &aJoin,OQueryTableWindow* pEntryTabTo)
{

    OQueryTableConnectionData* pEntryConnData = static_cast<OQueryTableConnectionData*>(pEntryConn->GetData());
    if(pEntryConnData->GetJoinType() == INNER_JOIN)
        return;

    if(!aJoin.getLength())
    {
        OQueryTableWindow* pEntryTabFrom = static_cast<OQueryTableWindow*>(pEntryConn->GetSourceWin());
        aJoin = BuildJoin(pEntryTabFrom,pEntryTabTo,pEntryConnData);
    }
    else if(pEntryTabTo == pEntryConn->GetDestWin())
    {
        ::rtl::OUString aTmpJoin('(');
        (aTmpJoin += aJoin) += ::rtl::OUString(')');
        aJoin = BuildJoin(aTmpJoin,pEntryTabTo,pEntryConnData);
    }
    else if(pEntryTabTo == pEntryConn->GetSourceWin())
    {
        ::rtl::OUString aTmpJoin('(');
        (aTmpJoin += aJoin) += ::rtl::OUString(')');
        aJoin = BuildJoin(pEntryTabTo,aTmpJoin,pEntryConnData);
    }

    pEntryConn->SetVisited(sal_True);

    // first search for the "to" window
    ::std::vector<OTableConnection*>::iterator aIter = pEntryConn->GetParent()->GetTabConnList()->begin();
    for(;aIter != pEntryConn->GetParent()->GetTabConnList()->end();++aIter)
    {
        OQueryTableConnection* pNext = static_cast<OQueryTableConnection*>(*aIter);
        if(!pNext->IsVisited() && (pNext->GetSourceWin() == pEntryTabTo || pNext->GetDestWin() == pEntryTabTo))
        {
            OQueryTableWindow* pEntryTab = pNext->GetSourceWin() == pEntryTabTo ? static_cast<OQueryTableWindow*>(pNext->GetDestWin()) : static_cast<OQueryTableWindow*>(pNext->GetSourceWin());
            // exists there a connection to a OQueryTableWindow that holds a connection that has been already visited
            JoinCycle(pNext,aJoin,pEntryTab);
            if(!pNext->IsVisited())
                GetNextJoin(pNext,aJoin,pEntryTab);
        }
    }

    // when nothing found found look for the "from" window
    if(aIter == pEntryConn->GetParent()->GetTabConnList()->end())
    {
        OQueryTableWindow* pEntryTabFrom = static_cast<OQueryTableWindow*>(pEntryConn->GetSourceWin());
        aIter = pEntryConn->GetParent()->GetTabConnList()->begin();
        for(;aIter != pEntryConn->GetParent()->GetTabConnList()->end();++aIter)
        {
            OQueryTableConnection* pNext = static_cast<OQueryTableConnection*>(*aIter);
            if(!pNext->IsVisited() && (pNext->GetSourceWin() == pEntryTabFrom || pNext->GetDestWin() == pEntryTabFrom))
            {
                OQueryTableWindow* pEntryTab = pNext->GetSourceWin() == pEntryTabFrom ? static_cast<OQueryTableWindow*>(pNext->GetDestWin()) : static_cast<OQueryTableWindow*>(pNext->GetSourceWin());
                // exists there a connection to a OQueryTableWindow that holds a connection that has been already visited
                JoinCycle(pNext,aJoin,pEntryTab);
                if(!pNext->IsVisited())
                    GetNextJoin(pNext,aJoin,pEntryTab);
            }
        }
    }
}

//------------------------------------------------------------------------------
::rtl::OUString OQueryDesignView::GenerateFromClause(const OQueryTableView::OTableWindowMap*    pTabList,::std::vector<OTableConnection*>*  pConnList)
{

    ::rtl::OUString aTableListStr;
    // wird gebraucht um sicher zustelllen das eine Tabelle nicht doppelt vorkommt

    // generate outer join clause in from
    if(pConnList->size())
    {
        ::std::vector<OTableConnection*>::iterator aIter = pConnList->begin();
        for(;aIter != pConnList->end();++aIter)
            static_cast<OQueryTableConnection*>(*aIter)->SetVisited(sal_False);

        aIter = pConnList->begin();
        for(;aIter != pConnList->end();++aIter)
        {
            OQueryTableConnection* pEntryConn = static_cast<OQueryTableConnection*>(*aIter);
            if(!pEntryConn->IsVisited())
            {
                ::rtl::OUString aJoin;
                GetNextJoin(pEntryConn,aJoin,static_cast<OQueryTableWindow*>(pEntryConn->GetDestWin()));
                if(aTableListStr.getLength())
                    aTableListStr += ::rtl::OUString(',');
                aTableListStr += aJoin;
            }
        }

        if(aTableListStr.getLength())
        {
            //  aTableListStr = aTableListStr.replaceAt(aTableListStr.getLength()-1,1, ::rtl::OUString() );
            ::rtl::OUString aStr = ::rtl::OUString::createFromAscii("{ OJ ");
            aStr += aTableListStr;
            aStr += ::rtl::OUString::createFromAscii(" },");
            aTableListStr = aStr;
        }
        // and now all inner joins
        map< ::rtl::OUString,sal_Bool,::comphelper::UStringMixLess> aTableNames;
        aIter = pConnList->begin();
        for(;aIter != pConnList->end();++aIter)
        {
            OQueryTableConnection* pEntryConn = static_cast<OQueryTableConnection*>(*aIter);
            if(!pEntryConn->IsVisited())
            {
                ::rtl::OUString aTabName(BuildTable(static_cast<OQueryTableWindow*>(pEntryConn->GetSourceWin())));
                if(aTableNames.find(aTabName) == aTableNames.end())
                {
                    aTableNames[aTabName] = sal_True;
                    aTableListStr += aTabName;
                    aTableListStr += ::rtl::OUString(',');
                }
                aTabName = BuildTable(static_cast<OQueryTableWindow*>(pEntryConn->GetDestWin()));
                if(aTableNames.find(aTabName) == aTableNames.end())
                {
                    aTableNames[aTabName] = sal_True;
                    aTableListStr += aTabName;
                    aTableListStr += ::rtl::OUString(',');
                }
            }
        }
    }
    // all tables that haven't a connection to anyone
    OQueryTableView::OTableWindowMap::const_iterator aTabIter = pTabList->begin();
    for(;aTabIter != pTabList->end();++aTabIter)
    {
        const OQueryTableWindow* pEntryTab = static_cast<const OQueryTableWindow*>(aTabIter->second);
        if(!pEntryTab->ExistsAConn())
        {
            aTableListStr += BuildTable(pEntryTab);
            aTableListStr += ::rtl::OUString(',');
        }
    }

    if(aTableListStr.getLength())
        aTableListStr = aTableListStr.replaceAt(aTableListStr.getLength()-1,1, ::rtl::OUString() );
    return aTableListStr;
}
//------------------------------------------------------------------------------
::rtl::OUString OQueryDesignView::getStatement()
{
    m_pSelectionBox->Save();    // Aktuelle eingabe uebernehmen

    // ----------------- Feldliste aufbauen ----------------------
    // erst die Felder zaehlen
    sal_uInt32 nFieldcount = 0;
    ::std::vector<OTableFieldDesc*>* pFieldList = static_cast<OQueryController*>(getController())->getTableFieldDesc();
    ::std::vector<OTableFieldDesc*>::iterator aIter = pFieldList->begin();
    for(;aIter != pFieldList->end();++aIter)
    {
        OTableFieldDesc* pEntryField = *aIter;
        if(pEntryField->GetField().getLength() && pEntryField->IsVisible())
            nFieldcount++;
    }
    if(!nFieldcount)    // keine Felder sichtbar also zur"uck
        return ::rtl::OUString();

    OQueryTableView::OTableWindowMap* pTabList   = m_pTableView->GetTabWinMap();
    sal_uInt32 nTabcount        = pTabList->size();

    ::rtl::OUString aFieldListStr(GenerateSelectList(pFieldList,nTabcount>1));
    if( !aFieldListStr.getLength() )
        return ::rtl::OUString();
    // Ausnahmebehandlung, wenn keine Felder angegeben worden sind
    // Dann darf die Tabpage nicht gewechselt werden
    // Im TabBarSelectHdl wird der SQL-::rtl::OUString auf STATEMENT_NOFIELDS abgefragt
    // und eine Errormeldung erzeugt
    // ----------------- Tabellenliste aufbauen ----------------------

    ::std::vector<OTableConnection*>*   pConnList  = m_pTableView->GetTabConnList();
    ::rtl::OUString aTableListStr(GenerateFromClause(pTabList,pConnList));
    DBG_ASSERT(aTableListStr.getLength(), "OQueryDesignView::GenerateSQL() : unerwartet : habe Felder, aber keine Tabellen !");
    // wenn es Felder gibt, koennen die nur durch Einfuegen aus einer schon existenten Tabelle entstanden sein; wenn andererseits
    // eine Tabelle geloescht wird, verschwinden auch die zugehoerigen Felder -> ergo KANN es das nicht geben, dass Felder
    // existieren, aber keine Tabellen (und aFieldListStr hat schon eine Laenge, das stelle ich oben sicher)
    ::rtl::OUString aHavingStr,aCriteriaListStr;
    // ----------------- Kriterien aufbauen ----------------------
    if (!GenerateCriterias(aCriteriaListStr,aHavingStr,pFieldList, nTabcount > 1))
        return ::rtl::OUString();

    ::rtl::OUString aJoinCrit;
    GenerateInnerJoinCriterias(aJoinCrit,pConnList);
    if(aJoinCrit.getLength())
    {
        ::rtl::OUString aTmp = ::rtl::OUString::createFromAscii("( ");
        aTmp += aJoinCrit;
        aTmp += ::rtl::OUString::createFromAscii(" )");
        if(aCriteriaListStr.getLength())
        {
            aTmp += C_AND;
            aTmp += aCriteriaListStr;
        }
        aCriteriaListStr = aTmp;
    }
    // ----------------- Statement aufbauen ----------------------
    ::rtl::OUString aSqlCmd(::rtl::OUString::createFromAscii("SELECT "));
    if(static_cast<OQueryController*>(getController())->isDistinct())
        aSqlCmd += ::rtl::OUString::createFromAscii(" DISTINCT ");
    aSqlCmd += aFieldListStr;
    aSqlCmd += ::rtl::OUString::createFromAscii(" FROM ");
    aSqlCmd += aTableListStr;

    if (aCriteriaListStr.getLength())
    {
        aSqlCmd += ::rtl::OUString::createFromAscii(" WHERE ");
        aSqlCmd += aCriteriaListStr;
    }
    // ----------------- GroupBy aufbauen und Anh"angen ------------
    aSqlCmd += GenerateGroupBy(pFieldList,nTabcount > 1);
    // ----------------- having Anh"angen ------------
    if(aHavingStr.getLength())
    {
        aSqlCmd += ::rtl::OUString::createFromAscii(" HAVING ");
        aSqlCmd += aHavingStr;
    }
    // ----------------- Sortierung aufbauen und Anh"angen ------------
    aSqlCmd += GenerateOrder(pFieldList,nTabcount > 1);

    return aSqlCmd;
}
//------------------------------------------------------------------------------
::rtl::OUString OQueryDesignView::GenerateGroupBy(::std::vector<OTableFieldDesc*>* pFieldList, sal_Bool bMulti )
{

    Reference< XConnection> xConnection = static_cast<OQueryController*>(getController())->getConnection();
    if(!xConnection.is())
        return ::rtl::OUString();

    Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
    ::rtl::OUString aQuote = xMetaData->getIdentifierQuoteString();

    ::rtl::OUString aGroupByStr;
    ::std::vector<OTableFieldDesc*>::iterator aIter = pFieldList->begin();
    for(;aIter != pFieldList->end();++aIter)
    {
        OTableFieldDesc*    pEntryField = *aIter;
        if(pEntryField->IsGroupBy())
        {
            DBG_ASSERT(pEntryField->GetField().getLength(),"Kein FieldName vorhanden!;-(");
            if (bMulti)
            {
                aGroupByStr += ::dbtools::quoteName(aQuote, ConvertAlias(pEntryField->GetAlias()));
                aGroupByStr += ::rtl::OUString('.');
            }

            aGroupByStr += ::dbtools::quoteName(aQuote, pEntryField->GetField());
            aGroupByStr += ::rtl::OUString(',');
        }
    }
    if(aGroupByStr.getLength())
    {
        aGroupByStr = aGroupByStr.replaceAt(aGroupByStr.getLength()-1,1, ::rtl::OUString(' ') );
        ::rtl::OUString aGroupByStr2 = ::rtl::OUString::createFromAscii(" GROUP BY ");
        aGroupByStr2 += aGroupByStr;
        aGroupByStr = aGroupByStr2;
    }
    return aGroupByStr;
}
//------------------------------------------------------------------------------
sal_Bool OQueryDesignView::GenerateCriterias(::rtl::OUString& rRetStr,::rtl::OUString& rHavingStr/*,::rtl::OUString& rOrderStr*/, ::std::vector<OTableFieldDesc*>* pFieldList, sal_Bool bMulti )
{

    DBG_ASSERT( pFieldList!=0, "OQueryDesignView::GenerateCriterias" );

    // * darf keine Filter enthalten : habe ich die entsprechende Warnung schon angezeigt ?
    sal_Bool bCritsOnAsterikWarning = sal_False;        // ** TMFS **

    ::rtl::OUString aFieldName,aCriteria,aWhereStr,aHavingStr,aWork/*,aOrderStr*/;
    // Zeilenweise werden die Ausdr"ucke mit AND verknuepft
    sal_uInt16 nMaxCriteria = 0;
    ::std::vector<OTableFieldDesc*>::iterator aIter = pFieldList->begin();
    for(;aIter != pFieldList->end();++aIter)
    {
        nMaxCriteria = ::std::max<sal_uInt16>(nMaxCriteria,(*aIter)->GetCriteria().size());
    }
    Reference< XConnection> xConnection = static_cast<OQueryController*>(getController())->getConnection();
    if(!xConnection.is())
        return FALSE;
    Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
    ::rtl::OUString aQuote = xMetaData->getIdentifierQuoteString();

    for (sal_uInt16 i=0 ; i < nMaxCriteria ; i++)
    {
        aHavingStr = aWhereStr = ::rtl::OUString();

        for(aIter = pFieldList->begin();aIter != pFieldList->end();++aIter)
        {
            OTableFieldDesc*    pEntryField = *aIter;
            aFieldName = pEntryField->GetField();

            if (!aFieldName.getLength())
                continue;
            aCriteria = pEntryField->GetCriteria( i );
            if (aCriteria.getLength())
            {
                if (aFieldName.toChar() == '*')             // * darf keine Filter besitzen
                {
                    // die entsprechende MessageBox nur beim ersten mal anzeigen
                    if (!bCritsOnAsterikWarning)
                        ErrorBox((OQueryDesignView*)this, ModuleRes( ERR_QRY_CRITERIA_ON_ASTERISK)).Execute();
                    bCritsOnAsterikWarning = sal_True;
                    continue;
                }
                aWork = ::rtl::OUString();

                if (bMulti)
                {
                    aWork += ::dbtools::quoteName(aQuote, ConvertAlias(pEntryField->GetAlias()));
                    aWork += ::rtl::OUString('.');
                }
                aWork += ::dbtools::quoteName(aQuote, aFieldName).getStr();
                if(pEntryField->GetFunctionType() == FKT_AGGREGATE || pEntryField->IsGroupBy())
                {
                    if (!aHavingStr.getLength())            // noch keine Kriterien
                        aHavingStr += ::rtl::OUString('(');         // Klammern
                    else
                        aHavingStr += C_AND;

                    if(pEntryField->GetFunctionType() == FKT_AGGREGATE)
                    {
                        aHavingStr += pEntryField->GetFunction();
                        aHavingStr += ::rtl::OUString('(');         // Klammern
                        aHavingStr += aWork;
                        aHavingStr += ::rtl::OUString(')');         // Klammern
                    }
                    else
                        aHavingStr += aWork;

                    aHavingStr += aCriteria;
                }
                else
                {
                    if (!aWhereStr.getLength())         // noch keine Kriterien
                        aWhereStr += ::rtl::OUString('(');          // Klammern
                    else
                        aWhereStr += C_AND;

                    aWhereStr += ::rtl::OUString(' ');
                    // aCriteria could have some german numbers so I have to be sure here
                    ::rtl::OUString aTmp(::rtl::OUString('='));
                    aTmp += aCriteria;
                    OQueryTableWindow* pWin = static_cast<OQueryTableWindow*>(pEntryField->GetTabWindow());

                    Reference<XPropertySet> xColumn;
                    if (pWin)
                    {
                        Reference<XNameAccess> xColumns = pWin->GetOriginalColumns();
                        if(xColumns->hasByName(aFieldName))
                            ::cppu::extractInterface(xColumn,xColumns->getByName(aFieldName));
                    }
                    ::rtl::OUString aErrorMsg;
                    ::connectivity::OSQLParser* pParser = static_cast<OQueryController*>(getController())->getParser();
                    ::connectivity::OSQLParseNode* pParseNode = pParser->predicateTree(aErrorMsg, aTmp, static_cast<OQueryController*>(getController())->getNumberFormatter(), xColumn);

                    if (pParseNode)
                    {
                        if (bMulti)
                            pParseNode->replaceNodeValue(ConvertAlias(pEntryField->GetAlias()),aFieldName);
                        ::rtl::OUString aWhere = aWhereStr;
                        pParseNode->parseNodeToStr( aWhere,
                                                    static_cast<OQueryController*>(getController())->getConnection()->getMetaData(),
                                                    &(static_cast<OQueryController*>(getController())->getParser()->getContext())
                                                    ,sal_False,sal_True);
                        aWhereStr = aWhere;
                        delete pParseNode;
                    }
                    else
                    {
                        aWhereStr += aWork;
                        aWhereStr += aCriteria;
                    }
                }
            }
            // nur einmal für jedes Feld
            else if(!i && pEntryField->GetFunctionType() == FKT_CONDITION)
            {
                if (!aWhereStr.getLength())         // noch keine Kriterien
                    aWhereStr += ::rtl::OUString('(');          // Klammern
                else
                    aWhereStr += C_AND;
                aWhereStr += pEntryField->GetField();
            }
        }
        if (aWhereStr.getLength())
        {
            aWhereStr += ::rtl::OUString(')');                      // Klammern zu fuer 'AND' Zweig
            if (rRetStr.getLength())                            // schon Feldbedingungen ?
                rRetStr += C_OR;
            else                                        // Klammern auf fuer 'OR' Zweig
                rRetStr += ::rtl::OUString('(');
            rRetStr += aWhereStr;
        }
        if (aHavingStr.getLength())
        {
            aHavingStr += ::rtl::OUString(')');                     // Klammern zu fuer 'AND' Zweig
            if (rHavingStr.getLength())                         // schon Feldbedingungen ?
                rHavingStr += C_OR;
            else                                        // Klammern auf fuer 'OR' Zweig
                rHavingStr += ::rtl::OUString('(');
            rHavingStr += aHavingStr;
        }
    }

    if (rRetStr.getLength())
        rRetStr += ::rtl::OUString(')');                                // Klammern zu fuer 'OR' Zweig
    if (rHavingStr.getLength())
        rHavingStr += ::rtl::OUString(')');                             // Klammern zu fuer 'OR' Zweig
    return sal_True;
}

//------------------------------------------------------------------------------

::rtl::OUString OQueryDesignView::GenerateOrder( ::std::vector<OTableFieldDesc*>* pFieldList,sal_Bool bMulti )
{

    ::rtl::OUString aRetStr, aColumnName;
    String aWorkStr;
    Reference< XConnection> xConnection = static_cast<OQueryController*>(getController())->getConnection();
    if(!xConnection.is())
        return aRetStr;

    Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
    ::rtl::OUString aQuote = xMetaData->getIdentifierQuoteString();
    // * darf keine Filter enthalten : habe ich die entsprechende Warnung schon angezeigt ?
    sal_Bool bCritsOnAsterikWarning = sal_False;        // ** TMFS **
    ::std::vector<OTableFieldDesc*>::iterator aIter = pFieldList->begin();

    for(;aIter != pFieldList->end();++aIter)
    {
        OTableFieldDesc*    pEntryField = *aIter;
        EOrderDir eOrder = pEntryField->GetOrderDir();

        // nur wenn eine Sortierung und ein Tabellenname vorhanden ist-> erzeugen
        // sonst werden die Expressions vom Order By im GenerateCriteria mit erzeugt
        if (eOrder != ORDER_NONE && pEntryField->GetTable().getLength())
        {
            aColumnName = pEntryField->GetField();
            if(aColumnName.toChar() == '*')
            {
                // die entsprechende MessageBox nur beim ersten mal anzeigen
                if (!bCritsOnAsterikWarning)
                    ErrorBox((OQueryDesignView*)this, ModuleRes( ERR_QRY_ORDERBY_ON_ASTERISK)).Execute();
                bCritsOnAsterikWarning = sal_True;
                continue;
            }
            if(pEntryField->GetFunctionType() == FKT_NONE)
            {
                if (bMulti && pEntryField->GetAlias().getLength())
                {
                    aWorkStr += ::dbtools::quoteName(aQuote, ConvertAlias(pEntryField->GetAlias())).getStr();
                    aWorkStr += String('.');
                }
                aWorkStr += ::dbtools::quoteName(aQuote, aColumnName).getStr();
            }
            else if(pEntryField->GetFieldAlias().getLength())
            {
                aWorkStr += ::dbtools::quoteName(aQuote, pEntryField->GetFieldAlias()).getStr();
            }
            else if(pEntryField->GetFunctionType() == FKT_AGGREGATE)
            {
                DBG_ASSERT(pEntryField->GetFunction().getLength(),"Functionname darf hier nicht leer sein! ;-(");
                aWorkStr += pEntryField->GetFunction().getStr();
                aWorkStr +=  String('(');
                if (bMulti && pEntryField->GetAlias().getLength())
                {
                    aWorkStr += ::dbtools::quoteName(aQuote, ConvertAlias(pEntryField->GetAlias())).getStr();
                    aWorkStr += String('.');
                }
                aWorkStr += ::dbtools::quoteName(aQuote, aColumnName).getStr();
                aWorkStr +=  String(')');
            }
            else
            {
                if (bMulti && pEntryField->GetAlias().getLength())
                {
                    aWorkStr += ::dbtools::quoteName(aQuote, ConvertAlias(pEntryField->GetAlias())).getStr();
                    aWorkStr += String('.');
                }
                aWorkStr += ::dbtools::quoteName(aQuote, aColumnName).getStr();
            }
            aWorkStr += String(' ');
            aWorkStr += String::CreateFromAscii( ";ASC;DESC" ).GetToken( eOrder );
            aWorkStr += String(',');
        }
    }

    aWorkStr.EraseTrailingChars( ',' );

    if( aWorkStr.Len() )
    {
        sal_Int32 nMaxOrder = xMetaData->getMaxColumnsInOrderBy();
        if(nMaxOrder && nMaxOrder < aWorkStr.GetTokenCount(','))
        {
            ErrorBox aBox( const_cast<OQueryDesignView*>(this), ModuleRes( ERR_QRY_TOO_LONG_STATEMENT ) );
            aBox.Execute();
        }
        else
        {
            aRetStr = ::rtl::OUString::createFromAscii(" ORDER BY ");
            aRetStr += aWorkStr;
        }
    }

    return aRetStr;
}

//------------------------------------------------------------------------------

::rtl::OUString OQueryDesignView::BuildACriteria( const ::rtl::OUString& _rVal, sal_Int32 aType )
{

    ::rtl::OUString aRetStr;
    String  aVal,rVal;
    String aOpList;aOpList.AssignAscii("<>;>=;<=;<;>;=;LIKE");
    xub_StrLen  nOpListCnt = aOpList.GetTokenCount();

    String aToken;
    for( xub_StrLen nIdx=0 ; nIdx < nOpListCnt ; nIdx++ )
    {
        aToken = aOpList.GetToken(nIdx);
        if (rVal.Search( aToken ) == 0)
        {
            aRetStr = ::rtl::OUString(' ');
            aRetStr += aToken;
            aRetStr += ::rtl::OUString(' ');
            aVal = rVal.Copy( aToken.Len() );
            aVal.EraseLeadingChars( ' ' );
            if( aVal.Search( '\'' ) == STRING_NOTFOUND )//XXX O'Brien???
            {
                aVal = QuoteField( aVal, aType );
            }
            aRetStr += aVal;
            break;
        }
    }

    if( !aRetStr.getLength()) //  == 0
    {
        aRetStr = rVal.Search( '%' ) == STRING_NOTFOUND ? ::rtl::OUString::createFromAscii(" = ") : ::rtl::OUString::createFromAscii(" LIKE ");
        aVal = rVal;
        if( aVal.Search( '\'' ) == STRING_NOTFOUND )//XXX O'Brien???
        {
            aVal = QuoteField( aVal, aType );
        }
        aRetStr += aVal;
    }

    return aRetStr;//XXX
}
// -----------------------------------------------------------------------------
sal_Int32 OQueryDesignView::GetColumnFormatKey(const ::connectivity::OSQLParseNode* pColumnRef)
{
    ::rtl::OUString aTableRange,aColumnName;
    ::connectivity::OSQLParseTreeIterator& rParseIter = static_cast<OQueryController*>(getController())->getParseIterator();
    rParseIter.getColumnRange( pColumnRef, aColumnName, aTableRange );

    OQueryTableWindow* pSTW = NULL;
    if (aTableRange.getLength())
        pSTW = static_cast<OQueryTableView*>(m_pTableView)->FindTable( aTableRange );
    else if(m_pTableView->GetTabWinMap()->size())
        pSTW = static_cast<OQueryTableWindow*>(m_pTableView->GetTabWinMap()->begin()->second);

    sal_Int32 nFormatKey = 0;
    if(pSTW)
    {
        Reference<XNameAccess> xColumns = pSTW->GetOriginalColumns();
        if(xColumns.is() && xColumns->hasByName(aColumnName))
        {
            Reference<XPropertySet> xColumn;
            ::cppu::extractInterface(xColumn,xColumns->getByName(aColumnName));
            OSL_ENSURE(xColumn.is(),"Column is null!");
            if(xColumn.is() && xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_NUMBERFORMAT))
                xColumn->getPropertyValue(PROPERTY_NUMBERFORMAT) >>= nFormatKey;
        }
    }
    return nFormatKey;
}
//------------------------------------------------------------------------------
sal_Bool OQueryDesignView::FillDragInfo(const ::connectivity::OSQLParseNode* pColumnRef,OTableFieldDesc& aDragInfo)
{

    sal_Bool bErg = sal_True;

    ::rtl::OUString aTableRange,aColumnName;
    sal_uInt16 nCntAccount;
    ::connectivity::OSQLParseTreeIterator& rParseIter = static_cast<OQueryController*>(getController())->getParseIterator();
    rParseIter.getColumnRange( pColumnRef, aColumnName, aTableRange );

    if (aTableRange.getLength())
    {
        OQueryTableWindow*  pSTW = static_cast<OQueryTableView*>(m_pTableView)->FindTable( aTableRange );
        if(!( pSTW && pSTW->ExistsField( aColumnName, aDragInfo ) ))
            bErg = sal_False;
    }
    else
        bErg = static_cast<OQueryTableView*>(m_pTableView)->FindTableFromField(aColumnName, aDragInfo, nCntAccount);

    return bErg;
}
// -----------------------------------------------------------------------------
int OQueryDesignView::GetSelectionCriteria( const ::connectivity::OSQLParseNode* pNode, int& rLevel, sal_Bool bJoinWhere)
{

    // Naechster freier Satz ...
    int nRet = 0;
    int nJoins=0;

    if (!SQL_ISRULE(pNode, select_statement))
    {
        ErrorBox    aBox( this, ModuleRes( ERR_QRY_NOSELECT ) );
        aBox.Execute();
        return 1;
    }

    // nyi: mehr Pruefung auf korrekte Struktur!
    pNode = pNode ? pNode->getChild(3)->getChild(1) : NULL;
    // Keine WHERE-Klausel.
    if (!pNode || pNode->isLeaf())
        return nRet;

    ::connectivity::OSQLParseNode * pCondition = pNode->getChild(1);
    if (!pCondition) // kein where clause
        return nRet;

    // jetzt die anderen Bedingungen eintragen
    ::connectivity::OSQLParseNode::negateSearchCondition(pCondition);

    ::connectivity::OSQLParseNode *pNodeTmp = pNode->getChild(1);
    ::connectivity::OSQLParseNode::disjunctiveNormalForm(pNodeTmp);
    pNodeTmp = pNode->getChild(1);
    ::connectivity::OSQLParseNode::absorptions(pNodeTmp);
    pNodeTmp = pNode->getChild(1);

    // first extract the inner joins conditions
    GetInnerJoinCriteria(pNodeTmp);

    // it could happen that pCondition is not more valid
    nRet = GetORCriteria(pNodeTmp, rLevel);

    if (nRet != 0)                                             // mindestens eine OR Verknuepfung
    {
        ErrorBox aBox( this, ModuleRes( ERR_QRY_TOOCOMPLEX ) );
        aBox.Execute();
        return 99;
    }
    return nRet;
}
//------------------------------------------------------------------------------
sal_Bool OQueryDesignView::GetInnerJoinCriteria(const ::connectivity::OSQLParseNode *pCondition)
{
    sal_Bool bFoundInnerJoin = sal_False;
    ::connectivity::OSQLParseNode *pTmp=NULL;
    bFoundInnerJoin = InsertJoinConnection(pCondition, INNER_JOIN);
    return bFoundInnerJoin;
}
//------------------------------------------------------------------------------

int OQueryDesignView::GetORCriteria(const ::connectivity::OSQLParseNode * pCondition, int& nLevel ,sal_Bool bHaving)
{

    int nRet = 0;

    // Runde Klammern um den Ausdruck
    if (pCondition->count() == 3 &&
        SQL_ISPUNCTUATION(pCondition->getChild(0),"(") &&
        SQL_ISPUNCTUATION(pCondition->getChild(2),")"))
    {
        nRet = GetORCriteria(pCondition->getChild(1),nLevel,bHaving);
    }
    // oder Verknuepfung
    // a searchcondition can only look like this: search_condition SQL_TOKEN_OR boolean_term
    else if (SQL_ISRULE(pCondition,search_condition))
    {
        for (int i = 0; i < 3; i+=2)
        {
            // Ist das erste Element wieder eine OR-Verknuepfung?
            // Dann rekursiv absteigen ...
            //if (!i && SQL_ISRULE(pCondition->getChild(i),search_condition))
            if (SQL_ISRULE(pCondition->getChild(i),search_condition))
            {
                nRet = GetORCriteria(pCondition->getChild(i),nLevel,bHaving);
            }
            else if (!nRet)
            {
                nRet = GetANDCriteria(pCondition->getChild(i), nLevel++,bHaving);
            }
            else
            {
                ErrorBox aBox( this, ModuleRes( ERR_QRY_TOOMANYCOND ) );
                aBox.Execute();
                return 1;
            }
        }
    }
    else
    {
        nRet = GetANDCriteria( pCondition, nLevel, bHaving );
    }
    return nRet;
}


//--------------------------------------------------------------------------------------------------
int OQueryDesignView::GetANDCriteria(const  ::connectivity::OSQLParseNode * pCondition, const int nLevel,sal_Bool bHaving )
{

    // ich werde ein paar Mal einen gecasteten Pointer auf meinen ::com::sun::star::sdbcx::Container brauchen
    int nRet = 0;

    // Runde Klammern
    if (SQL_ISRULE(pCondition,boolean_primary))
    {
        int nLevel2 = nLevel;
        nRet = GetORCriteria(pCondition->getChild(1), nLevel2,bHaving );
    }
    // Das erste Element ist (wieder) eine AND-Verknuepfung
    else if ( SQL_ISRULE(pCondition,boolean_term) && pCondition->count() == 3 )
    {
        nRet = GetANDCriteria(pCondition->getChild(0), nLevel,bHaving );
        if (!nRet)
            nRet = GetANDCriteria(pCondition->getChild(2), nLevel,bHaving );
    }
    else if (SQL_ISRULE( pCondition, comparison_predicate))
    {
        nRet = ComparsionPredicate(pCondition,nLevel,bHaving);
    }
    else if((SQL_ISRULE(pCondition,like_predicate)))
    {
        ::rtl::OUString aCondition;
        OTableFieldDesc aDragLeft;
        if(SQL_ISRULE(pCondition->getChild(0), column_ref ))
        {
            ::rtl::OUString aColumnName;
            // the international doesn't matter I have a string
            pCondition->parseNodeToPredicateStr(aCondition,static_cast<OQueryController*>(getController())->getConnection()->getMetaData(), static_cast<OQueryController*>(getController())->getNumberFormatter(),
                                        m_aLocale,
                                        m_sDecimalSep.toChar());
            pCondition->getChild(0)->parseNodeToPredicateStr(aColumnName,static_cast<OQueryController*>(getController())->getConnection()->getMetaData(), static_cast<OQueryController*>(getController())->getNumberFormatter(), m_aLocale,
                                        m_sDecimalSep.toChar());

            // don't display the column name
            aCondition = aCondition.copy(aColumnName.getLength());
            aCondition = aCondition.trim();

            if (FillDragInfo(pCondition->getChild(0),aDragLeft))
                m_pSelectionBox->AddCondition(aDragLeft, aCondition, nLevel);
            else
            {
                ErrorBox( this, ModuleRes( ERR_QRY_SYNTAX ) ).Execute();
                nRet = 5;
            }
        }
        else
        {
            ErrorBox( this, ModuleRes( ERR_QRY_SYNTAX ) ).Execute();
            nRet = 5;
        }
    }
    else if((SQL_ISRULE(pCondition,test_for_null) || SQL_ISRULE(pCondition,in_predicate) ||
                        SQL_ISRULE(pCondition,all_or_any_predicate) || SQL_ISRULE(pCondition,between_predicate)))
    {
        ::rtl::OUString aCondition;
        OTableFieldDesc aDragLeft;
        if(SQL_ISRULE(pCondition->getChild(0), column_ref ))
        {
            // Bedingung parsen
            for(sal_uInt16 i=1;i< pCondition->count();i++)
                pCondition->getChild(i)->parseNodeToPredicateStr(aCondition,
                                static_cast<OQueryController*>(getController())->getConnection()->getMetaData(),
                                static_cast<OQueryController*>(getController())->getNumberFormatter(),
                                m_aLocale,
                                m_sDecimalSep.toChar());
        }

        if (FillDragInfo(pCondition->getChild(0),aDragLeft))
            m_pSelectionBox->AddCondition(aDragLeft, aCondition, nLevel);
        else
        {
            ErrorBox( this, ModuleRes( ERR_QRY_SYNTAX ) ).Execute();
            nRet = 5;
        }
    }
    else if((SQL_ISRULE(pCondition,existence_test) || SQL_ISRULE(pCondition,unique_test)))
    {
        ::rtl::OUString aCondition;
        OTableFieldDesc aDragLeft;

        // Funktions-Bedingung parsen
        for(sal_uInt16 i=0;i< pCondition->count();i++)
            pCondition->getChild(i)->parseNodeToPredicateStr(aCondition,
                                        static_cast<OQueryController*>(getController())->getConnection()->getMetaData(),
                                        static_cast<OQueryController*>(getController())->getNumberFormatter(),
                                        m_aLocale,
                                        m_sDecimalSep.toChar());

        aDragLeft.SetField(aCondition);
        aDragLeft.SetFunctionType(FKT_CONDITION);

        m_pSelectionBox->InsertField(aDragLeft,-1,sal_False,sal_True);
    }
    else
    {
        // Etwas anderes unterstuetzen wir (noch) nicht. Basta!
        ErrorBox( this, ModuleRes( ERR_QRY_SYNTAX ) ).Execute();
        nRet = 5;
    }
    // Fehler einfach weiterreichen.
    return nRet;
}
//------------------------------------------------------------------------------
int OQueryDesignView::ComparsionPredicate(const ::connectivity::OSQLParseNode * pCondition, const int nLevel, sal_Bool bHaving )
{

    DBG_ASSERT(SQL_ISRULE( pCondition, comparison_predicate),"ComparsionPredicate: pCondition ist kein ComparsionPredicate");
    sal_uInt32 nRet = 0;
    if(SQL_ISRULE(pCondition->getChild(0), column_ref ) || SQL_ISRULE(pCondition->getChild(pCondition->count()-1), column_ref ))
    {
        ::rtl::OUString aCondition;
        OTableFieldDesc aDragLeft;
        sal_uInt32 nPos;
        if(SQL_ISRULE(pCondition->getChild(0), column_ref ) && SQL_ISRULE(pCondition->getChild(pCondition->count()-1), column_ref ))
        {
            OTableFieldDesc aDragRight;
            if (!FillDragInfo(pCondition->getChild(0),aDragLeft) ||
                !FillDragInfo(pCondition->getChild(2),aDragRight))
                return nRet;

            OQueryTableConnection* pConn = static_cast<OQueryTableConnection*>(m_pTableView->GetTabConn(static_cast<OQueryTableWindow*>(aDragLeft.GetTabWindow()),static_cast<OQueryTableWindow*>(aDragRight.GetTabWindow())));
            if(pConn)
            {
                ::std::vector<OConnectionLineData*>* pLineDataList = pConn->GetData()->GetConnLineDataList();
                ::std::vector<OConnectionLineData*>::iterator aIter = pLineDataList->begin();
                for(;aIter != pLineDataList->end();++aIter)
                {
                    if((*aIter)->GetSourceFieldName() == aDragLeft.GetField() ||
                       (*aIter)->GetDestFieldName() == aDragLeft.GetField() )
                        break;
                }
                if(aIter != pLineDataList->end())
                    return 0;
            }
        }

        if(SQL_ISRULE(pCondition->getChild(0), column_ref ))
        {
            nPos = 0;
            sal_uInt32 i=1;

            // don't display the equal
            if (pCondition->getChild(i)->getNodeType() == SQL_NODE_EQUAL)
                i++;

            //  International aInter(Shell()->BuildInternational(GetColumnFormatKey(pCondition->getChild(0))));
            // Bedingung parsen
            for(;i< pCondition->count();i++)
                pCondition->getChild(i)->parseNodeToPredicateStr(aCondition,
                                                                static_cast<OQueryController*>(getController())->getConnection()->getMetaData(),
                                                                static_cast<OQueryController*>(getController())->getNumberFormatter(),
                                                                m_aLocale,
                                                                m_sDecimalSep.toChar());
        }
        else if(SQL_ISRULE(pCondition->getChild(pCondition->count()-1), column_ref ))
        {
            nPos = pCondition->count()-1;

            sal_uInt32 i = pCondition->count() - 2;
            switch (pCondition->getChild(i)->getNodeType())
            {
                case SQL_NODE_EQUAL:
                    // don't display the equal
                    i--;
                    break;
                case SQL_NODE_LESS:
                    // take the opposite as we change the order
                    i--;
                    aCondition = aCondition + ::rtl::OUString::createFromAscii(">");
                    break;
                case SQL_NODE_LESSEQ:
                    // take the opposite as we change the order
                    i--;
                    aCondition = aCondition + ::rtl::OUString::createFromAscii(">=");
                    break;
                case SQL_NODE_GREAT:
                    // take the opposite as we change the order
                    i--;
                    aCondition = aCondition + ::rtl::OUString::createFromAscii("<");
                    break;
                case SQL_NODE_GREATEQ:
                    // take the opposite as we change the order
                    i--;
                    aCondition = aCondition + ::rtl::OUString::createFromAscii("<=");
                    break;
            }
            //  International aInter(Shell()->BuildInternational(GetColumnFormatKey(pCondition->getChild(pCondition->count()-1))));

            // go backward
            for (; i >= 0; i--)
                pCondition->getChild(i)->parseNodeToPredicateStr(aCondition,
                                        static_cast<OQueryController*>(getController())->getConnection()->getMetaData(),
                                        static_cast<OQueryController*>(getController())->getNumberFormatter(),
                                        m_aLocale,
                                        m_sDecimalSep.toChar());
        }
        if(FillDragInfo(pCondition->getChild(nPos),aDragLeft))
        {
            if(bHaving)
                aDragLeft.SetGroupBy(sal_True);
            m_pSelectionBox->AddCondition(aDragLeft, aCondition, nLevel);
        }
        else
        {
            ErrorBox( this, ModuleRes( ERR_QRY_SYNTAX ) ).Execute();
            nRet = 5;
        }
    }
    else if(SQL_ISRULE(pCondition->getChild(0), set_fct_spec ) || SQL_ISRULE(pCondition->getChild(0), general_set_fct ))
    {
        ::rtl::OUString aName,
                        aCondition;
        OTableFieldDesc aDragLeft;

        ::rtl::OUString aColumnName;
        pCondition->parseNodeToPredicateStr(aCondition,
                                            static_cast<OQueryController*>(getController())->getConnection()->getMetaData(),
                                            static_cast<OQueryController*>(getController())->getNumberFormatter(),
                                            m_aLocale,
                                            m_sDecimalSep.toChar());
        pCondition->getChild(0)->parseNodeToPredicateStr(aColumnName,
                                            static_cast<OQueryController*>(getController())->getConnection()->getMetaData(),
                                            static_cast<OQueryController*>(getController())->getNumberFormatter(),
                                            m_aLocale,
                                            m_sDecimalSep.toChar());

        // don't display the column name
        aCondition = aCondition.copy(aColumnName.getLength());
        aCondition = aCondition.trim();

        if(SQL_ISRULE(pCondition->getChild(0), general_set_fct ))
        {
            String aPara(aColumnName);
            if(!FillDragInfo(pCondition->getChild(0)->getChild(3),aDragLeft))
            {
                aPara = aPara.GetToken(1,'(');
                aDragLeft.SetField(aPara.GetToken(aPara.GetTokenCount()-1,')'));
            }
            aDragLeft.SetFunctionType(FKT_AGGREGATE);
            if(bHaving)
                aDragLeft.SetGroupBy(sal_True);
            aPara = aPara.GetToken(0,'(');
            aDragLeft.SetFunction(aPara);
        }
        else
        {
            // bei unbekannten Funktionen wird der gesamte Text in das Field gechrieben
            aDragLeft.SetField(aColumnName);
            if(bHaving)
                aDragLeft.SetGroupBy(sal_True);
            aDragLeft.SetFunctionType(FKT_OTHER);
        }
        m_pSelectionBox->AddCondition(aDragLeft, aCondition, nLevel);
    }
    else // kann sich nur um einen Expr. Ausdruck handeln
    {
        OTableFieldDesc aDragLeft;
        ::rtl::OUString aName,aCondition;

        ::connectivity::OSQLParseNode *pLhs = pCondition->getChild(0);
        ::connectivity::OSQLParseNode *pRhs = pCondition->getChild(2);
        // Feldnamen
        for(sal_uInt16 i=0;i< pLhs->count();i++)
            pCondition->getChild(i)->parseNodeToStr(aName,
                                        static_cast<OQueryController*>(getController())->getConnection()->getMetaData(),
                                        &static_cast<OQueryController*>(getController())->getParser()->getContext(),
                                        sal_True);
        // Kriterium
        aCondition = pCondition->getChild(1)->getTokenValue();
        for(i=0;i< pRhs->count();i++)
            pCondition->getChild(i)->parseNodeToPredicateStr(aCondition,
                                                        static_cast<OQueryController*>(getController())->getConnection()->getMetaData(),
                                                        static_cast<OQueryController*>(getController())->getNumberFormatter(),
                                                        m_aLocale,
                                                        m_sDecimalSep.toChar());

        aDragLeft.SetField(aName);
        aDragLeft.SetFunctionType(FKT_OTHER);
        // und anh"angen
        m_pSelectionBox->AddCondition(aDragLeft, aCondition, nLevel);
    }
    return nRet;
}
// -----------------------------------------------------------------------------
::rtl::OUString OQueryDesignView::QuoteField( const ::rtl::OUString& rValue, sal_Int32 aType )
{
    ::rtl::OUString rNewValue;
    switch (rValue.toChar())
    {
        case '?':
            if (rValue.getLength() != 1)
                break;
        case '\'':  // ::rtl::OUString Quotierung oder Datum
        //case '#': // Datumsquotierung // jetengine
        case ':':   // Parameter
        case '[':   // Parameter
            return rValue;
    }

    Reference< XDatabaseMetaData >  xMetaData = static_cast<OQueryController*>(getController())->getConnection()->getMetaData();
    ::rtl::OUString aQuote = xMetaData->getIdentifierQuoteString();

    switch( aType )
    {
        case DataType::DATE:
        case DataType::TIME:
        case DataType::TIMESTAMP:
            if (rValue.toChar() != '{') // nur quoten, wenn kein Access Datum
                rNewValue = ::dbtools::quoteName(aQuote,rValue);
            else
                rNewValue = rValue;
            break;
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::LONGVARCHAR:
            rNewValue = ::dbtools::quoteName(aQuote,rValue);
            break;
        case DataType::DECIMAL:
        case DataType::NUMERIC:
        case DataType::TINYINT:
        case DataType::SMALLINT:
        case DataType::INTEGER:
        case DataType::BIGINT:
        case DataType::REAL:
        case DataType::DOUBLE:
        case DataType::BINARY:
        case DataType::VARBINARY:
        case DataType::LONGVARBINARY:
            rNewValue = rValue;
            break;
        case DataType::BIT:
            {
                ::comphelper::UStringMixEqual bCase(xMetaData->storesMixedCaseQuotedIdentifiers());
                if (bCase(rValue, ::rtl::OUString(ModuleRes(STR_QUERY_TRUE))))
                    rNewValue = ::rtl::OUString::createFromAscii("TRUE");
                else if (bCase(rValue, ::rtl::OUString(ModuleRes(STR_QUERY_FALSE))))
                    rNewValue = ::rtl::OUString::createFromAscii("FALSE");
                else
                    rNewValue = rValue;
            }
            break;
        default:
            DBG_ERROR( "QuoteField: illegal type" );
        break;
    }
    return rNewValue;
}
// -----------------------------------------------------------------------------
sal_Bool OQueryDesignView::InsertJoinConnection(const ::connectivity::OSQLParseNode *pNode, const EJoinType& _eJoinType)
{
    if (pNode->count() == 3 &&  // Ausdruck is geklammert
        SQL_ISPUNCTUATION(pNode->getChild(0),"(") &&
        SQL_ISPUNCTUATION(pNode->getChild(2),")"))
    {
        return InsertJoinConnection(pNode->getChild(1), _eJoinType);
    }
    else if (SQL_ISRULE(pNode,search_condition) ||
             SQL_ISRULE(pNode,boolean_term) &&          // AND/OR-Verknuepfung:
             pNode->count() == 3)
    {
        // nur AND Verknüpfung zulassen
        if (!SQL_ISTOKEN(pNode->getChild(1),AND))
            return sal_False;
        else
            return InsertJoinConnection(pNode->getChild(0), _eJoinType) &&
                   InsertJoinConnection(pNode->getChild(2), _eJoinType);
    }
    else if (SQL_ISRULE(pNode,comparison_predicate))
    {
        // Nur ein Vergleich auf Spalten ist erlaubt
        DBG_ASSERT(pNode->count() == 3,"OQueryDesignView::InsertJoinConnection: Fehler im Parse Tree");
        if (!(SQL_ISRULE(pNode->getChild(0),column_ref) &&
              SQL_ISRULE(pNode->getChild(2),column_ref) &&
               pNode->getChild(1)->getNodeType() == SQL_NODE_EQUAL))
            return sal_False;

        OTableFieldDesc aDragLeft, aDragRight;
        if (!FillDragInfo(pNode->getChild(0),aDragLeft) ||
            !FillDragInfo(pNode->getChild(2),aDragRight))
            return sal_False;

        OQueryTableConnection* pConn = static_cast<OQueryTableConnection*>( m_pTableView->GetTabConn(static_cast<OQueryTableWindow*>(aDragLeft.GetTabWindow()),static_cast<OQueryTableWindow*>(aDragRight.GetTabWindow())));
        if(!pConn)
        {
            OQueryTableConnectionData aInfoData;
            aInfoData.InitFromDrag(aDragLeft, aDragRight);
            aInfoData.SetJoinType(_eJoinType);

            OQueryTableConnection aInfo(static_cast<OQueryTableView*>(m_pTableView), &aInfoData);
            // da ein OQueryTableConnection-Objekt nie den Besitz der uebergebenen Daten uebernimmt, sondern sich nur den Zeiger merkt,
            // ist dieser Zeiger auf eine lokale Variable hier unkritisch, denn aInfoData und aInfo haben die selbe Lebensdauer
            static_cast<OQueryTableView*>(m_pTableView)->NotifyTabConnection( aInfo );
        }
        else
        {
            ::rtl::OUString aSourceFieldName(aDragLeft.GetField());
            ::rtl::OUString aDestFieldName(aDragRight.GetField());
            // the connection could point on the other side
            if(pConn->GetSourceWin() == aDragRight.GetTabWindow())
            {
                ::rtl::OUString aTmp(aSourceFieldName);
                aSourceFieldName = aDestFieldName;
                aDestFieldName = aTmp;
            }
            pConn->GetData()->AppendConnLine( aSourceFieldName,aDestFieldName);
            pConn->UpdateLineList();
            // Modified-Flag
            //  SetModified();
            // und neu zeichnen
            pConn->RecalcLines();
                // fuer das unten folgende Invalidate muss ich dieser neuen Connection erst mal die Moeglichkeit geben,
                // ihr BoundingRect zu ermitteln
            pConn->Invalidate();
        }
        return sal_True;
    }
    return sal_False;
}
// -----------------------------------------------------------------------------
void OQueryDesignView::setSlotEnabled(sal_Int32 _nSlotId,sal_Bool _bEnable)
{
    sal_uInt16 nRow;
    switch (_nSlotId)
    {
        case ID_BROWSER_QUERY_VIEW_FUNCTIONS:
            nRow = BROW_FUNCTION_ROW;
            break;
        case ID_BROWSER_QUERY_VIEW_TABLES:
            nRow = BROW_TABLE_ROW;
            break;
        case ID_BROWSER_QUERY_VIEW_ALIASES:
            nRow = BROW_COLUMNALIAS_ROW;
            break;
    }
    m_pSelectionBox->SetRowVisible(nRow,_bEnable);
    m_pSelectionBox->Invalidate();
}
// -----------------------------------------------------------------------------
sal_Bool OQueryDesignView::isSlotEnabled(sal_Int32 _nSlotId)
{
    sal_uInt16 nRow;
    switch (_nSlotId)
    {
        case ID_BROWSER_QUERY_VIEW_FUNCTIONS:
            nRow = BROW_FUNCTION_ROW;
            break;
        case ID_BROWSER_QUERY_VIEW_TABLES:
            nRow = BROW_TABLE_ROW;
            break;
        case ID_BROWSER_QUERY_VIEW_ALIASES:
            nRow = BROW_COLUMNALIAS_ROW;
            break;
    }
    return m_pSelectionBox->IsRowVisible(nRow);
}
// -----------------------------------------------------------------------------
void OQueryDesignView::InitFromParseNode()
{
    m_pSelectionBox->ClearAll();
    m_pSelectionBox->Fill();

    ::connectivity::OSQLParseTreeIterator& aIterator = static_cast<OQueryController*>(getController())->getParseIterator();
    const ::connectivity::OSQLParseNode* pParseTree = aIterator.getParseTree();
    const ::connectivity::OSQLParseNode* pTableRefCommaList = 0;

    if (pParseTree)
    {
        if (!static_cast<OQueryController*>(getController())->isEsacpeProcessing())
        {
            WarningBox( this, ModuleRes(WARN_QRY_NATIVE) ).Execute();
        }
        else if (SQL_ISRULE(pParseTree,select_statement))
        {

            ::connectivity::OSQLParseNode* pTree = pParseTree->getChild(1);

            const OSQLTables& aMap = aIterator.getTables();
            ::comphelper::UStringMixEqual aKeyComp(static_cast< ::comphelper::UStringMixLess*>(&aMap.key_comp())->isCaseSensitive());

            Reference< XDatabaseMetaData >  xMetaData = static_cast<OQueryController*>(getController())->getConnection()->getMetaData();

            sal_Int32 nMax = xMetaData->getMaxTablesInSelect();
            if(!nMax || nMax >= (sal_Int32)aMap.size()) // Anzahl der Tabellen im Select-Statement "uberpr"ufen
            {
                ::rtl::OUString aComposedName;
                ::rtl::OUString aQualifierName;
                ::rtl::OUString sAlias;
                OSQLTables::const_iterator aIter = aMap.begin();
                for(;aIter != aMap.end();++aIter)
                {
                    OSQLTable xTable = aIter->second;
                    ::dbaui::composeTableName(xMetaData,Reference<XPropertySet>(xTable,UNO_QUERY),aComposedName,sal_False);

                    OQueryTableWindow* pExistentWin = static_cast<OQueryTableView*>(m_pTableView)->FindTable(aIter->first);
                    if (!pExistentWin)
                    {
                        m_pTableView->AddTabWin(aComposedName, aIter->first,sal_True);
                    }
                    else
                    {
                        // es existiert schon ein Fenster mit dem selben Alias ...
                        ::rtl::OUString aFullWinName();

                        if (!aKeyComp(pExistentWin->GetData()->GetComposedName(),aComposedName))
                            // ... aber anderem Tabellennamen -> neues Fenster
                            m_pTableView->AddTabWin(aComposedName, aIter->first);
                    }
                }

                // now delete the data for which we haven't any tablewindow
                OJoinTableView::OTableWindowMap* pTableMap = m_pTableView->GetTabWinMap();
                ::std::vector< OTableWindowData*>::iterator aDataIter = static_cast<OQueryController*>(getController())->getTableWindowData()->begin();
                for(;aDataIter != static_cast<OQueryController*>(getController())->getTableWindowData()->end();)
                {
                    OQueryTableWindowData* pData = static_cast<OQueryTableWindowData*>(*aDataIter);
                    if(pTableMap->find(pData->GetAliasName()) == pTableMap->end())
                    {
                        delete *aDataIter;
                        aDataIter = static_cast<OQueryController*>(getController())->getTableWindowData()->erase(aDataIter);
                    }
                    else
                        ++aDataIter;
                }

                FillOuterJoins(pParseTree->getChild(3)->getChild(0)->getChild(1));

                // check if we have a distinct statement
                if(SQL_ISTOKEN(pParseTree->getChild(1),DISTINCT))
                {
                    static_cast<OQueryController*>(getController())->setDistinct(sal_True);
                    static_cast<OQueryController*>(getController())->InvalidateFeature(ID_BROWSER_QUERY_DISTINCT_VALUES);
                }
                if (!InstallFields(pParseTree, m_pTableView->GetTabWinMap()))
                {
                    // GetSelectionCriteria mu"s vor GetHavingCriteria aufgerufen werden
                    int nLevel=0;

                    GetSelectionCriteria(pParseTree,nLevel,sal_True);
                    GetGroupCriteria(pParseTree);
                    GetHavingCriteria(pParseTree,nLevel);
                    GetOrderCriteria(pParseTree);
                }
            }
            else
            {
                ErrorBox aBox(this, ModuleRes( ERR_QRY_TOO_MANY_TABLES));
                aBox.Execute();
            }
        }
        else
        {
            ErrorBox aBox(this, ModuleRes( ERR_QRY_NOSELECT));
            aBox.Execute();
        }
    }

    // Durch das Neuerzeugen wurden wieder Undo-Actions in den Manager gestellt
    static_cast<OQueryController*>(getController())->getUndoMgr()->Clear();
}
// -----------------------------------------------------------------------------
int OQueryDesignView::InstallFields(const ::connectivity::OSQLParseNode* pNode, OJoinTableView::OTableWindowMap* pTabList )
{
    if( pNode==0 || !SQL_ISRULE(pNode,select_statement))
    {
        ErrorBox    aBox( this, ModuleRes( ERR_QRY_NOSELECT ) );
        aBox.Execute();
        return 1;
    }

    ::connectivity::OSQLParseNode* pParseTree = pNode->getChild(2);
    sal_Bool bFirstField = sal_True;    // bei der Initialisierung muß auf alle Faelle das erste Feld neu aktiviert werden

    if(pParseTree->isRule() && SQL_ISPUNCTUATION(pParseTree->getChild(0),"*"))
    {
        // SELECT * ...

        OTableFieldDesc aInfo;
        OJoinTableView::OTableWindowMap::iterator aIter = pTabList->begin();
        for(;aIter != pTabList->end();++aIter)
        {
            OQueryTableWindow* pTabWin = static_cast<OQueryTableWindow*>(aIter->second);

            if (pTabWin->ExistsField( ::rtl::OUString::createFromAscii("*"), aInfo ))
            {
                if(!InsertField(aInfo, sal_True, bFirstField))
                    return 1;
                bFirstField = sal_False;
            }
        }

        // Einfach alle Columns der Datei direkt uebernehmen:
    }
    else if (SQL_ISRULE(pParseTree,scalar_exp_commalist) )
    {
        // SELECT column, ...

        ::rtl::OUString aColumnName,aTableRange;
        for (sal_uInt32 i = 0; i < pParseTree->count(); i++)
        {
            ::connectivity::OSQLParseNode * pColumnRef = pParseTree->getChild(i);

            if (SQL_ISRULE(pColumnRef,select_sublist))
            {
                OTableFieldDesc aInfo;
                OJoinTableView::OTableWindowMap::iterator aIter = pTabList->begin();
                for(;aIter != pTabList->end();++aIter)
                {
                    OQueryTableWindow* pTabWin = static_cast<OQueryTableWindow*>(aIter->second);

                    if (pTabWin->ExistsField( ::rtl::OUString::createFromAscii("*"), aInfo ))
                    {
                        if(!InsertField(aInfo, sal_True, bFirstField))
                            return 1;

                        bFirstField = sal_False;
                    }
                }
            }
            else if (SQL_ISRULE(pColumnRef,derived_column))
            {
                ::rtl::OUString aColumnAlias(static_cast<OQueryController*>(getController())->getParseIterator().getColumnAlias(pColumnRef)); // kann leer sein
                pColumnRef = pColumnRef->getChild(0);
                if (SQL_ISRULE(pColumnRef,column_ref))
                {
                    OTableFieldDesc aInfo;
                    switch(InsertColumnRef(pColumnRef,aColumnName,aColumnAlias,aTableRange,aInfo,pTabList))
                    {
                        case 5:
                            ErrorBox( this, ModuleRes( ERR_QRY_AMB_FIELD ) ).Execute();
                            break;
                        default:
                            if(!InsertField(aInfo, sal_True, bFirstField))
                            return 1;
                        bFirstField = sal_False;
                    }
                }
                else if(SQL_ISRULE(pColumnRef,general_set_fct)  || SQL_ISRULE(pColumnRef,set_fct_spec)  ||
                        SQL_ISRULE(pColumnRef,position_exp)     || SQL_ISRULE(pColumnRef,extract_exp)   ||
                        SQL_ISRULE(pColumnRef,length_exp)       || SQL_ISRULE(pColumnRef,char_value_fct))
                {
                    OTableFieldDesc aInfo;
                    ::rtl::OUString aColumns;

                    pColumnRef->parseNodeToStr( aColumns,
                                                static_cast<OQueryController*>(getController())->getConnection()->getMetaData(),
                                                &static_cast<OQueryController*>(getController())->getParser()->getContext(),
                                                sal_True,
                                                sal_False);

                    ::connectivity::OSQLParseNode * pParamRef = pColumnRef->getChild(pColumnRef->count()-2);
                    if (SQL_ISRULE(pColumnRef,general_set_fct)
                        && SQL_ISRULE(pParamRef = pColumnRef->getChild(pColumnRef->count()-2),column_ref))
                    {
                        // Parameter auf Columnref pr"ufen
                        switch(InsertColumnRef(pParamRef,aColumnName,aColumnAlias,aTableRange,aInfo,pTabList))
                        {
                            case 5:
                                ErrorBox( this, ModuleRes( ERR_QRY_AMB_FIELD ) ).Execute();
                                break;
                        }
                    }
                    else
                    {
                        if(pParamRef && pParamRef->getTokenValue().toChar() == '*')
                        {
                            OJoinTableView::OTableWindowMap::iterator aIter = pTabList->begin();
                            for(;aIter != pTabList->end();++aIter)
                            {
                                OQueryTableWindow* pTabWin = static_cast<OQueryTableWindow*>(aIter->second);
                                if (pTabWin->ExistsField( ::rtl::OUString::createFromAscii("*"), aInfo ))
                                    break;
                            }
                        }
                        else
                        {
                            aInfo.SetDataType(DataType::DOUBLE);
                            aInfo.SetFieldType(TAB_NORMAL_FIELD);
                            aInfo.SetTabWindow(NULL);
                            aInfo.SetField(aColumns);
                            aInfo.SetFieldAlias(aColumnAlias);
                        }
                    }

                    if(SQL_ISRULE(pColumnRef,general_set_fct))
                    {
                        aInfo.SetFunctionType(FKT_AGGREGATE);
                        String aCol(aColumns);
                        aInfo.SetFunction(aCol.GetToken(0,'(').EraseTrailingChars(' '));
                    }
                    else
                        aInfo.SetFunctionType(FKT_OTHER);

                    if(!InsertField(aInfo, sal_True, bFirstField))
                        return 1;
                    bFirstField = sal_False;
                }
                else //if(SQL_ISRULE(pColumnRef,num_value_exp)  || SQL_ISRULE(pColumnRef,term))
                {
                    ::rtl::OUString aColumns;
                    pColumnRef->parseNodeToStr( aColumns,
                                                static_cast<OQueryController*>(getController())->getConnection()->getMetaData(),
                                                &static_cast<OQueryController*>(getController())->getParser()->getContext(),
                                                sal_True,sal_False);

                    OTableFieldDesc aInfo;
                    aInfo.SetDataType(DataType::DOUBLE);
                    aInfo.SetFieldType(TAB_NORMAL_FIELD);
                    aInfo.SetTabWindow(NULL);
                    aInfo.SetField(aColumns);
                    aInfo.SetFieldAlias(aColumnAlias);
                    aInfo.SetFunctionType(FKT_OTHER);

                    if(!InsertField(aInfo, sal_True, bFirstField))
                        return 1;
                    bFirstField = sal_False;
                }
            }
        }
    }
    else
    {
        ErrorBox( this, ModuleRes( ERR_QRY_SYNTAX ) ).Execute();
        return 4;
    }

    return 0;
}
//------------------------------------------------------------------------------
void OQueryDesignView::GetOrderCriteria(const ::connectivity::OSQLParseNode* pParseRoot )
{
    if (!pParseRoot->getChild(3)->getChild(4)->isLeaf())
    {
        ::connectivity::OSQLParseNode* pNode = pParseRoot->getChild(3)->getChild(4)->getChild(2);
        ::connectivity::OSQLParseNode* pParamRef = NULL;
        ::rtl::OUString aField, aAlias;
        sal_uInt16 nPos = 0;

        EOrderDir eOrderDir;
        OTableFieldDesc     aDragLeft;
        for( sal_uInt32 i=0 ; i<pNode->count() ; i++ )
        {
            eOrderDir = ORDER_ASC;
            ::connectivity::OSQLParseNode*  pChild = pNode->getChild( i );

            if (SQL_ISTOKEN( pChild->getChild(1), DESC ) )
                eOrderDir = ORDER_DESC;

            if(SQL_ISRULE(pChild->getChild(0),column_ref))
            {
                if(FillDragInfo(pChild->getChild(0),aDragLeft))
                    m_pSelectionBox->AddOrder( aDragLeft, eOrderDir, nPos);
                else // it could be a alias name for a field
                {
                    ::rtl::OUString aTableRange,aColumnName;
                    ::connectivity::OSQLParseTreeIterator& rParseIter = static_cast<OQueryController*>(getController())->getParseIterator();
                    rParseIter.getColumnRange( pChild->getChild(0), aColumnName, aTableRange );

                    ::std::vector<OTableFieldDesc*>* pList = static_cast<OQueryController*>(getController())->getTableFieldDesc();
                    ::std::vector<OTableFieldDesc*>::iterator aIter = pList->begin();
                    for(;aIter != pList->end();++aIter)
                    {
                        OTableFieldDesc* pEntry = *aIter;
                        if(pEntry && pEntry->GetFieldAlias() == aColumnName.getStr())
                            pEntry->SetOrderDir( eOrderDir );
                    }
                }
            }
            else if(SQL_ISRULE(pChild->getChild(0),general_set_fct) &&
                    SQL_ISRULE(pParamRef = pChild->getChild(0)->getChild(pChild->getChild(0)->count()-2),column_ref) &&
                    FillDragInfo(pParamRef,aDragLeft))
                m_pSelectionBox->AddOrder( aDragLeft, eOrderDir, nPos);
        }
    }
}
//------------------------------------------------------------------------------
void OQueryDesignView::GetHavingCriteria(const ::connectivity::OSQLParseNode* pSelectRoot, int &rLevel )
{
    if (!pSelectRoot->getChild(3)->getChild(3)->isLeaf())
        GetORCriteria(pSelectRoot->getChild(3)->getChild(3)->getChild(1),rLevel, sal_True);
}
//------------------------------------------------------------------------------
void OQueryDesignView::GetGroupCriteria(const ::connectivity::OSQLParseNode* pSelectRoot )
{
    if (!pSelectRoot->getChild(3)->getChild(2)->isLeaf())
    {
        ::connectivity::OSQLParseNode* pGroupBy = pSelectRoot->getChild(3)->getChild(2)->getChild(2);
        OTableFieldDesc aDragInfo;
        for( sal_uInt32 i=0 ; i < pGroupBy->count() ; i++ )
        {
            ::connectivity::OSQLParseNode* pColumnRef = pGroupBy->getChild( i );
            if(SQL_ISRULE(pColumnRef,column_ref))
            {
                FillDragInfo(pColumnRef,aDragInfo);
                aDragInfo.SetGroupBy(sal_True);
                m_pSelectionBox->AddGroupBy(aDragInfo);
            }
        }
    }
}
//------------------------------------------------------------------------------
void OQueryDesignView::FillOuterJoins(const ::connectivity::OSQLParseNode* pTableRefList)
{
    sal_uInt32 ncount = pTableRefList->count();

    if (ncount == 0)
    {
        ErrorBox( this, ModuleRes( ERR_QRY_ILLEGAL_JOIN ) ).Execute();
        return;
    }
    else
    {
        for (sal_uInt32 i=0; i < ncount; i++)
        {
            const ::connectivity::OSQLParseNode* pParseNode = pTableRefList->getChild(i);
            if (SQL_ISRULE(pParseNode , qualified_join) ||
                SQL_ISRULE(pParseNode , joined_table))
            {
                if (!InsertJoin(pParseNode))
                {
                    ErrorBox( this, ModuleRes( ERR_QRY_ILLEGAL_JOIN ) ).Execute();
                    return;
                }
            }
            else if(pParseNode->count() == 4 && SQL_ISPUNCTUATION(pParseNode->getChild(0),"{") && SQL_ISRULE(pParseNode,table_ref))
            {
                if (!InsertJoin(pParseNode->getChild(2)))
                {
                    ErrorBox( this, ModuleRes( ERR_QRY_ILLEGAL_JOIN ) ).Execute();
                    return;
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
int OQueryDesignView::InsertColumnRef(const ::connectivity::OSQLParseNode * pColumnRef,::rtl::OUString& aColumnName,const ::rtl::OUString& aColumnAlias,
                                    ::rtl::OUString& aTableRange,OTableFieldDesc& aInfo, OJoinTableView::OTableWindowMap* pTabList)
{

    // Tabellennamen zusammen setzen
    ::connectivity::OSQLParseTreeIterator& rParseIter = static_cast<OQueryController*>(getController())->getParseIterator();
    rParseIter.getColumnRange( pColumnRef, aColumnName, aTableRange );

    DBG_ASSERT(aColumnName.getLength(),"Columnname darf nicht leer sein");
    if (!aTableRange.getLength())
    {
        // SELECT column, ...
        sal_Bool bFound(sal_False);
        OJoinTableView::OTableWindowMap::iterator aIter = pTabList->begin();
        for(;aIter != pTabList->end();++aIter)
        {
            OQueryTableWindow* pTabWin = static_cast<OQueryTableWindow*>(aIter->second);
            if (pTabWin->ExistsField( aColumnName, aInfo ) )
            {
                if(aColumnName.toChar() != '*')
                    aInfo.SetFieldAlias(aColumnAlias);
                bFound = sal_True;
                break;
            }
        }
        if (!bFound)
        {
            aInfo.SetTable(::rtl::OUString());
            aInfo.SetAlias(::rtl::OUString());
            aInfo.SetField(aColumnName);
            aInfo.SetFieldAlias(aColumnAlias);  // nyi : hier ein fortlaufendes Expr_1, Expr_2 ...
            aInfo.SetFunctionType(FKT_OTHER);
        }
    }
    else
    {
        // SELECT range.column, ...
        OQueryTableWindow* pTabWin = static_cast<OQueryTableView*>(m_pTableView)->FindTable(aTableRange);

        if (pTabWin && pTabWin->ExistsField(aColumnName, aInfo))
        {
            if(aColumnName.toChar() != '*')
                aInfo.SetFieldAlias(aColumnAlias);
        }
        else
        {
            aInfo.SetTable(::rtl::OUString());
            aInfo.SetAlias(::rtl::OUString());
            aInfo.SetField(aColumnName);
            aInfo.SetFieldAlias(aColumnAlias);  // nyi : hier ein fortlaufendes Expr_1, Expr_2 ...
            aInfo.SetFunctionType(FKT_OTHER);
        }
    }
    return 0;
}
//-----------------------------------------------------------------------------
sal_Bool OQueryDesignView::InsertJoin(const ::connectivity::OSQLParseNode *pNode)
{
    DBG_ASSERT(SQL_ISRULE(pNode, qualified_join) || SQL_ISRULE(pNode, joined_table),
        "OQueryDesignView::InsertJoin: Fehler im Parse Tree");

    if (SQL_ISRULE(pNode,joined_table))
        return InsertJoin(pNode->getChild(1));

    if (SQL_ISRULE(pNode->getChild(0),qualified_join))
    {
        if (!InsertJoin(pNode->getChild(0)))
            return sal_False;
    }
    else if (SQL_ISRULE(pNode->getChild(0), joined_table))
    {
        if (!InsertJoin(pNode->getChild(0)->getChild(1)))
            return sal_False;
    }
    else if (!(SQL_ISRULE(pNode->getChild(0), table_ref) && (
             SQL_ISRULE(pNode->getChild(0)->getChild(0), catalog_name)   ||
             SQL_ISRULE(pNode->getChild(0)->getChild(0), schema_name)    ||
             SQL_ISRULE(pNode->getChild(0)->getChild(0), table_name))))
        return sal_False;

    // geschachtelter join?
    if (SQL_ISRULE(pNode->getChild(3),qualified_join))
    {
        if (!InsertJoin(pNode->getChild(3)))
            return sal_False;
    }
    else if (SQL_ISRULE(pNode->getChild(3), joined_table))
    {
        if (!InsertJoin(pNode->getChild(3)->getChild(1)))
            return sal_False;
    }
    // sonst sollte es eine Tabelle sein
    else if (!(SQL_ISRULE(pNode->getChild(3), table_ref) && (
             SQL_ISRULE(pNode->getChild(3)->getChild(0), catalog_name)   ||
             SQL_ISRULE(pNode->getChild(3)->getChild(0), schema_name)    ||
             SQL_ISRULE(pNode->getChild(3)->getChild(0), table_name))))
        return sal_False;

    // named column join wird später vieleicht noch implementiert
    // SQL_ISRULE(pNode->getChild(4),named_columns_join)
    if (SQL_ISRULE(pNode->getChild(4),join_condition))
    {
        EJoinType eJoinType;
        ::connectivity::OSQLParseNode* pJoinType = pNode->getChild(1); // join_type
        if (SQL_ISRULE(pJoinType,join_type) && SQL_ISTOKEN(pJoinType->getChild(0),INNER))
        {
            eJoinType = INNER_JOIN;
        }
        else
        {
            if (SQL_ISRULE(pJoinType,join_type))       // eine Ebene tiefer
                pJoinType = pJoinType->getChild(0);

            if (SQL_ISTOKEN(pJoinType->getChild(0),LEFT))
                eJoinType = LEFT_JOIN;
            else if(SQL_ISTOKEN(pJoinType->getChild(0),RIGHT))
                eJoinType = RIGHT_JOIN;
            else
                eJoinType = FULL_JOIN;
        }
        if(!InsertJoinConnection(pNode->getChild(4)->getChild(1), eJoinType))
            return sal_False;
    }
    else
        return sal_False;

    return sal_True;
}
// -----------------------------------------------------------------------------
void OQueryDesignView::zoomTableView(const Fraction& _rFraction)
{
    m_pTableView->SetZoom(_rFraction);
}
// -----------------------------------------------------------------------------
void OQueryDesignView::SaveUIConfig()
{
    OQueryController* pCtrl = static_cast<OQueryController*>(getController());
    if (pCtrl)
    {
        pCtrl->SaveTabWinsPosSize( m_pTableView->GetTabWinMap(), m_pScrollWindow->GetHScrollBar()->GetThumbPos(), m_pScrollWindow->GetVScrollBar()->GetThumbPos() );
        //  pCtrl->SaveTabFieldsWidth( m_pSelectionBox );
        pCtrl->setVisibleRows( m_pSelectionBox->GetNoneVisibleRows() );
        pCtrl->setSplitPos( m_aSplitter.GetSplitPosPixel() );
    }
}
// -----------------------------------------------------------------------------






