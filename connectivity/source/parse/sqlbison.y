%{
//--------------------------------------------------------------------------
//
// $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/connectivity/source/parse/sqlbison.y,v 1.19 2001-03-12 18:20:35 fs Exp $
//
// Copyright 2000 Sun Microsystems, Inc. All Rights Reserved.
//
// First creation:
//	OJ
//
// Last change:
//	$Author: fs $ $Date: 2001-03-12 18:20:35 $ $Revision: 1.19 $
//
// Description:
//
//
//--------------------------------------------------------------------------

#ifndef _VECTOR_
#include <vector>
#endif

#ifndef _CONNECTIVITY_SQLNODE_HXX
#include <connectivity/sqlnode.hxx>
#endif
#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include <connectivity/sqlparse.hxx>
#endif
#ifndef _CONNECTIVITY_SQLINTERNALNODE_HXX
#include <internalnode.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATS_HPP_
#include <com/sun/star/util/XNumberFormats.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_KPARSETYPE_HPP_
#include <com/sun/star/i18n/KParseType.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_KPARSETOKENS_HPP_
#include <com/sun/star/i18n/KParseTokens.hpp>
#endif
#ifndef _CONNECTIVITY_SQLSCAN_HXX
#include "sqlscan.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include "connectivity/dbconversion.hxx"
#endif

static ::rtl::OUString aEmptyString;

static connectivity::OSQLInternalNode* newNode(const sal_Char* pNewValue,
							     const connectivity::SQLNodeType eNodeType,
								 const sal_uInt16 nNodeID = 0)
{

	return new connectivity::OSQLInternalNode(pNewValue, eNodeType, nNodeID);
}

static connectivity::OSQLInternalNode* newNode(const ::rtl::OString& _NewValue,
							    const connectivity::SQLNodeType eNodeType,
								const sal_uInt16 nNodeID = 0)
{

	return new connectivity::OSQLInternalNode(_NewValue, eNodeType, nNodeID);
}

static connectivity::OSQLInternalNode* newNode(const ::rtl::OUString& _NewValue,
							    const connectivity::SQLNodeType eNodeType,
								const sal_uInt16 nNodeID = 0)
{

	return new connectivity::OSQLInternalNode(_NewValue, eNodeType, nNodeID);
}


// yyi ist die interne Nr. der Regel, die gerade reduziert wird.
// Ueber die Mapping-Tabelle yyrmap wird daraus eine externe Regel-Nr.
#define SQL_NEW_RULE 			newNode(aEmptyString, SQL_NODE_RULE, yyr1[yyn])
#define SQL_NEW_LISTRULE 		newNode(aEmptyString, SQL_NODE_LISTRULE, yyr1[yyn])
#define SQL_NEW_COMMALISTRULE   newNode(aEmptyString, SQL_NODE_COMMALISTRULE, yyr1[yyn])


connectivity::OSQLParser* xxx_pGLOBAL_SQLPARSER;

#define YYERROR_VERBOSE

#define SQLyyerror(s)						\
{											\
	xxx_pGLOBAL_SQLPARSER->error(s);		\
}

using namespace connectivity;
#define SQLyylex xxx_pGLOBAL_SQLPARSER->SQLlex
%}
	/* symbolic tokens */

%union {
	connectivity::OSQLParseNode * pParseNode;
}
%type <pParseNode> '(' ')' ',' ':' ';' '?' '[' ']' '{' '}' '.'

%token <pParseNode> SQL_TOKEN_STRING SQL_TOKEN_ACCESS_DATE SQL_TOKEN_INT SQL_TOKEN_REAL_NUM
%token <pParseNode> SQL_TOKEN_INTNUM SQL_TOKEN_APPROXNUM SQL_TOKEN_NOT SQL_TOKEN_NAME

	/* operators */
%left <pParseNode> SQL_TOKEN_NAME
%left <pParseNode> SQL_TOKEN_OR
%left <pParseNode> SQL_TOKEN_AND

%left <pParseNode> LESSEQ GREATEQ NOTEQUAL LESS GREAT EQUAL /* '<' '>' = <> < > <= >= != */
%left <pParseNode> '+' '-'
%left <pParseNode> '*' '/'
%left <pParseNode> SQL_TOKEN_NATURAL SQL_TOKEN_CROSS SQL_TOKEN_FULL SQL_TOKEN_LEFT SQL_TOKEN_RIGHT
%left <pParseNode> ')'
%right <pParseNode> '='
%right <pParseNode> '.'
%right <pParseNode> '('


%nonassoc <pParseNode> SQL_TOKEN_UMINUS



	/* literal keyword tokens */

%token <pParseNode> SQL_TOKEN_ALL SQL_TOKEN_ALTER SQL_TOKEN_AMMSC SQL_TOKEN_ANY SQL_TOKEN_AS SQL_TOKEN_ASC SQL_TOKEN_AT SQL_TOKEN_AUTHORIZATION SQL_TOKEN_AVG

%token <pParseNode> SQL_TOKEN_BETWEEN SQL_TOKEN_BIT SQL_TOKEN_BIT_LENGTH SQL_TOKEN_BOTH SQL_TOKEN_BY

%token <pParseNode> SQL_TOKEN_CAST SQL_TOKEN_CHARACTER SQL_TOKEN_CHAR_LENGTH SQL_TOKEN_CHECK SQL_TOKEN_COLLATE SQL_TOKEN_COMMIT SQL_TOKEN_CONTINUE SQL_TOKEN_CONVERT SQL_TOKEN_COUNT SQL_TOKEN_CREATE SQL_TOKEN_CROSS
%token <pParseNode> SQL_TOKEN_CURRENT SQL_TOKEN_CURRENT_DATE SQL_TOKEN_CURRENT_TIME SQL_TOKEN_CURRENT_TIMESTAMP SQL_TOKEN_CURSOR

%token <pParseNode> SQL_TOKEN_DATE SQL_TOKEN_DAY SQL_TOKEN_DEC SQL_TOKEN_DECIMAL SQL_TOKEN_DECLARE SQL_TOKEN_DEFAULT SQL_TOKEN_DELETE SQL_TOKEN_DESC
%token <pParseNode> SQL_TOKEN_DISTINCT SQL_TOKEN_DOUBLE SQL_TOKEN_DROP

%token <pParseNode> SQL_TOKEN_ESCAPE SQL_TOKEN_EXCEPT SQL_TOKEN_EXISTS SQL_TOKEN_EXTRACT SQL_TOKEN_FALSE SQL_TOKEN_FETCH SQL_TOKEN_FLOAT SQL_TOKEN_FOR SQL_TOKEN_FOREIGN SQL_TOKEN_FOUND SQL_TOKEN_FROM SQL_TOKEN_FULL

%token <pParseNode> SQL_TOKEN_GRANT SQL_TOKEN_GROUP SQL_TOKEN_HAVING SQL_TOKEN_HOUR SQL_TOKEN_IN SQL_TOKEN_INDICATOR SQL_TOKEN_INNER SQL_TOKEN_INSERT SQL_TOKEN_INTEGER SQL_TOKEN_INTO SQL_TOKEN_IS SQL_TOKEN_INTERSECT

%token <pParseNode> SQL_TOKEN_JOIN SQL_TOKEN_KEY SQL_TOKEN_LEADING SQL_TOKEN_LEFT SQL_TOKEN_LIKE SQL_TOKEN_LOCAL SQL_TOKEN_LOWER SQL_TOKEN_MINUTE SQL_TOKEN_MONTH
%token <pParseNode> SQL_TOKEN_MAX SQL_TOKEN_MIN SQL_TOKEN_NATURAL SQL_TOKEN_NCHAR SQL_TOKEN_NULL SQL_TOKEN_NUMERIC

%token <pParseNode> SQL_TOKEN_OCTECT_LENGTH SQL_TOKEN_OF SQL_TOKEN_ON SQL_TOKEN_OPTION SQL_TOKEN_ORDER SQL_TOKEN_OUTER

%token <pParseNode> SQL_TOKEN_POSITION SQL_TOKEN_PRECISION SQL_TOKEN_PRIMARY SQL_TOKEN_PRIVILEGES SQL_TOKEN_PROCEDURE SQL_TOKEN_PUBLIC
%token <pParseNode> SQL_TOKEN_REAL SQL_TOKEN_REFERENCES SQL_TOKEN_ROLLBACK SQL_TOKEN_RIGHT

%token <pParseNode> SQL_TOKEN_SCHEMA SQL_TOKEN_SECOND SQL_TOKEN_SELECT SQL_TOKEN_SET SQL_TOKEN_SIZE SQL_TOKEN_SMALLINT SQL_TOKEN_SOME SQL_TOKEN_SQLCODE SQL_TOKEN_SQLERROR SQL_TOKEN_SUBSTRING SQL_TOKEN_SUM

%token <pParseNode> SQL_TOKEN_TABLE SQL_TOKEN_TIME SQL_TOKEN_TIMESTAMP SQL_TOKEN_TIMEZONE_HOUR SQL_TOKEN_TIMEZONE_MINUTE SQL_TOKEN_TO SQL_TOKEN_TRAILING SQL_TOKEN_TRANSLATE SQL_TOKEN_TRIM SQL_TOKEN_TRUE SQL_TOKEN_UNION
%token <pParseNode> SQL_TOKEN_UNIQUE SQL_TOKEN_UNKNOWN SQL_TOKEN_UPDATE SQL_TOKEN_UPPER SQL_TOKEN_USAGE SQL_TOKEN_USER SQL_TOKEN_USING SQL_TOKEN_VALUES SQL_TOKEN_VIEW
%token <pParseNode> SQL_TOKEN_WHERE SQL_TOKEN_WITH SQL_TOKEN_WORK SQL_TOKEN_YEAR SQL_TOKEN_ZONE

/* ODBC KEYWORDS */
%token <pParseNode> SQL_TOKEN_CALL SQL_TOKEN_D SQL_TOKEN_FN SQL_TOKEN_T SQL_TOKEN_TS SQL_TOKEN_OJ


%token <pParseNode> SQL_TOKEN_INVALIDSYMBOL

/*%type <pParseNode> sql_single_statement */

%type <pParseNode> sql /*schema */
%type <pParseNode> column_def_opt_list column_def_opt table_constraint_def column_commalist
%type <pParseNode> view_def opt_with_check_option opt_column_commalist privilege_def
%type <pParseNode> opt_with_grant_option privileges operation_commalist operation
%type <pParseNode> grantee_commalist grantee opt_order_by_clause ordering_spec_commalist
%type <pParseNode> ordering_spec opt_asc_desc manipulative_statement commit_statement
%type <pParseNode> /*delete_statement_positioned*/ delete_statement_searched fetch_statement
%type <pParseNode> insert_statement values_or_query_spec
%type <pParseNode> rollback_statement select_statement_into opt_all_distinct
%type <pParseNode> /*update_statement_positioned*/ assignment_commalist assignment
%type <pParseNode> update_statement_searched target_commalist target opt_where_clause
%type <pParseNode> select_statement selection table_exp from_clause table_ref_commalist table_ref
%type <pParseNode> where_clause opt_group_by_clause column_ref_commalist opt_having_clause
%type <pParseNode> search_condition predicate comparison_predicate between_predicate
%type <pParseNode> like_predicate opt_escape test_for_null in_predicate
%type <pParseNode> all_or_any_predicate any_all_some existence_test subquery
%type <pParseNode> scalar_exp_commalist parameter_ref literal
%type <pParseNode> column_ref data_type column cursor parameter range_variable user /*like_check*/
/* neue Regeln bei OJ */
%type <pParseNode> derived_column as_clause table_name num_primary term num_value_exp
%type <pParseNode> value_exp_primary num_value_fct unsigned_value_spec cast_spec set_fct_spec  scalar_subquery
%type <pParseNode> position_exp extract_exp length_exp general_value_spec
%type <pParseNode> general_set_fct set_fct_type query_exp non_join_query_exp joined_table
%type <pParseNode> non_join_query_term non_join_query_primary simple_table
%type <pParseNode> table_value_const_list row_value_constructor row_value_const_list row_value_constructor_elem
%type <pParseNode> qualified_join value_exp query_term join_type outer_join_type join_condition boolean_term
%type <pParseNode> boolean_factor truth_value boolean_test boolean_primary named_columns_join join_spec
%type <pParseNode> cast_operand cast_target factor datetime_value_exp /*interval_value_exp*/ datetime_term datetime_factor
%type <pParseNode> datetime_primary datetime_value_fct time_zone time_zone_specifier /*interval_term*/ /*interval_qualifier*/
%type <pParseNode> /*start_field*/ non_second_datetime_field /*end_field*/ /*single_datetime_field*/ extract_field datetime_field time_zone_field
%type <pParseNode> extract_source char_length_exp octet_length_exp bit_length_exp select_sublist string_value_exp
%type <pParseNode> char_value_exp concatenation char_factor char_primary string_value_fct char_substring_fct fold
%type <pParseNode> form_conversion char_translation trim_fct trim_operands trim_spec bit_value_fct bit_substring_fct op_column_commalist
%type <pParseNode> /*bit_concatenation*/ bit_value_exp bit_factor bit_primary collate_clause char_value_fct unique_spec value_exp_commalist in_predicate_value unique_test update_source
%type <pParseNode> all query_primary as not for_length upper_lower comparison column_val  cross_union /*opt_schema_element_list*/
%type <pParseNode> /*op_authorization op_schema*/ nil_fkt schema_element base_table_def base_table_element base_table_element_commalist
%type <pParseNode> column_def odbc_fct_spec	odbc_call_spec odbc_fct_type op_parameter union_statement
%type <pParseNode> op_odbc_call_parameter odbc_parameter_commalist odbc_parameter
%type <pParseNode> catalog_name schema_name table_node
%%

/* Parse Tree an OSQLParser zurueckliefern
 * (der Zugriff ueber yyval nach Aufruf des Parsers scheitert,
 *
 */
sql_single_statement:
		sql
		{ xxx_pGLOBAL_SQLPARSER->setParseTree( $1 ); }
	|	sql ';'
		{ xxx_pGLOBAL_SQLPARSER->setParseTree( $1 ); }
	;
/*
sql_list:
		sql_ ';'
			{$$ = SQL_NEW_LISTRULE;
			$$->append($1);
			pSqlParseTreeRoot = $1; -- obsolete - Ergebnis in yyval! rdm }
	|	sql_list sql ';'
			{$1->append($2);
			$$ = $1;}
	;
*/


	/* schema definition language */
	/* Note: other ``sql:sal_Unicode() rules appear later in the grammar */
/***
sql:
	schema
	;

op_authorization:
		{$$ = SQL_NEW_RULE;}
	|	SQL_TOKEN_AUTHORIZATION user
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;
op_schema:
		{$$ = SQL_NEW_RULE;}
	|	SQL_TOKEN_NAME
	|	SQL_TOKEN_NAME '.' SQL_TOKEN_NAME
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode(".", SQL_NODE_PUNCTUATION));
			$$->append($3);
		}
	;

schema:
		SQL_TOKEN_CREATE SQL_TOKEN_SCHEMA op_schema op_authorization opt_schema_element_list
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
			$$->append($5);
		}
	;

opt_schema_element_list:
		/* empty * /     {$$ = SQL_NEW_RULE;}
	|       schema_glement_list
	;

schema_element_list:
		schema_element
			{$$ = SQL_NEW_LISTRULE;
			$$->append($1);}
	|       schema_element_list schema_element
			{$1->append($2);
			$$ = $1;}
	;
*/
sql:
		schema_element
		{$$ = SQL_NEW_RULE;
			$$->append($1);
			}
	;

schema_element:
			base_table_def
	|       view_def
	|       privilege_def
	;

base_table_def:
		SQL_TOKEN_CREATE SQL_TOKEN_TABLE table_node '(' base_table_element_commalist ')'
		{$$ = SQL_NEW_RULE;
		$$->append($1);
		$$->append($2);
		$$->append($3);
		$$->append($4 = newNode("(", SQL_NODE_PUNCTUATION));
		$$->append($5);
		$$->append($6 = newNode(")", SQL_NODE_PUNCTUATION));}
	;

base_table_element_commalist:
		base_table_element
		{$$ = SQL_NEW_COMMALISTRULE;
		$$->append($1);}
	|   base_table_element_commalist ',' base_table_element
		{$1->append($3);
		$$ = $1;}
	;

base_table_element:
		column_def
	|	table_constraint_def
	;

column_def:
		column data_type column_def_opt_list
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			}
	;

column_def_opt_list:
		/* empty */                 {$$ = SQL_NEW_LISTRULE;}
	|       column_def_opt_list column_def_opt
			{$1->append($2);
			$$ = $1;}
	;

nil_fkt:
	datetime_value_fct
	;
unique_spec:
		SQL_TOKEN_UNIQUE
	|	SQL_TOKEN_PRIMARY SQL_TOKEN_KEY
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;
column_def_opt:
		SQL_TOKEN_NOT SQL_TOKEN_NULL
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);}
	|       unique_spec
	|       SQL_TOKEN_DEFAULT literal
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);}
	|       SQL_TOKEN_DEFAULT SQL_TOKEN_NULL
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);}
	|       SQL_TOKEN_DEFAULT SQL_TOKEN_USER
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);}
	|		SQL_TOKEN_DEFAULT nil_fkt
			{
				$$ = SQL_NEW_RULE;
				$$->append($1);
				$$->append($2);
			}
	|       SQL_TOKEN_CHECK
	|       SQL_TOKEN_CHECK '(' search_condition ')'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));}
	|       SQL_TOKEN_REFERENCES table_node
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);}
	|       SQL_TOKEN_REFERENCES table_node '(' column_commalist ')'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($4);
			$$->append($5 = newNode(")", SQL_NODE_PUNCTUATION));}
	;

table_constraint_def:
		unique_spec '(' column_commalist ')'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));}
	|       SQL_TOKEN_FOREIGN SQL_TOKEN_KEY '(' column_commalist ')' SQL_TOKEN_REFERENCES table_node
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($4);
			$$->append($5 = newNode(")", SQL_NODE_PUNCTUATION));
			$$->append($6);
			$$->append($7);}
	|       SQL_TOKEN_FOREIGN SQL_TOKEN_KEY '(' column_commalist ')' SQL_TOKEN_REFERENCES table_node '(' column_commalist ')'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($4);
			$$->append($5 = newNode(")", SQL_NODE_PUNCTUATION));
			$$->append($6);
			$$->append($7);
			$$->append($8 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($9);
			$$->append($10 = newNode(")", SQL_NODE_PUNCTUATION));}
	|       SQL_TOKEN_CHECK '(' search_condition ')'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));}
	;
op_column_commalist:
	/* empty */ {$$ = SQL_NEW_RULE;}
	| '(' column_commalist ')'
		{$$ = SQL_NEW_RULE;
			$$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($2);
			$$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	;
column_commalist:
		column_commalist ',' column
			{$1->append($3);
			$$ = $1;}
	|	column
			{$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);}
	;

view_def:
		SQL_TOKEN_CREATE SQL_TOKEN_VIEW table_node opt_column_commalist SQL_TOKEN_AS select_statement opt_with_check_option
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
			$$->append($5);
			$$->append($6);
			$$->append($7);}
	;

opt_with_check_option:
		/* empty */         {$$ = SQL_NEW_RULE;}
	|       SQL_TOKEN_WITH SQL_TOKEN_CHECK SQL_TOKEN_OPTION
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);}
	;

opt_column_commalist:
		/* empty */         {$$ = SQL_NEW_RULE;}
	|       '(' column_commalist ')'
			{$$ = SQL_NEW_RULE;
			$$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($2);
			$$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));}
	;

privilege_def:
		SQL_TOKEN_GRANT privileges SQL_TOKEN_ON table_node SQL_TOKEN_TO grantee_commalist
		opt_with_grant_option
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
			$$->append($5);
			$$->append($6);
			$$->append($7);}
	;

opt_with_grant_option:
		/* empty */         {$$ = SQL_NEW_RULE;}
	|       SQL_TOKEN_WITH SQL_TOKEN_GRANT SQL_TOKEN_OPTION
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);}
	;

privileges:
		SQL_TOKEN_ALL SQL_TOKEN_PRIVILEGES
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);}
	|       operation_commalist
	;

operation_commalist:
		operation
			{$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);}
	|       operation_commalist ',' operation
			{$1->append($3);
			$$ = $1;}
	;

operation:
			SQL_TOKEN_SELECT
	|       SQL_TOKEN_INSERT opt_column_commalist
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);}
	|       SQL_TOKEN_DELETE
	|       SQL_TOKEN_UPDATE opt_column_commalist
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);}
	|       SQL_TOKEN_REFERENCES opt_column_commalist
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);}
	|		SQL_TOKEN_USAGE
	;


grantee_commalist:
		grantee
			{$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);}
	|       grantee_commalist ',' grantee
			{$1->append($3);
			$$ = $1;}
	;

grantee:
			SQL_TOKEN_PUBLIC
	|       user
	;

	/* module language */

opt_order_by_clause:
		/* empty */         {$$ = SQL_NEW_RULE;}
	|       SQL_TOKEN_ORDER SQL_TOKEN_BY ordering_spec_commalist
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);}
	;

ordering_spec_commalist:
		ordering_spec
			{$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);}
	|       ordering_spec_commalist ',' ordering_spec
			{$1->append($3);
			$$ = $1;}
	;

ordering_spec:
/*		SQL_TOKEN_INTNUM opt_asc_desc
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);}
*/
		predicate opt_asc_desc
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);}

	|	row_value_constructor_elem opt_asc_desc
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);}
	;

opt_asc_desc:
		{$$ = SQL_NEW_RULE;}
	|	SQL_TOKEN_ASC
	|	SQL_TOKEN_DESC
	;


/***
manipulative_statement_list:
		manipulative_statement
			{$$ = SQL_NEW_LISTRULE;
			$$->append($1);}
	|       manipulative_statement_list manipulative_statement
			{$1->append($2);
			$$ = $1;}
	;
***/

sql:
		{$$ = SQL_NEW_LISTRULE;}
	;
not:
	{$$ = SQL_NEW_RULE;}
	|	SQL_TOKEN_NOT
	;
	/* manipulative statements */

sql:    manipulative_statement
	;

manipulative_statement:
			commit_statement
/*	|       delete_statement_positioned*/
	|       delete_statement_searched
	|       fetch_statement
	|       insert_statement
	|       rollback_statement
	|       select_statement_into
/*	|       update_statement_positioned*/
	|       update_statement_searched
	|		union_statement
	|		'{' odbc_call_spec '}'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1 = newNode("{", SQL_NODE_PUNCTUATION));
			$$->append($2);
			$$->append($3 = newNode("}", SQL_NODE_PUNCTUATION));
		}
	;

union_statement:
			select_statement
	|		union_statement SQL_TOKEN_UNION all select_statement
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	;
commit_statement:
		SQL_TOKEN_COMMIT SQL_TOKEN_WORK
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);}
	;
/*
delete_statement_positioned:
		SQL_TOKEN_DELETE SQL_TOKEN_FROM table_node SQL_TOKEN_WHERE SQL_TOKEN_CURRENT SQL_TOKEN_OF cursor
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
			$$->append($5);
			$$->append($6);
			$$->append($7);}
	;
*/
delete_statement_searched:
		SQL_TOKEN_DELETE SQL_TOKEN_FROM table_node opt_where_clause
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);}
	;

fetch_statement:
		SQL_TOKEN_FETCH cursor SQL_TOKEN_INTO target_commalist
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);}
	;

insert_statement:
		SQL_TOKEN_INSERT SQL_TOKEN_INTO table_node opt_column_commalist values_or_query_spec
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
			$$->append($5);}
	;
values_or_query_spec:
		SQL_TOKEN_VALUES table_value_const_list
		{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;

table_value_const_list:
			row_value_constructor
			{$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);}
	|       table_value_const_list ',' row_value_constructor
			{$1->append($3);
			$$ = $1;}
	;
row_value_const_list:
			row_value_constructor_elem
			{$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);}
	|       row_value_const_list ',' row_value_constructor_elem
			{$1->append($3);
			$$ = $1;}
	;
row_value_constructor:
			row_value_constructor_elem
	  |		'(' row_value_const_list ')'
			{
				$$ = SQL_NEW_RULE;
				$$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
				$$->append($2);
				$$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));
			}
/*      |		subquery
			{
				$$ = SQL_NEW_RULE;
				$$->append($1);
			}*/
	;
row_value_constructor_elem:
			value_exp /*[^')']*/
	|       SQL_TOKEN_NULL
	|		SQL_TOKEN_DEFAULT
	;


rollback_statement:
		SQL_TOKEN_ROLLBACK SQL_TOKEN_WORK
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);}
	;


		/* INTO target_commalist herausgenommen */
select_statement_into:
		SQL_TOKEN_SELECT opt_all_distinct selection SQL_TOKEN_INTO target_commalist table_exp
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
			$$->append($5);
			$$->append($6); }
	;

opt_all_distinct:
			{$$ = SQL_NEW_RULE;}
		|	SQL_TOKEN_ALL
		|	SQL_TOKEN_DISTINCT

	;
/*
update_statement_positioned:
		SQL_TOKEN_UPDATE table_node SQL_TOKEN_SET assignment_commalist
		SQL_TOKEN_WHERE SQL_TOKEN_CURRENT SQL_TOKEN_OF cursor
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
			$$->append($5);
			$$->append($6);
			$$->append($7);
			$$->append($8);}
	;
*/
assignment_commalist:
			assignment
			{$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);}
	|       assignment_commalist ',' assignment
			{$1->append($3);
			$$ = $1;}
	;

assignment:
		column EQUAL update_source
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);}
	;
update_source:
		value_exp
	  | SQL_TOKEN_NULL
	  | SQL_TOKEN_DEFAULT
	;
update_statement_searched:
		SQL_TOKEN_UPDATE table_node SQL_TOKEN_SET assignment_commalist opt_where_clause
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
			$$->append($5);}
	;

target_commalist:
		target
			{$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);}
	|       target_commalist ',' target
			{$1->append($3);
			$$ = $1;}
	;

target:
		parameter_ref
	;

opt_where_clause:
		/* empty */             {$$ = SQL_NEW_RULE;}
	|       where_clause
	;

	/* query expressions */

query_term:
		non_join_query_term
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
/*      | joined_table
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
*/	;
/* SELECT STATEMENT */
select_statement:
		SQL_TOKEN_SELECT opt_all_distinct selection table_exp
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	;

selection:
		'*'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1 = newNode("*", SQL_NODE_PUNCTUATION));
		}
	|	scalar_exp_commalist
	;

table_exp:
		from_clause
		opt_where_clause
		opt_group_by_clause
		opt_having_clause
		opt_order_by_clause
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
			$$->append($5);}
	;

from_clause:
		SQL_TOKEN_FROM table_ref_commalist
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);}
	;

table_ref_commalist:
		table_ref
			{$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);}
	|       table_ref_commalist ',' table_ref /*[^SQL_TOKEN_CROSS SQL_TOKEN_FULL SQL_TOKEN_UNION SQL_TOKEN_LEFT SQL_TOKEN_RIGHT SQL_TOKEN_INNER SQL_TOKEN_NATURAL]*/
			{$1->append($3);
			$$ = $1;}
	;
as:
		{$$ = SQL_NEW_RULE;}
	|	SQL_TOKEN_AS
		{
			$$ = SQL_NEW_RULE;
		}
	;
table_ref:
		table_node
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	|	table_node as range_variable op_column_commalist
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	|	'(' joined_table ')' as range_variable op_column_commalist
		{
			$$ = SQL_NEW_RULE;
			$$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($2);
			$$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));
			$$->append($4);
			$$->append($5);
			$$->append($6);
		}
	|	joined_table
	|	'{' SQL_TOKEN_OJ joined_table '}'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1 = newNode("{", SQL_NODE_PUNCTUATION));
			$$->append($2);
			$$->append($3);
			$$->append($4 = newNode("}", SQL_NODE_PUNCTUATION));
		}
	;
where_clause:
		SQL_TOKEN_WHERE search_condition
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);}
	;

opt_group_by_clause:
		/* empty */      {$$ = SQL_NEW_RULE;}
	|       SQL_TOKEN_GROUP SQL_TOKEN_BY column_ref_commalist
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);}
	;

column_ref_commalist:
		column_ref
			{$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);}
	|       column_ref_commalist ',' column_ref
			{$1->append($3);
			$$ = $1;}
	;

opt_having_clause:
		/* empty */                 {$$ = SQL_NEW_RULE;}
	|       SQL_TOKEN_HAVING search_condition
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);}
	;

	/* search conditions */
truth_value:
		SQL_TOKEN_TRUE
	  | SQL_TOKEN_FALSE
	  | SQL_TOKEN_UNKNOWN
	  ;
boolean_primary:
		predicate
	  | '(' search_condition ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($2);
			$$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	;
subroutine:
	{
			if(!xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
				YYERROR;
		}
	;
boolean_test:
		boolean_primary
	|	boolean_primary SQL_TOKEN_IS truth_value
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	|	boolean_primary SQL_TOKEN_IS SQL_TOKEN_NOT truth_value %prec SQL_TOKEN_IS
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	|	subroutine SQL_TOKEN_NOT SQL_TOKEN_LIKE string_value_exp opt_escape
		{
			if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
			{
				OSQLParseNode* pColumnRef = newNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
				pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQL_NODE_NAME));

				$$ = SQL_NEW_RULE;
				$$->append(pColumnRef);
				$$->append($2);
				$$->append($3);
				if (xxx_pGLOBAL_SQLPARSER->buildLikeRule($$,$4,$5))
					$$->append($5);
				else
				{
					delete $$;
					YYABORT;
				}
			}
			else
				YYERROR;
		}
	;
boolean_factor:
		boolean_test
	|	SQL_TOKEN_NOT  boolean_test
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;
boolean_term:
		boolean_factor
	|	boolean_term SQL_TOKEN_AND boolean_factor
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	|	row_value_constructor_elem  /*[^')' ',']*/
		{
			$$ = SQL_NEW_RULE;
			sal_Int16 nErg = xxx_pGLOBAL_SQLPARSER->buildComparsionRule($$,$1);
			if(nErg == 1)
			{
				OSQLParseNode* pTemp = $$;
				$$ = pTemp->removeAt((sal_uInt32)0);
				delete pTemp;
			}
			else
			{
				delete $$;
				if(nErg)
					YYERROR;
				else
					YYABORT;
			}
		}
	|	boolean_term SQL_TOKEN_AND literal
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			sal_Int16 nErg = xxx_pGLOBAL_SQLPARSER->buildComparsionRule($$,$3);
			if(nErg < 1)
			{
				delete $$;
				if(nErg)
					YYERROR;
				else
					YYABORT;
			}

		}
	|	boolean_term SQL_TOKEN_AND SQL_TOKEN_STRING
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			sal_Int16 nErg = xxx_pGLOBAL_SQLPARSER->buildComparsionRule($$,$3);
			if(nErg < 1)
			{
				delete $$;
				if(nErg)
					YYERROR;
				else
					YYABORT;
			}

		}
	;
search_condition:
		boolean_term
	|	search_condition SQL_TOKEN_OR boolean_term
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	;
predicate:
		comparison_predicate
	|       between_predicate
	|       all_or_any_predicate
	|       existence_test
	|		unique_test
	|		test_for_null
	|       in_predicate
	|       like_predicate
	;

comparison_predicate:
		row_value_constructor comparison row_value_constructor
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	|	comparison row_value_constructor
		{
			$$ = SQL_NEW_RULE;
			sal_Int16 nErg = xxx_pGLOBAL_SQLPARSER->buildComparsionRule($$,$2,$1);
			if(nErg == 1)
			{
				OSQLParseNode* pTemp = $$;
				$$ = pTemp->removeAt((sal_uInt32)0);
				delete pTemp;
			}
			else
			{
				delete $$;
				YYABORT;
			}
		}
	;
comparison:
		LESS
	  | NOTEQUAL
	  | EQUAL
	  | GREAT
	  | LESSEQ
	  | GREATEQ
	;
between_predicate:
		row_value_constructor SQL_TOKEN_BETWEEN row_value_constructor SQL_TOKEN_AND row_value_constructor
		{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
			$$->append($5);
		}
	|	row_value_constructor SQL_TOKEN_NOT SQL_TOKEN_BETWEEN row_value_constructor SQL_TOKEN_AND row_value_constructor
		{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
			$$->append($5);
			$$->append($6);
		}
	|	subroutine SQL_TOKEN_NOT SQL_TOKEN_BETWEEN row_value_constructor SQL_TOKEN_AND row_value_constructor
		{
			if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
			{
				OSQLParseNode* pColumnRef = newNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
				pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQL_NODE_NAME));

				$$ = SQL_NEW_RULE;
				$$->append(pColumnRef);
				$$->append($2);
				$$->append($3);
				$$->append($4);
				$$->append($5);
				$$->append($6);
			}
			else
				YYERROR;
		}
	|	subroutine SQL_TOKEN_BETWEEN row_value_constructor SQL_TOKEN_AND row_value_constructor
		{
			if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
			{
				OSQLParseNode* pColumnRef = newNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
				pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQL_NODE_NAME));

				$$ = SQL_NEW_RULE;
				$$->append(pColumnRef);
				$$->append($2);
				$$->append($3);
				$$->append($4);
				$$->append($5);
			}
			else
				YYERROR;
		}
	;

like_predicate:
		row_value_constructor SQL_TOKEN_NOT SQL_TOKEN_LIKE string_value_exp opt_escape
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
			$$->append($5);
		}
	|	row_value_constructor SQL_TOKEN_LIKE string_value_exp opt_escape
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	|	row_value_constructor SQL_TOKEN_NOT SQL_TOKEN_LIKE value_exp_primary opt_escape
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
			$$->append($5);
		}
	|	row_value_constructor SQL_TOKEN_LIKE value_exp_primary opt_escape
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	/*|	SQL_TOKEN_NOT
		{
			if(!xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
				YYERROR;
		} SQL_TOKEN_LIKE string_value_exp opt_escape
		{
			if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
			{
				OSQLParseNode* pColumnRef = newNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
				pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQL_NODE_NAME));

				$$ = SQL_NEW_RULE;
				$$->append(pColumnRef);
				$$->append($1);
				$$->append($3);
				if (xxx_pGLOBAL_SQLPARSER->buildLikeRule($$,$4,$5))
					$$->append($5);
				else
				{
					delete $$;
					YYABORT;
				}
			}
			else
				YYERROR;
		}
	*/|	SQL_TOKEN_LIKE string_value_exp opt_escape
		{
			if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
			{
				OSQLParseNode* pColumnRef = newNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
				pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQL_NODE_NAME));

				$$ = SQL_NEW_RULE;
				$$->append(pColumnRef);
				$$->append($1);
				if (xxx_pGLOBAL_SQLPARSER->buildLikeRule($$,$2,$3))
					$$->append($3);
				else
				{
					delete $$;
					YYABORT;
				}
			}
			else
				YYERROR;
		}
	|	SQL_TOKEN_LIKE value_exp_primary opt_escape
		{
			if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
			{
				OSQLParseNode* pColumnRef = newNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
				pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQL_NODE_NAME));

				$$ = SQL_NEW_RULE;
				$$->append(pColumnRef);
				$$->append($1);
				if (xxx_pGLOBAL_SQLPARSER->buildLikeRule($$,$2,$3))
					$$->append($3);
				else
				{
					delete $$;
					YYABORT;
				}
			}
			else
				YYERROR;
		}
	|	subroutine SQL_TOKEN_NOT SQL_TOKEN_LIKE value_exp_primary opt_escape
		{
			if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
			{
				OSQLParseNode* pColumnRef = newNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
				pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQL_NODE_NAME));

				$$ = SQL_NEW_RULE;
				$$->append(pColumnRef);
				$$->append($2);
				$$->append($3);
				if (xxx_pGLOBAL_SQLPARSER->buildLikeRule($$,$4,$5))
					$$->append($5);
				else
				{
					delete $$;
					YYABORT;
				}
			}
			else
				YYERROR;
		}
	;

opt_escape:
		/* empty */                 {$$ = SQL_NEW_RULE;}
	|   SQL_TOKEN_ESCAPE string_value_exp
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);}
	|	'{' SQL_TOKEN_ESCAPE SQL_TOKEN_STRING '}'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1 = newNode("{", SQL_NODE_PUNCTUATION));
			$$->append($2);
			$$->append($3);
			$$->append($4 = newNode("}", SQL_NODE_PUNCTUATION));
		}
	;

test_for_null:
		row_value_constructor SQL_TOKEN_IS not SQL_TOKEN_NULL
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	|	SQL_TOKEN_IS not SQL_TOKEN_NULL
		{
			if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
			{
				OSQLParseNode* pColumnRef = newNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
				pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQL_NODE_NAME));

				$$ = SQL_NEW_RULE;
				$$->append(pColumnRef);
				$$->append($1);
				$$->append($2);
				$$->append($3);
			}
			else
				YYERROR;
		}
	;
in_predicate_value:
		subquery
		{$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  | '(' value_exp_commalist ')'
		{$$ = SQL_NEW_RULE;
			$$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($2);
			$$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	;
in_predicate:
		row_value_constructor not SQL_TOKEN_IN in_predicate_value
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
		|	subroutine  SQL_TOKEN_IN in_predicate_value
		{
			if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
			{
				OSQLParseNode* pColumnRef = newNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
				pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQL_NODE_NAME));

				$$ = SQL_NEW_RULE;
				$$->append(pColumnRef);
				$$->append($2);
				$$->append($3);
				/*$$->append($3);*/
			}
			else
				YYERROR;
		}
	;
all_or_any_predicate:
		row_value_constructor comparison any_all_some subquery
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	|	comparison any_all_some subquery
		{
			if(xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
			{
				OSQLParseNode* pColumnRef = newNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
				pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQL_NODE_NAME));

				$$ = SQL_NEW_RULE;
				$$->append(pColumnRef);
				$$->append($1);
				$$->append($2);
				$$->append($3);
			}
			else
				YYERROR;
		}
	;

any_all_some:
			SQL_TOKEN_ANY
	|       SQL_TOKEN_ALL
	|       SQL_TOKEN_SOME
	;

existence_test:
		SQL_TOKEN_EXISTS subquery
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);}
	;
unique_test:
		SQL_TOKEN_UNIQUE subquery
		{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);}
	;
subquery:
		'(' query_exp ')'
			{$$ = SQL_NEW_RULE;
			$$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($2);
			$$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));}
	;

	/* scalar expressions */
scalar_exp_commalist:
		select_sublist
		{
			$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);
		}
	|   scalar_exp_commalist ',' select_sublist
		{
			$1->append($3);
			$$ = $1;
		}
	;
select_sublist:
/*		table_node '.' '*'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode(".", SQL_NODE_PUNCTUATION));
			$$->append($3 = newNode("*", SQL_NODE_PUNCTUATION));
		}
*/
		derived_column

	;

parameter_ref:
		parameter
	;

/*
op_like:
		'*'
		{
			$$ = newNode("*", SQL_NODE_PUNCTUATION);
		}
	|	'?'
		{
			$$ = newNode("?", SQL_NODE_PUNCTUATION);
		}
	|   op_like	'*'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("*", SQL_NODE_PUNCTUATION));
			xxx_pGLOBAL_SQLPARSER->reduceLiteral($$, sal_False);
		}
	|	op_like '?'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("?", SQL_NODE_PUNCTUATION));
			xxx_pGLOBAL_SQLPARSER->reduceLiteral($$, sal_False);
		}
	;
*/

literal:
/*		SQL_TOKEN_STRING
	|   */SQL_TOKEN_INT
	|   SQL_TOKEN_REAL_NUM
	|   SQL_TOKEN_INTNUM
	|   SQL_TOKEN_APPROXNUM
	|	SQL_TOKEN_ACCESS_DATE
/*	rules for predicate check */
	|	literal SQL_TOKEN_STRING
		{
			if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
			{
				$$ = SQL_NEW_RULE;
				$$->append($1);
				$$->append($2);
				xxx_pGLOBAL_SQLPARSER->reduceLiteral($$, sal_True);
			}
			else
				YYERROR;
		}
	|	literal SQL_TOKEN_INT
		{
			if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
			{
				$$ = SQL_NEW_RULE;
				$$->append($1);
				$$->append($2);
				xxx_pGLOBAL_SQLPARSER->reduceLiteral($$, sal_True);
			}
			else
				YYERROR;
		}
	|	literal SQL_TOKEN_REAL_NUM
		{
			if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
			{
				$$ = SQL_NEW_RULE;
				$$->append($1);
				$$->append($2);
				xxx_pGLOBAL_SQLPARSER->reduceLiteral($$, sal_True);
			}
			else
				YYERROR;
		}
	|	literal SQL_TOKEN_APPROXNUM
		{
			if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
			{
				$$ = SQL_NEW_RULE;
				$$->append($1);
				$$->append($2);
				xxx_pGLOBAL_SQLPARSER->reduceLiteral($$, sal_True);
			}
			else
				YYERROR;
		}
	;

	/* miscellaneous */
as_clause:
		/* empty */ {$$ = SQL_NEW_RULE;}
	|	SQL_TOKEN_AS column
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	|	column
	;
position_exp:
		SQL_TOKEN_POSITION '(' string_value_exp SQL_TOKEN_IN string_value_exp ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4);
			$$->append($5);
			$$->append($6 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	;
num_value_fct:
		position_exp
	|	extract_exp
	|	length_exp
	;
char_length_exp:
		SQL_TOKEN_CHAR_LENGTH '(' string_value_exp ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	;
octet_length_exp:
		SQL_TOKEN_OCTECT_LENGTH '(' string_value_exp ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	;
bit_length_exp:
		SQL_TOKEN_BIT_LENGTH '(' string_value_exp ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	;
length_exp:
		char_length_exp
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  | octet_length_exp
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  | bit_length_exp
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	;
datetime_field:
		non_second_datetime_field
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  | SQL_TOKEN_SECOND
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	;
extract_field:
		datetime_field
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  | time_zone_field
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	;
time_zone_field:
		SQL_TOKEN_TIMEZONE_HOUR
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  | SQL_TOKEN_TIMEZONE_MINUTE
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	;
extract_source:
		datetime_value_exp
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
/*      | interval_value_exp
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		} */
	;
extract_exp:
		SQL_TOKEN_EXTRACT '(' extract_field SQL_TOKEN_FROM extract_source ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4);
			$$->append($5);
			$$->append($6 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	;
unsigned_value_spec:
		literal
	|	general_value_spec
	;
general_value_spec:
		parameter
	  | SQL_TOKEN_USER
	;
set_fct_spec:
		general_set_fct
	|	'{' odbc_fct_spec '}'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1 = newNode("{", SQL_NODE_PUNCTUATION));
			$$->append($2);
			$$->append($3 = newNode("}", SQL_NODE_PUNCTUATION));
		}
	|	SQL_TOKEN_NAME '(' value_exp_commalist ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	;
op_parameter:
		{$$ = SQL_NEW_RULE;}
	|	'?' EQUAL
		{
			$$ = SQL_NEW_RULE;
			$$->append($1 = newNode("?", SQL_NODE_PUNCTUATION));
			$$->append($2);
		}
	;
odbc_call_spec:
		op_parameter SQL_TOKEN_CALL table_node op_odbc_call_parameter
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	;

op_odbc_call_parameter:
		{$$ = SQL_NEW_RULE;}
	|	'(' odbc_parameter_commalist ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($2);
			$$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	;

odbc_parameter_commalist:
		odbc_parameter
		{$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);}
	|	odbc_parameter_commalist ',' odbc_parameter
		{
			$1->append($3);
			$$ = $1;
		}
	;
odbc_parameter:
	/* empty */ 	{$$ = SQL_NEW_RULE;}
	|	literal
	|	parameter
	;

odbc_fct_spec:
		odbc_fct_type SQL_TOKEN_STRING
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;

odbc_fct_type:
		SQL_TOKEN_FN
	| 	SQL_TOKEN_D
	| 	SQL_TOKEN_T
	| 	SQL_TOKEN_TS
	;

general_set_fct:
		set_fct_type '(' opt_all_distinct value_exp ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4);
			$$->append($5 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	|	SQL_TOKEN_COUNT '(' '*' ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3 = newNode("*", SQL_NODE_PUNCTUATION));
			$$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	|	SQL_TOKEN_COUNT '(' opt_all_distinct value_exp ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4);
			$$->append($5 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	;
set_fct_type:
		SQL_TOKEN_AVG
	|   SQL_TOKEN_MAX
	|   SQL_TOKEN_MIN
	|   SQL_TOKEN_SUM
	;

outer_join_type:
		SQL_TOKEN_LEFT %prec SQL_TOKEN_LEFT
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  | SQL_TOKEN_RIGHT %prec SQL_TOKEN_RIGHT
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  | SQL_TOKEN_FULL %prec SQL_TOKEN_FULL
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  ;
join_condition:
		SQL_TOKEN_ON search_condition
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
		;
join_spec:
		join_condition
	  | named_columns_join
	  ;
join_type:
		SQL_TOKEN_INNER
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  | outer_join_type
	  | outer_join_type SQL_TOKEN_OUTER
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;
cross_union:
		table_ref /*[SQL_TOKEN_CROSS]*/ SQL_TOKEN_CROSS SQL_TOKEN_JOIN table_ref
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	;

qualified_join:
		/* wenn SQL_TOKEN_NATURAL, dann keine join_spec */
		table_ref /*[SQL_TOKEN_NATURAL]*/ SQL_TOKEN_NATURAL join_type SQL_TOKEN_JOIN table_ref
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
			$$->append($5);
		}
	|	table_ref join_type SQL_TOKEN_JOIN table_ref join_spec
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
			$$->append($5);
		}
	|	cross_union
	;
joined_table:
		qualified_join
/*	|	query_exp*/
	|	'(' joined_table ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($2);
			$$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));
		}

	;
named_columns_join:
		SQL_TOKEN_USING '(' column_commalist ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	;
simple_table:
		select_statement
	  | values_or_query_spec
	  ;

non_join_query_primary:
		simple_table
	  | '(' non_join_query_exp ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($2);
			$$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	  ;
non_join_query_term:
		non_join_query_primary
	|	query_term SQL_TOKEN_INTERSECT all query_primary
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	;
query_primary:
		non_join_query_primary
	 /*|	joined_table*/
	;
non_join_query_exp:
		non_join_query_term
	  | query_exp SQL_TOKEN_UNION all query_term
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	  | query_exp SQL_TOKEN_EXCEPT all query_term
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	;
all:
	/* empty*/ {$$ = SQL_NEW_RULE;}
	|	SQL_TOKEN_ALL
	;
query_exp:
		non_join_query_exp /*[^')']*/
	;
scalar_subquery:
		subquery
	;
cast_operand:
		value_exp
	  | SQL_TOKEN_NULL
	;
cast_target:
		table_node
	  | data_type
	;
cast_spec:
	  SQL_TOKEN_CAST '(' cast_operand SQL_TOKEN_AS cast_target ')'
	  {
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4);
			$$->append($5);
			$$->append($6 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	;
value_exp_primary:
		unsigned_value_spec
	  | column_ref
	  | set_fct_spec
	  | scalar_subquery
	  | '(' value_exp ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($2);
			$$->append($3 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	  | cast_spec
	;

num_primary:
		value_exp_primary
	  | num_value_fct
	;
factor:
		num_primary
	|	'-' num_primary  %prec SQL_TOKEN_UMINUS
		{
			$$ = SQL_NEW_RULE;
			$$->append($1 = newNode("-", SQL_NODE_PUNCTUATION));
			$$->append($2);
		}
	|	'+' num_primary  %prec SQL_TOKEN_UMINUS
		{
			$$ = SQL_NEW_RULE;
			$$->append($1 = newNode("+", SQL_NODE_PUNCTUATION));
			$$->append($2);
		}
	;

term:
		factor
	  | term '*' factor
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("*", SQL_NODE_PUNCTUATION));
			$$->append($3);
		}
	  | term '/' factor
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("/", SQL_NODE_PUNCTUATION));
			$$->append($3);
		}
	  ;

num_value_exp:
		term
	  | num_value_exp '+' term
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("+", SQL_NODE_PUNCTUATION));
			$$->append($3);
		}
	  | num_value_exp '-' term
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("-", SQL_NODE_PUNCTUATION));
			$$->append($3);
		}
	  ;
datetime_primary:
/*		value_exp_primary
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  |*/ datetime_value_fct
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	;
datetime_value_fct:
		SQL_TOKEN_CURRENT_DATE
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  | SQL_TOKEN_CURRENT_TIME
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  | SQL_TOKEN_CURRENT_TIMESTAMP
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	;
time_zone:
		SQL_TOKEN_AT time_zone_specifier
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;
time_zone_specifier:
		SQL_TOKEN_LOCAL
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
/*      | SQL_TOKEN_TIME SQL_TOKEN_ZONE interval_value_exp
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}*/
	  ;
datetime_factor:
		datetime_primary
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	|	datetime_primary time_zone
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;
datetime_term:
		datetime_factor
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	;
/*
interval_term:
		literal
	  | interval_term '*' factor
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("*", SQL_NODE_PUNCTUATION));
			$$->append($3);
		}
	  | interval_term '/' factor
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("/", SQL_NODE_PUNCTUATION));
			$$->append($3);
		}
	;
*/
datetime_value_exp:
		datetime_term
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
/*      | interval_value_exp '+' datetime_term
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("+", SQL_NODE_PUNCTUATION));
			$$->append($3);
		}
	  | datetime_value_exp '+' interval_term
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("+", SQL_NODE_PUNCTUATION));
			$$->append($3);
		}
	  | datetime_value_exp '-' interval_term
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("-", SQL_NODE_PUNCTUATION));
			$$->append($3);
		}
*/	;
/*
interval_value_exp:
		interval_term
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  | interval_value_exp '+' interval_term
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("+", SQL_NODE_PUNCTUATION));
			$$->append($3);
		}
	  | interval_value_exp '-' interval_term
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("-", SQL_NODE_PUNCTUATION));
			$$->append($3);
		}
	  | '(' datetime_value_exp '-' datetime_term ')' interval_qualifier
		{
			$$ = SQL_NEW_RULE;
			$$->append($1 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($2);
			$$->append($3 = newNode("-", SQL_NODE_PUNCTUATION));
			$$->append($4);
			$$->append($5 = newNode(")", SQL_NODE_PUNCTUATION));
			$$->append($6);
		}
	;
*/
non_second_datetime_field:
		SQL_TOKEN_YEAR
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	|	SQL_TOKEN_MONTH
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	|	SQL_TOKEN_DAY
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	|	SQL_TOKEN_HOUR
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	|	SQL_TOKEN_MINUTE
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	;
/*start_field:
		non_second_datetime_field
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	|	non_second_datetime_field '(' SQL_TOKEN_INTNUM ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	;
end_field:
		non_second_datetime_field
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  | SQL_TOKEN_SECOND
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  | SQL_TOKEN_SECOND '(' SQL_TOKEN_INTNUM ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	;
*/
/*
single_datetime_field:
		non_second_datetime_field
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	|	non_second_datetime_field '(' SQL_TOKEN_INTNUM ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	|	SQL_TOKEN_SECOND
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	|	SQL_TOKEN_SECOND '(' SQL_TOKEN_INTNUM ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	|	SQL_TOKEN_SECOND '(' SQL_TOKEN_INTNUM ',' SQL_TOKEN_INTNUM ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(",", SQL_NODE_PUNCTUATION));
			$$->append($5);
			$$->append($6 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	;
*/
/*
interval_qualifier:
		start_field SQL_TOKEN_TO end_field
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	  | single_datetime_field
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	;
*/
value_exp_commalist:
		value_exp
			{$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);}
	|       value_exp_commalist ',' value_exp
			{$1->append($3);
			$$ = $1;}
	/*	this rule is only valid if we check predicates */
	|   value_exp_commalist ';' value_exp
		{
			if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
			{
				$1->append($3);
				$$ = $1;
			}
			else
				YYERROR;
		}
	;
value_exp:
		num_value_exp /*[^')']*/
	  | string_value_exp
	  | datetime_value_exp
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	;
string_value_exp:
		char_value_exp
/*      | bit_value_exp
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
*/	;
char_value_exp:
		char_factor
	|	concatenation
	;
concatenation:
	  char_value_exp '+' char_factor
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("+", SQL_NODE_PUNCTUATION));
			$$->append($3);
		}
	;

char_primary:
			SQL_TOKEN_STRING
	  |		string_value_fct
			{
				$$ = SQL_NEW_RULE;
				$$->append($1);
			}
	;
collate_clause:
		SQL_TOKEN_COLLATE table_node
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;
char_factor:
		char_primary
	|	char_primary collate_clause
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;
string_value_fct:
		char_value_fct
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  | bit_value_fct
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	;
bit_value_fct:
		bit_substring_fct
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	;
bit_substring_fct:
		SQL_TOKEN_SUBSTRING '(' bit_value_exp SQL_TOKEN_FROM string_value_exp for_length ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4);
			$$->append($5);
			$$->append($6);
			$$->append($7 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	;
bit_value_exp:
		bit_factor
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	;
/*
			bit_concatenation
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  |
bit_concatenation:
		bit_value_exp '+' bit_factor
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("+", SQL_NODE_PUNCTUATION));
			$$->append($3);
		}
	;
*/
bit_factor:
		bit_primary
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	;
bit_primary:
	{$$ = SQL_NEW_RULE;}
/*		value_exp_primary
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  | string_value_fct
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}*/
	;
char_value_fct:
		char_substring_fct
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  | fold
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  | form_conversion
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  | char_translation
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	  | trim_fct
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
		}
	;
for_length:
		{$$ = SQL_NEW_RULE;}
	|	SQL_TOKEN_FOR string_value_exp
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;
char_substring_fct:
		SQL_TOKEN_SUBSTRING '(' string_value_exp SQL_TOKEN_FROM string_value_exp for_length ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4);
			$$->append($5);
			$$->append($6);
			$$->append($7 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	;
upper_lower:
		SQL_TOKEN_UPPER
	|	SQL_TOKEN_LOWER
	;
fold:
		upper_lower '(' string_value_exp ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	;
form_conversion:
		SQL_TOKEN_CONVERT '(' string_value_exp SQL_TOKEN_USING table_node ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4);
			$$->append($5);
			$$->append($6 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	;
char_translation:
		SQL_TOKEN_TRANSLATE '(' string_value_exp SQL_TOKEN_USING table_node ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4);
			$$->append($5);
			$$->append($6 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	;
trim_fct:
		SQL_TOKEN_TRIM  '(' trim_operands ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));
		}
	;
trim_operands:
		string_value_exp
	|	trim_spec string_value_exp SQL_TOKEN_FROM string_value_exp
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	|	 trim_spec SQL_TOKEN_FROM string_value_exp
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	;
trim_spec:
		SQL_TOKEN_BOTH
	|	SQL_TOKEN_LEADING
	|	SQL_TOKEN_TRAILING
	;

derived_column:
		value_exp as_clause
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;
/* Tabellenname */
table_node:
		table_name
	|	schema_name
	|	catalog_name
;
catalog_name:
		SQL_TOKEN_NAME '.' schema_name
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode(".", SQL_NODE_PUNCTUATION));
			$$->append($3);
		}
	|	SQL_TOKEN_NAME ':' schema_name
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode(":", SQL_NODE_PUNCTUATION));
			$$->append($3);
		}
;
schema_name:
		SQL_TOKEN_NAME '.' table_name 
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode(".", SQL_NODE_PUNCTUATION));
			$$->append($3);
		}
;

table_name:
			SQL_TOKEN_NAME
			{$$ = SQL_NEW_RULE;
			$$->append($1);}
/*	|	    SQL_TOKEN_NAME '.' SQL_TOKEN_NAME %prec SQL_TOKEN_NAME
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode(".", SQL_NODE_PUNCTUATION));
			$$->append($3);}
	|       SQL_TOKEN_NAME '.' SQL_TOKEN_NAME '.' SQL_TOKEN_NAME %prec SQL_TOKEN_NAME
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2= newNode(".", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(".", SQL_NODE_PUNCTUATION));
			$$->append($5);}
	|       SQL_TOKEN_NAME ':' SQL_TOKEN_NAME '.' SQL_TOKEN_NAME
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2= newNode(":", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(".", SQL_NODE_PUNCTUATION));
			$$->append($5);}
/*	|       SQL_TOKEN_NAME ';' SQL_TOKEN_NAME '.' SQL_TOKEN_NAME
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2= newNode(";", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(".", SQL_NODE_PUNCTUATION));
			$$->append($5);}
*/	;
/* Columns */
column_ref:
			column
			{$$ = SQL_NEW_RULE;
			$$->append($1);}
/*	|       table_node '.' column_val %prec '.'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode(".", SQL_NODE_PUNCTUATION));
			$$->append($3);}
*/	
	|       SQL_TOKEN_NAME '.' column_val %prec '.'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode(".", SQL_NODE_PUNCTUATION));
			$$->append($3);
			}
	|       SQL_TOKEN_NAME '.' SQL_TOKEN_NAME '.' column_val %prec '.'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode(".", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(".", SQL_NODE_PUNCTUATION));
			$$->append($5);}
	|       SQL_TOKEN_NAME '.' SQL_TOKEN_NAME '.' SQL_TOKEN_NAME '.' column_val %prec '.'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2= newNode(".", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(".", SQL_NODE_PUNCTUATION));
			$$->append($5);
			$$->append($6 = newNode(".", SQL_NODE_PUNCTUATION));
			$$->append($7);
			}
	|       SQL_TOKEN_NAME ':' SQL_TOKEN_NAME '.' SQL_TOKEN_NAME '.' column_val %prec '.'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2= newNode(":", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(".", SQL_NODE_PUNCTUATION));
			$$->append($5);
			$$->append($6 = newNode(".", SQL_NODE_PUNCTUATION));
			$$->append($7);
			}
/*	|       SQL_TOKEN_NAME ';' SQL_TOKEN_NAME '.' SQL_TOKEN_NAME '.' column_val
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2= newNode(";", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(".", SQL_NODE_PUNCTUATION));
			$$->append($5);
			$$->append($6 = newNode(".", SQL_NODE_PUNCTUATION));
			$$->append($7);
			}
*/	;

		/* data types */
column_val:
		column
		{$$ = SQL_NEW_RULE;
			$$->append($1);}
	|	'*'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1 = newNode("*", SQL_NODE_PUNCTUATION));
		}
	;
data_type:
			SQL_TOKEN_CHARACTER
	|       SQL_TOKEN_CHARACTER '(' SQL_TOKEN_INTNUM ')'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));}
	|       SQL_TOKEN_NUMERIC
	|       SQL_TOKEN_NUMERIC '(' SQL_TOKEN_INTNUM ')'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));}
	|       SQL_TOKEN_NUMERIC '(' SQL_TOKEN_INTNUM ',' SQL_TOKEN_INTNUM ')'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(",", SQL_NODE_PUNCTUATION));
			$$->append($5);
			$$->append($6 = newNode(")", SQL_NODE_PUNCTUATION));}
	|       SQL_TOKEN_DECIMAL
	|       SQL_TOKEN_DECIMAL '(' SQL_TOKEN_INTNUM ')'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));}
	|       SQL_TOKEN_DECIMAL '(' SQL_TOKEN_INTNUM ',' SQL_TOKEN_INTNUM ')'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(",", SQL_NODE_PUNCTUATION));
			$$->append($5);
			$$->append($6 = newNode(")", SQL_NODE_PUNCTUATION));}
	|       SQL_TOKEN_INTEGER
	|       SQL_TOKEN_SMALLINT
	|       SQL_TOKEN_FLOAT
	|       SQL_TOKEN_FLOAT '(' SQL_TOKEN_INTNUM ')'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2 = newNode("(", SQL_NODE_PUNCTUATION));
			$$->append($3);
			$$->append($4 = newNode(")", SQL_NODE_PUNCTUATION));}
	|       SQL_TOKEN_REAL
	|       SQL_TOKEN_DOUBLE SQL_TOKEN_PRECISION
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);}
	;

	/* the various things you can name */

column:
		SQL_TOKEN_NAME
	|	SQL_TOKEN_POSITION
		{
			sal_uInt16 nNod = $$->getRuleID();
			delete $$;
			$$ = newNode(xxx_pGLOBAL_SQLPARSER->TokenIDToStr(nNod), SQL_NODE_NAME);
		}
	|	SQL_TOKEN_CHAR_LENGTH
		{
			sal_uInt16 nNod = $$->getRuleID();
			delete $$;
			$$ = newNode(xxx_pGLOBAL_SQLPARSER->TokenIDToStr(nNod), SQL_NODE_NAME);
		}
	|	SQL_TOKEN_EXTRACT
		{
			sal_uInt16 nNod = $$->getRuleID();
			delete $$;
			$$ = newNode(xxx_pGLOBAL_SQLPARSER->TokenIDToStr(nNod), SQL_NODE_NAME);
		}
	;

cursor:	SQL_TOKEN_NAME
			{$$ = SQL_NEW_RULE;
			$$->append($1);}
	;

/***
module:	SQL_TOKEN_NAME
			{$$ = SQL_NEW_RULE;
			$$->append($1);}
	;
***/

parameter:
		':' SQL_TOKEN_NAME
			{$$ = SQL_NEW_RULE;
			$$->append($1 = newNode(":", SQL_NODE_PUNCTUATION));
			$$->append($2);}
	|	'?'
			{$$ = SQL_NEW_RULE;
			$$->append($1 = newNode("?", SQL_NODE_PUNCTUATION));}
	|	'['	SQL_TOKEN_NAME ']'
			{$$ = SQL_NEW_RULE;
			$$->append($1 = newNode("[", SQL_NODE_PUNCTUATION));
			$$->append($2);
			$$->append($3 = newNode("]", SQL_NODE_PUNCTUATION));}
	;

/***
procedure:      SQL_TOKEN_NAME
			{$$ = SQL_NEW_RULE;
			$$->append($1);}
	;
***/

range_variable: SQL_TOKEN_NAME
	;

user:	SQL_TOKEN_NAME
	;

/* PREDICATECHECK RULES */
sql:
		search_condition /* checking predicats */
		{
			if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
			{
				$$ = $1;
			}
			else
				YYERROR;
		}
%%


using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::osl;
using namespace ::dbtools;
//	using namespace connectivity;

//============================================================
//= a helper for static ascii pseudo-unicode strings
//============================================================
// string constants
struct _ConstAsciiString_
{
	sal_Int32 length;
	sal_Char  const* str;

	operator rtl::OUString () const { return rtl::OUString(str, length, RTL_TEXTENCODING_ASCII_US); }
//	operator ::rtl::OUString () const { return ::rtl::OUString(str, length, RTL_TEXTENCODING_ASCII_US); }
	operator const sal_Char * () const { return str; }
	operator ::rtl::OString() const { return str; }
};

#define IMPLEMENT_CONSTASCII_STRING( name, string ) \
	_ConstAsciiString_ const name = { sizeof(string)-1, string }

IMPLEMENT_CONSTASCII_STRING(ERROR_STR_GENERAL, "Syntax error in SQL expression");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_GENERAL_HINT,	"in front of \"#\" expression." );
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_VALUE_NO_LIKE, "The value # can not be used with LIKE!");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_FIELD_NO_LIKE, "LIKE can not be used with this field!");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_COMPARE, "The entered criterion can not be compared with this field!");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_STRING_COMPARE, "The field can not be compared with a string!");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_DATE_COMPARE, "The field can not be compared with a date!");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_REAL_COMPARE,	"The field can not be compared with a floating point number!");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_INT_COMPARE,	"The field can not be compared with a number!");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_TABLE,	"The table \"#\" is unknown in the database!");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_COLUMN,	"The column \"#\" is unknown in the table \"#\"!");

IMPLEMENT_CONSTASCII_STRING(KEY_STR_LIKE, "LIKE");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_NOT, "NOT");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_NULL, "NULL");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_TRUE, "sal_True");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_FALSE, "sal_False");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_IS, "IS");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_BETWEEN, "BETWEEN");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_OR, "OR");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_AND, "AND");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_AVG, "AVG");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_COUNT, "COUNT");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_MAX, "MAX");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_MIN, "MIN");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_SUM, "SUM");

IMPLEMENT_CONSTASCII_STRING(FIELD_STR_LOCALE, "Locale");
IMPLEMENT_CONSTASCII_STRING(FIELD_STR_REALNAME, "RealName");
IMPLEMENT_CONSTASCII_STRING(FIELD_STR_FORMATKEY, "FormatKey");
IMPLEMENT_CONSTASCII_STRING(FIELD_STR_NAME, "Name");
IMPLEMENT_CONSTASCII_STRING(FIELD_STR_TYPE, "Type");
IMPLEMENT_CONSTASCII_STRING(FIELD_STR_NULLDATE, "NullDate");

IMPLEMENT_CONSTASCII_STRING(STR_SQL_TOKEN, "SQL_TOKEN_");

//==========================================================================
//= OParseContext
//==========================================================================
//-----------------------------------------------------------------------------
OParseContext::OParseContext()
{
}

//-----------------------------------------------------------------------------
OParseContext::~OParseContext()
{
}

//-----------------------------------------------------------------------------
::rtl::OUString OParseContext::getErrorMessage(ErrorCode _eCode) const
{
	::rtl::OUString aMsg;
	switch (_eCode)
	{
		case ERROR_GENERAL:					aMsg = ERROR_STR_GENERAL; break;
		case ERROR_GENERAL_HINT:			aMsg = ERROR_STR_GENERAL_HINT; break;
		case ERROR_VALUE_NO_LIKE:			aMsg = ERROR_STR_VALUE_NO_LIKE; break;
		case ERROR_FIELD_NO_LIKE:			aMsg = ERROR_STR_FIELD_NO_LIKE; break;
		case ERROR_INVALID_COMPARE:			aMsg = ERROR_STR_INVALID_COMPARE; break;
		case ERROR_INVALID_INT_COMPARE:		aMsg = ERROR_STR_INVALID_INT_COMPARE; break;
		case ERROR_INVALID_STRING_COMPARE:	aMsg = ERROR_STR_INVALID_STRING_COMPARE; break;
		case ERROR_INVALID_DATE_COMPARE:	aMsg = ERROR_STR_INVALID_DATE_COMPARE; break;
		case ERROR_INVALID_REAL_COMPARE:	aMsg = ERROR_STR_INVALID_REAL_COMPARE; break;
		case ERROR_INVALID_TABLE:			aMsg = ERROR_STR_INVALID_TABLE; break;
		case ERROR_INVALID_COLUMN:			aMsg = ERROR_STR_INVALID_COLUMN; break;
	}
	return aMsg;
}

//-----------------------------------------------------------------------------
::rtl::OString OParseContext::getIntlKeywordAscii(InternationalKeyCode _eKey) const
{
	::rtl::OString aKeyword;
	switch (_eKey)
	{
		case KEY_LIKE:		aKeyword = KEY_STR_LIKE; break;
		case KEY_NOT:		aKeyword = KEY_STR_NOT; break;
		case KEY_NULL:		aKeyword = KEY_STR_NULL; break;
		case KEY_TRUE:		aKeyword = KEY_STR_TRUE; break;
		case KEY_FALSE:		aKeyword = KEY_STR_FALSE; break;
		case KEY_IS:		aKeyword = KEY_STR_IS; break;
		case KEY_BETWEEN:	aKeyword = KEY_STR_BETWEEN; break;
		case KEY_OR:		aKeyword = KEY_STR_OR; break;
		case KEY_AND:		aKeyword = KEY_STR_AND; break;
		case KEY_AVG:		aKeyword = KEY_STR_AVG; break;
		case KEY_COUNT:		aKeyword = KEY_STR_COUNT; break;
		case KEY_MAX:		aKeyword = KEY_STR_MAX; break;
		case KEY_MIN:		aKeyword = KEY_STR_MIN; break;
		case KEY_SUM:		aKeyword = KEY_STR_SUM; break;
	}
	return aKeyword;
}

//-----------------------------------------------------------------------------
OParseContext::InternationalKeyCode OParseContext::getIntlKeyCode(const ::rtl::OString& rToken) const
{
	static OParseContext::InternationalKeyCode Intl_TokenID[] =
	{
		KEY_LIKE, KEY_NOT, KEY_NULL, KEY_TRUE,
		KEY_FALSE, KEY_IS, KEY_BETWEEN, KEY_OR,
		KEY_AND, KEY_AVG, KEY_COUNT, KEY_MAX,
		KEY_MIN, KEY_SUM
	};

	sal_uInt32 nCount = sizeof Intl_TokenID / sizeof Intl_TokenID[0];
	for (sal_uInt32 i = 0; i < nCount; i++)
	{
		::rtl::OString aKey = getIntlKeywordAscii(Intl_TokenID[i]);
		if (rToken.equalsIgnoreCase(aKey))
			return Intl_TokenID[i];
	}

	return KEY_NONE;
}

//------------------------------------------------------------------------------
const Locale& OParseContext::getDefaultLocale()
{
	static Locale aIntl(::rtl::OUString::createFromAscii("en_US"),::rtl::OUString::createFromAscii("en_US"),::rtl::OUString());
	static sal_Bool bInitialized = sal_False;
	if (!bInitialized)
	{	
		// ensure that the two members we're interested in are really set
		// (if the system doesn't know the locale en_US aIntl would be initialized with the
		// system language which may be anything - which we don't want ...)
		// 74342 - 21.03.00 - FS

		// TODO check the decimal sep and thousand sep
//		if(!m_xLocaleData.is())
//			m_xLocaleData = Reference<XLocaleData>(m_xServiceFactory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.i18n.LocaleData")),UNO_QUERY); 
//
//		m_xLocaleData->getLocaleItem(*m_pLocale).
//		aIntl.SetNumThousandSep(',');
//		aIntl.SetNumDecimalSep('.');
		bInitialized = sal_True;
	}
	return aIntl;
}

//==========================================================================
//= misc
//==========================================================================
// Der (leider globale) yylval fuer die Uebergabe von
// Werten vom Scanner an den Parser. Die globale Variable
// wird nur kurzzeitig verwendet, der Parser liest die Variable
// sofort nach dem Scanner-Aufruf in eine gleichnamige eigene
// Member-Variable.

const double fMilliSecondsPerDay = 86400000.0;

//------------------------------------------------------------------------------
Any getNumberFormatProperty(const Reference< ::com::sun::star::util::XNumberFormatsSupplier > &xSupplier,
												   sal_Int32 nKey,
												   const rtl::OUString& aPropertyName)
{
	OSL_ENSHURE(xSupplier.is(), "getNumberFormatProperty : the formatter doesn't implement a supplier !");
	Reference< ::com::sun::star::util::XNumberFormats >  xFormats = xSupplier->getNumberFormats();

	if (xFormats.is())
	{
		try
		{
			Reference< XPropertySet > xProperties(xFormats->getByKey(nKey));
			return xProperties->getPropertyValue(aPropertyName);
		}
		catch( ... )
		{
		}
	}
	return Any();
}

//------------------------------------------------------------------
::rtl::OUString ConvertLikeToken(const OSQLParseNode* pTokenNode, const OSQLParseNode* pEscapeNode, sal_Bool bInternational)
{
	::rtl::OUString aMatchStr;
	if (pTokenNode->isToken())
	{
		sal_Char cEscape = 0;
		if (pEscapeNode->count())
			cEscape = pEscapeNode->getChild(1)->getTokenValue().toChar();

		// Platzhalter austauschen
		aMatchStr = pTokenNode->getTokenValue();
		sal_uInt16 nLen = aMatchStr.getLength();
		const sal_Char* sSearch  = bInternational ? "%_" : "*?";
		const sal_Char* sReplace = bInternational ? "*?" : "%_";
		for (sal_uInt16 i = 0; i < nLen; i++)
		{
			sal_Char c = aMatchStr.getStr()[i];
			if (c == sSearch[0] || c == sSearch[1])
			{
				if (i > 0 && aMatchStr.getStr()[i-1] == cEscape)
					continue;
				else
				{
					sal_Unicode cCharacter = sReplace[(c == sSearch[0]) ? 0 : 1];
					aMatchStr.replaceAt(i , 1, ::rtl::OUString(&cCharacter, 1));
				}
			}
		}
	}
	return aMatchStr;
}

//==========================================================================
//= OSQLParser
//==========================================================================

sal_uInt32			OSQLParser::s_nRuleIDs[OSQLParseNode::rule_count + 1];
OParseContext		OSQLParser::s_aDefaultContext;

sal_Int32			OSQLParser::s_nRefCount	= 0;
::osl::Mutex		OSQLParser::s_aMutex;
OSQLScanner*		OSQLParser::s_pScanner = 0;
OSQLParseNodes*		OSQLParser::s_pGarbageCollector = 0;

//-----------------------------------------------------------------------------
OSQLParser::OSQLParser(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xServiceFactory,const OParseContext* _pContext)
		   :m_pContext(_pContext)
		   ,m_pParseTree(NULL)
		   ,m_pLocale(NULL)
		   ,m_nFormatKey(0)
		   ,m_xServiceFactory(_xServiceFactory)
{


	xxx_pGLOBAL_SQLPARSER = this;

#ifdef SQLYYDEBUG
#ifdef SQLYYDEBUG_ON
	SQLyydebug = 1;
#endif
#endif

	::osl::MutexGuard aGuard(s_aMutex);
	// do we have to initialize the data
	if (s_nRefCount == 0)
	{
		s_pScanner = new OSQLScanner();
		s_pScanner->setScanner();
		s_pGarbageCollector = new OSQLParseNodes();

		if(!m_xLocaleData.is())
			m_xLocaleData = Reference<XLocaleData>(m_xServiceFactory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.i18n.LocaleData")),UNO_QUERY);

		// auf 0 zuruecksetzen
		memset(OSQLParser::s_nRuleIDs,0,sizeof(sal_uInt16) * OSQLParseNode::rule_count+1);
	}
	++s_nRefCount;

	if (m_pContext == NULL)
		// take the default context
		m_pContext = &s_aDefaultContext;
}

//-----------------------------------------------------------------------------
OSQLParser::~OSQLParser()
{
	{
		::osl::MutexGuard aGuard(s_aMutex);
		OSL_ENSHURE(s_nRefCount > 0, "OSQLParser::~OSQLParser() : suspicious call : have a refcount of 0 !");
		if (!--s_nRefCount)
		{
			s_pScanner->setScanner(sal_True);
			delete s_pScanner;
			s_pScanner = NULL;

			delete s_pGarbageCollector;
			s_pGarbageCollector = NULL;
		}
		m_xLocaleData = NULL;
	}
}
// -------------------------------------------------------------------------
void OSQLParser::setParseTree(OSQLParseNode * pNewParseTree)
{
	::osl::MutexGuard aGuard(s_aMutex);
	m_pParseTree = pNewParseTree;
}
//-----------------------------------------------------------------------------
OSQLParseNode* OSQLParser::parseTree(::rtl::OUString& rErrorMessage,
									 const ::rtl::OUString& rStatement,
								     sal_Bool bInternational)
{


	// Guard the parsing
	::osl::MutexGuard aGuard(s_aMutex);
	// must be reset
	xxx_pGLOBAL_SQLPARSER = this;

	// defines how to scan
	s_pScanner->SetRule(s_pScanner->GetSQLRule()); // initial
	s_pScanner->prepareScan(rStatement, m_pContext, bInternational);

	SQLyylval.pParseNode = NULL;
	//	SQLyypvt = NULL;
	m_pParseTree = NULL;
	m_sErrorMessage = ::rtl::OUString();

	// ... und den Parser anwerfen ...
	if (SQLyyparse() != 0)
	{
		// only set the error message, if it's not already set
		if (!m_sErrorMessage.getLength())
			m_sErrorMessage = s_pScanner->getErrorMessage();
		if (!m_sErrorMessage.getLength())
			m_sErrorMessage = m_pContext->getErrorMessage(OParseContext::ERROR_GENERAL);

		rErrorMessage = m_sErrorMessage;

		// clear the garbage collector
		while (!s_pGarbageCollector->empty())
		{
			OSQLParseNode* pNode = *s_pGarbageCollector->begin();
			while (pNode->getParent())
				pNode = pNode->getParent();
			delete pNode;
		}
		return NULL;
	}
	else
	{
		s_pGarbageCollector->clear();

		// Das Ergebnis liefern (den Root Parse Node):

		//	OSL_ENSHURE(Sdbyyval.pParseNode != NULL,"OSQLParser: Parser hat keinen ParseNode geliefert");
		//	return Sdbyyval.pParseNode;
		// geht nicht wegen Bug in MKS YACC-erzeugtem Code (es wird ein falscher ParseNode
		// geliefert).

		// Stattdessen setzt die Parse-Routine jetzt den Member pParseTree
		// - einfach diesen zurueckliefern:
		OSL_ENSHURE(m_pParseTree != NULL,"OSQLParser: Parser hat keinen ParseTree geliefert");
		return m_pParseTree;
	}
}

//	static sal_Char* __READONLY_DATA PREDICATE_CHECK = "PREDICATE ";
//-----------------------------------------------------------------------------
OSQLParseNode* OSQLParser::predicateTree(::rtl::OUString& rErrorMessage, const ::rtl::OUString& rStatement,
 										 const Reference< ::com::sun::star::util::XNumberFormatter > & xFormatter,
										 const Reference< XPropertySet > & xField)
{


	// mutex for parsing
	static ::osl::Mutex aMutex;

	// Guard the parsing
	::osl::MutexGuard aGuard(s_aMutex);
	// must be reset
	xxx_pGLOBAL_SQLPARSER = this;

	// reset the parser
	if (!m_pLocale)
		m_pLocale = new Locale(m_pContext->getDefaultLocale());

	m_xField		= xField;
	m_xFormatter	= xFormatter;

	if (m_xField.is())
	{
		sal_Int32 nType=0;
		try
		{
			// get the field name
			rtl::OUString aString;

			// retrieve the fields name
			// #75243# use the RealName of the column if there is any otherwise the name which could be the alias
			// of the field
			if (m_xField->getPropertySetInfo()->hasPropertyByName(FIELD_STR_REALNAME))
				m_xField->getPropertyValue(FIELD_STR_REALNAME) >>= aString;
			else
				m_xField->getPropertyValue(FIELD_STR_NAME) >>= aString;

			m_sFieldName = aString;

			// get the field format key
			if (m_xField->getPropertySetInfo()->hasPropertyByName(FIELD_STR_FORMATKEY))
				m_xField->getPropertyValue(FIELD_STR_FORMATKEY) >>= m_nFormatKey;
			else
				m_nFormatKey = 0;

			// get the field type
			m_xField->getPropertyValue(FIELD_STR_TYPE) >>= nType;
		}
		catch ( Exception& )
		{
			OSL_ASSERT(0);
		}

		if (m_nFormatKey && m_xFormatter.is())
		{
			Any aValue = getNumberFormatProperty(m_xFormatter->getNumberFormatsSupplier(), m_nFormatKey, FIELD_STR_LOCALE);
			OSL_ENSHURE(aValue.getValueType() == ::getCppuType((const ::com::sun::star::lang::Locale*)0), "OSQLParser::PredicateTree : invalid language property !");

			if (aValue.getValueType() == ::getCppuType((const ::com::sun::star::lang::Locale*)0))
				aValue >>= *m_pLocale;
		}
		else
			*m_pLocale = m_pContext->getDefaultLocale();

		switch (nType)
		{
			case DataType::DATE:
			case DataType::TIME:
			case DataType::TIMESTAMP:
				s_pScanner->SetRule(s_pScanner->GetDATERule());
				break;
			case DataType::CHAR:
			case DataType::VARCHAR:
			case DataType::LONGVARCHAR:
				s_pScanner->SetRule(s_pScanner->GetSTRINGRule());
				break;
			default:
				if (m_pLocale && m_xLocaleData->getLocaleItem(*m_pLocale).decimalSeparator.toChar() == ',')
					s_pScanner->SetRule(s_pScanner->GetGERRule());
				else
					s_pScanner->SetRule(s_pScanner->GetENGRule());
		}

	}
	else
		s_pScanner->SetRule(s_pScanner->GetSQLRule());

	s_pScanner->prepareScan(rStatement, m_pContext, sal_True);

	SQLyylval.pParseNode = NULL;
	//	SQLyypvt = NULL;
	m_pParseTree = NULL;
	m_sErrorMessage= ::rtl::OUString();

	// ... und den Parser anwerfen ...
	if (SQLyyparse() != 0)
	{
		m_sFieldName= ::rtl::OUString();
		m_xField = NULL;
		m_xFormatter = NULL;
		m_nFormatKey = 0;

		if (!m_sErrorMessage.getLength())
			m_sErrorMessage = s_pScanner->getErrorMessage();
		if (!m_sErrorMessage.getLength())
			m_sErrorMessage = m_pContext->getErrorMessage(OParseContext::ERROR_GENERAL);

		rErrorMessage = m_sErrorMessage;

		// clear the garbage collector
		while (!s_pGarbageCollector->empty())
		{
			OSQLParseNode* pNode = *s_pGarbageCollector->begin();
			while (pNode->getParent())
				pNode = pNode->getParent();
			delete pNode;
		}
		return NULL;
	}
	else
	{
		s_pGarbageCollector->clear();

		m_sFieldName= ::rtl::OUString();
		m_xField = NULL;
		m_xFormatter = NULL;
		m_nFormatKey = 0;

		// Das Ergebnis liefern (den Root Parse Node):

		// Stattdessen setzt die Parse-Routine jetzt den Member pParseTree
		// - einfach diesen zurueckliefern:
		OSL_ENSHURE(m_pParseTree != NULL,"OSQLParser: Parser hat keinen ParseTree geliefert");
		return m_pParseTree;
	}
}

//-----------------------------------------------------------------------------
::rtl::OString OSQLParser::TokenIDToStr(sal_uInt32 nTokenID, const OParseContext* pContext)
{
	::rtl::OString aStr;
	if (pContext)
		aStr = pContext->getIntlKeywordAscii((OParseContext::InternationalKeyCode)nTokenID);

	if (!aStr.getLength())
	{
		aStr = yytname[YYTRANSLATE(nTokenID)];
		if(!aStr.compareTo("SQL_TOKEN_",10))
			aStr = aStr.copy(10);
	}
	return aStr;
}

//-----------------------------------------------------------------------------
/*sal_uInt32 OSQLParser::StrToTokenID(const ::rtl::OString & rName)
{
	::rtl::OString aName;
	if (rName.IsAlphaNumericAscii())
		aName = rName;
	else
	{
		aName = "'";
		aName += rName;
		aName += "'";
	}

	// Gewuenschten Token-Namen suchen:
	for (sal_uInt32 i = 0; i < SQLyyntoken; i++)
	{
		if (aName == TokenTypes[i].name)
			return TokenTypes[i].token;
	}

	// Nicht gefunden
	return 0;
}*/

//-----------------------------------------------------------------------------
::rtl::OUString OSQLParser::RuleIDToStr(sal_uInt32 nRuleID)
{
	OSL_ENSHURE(nRuleID >= (sizeof yytname/sizeof yytname[0]), "Invalid nRuleId!");
	return ::rtl::OUString::createFromAscii(yytname[nRuleID]);
}

//-----------------------------------------------------------------------------
sal_uInt32 OSQLParser::StrToRuleID(const ::rtl::OString & rValue)
{
	// In yysvar nach dem angegebenen Namen suchen, den ::com::sun::star::sdbcx::Index zurueckliefern
	// (oder 0, wenn nicht gefunden)
	static sal_Int32 nLen = sizeof(yytname)/sizeof(yytname[0]);
	for (sal_uInt32 i = YYTRANSLATE(SQL_TOKEN_INVALIDSYMBOL); i < (nLen-1); i++)
	{
		if (yytname && rValue == yytname[i])
			return i;
	}

	// Nicht gefunden
	return 0;
}

//-----------------------------------------------------------------------------
sal_uInt32 OSQLParser::RuleID(OSQLParseNode::Rule eRule)
{
	if (!s_nRuleIDs[eRule])
	{
		switch (eRule)
		{
			case OSQLParseNode::select_statement:
				s_nRuleIDs[eRule] = StrToRuleID("select_statement"); break;
			case OSQLParseNode::from_clause:
				s_nRuleIDs[eRule] = StrToRuleID("from_clause"); break;
			case OSQLParseNode::table_ref_commalist:
				s_nRuleIDs[eRule] = StrToRuleID("table_ref_commalist"); break;
			case OSQLParseNode::table_exp:
				s_nRuleIDs[eRule] = StrToRuleID("table_exp"); break;
			case OSQLParseNode::table_ref:
				s_nRuleIDs[eRule] = StrToRuleID("table_ref"); break;
			case OSQLParseNode::catalog_name:
				s_nRuleIDs[eRule] = StrToRuleID("catalog_name"); break;
			case OSQLParseNode::schema_name:
				s_nRuleIDs[eRule] = StrToRuleID("schema_name"); break;
			case OSQLParseNode::table_name:
				s_nRuleIDs[eRule] = StrToRuleID("table_name"); break;
			case OSQLParseNode::opt_column_commalist:
				s_nRuleIDs[eRule] = StrToRuleID("opt_column_commalist"); break;
			case OSQLParseNode::column_commalist:
				s_nRuleIDs[eRule] = StrToRuleID("column_commalist"); break;
			case OSQLParseNode::column_ref_commalist:
				s_nRuleIDs[eRule] = StrToRuleID("column_ref_commalist"); break;
			case OSQLParseNode::column_ref:
				s_nRuleIDs[eRule] = StrToRuleID("column_ref"); break;
			case OSQLParseNode::opt_order_by_clause:
				s_nRuleIDs[eRule] = StrToRuleID("opt_order_by_clause"); break;
			case OSQLParseNode::ordering_spec_commalist:
				s_nRuleIDs[eRule] = StrToRuleID("ordering_spec_commalist"); break;
			case OSQLParseNode::ordering_spec:
				s_nRuleIDs[eRule] = StrToRuleID("ordering_spec"); break;
			case OSQLParseNode::opt_asc_desc:
				s_nRuleIDs[eRule] = StrToRuleID("opt_asc_desc"); break;
			case OSQLParseNode::where_clause:
				s_nRuleIDs[eRule] = StrToRuleID("where_clause"); break;
			case OSQLParseNode::opt_where_clause:
				s_nRuleIDs[eRule] = StrToRuleID("opt_where_clause"); break;
			case OSQLParseNode::search_condition:
				s_nRuleIDs[eRule] = StrToRuleID("search_condition"); break;
			case OSQLParseNode::comparison_predicate:
				s_nRuleIDs[eRule] = StrToRuleID("comparison_predicate"); break;
			case OSQLParseNode::between_predicate:
				s_nRuleIDs[eRule] = StrToRuleID("between_predicate"); break;
			case OSQLParseNode::like_predicate:
				s_nRuleIDs[eRule] = StrToRuleID("like_predicate"); break;
			case OSQLParseNode::opt_escape:
				s_nRuleIDs[eRule] = StrToRuleID("opt_escape"); break;
			case OSQLParseNode::test_for_null:
				s_nRuleIDs[eRule] = StrToRuleID("test_for_null"); break;
			case OSQLParseNode::scalar_exp_commalist:
				s_nRuleIDs[eRule] = StrToRuleID("scalar_exp_commalist"); break;
			case OSQLParseNode::scalar_exp:
				s_nRuleIDs[eRule] = StrToRuleID("scalar_exp"); break;
			case OSQLParseNode::parameter_ref:
				s_nRuleIDs[eRule] = StrToRuleID("parameter_ref"); break;
			case OSQLParseNode::parameter:
				s_nRuleIDs[eRule] = StrToRuleID("parameter"); break;
			case OSQLParseNode::general_set_fct:
				s_nRuleIDs[eRule] = StrToRuleID("general_set_fct"); break;
			case OSQLParseNode::range_variable:
				s_nRuleIDs[eRule] = StrToRuleID("range_variable"); break;
			case OSQLParseNode::column:
				s_nRuleIDs[eRule] = StrToRuleID("column"); break;
			case OSQLParseNode::delete_statement_positioned:
				s_nRuleIDs[eRule] = StrToRuleID("delete_statement_positioned"); break;
			case OSQLParseNode::delete_statement_searched:
				s_nRuleIDs[eRule] = StrToRuleID("delete_statement_searched"); break;
			case OSQLParseNode::update_statement_positioned:
				s_nRuleIDs[eRule] = StrToRuleID("update_statement_positioned"); break;
			case OSQLParseNode::update_statement_searched:
				s_nRuleIDs[eRule] = StrToRuleID("update_statement_searched"); break;
			case OSQLParseNode::assignment_commalist:
				s_nRuleIDs[eRule] = StrToRuleID("assignment_commalist"); break;
			case OSQLParseNode::assignment:
				s_nRuleIDs[eRule] = StrToRuleID("assignment"); break;
			case OSQLParseNode::values_or_query_spec:
				s_nRuleIDs[eRule] = StrToRuleID("values_or_query_spec"); break;
			case OSQLParseNode::insert_statement:
				s_nRuleIDs[eRule] = StrToRuleID("insert_statement"); break;
			case OSQLParseNode::insert_atom_commalist:
				s_nRuleIDs[eRule] = StrToRuleID("insert_atom_commalist"); break;
			case OSQLParseNode::insert_atom:
				s_nRuleIDs[eRule] = StrToRuleID("insert_atom"); break;
			case OSQLParseNode::predicate_check:
				s_nRuleIDs[eRule] = StrToRuleID("predicate_check"); break;
			case OSQLParseNode::qualified_join:
				s_nRuleIDs[eRule] = StrToRuleID("qualified_join"); break;
			case OSQLParseNode::cross_union:
				s_nRuleIDs[eRule] = StrToRuleID("cross_union"); break;
			case OSQLParseNode::select_sublist:
				s_nRuleIDs[eRule] = StrToRuleID("select_sublist"); break;
			case OSQLParseNode::derived_column:
				s_nRuleIDs[eRule] = StrToRuleID("derived_column"); break;
			case OSQLParseNode::column_val:
				s_nRuleIDs[eRule] = StrToRuleID("column_val"); break;
			case OSQLParseNode::set_fct_spec:
				s_nRuleIDs[eRule] = StrToRuleID("set_fct_spec"); break;
			case OSQLParseNode::boolean_term:
				s_nRuleIDs[eRule] = StrToRuleID("boolean_term"); break;
			case OSQLParseNode::boolean_primary:
				s_nRuleIDs[eRule] = StrToRuleID("boolean_primary"); break;
			case OSQLParseNode::num_value_exp:
				s_nRuleIDs[eRule] = StrToRuleID("num_value_exp"); break;
			case OSQLParseNode::join_type:
				s_nRuleIDs[eRule] = StrToRuleID("join_type"); break;
			case OSQLParseNode::position_exp:
				s_nRuleIDs[eRule] = StrToRuleID("position_exp"); break;
			case OSQLParseNode::extract_exp:
				s_nRuleIDs[eRule] = StrToRuleID("extract_exp"); break;
			case OSQLParseNode::length_exp:
				s_nRuleIDs[eRule] = StrToRuleID("length_exp"); break;
			case OSQLParseNode::char_value_fct:
				s_nRuleIDs[eRule] = StrToRuleID("char_value_fct"); break;
			case OSQLParseNode::odbc_call_spec:
				s_nRuleIDs[eRule] = StrToRuleID("odbc_call_spec"); break;
			case OSQLParseNode::in_predicate:
				s_nRuleIDs[eRule] = StrToRuleID("in_predicate"); break;
			case OSQLParseNode::existence_test:
				s_nRuleIDs[eRule] = StrToRuleID("existence_test"); break;
			case OSQLParseNode::unique_test:
				s_nRuleIDs[eRule] = StrToRuleID("unique_test"); break;
			case OSQLParseNode::all_or_any_predicate:
				s_nRuleIDs[eRule] = StrToRuleID("all_or_any_predicate"); break;
			case OSQLParseNode::named_columns_join:
				s_nRuleIDs[eRule] = StrToRuleID("named_columns_join"); break;
			case OSQLParseNode::join_condition:
				s_nRuleIDs[eRule] = StrToRuleID("join_condition"); break;
			case OSQLParseNode::joined_table:
				s_nRuleIDs[eRule] = StrToRuleID("joined_table"); break;
			case OSQLParseNode::boolean_factor:
				s_nRuleIDs[eRule] = StrToRuleID("boolean_factor"); break;
			case OSQLParseNode::not:
				s_nRuleIDs[eRule] = StrToRuleID("not"); break;
			case OSQLParseNode::boolean_test:
				s_nRuleIDs[eRule] = StrToRuleID("boolean_test"); break;
			case OSQLParseNode::manipulative_statement:
				s_nRuleIDs[eRule] = StrToRuleID("manipulative_statement"); break;
			case OSQLParseNode::subquery:
				s_nRuleIDs[eRule] = StrToRuleID("subquery"); break;
			case OSQLParseNode::value_exp_commalist:
				s_nRuleIDs[eRule] = StrToRuleID("value_exp_commalist"); break;
			case OSQLParseNode::odbc_fct_spec:
				s_nRuleIDs[eRule] = StrToRuleID("odbc_fct_spec"); break;
			case OSQLParseNode::union_statement:
				s_nRuleIDs[eRule] = StrToRuleID("union_statement"); break;
			case OSQLParseNode::outer_join_type:
				s_nRuleIDs[eRule] = StrToRuleID("outer_join_type"); break;
			case OSQLParseNode::char_value_exp:
				s_nRuleIDs[eRule] = StrToRuleID("char_value_exp"); break;
			case OSQLParseNode::term:
				s_nRuleIDs[eRule] = StrToRuleID("term"); break;
			case OSQLParseNode::value_exp_primary:
				s_nRuleIDs[eRule] = StrToRuleID("value_exp_primary"); break;
			case OSQLParseNode::value_exp:
				s_nRuleIDs[eRule] = StrToRuleID("value_exp"); break;
			default:
				OSL_ASSERT("interner Fehler: Regel nicht bekannt, in OSQLParser::RuleID nachtragen!");
		}
	}
	return s_nRuleIDs[(sal_uInt16)eRule];
}
// -------------------------------------------------------------------------
::rtl::OUString OSQLParser::stringToDouble(const ::rtl::OUString& _rValue,sal_Int16 _nScale)
{
	::rtl::OUString aValue;
	if(!m_xCharClass.is())
		m_xCharClass  = Reference<XCharacterClassification>(m_xServiceFactory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.i18n.CharacterClassification")),UNO_QUERY);
	if(m_xCharClass.is() && m_xLocaleData.is())
	{
		try
		{
			ParseResult aResult = m_xCharClass->parsePredefinedToken(KParseType::ANY_NUMBER,_rValue,0,*m_pLocale,0,::rtl::OUString(),KParseType::ANY_NUMBER,::rtl::OUString());
			if((aResult.TokenType & KParseType::IDENTNAME) && aResult.EndPos == _rValue.getLength())
			{
				aValue = ::rtl::OUString::valueOf(aResult.Value);
				sal_Int32 nPos = aValue.lastIndexOf(::rtl::OUString::createFromAscii("."));
				if((nPos+_nScale) < aValue.getLength())
					aValue = aValue.replaceAt(nPos+_nScale,aValue.getLength()-nPos-_nScale,::rtl::OUString());
				aValue.replaceAt(aValue.lastIndexOf(nPos),1,m_xLocaleData->getLocaleItem(*m_pLocale).decimalSeparator);
				return aValue;
			}
		}
		catch(Exception&)
		{
		}
	}
	return aValue;
}
//-----------------------------------------------------------------------------
sal_Int16 OSQLParser::buildNode(OSQLParseNode*& pAppend,OSQLParseNode* pLiteral,OSQLParseNode*& pCompare)
{
	OSQLParseNode* pColumnRef = new OSQLInternalNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
	pColumnRef->append(new OSQLInternalNode(m_sFieldName,SQL_NODE_NAME));
	OSQLParseNode* pComp = new OSQLInternalNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::comparison_predicate));
	pComp->append(pColumnRef);
	pComp->append(pCompare);
	pComp->append(pLiteral);
	pAppend->append(pComp);
	return 1;
}

//-----------------------------------------------------------------------------
sal_Int16 OSQLParser::buildNode_STR_NUM(OSQLParseNode*& pAppend,OSQLParseNode*& pLiteral,OSQLParseNode*& pCompare)
{
	OSQLParseNode* pColumnRef = new OSQLInternalNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
	pColumnRef->append(new OSQLInternalNode(m_sFieldName,SQL_NODE_NAME));
	OSQLParseNode* pComp = new OSQLInternalNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::comparison_predicate));
	pComp->append(pColumnRef);
	pComp->append(pCompare);

	if (m_nFormatKey)
	{
		sal_Int16 nScale = 0;
		::rtl::OUString aDec;
		try
		{
			Any aValue = getNumberFormatProperty(m_xFormatter->getNumberFormatsSupplier(),
										m_nFormatKey, rtl::OUString::createFromAscii("Decimals"));
			aValue >>= nScale;
		}
		catch ( ... )
		{
		}

		pComp->append(new OSQLInternalNode(stringToDouble(pLiteral->getTokenValue(),nScale),SQL_NODE_STRING));
	}
	else
		pComp->append(new OSQLInternalNode(pLiteral->getTokenValue(),SQL_NODE_STRING));

	pAppend->append(pComp);

	delete pLiteral;
	pLiteral = NULL;

	return 1;
}

//-----------------------------------------------------------------------------
sal_Int16 OSQLParser::buildNode_Date(const double& fValue, sal_Int16 nType, OSQLParseNode*& pAppend,OSQLParseNode* pLiteral,OSQLParseNode*& pCompare)
{
	OSQLParseNode* pColumnRef = new OSQLInternalNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
	pColumnRef->append(new OSQLInternalNode(m_sFieldName,SQL_NODE_NAME));
	OSQLParseNode* pComp = new OSQLInternalNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::comparison_predicate));
	pComp->append(pColumnRef);
	pComp->append(pCompare);

	OSQLParseNode* pNewNode = new OSQLInternalNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::set_fct_spec));
	pNewNode->append(new OSQLInternalNode(::rtl::OUString::createFromAscii("{"), SQL_NODE_PUNCTUATION));
	OSQLParseNode* pDateNode = new OSQLInternalNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::odbc_fct_spec));
	pNewNode->append(pDateNode);
	pNewNode->append(new OSQLInternalNode(::rtl::OUString::createFromAscii("}"), SQL_NODE_PUNCTUATION));

	switch (nType)
	{
		case DataType::DATE:
		{
			Date aDate = DBTypeConversion::toDate(fValue,DBTypeConversion::getNULLDate(m_xFormatter->getNumberFormatsSupplier()));
			::rtl::OUString aString = DBTypeConversion::toDateString(aDate);
			pDateNode->append(new OSQLInternalNode(aEmptyString, SQL_NODE_KEYWORD, SQL_TOKEN_D));
			pDateNode->append(new OSQLInternalNode(aString, SQL_NODE_STRING));
			break;
		}
		case DataType::TIME:
		{
			Time aTime = DBTypeConversion::toTime(fValue);
			::rtl::OUString aString = DBTypeConversion::toTimeString(aTime);
			pDateNode->append(new OSQLInternalNode(aEmptyString, SQL_NODE_KEYWORD, SQL_TOKEN_T));
			pDateNode->append(new OSQLInternalNode(aString, SQL_NODE_STRING));
			break;
		}
		case DataType::TIMESTAMP:
		{
			DateTime aDateTime = DBTypeConversion::toDateTime(fValue,DBTypeConversion::getNULLDate(m_xFormatter->getNumberFormatsSupplier()));
			if (aDateTime.Seconds && aDateTime.Minutes && aDateTime.Hours)
			{
				::rtl::OUString aString = DBTypeConversion::toDateTimeString(aDateTime);
				pDateNode->append(new OSQLInternalNode(aEmptyString, SQL_NODE_KEYWORD, SQL_TOKEN_TS));
				pDateNode->append(new OSQLInternalNode(aString, SQL_NODE_STRING));
			}
			else
			{
				Date aDate(aDateTime.Day,aDateTime.Month,aDateTime.Year);
				pDateNode->append(new OSQLInternalNode(aEmptyString, SQL_NODE_KEYWORD, SQL_TOKEN_D));
				pDateNode->append(new OSQLInternalNode(DBTypeConversion::toDateString(aDate), SQL_NODE_STRING));
			}
			break;
		}
	}

	pComp->append(pNewNode);
	pAppend->append(pComp);

	delete pLiteral;
	pLiteral = NULL;

	return 1;
}

//-----------------------------------------------------------------------------
sal_Int16 OSQLParser::buildLikeRule(OSQLParseNode*& pAppend, OSQLParseNode*& pLiteral, const OSQLParseNode* pEscape)
{
	sal_Int16 nErg = 0;
	sal_Int32 nType = 0;

	if (!m_xField.is())
		return nErg;
	try
	{
		Any aValue;
		{
			aValue = m_xField->getPropertyValue(FIELD_STR_TYPE);
			aValue >>= nType;
		}
	}
	catch ( ... )
	{
		return nErg;
	}

	switch (nType)
	{
		case DataType::CHAR:
		case DataType::VARCHAR:
		case DataType::LONGVARCHAR:
			if(pLiteral->isRule())
			{
				pAppend->append(pLiteral);
				nErg = 1;
			}
			else
			{
				switch(pLiteral->getNodeType())
				{
					case SQL_NODE_STRING:
						pLiteral->m_aNodeValue = ConvertLikeToken(pLiteral, pEscape, sal_False);
						pAppend->append(pLiteral);
						nErg = 1;
						break;
					case SQL_NODE_APPROXNUM:
						if (m_xFormatter.is() && m_nFormatKey)
						{
							sal_Int16 nScale = 0;
							try
							{
								Any aValue = getNumberFormatProperty(m_xFormatter->getNumberFormatsSupplier(),
															m_nFormatKey, rtl::OUString::createFromAscii("Decimals"));
								aValue >>= nScale;
							}
							catch ( ... )
							{
							}

							pAppend->append(new OSQLInternalNode(stringToDouble(pLiteral->getTokenValue(),nScale),SQL_NODE_STRING));
						}
						else
							pAppend->append(new OSQLInternalNode(pLiteral->getTokenValue(),SQL_NODE_STRING));

						delete pLiteral;
						nErg = 1;
						break;
					default:
					{
						m_sErrorMessage = m_pContext->getErrorMessage(OParseContext::ERROR_VALUE_NO_LIKE);
						m_sErrorMessage.replaceAt(m_sErrorMessage.indexOf(::rtl::OUString::createFromAscii("#1")),2,pLiteral->getTokenValue());
					}
				}
			}
			break;
		default:
			m_sErrorMessage = m_pContext->getErrorMessage(OParseContext::ERROR_FIELD_NO_LIKE);
	}
	return nErg;
}
//-----------------------------------------------------------------------------
sal_Int16 OSQLParser::buildStringNodes(OSQLParseNode*& pLiteral)
{
	if(!pLiteral)
		return 1;

	if(SQL_ISRULE(pLiteral,set_fct_spec) || SQL_ISRULE(pLiteral,general_set_fct) || SQL_ISRULE(pLiteral,column_ref)
		|| SQL_ISRULE(pLiteral,subquery))
		return 1; // here I have a function that I can't transform into a string

	if(pLiteral->getNodeType() == SQL_NODE_INTNUM || pLiteral->getNodeType() == SQL_NODE_APPROXNUM || pLiteral->getNodeType() == SQL_NODE_ACCESS_DATE)
	{
		OSQLParseNode* pParent = pLiteral->getParent();

		OSQLParseNode* pNewNode = new OSQLInternalNode(pLiteral->getTokenValue(), SQL_NODE_STRING);
		pParent->replace(pLiteral, pNewNode);
		delete pLiteral;
		pLiteral = NULL;
		return 1;
	}

	for(sal_uInt32 i=0;i<pLiteral->count();++i)
	{
		OSQLParseNode* pChild = pLiteral->getChild(i);
		buildStringNodes(pChild);
	}
	if(SQL_ISRULE(pLiteral,term) || SQL_ISRULE(pLiteral,value_exp_primary))
	{
		m_sErrorMessage = m_pContext->getErrorMessage(OParseContext::ERROR_INVALID_COMPARE);
		return 0;
	}
	return 1;
}
//-----------------------------------------------------------------------------
sal_Int16 OSQLParser::buildComparsionRule(OSQLParseNode*& pAppend,OSQLParseNode* pLiteral)
{
	OSQLParseNode* pComp = new OSQLInternalNode(::rtl::OUString::createFromAscii("="), SQL_NODE_EQUAL);
	return buildComparsionRule(pAppend,pLiteral,pComp);
}

//-----------------------------------------------------------------------------
sal_Int16 OSQLParser::buildComparsionRule(OSQLParseNode*& pAppend,OSQLParseNode* pLiteral,OSQLParseNode*& pCompare)
{
	sal_Int16 nErg = 0;
	if (m_xField.is())
	{
		sal_Int32 nType = 0;
		try
		{
			Any aValue;
			{
				aValue = m_xField->getPropertyValue(FIELD_STR_TYPE);
				aValue >>= nType;
			}
		}
		catch ( ... )
		{
			return nErg;
		}

		if (pLiteral->isRule() && !SQL_ISRULE(pLiteral,value_exp))
		{
			switch(nType)
			{
				case DataType::CHAR:
				case DataType::VARCHAR:
				case DataType::LONGVARCHAR:
					if(!SQL_ISRULE(pLiteral,char_value_exp) && !buildStringNodes(pLiteral))
						break;
				default:
					nErg = buildNode(pAppend,pLiteral,pCompare);
			}
		}
		else
		{
			switch(pLiteral->getNodeType())
			{
				case SQL_NODE_STRING:
					switch(nType)
					{
						case DataType::CHAR:
						case DataType::VARCHAR:
						case DataType::LONGVARCHAR:
							nErg = buildNode(pAppend,pLiteral,pCompare);
							break;
						case DataType::DATE:
						case DataType::TIME:
						case DataType::TIMESTAMP:
							if (m_xFormatter.is())
							{
								try
								{
									// do we have a date
									double fValue = m_xFormatter->convertStringToNumber(m_nFormatKey, pLiteral->getTokenValue().getStr());
									nErg = buildNode_Date(fValue, nType, pAppend,pLiteral,pCompare);
								}
								catch ( ... )
								{
									try
									{
										Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xFormatSup = m_xFormatter->getNumberFormatsSupplier();
										Reference< ::com::sun::star::util::XNumberFormatTypes >  xFormatTypes(xFormatSup->getNumberFormats(),UNO_QUERY);
										if (xFormatTypes.is())
										{
											double fValue = m_xFormatter->convertStringToNumber(
												xFormatTypes->getStandardFormat(::com::sun::star::util::NumberFormat::DATE, *m_pLocale),
																				pLiteral->getTokenValue().getStr());
											nErg = buildNode_Date(fValue, nType, pAppend,pLiteral,pCompare);
										}
										else
										{
											nErg = -1;
											m_sErrorMessage = m_pContext->getErrorMessage(OParseContext::ERROR_INVALID_DATE_COMPARE);
										}

									}
									catch ( ... )
									{
										nErg = -1;
										m_sErrorMessage = m_pContext->getErrorMessage(OParseContext::ERROR_INVALID_DATE_COMPARE);
									}
								}
							}
							else
								nErg = buildNode(pAppend,pLiteral,pCompare);

							break;
						default:
							m_sErrorMessage = m_pContext->getErrorMessage(OParseContext::ERROR_INVALID_DATE_COMPARE);
					}
					break;
				case SQL_NODE_ACCESS_DATE:
					switch(nType)
					{
						case DataType::DATE:
						case DataType::TIME:
						case DataType::TIMESTAMP:
							if (m_xFormatter.is())
							{
								try
								{
									// do we have a date
									double fValue = m_xFormatter->convertStringToNumber(m_nFormatKey, pLiteral->getTokenValue().getStr());
									nErg = buildNode_Date(fValue, nType, pAppend,pLiteral,pCompare);
								}
								catch ( ... )
								{
									try
									{
										Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xFormatSup = m_xFormatter->getNumberFormatsSupplier();
										Reference< ::com::sun::star::util::XNumberFormatTypes >  xFormatTypes(xFormatSup->getNumberFormats(),UNO_QUERY);
										if (xFormatTypes.is())
										{
											double fValue = m_xFormatter->convertStringToNumber(
												xFormatTypes->getStandardFormat(::com::sun::star::util::NumberFormat::DATE, *m_pLocale),
																pLiteral->getTokenValue().getStr());
											nErg = buildNode_Date(fValue, nType, pAppend,pLiteral,pCompare);
										}
										else
										{
											nErg = -1;
											m_sErrorMessage = m_pContext->getErrorMessage(OParseContext::ERROR_INVALID_DATE_COMPARE);
										}
									}
									catch ( ... )
									{
										nErg = -1;
										m_sErrorMessage = m_pContext->getErrorMessage(OParseContext::ERROR_INVALID_DATE_COMPARE);
									}
								}
							}
							else
							{
								nErg = -1;
								m_sErrorMessage = m_pContext->getErrorMessage(OParseContext::ERROR_INVALID_DATE_COMPARE);
							}
							break;
						default:
							m_sErrorMessage = m_pContext->getErrorMessage(OParseContext::ERROR_INVALID_DATE_COMPARE);
					}
					break;
				case SQL_NODE_INTNUM:
					switch(nType)
					{
						case DataType::BIT:
						case DataType::DECIMAL:
						case DataType::NUMERIC:
						case DataType::TINYINT:
						case DataType::SMALLINT:
						case DataType::INTEGER:
						case DataType::BIGINT:
						case DataType::REAL:
						case DataType::DOUBLE:
							// kill thousand seperators if any
							if (m_xLocaleData->getLocaleItem(*m_pLocale).decimalSeparator.toChar() == ',' )
							{
								pLiteral->m_aNodeValue.replace('.', sal_Unicode());
								// and replace decimal
								pLiteral->m_aNodeValue.replace(',', '.');
							}
							else
								pLiteral->m_aNodeValue.replace(',', sal_Unicode());
							nErg = buildNode(pAppend,pLiteral,pCompare);
							break;
						case DataType::CHAR:
						case DataType::VARCHAR:
						case DataType::LONGVARCHAR:
							nErg = buildNode_STR_NUM(pAppend,pLiteral,pCompare);
							break;
						default:
							m_sErrorMessage = m_pContext->getErrorMessage(OParseContext::ERROR_INVALID_INT_COMPARE);
					}
					break;
				case SQL_NODE_APPROXNUM:
					switch(nType)
					{
						case DataType::DECIMAL:
						case DataType::NUMERIC:
						case DataType::REAL:
						case DataType::DOUBLE:
							if (inPredicateCheck())
							{
								// kill thousand seperators if any
								if (m_xLocaleData->getLocaleItem(*m_pLocale).decimalSeparator.toChar() == ',' )
								{
									pLiteral->m_aNodeValue.replace('.', sal_Unicode());
									// and replace decimal
									pLiteral->m_aNodeValue.replace(',', '.');
								}
								else
									pLiteral->m_aNodeValue.replace(',', sal_Unicode());
							}
							nErg = buildNode(pAppend,pLiteral,pCompare);
							break;
						case DataType::CHAR:
						case DataType::VARCHAR:
						case DataType::LONGVARCHAR:
							nErg = buildNode_STR_NUM(pAppend,pLiteral,pCompare);
							break;
						case DataType::INTEGER:
						default:
							m_sErrorMessage = m_pContext->getErrorMessage(OParseContext::ERROR_INVALID_REAL_COMPARE);
					}
					break;
			}
		}
		if (!nErg)
			--nErg;
	}
	if (!pCompare->getParent()) // I have no parent so I was not used and I must die :-)
		delete pCompare;
	return nErg;
}

//-----------------------------------------------------------------------------
void OSQLParser::reduceLiteral(OSQLParseNode*& pLiteral, sal_Bool bAppendBlank)
{
	OSL_ENSHURE(pLiteral->isRule(), "This is no ::com::sun::star::chaos::Rule");
	OSL_ENSHURE(pLiteral->count() == 2, "OSQLParser::ReduceLiteral() Invalid count");
	OSQLParseNode* pTemp = pLiteral;
	::rtl::OUString aValue;
	if (bAppendBlank)
	{
		((aValue = pLiteral->getChild(0)->getTokenValue()) += ::rtl::OUString::createFromAscii(" ")) +=
					pLiteral->getChild(1)->getTokenValue();
	}
	else
		(aValue = pLiteral->getChild(0)->getTokenValue()) +=
					pLiteral->getChild(1)->getTokenValue();

	pLiteral = new OSQLInternalNode(aValue,SQL_NODE_STRING);
	delete pTemp;
}
// -------------------------------------------------------------------------
void OSQLParser::error(sal_Char *fmt)
{
	if(!m_sErrorMessage.getLength())
	{
		::rtl::OUString sStr(fmt,strlen(fmt),RTL_TEXTENCODING_UTF8);
		::rtl::OUString sSQL_TOKEN(::rtl::OUString::createFromAscii("SQL_TOKEN_"));

		sal_Int32 nPos1 = sStr.indexOf(sSQL_TOKEN);
		if(nPos1 != -1)
		{
			::rtl::OUString sFirst  = sStr.copy(0,nPos1);
			sal_Int32 nPos2 = sStr.indexOf(sSQL_TOKEN,nPos1+1);
			if(nPos2 != -1)
			{
				::rtl::OUString sSecond = sStr.copy(nPos1+sSQL_TOKEN.getLength(),nPos2-nPos1-sSQL_TOKEN.getLength());
				sFirst  += sSecond;
				sFirst  += sStr.copy(nPos2+sSQL_TOKEN.getLength());
			}

			m_sErrorMessage = sFirst;
		}
		else
			m_sErrorMessage = sStr;

		::rtl::OUString aError = s_pScanner->getErrorMessage();
		if(aError.getLength())
		{
			m_sErrorMessage += ::rtl::OUString::createFromAscii(", ");
			m_sErrorMessage += aError;
		}
	}
}
// -------------------------------------------------------------------------
int OSQLParser::SQLlex()
{
	return s_pScanner->SQLlex();
}
/*------------------------------------------------------------------------

	$Log: not supported by cvs2svn $
	Revision 1.1  2000/07/25 10:39:29  oj
	new revision

	Revision 1.0 21.07.2000 12:27:34  oj
------------------------------------------------------------------------*/

