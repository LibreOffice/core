/*************************************************************************
 *
 *  $RCSfile: sqliterator.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:28 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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

#ifndef _CONNECTIVITY_PARSE_SQLITERATOR_HXX_
#include "connectivity/sqliterator.hxx"
#endif

#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include <connectivity/sqlparse.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifdef SQL_TEST_PARSETREEITERATOR
#include <iostream.h>
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/PColumn.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include "connectivity/sdbcx/VCollection.hxx"
#endif
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif

using namespace connectivity;
using namespace connectivity::parse;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;

DBG_NAME(OSQLParseTreeIterator);
static String aEmptyString;

class OPrivateColumns : public sdbcx::OCollection
{
    OSQLColumns m_aColumns;
protected:
    virtual Reference< XNamed > createObject(const ::rtl::OUString& _rName);
    virtual void impl_refresh() throw(RuntimeException) {}
    virtual Reference< XPropertySet > createEmptyObject()
    {
        return NULL;
    }
public:
    OPrivateColumns(const OSQLColumns& _rColumns,
                    ::cppu::OWeakObject& _rParent,
                    ::osl::Mutex& _rMutex,
                    const ::std::vector< ::rtl::OUString> &_rVector
                    ) : sdbcx::OCollection(_rParent,sal_True,_rMutex,_rVector)
                    ,m_aColumns(_rColumns)
    {}
    ~OPrivateColumns()
    {
        disposing();
    }
};
// -------------------------------------------------------------------------
Reference< XNamed > OPrivateColumns::createObject(const ::rtl::OUString& _rName)
{
    return Reference< XNamed >(*find(m_aColumns.begin(),m_aColumns.end(),_rName,isCaseSensitive()),UNO_QUERY);
}
// -------------------------------------------------------------------------
class OPrivateTables : public sdbcx::OCollection
{
    OSQLTables  m_aTables;
protected:
    virtual Reference< XNamed > createObject(const ::rtl::OUString& _rName);
    virtual void impl_refresh() throw(RuntimeException) {}
    virtual Reference< XPropertySet > createEmptyObject()
    {
        return NULL;
    }
public:
    OPrivateTables(const OSQLTables& _rColumns,
                    ::cppu::OWeakObject& _rParent,
                    ::osl::Mutex& _rMutex,
                    const ::std::vector< ::rtl::OUString> &_rVector
                    ) : sdbcx::OCollection(_rParent,sal_True,_rMutex,_rVector)
                    ,m_aTables(_rColumns)
    {}
};
// -------------------------------------------------------------------------
Reference< XNamed > OPrivateTables::createObject(const ::rtl::OUString& _rName)
{
    return Reference< XNamed >(m_aTables.find(_rName)->second,UNO_QUERY);
}

//-----------------------------------------------------------------------------
OSQLParseTreeIterator::OSQLParseTreeIterator()
    : m_pParseTree(NULL)
    , m_xTables(NULL)
    , m_xDatabaseMetaData(NULL)
{
    DBG_CTOR(OSQLParseTreeIterator,NULL);
    m_aSelectColumns = new OSQLColumns();
}
//-----------------------------------------------------------------------------
OSQLParseTreeIterator::OSQLParseTreeIterator(const Reference< XNameAccess>& _xTables ,
                                             const Reference< XDatabaseMetaData>& _xDatabaseMetaData,
                                             const OSQLParseNode* pRoot)
    : m_xTables(_xTables)
    , m_xDatabaseMetaData(_xDatabaseMetaData)
    , m_aTables(_xDatabaseMetaData->storesMixedCaseQuotedIdentifiers())
    , m_aCaseEqual(_xDatabaseMetaData->storesMixedCaseQuotedIdentifiers())
{
    DBG_CTOR(OSQLParseTreeIterator,NULL);
    m_aSelectColumns = new OSQLColumns();// must be done because we need an empty column at zero
    setParseTree(pRoot);
}
//-----------------------------------------------------------------------------
OSQLParseTreeIterator::OSQLParseTreeIterator(const OSQLParseTreeIterator & rIter)
    :m_xTables(NULL)
    ,m_pParseTree(NULL)
    , m_xDatabaseMetaData(NULL)
{
    DBG_CTOR(OSQLParseTreeIterator,NULL);

    DBG_ERROR("OSQLParseTreeIterator: Copy-Konstruktor nicht implementiert!");
}

//-----------------------------------------------------------------------------
OSQLParseTreeIterator::~OSQLParseTreeIterator()
{
    DBG_DTOR(OSQLParseTreeIterator,NULL);
    m_aTables.clear();
}
// -------------------------------------------------------------------------
Reference< XNameAccess > OSQLParseTreeIterator::getSelectAsNameAccess(::cppu::OWeakObject& _rParent,::osl::Mutex& _rMutex) const
{
    ::std::vector< ::rtl::OUString> aNames;
    for(OSQLColumns::const_iterator aIter = m_aSelectColumns->begin(); aIter != m_aSelectColumns->end();++aIter)
        aNames.push_back(getString((*aIter)->getFastPropertyValue(PROPERTY_ID_NAME)));
    OPrivateColumns* pCols = new OPrivateColumns(*m_aSelectColumns,_rParent,_rMutex,aNames);
    return pCols;
}
// -------------------------------------------------------------------------
Reference< XNameAccess > OSQLParseTreeIterator::getTablesAsNameAccess(::cppu::OWeakObject& _rParent,::osl::Mutex& _rMutex) const
{
    ::std::vector< ::rtl::OUString> aNames;
    for(OSQLTables::const_iterator aIter = m_aTables.begin(); aIter != m_aTables.end();++aIter)
    {
        Reference<XNamed> xName(aIter->second,UNO_QUERY);
        aNames.push_back(xName->getName());
    }
    OPrivateTables* pTabs = new OPrivateTables(m_aTables,_rParent,_rMutex,aNames);
    return pTabs;
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setParseTree(const OSQLParseNode * pNewParseTree)
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);
    // alle Eingaben zur"uck setzen
    //  aIteratorStatus.Clear();
//  for(ULONG i=0; i< m_aTables.count();i++)
//      delete m_aTables.getObject(i);
    m_aTables.clear();

    m_aSelectColumns->clear();

    m_pParseTree = pNewParseTree;
    if (!m_pParseTree)
    {
        m_eStatementType = SQL_STATEMENT_UNKNOWN;
        return;
    }

    // falls m_pParseTree aber keine Connection, dann Fehler
    if(!m_xTables.is())
    {
        //aIteratorStatus.setInvalidStatement();
        CallError(RET_BREAK);
        return;
    }

    //  m_aTables.setCaseSensitive(TablesAreSensitive());


    // Statement-Typ ermitteln ...
    if (SQL_ISRULE(m_pParseTree,select_statement) || SQL_ISRULE(m_pParseTree,union_statement) )
    {
        m_eStatementType = SQL_STATEMENT_SELECT;
    }
    else if (SQL_ISRULE(m_pParseTree,insert_statement))
    {
        m_eStatementType = SQL_STATEMENT_INSERT;
    }
    else if (SQL_ISRULE(m_pParseTree,update_statement_positioned))
    {
        m_eStatementType = SQL_STATEMENT_UPDATE;
    }
    else if (SQL_ISRULE(m_pParseTree,update_statement_searched))
    {
        m_eStatementType = SQL_STATEMENT_UPDATE;
    }
    else if (SQL_ISRULE(m_pParseTree,delete_statement_positioned))
    {
        m_eStatementType = SQL_STATEMENT_DELETE;
    }
    else if (SQL_ISRULE(m_pParseTree,delete_statement_searched))
    {
        m_eStatementType = SQL_STATEMENT_DELETE;
    }
    else if (m_pParseTree->count() == 3 && SQL_ISRULE(m_pParseTree->getChild(1),odbc_call_spec))
    {
        m_eStatementType = SQL_STATEMENT_ODBC_CALL;
    }
    else
    {
        m_eStatementType = SQL_STATEMENT_UNKNOWN;
        //aIteratorStatus.setInvalidStatement();
        CallError(RET_BREAK);
        return;
    }
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseOneTableName(const OSQLParseNode * pTableName, const String & rTableRange)
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);

    DBG_ASSERT(pTableName != NULL,"OSQLParseTreeIterator::traverseOneTableName: pTableName == NULL");

    String aTableName;
    String aTableRange(rTableRange);

    // Tabellenname abholen
    pTableName->parseNodeToStr(aTableName,m_xDatabaseMetaData,NULL,FALSE,FALSE);
    // Wenn keine Range Variable angegeben, dann den Tabellennamen verwenden.
    if (!aTableRange.Len())
        aTableRange = aTableName;

    if(aTableName.Len())
    {
        try
        {
            if(!m_xTables->hasByName(aTableName)) // name not in XNameAccess
            {
                const ::rtl::OUString sAll = ::rtl::OUString::createFromAscii("%");
                Sequence< ::rtl::OUString > aSeq;
                Reference< XResultSet> xRes = m_xDatabaseMetaData->getTables(Any(),sAll,aTableName,aSeq);
                if(xRes.is() && xRes->next())
                {
                    ::rtl::OUString sCatalog, sSchema, sName;
                    Reference< XRow > xCurrentRow(xRes, UNO_QUERY);
                    sCatalog = xCurrentRow->getString(1);
                    sSchema = xCurrentRow->getString(2);
                    sName = xCurrentRow->getString(3);

                    if(sCatalog.getLength())
                    {
                        aTableName = sCatalog;
                        aTableName += m_xDatabaseMetaData->getCatalogSeparator().getStr();
                    }
                    if(sSchema.getLength())
                    {
                        aTableName += sSchema.getStr();
                        aTableName += ::rtl::OUString::createFromAscii(".").getStr();
                    }
                    aTableName += sName.getStr();
                }
            }
            Any aTable(m_xTables->getByName(aTableName));
            OSQLTable xSet;

            if(aTable >>= xSet)
                m_aTables[aTableRange] = xSet;
        }catch(...)
        {
            if (CallError(RET_CONTINUE))
                m_aTables[aTableRange] = NULL;  // ich soll den Fehler ignorieren
        }

    }
    else
        CallError(RET_BREAK,rTableRange);
}
//-----------------------------------------------------------------------------
OSQLParseNode * OSQLParseTreeIterator::getQualified_join(OSQLParseNode *pTableRef,String& aTableRange)
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);
    aTableRange.Erase();

    OSQLParseNode *pNode = getTableRef(pTableRef->getChild(0),aTableRange);
    if(pNode)
        traverseOneTableName(pNode,aTableRange);
    ULONG nPos = 4;
    if(SQL_ISRULE(pTableRef,cross_union) || pTableRef->getChild(1)->getTokenID() != SQL_TOKEN_NATURAL)
        nPos = 3;


    pNode = getTableRef(pTableRef->getChild(nPos),aTableRange);
    if(pNode)
        traverseOneTableName(pNode,aTableRange);
    return pNode;
}
//-----------------------------------------------------------------------------
OSQLParseNode * OSQLParseTreeIterator::getTableRef(OSQLParseNode *pTableRef,String& aTableRange)
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);
    OSQLParseNode *pTableName = pTableRef;
    if (pTableName->count() == 4 )
    {
        if(SQL_ISPUNCTUATION(pTableName->getChild(0),"{"))
        { // { OJ joined_table }
            getQualified_join(pTableName->getChild(2),aTableRange);
            pTableName = NULL;
        }
        else
        {
            // Tabellennamen gefunden
            if(!SQL_ISRULE(pTableName,table_name))
                pTableName = pTableRef->getChild(0);
            aTableRange.Erase();
            if(pTableRef->count() == 4)
                aTableRange = pTableRef->getChild(2)->getTokenValue(); // Tabellenrange an Pos 2
        }
    }
    else if(SQL_ISRULE(pTableName,qualified_join) || SQL_ISRULE(pTableName,cross_union))
    {
        getQualified_join(pTableRef,aTableRange);
        pTableName = NULL;
    }
    else if (SQL_ISRULE(pTableName,joined_table))
    {
        // '(' qualified_join ')'
        getQualified_join(pTableName->getChild(1),aTableRange);
        pTableName = NULL;
    }
    else if(pTableRef->count() == 6)
    {
        // '(' joined_table ')' as range_variable op_column_commalist
        if(SQL_ISRULE(pTableRef->getChild(1),qualified_join) || SQL_ISRULE(pTableRef->getChild(1),cross_union))
            getQualified_join(pTableRef->getChild(1),aTableRange);
        else if(SQL_ISRULE(pTableRef->getChild(1),select_statement)) // Unterabfrage
            getSelect_statement(pTableRef->getChild(1));
        else if(pTableRef->getChild(1)->count() == 4)
            // pTableRef->getChild(1) ->> non_join_query_exp
            getSelect_statement(pTableRef->getChild(0)); // query_exp SQL_TOKEN_UNION all query_term
        else
        {// nyi: tiefere Verschachtelung m"oglch
            CallError(4);
        }
    }
    return pTableName;
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::getSelect_statement(OSQLParseNode *pSelect)
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);
    if(SQL_ISRULE(pSelect,union_statement))
    {
        getSelect_statement(pSelect->getChild(0));
        //getSelect_statement(pSelect->getChild(3));
        return;
    }
    OSQLParseNode * pTableRefCommalist = pSelect->getChild(3)->getChild(0)->getChild(1);

    DBG_ASSERT(pTableRefCommalist != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
    DBG_ASSERT(SQL_ISRULE(pTableRefCommalist,table_ref_commalist),"OSQLParseTreeIterator: Fehler im Parse Tree");

    OSQLParseNode * pTableName = NULL;
    String aTableRange;
    for (ULONG i = 0; i < pTableRefCommalist->count(); i++)
    { // from clause durchlaufen
        aTableRange.Erase();

        if (SQL_ISRULE(pTableRefCommalist->getChild(i),table_name))
        {
            pTableName = pTableRefCommalist->getChild(i);
            traverseOneTableName(pTableName,aTableRange);// Keine Range Variable
        }
        else if (SQL_ISRULE(pTableRefCommalist->getChild(i),table_ref))
        {
            // Tabellenreferenz kann aus Tabellennamen, Tabellennamen (+),'('joined_table')'(+) bestehen
            pTableName = pTableRefCommalist->getChild(i)->getChild(0);
            if (SQL_ISRULE(pTableName,table_name))
            {   // Tabellennamen gefunden
                if(pTableRefCommalist->getChild(i)->count() == 4)   // Tabellenrange an Pos 2
                    aTableRange = pTableRefCommalist->getChild(i)->getChild(2)->getTokenValue();
                traverseOneTableName(pTableName,aTableRange);
            }
            else if(SQL_ISPUNCTUATION(pTableName,"{"))
                getQualified_join(pTableRefCommalist->getChild(i)->getChild(2),aTableRange);
            else // '(' joined_table ')' as range_variable op_column_commalist
                getTableRef(pTableRefCommalist->getChild(i),aTableRange);
        }
        else if (SQL_ISRULE(pTableRefCommalist->getChild(i),qualified_join) || SQL_ISRULE(pTableRefCommalist->getChild(i),cross_union)      )
        {
            // qualified_join oder cross_union vorhanden
            getQualified_join(pTableRefCommalist->getChild(i),aTableRange);
        }
        else if (SQL_ISRULE(pTableRefCommalist->getChild(i),joined_table))
        {
            // '(' qualified_join ')'
            getQualified_join(pTableRefCommalist->getChild(i)->getChild(1),aTableRange);
        }
        else
        {
            CallError(3,aTableRange);
        }

        //  if (! aIteratorStatus.IsSuccessful()) break;
    }
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseTableNames()
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);
    //  aIteratorStatus.Clear();

    if (m_pParseTree == NULL) {
        //aIteratorStatus.setInvalidStatement();
        CallError(3);
        return;
    }

    OSQLParseNode * pTableName = NULL;
    String aTableRange;

    // pTableName auf den (einzigen) Tabellennamen im Statement setzen.
    // Je nach Statement (SELECT, INSERT, UPDATE, DELETE) an anderer Position.
    // Bei Select koennen mehrere Tabellennamen auftreten, bei den uebrigen Statements
    // ist ohnehin immer nur einer moeglich.

    if (m_eStatementType == SQL_STATEMENT_SELECT)
    {
        OSQLParseNode *pTmp = (OSQLParseNode *)m_pParseTree;
        getSelect_statement(pTmp);
    }
    else if (m_eStatementType == SQL_STATEMENT_INSERT)
    {
        pTableName = m_pParseTree->getChild(2);
        traverseOneTableName(pTableName,aTableRange);
    }
    else if (m_eStatementType == SQL_STATEMENT_UPDATE)
    {
        pTableName = m_pParseTree->getChild(1);
        traverseOneTableName(pTableName,aTableRange);
    }
    else if (m_eStatementType == SQL_STATEMENT_DELETE)
    {
        pTableName = m_pParseTree->getChild(2);
        traverseOneTableName(pTableName,aTableRange);
    } else
    {
        CallError(5,aTableRange);
    }
}
//-----------------------------------------------------------------------------
String OSQLParseTreeIterator::getColumnAlias(const OSQLParseNode* pDerivedColumn) const
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);
    String aColumnAlias;
    if(pDerivedColumn->getChild(1)->count() == 2)
        aColumnAlias = pDerivedColumn->getChild(1)->getChild(1)->getTokenValue();
    else if(!pDerivedColumn->getChild(1)->isRule())
        aColumnAlias = pDerivedColumn->getChild(1)->getTokenValue();
    return aColumnAlias;
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::getColumnRange(const OSQLParseNode* pColumnRef,String &rColumnName,String &rTableRange) const
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);
    rColumnName.Erase();
    rTableRange.Erase();
    if(SQL_ISRULE(pColumnRef,column_ref))// ab hier ist es sicher eine Columnref
    {
        if(pColumnRef->count() > 1)
        {
            for(INT32 i=0;i<((INT32)pColumnRef->count())-2;i++)  // mu"s signed sein, falls count == 1
                pColumnRef->getChild(i)->parseNodeToStr(rTableRange,m_xDatabaseMetaData,NULL,FALSE,FALSE);
            // Spaltenname
            rColumnName = pColumnRef->getChild(pColumnRef->count()-1)->getChild(0)->getTokenValue();
        }
        else
            rColumnName = pColumnRef->getChild(0)->getTokenValue();
    }
    else if(SQL_ISRULE(pColumnRef,general_set_fct) || SQL_ISRULE(pColumnRef,set_fct_spec))
    { // Funktion
        pColumnRef->parseNodeToStr(rColumnName,m_xDatabaseMetaData);
    }
    else  if(pColumnRef->getNodeType() == SQL_NODE_NAME)
        rColumnName = pColumnRef->getTokenValue();
}

//-----------------------------------------------------------------------------
BOOL OSQLParseTreeIterator::getColumnTableRange(const OSQLParseNode* pNode, String &rTableRange) const
{
    // Ermitteln ob alle Spalten zu einer Tabelle gehoeren
    if (SQL_ISRULE(pNode,column_ref))
    {
        String aColName, aTableRange;
        getColumnRange(pNode, aColName, aTableRange);
        if (!aTableRange.Len()) // keinen gefunden
        {
            // dann die Spalte in den Tabellen suchen
            for (ConstOSQLTablesIterator aIter = m_aTables.begin(); aIter != m_aTables.end(); ++aIter)
            {
                if (aIter->second.is())
                {
                    try
                    {
                        Reference< XNameAccess > xColumns = aIter->second->getColumns();
                        Any aColumn(xColumns->getByName(aColName));
                        Reference< XFastPropertySet > xColumn;

                        if (aColumn >>= xColumn)
                        {
                            aTableRange = aIter->first;
                            break;
                        }
                    }
                    catch(...)
                    {
                    }
                }
            }
            if (!aTableRange.Len())
                return FALSE;
        }


        if (!rTableRange.Len())
            rTableRange = aTableRange;
        else if (rTableRange != aTableRange)
            return FALSE;
    }
    else
    {
        for (UINT32 i = 0, ncount = pNode->count(); i < ncount; i++)
        {
            if (!getColumnTableRange(pNode->getChild(i), rTableRange))
                return FALSE;
        }
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseSelectColumnNames(const OSQLParseNode* pSelectNode)
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);
    //  aIteratorStatus.Clear();

    if (!pSelectNode || m_eStatementType != SQL_STATEMENT_SELECT || !m_aTables.size())
    {
        //aIteratorStatus.setInvalidStatement();
        CallError(3);
        return;
    }

    if(SQL_ISRULE(pSelectNode,union_statement))
    {
        traverseSelectColumnNames(pSelectNode->getChild(0));
//      traverseSelectColumnNames(pSelectNode->getChild(3));
        return;
    }

    // nyi: mehr Pruefung auf korrekte Struktur!
    if (pSelectNode->getChild(2)->isRule() && SQL_ISPUNCTUATION(pSelectNode->getChild(2)->getChild(0),"*"))
    {
        // SELECT * ...
        setSelectColumnName(String::CreateFromAscii("*"), aEmptyString,aEmptyString);
    }
    else if (SQL_ISRULE(pSelectNode->getChild(2),scalar_exp_commalist))
    {
        // SELECT column(,column) oder SELECT COUNT(*) ...
        OSQLParseNode * pSelection = pSelectNode->getChild(2);

        for (ULONG i = 0; i < pSelection->count(); i++)
        {
            OSQLParseNode *pColumnRef = pSelection->getChild(i);

            if (SQL_ISRULE(pColumnRef,select_sublist))
            {
                // alle Spalten der Tabelle
                String aTableRange;
                pColumnRef->getChild(0)->parseNodeToStr(aTableRange,m_xDatabaseMetaData,NULL,FALSE,FALSE);
                setSelectColumnName(String::CreateFromAscii("*"), aEmptyString,aTableRange);
                continue;
            }else if (SQL_ISRULE(pColumnRef,derived_column))
            {
                String aColumnAlias(getColumnAlias(pColumnRef)); // kann leer sein
                String aColumnName;
                String aTableRange;
                BOOL bFkt(FALSE);
                pColumnRef = pColumnRef->getChild(0);
                if (SQL_ISRULE(pColumnRef,column_ref))
                {
                    getColumnRange(pColumnRef,aColumnName,aTableRange);
                    DBG_ASSERT(aColumnName.Len(),"Columnname darf nicht leer sein");
                }
                else /*if (SQL_ISRULE(pColumnRef,general_set_fct) || SQL_ISRULE(pColumnRef,set_fct_spec)    ||
                         SQL_ISRULE(pColumnRef,position_exp)    || SQL_ISRULE(pColumnRef,extract_exp)   ||
                         SQL_ISRULE(pColumnRef,length_exp)      || SQL_ISRULE(pColumnRef,char_value_fct)||
                         SQL_ISRULE(pColumnRef,num_value_exp)   || SQL_ISRULE(pColumnRef,term))*/
                {
                    /* Funktionsaufruf vorhanden */
                    pColumnRef->parseNodeToStr(aColumnName,m_xDatabaseMetaData,NULL,FALSE,TRUE);

                    // gehoeren alle beteiligten Spalten der Funktion zu einer Tabelle
                    if (m_aTables.size() == 1)
                    {
                        aTableRange = m_aTables.begin()->first;
                    }
                    else
                    {
                        getColumnTableRange(pColumnRef,aTableRange);
                    }
                    bFkt = TRUE;
                }
                /*
                else
                {
                    aIteratorStatus.setStatementTooComplex();
                    return;
                }
                */
                if(!aColumnAlias.Len())
                    aColumnAlias = aColumnName;
                setSelectColumnName(aColumnName,aColumnAlias,aTableRange,bFkt);
            }
        }

    } else
    {
        //aIteratorStatus.setInvalidStatement();
        CallError(3);
    }
}


//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseOrderByColumnNames(const OSQLParseNode* pSelectNode)
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);
    //  aIteratorStatus.Clear();

    if (pSelectNode == NULL)
    {
        //aIteratorStatus.setInvalidStatement();
        CallError(3);
        return;
    }

    if (m_eStatementType != SQL_STATEMENT_SELECT)
    {
        CallError(3);
        //aIteratorStatus.setInvalidStatement();
        return;
    }

    if(SQL_ISRULE(pSelectNode,union_statement))
    {
        traverseOrderByColumnNames(pSelectNode->getChild(0));
        //traverseOrderByColumnNames(pSelectNode->getChild(3));
        return;
    }

    DBG_ASSERT(pSelectNode->count() >= 4,"OSQLParseTreeIterator: Fehler im Parse Tree");

    OSQLParseNode * pTableExp = pSelectNode->getChild(3);
    DBG_ASSERT(pTableExp != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
    DBG_ASSERT(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator: Fehler im Parse Tree");
    DBG_ASSERT(pTableExp->count() == 5,"OSQLParseTreeIterator: Fehler im Parse Tree");

    OSQLParseNode * pOptOrderByClause = pTableExp->getChild(4);
    DBG_ASSERT(pOptOrderByClause != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
    DBG_ASSERT(SQL_ISRULE(pOptOrderByClause,opt_order_by_clause),"OSQLParseTreeIterator: Fehler im Parse Tree");
    if (pOptOrderByClause->count() == 0)
        return;

    DBG_ASSERT(pOptOrderByClause->count() == 3,"OSQLParseTreeIterator: Fehler im Parse Tree");

    OSQLParseNode * pOrderingSpecCommalist = pOptOrderByClause->getChild(2);
    DBG_ASSERT(pOrderingSpecCommalist != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
    DBG_ASSERT(SQL_ISRULE(pOrderingSpecCommalist,ordering_spec_commalist),"OSQLParseTreeIterator: Fehler im Parse Tree");
    DBG_ASSERT(pOrderingSpecCommalist->count() > 0,"OSQLParseTreeIterator: Fehler im Parse Tree");

    String aColumnName,aColumnAlias;
    String aTableRange;
    for (UINT32 i = 0; i < pOrderingSpecCommalist->count(); i++)
    {
        OSQLParseNode * pOrderingSpec = pOrderingSpecCommalist->getChild(i);
        DBG_ASSERT(pOrderingSpec != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
        DBG_ASSERT(SQL_ISRULE(pOrderingSpec,ordering_spec),"OSQLParseTreeIterator: Fehler im Parse Tree");
        DBG_ASSERT(pOrderingSpec->count() == 2,"OSQLParseTreeIterator: Fehler im Parse Tree");

        OSQLParseNode * pColumnRef = pOrderingSpec->getChild(0);
        aTableRange.Erase();
        aColumnName.Erase();
        if(SQL_ISRULE(pColumnRef,column_ref))
        {
            // Column-Name (und TableRange):
            if(SQL_ISRULE(pColumnRef,column_ref))
                getColumnRange(pColumnRef,aColumnName,aTableRange);
            else // eine Expression
                pColumnRef->parseNodeToStr(aColumnName,m_xDatabaseMetaData,NULL,FALSE,FALSE);

            DBG_ASSERT(aColumnName.Len(),"aColumnName darf nicht leer sein");
        }
        else
        {   // here I found a predicate
            pColumnRef->parseNodeToStr(aColumnName,m_xDatabaseMetaData,NULL,FALSE,FALSE);
        }
        DBG_ASSERT(pColumnRef != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
        // Ascending/Descending
        OSQLParseNode * pOptAscDesc = pOrderingSpec->getChild(1);
        DBG_ASSERT(pOptAscDesc != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");

        BOOL bAscending = TRUE;
        if(pOptAscDesc)
        {
            if (SQL_ISTOKEN(pOptAscDesc,ASC))
                bAscending = TRUE;
            else if (SQL_ISTOKEN(pOptAscDesc,DESC))
                bAscending = FALSE;
        }

        setOrderByColumnName(aColumnName, aTableRange,bAscending);
//      if (! aIteratorStatus.IsSuccessful())
//          return;

    }
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseSelectionCriteria(const OSQLParseNode* pSelectNode)
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);
    //  aIteratorStatus.Clear();

    if (pSelectNode == NULL)
    {
        CallError(3);
        //aIteratorStatus.setInvalidStatement();
        return;
    }


    // Parse Tree analysieren (je nach Statement-Typ)
    // und Zeiger auf WHERE-Klausel setzen:
    OSQLParseNode * pWhereClause = NULL;

    if (m_eStatementType == SQL_STATEMENT_SELECT)
    {
        if(SQL_ISRULE(pSelectNode,union_statement))
        {
            traverseSelectionCriteria(pSelectNode->getChild(0));
            traverseSelectionCriteria(pSelectNode->getChild(3));
            return;
        }
        DBG_ASSERT(pSelectNode->count() >= 4,"OSQLParseTreeIterator: Fehler im Parse Tree");

        OSQLParseNode * pTableExp = pSelectNode->getChild(3);
        DBG_ASSERT(pTableExp != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
        DBG_ASSERT(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator: Fehler im Parse Tree");
        DBG_ASSERT(pTableExp->count() == 5,"OSQLParseTreeIterator: Fehler im Parse Tree");

        pWhereClause = pTableExp->getChild(1);
    } else if (SQL_ISRULE(pSelectNode,update_statement_searched)) {
        DBG_ASSERT(pSelectNode->count() == 5,"OSQLParseTreeIterator: Fehler im Parse Tree");
        pWhereClause = pSelectNode->getChild(4);
    } else if (SQL_ISRULE(pSelectNode,update_statement_positioned)) {
        // nyi
        DBG_ERROR("OSQLParseTreeIterator::getSelectionCriteria: positioned nyi");
    } else if (SQL_ISRULE(pSelectNode,delete_statement_searched)) {
        DBG_ASSERT(pSelectNode->count() == 4,"OSQLParseTreeIterator: Fehler im Parse Tree");
        pWhereClause = pSelectNode->getChild(3);
    } else if (SQL_ISRULE(pSelectNode,delete_statement_positioned)) {
        // nyi
        DBG_ERROR("OSQLParseTreeIterator::getSelectionCriteria: positioned nyi");
    } else {
        // Anderes Statement. Keine Selektionskriterien.
        return;
    }

    if (! SQL_ISRULE(pWhereClause,where_clause)) {
        // Die Where Clause ist meistens optional, d. h. es koennte sich auch
        // um "optional_where_clause" handeln.
        DBG_ASSERT(SQL_ISRULE(pWhereClause,opt_where_clause),"OSQLParseTreeIterator: Fehler im Parse Tree");
        return;
    }

    // Wenn es aber eine where_clause ist, dann darf sie nicht leer sein:
    DBG_ASSERT(pWhereClause->count() == 2,"OSQLParseTreeIterator: Fehler im Parse Tree");

    OSQLParseNode * pComparisonPredicate = pWhereClause->getChild(1);
    DBG_ASSERT(pComparisonPredicate != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");


    //
    // Und nun die Vergleichskriterien abarbeiten (rekursiv, alles ist erstmal ein OR-Kriterium):
    //
    setORCriteriaPre();
    //  if (! aIteratorStatus.IsSuccessful()) return;

    traverseORCriteria(pComparisonPredicate);
    //  if (! aIteratorStatus.IsSuccessful()) return;

    setORCriteriaPost();

    // Fehler wird ggf. einfach weitergereicht.
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseORCriteria(OSQLParseNode * pSearchCondition)
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);

    if (
            pSearchCondition->count() == 3 &&
            SQL_ISPUNCTUATION(pSearchCondition->getChild(0),"(") &&
            SQL_ISPUNCTUATION(pSearchCondition->getChild(2),")")
        )
    {
        // Runde Klammern um den Ausdruck
        traverseORCriteria(pSearchCondition->getChild(1));
    } else if (SQL_ISRULE(pSearchCondition,search_condition) &&
        pSearchCondition->count() == 3 &&
        SQL_ISTOKEN(pSearchCondition->getChild(1),OR))
    {
        // OR-Verknuepfung:

        for (int i = 0; i < 3; i++) {
            if (i == 1) continue;       // Schluesselwort OR ueberspringen

            // Ist das erste Element wieder eine OR-Verknuepfung?
            if (i == 0 &&
                SQL_ISRULE(pSearchCondition->getChild(0),search_condition) &&
                pSearchCondition->getChild(0)->count() == 3 &&
                SQL_ISTOKEN(pSearchCondition->getChild(0)->getChild(1),OR))
            {
                // Dann rekursiv absteigen ...
                traverseORCriteria(pSearchCondition->getChild(0));

            } else {
                // AND-Kriterien ...
                setANDCriteriaPre();
                //  if (! aIteratorStatus.IsSuccessful()) break;

                traverseANDCriteria(pSearchCondition->getChild(i));
                //  if (! aIteratorStatus.IsSuccessful()) break;

                setANDCriteriaPost();
            }

            //  if (! aIteratorStatus.IsSuccessful()) break;
        }
    } else {
        // Nur *ein* Kriterium oder eine AND-Verknuepfung von Kriterien.
        // Direkt die AND-Kriterien behandeln.
        setANDCriteriaPre();
        //  if (! aIteratorStatus.IsSuccessful()) return;

        traverseANDCriteria(pSearchCondition);
        //  if (! aIteratorStatus.IsSuccessful()) return;

        setANDCriteriaPost();
    }

    // Fehler einfach weiterreichen.
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseANDCriteria(OSQLParseNode * pSearchCondition)
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);

    if (
            SQL_ISRULE(pSearchCondition,boolean_primary) &&
            pSearchCondition->count() == 3 &&
            SQL_ISPUNCTUATION(pSearchCondition->getChild(0),"(") &&
            SQL_ISPUNCTUATION(pSearchCondition->getChild(2),")")
        )
    {
        // Runde Klammern
        traverseANDCriteria(pSearchCondition->getChild(1));
    }
    // Das erste Element ist eine OR-Verknuepfung
    else  if ( SQL_ISRULE(pSearchCondition,search_condition) && pSearchCondition->count() == 3 )
    {
        // Dann rekursiv absteigen (dieselbe Row benutzen) ...
        traverseORCriteria(pSearchCondition->getChild(0));
//      if (! aIteratorStatus.IsSuccessful())
//          return;

        // Und mit dem rechten Child weitermachen:
        traverseANDCriteria(pSearchCondition->getChild(2));
    }
    // Das erste Element ist (wieder) eine AND-Verknuepfung
    else if ( SQL_ISRULE(pSearchCondition,boolean_term) && pSearchCondition->count() == 3 )
    {
        // Dann rekursiv absteigen (dieselbe Row benutzen) ...
        traverseANDCriteria(pSearchCondition->getChild(0));
//      if (! aIteratorStatus.IsSuccessful())
//          return;

        // Und mit dem rechten Child weitermachen:
        traverseANDCriteria(pSearchCondition->getChild(2));
    }
     // Sonst einzelne Suchkriterien wie =, !=, ..., LIKE, IS NULL usw. behandeln:
    else if (SQL_ISRULE(pSearchCondition,comparison_predicate) )
    {
        OSQLPredicateType ePredicateType;
        OSQLParseNode *pPrec = pSearchCondition->getChild(1);
        if (pPrec->getNodeType() == SQL_NODE_EQUAL)
            ePredicateType = SQL_PRED_EQUAL;
        else if (pPrec->getNodeType() == SQL_NODE_NOTEQUAL)
            ePredicateType = SQL_PRED_NOTEQUAL;
        else if (pPrec->getNodeType() == SQL_NODE_LESS)
            ePredicateType = SQL_PRED_LESS;
        else if (pPrec->getNodeType() == SQL_NODE_LESSEQ)
            ePredicateType = SQL_PRED_LESSOREQUAL;
        else if (pPrec->getNodeType() == SQL_NODE_GREATEQ)
            ePredicateType = SQL_PRED_GREATEROREQUAL;
        else if (pPrec->getNodeType() == SQL_NODE_GREAT)
            ePredicateType = SQL_PRED_GREATER;

        String aValue;
        pSearchCondition->getChild(2)->parseNodeToStr(aValue,m_xDatabaseMetaData,NULL,FALSE,FALSE);
        traverseOnePredicate(pSearchCondition->getChild(0),ePredicateType,aValue,FALSE,pSearchCondition->getChild(2));
//      if (! aIteratorStatus.IsSuccessful())
//          return;
    }
    else if (SQL_ISRULE(pSearchCondition,like_predicate) /*&& SQL_ISRULE(pSearchCondition->getChild(0),column_ref)*/)
    {
        OSQLPredicateType ePredicateType;

        DBG_ASSERT(pSearchCondition->count() >= 5,"OSQLParseTreeIterator: Fehler im Parse Tree");

        OSQLParseNode * pNum_value_exp  = pSearchCondition->getChild(3);
        OSQLParseNode * pOptEscape      = pSearchCondition->getChild(4);

        if (pSearchCondition->getChild(1)->getTokenID() == SQL_TOKEN_NOT)
            ePredicateType = SQL_PRED_NOTLIKE;
        else
            ePredicateType = SQL_PRED_LIKE;

        DBG_ASSERT(pNum_value_exp != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
        DBG_ASSERT(pOptEscape != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");

        if (pOptEscape->count() != 0)
        {
            //  aIteratorStatus.setStatementTooComplex();
            return;
        }

        String aValue;
        OSQLParseNode * pParam = NULL;
        if (SQL_ISRULE(pNum_value_exp,parameter))
            pParam = pNum_value_exp;
        else if(pNum_value_exp->isToken())
            // Normaler Wert
            aValue = pNum_value_exp->getTokenValue();
        else
        {
            pNum_value_exp->parseNodeToStr(aValue,m_xDatabaseMetaData,NULL,FALSE,FALSE);
            pParam = pNum_value_exp;
        }

        traverseOnePredicate(pSearchCondition->getChild(0),ePredicateType,aValue,FALSE,pParam);
//      if (! aIteratorStatus.IsSuccessful())
//          return;
    }
    else if (SQL_ISRULE(pSearchCondition,test_for_null) /*&& SQL_ISRULE(pSearchCondition->getChild(0),column_ref)*/)
    {
        OSQLPredicateType ePredicateType;

        DBG_ASSERT(pSearchCondition->count() >= 3,"OSQLParseTreeIterator: Fehler im Parse Tree");
        DBG_ASSERT(SQL_ISTOKEN(pSearchCondition->getChild(1),IS),"OSQLParseTreeIterator: Fehler im Parse Tree")

        if (SQL_ISTOKEN(pSearchCondition->getChild(2),NOT) )
            ePredicateType = SQL_PRED_ISNOTNULL;
        else
            ePredicateType = SQL_PRED_ISNULL;

        String aString;
        traverseOnePredicate(pSearchCondition->getChild(0),ePredicateType,aString,TRUE,NULL);
        //  if (! aIteratorStatus.IsSuccessful()) return;
    } else {
        // Etwas anderes unterstuetzen wir (noch) nicht. Basta!
        //  aIteratorStatus.setStatementTooComplex();
    }
    // Fehler einfach weiterreichen.
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseOnePredicate(
                                OSQLParseNode * pColumnRef,
                                OSQLPredicateType ePredicateType,
                                String& rValue,
                                BOOL bCompareNull,
                                OSQLParseNode * pParseNode)
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);

    // Column-Name (und TableRange):
    String aColumnName;
    String aTableRange;
    getColumnRange(pColumnRef,aColumnName,aTableRange);

    String aName;
    if (pParseNode)         //event. Parameter, oder Columnref oder
    {
        if (SQL_ISRULE(pParseNode,parameter))
        {
            DBG_ASSERT(pParseNode->count() > 0,"OSQLParseTreeIterator: Fehler im Parse Tree");
            OSQLParseNode * pMark = pParseNode->getChild(0);

            String aParameterName;
            if (SQL_ISPUNCTUATION(pMark,"?"))
            {
                // Name = "?", da kein Parametername verfuegbar (z. B. bei Native SQL)
                rValue = '?';
                aName  = '?';
            }
            else if (SQL_ISPUNCTUATION(pMark,":"))
            {
                rValue = pParseNode->getChild(1)->getTokenValue();
                aName = ':';
            }
            else if (SQL_ISPUNCTUATION(pMark,"["))
            {
                rValue = pParseNode->getChild(1)->getTokenValue();
                aName = '[';
            }
            else
            {
                DBG_ERROR("OSQLParseTreeIterator: Fehler im Parse Tree");
            }
        }
        else if (SQL_ISRULE(pParseNode,column_ref))// Column-Name (und TableRange):
            getColumnRange(pParseNode,aName,rValue);
    }

    // "set"-Routine aufrufen ...
    if (bCompareNull) {
        // nyi: was soll das? Vergleich auf NULL geht nur ueber IS NULL oder IS NOT NULL!
        setPredicate(aColumnName, aTableRange,ePredicateType, String(), String());
    } else {
        setPredicate(aColumnName, aTableRange, ePredicateType, rValue, aName);
    }

    // Fehler einfach weiterreichen ...
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseAssignments()
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);

    DBG_ERROR("OSQLParseTreeIterator::traverseAssignments: noch nicht implementiert");
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseAll()
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);

    traverseTableNames();
    //  if (! aIteratorStatus.IsSuccessful()) return;

    if (m_eStatementType == SQL_STATEMENT_SELECT)
    {
        const OSQLParseNode* pSelectNode = m_pParseTree;
        traverseSelectColumnNames(pSelectNode);
//      if (! aIteratorStatus.IsSuccessful())
//          return;
        traverseOrderByColumnNames(pSelectNode);
//      if (! aIteratorStatus.IsSuccessful())
//          return;
        traverseSelectionCriteria(pSelectNode);
        //  if (! aIteratorStatus.IsSuccessful())
            //  return;
    }
    else if (m_eStatementType == SQL_STATEMENT_INSERT || m_eStatementType == SQL_STATEMENT_UPDATE)
    {
        traverseAssignments();
        //  if (! aIteratorStatus.IsSuccessful())
            //  return;
    } else if (m_eStatementType == SQL_STATEMENT_INSERT) {
        // schon alles erledigt
    } else {
        CallError(3);
        //aIteratorStatus.setInvalidStatement();
    }
}

// Dummy-Implementationen:

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setTableName(const String & rTableName, const String & rDBName, const String& rOwner,
                              const String & rTableRange)
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);
    // nichts zu tun ...

#ifdef SQL_TEST_PARSETREEITERATOR
    cout << "OSQLParseTreeIterator::setTableName"
         << (const char *) rTableName << ", "
         << (const char *) rSchemaName << ", "
         << (const char *) rTableRange
         << "\n";
#endif
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::appendColumns(const OSQLTable& _rTable)
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);
    if (!_rTable.is())
        return;

    Reference<XNameAccess> xColumns = _rTable->getColumns();

    Sequence< ::rtl::OUString > aColNames =  xColumns->getElementNames();
    const ::rtl::OUString* pBegin = aColNames.getConstArray();
    const ::rtl::OUString* pEnd = pBegin + aColNames.getLength();

    for(;pBegin != pEnd;++pBegin)
    {
        OSQLColumns::const_iterator aIter = find(m_aSelectColumns->begin(),m_aSelectColumns->end(),*pBegin,m_aCaseEqual);
        ::rtl::OUString aName(*pBegin);
        sal_Int32 i=1;
        while(aIter != m_aSelectColumns->end())
        {
            aName = *pBegin + ::rtl::OUString::valueOf(i++);
            aIter = find(m_aSelectColumns->begin(),m_aSelectColumns->end(),aName,m_aCaseEqual);
        }
        Reference< XFastPropertySet > xColumn;
        if(xColumns->getByName(*pBegin) >>= xColumn)
        {
            OParseColumn* pColumn = new OParseColumn(xColumn,m_xDatabaseMetaData->storesMixedCaseQuotedIdentifiers());
            //  pColumn->setTableName(aIter->first);
            pColumn->setRealName(aName);
            Reference< XFastPropertySet> xCol = pColumn;
            m_aSelectColumns->push_back(xCol);
        }
    }
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setSelectColumnName(const String & rColumnName,const String & rColumnAlias, const String & rTableRange,BOOL bFkt)
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);
    if(rColumnName.GetChar(0) == '*' && !rTableRange.Len())
    {   // Suche "uber alle vorkommenden Tabellen
        for(ConstOSQLTablesIterator aIter = m_aTables.begin(); aIter != m_aTables.end();++aIter)
            appendColumns(aIter->second);
    }
    else if(rColumnName.GetChar(0) == '*' && rTableRange.Len())  // alle Columns aus dieser Tabelle
    {
        ConstOSQLTablesIterator aFind = m_aTables.find(rTableRange);

        if(aFind == m_aTables.end())
        {
            String strExpression = rTableRange;
            strExpression += '.';
            strExpression += rColumnName;
            CallError(2, strExpression);
        }
        else
            appendColumns(aFind->second);
    }
    else if(!rTableRange.Len())// ein Columnname existiert
    {
        if(!bFkt)
        {

            UINT32 ncount = m_aSelectColumns->size();
            for(OSQLTablesIterator aIter = m_aTables.begin(); aIter != m_aTables.end(); ++aIter)
            {
                if (aIter->second.is())
                {
                    Reference<XNameAccess> xColumns = aIter->second->getColumns();

                    Reference< XFastPropertySet > xColumn;
                    if(xColumns->getByName(rColumnName) >>= xColumn)
                    {
                        String aNewColName(getUniqueColumnName(rColumnAlias));

                        OParseColumn* pColumn = new OParseColumn(xColumn,m_xDatabaseMetaData->storesMixedCaseQuotedIdentifiers());
                        pColumn->setTableName(aIter->first);
                        pColumn->setName(aNewColName);
                        pColumn->setRealName(rColumnName);

                        Reference< XFastPropertySet> xCol = pColumn;
                        m_aSelectColumns->push_back(xCol);
                        continue; // diese Column darf nur einmal vorkommen
                    }
                }
            }
            if (ncount == m_aSelectColumns->size())
            {
                String strExpression = rTableRange;
                if (strExpression.Len())
                    strExpression += '.';
                strExpression += rColumnName;
                CallError(2, strExpression);
            }
        }
        else
        {
            String aNewColName(getUniqueColumnName(rColumnAlias));

            OParseColumn* pColumn = new OParseColumn(aNewColName,::rtl::OUString(),::rtl::OUString(),
                ColumnValue::NULLABLE_UNKNOWN,0,0,DataType::VARCHAR,sal_False,sal_False,m_xDatabaseMetaData->storesMixedCaseQuotedIdentifiers());
            pColumn->setFunction(bFkt);
            pColumn->setRealName(rColumnName);

            Reference< XFastPropertySet> xCol = pColumn;
            m_aSelectColumns->push_back(xCol);
        }
    }
    else    // ColumnName und Tablename vorhanden
    {
        ConstOSQLTablesIterator aFind = m_aTables.find(rTableRange);

        BOOL bError = FALSE;
        if (aFind->second.is())
        {

            if (bFkt)
            {
                String aNewColName(getUniqueColumnName(rColumnAlias));

                OParseColumn* pColumn = new OParseColumn(aNewColName,::rtl::OUString(),::rtl::OUString(),
                    ColumnValue::NULLABLE_UNKNOWN,0,0,DataType::VARCHAR,sal_False,sal_False,m_xDatabaseMetaData->storesMixedCaseQuotedIdentifiers());
                pColumn->setFunction(sal_True);
                pColumn->setRealName(rColumnName);
                pColumn->setTableName(aFind->first);

                Reference< XFastPropertySet> xCol = pColumn;
                m_aSelectColumns->push_back(xCol);
            }
            else
            {
                Reference< XFastPropertySet > xColumn;
                if (aFind->second->getColumns()->getByName(rColumnName) >>= xColumn)
                {
                    String aNewColName(getUniqueColumnName(rColumnAlias));

                    OParseColumn* pColumn = new OParseColumn(xColumn,m_xDatabaseMetaData->storesMixedCaseQuotedIdentifiers());
                    pColumn->setName(aNewColName);
                    pColumn->setRealName(rColumnName);
                    pColumn->setTableName(aFind->first);

                    Reference< XFastPropertySet> xCol = pColumn;
                    m_aSelectColumns->push_back(xCol);
                }
                else
                    bError = TRUE;
            }
        }
        else
            bError = TRUE;

        // Tabelle existiert nicht oder Feld nicht vorhanden
        if (bError)
        {
            String strExpression = rTableRange;
            if (strExpression.Len())
                strExpression += '.';
            strExpression += rColumnName;
            if (!CallError(2, strExpression))
                return;

            String aNewColName(getUniqueColumnName(rColumnAlias));

            OParseColumn* pColumn = new OParseColumn(aNewColName,::rtl::OUString(),::rtl::OUString(),
                ColumnValue::NULLABLE_UNKNOWN,0,0,DataType::VARCHAR,sal_False,sal_False,m_xDatabaseMetaData->storesMixedCaseQuotedIdentifiers());
            pColumn->setFunction(sal_True);


            Reference< XFastPropertySet> xCol = pColumn;
            m_aSelectColumns->push_back(xCol);
        }
    }
}
//-----------------------------------------------------------------------------
String OSQLParseTreeIterator::getUniqueColumnName(const String & rColumnName)   const
{
    ::rtl::OUString aAlias(rColumnName);

    OSQLColumns::const_iterator aIter = find(m_aSelectColumns->begin(),m_aSelectColumns->end(),aAlias,m_aCaseEqual);
    sal_Int32 i=1;
    while(aIter != m_aSelectColumns->end())
    {
        (aAlias = rColumnName) += ::rtl::OUString::valueOf(i++);
        aIter = find(m_aSelectColumns->begin(),m_aSelectColumns->end(),aAlias,m_aCaseEqual);
    }
    return aAlias;
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setOrderByColumnName(const String & rColumnName, const String & rTableRange,BOOL bAscending)
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);
    // nichts zu tun ...

#ifdef SQL_TEST_PARSETREEITERATOR
    cout << "OSQLParseTreeIterator::setOrderByColumnName: "
         << (const char *) rColumnName << ", "
         << (const char *) rTableRange << ", "
         << (bAscending ? "TRUE" : "FALSE")
         << "\n";
#endif
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setORCriteriaPre()
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);
    // nichts zu tun ...

#ifdef SQL_TEST_PARSETREEITERATOR
    cout << "OSQLParseTreeIterator::setORCriteriaPre\n";
#endif
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setORCriteriaPost()
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);
    // nichts zu tun ...

#ifdef SQL_TEST_PARSETREEITERATOR
    cout << "OSQLParseTreeIterator::setORCriteriaPost\n";
#endif
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setANDCriteriaPre()
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);
    // nichts zu tun ...

#ifdef SQL_TEST_PARSETREEITERATOR
    cout << "OSQLParseTreeIterator::setANDCriteriaPre\n";
#endif
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setANDCriteriaPost()
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);
    // nichts zu tun ...

#ifdef SQL_TEST_PARSETREEITERATOR
    cout << "OSQLParseTreeIterator::setANDCriteriaPost\n";
#endif
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setPredicate(const String & rColumnName,
                              const String & rTableRange,
                              OSQLPredicateType ePredicateType,
                              const String & rValue,
                              const String & rParameterName)
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);
    // nichts zu tun ...

#ifdef SQL_TEST_PARSETREEITERATOR
    cout << "OSQLParseTreeIterator::setPredicate: "
         << (const char *) rColumnName << ", "
         << (const char *) rTableRange << ", "
         << (char) ePredicateType << ", "
         << (const char *) rValue << ", "
         << (const char *) rParameterName
         << "\n";
#endif
}


//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setAssign(const String & rColumnName,
                           const String & rValue, BOOL bsetNull,
                           const String & rParameterName)
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);
    // nichts zu tun ...

#ifdef SQL_TEST_PARSETREEITERATOR
    cout << "OSQLParseTreeIterator::setAssign: "
         << (const char *) rColumnName << ", "
         << (const char *) rValue << ", "
         << (bsetNull ? "TRUE" : "FALSE") << ", "
         << (const char *) rParameterName
         << "\n";
#endif
}

//-----------------------------------------------------------------------------
const OSQLParseNode* OSQLParseTreeIterator::getWhereTree() const
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);

    if (!m_pParseTree)
        return NULL;

    // Parse Tree analysieren (je nach Statement-Typ)
    // und Zeiger auf WHERE-Klausel setzen:
    OSQLParseNode * pWhereClause = NULL;
    if(getStatementType() == SQL_STATEMENT_SELECT)
    {
        DBG_ASSERT(m_pParseTree->count() >= 4,"ParseTreeIterator: Fehler im Parse Tree");
        OSQLParseNode * pTableExp = m_pParseTree->getChild(3);
        DBG_ASSERT(pTableExp != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
        DBG_ASSERT(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator: Fehler im Parse Tree");
        DBG_ASSERT(pTableExp->count() == 5,"OSQLParseTreeIterator: Fehler im Parse Tree");

        pWhereClause = pTableExp->getChild(1);
    }
    else if (SQL_ISRULE(m_pParseTree,update_statement_searched) ||
             SQL_ISRULE(m_pParseTree,delete_statement_searched))
    {
        pWhereClause = m_pParseTree->getChild(m_pParseTree->count()-1);
    }
    if(pWhereClause->count() != 2)
        pWhereClause = NULL;
    return pWhereClause;
}

//-----------------------------------------------------------------------------
const OSQLParseNode* OSQLParseTreeIterator::getOrderTree() const
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);

    if (!m_pParseTree || getStatementType() != SQL_STATEMENT_SELECT)
        return NULL;

    // Parse Tree analysieren (je nach Statement-Typ)
    // und Zeiger auf ORDER-Klausel setzen:
    OSQLParseNode * pOrderClause = NULL;
    DBG_ASSERT(m_pParseTree->count() >= 4,"ParseTreeIterator: Fehler im Parse Tree");
    OSQLParseNode * pTableExp = m_pParseTree->getChild(3);
    DBG_ASSERT(pTableExp != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
    DBG_ASSERT(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator: Fehler im Parse Tree");
    DBG_ASSERT(pTableExp->count() == 5,"OSQLParseTreeIterator: Fehler im Parse Tree");

    pOrderClause = pTableExp->getChild(4);
    // Wenn es aber eine order_by ist, dann darf sie nicht leer sein:
    if(pOrderClause->count() != 3)
        pOrderClause = NULL;
    return pOrderClause;
}
//-----------------------------------------------------------------------------
const OSQLParseNode* OSQLParseTreeIterator::getGroupByTree() const
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);

    if (!m_pParseTree || getStatementType() != SQL_STATEMENT_SELECT)
        return NULL;

    // Parse Tree analysieren (je nach Statement-Typ)
    // und Zeiger auf ORDER-Klausel setzen:
    OSQLParseNode * pGroupClause = NULL;
    DBG_ASSERT(m_pParseTree->count() >= 4,"ParseTreeIterator: Fehler im Parse Tree");
    OSQLParseNode * pTableExp = m_pParseTree->getChild(3);
    DBG_ASSERT(pTableExp != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
    DBG_ASSERT(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator: Fehler im Parse Tree");
    DBG_ASSERT(pTableExp->count() == 5,"OSQLParseTreeIterator: Fehler im Parse Tree");

    pGroupClause = pTableExp->getChild(3);
    // Wenn es aber eine order_by ist, dann darf sie nicht leer sein:
    if(pGroupClause->count() != 3)
        pGroupClause = NULL;
    return pGroupClause;
}
//-----------------------------------------------------------------------------
const OSQLParseNode* OSQLParseTreeIterator::getHavingTree() const
{
    DBG_CHKTHIS(OSQLParseTreeIterator,NULL);

    if (!m_pParseTree || getStatementType() != SQL_STATEMENT_SELECT)
        return NULL;

    // Parse Tree analysieren (je nach Statement-Typ)
    // und Zeiger auf ORDER-Klausel setzen:
    OSQLParseNode * pHavingClause = NULL;
    DBG_ASSERT(m_pParseTree->count() >= 4,"ParseTreeIterator: Fehler im Parse Tree");
    OSQLParseNode * pTableExp = m_pParseTree->getChild(3);
    DBG_ASSERT(pTableExp != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
    DBG_ASSERT(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator: Fehler im Parse Tree");
    DBG_ASSERT(pTableExp->count() == 5,"OSQLParseTreeIterator: Fehler im Parse Tree");

    pHavingClause = pTableExp->getChild(3);
    // Wenn es aber eine order_by ist, dann darf sie nicht leer sein:
    if(pHavingClause->count() < 1)
        pHavingClause = NULL;
    return pHavingClause;
}
//-----------------------------------------------------------------------------
BOOL OSQLParseTreeIterator::CallError(USHORT nError,const String& rExpression)
{
//  SdbParseIteratorErrorInfo aErrorInfo;
//  aErrorInfo.aStatus = rStatus;
//  aErrorInfo.nErrorCode = nError;
//  aErrorInfo.aExpression = rExpression;
//
//  long nResult = aErrorHdl.Call(&aErrorInfo);
//  if (nResult == RET_CONTINUE)
        return TRUE;

//  if (nResult == RET_HANDLED)
//  {
//      //  aIteratorStatus.set(SQL_STAT_SUCCESS);
//      return FALSE;
//  }
//
//  aIteratorStatus = rStatus;
//  return FALSE;
//
}

