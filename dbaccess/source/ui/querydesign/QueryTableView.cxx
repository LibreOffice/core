/*************************************************************************
 *
 *  $RCSfile: QueryTableView.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-23 09:46:40 $
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
#ifndef DBAUI_OQUERYMOVETABWINUNDOACT_HXX
#include "QueryMoveTabWinUndoAct.hxx"
#endif
#ifndef DBAUI_QUERYADDTABCONNUNDOACTION_HXX
#include "QueryAddTabConnUndoAction.hxx"
#endif
#ifndef DBAUI_QUERYTABWINSHOWUNDOACT_HXX
#include "QueryTabWinShowUndoAct.hxx"
#endif
#ifndef DBAUI_QUERYSIZETABWINUNDOACT_HXX
#include "QuerySizeTabWinUndoAct.hxx"
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

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

TYPEINIT1(OQueryTableView, OJoinTableView);

//------------------------------------------------------------------------------
::rtl::OUString ConvertAlias(const ::rtl::OUString& _rName)
{
    if (!_rName.getLength())
        return _rName;

    String rName(_rName);

    const sal_Unicode* pStr = rName.GetBuffer();
    sal_Bool bValid(!((*pStr >= 48) && (*pStr <=  57)));    // keine Zahl am Anfang

    String aTmp;
    if (bValid)
        aTmp = rName;

    for (sal_Int32 i=0; i < rName.Len() && *pStr;i++, pStr++ ){
        if ( ((*pStr >= 97) && (*pStr <= 122)) ||((*pStr >= 65) && (*pStr <=  90)) ||
             ((*pStr >= 48) && (*pStr <=  57)) ||  *pStr == '_'                    )
              ;
        else
            aTmp.SearchAndReplace(*pStr,'_');

    }

    return aTmp;
}

//==================================================================
// class OQueryTableView
//==================================================================
DBG_NAME(OQueryTableView);
//------------------------------------------------------------------------
OQueryTableView::OQueryTableView( Window* pParent,OQueryDesignView* pView)
    : OJoinTableView( pParent,pView)
{
    DBG_CTOR(OQueryTableView,NULL);
    SetHelpId(HID_CTL_QRYDGNTAB);
    EnableDrop();
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
void OQueryTableView::TabWinMoved(OTableWindow* pWhich, const Point& ptOldPosition)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    OJoinTableView::TabWinMoved(pWhich, ptOldPosition);

    SfxUndoAction* pUndoAction = new OQueryMoveTabWinUndoAct(this, ptOldPosition, static_cast< OQueryTableWindow*>(pWhich));
    m_pView->getController()->getUndoMgr()->AddUndoAction(pUndoAction);
}

//------------------------------------------------------------------------
void OQueryTableView::TabWinSized(OTableWindow* pWhich, const Point& ptOldPosition, const Size& szOldSize)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    OJoinTableView::TabWinSized(pWhich, ptOldPosition, szOldSize);

    SfxUndoAction* pUndoAction = new OQuerySizeTabWinUndoAct(this, ptOldPosition, szOldSize, static_cast< OQueryTableWindow*>(pWhich));
    m_pView->getController()->getUndoMgr()->AddUndoAction(pUndoAction);
}

//------------------------------------------------------------------------
void OQueryTableView::ReSync()
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    ::std::vector< OTableWindowData*>* pTabWinDataList = m_pView->getController()->getTableWindowData();
    DBG_ASSERT((GetTabConnList()->size()==0) && (GetTabWinMap()->size()==0),
        "vor OQueryTableView::ReSync() bitte ClearAll aufrufen !");

    // ich brauche eine Sammlung aller Fensternamen, deren Anlegen schief geht, damit ich die entsprechenden Connections
    // gar nicht erst anlege
    ::std::vector<String> arrInvalidTables;

    ::std::vector< OTableWindowData*>::reverse_iterator aIter = pTabWinDataList->rbegin();
    // Fenster kreieren und einfuegen

    for(;aIter != pTabWinDataList->rend();++aIter)
    {
        OQueryTableWindowData* pData = static_cast<OQueryTableWindowData*>(*aIter);
        OQueryTableWindow* pTabWin = new OQueryTableWindow(this, pData);

        // ich gehe jetzt NICHT ueber ShowTabWin, da dieses die Daten des Fensters in die Liste des Docs einfuegt, was
        // schlecht waere, denn genau von dort hole ich sie ja gerade
        // also Schritt fuer Schritt
        if (!pTabWin->Init())
        {
            // das Initialisieren ging schief, dass heisst, dieses TabWin steht nicht zur Verfuegung, also muss ich es inklusive
            // seiner Daten am Dokument aufraeumen
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

        GetTabConnList()->push_back(new OQueryTableConnection(this, pTabConnData));
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

//------------------------------------------------------------------------------
void OQueryTableView::NotifyTabConnection(const OQueryTableConnection& rNewConn, sal_Bool _bCreateUndoAction)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    // erst mal schauen, ob ich diese Connection schon habe
    OQueryTableConnection* pTabConn = NULL;
    ::std::vector<OTableConnection*>::iterator aIter = ::std::find(GetTabConnList()->begin(),GetTabConnList()->end(),static_cast<const OTableConnection*>(&rNewConn));
    if(aIter == GetTabConnList()->end())
    {
        aIter = GetTabConnList()->begin();
        for(;aIter != GetTabConnList()->end();++aIter)
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
        // ... an das Dokument anhaengen
        m_pView->getController()->getTableConnectionData()->push_back(pNewData);
        // ... und an eine neue OQueryTableConnection-Instanz haengen, die ich mir gleich merke
        OQueryTableConnection* pNewConn = new OQueryTableConnection(this, pNewData);
        GetTabConnList()->push_back(pNewConn);

        // Modified-Flag
        m_pView->getController()->setModified(sal_True);

        // eine Undo-Action fuer die Connection
        if (_bCreateUndoAction)
        {
            OQueryTabConnUndoAction* pUndoAction = new OQueryAddTabConnUndoAction(this);
            pUndoAction->SetOwnership(sal_False);
            pUndoAction->SetConnection(pNewConn);
            m_pView->getController()->getUndoMgr()->AddUndoAction(pUndoAction);
        }

        // und neu zeichnen
        pNewConn->RecalcLines();
            // fuer das unten folgende Invalidate muss ich dieser neuen Connection erst mal die Moeglichkeit geben,
            // ihr BoundingRect zu ermitteln
        pNewConn->Invalidate();
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
    ::rtl::OUString sCatalog, sSchema, sTable;
    ::dbtools::qualifiedNameComponents(xConnection->getMetaData(),
                                strDatabase,
                                sCatalog,
                                sSchema,
                                sTable);
    ::rtl::OUString sRealName(sSchema);
    if (sRealName.getLength())
        sRealName+= ::rtl::OUString('.');
    sRealName += sTable;

    AddTabWin(strDatabase, sRealName, ConvertAlias(sTable), bNewTable);
}
// -----------------------------------------------------------------------------
// find the table which has a foreign key with this referencedTable name
Reference<XPropertySet> getKeyReferencedTo(const Reference<XKeysSupplier>& _rxKeys,const ::rtl::OUString& _rReferencedTable)
{
    if(!_rxKeys.is())
        return Reference<XPropertySet>();

    Reference< XIndexAccess> xKeyIndex = _rxKeys->getKeys();
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
// -----------------------------------------------------------------------------
sal_Bool isColumnInKeyType(const Reference<XKeysSupplier>& _rxKeys,const ::rtl::OUString& _rColumnName,sal_Int32 _nKeyType)
{
    sal_Bool bReturn = sal_False;
    if(_rxKeys.is())
    {
        Reference< XIndexAccess> xKeyIndex = _rxKeys->getKeys();
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
                    xColumnsSupplier = Reference<XColumnsSupplier>(xProp,UNO_QUERY);
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
void OQueryTableView::addConnections(const OQueryTableWindow* _pSource,const OQueryTableWindow* _pDest,const Reference<XNameAccess>& _rxSourceForeignKeyColumns)
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
        OQueryTableConnection aNewConn(this, &aNewConnData);
            // der Verweis auf die lokale Variable ist unkritisch, da NotifyQueryTabConn eine neue Kopie anlegt
        // und mir hinzufuegen (wenn nicht schon existent)
        NotifyTabConnection(aNewConn, sal_False);
            // don't create an Undo-Action for the new connection : the connection is
            // covered by the Undo-Action for the tabwin, as the "Undo the insert" will
            // automatically remove all connections adjacent to the win.
            // (Because of this automatism we would have an ownerhsip ambiguity for
            // the connection data if we would insert the conn-Undo-Action)
            // FS - 21.10.99 - 69183
    }
}
//------------------------------------------------------------------------------
void OQueryTableView::AddTabWin(const ::rtl::OUString& _rComposedName, const ::rtl::OUString& strTableName, const ::rtl::OUString& strAlias, sal_Bool bNewTable)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    DBG_ASSERT(strTableName.getLength() || strAlias.getLength(), "OQueryTableView::AddTabWin : kein Tabellen- und kein Aliasname !");
        // wenn der Tabellenname nicht gesetzt ist, steht das fuer ein Dummy-Fenster, das braucht aber wenigstens einen Alias-Namen

    // neue Datenstruktur erzeugen
    // first check if this already hav it's data
    sal_Bool bAppend = sal_True;
    OQueryTableWindowData* pNewTabWinData = NULL;
//  ::std::vector< OTableWindowData*>* pWindowData = getDesignView()->getController()->getTableWindowData();
//  ::std::vector< OTableWindowData*>::iterator aWinIter = pWindowData->begin();
//  for(;aWinIter != pWindowData->end();++aWinIter)
//  {
//      pNewTabWinData = PTR_CAST(OQueryTableWindowData, *aWinIter);
//      if (pNewTabWinData && pNewTabWinData->GetWinName() == strAlias && pNewTabWinData->GetComposedName() == _rComposedName && pNewTabWinData->GetTableName() == strTableName)
//          break;
//  }
//  if(bAppend = (aWinIter == pWindowData->end()))
        pNewTabWinData = new OQueryTableWindowData(_rComposedName, strTableName, strAlias);
        // die TabWinData brauche ich nicht in die entsprechende Liste der DocShell eintragen, das macht ShowTabWin

    // neues Fenster erzeugen
    OQueryTableWindow* pNewTabWin = new OQueryTableWindow(this, pNewTabWinData);
    // das Init kann ich hier weglassen, da das in ShowTabWin passiert

    // Neue UndoAction
    OQueryTabWinShowUndoAct* pUndoAction = new OQueryTabWinShowUndoAct(this);
    pUndoAction->SetTabWin(pNewTabWin); // Fenster
    sal_Bool bSuccess = ShowTabWin(pNewTabWin, pUndoAction,bAppend);

    // Relationen zwischen den einzelnen Tabellen anzeigen
    OTableWindowMap* pTabWins = GetTabWinMap();
    if(bNewTable && pTabWins->size() && strTableName.getLength())
    {
        Reference<XConnection> xCon = m_pView->getController()->getConnection();
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
                Reference<XColumnsSupplier> xColumnsSupplier;
                for(sal_Int32 i=0;i< xKeyIndex->getCount();++i)
                {
                    Reference<XPropertySet> xProp;
                    ::cppu::extractInterface(xProp,xKeyIndex->getByIndex(i));
                    sal_Int32 nKeyType = 0;
                    xProp->getPropertyValue(PROPERTY_TYPE) >>= nKeyType;
                    xColumnsSupplier = Reference<XColumnsSupplier>(xProp,UNO_QUERY);
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
                            addConnections(pNewTabWin,static_cast<OQueryTableWindow*>(aIter->second),xFKeyColumns);
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
                                    Reference<XColumnsSupplier> xFKColumnsSupplier = Reference<XColumnsSupplier>(xFKKey,UNO_QUERY);
                                    OSL_ENSURE(xFKColumnsSupplier.is(),"Key isn't a column supplier");
                                    Reference<XNameAccess> xTColumns = xFKColumnsSupplier->getColumns();
                                    OSL_ENSURE(xTColumns.is(),"No Key columns available!");
                                    addConnections(pTabWinTmp,pNewTabWin,xTColumns);
                                }
                            }
                        }
                    }
                }

            }
        }
    }

    // mein Parent brauche ich, da es vom Loeschen erfahren soll
    m_pView->getController()->setModified(sal_True);

    m_pView->getController()->getUndoMgr()->AddUndoAction( pUndoAction );

    if (bSuccess && m_lnkTabWinsChangeHandler.IsSet())
    {
        TabWinsChangeNotification aHint(TabWinsChangeNotification::AT_ADDED_WIN, pNewTabWin->GetAliasName());
        m_lnkTabWinsChangeHandler.Call(&aHint);
    }

    m_pView->getController()->InvalidateFeature(ID_BROWSER_UNDORECORD);
    m_pView->getController()->InvalidateFeature(ID_BROWSER_REDO);
    //  GetViewShell()->GetViewShell()->UIFeatureChanged();
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
        pConn->UpdateLineList();
        // Modified-Flag
        m_pView->getController()->setModified(sal_True);

        // eine Undo-Action fuer die Connection
        OQueryTabConnUndoAction* pUndoAction = new OQueryAddTabConnUndoAction(this);
        pUndoAction->SetOwnership(sal_False);
        pUndoAction->SetConnection(static_cast< OQueryTableConnection*>(pConn));
        m_pView->getController()->getUndoMgr()->AddUndoAction(pUndoAction);

        // und neu zeichnen
        pConn->RecalcLines();
            // fuer das unten folgende Invalidate muss ich dieser neuen Connection erst mal die Moeglichkeit geben,
            // ihr BoundingRect zu ermitteln
        pConn->Invalidate();
    }
}


//------------------------------------------------------------------------
void OQueryTableView::ConnDoubleClicked(OTableConnection* pConnection)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    DBG_ASSERT(pConnection->ISA(OQueryTableConnection), "OQueryTableView::ConnDoubleClicked : pConnection hat falschen Typ");
        // da ich nur solche selber verwende, duerfen auch nur solche hier ankommen

    DlgQryJoin aDlg(this,static_cast< OQueryTableConnectionData*>(pConnection->GetData()),m_pView->getController()->getConnection()->getMetaData());
    aDlg.Execute();
    if(static_cast< OQueryTableConnectionData*>(pConnection->GetData())->GetJoinType() != aDlg.GetJoinType())
    {
        static_cast< OQueryTableConnectionData*>(pConnection->GetData())->SetJoinType(aDlg.GetJoinType());
        m_pView->getController()->setModified(sal_True);
    }
/*
    String strMessage(ModuleRes(RID_STR_CONNECTION_DATA));
    strMessage.SearchAndReplace("$alias1$", static_cast< OQueryTableConnection*>(pConnection)->GetAliasName(JTCS_FROM));
    strMessage.SearchAndReplace("$alias2$", static_cast< OQueryTableConnection*>(pConnection)->GetAliasName(JTCS_TO));
    strMessage.SearchAndReplace("$field1$", static_cast< OQueryTableConnection*>(pConnection)->GetFieldName(JTCS_FROM));
    strMessage.SearchAndReplace("$field2$", static_cast< OQueryTableConnection*>(pConnection)->GetFieldName(JTCS_TO));

    InfoBox(this, strMessage).Execute();
*/
}

//------------------------------------------------------------------------------
sal_Bool OQueryTableView::RemoveConnection( OTableConnection* pConn )
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    DBG_ASSERT(pConn->ISA(OQueryTableConnection), "OQueryTableView::RemoveConnection : Connection ist vom falschen Typ !");
    // NICHT die Basisklasse erledigen lassen (die loescht die Connection hart, ich will sie aber ans Undo uebergeben)
    DropConnection(static_cast< OQueryTableConnection*>(pConn));

    // eine Undo-Action
    OQueryTabConnUndoAction* pUndoAction = new OQueryDelTabConnUndoAction(this);
    pUndoAction->SetOwnership(sal_True);
    pUndoAction->SetConnection(static_cast< OQueryTableConnection*>(pConn));
    m_pView->getController()->getUndoMgr()->AddUndoAction(pUndoAction);

    // modified-Flag
    m_pView->getController()->setModified(sal_True);
    return sal_True;
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
sal_Bool OQueryTableView::FindTableFromField(const String& rFieldName, OTableFieldDesc& rInfo, sal_uInt16& rCnt)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    sal_Bool bRet = sal_False;
    rCnt = 0;
    OTableWindowMap::const_iterator aIter = GetTabWinMap()->begin();
    for(;aIter != GetTabWinMap()->end();++aIter)
    {
        if(static_cast<OQueryTableWindow*>(aIter->second)->ExistsField(rFieldName, rInfo))
            rCnt++;
    }

    return rCnt == 1;
}

//------------------------------------------------------------------------------
sal_Bool OQueryTableView::RemoveTabWin(const String& strAliasName)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    OQueryTableWindow* pTabWin = FindTable(strAliasName);
    if (!pTabWin)
        return sal_False;

    RemoveTabWin(pTabWin);
    return sal_True;
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

    // Undo Actions und Loeschen der Felder in SelectionBrowseBox
    pParent->TableDeleted( static_cast< OQueryTableWindowData*>(pTabWin->GetData())->GetAliasName() );
    m_pView->getController()->setModified(sal_True);

    // Undo-Action anlegen
    OQueryTabWinDelUndoAct* pUndoAction = new OQueryTabWinDelUndoAct(this);
    pUndoAction->SetTabWin(static_cast< OQueryTableWindow*>(pTabWin));

    // und Fenster verstecken
    HideTabWin(static_cast< OQueryTableWindow*>(pTabWin), pUndoAction);

    pUndoMgr->AddUndoAction( pUndoAction );
    pUndoMgr->LeaveListAction();

    m_pView->getController()->InvalidateFeature(ID_BROWSER_UNDORECORD);
    m_pView->getController()->InvalidateFeature(ID_BROWSER_REDO);
    //  GetViewShell()->GetViewShell()->UIFeatureChanged();

    if (m_lnkTabWinsChangeHandler.IsSet())
    {
        TabWinsChangeNotification aHint(TabWinsChangeNotification::AT_REMOVED_WIN, static_cast< OQueryTableWindow*>(pTabWin)->GetAliasName());
        m_lnkTabWinsChangeHandler.Call(&aHint);
    }
}

//------------------------------------------------------------------------------
void OQueryTableView::HideTabWins()
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    SetUpdateMode(sal_False);

    OTableWindowMap* pTabWins = GetTabWinMap();
    if (pTabWins)
    {
        OTableWindowMap::const_iterator aIter = GetTabWinMap()->begin();
        for(;aIter != GetTabWinMap()->end();++aIter)
            RemoveTabWin(aIter->second);
    }

    m_pView->getController()->setModified(sal_True);

    SetUpdateMode(sal_True);

}

//------------------------------------------------------------------------
void OQueryTableView::EnsureVisible(const OTableWindow* pWin)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    OJoinTableView::EnsureVisible(pWin);
    Invalidate(INVALIDATE_NOCHILDREN);
    return;
}

//------------------------------------------------------------------------
void OQueryTableView::GetConnection(OQueryTableConnection* pConn)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    // bei mir und dem Dokument einfuegen
    GetTabConnList()->push_back(pConn);
    m_pView->getController()->getTableConnectionData()->push_back(pConn->GetData());

    // invalidieren (damit es neu gezeichnet wird)
    pConn->Invalidate();

    // modified-Flag
    m_pView->getController()->setModified(sal_True);
}

//------------------------------------------------------------------------
void OQueryTableView::DropConnection(OQueryTableConnection* pConn)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    // Selektion beachten
    DeselectConn(GetSelectedConn());

    // bei mir und dem Dokument rausnehmen
    GetTabConnList()->erase( ::std::find(GetTabConnList()->begin(),GetTabConnList()->end(),static_cast<OTableConnection*>(pConn)));
    m_pView->getController()->getTableConnectionData()->erase( ::std::find(m_pView->getController()->getTableConnectionData()->begin(),m_pView->getController()->getTableConnectionData()->end(),pConn->GetData()));

    // invalidieren (damit es neu gezeichnet wird)
    pConn->Invalidate();

    // modified-Flag
    m_pView->getController()->setModified(sal_True);
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
        for(;aIter != GetTabWinMap()->end();++aIter)
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
        ::std::vector<OTableConnection*>* pTabConList = GetTabConnList();
        ::std::vector<OTableConnection*>::iterator aIter2 = pTabConList->begin();
        for(;aIter2 != pTabConList->end();)
        {
            OQueryTableConnection* pTmpEntry = static_cast<OQueryTableConnection*>(*aIter2);
            OSL_ENSURE(pTmpEntry,"OQueryTableConnection is null!");
            if( pTmpEntry->GetAliasName(JTCS_FROM) == pTabWin->GetAliasName() ||
                pTmpEntry->GetAliasName(JTCS_TO) == pTabWin->GetAliasName() )
            {
                pUndoAction->InsertConnection(pTmpEntry);
                // die Connection invalidieren (ich kann nicht unten einfach ein InvalidateConnections machen, da ja dann die Connection
                // in meiner Liste nicht mehr existiert !)
                pTmpEntry->Invalidate();

                // die Daten aus dem Doc entfernen (Kommentar wie oben bei den Tabellendaten)
                pTabConnDataList->erase( ::std::find(pTabConnDataList->begin(),pTabConnDataList->end(),pTmpEntry->GetData()) );
                // die Connection selber weg und weiter
                aIter2 = pTabConList->erase(aIter2); // TODO check if this is ok
                nCnt++;
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
        m_pView->getController()->InvalidateFeature( ID_BROWSER_SAVEDOC );
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
            (*GetTabWinMap())[static_cast< OQueryTableWindowData*>(pData)->GetAliasName()] = pTabWin;
            pTabWin->Show();

            pTabWin->Update();
                // Das Update ist notwendig, damit die Connections an dem Fenster richtig gezeichnet werden. Klingt absurd,
                // ich weiss. Aber die Listbox haelt sich intern ein Member, was bei ersten Zeichnen (nachdem die Listbox im Init
                // gerade neu gefuellt wurde) initialisiert wird, und genau dieses Member wird irgendwann benoetigt fuer
                // GetEntryPos, und dieses wiederum von der Connection, wenn sie ihren Ansatzpunkt am Fenster feststellen will.

            // die Connections
            sal_Int16 nCount(0);
            ::std::vector<OTableConnection*>* pTableCon = pUndoAction->GetTabConnList();
            ::std::vector<OTableConnection*>::iterator aIter = pTableCon->begin();

            for(;aIter != pTableCon->end();++aIter,++nCount)
            {
                // und in meine Liste rein
                GetTabConnList()->push_back(*aIter);
                // die Daten wieder in das Doc einfuegen
                m_pView->getController()->getTableConnectionData()->push_back((*aIter)->GetData());
            }
            pTableCon->clear();

            if (nCount)
                InvalidateConnections();

            // und die Daten des Fensters ebenfalls in Liste (des Docs)
            if(_bAppend)
                m_pView->getController()->getTableWindowData()->push_back(pTabWin->GetData());

            m_pView->getController()->InvalidateFeature(ID_BROWSER_ADDTABLE);

            // und der UndoAction sagen, dass das Fenster jetzt meine ist ...
            pUndoAction->SetOwnership(sal_False);

            bSuccess = sal_True;
        }
        else
            //////////////////////////////////////////////////////////////////
            // Initialisierung fehlgeschlagen
            // (z.B. wenn Verbindung zur Datenbank in diesem Augenblick unterbrochen worden ist)
            delete pTabWin;
    }

    // damit habe ich das Doc natuerlich modifiziert
    if(!m_pView->getController()->isReadOnly())
        m_pView->getController()->setModified( sal_True );
    m_pView->getController()->InvalidateFeature( ID_BROWSER_SAVEDOC );
    m_pView->getController()->InvalidateFeature(ID_BROWSER_CLEAR_QUERY);

    return bSuccess;
}


//------------------------------------------------------------------------
void OQueryTableView::InsertField(const OTableFieldDesc& rInfo)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    DBG_ASSERT(getDesignView() != NULL, "OQueryTableView::InsertField : habe kein Parent !");
    static_cast<OQueryDesignView*>(getDesignView())->InsertField(rInfo);
}

//------------------------------------------------------------------------------
sal_Bool OQueryTableView::Drop(const DropEvent& rEvt)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    sal_Bool bDrop = sal_False;
/*
    SbaDatabaseRef xDatabase(GetDatabase());
    ::com::sun::star::uno::Reference< ::com::sun::star::data::XDatabaseDescriptor >  rMetaData = xDatabase->GetMetaData();
    ::com::sun::star::uno::Any aJoin = rMetaData->getInfo(::com::sun::star::data::DatabaseInfo::OUTER_JOIN_SUPPORT);

    if (!xDatabase.Is() || !((::utl::getINT16(aJoin) & ::com::sun::star::data::DatabaseOuterJoinSupport::PARTIAL) || (::utl::getINT16(aJoin) & ::com::sun::star::data::DatabaseOuterJoinSupport::YES)) && GetTabWinMap()->size())
    {
        Sound::Beep();
        return sal_False;
    }

    sal_Bool bDrop = sal_False;

    for (sal_uInt16 i = 0; i < DragServer::GetItemCount(); ++i)
    {
        if (INetBookmark::DragServerHasFormat(i) )
        {
            INetBookmark aBmk;
            if (aBmk.PasteDragServer(i))
            {
                INetURLObject aObj(aBmk.GetURL());
                aObj.SetSmartProtocol(INET_PROT_FILE);
                String aMark(aObj.GetMark());

                if (aMark.GetTokenCount(';') > 1)
                {
                    String sType = aMark.GetToken(0, ';');
                    sType += ';';
                    if (sType == String::CreateFromAscii(SDB_TABLEMARK_HEADER))
                    {
                        aMark.Erase(0, strlen(SDB_TABLEMARK_HEADER));

                        String aDatabaseName = aObj.PathToFileName();
                        DirEntry aDBEntry(aDatabaseName);

                        if (aDBEntry == DirEntry(xDatabase->Name()))
                        {
                            SbaDBDefRef aDef = xDatabase->OpenDBDef(dbTable, aMark);
                            if (aDef.Is())
                            {
                                if (aDef->Status().IsError())
                                    SBA_MOD()->ShowDbStatus(aDef->Status(), dbReadError,NULL);

                                SdbTable* pTable = (static_cast< SbaTableDef*>(&aDef)->GetTable();
                                if (pTable && pTable->IsOpen())
                                {
                                    AddTabWin(pTable->QualifierName(),pTable->GetFullName());
                                    bDrop = sal_True;
                                }
                            }
                        }
                    }
                }

                if (!bDrop)
                    Sound::Beep();
                break;
            }
        }
    }
*/
    return bDrop;
}

//------------------------------------------------------------------------------
sal_Bool OQueryTableView::QueryDrop(DropEvent& rEvt)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    sal_Bool bDrop = sal_False;
/*
    SbaDatabaseRef xDatabase(GetDatabase());

    ::com::sun::star::uno::Reference< ::com::sun::star::data::XDatabaseDescriptor >  rMetaData = xDatabase->GetMetaData();
    ::com::sun::star::uno::Any aJoin = rMetaData->getInfo(::com::sun::star::data::DatabaseInfo::OUTER_JOIN_SUPPORT);
    if (!xDatabase.Is() || !((::utl::getINT16(aJoin) & ::com::sun::star::data::DatabaseOuterJoinSupport::PARTIAL) || (::utl::getINT16(aJoin) & ::com::sun::star::data::DatabaseOuterJoinSupport::YES)) && GetTabWinMap()->size())
        return sal_False;

    sal_Bool bDrop = sal_False;
    DropAction eAction = rEvt.GetAction();

    for (sal_uInt16 i = 0; i < DragServer::GetItemCount(); ++i)
    {
        if (INetBookmark::DragServerHasFormat(i))
        {
            INetBookmark aBmk;
            if (aBmk.PasteDragServer(i))
            {   // it's a INetBookmark
                INetURLObject aObj(aBmk.GetURL());
                aObj.SetSmartProtocol(INET_PROT_FILE);
                String aMark(aObj.GetMark());

                // maybe it's a reference to a db object
                if (aMark.GetTokenCount(';') > 1)
                {
                    String sType = aMark.GetToken(0, ';');
                    sType += ';';
                    if (sType == String::CreateFromAscii(SDB_TABLEMARK_HEADER))
                    {   // it's a reference to a table
                        aMark.Erase(0, strlen(SDB_TABLEMARK_HEADER));

                        String aDatabaseName = aObj.PathToFileName();
                        DirEntry aDBEntry(aDatabaseName);

                        if (aDBEntry == DirEntry(xDatabase->Name()))
                        {   // it's a reference to a table of our own database
                            if (xDatabase->HasObjectByURL(aObj.GetMainURL()))
                            {
                                rEvt.SetAction(DROP_COPY);
                                bDrop = sal_True;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
*/
    return bDrop;
}
//------------------------------------------------------------------------------
sal_Bool OQueryTableView::ExistsAVisitedConn(const OQueryTableWindow* pFrom) const
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    ::std::vector<OTableConnection*>* pList = const_cast< OQueryTableView*>(this)->GetTabConnList();
    if (pList)
    {
        ::std::vector<OTableConnection*>::iterator aIter = pList->begin();
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




