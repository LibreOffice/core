/*************************************************************************
 *
 *  $RCSfile: sqliterator.cxx,v $
 *
 *  $Revision: 1.41 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:16:44 $
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
#ifndef _CONNECTIVITY_SDBCX_TABLE_HXX_
#include "connectivity/sdbcx/VTable.hxx"
#endif

#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include <connectivity/sqlparse.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
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
#include <iostream>
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/PColumn.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include "connectivity/dbtools.hxx"
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLFILTEROPERATOR_HPP_
#include <com/sun/star/sdb/SQLFilterOperator.hpp>
#endif

using namespace ::comphelper;
using namespace ::connectivity;
using namespace ::connectivity::sdbcx;
using namespace ::dbtools;
using namespace ::connectivity::parse;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;

static ::rtl::OUString aEmptyString;

//-----------------------------------------------------------------------------
OSQLParseTreeIterator::OSQLParseTreeIterator()
    : m_pParseTree(NULL)
    , m_xTables(NULL)
    , m_xDatabaseMetaData(NULL)
    ,m_pParser(NULL)
{
//  m_aSelectColumns    = new OSQLColumns();
//  m_aParameters       = new OSQLColumns();
    setParseTree(NULL);
}
//-----------------------------------------------------------------------------
OSQLParseTreeIterator::OSQLParseTreeIterator(const Reference< XNameAccess>& _xTables ,
                                             const Reference< XDatabaseMetaData>& _xDatabaseMetaData,
                                             const OSQLParseNode* pRoot,
                                             const OSQLParser* _pParser)
    : m_xTables(_xTables)
    , m_xDatabaseMetaData(_xDatabaseMetaData)
    , m_aTables(_xDatabaseMetaData.is() && _xDatabaseMetaData->storesMixedCaseQuotedIdentifiers())
    , m_aCaseEqual(_xDatabaseMetaData.is() && _xDatabaseMetaData->storesMixedCaseQuotedIdentifiers())
    ,m_pParser(_pParser)
{
//  m_aSelectColumns = new OSQLColumns();// must be done because we need an empty column at zero
//  m_aParameters    = new OSQLColumns();
    setParseTree(pRoot);
}
//-----------------------------------------------------------------------------
OSQLParseTreeIterator::OSQLParseTreeIterator(const OSQLParseTreeIterator & rIter)
    :m_xTables(NULL)
    ,m_pParseTree(NULL)
    , m_xDatabaseMetaData(NULL)
    ,m_pParser(NULL)
{
    OSL_ASSERT("OSQLParseTreeIterator: Copy-Konstruktor nicht implementiert!");
}

//-----------------------------------------------------------------------------
OSQLParseTreeIterator::~OSQLParseTreeIterator()
{
    dispose();
}
// -----------------------------------------------------------------------------
void OSQLParseTreeIterator::dispose()
{
    m_aSelectColumns    = NULL;
    m_aGroupColumns     = NULL;
    m_aOrderColumns     = NULL;
    m_aParameters       = NULL;
    m_xTables           = NULL;
    m_xDatabaseMetaData = NULL;
    m_pParser           = NULL;
    m_aCreateColumns    = NULL;
    m_aTables.clear();
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setParseTree(const OSQLParseNode * pNewParseTree)
{
    m_aTables.clear();

    m_aSelectColumns = new OSQLColumns();
    m_aGroupColumns = new OSQLColumns();
    m_aOrderColumns = new OSQLColumns();
    m_aParameters    = new OSQLColumns();
    m_aCreateColumns = new OSQLColumns();

    m_pParseTree = pNewParseTree;
    if (!m_pParseTree)
    {
        m_eStatementType = SQL_STATEMENT_UNKNOWN;
        return;
    }

    // falls m_pParseTree aber keine Connection, dann Fehler
    if(!m_xTables.is())
    {
        return;
    }

    m_aWarning = SQLWarning();
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
    else if (SQL_ISRULE(m_pParseTree,update_statement_searched))
    {
        m_eStatementType = SQL_STATEMENT_UPDATE;
    }
    else if (SQL_ISRULE(m_pParseTree,delete_statement_searched))
    {
        m_eStatementType = SQL_STATEMENT_DELETE;
    }
    else if (m_pParseTree->count() == 3 && SQL_ISRULE(m_pParseTree->getChild(1),odbc_call_spec))
    {
        m_eStatementType = SQL_STATEMENT_ODBC_CALL;
    }
    else if (SQL_ISRULE(m_pParseTree->getChild(0),base_table_def))
    {
        m_eStatementType = SQL_STATEMENT_CREATE_TABLE;
        m_pParseTree = m_pParseTree->getChild(0);
    }
    else
    {
        m_eStatementType = SQL_STATEMENT_UNKNOWN;
        //aIteratorStatus.setInvalidStatement();
        return;
    }
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseOneTableName(const OSQLParseNode * pTableName, const ::rtl::OUString & rTableRange, const sal_Bool bIsCreateTable)
{


    OSL_ENSURE(pTableName != NULL,"OSQLParseTreeIterator::traverseOneTableName: pTableName == NULL");

    Any aCatalog;
    ::rtl::OUString aSchema,aTableName,aComposedName;
    ::rtl::OUString aTableRange(rTableRange);

    // Tabellenname abholen
    OSQLParseNode::getTableComponents(pTableName,aCatalog,aSchema,aTableName);

    // create the composed name like DOMAIN.USER.TABLE1
    ::dbtools::composeTableName(m_xDatabaseMetaData,
                                aCatalog.hasValue() ? ::comphelper::getString(aCatalog) : ::rtl::OUString(),
                                aSchema,
                                aTableName,
                                aComposedName,
                                sal_False,
                                ::dbtools::eInDataManipulation);
    // if there is no alias for the table name assign the orignal name to it
    if (!aTableRange.getLength())
        aTableRange = aComposedName;

    if(aComposedName.getLength())
    {
        try
        {
            if(!m_xTables->hasByName(aComposedName)) // name not in XNameAccess
            {
                // exists the name in the metadata ?!
                if(!aSchema.getLength())
                    aSchema = ::rtl::OUString::createFromAscii("%");

                static const ::rtl::OUString s_sTableTypeView(RTL_CONSTASCII_USTRINGPARAM("VIEW"));
                static const ::rtl::OUString s_sTableTypeTable(RTL_CONSTASCII_USTRINGPARAM("TABLE"));

                // we want all catalogues, all schemas, all tables
                Sequence< ::rtl::OUString > sTableTypes(3);
                static const ::rtl::OUString sWildcard = ::rtl::OUString::createFromAscii("%");
                sTableTypes[0] = s_sTableTypeView;
                sTableTypes[1] = s_sTableTypeTable;
                sTableTypes[2] = sWildcard; // just to be sure to include anything else ....

                if ( m_xDatabaseMetaData.is() )
                {
                    Reference< XResultSet> xRes = m_xDatabaseMetaData->getTables(aCatalog,aSchema,aTableName,sTableTypes);
                    aComposedName = ::rtl::OUString(); // now clear the name to avoid reassignment
                    if(xRes.is() && xRes->next())
                    {
                        ::rtl::OUString sCatalog, sSchema, sName;
                        Reference< XRow > xCurrentRow(xRes, UNO_QUERY);
                        sCatalog    = xCurrentRow->getString(1);
                        if(xCurrentRow->wasNull())
                            sCatalog= ::rtl::OUString();

                        sSchema     = xCurrentRow->getString(2);
                        if(xCurrentRow->wasNull())
                            sSchema= ::rtl::OUString();
                        sName       = xCurrentRow->getString(3);
                        if(xCurrentRow->wasNull())
                            sName= ::rtl::OUString();
                        ::dbtools::composeTableName(m_xDatabaseMetaData, sCatalog,
                                                                        sSchema,
                                                                        sName,
                                                                        aComposedName,
                                                                        sal_False,
                                                                        ::dbtools::eInDataManipulation);
                    }
                }
            }

            sal_Bool bHasByName=m_xTables->hasByName(aComposedName);
            if(!bIsCreateTable) //for select statement etc.
            {
                if (bHasByName)
                    m_xTables->getByName(aComposedName) >>= m_aTables[aTableRange];
                else if(m_pParser)
                {
                    ::rtl::OUString sErrMsg = m_pParser->getContext().getErrorMessage(IParseContext::ERROR_INVALID_TABLE);
                    sErrMsg = sErrMsg.replaceAt(sErrMsg.indexOf('#'),1,aTableName);
                    appendWarning(sErrMsg);
                }
            }
            else    //for create table or view, can't have the same name
            {
                if (!bHasByName)
                    setTableName(aTableName
                                ,aCatalog.hasValue() ? ::comphelper::getString(aCatalog) : ::rtl::OUString()
                                ,aSchema
                                ,aTableRange);
                else if(m_pParser)
                {
                    ::rtl::OUString sErrMsg = m_pParser->getContext().getErrorMessage(IParseContext::ERROR_INVALID_TABLE_EXIST);
                    sErrMsg = sErrMsg.replaceAt(sErrMsg.indexOf('#'),1,aTableName);
                    appendWarning(sErrMsg);
                }
            }

        }
        catch(Exception&)
        {
            if(m_pParser)
            {
                ::rtl::OUString sErrMsg = m_pParser->getContext().getErrorMessage(IParseContext::ERROR_INVALID_TABLE);
                sErrMsg = sErrMsg.replaceAt(sErrMsg.indexOf('#'),1,aTableName);
                appendWarning(sErrMsg);
            }
        }

    }
}
//-----------------------------------------------------------------------------
OSQLParseNode * OSQLParseTreeIterator::getQualified_join(OSQLParseNode *pTableRef,::rtl::OUString& aTableRange)
{

    aTableRange = ::rtl::OUString();

    OSQLParseNode *pNode = getTableRef(pTableRef->getChild(0),aTableRange);
    if(isTableNode(pNode))
        traverseOneTableName(pNode,aTableRange,sal_False);

    sal_uInt32 nPos = 4;
    if(SQL_ISRULE(pTableRef,cross_union) || pTableRef->getChild(1)->getTokenID() != SQL_TOKEN_NATURAL)
        nPos = 3;

    pNode = getTableRef(pTableRef->getChild(nPos),aTableRange);
    if(isTableNode(pNode))
        traverseOneTableName(pNode,aTableRange,sal_False);

    return pNode;
}
//-----------------------------------------------------------------------------
OSQLParseNode * OSQLParseTreeIterator::getTableRef(OSQLParseNode *pTableRef,::rtl::OUString& aTableRange)
{

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
            if(!isTableNode(pTableName))
                pTableName = pTableRef->getChild(0);
            aTableRange = ::rtl::OUString();
            if(pTableRef->count() == 4)
                aTableRange = pTableRef->getChild(2)->getTokenValue(); // Tabellenrange an Pos 2
        }
    }
    else if(SQL_ISRULE(pTableName,table_ref))
    {
        pTableName = pTableRef->getChild(0);
        aTableRange = ::rtl::OUString();
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
        }
    }
    return pTableName;
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::getSelect_statement(OSQLParseNode *pSelect)
{

    if(SQL_ISRULE(pSelect,union_statement))
    {
        getSelect_statement(pSelect->getChild(0));
        //getSelect_statement(pSelect->getChild(3));
        return;
    }
    OSQLParseNode * pTableRefCommalist = pSelect->getChild(3)->getChild(0)->getChild(1);

    OSL_ENSURE(pTableRefCommalist != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
    OSL_ENSURE(SQL_ISRULE(pTableRefCommalist,table_ref_commalist),"OSQLParseTreeIterator: Fehler im Parse Tree");

    OSQLParseNode * pTableName = NULL;
    ::rtl::OUString aTableRange;
    for (sal_uInt32 i = 0; i < pTableRefCommalist->count(); i++)
    { // from clause durchlaufen
        aTableRange = ::rtl::OUString();

        if (isTableNode(pTableRefCommalist->getChild(i)))
        {
            pTableName = pTableRefCommalist->getChild(i);
            traverseOneTableName(pTableName,aTableRange,sal_False);// aTableRange will be set inside to the tablename
        }
        else if (SQL_ISRULE(pTableRefCommalist->getChild(i),table_ref))
        {
            // Tabellenreferenz kann aus Tabellennamen, Tabellennamen (+),'('joined_table')'(+) bestehen
            pTableName = pTableRefCommalist->getChild(i)->getChild(0);
            if(isTableNode(pTableName))
            {   // Tabellennamen gefunden
                if(pTableRefCommalist->getChild(i)->count() == 4)   // Tabellenrange an Pos 2
                    aTableRange = pTableRefCommalist->getChild(i)->getChild(2)->getTokenValue();
                traverseOneTableName(pTableName,aTableRange,sal_False);
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

        //  if (! aIteratorStatus.IsSuccessful()) break;
    }
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseTableNames()
{

    //  aIteratorStatus.Clear();

    if (m_pParseTree == NULL) {
        //aIteratorStatus.setInvalidStatement();
        return;
    }

    OSQLParseNode * pTableName = NULL;
    ::rtl::OUString aTableRange;

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
        traverseOneTableName(pTableName,aTableRange,sal_False);
    }
    else if (m_eStatementType == SQL_STATEMENT_UPDATE)
    {
        pTableName = m_pParseTree->getChild(1);
        traverseOneTableName(pTableName,aTableRange,sal_False);
    }
    else if (m_eStatementType == SQL_STATEMENT_DELETE)
    {
        pTableName = m_pParseTree->getChild(2);
        traverseOneTableName(pTableName,aTableRange,sal_False);
    }
    else if (m_eStatementType == SQL_STATEMENT_CREATE_TABLE)
    {
        pTableName = m_pParseTree->getChild(2);
        traverseOneTableName(pTableName,aTableRange,sal_True);
    }
}
//-----------------------------------------------------------------------------
::rtl::OUString OSQLParseTreeIterator::getColumnAlias(const OSQLParseNode* _pDerivedColumn)
{
    OSL_ENSURE(SQL_ISRULE(_pDerivedColumn,derived_column),"No derived column!");
    ::rtl::OUString sColumnAlias;
    if(_pDerivedColumn->getChild(1)->count() == 2)
        sColumnAlias = _pDerivedColumn->getChild(1)->getChild(1)->getTokenValue();
    else if(!_pDerivedColumn->getChild(1)->isRule())
        sColumnAlias = _pDerivedColumn->getChild(1)->getTokenValue();
    return sColumnAlias;
}
// -----------------------------------------------------------------------------
void OSQLParseTreeIterator::getColumnRange( const OSQLParseNode* _pColumnRef,
                        ::rtl::OUString &_rColumnName,
                        ::rtl::OUString& _rTableRange) const
{
    getColumnRange( _pColumnRef,m_xDatabaseMetaData,_rColumnName,_rTableRange);
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::getColumnRange(const OSQLParseNode* pColumnRef,
                                           const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _xMetaData,
                                           ::rtl::OUString &rColumnName,
                                           ::rtl::OUString &rTableRange)
{

    rColumnName = ::rtl::OUString();
    rTableRange = ::rtl::OUString();
    if(SQL_ISRULE(pColumnRef,column_ref))// ab hier ist es sicher eine Columnref
    {
        if(pColumnRef->count() > 1)
        {
            for(sal_Int32 i=0;i<((sal_Int32)pColumnRef->count())-2;i++)  // mu"s signed sein, falls count == 1
                pColumnRef->getChild(i)->parseNodeToStr(rTableRange,_xMetaData,NULL,sal_False,sal_False);
            // Spaltenname
            rColumnName = pColumnRef->getChild(pColumnRef->count()-1)->getChild(0)->getTokenValue();
        }
        else
            rColumnName = pColumnRef->getChild(0)->getTokenValue();
    }
    else if(SQL_ISRULE(pColumnRef,general_set_fct) || SQL_ISRULE(pColumnRef,set_fct_spec))
    { // Funktion
        pColumnRef->parseNodeToStr(rColumnName,_xMetaData);
    }
    else  if(pColumnRef->getNodeType() == SQL_NODE_NAME)
        rColumnName = pColumnRef->getTokenValue();
}

//-----------------------------------------------------------------------------
sal_Bool OSQLParseTreeIterator::getColumnTableRange(const OSQLParseNode* pNode, ::rtl::OUString &rTableRange) const
{
    // Ermitteln ob alle Spalten zu einer Tabelle gehoeren
    if (SQL_ISRULE(pNode,column_ref))
    {
        ::rtl::OUString aColName, aTableRange;
        getColumnRange(pNode, aColName, aTableRange);
        if (!aTableRange.getLength())   // keinen gefunden
        {
            // dann die Spalte in den Tabellen suchen
            for (ConstOSQLTablesIterator aIter = m_aTables.begin(); aIter != m_aTables.end(); ++aIter)
            {
                if (aIter->second.is())
                {
                    try
                    {
                        Reference< XNameAccess > xColumns = aIter->second->getColumns();
                        if(xColumns->hasByName(aColName))
                        {
                            Reference< XPropertySet > xColumn;
                            if (xColumns->getByName(aColName) >>= xColumn)
                            {
                                OSL_ENSURE(xColumn.is(),"Column isn't a propertyset!");
                                aTableRange = aIter->first;
                                break;
                            }
                        }
                    }
                    catch(Exception&)
                    {
                    }
                }
            }
            if (!aTableRange.getLength())
                return sal_False;
        }


        if (!rTableRange.getLength())
            rTableRange = aTableRange;
        else if (rTableRange != aTableRange)
            return sal_False;
    }
    else
    {
        for (sal_uInt32 i = 0, ncount = pNode->count(); i < ncount; i++)
        {
            if (!getColumnTableRange(pNode->getChild(i), rTableRange))
                return sal_False;
        }
    }
    return sal_True;
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseCreateColumns(const OSQLParseNode* pSelectNode)
{
    //  aIteratorStatus.Clear();

    if (!pSelectNode || m_eStatementType != SQL_STATEMENT_CREATE_TABLE || m_aTables.empty())
    {
        if(m_pParser)
            appendWarning(m_pParser->getContext().getErrorMessage(IParseContext::ERROR_GENERAL));
        return;
    }
    if (!SQL_ISRULE(pSelectNode,base_table_element_commalist))
        return ;

    for (sal_uInt32 i = 0; i < pSelectNode->count(); i++)
    {
        OSQLParseNode *pColumnRef = pSelectNode->getChild(i);

        if (SQL_ISRULE(pColumnRef,column_def))
        {
            ::rtl::OUString aColumnName;
            ::rtl::OUString aTypeName;
            ::rtl::OUString aTableRange;
            sal_Int32 nType = DataType::VARCHAR;
            sal_Int32 nLen  = 0;
            aColumnName = pColumnRef->getChild(0)->getTokenValue();

            OSQLParseNode *pDatatype = pColumnRef->getChild(1);
            if (pDatatype && SQL_ISRULE(pDatatype,data_type))
            {
                //data type
                //  0 | 1| 2  |3
                //char  (  20  )
                aTypeName = pDatatype->getChild(0)->getTokenValue();
                if (pDatatype->count() == 4
                    && SQL_ISPUNCTUATION(pDatatype->getChild(1), "(")
                    && SQL_ISPUNCTUATION(pDatatype->getChild(3) , ")") )
                {
                    nLen = pDatatype->getChild(2)->getTokenValue().toInt32();
                }
            }
            else if(pDatatype && pDatatype->getNodeType() == SQL_NODE_KEYWORD)
            {
                aTypeName = ::rtl::OUString::createFromAscii("VARCHAR");
            }

            if (aTypeName.getLength())
            {
                //TO DO:Create a new class for create statement to handle field length
                OParseColumn* pColumn = new OParseColumn(aColumnName,aTypeName,::rtl::OUString(),
                    ColumnValue::NULLABLE_UNKNOWN,0,0,nType,sal_False,sal_False,m_aCaseEqual.isCaseSensitive());
                pColumn->setFunction(sal_False);
                pColumn->setRealName(aColumnName);

                Reference< XPropertySet> xCol = pColumn;
                m_aCreateColumns->push_back(xCol);
            }
        }

    }
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseSelectColumnNames(const OSQLParseNode* pSelectNode)
{

    //  aIteratorStatus.Clear();

    if (!pSelectNode || m_eStatementType != SQL_STATEMENT_SELECT || m_aTables.empty())
    {
        if(m_pParser)
            appendWarning(m_pParser->getContext().getErrorMessage(IParseContext::ERROR_GENERAL));
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
        setSelectColumnName(m_aSelectColumns,::rtl::OUString::createFromAscii("*"), aEmptyString,aEmptyString);
    }
    else if (SQL_ISRULE(pSelectNode->getChild(2),scalar_exp_commalist))
    {
        // SELECT column[,column] oder SELECT COUNT(*) ...
        OSQLParseNode * pSelection = pSelectNode->getChild(2);

        for (sal_uInt32 i = 0; i < pSelection->count(); i++)
        {
            OSQLParseNode *pColumnRef = pSelection->getChild(i);

            //if (SQL_ISRULE(pColumnRef,select_sublist))
            if (SQL_ISRULE(pColumnRef,derived_column) &&
                SQL_ISRULE(pColumnRef->getChild(0),column_ref) &&
                pColumnRef->getChild(0)->count() == 3 &&
                SQL_ISPUNCTUATION(pColumnRef->getChild(0)->getChild(2),"*"))
            {
                // alle Spalten der Tabelle
                ::rtl::OUString aTableRange;
                pColumnRef->getChild(0)->parseNodeToStr(aTableRange,m_xDatabaseMetaData,NULL,sal_False,sal_False);
                setSelectColumnName(m_aSelectColumns,::rtl::OUString::createFromAscii("*"), aEmptyString,aTableRange);
                continue;
            }
            else if (SQL_ISRULE(pColumnRef,derived_column))
            {
                ::rtl::OUString aColumnAlias(getColumnAlias(pColumnRef)); // kann leer sein
                ::rtl::OUString aColumnName;
                ::rtl::OUString aTableRange;
                sal_Int32 nType = DataType::VARCHAR;
                sal_Bool bFkt(sal_False);
                pColumnRef = pColumnRef->getChild(0);
                if (
                        pColumnRef->count() == 3 &&
                        SQL_ISPUNCTUATION(pColumnRef->getChild(0),"(") &&
                        SQL_ISPUNCTUATION(pColumnRef->getChild(2),")")
                    )
                    pColumnRef = pColumnRef->getChild(1);

                if (SQL_ISRULE(pColumnRef,column_ref))
                {
                    getColumnRange(pColumnRef,aColumnName,aTableRange);
                    OSL_ENSURE(aColumnName.getLength(),"Columnname darf nicht leer sein");
                }
                else /*if (SQL_ISRULE(pColumnRef,general_set_fct) || SQL_ISRULE(pColumnRef,set_fct_spec)    ||
                         SQL_ISRULE(pColumnRef,position_exp)    || SQL_ISRULE(pColumnRef,extract_exp)   ||
                         SQL_ISRULE(pColumnRef,length_exp)      || SQL_ISRULE(pColumnRef,char_value_fct)||
                         SQL_ISRULE(pColumnRef,num_value_exp)   || SQL_ISRULE(pColumnRef,term))*/
                {
                    /* Funktionsaufruf vorhanden */
                    pColumnRef->parseNodeToStr(aColumnName,m_xDatabaseMetaData,NULL,sal_False,sal_True);

                    // gehoeren alle beteiligten Spalten der Funktion zu einer Tabelle
                    if (m_aTables.size() == 1)
                    {
                        aTableRange = m_aTables.begin()->first;
                    }
                    else
                    {
                        getColumnTableRange(pColumnRef,aTableRange);
                    }
                    if ( pColumnRef->isRule() )
                    {
                        bFkt = sal_True;
                        if ( SQL_ISRULE(pColumnRef,num_value_exp) || SQL_ISRULE(pColumnRef,term) || SQL_ISRULE(pColumnRef,factor) )
                        {
                            nType = DataType::DOUBLE;
                        }
                        else
                        {
                            ::rtl::OUString sFunctionName;
                            if ( SQL_ISRULE(pColumnRef,length_exp) )
                                pColumnRef->getChild(0)->getChild(0)->parseNodeToStr(sFunctionName,m_xDatabaseMetaData,NULL,sal_False,sal_False);
                            else
                                pColumnRef->getChild(0)->parseNodeToStr(sFunctionName,m_xDatabaseMetaData,NULL,sal_False,sal_False);
                            nType = ::connectivity::OSQLParser::getFunctionReturnType(sFunctionName,m_pParser ? &m_pParser->getContext() : NULL);
                        }
                    }
                }
                /*
                else
                {
                    aIteratorStatus.setStatementTooComplex();
                    return;
                }
                */
                if(!aColumnAlias.getLength())
                    aColumnAlias = aColumnName;
                setSelectColumnName(m_aSelectColumns,aColumnName,aColumnAlias,aTableRange,bFkt,nType);
            }
        }

    }
}


//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseOrderByColumnNames(const OSQLParseNode* pSelectNode)
{
    traverseByColumnNames(pSelectNode,sal_True);
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseByColumnNames(const OSQLParseNode* pSelectNode,sal_Bool _bOrder)
{
    //  aIteratorStatus.Clear();

    if (pSelectNode == NULL)
    {
        //aIteratorStatus.setInvalidStatement();
        return;
    }

    if (m_eStatementType != SQL_STATEMENT_SELECT)
    {
        //aIteratorStatus.setInvalidStatement();
        return;
    }

    if(SQL_ISRULE(pSelectNode,union_statement))
    {
        traverseByColumnNames(pSelectNode->getChild(0),_bOrder);
        return;
    }

    OSL_ENSURE(pSelectNode->count() >= 4,"OSQLParseTreeIterator: Fehler im Parse Tree");

    OSQLParseNode * pTableExp = pSelectNode->getChild(3);
    OSL_ENSURE(pTableExp != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
    OSL_ENSURE(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator:table_exp Fehler im Parse Tree");
    OSL_ENSURE(pTableExp->count() == 5,"OSQLParseTreeIterator: Fehler im Parse Tree");

    sal_uInt32 nPos = ( _bOrder ? 4 : 2 );

    OSQLParseNode * pOptByClause = pTableExp->getChild(nPos);
    OSL_ENSURE(pOptByClause != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
    if ( pOptByClause->count() == 0 )
        return;

    OSL_ENSURE(pOptByClause->count() == 3,"OSQLParseTreeIterator: Fehler im Parse Tree");

    OSQLParseNode * pOrderingSpecCommalist = pOptByClause->getChild(2);
    OSL_ENSURE(pOrderingSpecCommalist != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
    OSL_ENSURE(!_bOrder || SQL_ISRULE(pOrderingSpecCommalist,ordering_spec_commalist),"OSQLParseTreeIterator:ordering_spec_commalist Fehler im Parse Tree");
    OSL_ENSURE(pOrderingSpecCommalist->count() > 0,"OSQLParseTreeIterator: Fehler im Parse Tree");

    ::rtl::OUString aColumnName,aColumnAlias;
    ::rtl::OUString aTableRange;
    sal_uInt32 nCount = pOrderingSpecCommalist->count();
    for (sal_uInt32 i = 0; i < nCount; ++i)
    {
        OSQLParseNode* pColumnRef  = pOrderingSpecCommalist->getChild(i);
        OSL_ENSURE(pColumnRef  != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
        if ( _bOrder )
        {
            OSL_ENSURE(SQL_ISRULE(pColumnRef,ordering_spec),"OSQLParseTreeIterator:ordering_spec Fehler im Parse Tree");
            OSL_ENSURE(pColumnRef->count() == 2,"OSQLParseTreeIterator: Fehler im Parse Tree");

            pColumnRef = pColumnRef->getChild(0);
        }
        aTableRange = ::rtl::OUString();
        aColumnName = ::rtl::OUString();
        if ( SQL_ISRULE(pColumnRef,column_ref) )
        {
            // Column-Name (und TableRange):
            if(SQL_ISRULE(pColumnRef,column_ref))
                getColumnRange(pColumnRef,aColumnName,aTableRange);
            else // eine Expression
                pColumnRef->parseNodeToStr(aColumnName,m_xDatabaseMetaData,NULL,sal_False,sal_False);

            OSL_ENSURE(aColumnName.getLength(),"aColumnName darf nicht leer sein");
        }
        else
        {   // here I found a predicate
            pColumnRef->parseNodeToStr(aColumnName,m_xDatabaseMetaData,NULL,sal_False,sal_False);
        }
        OSL_ENSURE(pColumnRef != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
        if ( _bOrder )
        {
            // Ascending/Descending
            OSQLParseNode * pOptAscDesc = pColumnRef->getParent()->getChild(1);
            OSL_ENSURE(pOptAscDesc != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");

            sal_Bool bAscending = pOptAscDesc && SQL_ISTOKEN(pOptAscDesc,ASC);
            setOrderByColumnName(aColumnName, aTableRange,bAscending);
        }
        else
            setGroupByColumnName(aColumnName, aTableRange);
    }
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseGroupByColumnNames(const OSQLParseNode* pSelectNode)
{
    traverseByColumnNames(pSelectNode,sal_False);
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseSelectionCriteria(const OSQLParseNode* pSelectNode)
{

    //  aIteratorStatus.Clear();

    if (pSelectNode == NULL)
    {
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
        OSL_ENSURE(pSelectNode->count() >= 4,"OSQLParseTreeIterator: Fehler im Parse Tree");

        OSQLParseNode * pTableExp = pSelectNode->getChild(3);
        OSL_ENSURE(pTableExp != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
        OSL_ENSURE(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator: Fehler im Parse Tree");
        OSL_ENSURE(pTableExp->count() == 5,"OSQLParseTreeIterator: Fehler im Parse Tree");

        pWhereClause = pTableExp->getChild(1);
    } else if (SQL_ISRULE(pSelectNode,update_statement_searched)) {
        OSL_ENSURE(pSelectNode->count() == 5,"OSQLParseTreeIterator: Fehler im Parse Tree");
        pWhereClause = pSelectNode->getChild(4);
    } else if (SQL_ISRULE(pSelectNode,delete_statement_searched)) {
        OSL_ENSURE(pSelectNode->count() == 4,"OSQLParseTreeIterator: Fehler im Parse Tree");
        pWhereClause = pSelectNode->getChild(3);
    } else if (SQL_ISRULE(pSelectNode,delete_statement_positioned)) {
        // nyi
        OSL_ASSERT("OSQLParseTreeIterator::getSelectionCriteria: positioned nyi");
    } else {
        // Anderes Statement. Keine Selektionskriterien.
        return;
    }

    if (! SQL_ISRULE(pWhereClause,where_clause)) {
        // Die Where Clause ist meistens optional, d. h. es koennte sich auch
        // um "optional_where_clause" handeln.
        OSL_ENSURE(SQL_ISRULE(pWhereClause,opt_where_clause),"OSQLParseTreeIterator: Fehler im Parse Tree");
        return;
    }

    // Wenn es aber eine where_clause ist, dann darf sie nicht leer sein:
    OSL_ENSURE(pWhereClause->count() == 2,"OSQLParseTreeIterator: Fehler im Parse Tree");

    OSQLParseNode * pComparisonPredicate = pWhereClause->getChild(1);
    OSL_ENSURE(pComparisonPredicate != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");


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
        sal_Int32 ePredicateType;
        OSQLParseNode *pPrec = pSearchCondition->getChild(1);
        if (pPrec->getNodeType() == SQL_NODE_EQUAL)
            ePredicateType = SQLFilterOperator::EQUAL;
        else if (pPrec->getNodeType() == SQL_NODE_NOTEQUAL)
            ePredicateType = SQLFilterOperator::NOT_EQUAL;
        else if (pPrec->getNodeType() == SQL_NODE_LESS)
            ePredicateType = SQLFilterOperator::LESS;
        else if (pPrec->getNodeType() == SQL_NODE_LESSEQ)
            ePredicateType = SQLFilterOperator::LESS_EQUAL;
        else if (pPrec->getNodeType() == SQL_NODE_GREATEQ)
            ePredicateType = SQLFilterOperator::GREATER_EQUAL;
        else if (pPrec->getNodeType() == SQL_NODE_GREAT)
            ePredicateType = SQLFilterOperator::GREATER;

        ::rtl::OUString aValue;
        pSearchCondition->getChild(2)->parseNodeToStr(aValue,m_xDatabaseMetaData,NULL,sal_False,sal_False);
        traverseOnePredicate(pSearchCondition->getChild(0),ePredicateType,aValue,sal_False,pSearchCondition->getChild(2));
//      if (! aIteratorStatus.IsSuccessful())
//          return;
    }
    else if (SQL_ISRULE(pSearchCondition,like_predicate) /*&& SQL_ISRULE(pSearchCondition->getChild(0),column_ref)*/)
    {
        sal_Int32 ePredicateType;

        OSL_ENSURE(pSearchCondition->count() >= 4,"OSQLParseTreeIterator: Fehler im Parse Tree");

        sal_Int32 nCurentPos = pSearchCondition->count()-2;

        OSQLParseNode * pNum_value_exp  = pSearchCondition->getChild(nCurentPos);
        OSQLParseNode * pOptEscape      = pSearchCondition->getChild(nCurentPos+1);

        if (pSearchCondition->getChild(1)->getTokenID() == SQL_TOKEN_NOT)
            ePredicateType = SQLFilterOperator::NOT_LIKE;
        else
            ePredicateType = SQLFilterOperator::LIKE;

        OSL_ENSURE(pNum_value_exp != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
        OSL_ENSURE(pOptEscape != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");

        if (pOptEscape->count() != 0)
        {
            //  aIteratorStatus.setStatementTooComplex();
            return;
        }

        ::rtl::OUString aValue;
        OSQLParseNode * pParam = NULL;
        if (SQL_ISRULE(pNum_value_exp,parameter))
            pParam = pNum_value_exp;
        else if(pNum_value_exp->isToken())
            // Normaler Wert
            aValue = pNum_value_exp->getTokenValue();
        else
        {
            pNum_value_exp->parseNodeToStr(aValue,m_xDatabaseMetaData,NULL,sal_False,sal_False);
            pParam = pNum_value_exp;
        }

        traverseOnePredicate(pSearchCondition->getChild(0),ePredicateType,aValue,sal_False,pParam);
//      if (! aIteratorStatus.IsSuccessful())
//          return;
    }
    else if (SQL_ISRULE(pSearchCondition,in_predicate))
    {
        OSL_ENSURE(pSearchCondition->count() == 4,"OSQLParseTreeIterator: Fehler im Parse Tree");
        setORCriteriaPre();
        //  if (! aIteratorStatus.IsSuccessful()) return;

        traverseORCriteria(pSearchCondition->getChild(0));
        //  if (! aIteratorStatus.IsSuccessful()) return;

        setORCriteriaPost();

        OSQLParseNode* pChild = pSearchCondition->getChild(3);
        if ( SQL_ISRULE(pChild->getChild(0),subquery) )
        {
            traverseSelectionCriteria(pChild->getChild(0)->getChild(1));
        }
        else
        { // '(' value_exp_commalist ')'
            pChild = pChild->getChild(1);
            sal_Int32 nCount = pChild->count();
            for (sal_Int32 i=0; i < nCount; ++i)
            {
                traverseANDCriteria(pChild->getChild(i));
            }
        }
    }
    else if (SQL_ISRULE(pSearchCondition,test_for_null) /*&& SQL_ISRULE(pSearchCondition->getChild(0),column_ref)*/)
    {
        sal_Int32 ePredicateType;

        OSL_ENSURE(pSearchCondition->count() >= 3,"OSQLParseTreeIterator: Fehler im Parse Tree");
        OSL_ENSURE(SQL_ISTOKEN(pSearchCondition->getChild(1),IS),"OSQLParseTreeIterator: Fehler im Parse Tree");

        if (SQL_ISTOKEN(pSearchCondition->getChild(2),NOT) )
            ePredicateType = SQLFilterOperator::NOT_SQLNULL;
        else
            ePredicateType = SQLFilterOperator::SQLNULL;

        ::rtl::OUString aString;
        traverseOnePredicate(pSearchCondition->getChild(0),ePredicateType,aString,sal_True,NULL);
        //  if (! aIteratorStatus.IsSuccessful()) return;
    }
    else if (SQL_ISRULE(pSearchCondition,num_value_exp) || SQL_ISRULE(pSearchCondition,term))
    {
        sal_Int32 ePredicateType = SQLFilterOperator::EQUAL;
        ::rtl::OUString aString;
        traverseOnePredicate(pSearchCondition->getChild(0),ePredicateType,aString,sal_False,pSearchCondition->getChild(0));
        traverseOnePredicate(pSearchCondition->getChild(2),ePredicateType,aString,sal_False,pSearchCondition->getChild(2));
    }
    // Fehler einfach weiterreichen.
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseOnePredicate(
                                OSQLParseNode * pColumnRef,
                                sal_Int32 ePredicateType,
                                ::rtl::OUString& rValue,
                                sal_Bool bCompareNull,
                                OSQLParseNode * pParseNode)
{


    // Column-Name (und TableRange):
    ::rtl::OUString aColumnName;
    ::rtl::OUString aTableRange;
    getColumnRange(pColumnRef,aColumnName,aTableRange);

    ::rtl::OUString aName;
    if (pParseNode)         //event. Parameter, oder Columnref oder
    {
        if (SQL_ISRULE(pParseNode,parameter))
        {
            OSL_ENSURE(pParseNode->count() > 0,"OSQLParseTreeIterator: Fehler im Parse Tree");
            OSQLParseNode * pMark = pParseNode->getChild(0);

            ::rtl::OUString aParameterName;
            if (SQL_ISPUNCTUATION(pMark,"?"))
            {
                // Name = "?", da kein Parametername verfuegbar (z. B. bei Native SQL)
                rValue = ::rtl::OUString::createFromAscii("?");
                if(aColumnName.getLength())
                    rValue = aColumnName;
                aName  = ::rtl::OUString::createFromAscii("?");
            }
            else if (SQL_ISPUNCTUATION(pMark,":"))
            {
                rValue = pParseNode->getChild(1)->getTokenValue();
                aName = ::rtl::OUString::createFromAscii(":");
            }
            else if (SQL_ISPUNCTUATION(pMark,"["))
            {
                rValue = pParseNode->getChild(1)->getTokenValue();
                aName = ::rtl::OUString::createFromAscii("[");
            }
            else
            {
                OSL_ASSERT("OSQLParseTreeIterator: Fehler im Parse Tree");
            }
            // found a parameter
            if(SQL_ISRULE(pColumnRef,general_set_fct) || SQL_ISRULE(pColumnRef,set_fct_spec))
            {// found a function as column_ref
                ::rtl::OUString sFunctionName;
                pColumnRef->getChild(0)->parseNodeToStr(sFunctionName,m_xDatabaseMetaData,NULL,sal_False,sal_False);
                sal_Int32 nType = ::connectivity::OSQLParser::getFunctionReturnType(sFunctionName,m_pParser ? &m_pParser->getContext() : NULL);

                OParseColumn* pColumn = new OParseColumn(   rValue  ,
                                                            ::rtl::OUString(),
                                                            ::rtl::OUString(),
                                                            ColumnValue::NULLABLE_UNKNOWN,
                                                            0,
                                                            0,
                                                            nType,
                                                            sal_False,
                                                            sal_False,
                                                            m_aCaseEqual.isCaseSensitive());
                pColumn->setFunction(sal_True);
                pColumn->setRealName(sFunctionName);
                m_aParameters->push_back(pColumn);
            }
            else
            {
                OSQLColumns::const_iterator aIter = ::connectivity::find(m_aSelectColumns->begin(),m_aSelectColumns->end(),aColumnName,m_aCaseEqual);
                if(aIter != m_aSelectColumns->end())
                {
                    OParseColumn* pNewColumn = new OParseColumn(*aIter,m_aCaseEqual.isCaseSensitive());
                    pNewColumn->setName(rValue);
                    pNewColumn->setRealName(aColumnName);
                    m_aParameters->push_back(pNewColumn);
                }
                else if(aColumnName.getLength())// search in the tables for the right one
                {
                    OSQLTables::const_iterator aTableIter = m_aTables.end();
                    if(aTableRange.getLength())
                        aTableIter = m_aTables.find(aTableRange);
                    if(aTableIter == m_aTables.end())
                        aTableIter = m_aTables.begin();

                    for(;aTableIter != m_aTables.end();++aTableIter)
                    {
                        if(aTableIter->second.is())
                        {
                            Reference<XNameAccess> xColumns = aTableIter->second->getColumns();
                            if(xColumns.is() && xColumns->hasByName(aColumnName))
                            {
                                Reference<XPropertySet> xColumn;
                                xColumns->getByName(aColumnName) >>= xColumn;
                                OParseColumn* pNewColumn = new OParseColumn(xColumn,m_aCaseEqual.isCaseSensitive());
                                pNewColumn->setName(rValue);
                                pNewColumn->setRealName(aColumnName);
                                m_aParameters->push_back(pNewColumn);
                                break;
                            }
                        }
                    }
                }
                else
                {
                    ::rtl::OUString aNewColName(getUniqueColumnName(rValue));

                    OParseColumn* pColumn = new OParseColumn(aNewColName,
                                                            ::rtl::OUString(),
                                                            ::rtl::OUString(),
                                                            ColumnValue::NULLABLE_UNKNOWN,
                                                            0,
                                                            0,
                                                            DataType::VARCHAR,
                                                            sal_False,
                                                            sal_False,
                                                            m_xDatabaseMetaData.is() && m_xDatabaseMetaData->storesMixedCaseQuotedIdentifiers());
                    pColumn->setName(rValue);
                    pColumn->setRealName(rValue);
                    m_aParameters->push_back(pColumn);
                }
            }
        }
        else if (SQL_ISRULE(pParseNode,column_ref))// Column-Name (und TableRange):
            getColumnRange(pParseNode,aName,rValue);
        else
        {
            setORCriteriaPre();
            //  if (! aIteratorStatus.IsSuccessful()) return;

            traverseORCriteria(pParseNode);
            //  if (! aIteratorStatus.IsSuccessful()) return;

            setORCriteriaPost();
        }
    }

    // "set"-Routine aufrufen ...
    if (bCompareNull) {
        // nyi: was soll das? Vergleich auf NULL geht nur ueber IS NULL oder IS NOT NULL!
        setPredicate(aColumnName, aTableRange,ePredicateType, ::rtl::OUString(), ::rtl::OUString());
    } else {
        setPredicate(aColumnName, aTableRange, ePredicateType, rValue, aName);
    }

    // Fehler einfach weiterreichen ...
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseAssignments()
{


    OSL_ASSERT("OSQLParseTreeIterator::traverseAssignments: noch nicht implementiert");
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseAll()
{


    traverseTableNames();
    //  if (! aIteratorStatus.IsSuccessful()) return;

    if (m_eStatementType == SQL_STATEMENT_SELECT)
    {
        const OSQLParseNode* pSelectNode = m_pParseTree;
        traverseSelectColumnNames(pSelectNode);
//      if (! aIteratorStatus.IsSuccessful())
//          return;
        traverseOrderByColumnNames(pSelectNode);
        traverseGroupByColumnNames(pSelectNode);
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
    } else if (m_eStatementType == SQL_STATEMENT_CREATE_TABLE) {
        //0     |  1  |  2   |3|        4         |5
        //create table sc.foo ( a char(20), b char )
        const OSQLParseNode* pCreateNode = m_pParseTree->getChild(4);
        traverseCreateColumns(pCreateNode);
    }
}

// Dummy-Implementationen:

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setTableName(const ::rtl::OUString & rTableName, const ::rtl::OUString & rCatalogName, const ::rtl::OUString& rSchemaName,
                              const ::rtl::OUString & rTableRange)
{

    // nichts zu tun ...
    OTable * xTab=new OTable(NULL,sal_False,rTableName
                    ,::rtl::OUString::createFromAscii("Table")
                    ,::rtl::OUString::createFromAscii("New Created Table")
                    ,rSchemaName,rCatalogName);

    m_aTables[rTableRange] = xTab;
#ifdef SQL_TEST_PARSETREEITERATOR
    cout << "OSQLParseTreeIterator::setTableName"
         << (const char *) rTableName << ", "
         << (const char *) rCatalogName << ", "
         << (const char *) rSchemaName << ", "
         << (const char *) rTableRange
         << "\n";
#endif
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::appendColumns(::vos::ORef<OSQLColumns>& _rColumns,const ::rtl::OUString& _rTableAlias,const OSQLTable& _rTable)
{

    if (!_rTable.is())
        return;

    Reference<XNameAccess> xColumns = _rTable->getColumns();
    if ( !xColumns.is() )
        return;

    Sequence< ::rtl::OUString > aColNames =  xColumns->getElementNames();
    const ::rtl::OUString* pBegin = aColNames.getConstArray();
    const ::rtl::OUString* pEnd = pBegin + aColNames.getLength();

    sal_Bool bCase = m_xDatabaseMetaData.is() && m_xDatabaseMetaData->storesMixedCaseQuotedIdentifiers();
    for(;pBegin != pEnd;++pBegin)
    {

        ::rtl::OUString aName(getUniqueColumnName(*pBegin));
        Reference< XPropertySet > xColumn;
        if(xColumns->hasByName(*pBegin) && (xColumns->getByName(*pBegin) >>= xColumn) && xColumn.is())
        {
            OParseColumn* pColumn = new OParseColumn(aName
                                                ,   getString(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME)))
                                                ,   getString(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE)))
                                                ,   getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE)))
                                                ,   getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION)))
                                                ,   getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE)))
                                                ,   getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)))
                                                ,   getBOOL(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT)))
                                                ,   getBOOL(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISCURRENCY)))
                                                ,   bCase);

            pColumn->setTableName(_rTableAlias);
            pColumn->setRealName(*pBegin);
            Reference< XPropertySet> xCol = pColumn;
            _rColumns->push_back(xCol);
        }
        else if(m_pParser)
        {
            ::rtl::OUString sErrMsg = m_pParser->getContext().getErrorMessage(IParseContext::ERROR_INVALID_COLUMN);
            sErrMsg = sErrMsg.replaceAt(sErrMsg.indexOf('#'),1,*pBegin);
            sErrMsg = sErrMsg.replaceAt(sErrMsg.indexOf('#'),1,_rTableAlias);
            appendWarning(sErrMsg);
        }

    }
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setSelectColumnName(::vos::ORef<OSQLColumns>& _rColumns,const ::rtl::OUString & rColumnName,const ::rtl::OUString & rColumnAlias, const ::rtl::OUString & rTableRange,sal_Bool bFkt,sal_Int32 _nType)
{
    if(rColumnName.toChar() == '*' && !rTableRange.getLength())
    {   // Suche "uber alle vorkommenden Tabellen
        OSL_ENSURE(_rColumns == m_aSelectColumns,"Invalid columns used here!");
        for(ConstOSQLTablesIterator aIter = m_aTables.begin(); aIter != m_aTables.end();++aIter)
            appendColumns(_rColumns,aIter->first,aIter->second);
    }
    else if(rColumnName.toChar() == '*' && rTableRange.getLength())  // alle Columns aus dieser Tabelle
    {
        OSL_ENSURE(_rColumns == m_aSelectColumns,"Invalid columns used here!");
        ConstOSQLTablesIterator aFind = m_aTables.find(rTableRange);

        if(aFind == m_aTables.end())
        {
            ::rtl::OUString strExpression = rTableRange;
            strExpression += ::rtl::OUString::createFromAscii(".");
            strExpression += rColumnName;
        }
        else
            appendColumns(_rColumns,rTableRange,aFind->second);
    }
    else if( !rTableRange.getLength() )// ein Columnname existiert
    {
        if(!bFkt)
        {
            sal_uInt32 ncount = _rColumns->size();

            for(OSQLTablesIterator aIter = m_aTables.begin(); aIter != m_aTables.end(); ++aIter)
            {
                if (aIter->second.is())
                {
                    Reference<XNameAccess> xColumns = aIter->second->getColumns();

                    Reference< XPropertySet > xColumn;
                    if ( xColumns->hasByName(rColumnName) && (xColumns->getByName(rColumnName) >>= xColumn) )
                    {
                        OSL_ENSURE(xColumn.is(),"Column isn't a propertyset!");
                        ::rtl::OUString aNewColName(getUniqueColumnName(rColumnAlias));

                        Reference< XPropertySet> xCol;
                        OParseColumn* pColumn = new OParseColumn(xColumn,m_aCaseEqual.isCaseSensitive());
                        xCol = pColumn;
                        pColumn->setTableName(aIter->first);
                        pColumn->setName(aNewColName);
                        pColumn->setRealName(rColumnName);

                        _rColumns->push_back(xCol);
                        continue; // diese Column darf nur einmal vorkommen
                    }
                }
            }
            if ( ncount == _rColumns->size() )
            {
                ::rtl::OUString strExpression = rTableRange;
                if (strExpression.getLength())
                    strExpression += ::rtl::OUString::createFromAscii(".");
                strExpression += rColumnName;
            }
        }
        else
        {
            ::rtl::OUString aNewColName(getUniqueColumnName(rColumnAlias));

            OParseColumn* pColumn = new OParseColumn(aNewColName,::rtl::OUString(),::rtl::OUString(),
                ColumnValue::NULLABLE_UNKNOWN,0,0,_nType,sal_False,sal_False,m_aCaseEqual.isCaseSensitive());
            pColumn->setFunction(bFkt);
            pColumn->setRealName(rColumnName);

            Reference< XPropertySet> xCol = pColumn;
            _rColumns->push_back(xCol);
        }
    }
    else    // ColumnName und Tablename vorhanden
    {
        ConstOSQLTablesIterator aFind = m_aTables.find(rTableRange);

        sal_Bool bError = sal_False;
        if (aFind != m_aTables.end() && aFind->second.is())
        {
            if (bFkt)
            {
                ::rtl::OUString aNewColName(getUniqueColumnName(rColumnAlias));

                OParseColumn* pColumn = new OParseColumn(aNewColName,::rtl::OUString(),::rtl::OUString(),
                    ColumnValue::NULLABLE_UNKNOWN,0,0,_nType,sal_False,sal_False,m_aCaseEqual.isCaseSensitive());
                pColumn->setFunction(sal_True);
                pColumn->setRealName(rColumnName);
                pColumn->setTableName(aFind->first);

                Reference< XPropertySet> xCol = pColumn;
                _rColumns->push_back(xCol);
            }
            else
            {
                Reference< XPropertySet > xColumn;
                if (aFind->second->getColumns()->hasByName(rColumnName) && (aFind->second->getColumns()->getByName(rColumnName) >>= xColumn))
                {
                    ::rtl::OUString aNewColName(getUniqueColumnName(rColumnAlias));

                    OParseColumn* pColumn = new OParseColumn(xColumn,m_aCaseEqual.isCaseSensitive());
                    pColumn->setName(aNewColName);
                    pColumn->setRealName(rColumnName);
                    pColumn->setTableName(aFind->first);

                    Reference< XPropertySet> xCol = pColumn;
                    _rColumns->push_back(xCol);
                }
                else
                    bError = sal_True;
            }
        }
        else
            bError = sal_True;

        // Tabelle existiert nicht oder Feld nicht vorhanden
        if (bError)
        {
            ::rtl::OUString strExpression = rTableRange;
            if (strExpression.getLength())
                strExpression += ::rtl::OUString::createFromAscii(".");
            strExpression += rColumnName;

            ::rtl::OUString aNewColName(getUniqueColumnName(rColumnAlias));

            OParseColumn* pColumn = new OParseColumn(aNewColName,::rtl::OUString(),::rtl::OUString(),
                ColumnValue::NULLABLE_UNKNOWN,0,0,DataType::VARCHAR,sal_False,sal_False,m_aCaseEqual.isCaseSensitive());
            pColumn->setFunction(sal_True);


            Reference< XPropertySet> xCol = pColumn;
            _rColumns->push_back(xCol);
        }
    }
}
//-----------------------------------------------------------------------------
::rtl::OUString OSQLParseTreeIterator::getUniqueColumnName(const ::rtl::OUString & rColumnName) const
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
void OSQLParseTreeIterator::setOrderByColumnName(const ::rtl::OUString & rColumnName, const ::rtl::OUString & rTableRange,sal_Bool bAscending)
{
    Reference<XPropertySet> xColumn = findColumn(rColumnName,rTableRange);
    if ( xColumn.is() )
        m_aOrderColumns->push_back(new OOrderColumn(xColumn,m_aCaseEqual.isCaseSensitive(),bAscending));
    else
    {
        sal_Int32 nId = rColumnName.toInt32();
        if ( nId > 0 && nId < static_cast<sal_Int32>(m_aSelectColumns->size()) )
            m_aOrderColumns->push_back(new OOrderColumn((*m_aSelectColumns)[nId-1],m_aCaseEqual.isCaseSensitive(),bAscending));
    }

#ifdef SQL_TEST_PARSETREEITERATOR
    cout << "OSQLParseTreeIterator::setOrderByColumnName: "
         << (const char *) rColumnName << ", "
         << (const char *) rTableRange << ", "
         << (bAscending ? "sal_True" : "sal_False")
         << "\n";
#endif
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setGroupByColumnName(const ::rtl::OUString & rColumnName, const ::rtl::OUString & rTableRange)
{
    Reference<XPropertySet> xColumn = findColumn(rColumnName,rTableRange);
    if ( xColumn.is() )
        m_aGroupColumns->push_back(new OParseColumn(xColumn,m_aCaseEqual.isCaseSensitive()));
    else
    {
        sal_Int32 nId = rColumnName.toInt32();
        if ( nId > 0 && nId < static_cast<sal_Int32>(m_aSelectColumns->size()) )
            m_aGroupColumns->push_back(new OParseColumn((*m_aSelectColumns)[nId-1],m_aCaseEqual.isCaseSensitive()));
    }

#ifdef SQL_TEST_PARSETREEITERATOR
    cout << "OSQLParseTreeIterator::setOrderByColumnName: "
         << (const char *) rColumnName << ", "
         << (const char *) rTableRange << ", "
         << (bAscending ? "sal_True" : "sal_False")
         << "\n";
#endif
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setORCriteriaPre()
{

    // nichts zu tun ...

#ifdef SQL_TEST_PARSETREEITERATOR
    cout << "OSQLParseTreeIterator::setORCriteriaPre\n";
#endif
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setORCriteriaPost()
{

    // nichts zu tun ...

#ifdef SQL_TEST_PARSETREEITERATOR
    cout << "OSQLParseTreeIterator::setORCriteriaPost\n";
#endif
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setANDCriteriaPre()
{

    // nichts zu tun ...

#ifdef SQL_TEST_PARSETREEITERATOR
    cout << "OSQLParseTreeIterator::setANDCriteriaPre\n";
#endif
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setANDCriteriaPost()
{

    // nichts zu tun ...

#ifdef SQL_TEST_PARSETREEITERATOR
    cout << "OSQLParseTreeIterator::setANDCriteriaPost\n";
#endif
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setPredicate(const ::rtl::OUString & rColumnName,
                              const ::rtl::OUString & rTableRange,
                              sal_Int32 ePredicateType,
                              const ::rtl::OUString & rValue,
                              const ::rtl::OUString & rParameterName)
{

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
void OSQLParseTreeIterator::setAssign(const ::rtl::OUString & rColumnName,
                           const ::rtl::OUString & rValue, sal_Bool bsetNull,
                           const ::rtl::OUString & rParameterName)
{

    // nichts zu tun ...

#ifdef SQL_TEST_PARSETREEITERATOR
    cout << "OSQLParseTreeIterator::setAssign: "
         << (const char *) rColumnName << ", "
         << (const char *) rValue << ", "
         << (bsetNull ? "sal_True" : "sal_False") << ", "
         << (const char *) rParameterName
         << "\n";
#endif
}

//-----------------------------------------------------------------------------
const OSQLParseNode* OSQLParseTreeIterator::getWhereTree() const
{


    if (!m_pParseTree)
        return NULL;

    // Parse Tree analysieren (je nach Statement-Typ)
    // und Zeiger auf WHERE-Klausel setzen:
    OSQLParseNode * pWhereClause = NULL;
    if(getStatementType() == SQL_STATEMENT_SELECT)
    {
        OSL_ENSURE(m_pParseTree->count() >= 4,"ParseTreeIterator: Fehler im Parse Tree");
        OSQLParseNode * pTableExp = m_pParseTree->getChild(3);
        OSL_ENSURE(pTableExp != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
        OSL_ENSURE(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator: Fehler im Parse Tree");
        OSL_ENSURE(pTableExp->count() == 5,"OSQLParseTreeIterator: Fehler im Parse Tree");

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


    if (!m_pParseTree || getStatementType() != SQL_STATEMENT_SELECT)
        return NULL;

    // Parse Tree analysieren (je nach Statement-Typ)
    // und Zeiger auf ORDER-Klausel setzen:
    OSQLParseNode * pOrderClause = NULL;
    OSL_ENSURE(m_pParseTree->count() >= 4,"ParseTreeIterator: Fehler im Parse Tree");
    OSQLParseNode * pTableExp = m_pParseTree->getChild(3);
    OSL_ENSURE(pTableExp != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
    OSL_ENSURE(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator: Fehler im Parse Tree");
    OSL_ENSURE(pTableExp->count() == 5,"OSQLParseTreeIterator: Fehler im Parse Tree");

    pOrderClause = pTableExp->getChild(4);
    // Wenn es aber eine order_by ist, dann darf sie nicht leer sein:
    if(pOrderClause->count() != 3)
        pOrderClause = NULL;
    return pOrderClause;
}
//-----------------------------------------------------------------------------
const OSQLParseNode* OSQLParseTreeIterator::getGroupByTree() const
{
    if (!m_pParseTree || getStatementType() != SQL_STATEMENT_SELECT)
        return NULL;

    // Parse Tree analysieren (je nach Statement-Typ)
    // und Zeiger auf ORDER-Klausel setzen:
    OSQLParseNode * pGroupClause = NULL;
    OSL_ENSURE(m_pParseTree->count() >= 4,"ParseTreeIterator: Fehler im Parse Tree");
    OSQLParseNode * pTableExp = m_pParseTree->getChild(3);
    OSL_ENSURE(pTableExp != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
    OSL_ENSURE(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator: Fehler im Parse Tree");
    OSL_ENSURE(pTableExp->count() == 5,"OSQLParseTreeIterator: Fehler im Parse Tree");

    pGroupClause = pTableExp->getChild(2);
    // Wenn es aber eine order_by ist, dann darf sie nicht leer sein:
    if(pGroupClause->count() != 3)
        pGroupClause = NULL;
    return pGroupClause;
}
//-----------------------------------------------------------------------------
const OSQLParseNode* OSQLParseTreeIterator::getHavingTree() const
{
    if (!m_pParseTree || getStatementType() != SQL_STATEMENT_SELECT)
        return NULL;

    // Parse Tree analysieren (je nach Statement-Typ)
    // und Zeiger auf ORDER-Klausel setzen:
    OSQLParseNode * pHavingClause = NULL;
    OSL_ENSURE(m_pParseTree->count() >= 4,"ParseTreeIterator: Fehler im Parse Tree");
    OSQLParseNode * pTableExp = m_pParseTree->getChild(3);
    OSL_ENSURE(pTableExp != NULL,"OSQLParseTreeIterator: Fehler im Parse Tree");
    OSL_ENSURE(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator: Fehler im Parse Tree");
    OSL_ENSURE(pTableExp->count() == 5,"OSQLParseTreeIterator: Fehler im Parse Tree");

    pHavingClause = pTableExp->getChild(3);
    // Wenn es aber eine order_by ist, dann darf sie nicht leer sein:
    if(pHavingClause->count() < 1)
        pHavingClause = NULL;
    return pHavingClause;
}
// -----------------------------------------------------------------------------
sal_Bool OSQLParseTreeIterator::isTableNode(const OSQLParseNode* _pTableNode) const
{
    return _pTableNode && (SQL_ISRULE(_pTableNode,catalog_name) ||
                           SQL_ISRULE(_pTableNode,schema_name)  ||
                           SQL_ISRULE(_pTableNode,table_name));
}
// -----------------------------------------------------------------------------
void OSQLParseTreeIterator::appendWarning(const ::rtl::OUString& _sErrMsg)
{
    OSL_ENSURE(m_pParser,"This should normally be checked outside!");
    if(m_pParser)
    {
        if(m_aWarning.Message.getLength())
        {
            SQLWarning aWarning = m_aWarning;
            while(aWarning.NextException.hasValue())
                aWarning.NextException >>= aWarning;
            aWarning.NextException <<= SQLWarning(_sErrMsg,NULL,OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HY0000),1000,Any());
        }
        else
            m_aWarning = SQLWarning(_sErrMsg,NULL,OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HY0000),1000,Any());
    }
}
// -----------------------------------------------------------------------------
const OSQLParseNode* OSQLParseTreeIterator::getSimpleWhereTree() const
{
    const OSQLParseNode* pNode = getWhereTree();
    return pNode ? pNode->getChild(1) : NULL;
}
// -----------------------------------------------------------------------------
const OSQLParseNode* OSQLParseTreeIterator::getSimpleOrderTree() const
{
    const OSQLParseNode* pNode = getOrderTree();
    return pNode ? pNode->getChild(2) : NULL;
}
// -----------------------------------------------------------------------------
const OSQLParseNode* OSQLParseTreeIterator::getSimpleGroupByTree() const
{
    const OSQLParseNode* pNode = getGroupByTree();
    return pNode ? pNode->getChild(2) : NULL;
}
// -----------------------------------------------------------------------------
const OSQLParseNode* OSQLParseTreeIterator::getSimpleHavingTree() const
{
    const OSQLParseNode* pNode = getHavingTree();
    return pNode ? pNode->getChild(1) : NULL;
}
// -----------------------------------------------------------------------------
Reference< XPropertySet > OSQLParseTreeIterator::findColumn(const ::rtl::OUString & rColumnName, const ::rtl::OUString & rTableRange)
{
    Reference< XPropertySet > xColumn;
    if ( rTableRange.getLength() )
    {
        ConstOSQLTablesIterator aFind = m_aTables.find(rTableRange);

        if ( aFind != m_aTables.end()
            && aFind->second.is()
            && aFind->second->getColumns().is()
            && aFind->second->getColumns()->hasByName(rColumnName) )
            aFind->second->getColumns()->getByName(rColumnName) >>= xColumn;
    }
    if ( !xColumn.is() )
    {
        OSQLTablesIterator aEnd = m_aTables.end();
        for(OSQLTablesIterator aIter = m_aTables.begin(); aIter != aEnd; ++aIter)
        {
            if ( aIter->second.is() )
            {
                Reference<XNameAccess> xColumns = aIter->second->getColumns();
                if( xColumns.is() && xColumns->hasByName(rColumnName) && (xColumns->getByName(rColumnName) >>= xColumn) )
                {
                    OSL_ENSURE(xColumn.is(),"Column isn't a propertyset!");
                    break; // diese Column darf nur einmal vorkommen
                }
            }
        }
    }
    return xColumn;
}
// -----------------------------------------------------------------------------
