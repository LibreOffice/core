/*************************************************************************
 *
 *  $RCSfile: sqliterator.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-23 14:54:11 $
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
#define _CONNECTIVITY_PARSE_SQLITERATOR_HXX_

//#ifndef _SVTOOLS_HASHCONT_HXX //autogen
//#include <svtools/hashcont.hxx>
//#endif
#ifndef _CONNECTIVITY_SQLNODE_HXX
#include "connectivity/sqlnode.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLWARNING_HPP_
#include <com/sun/star/sdbc/SQLWarning.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _MAP_
#include <map>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

namespace connectivity
{

    class OSQLParseNode;


    enum OSQLStatementType {
        SQL_STATEMENT_UNKNOWN,
        SQL_STATEMENT_SELECT,
        SQL_STATEMENT_INSERT,
        SQL_STATEMENT_UPDATE,
        SQL_STATEMENT_DELETE,
        SQL_STATEMENT_ODBC_CALL,
        SQL_STATEMENT_SELECT_COUNT
    };

    enum OSQLPredicateType {
        SQL_PRED_EQUAL = 'a',       // Als sichtbare ASCII-Zeichen, damit man
        SQL_PRED_NOTEQUAL,          // den Predicate Type auch in Strings
        SQL_PRED_LESS,              // speichern kann.
        SQL_PRED_LESSOREQUAL,
        SQL_PRED_GREATER,
        SQL_PRED_GREATEROREQUAL,
        SQL_PRED_LIKE,
        SQL_PRED_ISNULL,
        SQL_PRED_ISNOTNULL,
        SQL_PRED_NOTLIKE
    };

    //==================================================================
    // SbaParseIteratorErrorInfo wird dem Call von aErrorHdl aus SbaParseIterator "ubergeben
    // nErrorCode enth"alt eine Zusatzinformation "uber den Fehler
    // 1    ->      Tabelle nicht gefunden
    // 2    ->      Spalte nicht gefunden
    //==================================================================
    struct OSQLParseIteratorErrorInfo
    {
        sal_uInt16      nErrorCode;     // 1 == Tabelle nicht gefunden, 2 == Spalte nicht gefunden
        ::rtl::OUString     aExpression;    // der Teil-Ausdruck, der das Problem verursacht hat (kann leer sein)
    };

    #define RET_CONTINUE    1   // Parsevorgang fortsetzen
    #define RET_HANDLED     2   // der Fehler wurde schon behandelt, das Parsen soll (mit Status auf Success) abgebrochen werden
    #define RET_BREAK       3   // Abbrechen, Status-Fehlercode setzen

    class OSQLParseTreeIterator
    {
    private:
        ::com::sun::star::sdbc::SQLWarning  m_aWarning;         // conatins the error while iterating through the statement
        const OSQLParseNode*                m_pParseTree;       // aktueller ParseTree
        const OSQLParser*                   m_pParser;          // if set used for general error messages from the context
        OSQLStatementType                   m_eStatementType;   // Art des Statements
        OSQLTables                          m_aTables;          // Alle Tabellen die im ParseTree und bei der Connection gefunden wurden
        ::vos::ORef<OSQLColumns>            m_aSelectColumns;   // alle Spalten aus dem Select-Clause
        ::comphelper::UStringMixEqual       m_aCaseEqual;

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>     m_xTables;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>    m_xDatabaseMetaData;

        void appendWarning(const ::rtl::OUString& _sErrMsg); // append warnings if m_pParser is set
        // F"ugt eine Tabelle in die Map ein
        void                traverseOneTableName(const OSQLParseNode * pTableName, const ::rtl::OUString & rTableRange);
        void                traverseORCriteria(OSQLParseNode * pSearchCondition);
        void                traverseANDCriteria(OSQLParseNode * pSearchCondition);
        void                traverseOnePredicate(
                                                OSQLParseNode * pColumnRef,
                                                OSQLPredicateType ePredicateType,
                                                ::rtl::OUString& aValue,
                                                sal_Bool bCompareNull,
                                                OSQLParseNode * pParameter);
        OSQLParseNode *     getTableRef(OSQLParseNode *pTableRef,::rtl::OUString& aTableRange);
        OSQLParseNode *     getQualified_join(OSQLParseNode *pTableRef,::rtl::OUString& aTableRange);
        void                getSelect_statement(OSQLParseNode *pSelect);
        ::rtl::OUString     getUniqueColumnName(const ::rtl::OUString & rColumnName)    const;

      protected:
        void setSelectColumnName(const ::rtl::OUString & rColumnName,const ::rtl::OUString & rColumnAlias, const ::rtl::OUString & rTableRange,sal_Bool bFkt=sal_False);
        void appendColumns(const ::rtl::OUString& _rTableAlias,const OSQLTable& _rTable);
        // Weitere Member-Variable, die in den "set"-Funktionen zur
        // Verfuegung stehen sollen, koennen in der abgeleiteten Klasse
        // definiert werden und z. B. in deren Konstruktor initialisiert
        // bzw. nach Benutzung der "traverse"-Routinen mit Hilfe weiterer
        // Funktionen abgefragt werden.


      public:
        OSQLParseTreeIterator();
        OSQLParseTreeIterator(const OSQLParseTreeIterator & rIter);
        OSQLParseTreeIterator(  const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xTableSupplier ,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _xDatabaseMetaData,
                                const OSQLParseNode* pRoot,
                                const OSQLParser* _pParser = NULL);
        ~OSQLParseTreeIterator();

        void dispose();
        sal_Bool isCaseSensitive() const { return m_aCaseEqual.isCaseSensitive(); }
        // Der zu analysierende/zu traversierende Parse Tree:
        // bei "Ubergabe von NULL wird der aktuelle Parsetree gel"oscht und der Fehlerstatus gecleared
        void setParseTree(const OSQLParseNode * pNewParseTree);
        void setParser(const OSQLParser* _pParser) { m_pParser = _pParser; }
        const OSQLParseNode * getParseTree() const { return m_pParseTree; };

        // Teilbaueme bei einem select statement
        const OSQLParseNode* getWhereTree() const;
        const OSQLParseNode* getOrderTree() const;
        const OSQLParseNode* getGroupByTree() const;
        const OSQLParseNode* getHavingTree() const;

        ::com::sun::star::sdbc::SQLWarning  getWarning() const { return m_aWarning; }
        // Statement-Typ (wird bereits in setParseTree gesetzt):
        OSQLStatementType getStatementType() const { return m_eStatementType; }

        // Die "traverse"-Routinen durchlaufen bestimmte Teile des Parse Tree
        // und rufen fuer die erkannten Konstrukte die virtuellen "set"-Routinen
        // auf und uebergeben diesen die erkannten Informationen als Parameter.
        //
        // Der Parse Tree muss in einer bestimmten Form vorliegen:
        //      SELECT [<tablerange>.]<columnname>, [<tablerange>.]<columnname>, ...
        //        FROM <tablename> [<tablerange>], <tablename> [<tablerange>], ...
        //       WHERE (
        //                  <predicate>
        //              AND <predicate>
        //              ...
        //             )
        //          OR (
        //              ...
        //             )
        //          ...
        //       ORDER BY <columname>, <columnname>, ...
        //
        // (die Klammern sind optional bzw. zusaetzliche Klammern stoeren nicht, aber
        // es duerfen nur mehrere AND-Bedingungen, die ihrerseits mit OR verknuepft sind,
        // auftreten).
        //
        //
        // <predicate> kann sein:
        //          [<tablerange>.]<columnname> <operator> <value>
        //          [<tablerange>.]<columnname> LIKE <value>
        //          [<tablerange>.]<columnname> NOT LIKE <value>
        //          [<tablerange>.]<columnname> IS NULL
        //          [<tablerange>.]<columnname> IS NOT NULL
        //
        // <operator> kann sein:
        //          "="
        //          "<>"
        //          "<"
        //          "<="
        //          ">"
        //          ">="
        //
        // <value> kann auch ein Parameter sein, in diesem Fall enthaelt
        // das Argument "ParameterName" der "set"-Funktionen den Namen des Parameters
        // (ohne fuehrenden Doppelpunkt) bzw. "?" bei unbenannten Parametern.
        //
        // <columnname> in der Select-Klausel kann auch "*" oder "COUNT(*)" sein.
        //
        //
        // Wenn das Statement NICHT diese Form hat, oder wenn eine der "set"-Routinen
        // den IteratorStatus != IsSuccessful() hinterlaesst, wird die weitere Analyse
        // des Parse Tree abgebrochen. Ansonsten liefert "Status().IsSuccessful() == TRUE".

        void traverseTableNames();
        virtual void setTableName(const ::rtl::OUString & rTableName, const ::rtl::OUString & rDBName, const ::rtl::OUString& rOwner,
                                  const ::rtl::OUString & rTableRange);
        // [TableName enthaelt immer einen Namen, TableRange ist, falls angegeben, die "Range"-
        // Variable (eine Art Alias-Name fuer den TableName), falls nicht angegeben, identisch
        // zum TableName. SchemaName ist leer, wenn nicht angegeben.]

        void traverseSelectColumnNames(const OSQLParseNode* pSelectNode);
        // [TableRange kann leer sein, wenn nicht angegeben]

        void traverseOrderByColumnNames(const OSQLParseNode* pSelectNode);
        virtual void setOrderByColumnName(const ::rtl::OUString & rColumnName, const ::rtl::OUString & rTableRange, sal_Bool bAscending);
        // [TableRange kann leer sein, wenn nicht angegeben]

        // Bei Selektionskriterien werden (selbst bei einem einfachen Praedikat)
        // folgende "set"-Funktionen in der angegebenen Reihenfolge aufgerufen:
        //
        //          setORCriteriaPre
        //          |
        //          |   setANDCriteriaPre
        //          |   |   setPredicate
        //          |   |   [weitere setPredicate-Aufrufe] ...
        //          |   setANDCriteriaPost
        //          |
        //          |   ... [weitere setANDCriteriaPre/Post-Paare und darin setPredicate-Aufrufe]
        //          |
        //          setORCriteriaPost
        //
        // Die AND-Verknuepfungen sind also implizit ODER-Verknuepft. setORCriteriaPre und
        // setORCriteriaPost werden jeweils nur ein einziges Mal aufgerufen (sind also
        // eigentlich ziemlich ueberfluessig, da man diese Aktionen auch vor bzw. nach dem
        // traverse-Aufruf erledigen kann)!
        //
        void traverseSelectionCriteria(const OSQLParseNode* pSelectNode);
        virtual void setORCriteriaPre();
        virtual void setORCriteriaPost();
        virtual void setANDCriteriaPre();
        virtual void setANDCriteriaPost();
        virtual void setPredicate(const ::rtl::OUString & rColumnName,
                                  const ::rtl::OUString & rTableRange,
                                  OSQLPredicateType ePredicateType,
                                  const ::rtl::OUString & rValue,
                                  const ::rtl::OUString & rParameterName);


        // Erweiterung auf UPDATE- und INSERT-Statement ... (nyi):
        void traverseAssignments();
        virtual void setAssign(const ::rtl::OUString & rColumnName,
                               const ::rtl::OUString & rValue, sal_Bool bsetNull,
                               const ::rtl::OUString & rParameterName);

        // Alle "traverse"-Routinen hintereinander aufrufen. Je nach Statement-Typ:
        // Bei UPDATE und INSERT-Statement nur traverseTableNames und traverseAssignments,
        // bei DELETE nur traverseTableNames und bei SELECT-Statement
        // in der Reihenfolge: traverseTableNames, traverseSelectColumnNames,
        // traverseOrderByColumnNames, traverseSelectionCriteria.
        // Bricht bei irgendwelchen Fehlern sofort ab und liefert entsprechenden
        // Status.
        void traverseAll();

        // Die TableRangeMap enth"alt alle Tabellen unter dem zugeh"origen Rangenamen der zuerst gefunden wird
        const OSQLTables& getTables() const { return m_aTables;};

        ::vos::ORef<OSQLColumns> getSelectColumns() const { return m_aSelectColumns;}
        // gibt den Aliasnamen der Column zur"uck, Leer falls nicht vorhanden
        ::rtl::OUString getColumnAlias(const OSQLParseNode* pDerivedColumn) const;
        // gibt den Columnnamen und die Tablerange (falls vorhanden) zur"uck
        void getColumnRange(const OSQLParseNode* pColumnRef,::rtl::OUString &rColumnName,::rtl::OUString &rTableRange) const;

        // Ermittelt fuer eine Funktion, Spalten den zugehoeren TableRange,
        // wenn nicht eindeutig, dann leer
        sal_Bool getColumnTableRange(const OSQLParseNode* pNode, ::rtl::OUString &rTableRange) const;

        // return true when the tableNode is a rule like catalog_name, schema_name or table_name
        sal_Bool isTableNode(const OSQLParseNode* _pTableNode) const;
    };
}

#endif // _CONNECTIVITY_PARSE_SQLITERATOR_HXX_

