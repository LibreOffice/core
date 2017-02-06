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
#ifndef INCLUDED_CONNECTIVITY_SQLNODE_HXX
#define INCLUDED_CONNECTIVITY_SQLNODE_HXX

#include <connectivity/dbtoolsdllapi.hxx>
#include <connectivity/dbmetadata.hxx>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <functional>
#include <memory>
#include <set>
#include <vector>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace beans
            {
                class XPropertySet;
            }
            namespace util
            {
                class XNumberFormatter;
            }
            namespace container
            {
                class XNameAccess;
            }
        }
    }
}

#define ORDER_BY_CHILD_POS  5
#define TABLE_EXPRESSION_CHILD_COUNT    9

namespace connectivity
{
    class OSQLParser;
    class OSQLParseNode;
    class IParseContext;

    typedef ::std::vector< OSQLParseNode* >                  OSQLParseNodes;

    enum class SQLNodeType { Rule, ListRule, CommaListRule,
                         Keyword, Comparison, Name,
                         String, IntNum, ApproxNum,
                         Equal, Less, Great, LessEq, GreatEq, NotEqual,
                         Punctuation, AMMSC, AccessDate, Date, Concat};

    typedef ::std::set< OUString >   QueryNameSet;

    //= SQLParseNodeParameter

    struct OOO_DLLPUBLIC_DBTOOLS SQLParseNodeParameter
    {
        const css::lang::Locale&                              rLocale;
        ::dbtools::DatabaseMetaData                           aMetaData;
        OSQLParser*                                           pParser;
        std::shared_ptr< QueryNameSet >                       pSubQueryHistory;
        css::uno::Reference< css::util::XNumberFormatter >    xFormatter;
        css::uno::Reference< css::beans::XPropertySet >       xField;
        OUString                                              sPredicateTableAlias;
        css::uno::Reference< css::container::XNameAccess >    xQueries;  // see bParseToSDBCLevel
        const IParseContext&                                  m_rContext;
        sal_Char            cDecSep;
        bool                bQuote                      : 1;    /// should we quote identifiers?
        bool                bInternational              : 1;    /// should we internationalize keywords and placeholders?
        bool                bPredicate                  : 1;    /// are we going to parse a mere predicate?
        bool                bParseToSDBCLevel           : 1;    /// should we create an SDBC-level statement (e.g. with substituted sub queries)?

        SQLParseNodeParameter(
            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
            const css::uno::Reference< css::util::XNumberFormatter >& _xFormatter,
            const css::uno::Reference< css::beans::XPropertySet >& _xField,
            const OUString &_sPredicateTableAlias,
            const css::lang::Locale& _rLocale,
            const IParseContext* _pContext,
            bool _bIntl,
            bool _bQuote,
            sal_Char _cDecSep,
            bool _bPredicate,
            bool _bParseToSDBC
        );
        ~SQLParseNodeParameter();
    };

    //= OSQLParseNode

    class OOO_DLLPUBLIC_DBTOOLS OSQLParseNode
    {
        friend class OSQLParser;

        OSQLParseNodes                  m_aChildren;
        OSQLParseNode*                  m_pParent;      // pParent for reverse linkage in the tree
        OUString                 m_aNodeValue;   // token name, or empty in case of rules,
                                                        // or OUString in case of
                                                        // OUString, INT, etc.
        SQLNodeType                     m_eNodeType;    // see above
        sal_uInt32                      m_nNodeID;      // Rule ID (if IsRule())
                                                        // or Token ID (if !IsRule())
                                            // Rule IDs and Token IDs can't
                                            // be distinguished by their values,
                                            // IsRule has to be used for that!
    public:
        enum Rule
        {
            UNKNOWN_RULE = 0,  // ID indicating that a node is no rule with a matching Rule-enum value (see getKnownRuleID)
                               // we make sure it is 0 so that it is the default-constructor value of this enum
                               // and std::map<foo,Rule>::operator[](bar) default-inserts UNKNOWN_RULE rather than select_statement (!)
            select_statement,
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
            comparison,
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
            sql_not,
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
            selection,
            fold,
            char_substring_fct,
            factor,
            base_table_def,
            base_table_element_commalist,
            data_type,
            column_def,
            table_node,
            as_clause,
            opt_as,
            op_column_commalist,
            table_primary_as_range_column,
            datetime_primary,
            concatenation,
            char_factor,
            bit_value_fct,
            comparison_predicate_part_2,
            parenthesized_boolean_value_expression,
            character_string_type,
            other_like_predicate_part_2,
            between_predicate_part_2,
            null_predicate_part_2,
            cast_spec,
            window_function,
            rule_count             // last value
        };

        // must be ascii encoding for the value
        OSQLParseNode(const sal_Char* _pValueStr,
                      SQLNodeType _eNodeType,
                      sal_uInt32 _nNodeID = 0);

        OSQLParseNode(const OString& _rValue,
                      SQLNodeType eNewNodeType,
                      sal_uInt32 nNewNodeID=0);

        OSQLParseNode(const OUString& _rValue,
                      SQLNodeType _eNodeType,
                      sal_uInt32 _nNodeID = 0);

            // copies the respective ParseNode
        OSQLParseNode(const OSQLParseNode& rParseNode);
        OSQLParseNode& operator=(const OSQLParseNode& rParseNode);

        bool operator==(OSQLParseNode& rParseNode) const;

        // destructor destructs the tree recursively
        virtual ~OSQLParseNode();

        OSQLParseNode* getParent() const {return m_pParent;};

        void setParent(OSQLParseNode* pParseNode) {m_pParent = pParseNode;};

        size_t count() const {return m_aChildren.size();};
        inline OSQLParseNode* getChild(sal_uInt32 nPos) const;

        void append(OSQLParseNode* pNewSubTree);
        void insert(sal_uInt32 nPos, OSQLParseNode* pNewSubTree);

        OSQLParseNode* replace(OSQLParseNode* pOldSubTree, OSQLParseNode* pNewSubTree);

        OSQLParseNode* removeAt(sal_uInt32 nPos);

        void replaceNodeValue(const OUString& rTableAlias,const OUString& rColumnName);

        /** parses the node to a string which can be passed to a driver's connection for execution

            Any particles of the parse tree which represent application-level features - such
            as queries appearing in the FROM part - are substituted, so that the resulting statement can
            be executed at an SDBC-level connection.

            @param  _out_rString
                is an output parameter taking the resulting SQL statement

            @param  _rxConnection
                the connection relative to which to parse. This must be an SDB-level connection (e.g.
                support the XQueriesSupplier interface) for the method to be able to do all necessary
                substitutions.

            @param _rParser
                the SQLParser used to create the node. This is needed in case we need to parse
                sub queries which are present in the SQL statement - those sub queries need to be parsed,
                too, to check whether they contain nested sub queries.

            @param _pErrorHolder
                takes the error which occurred while generating the statement, if any. Might be <NULL/>,
                in this case the error is not reported back, and can only be recognized by examing the
                return value.

            @return
                <TRUE/> if and only if the parsing was successful.<br/>

                Currently, there's only one condition how this method can fail: If it contains a nested
                query which causes a cycle. E.g., consider a statement <code>SELECT * from "foo"</code>,
                where <code>foo</code> is a query defined as <code>SELECT * FROM "bar"</code>, where
                <code>bar</code> is defined as <code>SELECT * FROM "foo"</code>. This statement obviously
                cannot be parsed to an executable statement.

                If this method returns <FALSE/>, you're encouraged to check and handle the error in
                <arg>_pErrorHolder</arg>.
        */
        bool parseNodeToExecutableStatement( OUString& _out_rString,
            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
            OSQLParser& _rParser,
            css::sdbc::SQLException* _pErrorHolder ) const;

        void parseNodeToStr(OUString& rString,
                            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
                            const IParseContext* pContext = nullptr,
                            bool _bIntl = false,
                            bool _bQuote= true) const;

        // quoted and internationalised
        void parseNodeToPredicateStr(OUString& rString,
                                     const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
                                     const css::uno::Reference< css::util::XNumberFormatter > & xFormatter,
                                     const css::lang::Locale& rIntl,
                                     sal_Char _cDec,
                                     const IParseContext* pContext = nullptr ) const;

        void parseNodeToPredicateStr(OUString& rString,
                                     const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
                                     const css::uno::Reference< css::util::XNumberFormatter > & xFormatter,
                                     const css::uno::Reference< css::beans::XPropertySet > & _xField,
                                     const OUString &_sTableAlias,
                                     const css::lang::Locale& rIntl,
                                     sal_Char _cDec,
                                     const IParseContext* pContext = nullptr ) const;

        OSQLParseNode* getByRule(OSQLParseNode::Rule eRule) const;

#if OSL_DEBUG_LEVEL > 1
        // shows the ParseTree with tabs and linefeeds
        void showParseTree( OUString& rString ) const;
        void showParseTree( OUStringBuffer& _inout_rBuf, sal_uInt32 nLevel ) const;
#endif

        SQLNodeType getNodeType() const {return m_eNodeType;};

        // RuleId returns the RuleID of the node's rule (only if IsRule())
        sal_uInt32 getRuleID() const {return m_nNodeID;}

        /** returns the ID of the rule represented by the node
            If the node does not represent a rule, UNKNOWN_RULE is returned
        */
        Rule getKnownRuleID() const;

            // returns the TokenId of the node's token (only if !isRule())
        sal_uInt32 getTokenID() const {return m_nNodeID;}

            // IsRule tests whether a node is a rule (NonTerminal)
            // ATTENTION: rules can be leaves, for example empty lists
        bool isRule() const
            { return (m_eNodeType == SQLNodeType::Rule) || (m_eNodeType == SQLNodeType::ListRule)
                || (m_eNodeType == SQLNodeType::CommaListRule);}

            // IsToken tests whether a Node is a Token (Terminal but not a rule)
        bool isToken() const {return !isRule();}

        const OUString& getTokenValue() const {return m_aNodeValue;}

        bool isLeaf() const {return m_aChildren.empty();}

        // negate only a searchcondition, any other rule could cause a gpf
        static void negateSearchCondition(OSQLParseNode*& pSearchCondition, bool bNegate=false);

        // normalize a logic form
        // e.q. (a or b) and (c or d) <=> a and c or a and d or b and c or b and d
        static void disjunctiveNormalForm(OSQLParseNode*& pSearchCondition);

        //   Simplifies logic expressions
        // a and a        = a
        // a or a         = a
        // a and ( a + b) = a
        // a or a and b   = a
        static void absorptions(OSQLParseNode*& pSearchCondition);

        // erase unnecessary braces
        static void eraseBraces(OSQLParseNode*& pSearchCondition);

        // makes the logic formula a little smaller
        static void compress(OSQLParseNode*& pSearchCondition);
        // return the catalog, schema and tablename form this node
        // _pTableNode must be a rule of that above or a SQL_TOKEN_NAME
        static bool getTableComponents(const OSQLParseNode* _pTableNode,
                                            css::uno::Any &_rCatalog,
                                            OUString &_rSchema,
                                            OUString &_rTable,
                                            const css::uno::Reference< css::sdbc::XDatabaseMetaData >& _xMetaData);

        // substitute all occurrences of :var or [name] into the dynamic parameter ?
        // _pNode will be modified if parameters exists
        static void substituteParameterNames(OSQLParseNode* _pNode);

        /** return a table range when it exists.
        */
        static OUString getTableRange(const OSQLParseNode* _pTableRef);

    protected:
        // ParseNodeToStr concatenates all Tokens (leaves) of the ParseNodes.
        void parseNodeToStr(OUString& rString,
                            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
                            const css::uno::Reference< css::util::XNumberFormatter > & xFormatter,
                            const css::uno::Reference< css::beans::XPropertySet > & _xField,
                            const OUString &_sPredicateTableAlias,
                            const css::lang::Locale& rIntl,
                            const IParseContext* pContext,
                            bool _bIntl,
                            bool _bQuote,
                            sal_Char _cDecSep,
                            bool _bPredicate) const;

    private:
        void impl_parseNodeToString_throw( OUStringBuffer& rString, const SQLParseNodeParameter& rParam, bool bSimple=true ) const;
        void impl_parseLikeNodeToString_throw( OUStringBuffer& rString, const SQLParseNodeParameter& rParam, bool bSimple=true ) const;
        void impl_parseTableRangeNodeToString_throw( OUStringBuffer& rString, const SQLParseNodeParameter& rParam ) const;

        /** parses a table_name node into a SQL statement particle.
            @return
                <TRUE/> if and only if parsing was successful, <FALSE/> if default handling should
                be applied.
        */
        bool impl_parseTableNameNodeToString_throw( OUStringBuffer& rString, const SQLParseNodeParameter& rParam ) const;

        bool addDateValue(OUStringBuffer& rString, const SQLParseNodeParameter& rParam) const;
        static OUString convertDateTimeString(const SQLParseNodeParameter& rParam, const OUString& rString);
        static OUString convertDateString(const SQLParseNodeParameter& rParam, const OUString& rString);
        static OUString convertTimeString(const SQLParseNodeParameter& rParam, const OUString& rString);
        void parseLeaf(OUStringBuffer& rString, const SQLParseNodeParameter& rParam) const;
    };

    inline OSQLParseNode* OSQLParseNode::getChild(sal_uInt32 nPos) const
    {
        assert(nPos < m_aChildren.size());

        return m_aChildren[nPos];
    }

    // utilities to query for a specific rule, token or punctuation
    #define SQL_ISRULE(pParseNode, eRule)   ((pParseNode)->isRule() && (pParseNode)->getRuleID() == OSQLParser::RuleID(OSQLParseNode::eRule))
    #define SQL_ISRULEOR2(pParseNode, e1, e2)  ((pParseNode)->isRule() && ( \
                                                  (pParseNode)->getRuleID() == OSQLParser::RuleID(OSQLParseNode::e1) || \
                                                  (pParseNode)->getRuleID() == OSQLParser::RuleID(OSQLParseNode::e2)))
    #define SQL_ISRULEOR3(pParseNode, e1, e2, e3)  ((pParseNode)->isRule() && ( \
                                                      (pParseNode)->getRuleID() == OSQLParser::RuleID(OSQLParseNode::e1) || \
                                                      (pParseNode)->getRuleID() == OSQLParser::RuleID(OSQLParseNode::e2) || \
                                                      (pParseNode)->getRuleID() == OSQLParser::RuleID(OSQLParseNode::e3)))
    #define SQL_ISTOKEN(pParseNode, token) ((pParseNode)->isToken() && (pParseNode)->getTokenID() == SQL_TOKEN_##token)
    #define SQL_ISTOKENOR2(pParseNode, tok0, tok1) ((pParseNode)->isToken() &&  ( (pParseNode)->getTokenID() == SQL_TOKEN_##tok0 || (pParseNode)->getTokenID() == SQL_TOKEN_##tok1 ))
    #define SQL_ISTOKENOR3(pParseNode, tok0, tok1, tok2) ((pParseNode)->isToken() && ( (pParseNode)->getTokenID() == SQL_TOKEN_##tok0 || (pParseNode)->getTokenID() == SQL_TOKEN_##tok1 || (pParseNode)->getTokenID() == SQL_TOKEN_##tok2 ))
    #define SQL_ISPUNCTUATION(pParseNode, aString) ((pParseNode)->getNodeType() == SQLNodeType::Punctuation && (pParseNode)->getTokenValue() == (aString))
}

#endif // INCLUDED_CONNECTIVITY_SQLNODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
