/*************************************************************************
 *
 *  $RCSfile: QueryTableView.cxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:13:14 $
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

#ifndef DBAUI_QUERYTABLEVIEW_HXX
#include "QueryTableView.hxx"
#endif
#ifndef DBAUI_TABLEFIELDINFO_HXX
#include "TableFieldInfo.hxx"
#endif
#ifndef DBAUI_TABLEFIELDDESC_HXX
#include "TableFieldDescription.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef DBAUI_QUERY_TABLEWINDOW_HXX
#include "QTableWindow.hxx"
#endif
#ifndef DBAUI_QUERYTABLECONNECTION_HXX
#include "QTableConnection.hxx"
#endif
#ifndef DBAUI_QTABLECONNECTIONDATA_HXX
#include "QTableConnectionData.hxx"
#endif
#ifndef DBAUI_QUERYDESIGNVIEW_HXX
#include "QueryDesignView.hxx"
#endif
#ifndef DBAUI_QUERYCONTROLLER_HXX
#include "querycontroller.hxx"
#endif
#ifndef DBAUI_QUERYADDTABCONNUNDOACTION_HXX
#include "QueryAddTabConnUndoAction.hxx"
#endif
#ifndef DBAUI_QUERYTABWINSHOWUNDOACT_HXX
#include "QueryTabWinShowUndoAct.hxx"
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XKEYSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#ifndef DBACCESS_JACCESS_HXX
#include "JAccess.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef DBAUI_QUERYDLG_HXX
#include "querydlg.hxx"
#endif
#ifndef DBAUI_JOINEXCHANGE_HXX
#include "JoinExchange.hxx"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef DBAUI_QUERYDESIGNVIEW_HXX
#include "QueryDesignView.hxx"
#endif
#ifndef _DBU_QRY_HRC_
#include "dbu_qry.hrc"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::accessibility;

TYPEINIT1(OQueryTableView, OJoinTableView);

//------------------------------------------------------------------------------
namespace
{
    // -----------------------------------------------------------------------------
    sal_Bool isColumnInKeyType(const Reference<XKeysSupplier>& _rxKeys,const ::rtl::OUString& _rColumnName,sal_Int32 _nKeyType)
    {
        sal_Bool bReturn = sal_False;
        if(_rxKeys.is())
        {
            Reference< XIndexAccess> xKeyIndex = _rxKeys->getKeys();
            if ( !xKeyIndex.is() )
                return sal_False;
            Reference<XColumnsSupplier> xColumnsSupplier;
            // search the one and only primary key
            for(sal_Int32 i=0;i< xKeyIndex->getCount();++i)
            {
                Reference<XPropertySet> xProp;
                ::cppu::extractInterface(xProp,xKeyIndex->getByIndex(i));
                if(xProp.is())
                {
                    sal_Int32 nKeyType = 0;
                    xProp->getPropertyValue(PROPERTY_TYPE) >>= nKeyType;
                    if(_nKeyType == nKeyType)
                    {
                        xColumnsSupplier.set(xProp,UNO_QUERY);
                        if(xColumnsSupplier.is())
                        {
                            Reference<XNameAccess> xColumns = xColumnsSupplier->getColumns();
                            if(xColumns.is() && xColumns->hasByName(_rColumnName))
                            {
                                bReturn = sal_True;
                                break;
                            }
                        }
                    }
                }
            }
        }
        return bReturn;
    }
    // -----------------------------------------------------------------------------
    /** appends a new TabAdd Undo action at controller
        @param  _pView          the view which we use
        @param  _pUndoAction    the undo action which should be added
        @param  _pConnection    the connection for which the undo action should be appended
        @param  _bOwner         is the undo action the owner
    */
    // -----------------------------------------------------------------------------
    void addUndoAction( OQueryTableView* _pView,
                        OQueryTabConnUndoAction* _pUndoAction,
                        OQueryTableConnection* _pConnection,
                        sal_Bool _bOwner = sal_False)
    {
        _pUndoAction->SetOwnership(_bOwner);
        _pUndoAction->SetConnection(_pConnection);
        _pView->getDesignView()->getController()->addUndoActionAndInvalidate(_pUndoAction);
    }
    // -----------------------------------------------------------------------------
    /** openJoinDialog opens the join dialog with this connection data
        @param  _pView              the view which we use
        @param  _pConnectionData    the connection data

        @return true when OK was pressed otherwise false
    */
    sal_Bool openJoinDialog(OQueryTableView* _pView,OTableConnectionData* _pConnectionData,BOOL _bSelectableTables)
    {
        OQueryTableConnectionData* pData = static_cast< OQueryTableConnectionData*>(_pConnectionData);

        DlgQryJoin aDlg(_pView,pData,_pView->GetTabWinMap(),_pView->getDesignView()->getController()->getConnection(),_bSelectableTables);
        sal_Bool bOk = aDlg.Execute() == RET_OK;
        if( bOk )
        {
            pData->SetJoinType(aDlg.GetJoinType());
            _pView->getDesignView()->getController()->setModified(sal_True);
        }

        return bOk;
    }
    // -----------------------------------------------------------------------------
    /** connectionModified adds an undo action for the modified connection and forces an redraw
        @param  _pView              the view which we use
        @param  _pConnection    the connection which was modified
        @param  _bAddUndo       true when an undo action should be appended
    */
    void connectionModified(OQueryTableView* _pView,
                            OTableConnection* _pConnection,
                            sal_Bool _bAddUndo)
    {
        OSL_ENSURE(_pConnection,"Invalid connection!");
        _pConnection->UpdateLineList();

        // add an undo action
        if ( _bAddUndo )
            addUndoAction(  _pView,
                            new OQueryAddTabConnUndoAction(_pView),
                            static_cast< OQueryTableConnection*>(_pConnection));
        // redraw
        _pConnection->RecalcLines();
        // force an invalidation of the bounding rectangle
        _pConnection->Invalidate();

        _pView->Invalidate(INVALIDATE_NOCHILDREN);
    }
    // -----------------------------------------------------------------------------
    void addConnections(OQueryTableView* _pView,
                        const OQueryTableWindow* _pSource,
                        const OQueryTableWindow* _pDest,
                        const Reference<XNameAccess>& _rxSourceForeignKeyColumns)
    {
        // we found a table in our view where we can insert some connections
        // the key columns have a property called RelatedColumn
        // OQueryTableConnectionData aufbauen
        OQueryTableConnectionData aNewConnData( _pSource->GetTableName(), _pDest->GetTableName(),
                                                _pSource->GetAliasName(), _pDest->GetAliasName());

        Reference<XKeysSupplier> xReferencedKeys(_pDest->GetTable(),UNO_QUERY);
        ::rtl::OUString sRelatedColumn;

        // iterate through all foreignkey columns to create the connections
        Sequence< ::rtl::OUString> aElements(_rxSourceForeignKeyColumns->getElementNames());
        const ::rtl::OUString* pBegin = aElements.getConstArray();
        const ::rtl::OUString* pEnd   = pBegin + aElements.getLength();
        for(sal_Int32 i=0;pBegin != pEnd;++pBegin,++i)
        {
            Reference<XPropertySet> xColumn;
            ::cppu::extractInterface(xColumn,_rxSourceForeignKeyColumns->getByName(*pBegin));

            aNewConnData.SetFieldType(JTCS_FROM,TAB_NORMAL_FIELD);

            xColumn->getPropertyValue(PROPERTY_RELATEDCOLUMN) >>= sRelatedColumn;
            aNewConnData.SetFieldType(JTCS_TO,isColumnInKeyType(xReferencedKeys,sRelatedColumn,KeyType::PRIMARY) ? TAB_PRIMARY_FIELD : TAB_NORMAL_FIELD);

            {
                Sequence< sal_Int16> aFind(::comphelper::findValue(_pSource->GetOriginalColumns()->getElementNames(),*pBegin,sal_True));
                if(aFind.getLength())
                    aNewConnData.SetFieldIndex(JTCS_FROM,aFind[0]+1);
                else
                    OSL_ENSURE(0,"Column not found!");
            }
            // get the position inside the tabe
            Reference<XNameAccess> xRefColumns = _pDest->GetOriginalColumns();
            if(xRefColumns.is())
            {
                Sequence< sal_Int16> aFind(::comphelper::findValue(xRefColumns->getElementNames(),sRelatedColumn,sal_True));
                if(aFind.getLength())
                    aNewConnData.SetFieldIndex(JTCS_TO,aFind[0]+1);
                else
                    OSL_ENSURE(0,"Column not found!");
            }
            aNewConnData.AppendConnLine(*pBegin,sRelatedColumn);

            // dann die Conn selber dazu
            OQueryTableConnection aNewConn(_pView, &aNewConnData);
                // der Verweis auf die lokale Variable ist unkritisch, da NotifyQueryTabConn eine neue Kopie anlegt
            // und mir hinzufuegen (wenn nicht schon existent)
            _pView->NotifyTabConnection(aNewConn, sal_False);
                // don't create an Undo-Action for the new connection : the connection is
                // covered by the Undo-Action for the tabwin, as the "Undo the insert" will
                // automatically remove all connections adjacent to the win.
                // (Because of this automatism we would have an ownerhsip ambiguity for
                // the connection data if we would insert the conn-Undo-Action)
                // FS - 21.10.99 - 69183
        }
    }
}
//==================================================================
// class OQueryTableView
//==================================================================
DBG_NAME(OQueryTableView)
//------------------------------------------------------------------------
OQueryTableView::OQueryTableView( Window* pParent,OQueryDesignView* pView)
    : OJoinTableView( pParent,pView)
{
    DBG_CTOR(OQueryTableView,NULL);
    SetHelpId(HID_CTL_QRYDGNTAB);
}

//------------------------------------------------------------------------
OQueryTableView::~OQueryTableView()
{
    DBG_DTOR(OQueryTableView,NULL);
}

//------------------------------------------------------------------------
sal_Int32 OQueryTableView::CountTableAlias(const String& rName, sal_Int32& rMax)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    sal_Int32 nRet = 0;

    OTableWindowMapIterator aIter = GetTabWinMap()->find(rName);
    while(aIter != GetTabWinMap()->end())
    {
        OTableWindow* pWin = aIter->second;

        String aNewName;
        aNewName = rName;
        aNewName += '_';
        aNewName += String::CreateFromInt32(++nRet);

        aIter = GetTabWinMap()->find(aNewName);
    }

    rMax = nRet;

    return nRet;
}
//------------------------------------------------------------------------
void OQueryTableView::ReSync()
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    ::std::vector< OTableWindowData*>* pTabWinDataList = m_pView->getController()->getTableWindowData();
    DBG_ASSERT((getTableConnections()->size()==0) && (GetTabWinMap()->size()==0),
        "vor OQueryTableView::ReSync() bitte ClearAll aufrufen !");

    // ich brauche eine Sammlung aller Fensternamen, deren Anlegen schief geht, damit ich die entsprechenden Connections
    // gar nicht erst anlege
    ::std::vector<String> arrInvalidTables;

    ::std::vector< OTableWindowData*>::reverse_iterator aIter = pTabWinDataList->rbegin();
    // Fenster kreieren und einfuegen

    for(;aIter != pTabWinDataList->rend();++aIter)
    {
        OQueryTableWindowData* pData = static_cast<OQueryTableWindowData*>(*aIter);
        OTableWindow* pTabWin = createWindow(pData);

        // ich gehe jetzt NICHT ueber ShowTabWin, da dieses die Daten des Fensters in die Liste des Docs einfuegt, was
        // schlecht waere, denn genau von dort hole ich sie ja gerade
        // also Schritt fuer Schritt
        if (!pTabWin->Init())
        {
            // das Initialisieren ging schief, dass heisst, dieses TabWin steht nicht zur Verfuegung, also muss ich es inklusive
            // seiner Daten am Dokument aufraeumen
            pTabWin->clearListBox();
            delete pTabWin;
            arrInvalidTables.push_back(pData->GetAliasName());

            pTabWinDataList->erase( ::std::find(pTabWinDataList->begin(),pTabWinDataList->end(),*aIter) );
            delete pData;
            continue;
        }

        (*GetTabWinMap())[pData->GetAliasName()] = pTabWin; // am Anfang einfuegen, da ich die DataList ja rueckwaerts durchlaufe
        // wenn in den Daten keine Position oder Groesse steht -> Default
        if (!pData->HasPosition() && !pData->HasSize())
            SetDefaultTabWinPosSize(pTabWin);

        pTabWin->Show();
    }

    // Verbindungen einfuegen
    ::std::vector< OTableConnectionData*>* pTabConnDataList = m_pView->getController()->getTableConnectionData();
    ::std::vector< OTableConnectionData*>::reverse_iterator aConIter = pTabConnDataList->rbegin();

    for(;aConIter != pTabConnDataList->rend();++aConIter)
    {
        OQueryTableConnectionData* pTabConnData =  static_cast<OQueryTableConnectionData*>(*aConIter);

        // gibt es die beiden Tabellen zur Connection ?
        String strTabExistenceTest = pTabConnData->GetSourceWinName();
        sal_Bool bInvalid = ::std::find(arrInvalidTables.begin(),arrInvalidTables.end(),strTabExistenceTest) != arrInvalidTables.end();
        strTabExistenceTest = pTabConnData->GetDestWinName();
        bInvalid |= ::std::find(arrInvalidTables.begin(),arrInvalidTables.end(),strTabExistenceTest) != arrInvalidTables.end();

        if (bInvalid)
        {   // nein -> Pech gehabt, die Connection faellt weg
            pTabConnDataList->erase( ::std::find(pTabConnDataList->begin(),pTabConnDataList->end(),*aConIter) );
            delete pTabConnData;
            continue;
        }

        // adds a new connection to join view and notifies our accessible and invaldates the controller
        addConnection(new OQueryTableConnection(this, pTabConnData));
    }
}

//------------------------------------------------------------------------
void OQueryTableView::ClearAll()
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    OJoinTableView::ClearAll();

    SetUpdateMode(sal_True);
    m_pView->getController()->setModified(sal_True);
}

// -----------------------------------------------------------------------------
OTableWindow* OQueryTableView::createWindow(OTableWindowData* _pData)
{
    return new OQueryTableWindow(this,static_cast<OQueryTableWindowData*>(_pData));
}

//------------------------------------------------------------------------------
void OQueryTableView::NotifyTabConnection(const OQueryTableConnection& rNewConn, sal_Bool _bCreateUndoAction)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    // erst mal schauen, ob ich diese Connection schon habe
    OQueryTableConnection* pTabConn = NULL;
    const ::std::vector<OTableConnection*>* pConnections = getTableConnections();
    ::std::vector<OTableConnection*>::const_iterator aIter = ::std::find(   pConnections->begin(),
                                                    pConnections->end(),
                                                    static_cast<const OTableConnection*>(&rNewConn)
                                                    );
    if(aIter == pConnections->end())
    {
        aIter = pConnections->begin();
        for(;aIter != pConnections->end();++aIter)
        {
            if(*static_cast<OQueryTableConnection*>(*aIter) == rNewConn)
            {
                pTabConn = static_cast<OQueryTableConnection*>(*aIter);
                break;
            }
        }
    }
    else
        pTabConn = static_cast<OQueryTableConnection*>(*aIter);
    // nein -> einfuegen
    if (pTabConn == NULL)
    {
        // die neuen Daten ...
        OQueryTableConnectionData* pNewData = static_cast< OQueryTableConnectionData*>(rNewConn.GetData()->NewInstance());
        pNewData->CopyFrom(*rNewConn.GetData());
        OQueryTableConnection* pNewConn = new OQueryTableConnection(this, pNewData);
        GetConnection(pNewConn);

        connectionModified(this,pNewConn,_bCreateUndoAction);
    }
}
// -----------------------------------------------------------------------------
OTableWindowData* OQueryTableView::CreateImpl(const ::rtl::OUString& _rComposedName,
                                             const ::rtl::OUString& _rWinName)
{
    return new OQueryTableWindowData( _rComposedName, _rWinName ,String());
}
//------------------------------------------------------------------------------
void OQueryTableView::AddTabWin(const ::rtl::OUString& strDatabase, const ::rtl::OUString& strTableName, sal_Bool bNewTable)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    // das ist die aus der Basisklasse geerbte Methode, die fuehre ich auf die an meinem Parent zurueck, die mir eventuell einen
    // Alias dazu bastelt und das an mein anderes AddTabWin weiterreicht

    // leider ist strTableName voll qualifiziert, das OQueryDesignView erwartet aber einen String, der
    // nur aus Schema und Tabelle besteht und keinen Katalog enthaelt.
    Reference< XConnection> xConnection = m_pView->getController()->getConnection();
    if(!xConnection.is())
        return;
    try
    {
        Reference< XDatabaseMetaData > xMetaData = xConnection->getMetaData();
        ::rtl::OUString sCatalog, sSchema, sTable;
        ::dbtools::qualifiedNameComponents(xMetaData,
                                    strDatabase,
                                    sCatalog,
                                    sSchema,
                                    sTable,
                                    ::dbtools::eInDataManipulation);
        ::rtl::OUString sRealName(sSchema);
        if (sRealName.getLength())
            sRealName+= ::rtl::OUString('.');
        sRealName += sTable;

        AddTabWin(strDatabase, sRealName, strTableName, bNewTable);
    }
    catch(SQLException&)
    {
        OSL_ASSERT(!"qualifiedNameComponents");
    }
}
// -----------------------------------------------------------------------------
// find the table which has a foreign key with this referencedTable name
Reference<XPropertySet> getKeyReferencedTo(const Reference<XKeysSupplier>& _rxKeys,const ::rtl::OUString& _rReferencedTable)
{
    if(!_rxKeys.is())
        return Reference<XPropertySet>();

    Reference< XIndexAccess> xKeyIndex = _rxKeys->getKeys();
    if ( !xKeyIndex.is() )
        return Reference<XPropertySet>();
    // search the one and only primary key
    for(sal_Int32 i=0;i< xKeyIndex->getCount();++i)
    {
        Reference<XPropertySet> xKey;
        ::cppu::extractInterface(xKey,xKeyIndex->getByIndex(i));
        if(xKey.is())
        {
            sal_Int32 nKeyType = 0;
            xKey->getPropertyValue(PROPERTY_TYPE) >>= nKeyType;
            if(KeyType::FOREIGN == nKeyType)
            {
                ::rtl::OUString sReferencedTable;
                xKey->getPropertyValue(PROPERTY_REFERENCEDTABLE) >>= sReferencedTable;
                // TODO check case
                if(sReferencedTable == _rReferencedTable)
                    return xKey;
            }
        }
    }
    return Reference<XPropertySet>();
}
//------------------------------------------------------------------------------
void OQueryTableView::AddTabWin(const ::rtl::OUString& _rComposedName, const ::rtl::OUString& strTableName, const ::rtl::OUString& strAlias, sal_Bool bNewTable)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    DBG_ASSERT(strTableName.getLength() || strAlias.getLength(), "OQueryTableView::AddTabWin : kein Tabellen- und kein Aliasname !");
        // wenn der Tabellenname nicht gesetzt ist, steht das fuer ein Dummy-Fenster, das braucht aber wenigstens einen Alias-Namen

    // neue Datenstruktur erzeugen
    // first check if this already hav it's data
    sal_Bool bAppend = bNewTable;
    OQueryTableWindowData* pNewTabWinData = NULL;
    ::std::vector< OTableWindowData*>* pWindowData = getDesignView()->getController()->getTableWindowData();
    ::std::vector< OTableWindowData*>::iterator aWinIter = pWindowData->begin();
    for(;aWinIter != pWindowData->end();++aWinIter)
    {
        pNewTabWinData = PTR_CAST(OQueryTableWindowData, *aWinIter);
        if (pNewTabWinData && pNewTabWinData->GetWinName() == strAlias && pNewTabWinData->GetComposedName() == _rComposedName && pNewTabWinData->GetTableName() == strTableName)
            break;
    }
    if(bAppend || (bAppend = (aWinIter == pWindowData->end())))
        pNewTabWinData = new OQueryTableWindowData(_rComposedName, strTableName, strAlias);
        // die TabWinData brauche ich nicht in die entsprechende Liste der DocShell eintragen, das macht ShowTabWin

    // neues Fenster erzeugen
    OQueryTableWindow* pNewTabWin = static_cast<OQueryTableWindow*>(createWindow(pNewTabWinData));
    // das Init kann ich hier weglassen, da das in ShowTabWin passiert

    // Neue UndoAction
    OQueryTabWinShowUndoAct* pUndoAction = new OQueryTabWinShowUndoAct(this);
    pUndoAction->SetTabWin(pNewTabWin); // Fenster
    sal_Bool bSuccess = ShowTabWin(pNewTabWin, pUndoAction,bAppend);
    if(!bSuccess)
    {
        // reset table window
        pUndoAction->SetTabWin(NULL);
        pUndoAction->SetOwnership(sal_False);

        delete pUndoAction;
        if(bAppend)
            delete pNewTabWinData;
        return;
    }

    // Relationen zwischen den einzelnen Tabellen anzeigen
    OTableWindowMap* pTabWins = GetTabWinMap();
    if(bNewTable && !pTabWins->empty() && strTableName.getLength())
    {
        modified();
        if ( m_pAccessible )
            m_pAccessible->notifyAccessibleEvent(   AccessibleEventId::CHILD,
                                                    Any(),
                                                    makeAny(pNewTabWin->GetAccessible())
                                                    );
        try
        {
            Reference<XConnection> xCon = m_pView->getController()->getConnection();
            OSL_ENSURE(xCon.is(),"OQueryTableView::AddTabWin connection is null!");
            if(!xCon.is())
                return;
            Reference<XTablesSupplier> xSup(xCon,UNO_QUERY);
            Reference<XNameAccess> xTables = xSup->getTables();
            Reference<XPropertySet> xTable;

            if(xTables->hasByName(strTableName) && ::cppu::extractInterface(xTable,xTables->getByName(strTableName)) && xTable.is())
            {
                //////////////////////////////////////////////////////////////////////
                // find relations between the table an the tables already inserted
                Reference<XKeysSupplier> xKeys(xTable,UNO_QUERY);
                Reference<XNameAccess> xFKeyColumns;
                ::rtl::OUString aReferencedTable;
                if(xKeys.is())
                {
                    Reference< XIndexAccess> xKeyIndex = xKeys->getKeys();
                    if ( xKeyIndex.is() )
                    {
                        Reference<XColumnsSupplier> xColumnsSupplier;
                        for(sal_Int32 i=0;i< xKeyIndex->getCount();++i)
                        {
                            Reference<XPropertySet> xProp;
                            ::cppu::extractInterface(xProp,xKeyIndex->getByIndex(i));
                            sal_Int32 nKeyType = 0;
                            xProp->getPropertyValue(PROPERTY_TYPE) >>= nKeyType;
                            xColumnsSupplier.set(xProp,UNO_QUERY);
                            OSL_ENSURE(xColumnsSupplier.is(),"Key isn't a column supplier");
                            xFKeyColumns = xColumnsSupplier->getColumns();
                            OSL_ENSURE(xFKeyColumns.is(),"No Key columns available!");

                            if(KeyType::FOREIGN == nKeyType)
                            {   // our new table has a foreign key
                                // so look if the referenced table is already in our list
                                xProp->getPropertyValue(PROPERTY_REFERENCEDTABLE) >>= aReferencedTable;
                                OSL_ENSURE(aReferencedTable.getLength(),"Foreign key without referencedTableName");

                                OTableWindowMap::const_iterator aIter = pTabWins->find(aReferencedTable);
                                if(aIter == pTabWins->end())
                                {
                                    for(aIter = pTabWins->begin();aIter != pTabWins->end();++aIter)
                                    {
                                        OQueryTableWindow* pTabWinTmp = static_cast<OQueryTableWindow*>(aIter->second);
                                        OSL_ENSURE(pTabWinTmp,"TableWindow is null!");
                                        if(pTabWinTmp != pNewTabWin && pTabWinTmp->GetComposedName() == aReferencedTable.getStr())
                                            break;
                                    }
                                }
                                if(aIter != pTabWins->end())
                                    addConnections(this,pNewTabWin,static_cast<OQueryTableWindow*>(aIter->second),xFKeyColumns);
                            }
                            else if(KeyType::PRIMARY == nKeyType)
                            {
                                // we have a primary key so look in our list if there exsits a key which this is refered to
                                OTableWindowMap::const_iterator aIter = pTabWins->begin();
                                for(;aIter != pTabWins->end();++aIter)
                                {
                                    OQueryTableWindow* pTabWinTmp = static_cast<OQueryTableWindow*>(aIter->second);
                                    if(pTabWinTmp != pNewTabWin)
                                    {
                                        OSL_ENSURE(pTabWinTmp,"TableWindow is null!");
                                        Reference<XPropertySet> xFKKey = getKeyReferencedTo(Reference<XKeysSupplier>(pTabWinTmp->GetTable(),UNO_QUERY),pNewTabWin->GetComposedName());
                                        if(xFKKey.is())
                                        {
                                            Reference<XColumnsSupplier> xFKColumnsSupplier(xFKKey,UNO_QUERY);
                                            OSL_ENSURE(xFKColumnsSupplier.is(),"Key isn't a column supplier");
                                            Reference<XNameAccess> xTColumns = xFKColumnsSupplier->getColumns();
                                            OSL_ENSURE(xTColumns.is(),"No Key columns available!");
                                            addConnections(this,pTabWinTmp,pNewTabWin,xTColumns);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        catch(const Exception&)
        {
        }
    }

    // mein Parent brauche ich, da es vom Loeschen erfahren soll
    m_pView->getController()->addUndoActionAndInvalidate( pUndoAction );

    if (bSuccess && m_lnkTabWinsChangeHandler.IsSet())
    {
        TabWinsChangeNotification aHint(TabWinsChangeNotification::AT_ADDED_WIN, pNewTabWin->GetAliasName());
        m_lnkTabWinsChangeHandler.Call(&aHint);
    }
}
//------------------------------------------------------------------------
void OQueryTableView::AddConnection(const OJoinExchangeData& jxdSource, const OJoinExchangeData& jxdDest)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    OQueryTableWindow* pSourceWin = static_cast< OQueryTableWindow*>(jxdSource.pListBox->GetTabWin());
    OQueryTableWindow* pDestWin = static_cast< OQueryTableWindow*>(jxdDest.pListBox->GetTabWin());

    String aSourceFieldName, aDestFieldName;
    aSourceFieldName    = jxdSource.pListBox->GetEntryText(jxdSource.pEntry);
    aDestFieldName      = jxdDest.pListBox->GetEntryText(jxdDest.pEntry);

    OTableConnection* pConn = GetTabConn(pSourceWin,pDestWin);
    if(!pConn)
    {
        // neues Daten-Objekt
        OQueryTableConnectionData aNewConnectionData(pSourceWin->GetTableName(), pDestWin->GetTableName(), pSourceWin->GetAliasName(), pDestWin->GetAliasName());

        sal_uInt32          nSourceFieldIndex, nDestFieldIndex;
        ETableFieldType eSourceFieldType, eDestFieldType;

        // Namen/Position/Typ der beiden betroffenen Felder besorgen ...
        // Source

        nSourceFieldIndex = jxdSource.pListBox->GetModel()->GetAbsPos(jxdSource.pEntry);
        eSourceFieldType = static_cast< OTableFieldInfo*>(jxdSource.pEntry->GetUserData())->GetKeyType();

        // Dest

        nDestFieldIndex = jxdDest.pListBox->GetModel()->GetAbsPos(jxdDest.pEntry);
        eDestFieldType = static_cast< OTableFieldInfo*>(jxdDest.pEntry->GetUserData())->GetKeyType();

        // ... und setzen

        aNewConnectionData.SetFieldIndex(JTCS_FROM, nSourceFieldIndex);
        aNewConnectionData.SetFieldIndex(JTCS_TO, nDestFieldIndex);

        aNewConnectionData.SetFieldType(JTCS_FROM, eSourceFieldType);
        aNewConnectionData.SetFieldType(JTCS_TO, eDestFieldType);

        aNewConnectionData.AppendConnLine( aSourceFieldName,aDestFieldName );

        OQueryTableConnection aNewConnection(this, &aNewConnectionData);
        NotifyTabConnection(aNewConnection);
            // wie immer bei NotifyTabConnection ist das Verwenden lokaler Variablen unkritisch, da sowieso eine Kopie erzeugt wird
    }
    else
    {
        // the connection could point on the other side
        if(pConn->GetSourceWin() == pDestWin)
        {
            String aTmp(aSourceFieldName);
            aSourceFieldName = aDestFieldName;
            aDestFieldName = aTmp;
        }

        pConn->GetData()->AppendConnLine( aSourceFieldName,aDestFieldName );

        connectionModified(this,pConn,sal_False);
    }
}
// -----------------------------------------------------------------------------
void OQueryTableView::ConnDoubleClicked(OTableConnection* pConnection)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    DBG_ASSERT(pConnection->ISA(OQueryTableConnection), "OQueryTableView::ConnDoubleClicked : pConnection hat falschen Typ");
        // da ich nur solche selber verwende, duerfen auch nur solche hier ankommen

    if( openJoinDialog(this,pConnection->GetData(),FALSE) )
    {
        connectionModified(this,pConnection,sal_False);
        SelectConn( pConnection );
    }
}
// -----------------------------------------------------------------------------
void OQueryTableView::createNewConnection()
{
    OQueryTableConnectionData* pData = new OQueryTableConnectionData();
    if( openJoinDialog(this,pData,TRUE) )
    {
        OTableWindowMap* pMap = GetTabWinMap();
        OQueryTableWindow* pSourceWin   = static_cast< OQueryTableWindow*>((*pMap)[pData->GetSourceWinName()]);
        OQueryTableWindow* pDestWin     = static_cast< OQueryTableWindow*>((*pMap)[pData->GetDestWinName()]);
        // first we have to look if the this connection already exists
        OTableConnection* pConn = GetTabConn(pSourceWin,pDestWin);
        sal_Bool bNew = sal_True;
        if ( pConn )
        {
            pConn->GetData()->CopyFrom( *pData );
            delete pData;
            bNew = sal_False;
        }
        else
        {
            // create a new conenction and append it
            OQueryTableConnection* pQConn = new OQueryTableConnection(this, pData);
            GetConnection(pQConn);
            pConn = pQConn;
        }
        connectionModified(this,pConn,bNew);
        if ( !bNew && pConn == GetSelectedConn() ) // our connection was selected before so we have to reselect it
            SelectConn( pConn );
    }
    else
        delete pData;
}
//------------------------------------------------------------------------------
::std::vector<OTableConnection*>::const_iterator OQueryTableView::RemoveConnection( OTableConnection* _pConnection,sal_Bool _bDelete )
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    DBG_ASSERT(_pConnection->ISA(OQueryTableConnection), "OQueryTableView::RemoveConnection : Connection ist vom falschen Typ !");
    // NICHT die Basisklasse erledigen lassen (die loescht die Connection hart, ich will sie aber ans Undo uebergeben)

    // we don't want that our connection will be deleted, we put it in the undo manager
    ::std::vector<OTableConnection*>::const_iterator aNextPos = OJoinTableView::RemoveConnection( _pConnection,sal_False);

    // add undo action
    addUndoAction(  this,
                    new OQueryDelTabConnUndoAction(this),
                    static_cast< OQueryTableConnection*>(_pConnection),
                    sal_True);
    return aNextPos;
}

//------------------------------------------------------------------------------
void OQueryTableView::KeyInput( const KeyEvent& rEvt )
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    OJoinTableView::KeyInput( rEvt );
}

//------------------------------------------------------------------------------
OQueryTableWindow* OQueryTableView::FindTable(const String& rAliasName)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    DBG_ASSERT(rAliasName.Len(), "OQueryTableView::FindTable : der AliasName sollte nicht leer sein !");
        // (nicht dass es schadet, aber es ist sinnlos und weist vielleicht auf Fehler beim Aufrufer hin)
    OTableWindowMap::const_iterator aIter = GetTabWinMap()->find(rAliasName);
    if(aIter != GetTabWinMap()->end())
        return static_cast<OQueryTableWindow*>(aIter->second);
    return NULL;
}

//------------------------------------------------------------------------------
sal_Bool OQueryTableView::FindTableFromField(const String& rFieldName, OTableFieldDescRef& rInfo, sal_uInt16& rCnt)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    sal_Bool bRet = sal_False;
    rCnt = 0;
    OTableWindowMap::const_iterator aIter = GetTabWinMap()->begin();
    for(;aIter != GetTabWinMap()->end();++aIter)
    {
        if(static_cast<OQueryTableWindow*>(aIter->second)->ExistsField(rFieldName, rInfo))
            ++rCnt;
    }

    return rCnt == 1;
}

//------------------------------------------------------------------------------
sal_Bool OQueryTableView::RemoveTabWin(const String& strAliasName)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    OQueryTableWindow* pTabWin = FindTable(strAliasName);
    if ( pTabWin )
        RemoveTabWin(pTabWin);
    return pTabWin != NULL;
}

//------------------------------------------------------------------------------
void OQueryTableView::RemoveTabWin(OTableWindow* pTabWin)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    DBG_ASSERT(pTabWin != NULL, "OQueryTableView::RemoveTabWin : Fenster sollte ungleich NULL sein !");
    DBG_ASSERT(pTabWin->ISA(OQueryTableWindow), "OQueryTableView::RemoveTabWin : Fenster sollte ein OQueryTableWindow sein !");

    // mein Parent brauche ich, da es vom Loeschen erfahren soll
    OQueryDesignView* pParent = static_cast<OQueryDesignView*>(getDesignView());

    SfxUndoManager* pUndoMgr = m_pView->getController()->getUndoMgr();
    pUndoMgr->EnterListAction( String( ModuleRes(STR_QUERY_UNDO_TABWINDELETE) ), String() );

    // Undo-Action anlegen
    OQueryTabWinDelUndoAct* pUndoAction = new OQueryTabWinDelUndoAct(this);
    pUndoAction->SetTabWin(static_cast< OQueryTableWindow*>(pTabWin));

    // und Fenster verstecken
    HideTabWin(static_cast< OQueryTableWindow*>(pTabWin), pUndoAction);

    // Undo Actions und Loeschen der Felder in SelectionBrowseBox
    pParent->TableDeleted( static_cast< OQueryTableWindowData*>(pTabWin->GetData())->GetAliasName() );

    m_pView->getController()->addUndoActionAndInvalidate( pUndoAction );
    pUndoMgr->LeaveListAction();

    if (m_lnkTabWinsChangeHandler.IsSet())
    {
        TabWinsChangeNotification aHint(TabWinsChangeNotification::AT_REMOVED_WIN, static_cast< OQueryTableWindow*>(pTabWin)->GetAliasName());
        m_lnkTabWinsChangeHandler.Call(&aHint);
    }

    modified();
    if ( m_pAccessible )
        m_pAccessible->notifyAccessibleEvent(   AccessibleEventId::CHILD,
                                                makeAny(pTabWin->GetAccessible()),
                                                Any()
                                                );
}

//------------------------------------------------------------------------
void OQueryTableView::EnsureVisible(const OTableWindow* pWin)
{
    DBG_CHKTHIS(OQueryTableView,NULL);

    Invalidate(INVALIDATE_NOCHILDREN);
    OJoinTableView::EnsureVisible(pWin);
}

//------------------------------------------------------------------------
void OQueryTableView::GetConnection(OQueryTableConnection* pConn)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    // bei mir und dem Dokument einfuegen

    addConnection( pConn );
    // invalidieren (damit es neu gezeichnet wird)
    //  pConn->Invalidate();
}

//------------------------------------------------------------------------
void OQueryTableView::DropConnection(OQueryTableConnection* pConn)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    // Selektion beachten
    // bei mir und dem Dokument rausnehmen
    RemoveConnection( pConn ,sal_False);
}

//------------------------------------------------------------------------
void OQueryTableView::HideTabWin( OQueryTableWindow* pTabWin, OQueryTabWinUndoAct* pUndoAction )
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    OTableWindowMap* pTabWins = GetTabWinMap();
    DBG_ASSERT(pTabWins != NULL, "OQueryTableView::HideTabWin : habe keine TabWins !");

    if (pTabWin)
    {
        // Fenster
        // die Position in seinen Daten speichern
        getDesignView()->SaveTabWinUIConfig(pTabWin);
            // (ich muss ueber das Parent gehen, da nur das die Position der Scrollbars kennt)
        // dann aus der Liste der TabWins raus und verstecken
        OTableWindowMap::iterator aIter = pTabWins->begin();
        for(;aIter != pTabWins->end();++aIter)
            if(aIter->second == pTabWin)
                pTabWins->erase(aIter);

        pTabWin->Hide();    // nicht zerstoeren, steht im Undo!!

        // die Daten zum TabWin muessen auch aus meiner Verantwortung entlassen werden
        ::std::vector< OTableWindowData*>* pTabWinDataList = m_pView->getController()->getTableWindowData();
        pTabWinDataList->erase( ::std::find(pTabWinDataList->begin(),pTabWinDataList->end(),pTabWin->GetData()));
            // NICHT loeschen, da ja das TabWin selber - das noch lebt - sie auch noch braucht
            // Entweder geht es irgendwann wieder in meine Verantwortung ueber, (ueber ShowTabWin), dann fuege ich
            // auch die Daten wieder ein, oder die Undo-Action, die im Augenblick die alleinige Verantwortung fuer das Fenster
            // und dessen Daten hat, wird zestoert, dann loescht es beides

        if (m_pLastFocusTabWin == pTabWin)
            m_pLastFocusTabWin = NULL;

        // Verbindungen, die zum Fenster gehoeren, einsammeln und der UndoAction uebergeben
        ::std::vector< OTableConnectionData*>* pTabConnDataList = m_pView->getController()->getTableConnectionData();

        sal_Int16 nCnt = 0;
        const ::std::vector<OTableConnection*>* pTabConList = getTableConnections();
        ::std::vector<OTableConnection*>::const_iterator aIter2 = pTabConList->begin();
        for(;aIter2 != pTabConList->end();)
        {
            OQueryTableConnection* pTmpEntry = static_cast<OQueryTableConnection*>(*aIter2);
            OSL_ENSURE(pTmpEntry,"OQueryTableConnection is null!");
            if( pTmpEntry->GetAliasName(JTCS_FROM) == pTabWin->GetAliasName() ||
                pTmpEntry->GetAliasName(JTCS_TO) == pTabWin->GetAliasName() )
            {
                // add to undo list
                pUndoAction->InsertConnection(pTmpEntry);

                // call base class because we append an undo action
                // but this time we are in a undo action list
                aIter2 = OJoinTableView::RemoveConnection(pTmpEntry,sal_False);
                ++nCnt;
            }
            else
                ++aIter2;
        }

        if (nCnt)
            InvalidateConnections();

        m_pView->getController()->InvalidateFeature(ID_BROWSER_ADDTABLE);

        // der UndoAction sagen, dass das Fenster (inklusive der Connections) jetzt in seinem Besitzt ist
        pUndoAction->SetOwnership(sal_True);

        // damit habe ich das Doc natuerlich modifiziert
        m_pView->getController()->setModified( sal_True );
        m_pView->getController()->InvalidateFeature(ID_BROWSER_CLEAR_QUERY);
    }
}

//------------------------------------------------------------------------
sal_Bool OQueryTableView::ShowTabWin( OQueryTableWindow* pTabWin, OQueryTabWinUndoAct* pUndoAction,sal_Bool _bAppend )
{
    DBG_CHKTHIS(OQueryTableView,NULL);

    sal_Bool bSuccess = sal_False;

    if (pTabWin)
    {
        if (pTabWin->Init())
        {
            OTableWindowData* pData = pTabWin->GetData();
            DBG_ASSERT(pData != NULL, "OQueryTableView::ShowTabWin : TabWin hat keine Daten !");
            // Wenn die Daten schon PosSize haben, diese benutzen
            if (pData->HasPosition() && pData->HasSize())
            {
                Size aSize(CalcZoom(pData->GetSize().Width()),CalcZoom(pData->GetSize().Height()));
                pTabWin->SetPosSizePixel(pData->GetPosition(), aSize);
            }
            else
                // ansonsten selber eine Default-Position ermitteln
                SetDefaultTabWinPosSize(pTabWin);

            // Fenster zeigen und in Liste eintragen
            ::rtl::OUString sName = static_cast< OQueryTableWindowData*>(pData)->GetAliasName();
            OSL_ENSURE(GetTabWinMap()->find(sName) == GetTabWinMap()->end(),"Alias name already in list!");
            GetTabWinMap()->insert(OTableWindowMap::value_type(sName,pTabWin));

            pTabWin->Show();

            pTabWin->Update();
                // Das Update ist notwendig, damit die Connections an dem Fenster richtig gezeichnet werden. Klingt absurd,
                // ich weiss. Aber die Listbox haelt sich intern ein Member, was bei ersten Zeichnen (nachdem die Listbox im Init
                // gerade neu gefuellt wurde) initialisiert wird, und genau dieses Member wird irgendwann benoetigt fuer
                // GetEntryPos, und dieses wiederum von der Connection, wenn sie ihren Ansatzpunkt am Fenster feststellen will.

            // die Connections
            ::std::vector<OTableConnection*>* pTableCon = pUndoAction->GetTabConnList();
            ::std::vector<OTableConnection*>::iterator aIter = pTableCon->begin();

            const ::std::vector<OTableConnection*>* pOwnList = getTableConnections();
            for(;aIter != pTableCon->end();++aIter)
                addConnection(*aIter); // add all connections from the undo action

            // each connection should invalidated inside addConnection so we don't need this here any longer
//          if ( !pOwnList->empty() )
//              InvalidateConnections();
            pTableCon->clear();

            // und die Daten des Fensters ebenfalls in Liste (des Docs)
            if(_bAppend)
                m_pView->getController()->getTableWindowData()->push_back(pTabWin->GetData());

            m_pView->getController()->InvalidateFeature(ID_BROWSER_ADDTABLE);

            // und der UndoAction sagen, dass das Fenster jetzt meine ist ...
            pUndoAction->SetOwnership(sal_False);

            bSuccess = sal_True;
        }
        else
        {
            //////////////////////////////////////////////////////////////////
            // Initialisierung fehlgeschlagen
            // (z.B. wenn Verbindung zur Datenbank in diesem Augenblick unterbrochen worden ist)
            pTabWin->clearListBox();
            delete pTabWin;
        }
    }

    // damit habe ich das Doc natuerlich modifiziert
    if(!m_pView->getController()->isReadOnly())
        m_pView->getController()->setModified( sal_True );

    m_pView->getController()->InvalidateFeature(ID_BROWSER_CLEAR_QUERY);

    return bSuccess;
}


//------------------------------------------------------------------------
void OQueryTableView::InsertField(const OTableFieldDescRef& rInfo)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    DBG_ASSERT(getDesignView() != NULL, "OQueryTableView::InsertField : habe kein Parent !");
    static_cast<OQueryDesignView*>(getDesignView())->InsertField(rInfo);
}
//------------------------------------------------------------------------------
sal_Bool OQueryTableView::ExistsAVisitedConn(const OQueryTableWindow* pFrom) const
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    const ::std::vector<OTableConnection*>* pList = getTableConnections();
    if (pList)
    {
        ::std::vector<OTableConnection*>::const_iterator aIter = pList->begin();
        for(;aIter != pList->end();++aIter)
        {
            OQueryTableConnection* pTemp = static_cast<OQueryTableConnection*>(*aIter);
            if (pTemp->IsVisited() &&
                (pFrom == static_cast< OQueryTableWindow*>(pTemp->GetSourceWin()) || pFrom == static_cast< OQueryTableWindow*>(pTemp->GetDestWin())))
                return pTemp != NULL;
        }
    }

    return sal_False;
}
// -----------------------------------------------------------------------------


