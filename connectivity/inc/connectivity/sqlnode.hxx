/*************************************************************************
 *
 *  $RCSfile: sqlnode.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-20 16:46:48 $
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
#ifndef _CONNECTIVITY_SQLNODE_HXX
#define _CONNECTIVITY_SQLNODE_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef __SGI_STL_VECTOR
#include <vector>
#endif

// forward declarations
namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace sdbc
            {
                class XDatabaseMetaData;
            }
            namespace beans
            {
                class XPropertySet;
            }
            namespace util
            {
                class XNumberFormatter;
            }
        }
    }
}

namespace connectivity
{

    class OSQLParseNode;
    class OParseContext;

    typedef ::std::vector< OSQLParseNode* > OSQLParseNodes;

    enum SQLNodeType    {SQL_NODE_RULE, SQL_NODE_LISTRULE, SQL_NODE_COMMALISTRULE,
                         SQL_NODE_KEYWORD, SQL_NODE_COMPARISON, SQL_NODE_NAME,
                         SQL_NODE_STRING,   SQL_NODE_INTNUM, SQL_NODE_APPROXNUM,
                         SQL_NODE_EQUAL,SQL_NODE_LESS,SQL_NODE_GREAT,SQL_NODE_LESSEQ,SQL_NODE_GREATEQ,SQL_NODE_NOTEQUAL,
                         SQL_NODE_PUNCTUATION, SQL_NODE_AMMSC, SQL_NODE_ACCESS_DATE,SQL_NODE_DATE};

    //==========================================================================
    //= OSQLParseNode
    //==========================================================================
    class OSQLParseNode
    {
        friend class OSQLParser;

        OSQLParseNodes                  m_aChilds;
        OSQLParseNode*                  m_pParent;      // pParent fuer Reuckverkettung im Baum
        ::rtl::OUString                 m_aNodeValue;   // Token-Name oder leer bei Regeln oder ::rtl::OUString bei
                                                        // ::rtl::OUString, INT, usw. -Werten
        SQLNodeType                     m_eNodeType;    // s. o.
        sal_uInt32                      m_nNodeID;      // ::com::sun::star::chaos::Rule ID (bei IsRule()) oder Token ID (bei !IsRule())
                                            // ::com::sun::star::chaos::Rule IDs und Token IDs koennen nicht anhand des Wertes
                                            // unterschieden werden, dafuer ist IsRule() abzufragen!
    protected:
        struct SQLParseNodeParameter
        {
            const ::com::sun::star::lang::Locale&   rLocale;
            const ::rtl::OUString                   aIdentifierQuote;
            const ::rtl::OUString                   aCatalogSeparator;
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > xFormatter;
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xField;
            const OParseContext& rContext;
            sal_Char            cDecSep ;
            sal_Bool            bQuote : 1;
            sal_Bool            bInternational : 1;
            sal_Bool            bPredicate : 1;

            SQLParseNodeParameter(const ::rtl::OUString& _rIdentifierQuote,
                                  const ::rtl::OUString& _rCatalogSep,
                                  const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > & _xFormatter,
                                  const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & _xField,
                                  const ::com::sun::star::lang::Locale& _rIntl,
                                  const OParseContext* _pContext,
                                  sal_Bool _bIntl = sal_False,
                                  sal_Bool _bQuote= sal_True,
                                  sal_Char _cDecSep = '.',
                                  sal_Bool _bPredicate = sal_False);
        };

    public:
        enum Rule
        {
            select_statement = 0,
            table_exp,
            table_ref_commalist,
            table_ref,
            catalog_name,
            schema_name,
            table_name,
            opt_column_commalist,
            column_commalist,
            column_ref_commalist,
            column_ref,
            opt_order_by_clause,
            ordering_spec_commalist,
            ordering_spec,
            opt_asc_desc,
            where_clause,
            opt_where_clause,
            search_condition,
            comparison_predicate,
            between_predicate,
            like_predicate,
            opt_escape,
            test_for_null,
            scalar_exp_commalist,
            scalar_exp,
            parameter_ref,
            parameter,
            general_set_fct,
            range_variable,
            column,
            delete_statement_positioned,
            delete_statement_searched,
            update_statement_positioned,
            update_statement_searched,
            assignment_commalist,
            assignment,
            values_or_query_spec,
            insert_statement,
            insert_atom_commalist,
            insert_atom,
            predicate_check,
            from_clause,
            qualified_join,
            cross_union,
            select_sublist,
            derived_column,
            column_val,
            set_fct_spec,
            boolean_term,
            boolean_primary,
            num_value_exp,
            join_type,
            position_exp,
            extract_exp,
            length_exp,
            char_value_fct,
            odbc_call_spec,
            in_predicate,
            existence_test,
            unique_test,
            all_or_any_predicate,
            named_columns_join,
            join_condition,
            joined_table,
            boolean_factor,
            not,
            boolean_test,
            manipulative_statement,
            subquery,
            value_exp_commalist,
            odbc_fct_spec,
            union_statement,
            outer_join_type,
            char_value_exp,
            term,
            value_exp_primary,
            value_exp,
            rule_count                      // letzter_wert
        };

        // must be ascii encoding for the value
        OSQLParseNode(const sal_Char* _pValueStr,
                      SQLNodeType _eNodeType,
                      sal_uInt32 _nNodeID = 0);

        OSQLParseNode(const ::rtl::OString& _rValue,
                      SQLNodeType eNewNodeType,
                      sal_uInt32 nNewNodeID=0);

        OSQLParseNode(const sal_Unicode* _pValue,
                      SQLNodeType _eNodeType,
                      sal_uInt32 _nNodeID = 0);

        OSQLParseNode(const ::rtl::OUString& _rValue,
                      SQLNodeType _eNodeType,
                      sal_uInt32 _nNodeID = 0);

            // Kopiert den entsprechenden ParseNode
        OSQLParseNode(const OSQLParseNode& rParseNode);
        OSQLParseNode& operator=(const OSQLParseNode& rParseNode);

        sal_Bool operator==(OSQLParseNode& rParseNode) const;

        // Destruktor raeumt rekursiv den Baum ab
        virtual ~OSQLParseNode();

        // Parent gibt den Zeiger auf den Parent zurueck
        OSQLParseNode* getParent() const {return m_pParent;};

        // SetParent setzt den Parent-Zeiger eines ParseNodes
        void setParent(OSQLParseNode* pParseNode) {m_pParent = pParseNode;};

        // ChildCount liefert die Anzahl der Kinder eines Knotens
        sal_uInt32 count() const {return m_aChilds.size();};
        inline OSQLParseNode* getChild(sal_uInt32 nPos) const;

        void append(OSQLParseNode* pNewSubTree);
        void insert(sal_uInt32 nPos, OSQLParseNode* pNewSubTree);

        OSQLParseNode* replaceAt(sal_uInt32 nPos, OSQLParseNode* pNewSubTree);
        OSQLParseNode* replace(OSQLParseNode* pOldSubTree, OSQLParseNode* pNewSubTree);

        OSQLParseNode* removeAt(sal_uInt32 nPos);
        OSQLParseNode* remove(OSQLParseNode* pSubTree);

        void replaceNodeValue(const ::rtl::OUString& rTableAlias,const ::rtl::OUString& rColumnName);

        void parseNodeToStr(::rtl::OUString& rString,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > & xMeta,
                            const OParseContext* pContext = NULL,
                            sal_Bool _bIntl = sal_False,
                            sal_Bool _bQuote= sal_True) const;

        // quoted und internationalisert
        void parseNodeToPredicateStr(::rtl::OUString& rString,
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > & xMeta,
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > & xFormatter,
                                     const ::com::sun::star::lang::Locale& rIntl,
                                     sal_Char _cDec,
                                     const OParseContext* pContext = NULL ) const;

        void parseNodeToPredicateStr(::rtl::OUString& rString,
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > & xMeta,
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > & xFormatter,
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & _xField,
                                     const ::com::sun::star::lang::Locale& rIntl,
                                     sal_Char _cDec,
                                     const OParseContext* pContext = NULL ) const;

        OSQLParseNode* getByRule(OSQLParseNode::Rule eRule) const;

            // zeigt den ParseTree mit tabs und linefeeds
        void showParseTree(::rtl::OUString& rString, sal_uInt32 nLevel=0);

            // GetNodeType gibt den Knotentyp zurueck
        SQLNodeType getNodeType() const {return m_eNodeType;};

            // RuleId liefert die RuleId der Regel des Knotens (nur bei IsRule())
        sal_uInt32 getRuleID() const {return m_nNodeID;}

            // RuleId liefert die TokenId des Tokens des Knotens (nur bei ! IsRule())
        sal_uInt32 getTokenID() const {return m_nNodeID;}

            // IsRule testet ob ein Node eine Regel (NonTerminal) ist
            // Achtung : Regeln koenne auch Blaetter sein, z.B. leere Listen
        sal_Bool isRule() const
            { return (m_eNodeType == SQL_NODE_RULE) || (m_eNodeType == SQL_NODE_LISTRULE)
                || (m_eNodeType == SQL_NODE_COMMALISTRULE);}

            // IsToken testet ob ein Node ein Token (Terminal) ist
        sal_Bool isToken() const {return !isRule();} // ein Token ist keine Regel

                // TokenValue liefert den NodeValue eines Tokens
        const ::rtl::OUString& getTokenValue() const {return m_aNodeValue;}

            // SetTokenValue setzt den NodeValue
        void setTokenValue(const ::rtl::OUString& rString) {    if (isToken()) m_aNodeValue = rString;}

            // IsLeaf testet ob ein Node ein Blatt ist
        sal_Bool isLeaf() const {return m_aChilds.empty();}

        // negate only a searchcondition, any other rule could cause a gpf
        static void negateSearchCondition(OSQLParseNode*& pSearchCondition,sal_Bool bNegate=sal_False);

        // normalize a logic form
        // e.q. (a or b) and (c or d) <=> a and c or a and d or b and c or b and d
        static void disjunctiveNormalForm(OSQLParseNode*& pSearchCondition);

        //   Simplies logic expressions
        // a * a        = a
        // a + a        = a
        // a * ( a + b) = a
        // a + a * b    = a
        static void absorptions(OSQLParseNode*& pSearchCondition);

        // erase not nessary braces
        static void eraseBraces(OSQLParseNode*& pSearchCondition);

        // makes the logic formula a little more smaller
        static void compress(OSQLParseNode*& pSearchCondition);
        // return the catalog, schema and tablename form this node
        // _pTableNode must be a rule of that above or a SQL_TOKEN_NAME
        static sal_Bool getTableComponents(const OSQLParseNode* _pTableNode,
                                            ::com::sun::star::uno::Any &_rCatalog,
                                            ::rtl::OUString &_rSchema,
                                            ::rtl::OUString &_rTable);

    protected:
        // ParseNodeToStr konkateniert alle Token (Blaetter) des ParseNodes
        void parseNodeToStr(::rtl::OUString& rString,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > & xMeta,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > & xFormatter,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & _xField,
                            const ::com::sun::star::lang::Locale& rIntl,
                            const OParseContext* pContext,
                            sal_Bool _bIntl,
                            sal_Bool _bQuote,
                            sal_Char _cDecSep,
                            sal_Bool bPredicate) const;

        virtual void parseNodeToStr(::rtl::OUString& rString,
                                    const SQLParseNodeParameter& rParam) const;
    private:
        void likeNodeToStr(::rtl::OUString& rString,
                           const SQLParseNodeParameter& rParam) const;
        void tableRangeNodeToStr(::rtl::OUString& rString,
                                 const SQLParseNodeParameter& rParam) const;
        sal_Bool addDateValue(::rtl::OUString& rString, const SQLParseNodeParameter& rParam) const;
        ::rtl::OUString convertDateTimeString(const SQLParseNodeParameter& rParam, const ::rtl::OUString& rString) const;
        ::rtl::OUString convertDateString(const SQLParseNodeParameter& rParam, const ::rtl::OUString& rString) const;
        ::rtl::OUString convertTimeString(const SQLParseNodeParameter& rParam, const ::rtl::OUString& rString) const;
    };

    //-----------------------------------------------------------------------------
    inline OSQLParseNode* OSQLParseNode::getChild(sal_uInt32 nPos) const
    {
        OSL_ENSURE(nPos < m_aChilds.size(), "Invalid Position");
        return m_aChilds[nPos];
    }

    // Utility-Methoden zum Abfragen auf bestimmte Rules, Token oder Punctuation:
    #define SQL_ISRULE(pParseNode, eRule)   ((pParseNode)->isRule() && (pParseNode)->getRuleID() == OSQLParser::RuleID(OSQLParseNode::##eRule))
    #define SQL_ISTOKEN(pParseNode, token) ((pParseNode)->isToken() && (pParseNode)->getTokenID() == SQL_TOKEN_##token)
    #define SQL_ISPUNCTUATION(pParseNode, aString) ((pParseNode)->getNodeType() == SQL_NODE_PUNCTUATION && !(pParseNode)->getTokenValue().compareToAscii(aString))
}

#endif  //_CONNECTIVITY_SQLNODE_HXX
