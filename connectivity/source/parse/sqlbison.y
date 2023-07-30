%glr-parser
%token-table
%{
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

#include <vector>
#include <string.h>

#include <connectivity/sqlnode.hxx>
#include <connectivity/sqlparse.hxx>
#include <connectivity/sqlbison_exports.hxx>
#include <connectivity/sqlscan.hxx>
#include <connectivity/internalnode.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/i18n/KParseType.hpp>
#include <com/sun/star/i18n/KParseTokens.hpp>

#include <osl/diagnose.h>
#include "connectivity/dbconversion.hxx"
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4324) // structure was padded due to alignment specifier
#pragma warning(disable: 4065) // switch statement contains 'default' but no 'case' labels
#pragma warning(disable: 4702) // unreachable code
#endif

#ifdef __GNUC__
# pragma GCC diagnostic ignored "-Wwrite-strings"
# pragma GCC diagnostic ignored "-Wunused-function"
#endif

inline connectivity::OSQLInternalNode* newNode(const char* pNewValue,
        const connectivity::SQLNodeType eNodeType,
        const sal_uInt32 nNodeID = 0);

inline connectivity::OSQLInternalNode* newNode(const OString& _newValue,
        const connectivity::SQLNodeType eNodeType,
        const sal_uInt32 nNodeID = 0);

inline connectivity::OSQLInternalNode* newNode(const OUString& _newValue,
        const connectivity::SQLNodeType eNodeType,
        const sal_uInt32 nNodeID = 0);


// yyi is the internal number of the rule that is currently being reduced
// This can be mapped to external rule number via the yyrmap.
#if defined YYBISON && YYBISON >= 30800
#define SQL_NEW_RULE 			newNode("", SQLNodeType::Rule, yyr1[yyrule])
#define SQL_NEW_LISTRULE 		newNode("", SQLNodeType::ListRule, yyr1[yyrule])
#define SQL_NEW_COMMALISTRULE   newNode("", SQLNodeType::CommaListRule, yyr1[yyrule])
#else
#define SQL_NEW_RULE 			newNode("", SQLNodeType::Rule, yyr1[yyn])
#define SQL_NEW_LISTRULE 		newNode("", SQLNodeType::ListRule, yyr1[yyn])
#define SQL_NEW_COMMALISTRULE   newNode("", SQLNodeType::CommaListRule, yyr1[yyn])
#endif


extern connectivity::OSQLParser* xxx_pGLOBAL_SQLPARSER;

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
%type <pParseNode> '(' ')' ',' ':' ';' '?' '[' ']' '{' '}' '.' 'K' 'M' 'G' 'T' 'P'

%token <pParseNode> SQL_TOKEN_STRING SQL_TOKEN_ACCESS_DATE SQL_TOKEN_INT SQL_TOKEN_REAL_NUM
%token <pParseNode> SQL_TOKEN_INTNUM SQL_TOKEN_APPROXNUM SQL_TOKEN_NOT SQL_TOKEN_NAME


%nonassoc <pParseNode> SQL_TOKEN_UMINUS



	/* literal keyword tokens */

%token <pParseNode> SQL_TOKEN_ALL SQL_TOKEN_ALTER SQL_TOKEN_AMMSC SQL_TOKEN_ANY SQL_TOKEN_AS SQL_TOKEN_ASC SQL_TOKEN_AT SQL_TOKEN_AUTHORIZATION SQL_TOKEN_AVG

%token <pParseNode> SQL_TOKEN_BETWEEN SQL_TOKEN_BIT SQL_TOKEN_BOTH SQL_TOKEN_BY

%token <pParseNode> SQL_TOKEN_CAST SQL_TOKEN_CHARACTER SQL_TOKEN_CHECK SQL_TOKEN_COLLATE SQL_TOKEN_COMMIT SQL_TOKEN_CONTINUE SQL_TOKEN_CONVERT SQL_TOKEN_COUNT SQL_TOKEN_CREATE SQL_TOKEN_CROSS
%token <pParseNode> SQL_TOKEN_CURRENT SQL_TOKEN_CURSOR

%token <pParseNode> SQL_TOKEN_DATE SQL_TOKEN_DATEVALUE SQL_TOKEN_DAY SQL_TOKEN_DEC SQL_TOKEN_DECIMAL SQL_TOKEN_DECLARE SQL_TOKEN_DEFAULT SQL_TOKEN_DELETE SQL_TOKEN_DESC
%token <pParseNode> SQL_TOKEN_DISTINCT SQL_TOKEN_DOUBLE SQL_TOKEN_DROP

%token <pParseNode> SQL_TOKEN_ESCAPE SQL_TOKEN_EXCEPT SQL_TOKEN_EXISTS SQL_TOKEN_FALSE SQL_TOKEN_FETCH SQL_TOKEN_FLOAT SQL_TOKEN_FOR SQL_TOKEN_FOREIGN SQL_TOKEN_FOUND SQL_TOKEN_FROM SQL_TOKEN_FULL

%token <pParseNode> SQL_TOKEN_GRANT SQL_TOKEN_GROUP SQL_TOKEN_HAVING SQL_TOKEN_IN SQL_TOKEN_INDICATOR SQL_TOKEN_INNER SQL_TOKEN_INTEGER SQL_TOKEN_INTO SQL_TOKEN_IS SQL_TOKEN_INTERSECT

%left <pParseNode> SQL_TOKEN_JOIN
%token <pParseNode> SQL_TOKEN_KEY SQL_TOKEN_LEADING SQL_TOKEN_LIKE SQL_TOKEN_LOCAL SQL_TOKEN_LOWER
%token <pParseNode> SQL_TOKEN_MAX SQL_TOKEN_MIN SQL_TOKEN_NATURAL SQL_TOKEN_NCHAR SQL_TOKEN_NULL SQL_TOKEN_NUMERIC

%token <pParseNode> SQL_TOKEN_OCTET_LENGTH SQL_TOKEN_OF SQL_TOKEN_ON SQL_TOKEN_OPTION SQL_TOKEN_ORDER SQL_TOKEN_OUTER

%token <pParseNode> SQL_TOKEN_PRECISION SQL_TOKEN_PRIMARY SQL_TOKEN_PRIVILEGES SQL_TOKEN_PROCEDURE SQL_TOKEN_PUBLIC
%token <pParseNode> SQL_TOKEN_REAL SQL_TOKEN_REFERENCES SQL_TOKEN_ROLLBACK

%token <pParseNode> SQL_TOKEN_SCHEMA SQL_TOKEN_SELECT SQL_TOKEN_SET SQL_TOKEN_SIZE SQL_TOKEN_SMALLINT SQL_TOKEN_SOME SQL_TOKEN_SQLCODE SQL_TOKEN_SQLERROR SQL_TOKEN_SUM

%token <pParseNode> SQL_TOKEN_TABLE SQL_TOKEN_TIME SQL_TOKEN_TIMESTAMP SQL_TOKEN_TIMEZONE_HOUR SQL_TOKEN_TIMEZONE_MINUTE SQL_TOKEN_TO SQL_TOKEN_TRAILING SQL_TOKEN_TRANSLATE SQL_TOKEN_TRIM SQL_TOKEN_TRUE SQL_TOKEN_UNION
%token <pParseNode> SQL_TOKEN_UNIQUE SQL_TOKEN_UNKNOWN SQL_TOKEN_UPDATE SQL_TOKEN_UPPER SQL_TOKEN_USAGE SQL_TOKEN_USER SQL_TOKEN_USING SQL_TOKEN_VALUES SQL_TOKEN_VIEW
%token <pParseNode> SQL_TOKEN_WHERE SQL_TOKEN_WITH SQL_TOKEN_WORK SQL_TOKEN_ZONE

/* ODBC KEYWORDS */
%token <pParseNode> SQL_TOKEN_CALL SQL_TOKEN_D SQL_TOKEN_FN SQL_TOKEN_T SQL_TOKEN_TS SQL_TOKEN_OJ
/* string functions */
%token <pParseNode> SQL_TOKEN_ASCII SQL_TOKEN_BIT_LENGTH  SQL_TOKEN_CHAR  SQL_TOKEN_CHAR_LENGTH  SQL_TOKEN_SQL_TOKEN_INTNUM
%token <pParseNode> SQL_TOKEN_CONCAT
%token <pParseNode> SQL_TOKEN_DIFFERENCE  SQL_TOKEN_INSERT SQL_TOKEN_LCASE  SQL_TOKEN_LEFT SQL_TOKEN_LENGTH  SQL_TOKEN_LOCATE
%token <pParseNode> SQL_TOKEN_LOCATE_2 SQL_TOKEN_LTRIM SQL_TOKEN_POSITION SQL_TOKEN_REPEAT SQL_TOKEN_REPLACE
%token <pParseNode> SQL_TOKEN_RIGHT SQL_TOKEN_RTRIM SQL_TOKEN_SOUNDEX SQL_TOKEN_SPACE  SQL_TOKEN_SUBSTRING SQL_TOKEN_UCASE

/* time and date functions */
%token <pParseNode> SQL_TOKEN_CURRENT_DATE SQL_TOKEN_CURRENT_TIME SQL_TOKEN_CURRENT_TIMESTAMP SQL_TOKEN_CURDATE SQL_TOKEN_CURTIME
%token <pParseNode> SQL_TOKEN_DAYNAME  SQL_TOKEN_DAYOFMONTH  SQL_TOKEN_DAYOFWEEK  SQL_TOKEN_DAYOFYEAR SQL_TOKEN_EXTRACT
%token <pParseNode> SQL_TOKEN_HOUR SQL_TOKEN_MILLISECOND SQL_TOKEN_MINUTE  SQL_TOKEN_MONTH  SQL_TOKEN_MONTHNAME SQL_TOKEN_NOW SQL_TOKEN_QUARTER SQL_TOKEN_DATEDIFF
%token <pParseNode> SQL_TOKEN_SECOND SQL_TOKEN_TIMESTAMPADD SQL_TOKEN_TIMESTAMPDIFF SQL_TOKEN_TIMEVALUE SQL_TOKEN_WEEK SQL_TOKEN_WEEKDAY SQL_TOKEN_YEAR SQL_TOKEN_YEARDAY
%token <pParseNode> SQL_TOKEN_DATEADD

/* numeric functions */
%token <pParseNode> SQL_TOKEN_ABS SQL_TOKEN_ACOS SQL_TOKEN_ASIN SQL_TOKEN_ATAN SQL_TOKEN_ATAN2 SQL_TOKEN_CEILING
%token <pParseNode> SQL_TOKEN_COS SQL_TOKEN_COT SQL_TOKEN_DEGREES SQL_TOKEN_EXP SQL_TOKEN_FLOOR SQL_TOKEN_LOGF  SQL_TOKEN_LOG SQL_TOKEN_LN
%token <pParseNode> SQL_TOKEN_LOG10 SQL_TOKEN_MOD SQL_TOKEN_PI SQL_TOKEN_POWER SQL_TOKEN_RADIANS SQL_TOKEN_RAND SQL_TOKEN_ROUNDMAGIC
%token <pParseNode> SQL_TOKEN_ROUND   SQL_TOKEN_SIGN    SQL_TOKEN_SIN     SQL_TOKEN_SQRT    SQL_TOKEN_TAN SQL_TOKEN_TRUNCATE

// computational operation
%token <pParseNode> SQL_TOKEN_EVERY SQL_TOKEN_INTERSECTION SQL_TOKEN_FUSION SQL_TOKEN_COLLECT SQL_TOKEN_VAR_POP SQL_TOKEN_VAR_SAMP
%token <pParseNode> SQL_TOKEN_STDDEV_SAMP SQL_TOKEN_STDDEV_POP

%token <pParseNode> SQL_TOKEN_RANK SQL_TOKEN_DENSE_RANK	SQL_TOKEN_PERCENT_RANK	SQL_TOKEN_CUME_DIST SQL_TOKEN_PERCENTILE_CONT SQL_TOKEN_PERCENTILE_DISC SQL_TOKEN_WITHIN SQL_TOKEN_ARRAY_AGG
%token <pParseNode> SQL_TOKEN_CASE SQL_TOKEN_THEN SQL_TOKEN_END SQL_TOKEN_NULLIF SQL_TOKEN_COALESCE SQL_TOKEN_WHEN SQL_TOKEN_ELSE
%token <pParseNode> SQL_TOKEN_BEFORE SQL_TOKEN_AFTER SQL_TOKEN_INSTEAD SQL_TOKEN_EACH SQL_TOKEN_REFERENCING SQL_TOKEN_BEGIN SQL_TOKEN_ATOMIC SQL_TOKEN_TRIGGER SQL_TOKEN_ROW SQL_TOKEN_STATEMENT
%token <pParseNode> SQL_TOKEN_NEW SQL_TOKEN_OLD
%token <pParseNode> SQL_TOKEN_VALUE SQL_TOKEN_CURRENT_CATALOG SQL_TOKEN_CURRENT_DEFAULT_TRANSFORM_GROUP SQL_TOKEN_CURRENT_PATH SQL_TOKEN_CURRENT_ROLE SQL_TOKEN_CURRENT_SCHEMA SQL_TOKEN_CURRENT_USER
%token <pParseNode> SQL_TOKEN_SESSION_USER SQL_TOKEN_SYSTEM_USER SQL_TOKEN_VARCHAR SQL_TOKEN_VARBINARY SQL_TOKEN_VARYING SQL_TOKEN_OBJECT SQL_TOKEN_NCLOB SQL_TOKEN_NATIONAL
%token <pParseNode> SQL_TOKEN_LARGE SQL_TOKEN_CLOB SQL_TOKEN_BLOB SQL_TOKEN_BIGINT SQL_TOKEN_BINARY SQL_TOKEN_WITHOUT SQL_TOKEN_BOOLEAN SQL_TOKEN_INTERVAL
// window function
%token <pParseNode> SQL_TOKEN_OVER SQL_TOKEN_ROW_NUMBER SQL_TOKEN_NTILE SQL_TOKEN_LEAD SQL_TOKEN_LAG SQL_TOKEN_RESPECT SQL_TOKEN_IGNORE SQL_TOKEN_NULLS
%token <pParseNode> SQL_TOKEN_FIRST_VALUE SQL_TOKEN_LAST_VALUE SQL_TOKEN_NTH_VALUE SQL_TOKEN_FIRST SQL_TOKEN_LAST
%token <pParseNode> SQL_TOKEN_EXCLUDE SQL_TOKEN_OTHERS SQL_TOKEN_TIES SQL_TOKEN_FOLLOWING SQL_TOKEN_UNBOUNDED SQL_TOKEN_PRECEDING SQL_TOKEN_RANGE SQL_TOKEN_ROWS
%token <pParseNode> SQL_TOKEN_PARTITION SQL_TOKEN_WINDOW SQL_TOKEN_NO
// LIMIT and OFFSEt
%token <pParseNode> SQL_TOKEN_LIMIT SQL_TOKEN_OFFSET SQL_TOKEN_NEXT SQL_TOKEN_ONLY

	/* operators */
%left SQL_TOKEN_NAME
%left <pParseNode> SQL_TOKEN_OR
%left <pParseNode> SQL_TOKEN_AND

%left <pParseNode> SQL_LESSEQ SQL_GREATEQ SQL_NOTEQUAL SQL_LESS SQL_GREAT SQL_EQUAL /* '<' '>' = <> < > <= >= != */
%left <pParseNode> '+' '-' SQL_CONCAT
%left <pParseNode> '*' '/'
%left SQL_TOKEN_NATURAL SQL_TOKEN_CROSS SQL_TOKEN_FULL SQL_TOKEN_LEFT SQL_TOKEN_RIGHT
%left ')'
%right '='
%right '.'
%right '('


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
%type <pParseNode> search_condition predicate comparison_predicate comparison_predicate_part_2 between_predicate between_predicate_part_2
%type <pParseNode> like_predicate opt_escape test_for_null null_predicate_part_2 in_predicate in_predicate_part_2 character_like_predicate_part_2 other_like_predicate_part_2
%type <pParseNode> all_or_any_predicate any_all_some existence_test subquery quantified_comparison_predicate_part_2
%type <pParseNode> scalar_exp_commalist parameter_ref literal parenthesized_boolean_value_expression
%type <pParseNode> column_ref data_type column cursor parameter range_variable user /*like_check*/ datetime_unit
/* new rules at OJ */
%type <pParseNode> derived_column as_clause table_name num_primary term num_value_exp
%type <pParseNode> value_exp_primary num_value_fct unsigned_value_spec cast_spec set_fct_spec  scalar_subquery
%type <pParseNode> position_exp extract_exp length_exp general_value_spec
%type <pParseNode> general_set_fct set_fct_type query_exp non_join_query_exp joined_table
%type <pParseNode> non_join_query_term non_join_query_primary simple_table
%type <pParseNode> table_value_const_list row_value_constructor /*row_value_const_list*/ row_value_constructor_elem
%type <pParseNode> qualified_join value_exp query_term join_type outer_join_type join_condition boolean_term
%type <pParseNode> boolean_factor boolean_primary named_columns_join join_spec
%type <pParseNode> cast_operand cast_target factor datetime_value_exp /*interval_value_exp*/ datetime_term datetime_factor
%type <pParseNode> datetime_primary datetime_value_fct time_zone time_zone_specifier /*interval_term*/ interval_qualifier
%type <pParseNode> start_field non_second_datetime_field end_field single_datetime_field extract_field datetime_field time_zone_field
%type <pParseNode> char_length_exp octet_length_exp bit_length_exp select_sublist string_value_exp
%type <pParseNode> char_value_exp concatenation char_factor char_primary string_value_fct char_substring_fct fold
%type <pParseNode> form_conversion char_translation trim_fct trim_operands trim_spec bit_value_fct bit_substring_fct op_column_commalist
%type <pParseNode> /*bit_concatenation*/ bit_value_exp bit_factor bit_primary collate_clause char_value_fct unique_spec value_exp_commalist in_predicate_value unique_test update_source
%type <pParseNode> function_arg_commalist3 string_function_3Argument function_arg_commalist4 string_function_4Argument function_arg_commalist2 string_function_1Argument string_function_2Argument
%type <pParseNode> date_function_0Argument date_function_1Argument function_name12 function_name23 function_name1 function_name2 function_name3 function_name0 numeric_function_0Argument numeric_function_1Argument numeric_function_2Argument
%type <pParseNode> all query_primary sql_not for_length upper_lower comparison column_val  cross_union /*opt_schema_element_list*/
%type <pParseNode> /*op_authorization op_schema*/ nil_fkt schema_element base_table_def base_table_element base_table_element_commalist
%type <pParseNode> column_def odbc_fct_spec	odbc_call_spec odbc_fct_type op_parameter union_statement
%type <pParseNode> op_odbc_call_parameter odbc_parameter_commalist odbc_parameter function_args_commalist function_arg
%type <pParseNode> catalog_name schema_name table_node numeric_function string_function function_name date_function table_primary_as_range_column opt_as
%type <pParseNode> ordered_set_function inverse_distribution_function hypothetical_set_function hypothetical_set_function_value_expression_list rank_function_type within_group_specification inverse_distribution_function_type array_aggregate_function inverse_distribution_function_argument
%type <pParseNode> case_expression else_clause result_expression result case_abbreviation case_specification searched_when_clause simple_when_clause searched_case simple_case
%type <pParseNode> when_operand_list when_operand case_operand
%type <pParseNode> trigger_definition trigger_name trigger_action_time trigger_event transition_table_or_variable_list triggered_action trigger_column_list triggered_when_clause triggered_SQL_statement SQL_procedure_statement old_transition_variable_name new_transition_variable_name
%type <pParseNode> op_referencing op_trigger_columnlist op_triggered_action_for opt_row trigger_for SQL_procedure_statement_list transition_table_or_variable old_transition_table_name new_transition_table_name transition_table_name
%type <pParseNode> searched_when_clause_list simple_when_clause_list predefined_type opt_char_set_spec opt_collate_clause character_string_type national_character_string_type
%type <pParseNode> binary_string_type numeric_type boolean_type datetime_type interval_type opt_paren_precision paren_char_length opt_paren_char_large_length paren_character_large_object_length
%type <pParseNode> large_object_length opt_multiplier character_large_object_type national_character_large_object_type binary_large_object_string_type opt_with_or_without_time_zone
%type <pParseNode> approximate_numeric_type exact_numeric_type opt_paren_precision_scale
/* window function rules */
%type <pParseNode> window_function window_function_type ntile_function number_of_tiles lead_or_lag_function lead_or_lag lead_or_lag_extent offset default_expression null_treatment
%type <pParseNode> first_or_last_value_function first_or_last_value nth_value_function nth_row from_first_or_last window_name_or_specification in_line_window_specification opt_lead_or_lag_function
%type <pParseNode> opt_null_treatment opt_from_first_or_last simple_value_specification dynamic_parameter_specification window_name window_clause window_definition_list window_definition
%type <pParseNode> new_window_name existing_window_name window_partition_clause window_partition_column_reference_list window_partition_column_reference window_frame_clause
%type <pParseNode> window_frame_units window_frame_extent window_frame_start window_frame_preceding window_frame_between window_frame_bound_1 window_frame_bound_2 window_frame_bound window_frame_following window_frame_exclusion
%type <pParseNode> opt_window_frame_clause opt_window_partition_clause opt_existing_window_name window_specification opt_window_frame_exclusion opt_window_clause opt_offset
%type <pParseNode> opt_fetch_first_row_count fetch_first_clause offset_row_count fetch_first_row_count first_or_next row_or_rows opt_result_offset_clause result_offset_clause
/* LIMIT and OFFSET */
%type <pParseNode> opt_limit_offset_clause limit_offset_clause opt_fetch_first_clause
%%

/* Return Parse Tree to OSQLParser
 * (the access over yyval after calling the parser fails,
 *
 */
sql_single_statement:
		sql
		{ xxx_pGLOBAL_SQLPARSER->setParseTree( $1 ); }
	|	sql ';'
		{ xxx_pGLOBAL_SQLPARSER->setParseTree( $1 ); }
	;

	/* schema definition language */
	/* Note: other ``sql:sal_Unicode() rules appear later in the grammar */

sql:
		manipulative_statement
	|	schema_element
           {
                $$ = SQL_NEW_RULE;
		$$->append($1);
	   }
	;

/***

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
			$$->append(newNode(".", SQLNodeType::Punctuation));
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
			{$$ = SQL_NEW_RULE;}
	|       schema_element_list
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

schema_element:
			base_table_def
	|       view_def
	|       privilege_def
	|		trigger_definition
	;

base_table_def:
		SQL_TOKEN_CREATE SQL_TOKEN_TABLE table_node '(' base_table_element_commalist ')'
		{$$ = SQL_NEW_RULE;
		$$->append($1);
		$$->append($2);
		$$->append($3);
		$$->append(newNode("(", SQLNodeType::Punctuation));
		$$->append($5);
		$$->append(newNode(")", SQLNodeType::Punctuation));}
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
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));}
	|       SQL_TOKEN_REFERENCES table_node
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);}
	|       SQL_TOKEN_REFERENCES table_node '(' column_commalist ')'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($4);
			$$->append(newNode(")", SQLNodeType::Punctuation));}
	;

table_constraint_def:
		unique_spec '(' column_commalist ')'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));}
	|       SQL_TOKEN_FOREIGN SQL_TOKEN_KEY '(' column_commalist ')' SQL_TOKEN_REFERENCES table_node
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($4);
			$$->append(newNode(")", SQLNodeType::Punctuation));
			$$->append($6);
			$$->append($7);}
	|       SQL_TOKEN_FOREIGN SQL_TOKEN_KEY '(' column_commalist ')' SQL_TOKEN_REFERENCES table_node '(' column_commalist ')'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($4);
			$$->append(newNode(")", SQLNodeType::Punctuation));
			$$->append($6);
			$$->append($7);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($9);
			$$->append(newNode(")", SQLNodeType::Punctuation));}
	|       SQL_TOKEN_CHECK '(' search_condition ')'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));}
	;
op_column_commalist:
	/* empty */ {$$ = SQL_NEW_RULE;}
	| '(' column_commalist ')'
		{$$ = SQL_NEW_RULE;
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($2);
			$$->append(newNode(")", SQLNodeType::Punctuation));
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
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($2);
			$$->append(newNode(")", SQLNodeType::Punctuation));}
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

sql_not:
/* vide */
	{$$ = SQL_NEW_RULE;}
	|	SQL_TOKEN_NOT
	;

/* manipulative statements */

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
			$$->append(newNode("{", SQLNodeType::Punctuation));
			$$->append($2);
			$$->append(newNode("}", SQLNodeType::Punctuation));
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
		SQL_TOKEN_INSERT SQL_TOKEN_INTO table_node opt_column_commalist query_exp
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
			$$->append($5);}
	;
values_or_query_spec:
		SQL_TOKEN_VALUES '(' table_value_const_list ')'
		{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));
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
/*
row_value_const_list:
			row_value_constructor_elem
			{$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);}
	|       row_value_const_list ',' row_value_constructor_elem
			{$1->append($3);
			$$ = $1;}
	;
*/
row_value_constructor:
			row_value_constructor_elem
/*	  |		'(' row_value_const_list ')'
			{
				$$ = SQL_NEW_RULE;
				$$->append(newNode("(", SQLNodeType::Punctuation));
				$$->append($2);
				$$->append(newNode(")", SQLNodeType::Punctuation));
			}
			*/
	;
row_value_constructor_elem:
			value_exp /*[^')']*/
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
		column SQL_EQUAL update_source
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);}
	;
update_source:
		value_exp
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
    ;
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
			$$->append(newNode("*", SQLNodeType::Punctuation));
		}
	|	scalar_exp_commalist
	;
opt_result_offset_clause:
		/* empty */ {$$ = SQL_NEW_RULE;}
	|	result_offset_clause
	;
result_offset_clause:
	SQL_TOKEN_OFFSET offset_row_count row_or_rows
	{
		$$ = SQL_NEW_RULE;
		$$->append($1);
		$$->append($2);
		$$->append($3);
	}
	;
opt_fetch_first_row_count:
		/* empty */ {$$ = SQL_NEW_RULE;}
	|	fetch_first_row_count
	;
first_or_next:
		SQL_TOKEN_FIRST
	|	SQL_TOKEN_NEXT
	;
row_or_rows:
		SQL_TOKEN_ROW
	|	SQL_TOKEN_ROWS
	;
opt_fetch_first_clause:
		/* empty */ {$$ = SQL_NEW_RULE;}
	|	fetch_first_clause
	;
fetch_first_clause:
	SQL_TOKEN_FETCH first_or_next opt_fetch_first_row_count row_or_rows SQL_TOKEN_ONLY
	{
		$$ = SQL_NEW_RULE;
		$$->append($1);
		$$->append($2);
		$$->append($3);
		$$->append($4);
		$$->append($5);
	}
	;
offset_row_count:
	literal
	;
fetch_first_row_count:
	literal
	;

opt_limit_offset_clause:
		/* empty */ {$$ = SQL_NEW_RULE;}
	|	limit_offset_clause
	;
opt_offset:
		/* empty */ {$$ = SQL_NEW_RULE;}
	|	SQL_TOKEN_OFFSET SQL_TOKEN_INTNUM
	{
		$$ = SQL_NEW_RULE;
		$$->append($1);
		$$->append($2);
	}
	;
limit_offset_clause:
	SQL_TOKEN_LIMIT SQL_TOKEN_INTNUM opt_offset
	{
		$$ = SQL_NEW_RULE;
		$$->append($1);
		$$->append($2);
		$$->append($3);
	}
	;
table_exp:
		from_clause opt_where_clause opt_group_by_clause opt_having_clause opt_window_clause opt_order_by_clause opt_limit_offset_clause opt_result_offset_clause opt_fetch_first_clause
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
			$$->append($5);
			$$->append($6);
			$$->append($7);
			$$->append($8);
			$$->append($9);
		}
	;

from_clause:
		SQL_TOKEN_FROM table_ref_commalist
			{ $$ = SQL_NEW_RULE;
			  $$->append($1);
			  $$->append($2); }
	;

table_ref_commalist:

		table_ref
			{ $$ = SQL_NEW_COMMALISTRULE;
			  $$->append($1); }
	|       table_ref_commalist ',' table_ref
			{ $1->append($3);
			  $$ = $1; }
	;

opt_as:
		/* empty */ {$$ = SQL_NEW_RULE;}
	|	SQL_TOKEN_AS
	;
opt_row:
		/* empty */ {$$ = SQL_NEW_RULE;}
	|	SQL_TOKEN_ROW
	;
table_primary_as_range_column:
        {$$ = SQL_NEW_RULE;}
	|   opt_as SQL_TOKEN_NAME op_column_commalist
		{
		    $$ = SQL_NEW_RULE;
		    $$->append($1);
		    $$->append($2);
		    $$->append($3);
		}
    ;
table_ref:
        table_node table_primary_as_range_column
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	|   subquery range_variable op_column_commalist
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	|	joined_table
	|	'{' SQL_TOKEN_OJ joined_table '}'
		{
			$$ = SQL_NEW_RULE;
			$$->append(newNode("{", SQLNodeType::Punctuation));
			$$->append($2);
			$$->append($3);
			$$->append(newNode("}", SQLNodeType::Punctuation));
		}
	|	'(' joined_table ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($2);
			$$->append(newNode(")", SQLNodeType::Punctuation));
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
	|   SQL_TOKEN_GROUP SQL_TOKEN_BY column_ref_commalist
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);}
	;

column_ref_commalist:
		column_ref
			{$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);}
	|	set_fct_spec
		{$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);}
	|   column_ref_commalist ',' column_ref
			{$1->append($3);
			$$ = $1;}
	|   column_ref_commalist ',' set_fct_spec
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
boolean_primary:
		predicate
	|   '(' search_condition ')'
		{ // boolean_primary: rule 2
			$$ = SQL_NEW_RULE;
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($2);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	|	row_value_constructor_elem  /*[^')' ',']*/
		{
                    if(xxx_pGLOBAL_SQLPARSER->inPredicateCheck())// boolean_primary: rule 3
                    {
                        $$ = SQL_NEW_RULE;
                        sal_Int16 nErg = 0;
                        if ( SQL_ISTOKEN( $1, NULL))
                        {
                            OSQLParseNode* pColumnRef = newNode("", SQLNodeType::Rule,OSQLParser::RuleID(OSQLParseNode::column_ref));
                            pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQLNodeType::Name));
                            OSQLParseNode* pTFN = new OSQLInternalNode("", SQLNodeType::Rule,OSQLParser::RuleID(OSQLParseNode::test_for_null));
                            pTFN->append(pColumnRef);

                            OSQLParseNode* pNPP2 = new OSQLInternalNode("", SQLNodeType::Rule, OSQLParser::RuleID(OSQLParseNode::null_predicate_part_2));
                            pNPP2->append(new OSQLInternalNode("", SQLNodeType::Keyword, SQL_TOKEN_IS));
                            pNPP2->append(new OSQLInternalNode("", SQLNodeType::Rule, OSQLParser::RuleID(OSQLParseNode::sql_not)));
                            pNPP2->append(new OSQLInternalNode("", SQLNodeType::Keyword, SQL_TOKEN_NULL));
                            pTFN->append(pNPP2);

                            $$->append(pTFN);

                            nErg = 1;
                        }
                        else
                        {
                            nErg = xxx_pGLOBAL_SQLPARSER->buildComparisonRule($$,$1);
                        }
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
                    else
                        YYERROR;
                }
	;
parenthesized_boolean_value_expression:
   '(' search_condition ')'
	{ // boolean_primary: rule 2
		$$ = SQL_NEW_RULE;
		$$->append(newNode("(", SQLNodeType::Punctuation));
		$$->append($2);
		$$->append(newNode(")", SQLNodeType::Punctuation));
	}
	;
boolean_factor:
	    boolean_primary			%dprec 2
	|   SQL_TOKEN_NOT boolean_primary	%dprec 1
		{ // boolean_factor: rule 1
		    $$ = SQL_NEW_RULE;
		    $$->append($1);
		    $$->append($2);
		}
	;
boolean_term:
		boolean_factor
	|	boolean_term SQL_TOKEN_AND boolean_factor
		{
			$$ = SQL_NEW_RULE; // boolean_term: rule 1
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	;
search_condition:
		boolean_term
	|	search_condition SQL_TOKEN_OR boolean_term
		{
			$$ = SQL_NEW_RULE; // search_condition
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	;
predicate:
		comparison_predicate     %dprec 1
	|       between_predicate
	|       all_or_any_predicate
	|       existence_test
	|		unique_test
	|		test_for_null    %dprec 2
	|       in_predicate
	|       like_predicate
	;
comparison_predicate_part_2:
		comparison row_value_constructor
		{
			$$ = SQL_NEW_RULE; // comparison_predicate: rule 1
			$$->append($1);
			$$->append($2);
		}
comparison_predicate:
		row_value_constructor comparison row_value_constructor
		{
			$$ = SQL_NEW_RULE; // comparison_predicate: rule 1
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	|	comparison row_value_constructor
		{
			if(xxx_pGLOBAL_SQLPARSER->inPredicateCheck()) // comparison_predicate: rule 2
			{
				$$ = SQL_NEW_RULE;
				sal_Int16 nErg = xxx_pGLOBAL_SQLPARSER->buildPredicateRule($$,$2,$1);
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
			else
			{
				YYERROR;
			}
		}
	;
comparison:
	    SQL_LESS
	  | SQL_NOTEQUAL
	  | SQL_EQUAL
	  | SQL_GREAT
	  | SQL_LESSEQ
	  | SQL_GREATEQ
	  | SQL_TOKEN_IS sql_not SQL_TOKEN_DISTINCT SQL_TOKEN_FROM
	  	{
		  $$ = SQL_NEW_RULE;
		  $$->append($1);
		  $$->append($2);
		  $$->append($3);
		  $$->append($4);
		}
	  | SQL_TOKEN_IS sql_not
	  	{
		  $$ = SQL_NEW_RULE;
		  $$->append($1);
		  $$->append($2);
		}
	;
between_predicate_part_2:
	sql_not SQL_TOKEN_BETWEEN row_value_constructor SQL_TOKEN_AND row_value_constructor
		{
			if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck()) // between_predicate: rule 2
			{
				$$ = SQL_NEW_RULE;

				sal_Int16 nErg = xxx_pGLOBAL_SQLPARSER->buildPredicateRule($$,$3,$2,$5);
				if(nErg == 1)
				{
					OSQLParseNode* pTemp = $$;
					$$ = pTemp->removeAt((sal_uInt32)0);
					OSQLParseNode* pColumnRef = $$->removeAt((sal_uInt32)0);
					$$->insert(0,$1);
					OSQLParseNode* pBetween_predicate = new OSQLInternalNode("", SQLNodeType::Rule,OSQLParser::RuleID(OSQLParseNode::between_predicate));
					pBetween_predicate->append(pColumnRef);
					pBetween_predicate->append($$);
					$$ = pBetween_predicate;

					delete pTemp;
					delete $4;
				}
				else
				{
					delete $$;
					YYABORT;
				}
			}
			else
			{
				$$ = SQL_NEW_RULE; // between_predicate: rule 1
				$$->append($1);
				$$->append($2);
				$$->append($3);
				$$->append($4);
				$$->append($5);
			}
		}
between_predicate:
		row_value_constructor between_predicate_part_2
		{
			$$ = SQL_NEW_RULE; // between_predicate: rule 1
			$$->append($1);
			$$->append($2);
		}
	|	between_predicate_part_2
	;
character_like_predicate_part_2:
	sql_not SQL_TOKEN_LIKE string_value_exp opt_escape
		{
			$$ = SQL_NEW_RULE; // like_predicate: rule 1
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	;
other_like_predicate_part_2:
	sql_not SQL_TOKEN_LIKE value_exp_primary opt_escape
		{
			$$ = SQL_NEW_RULE; // like_predicate: rule 1
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	;
like_predicate:
		row_value_constructor character_like_predicate_part_2
		{
			$$ = SQL_NEW_RULE; // like_predicate: rule 1
			$$->append($1);
			$$->append($2);
		}
	|	row_value_constructor other_like_predicate_part_2
		{
			$$ = SQL_NEW_RULE;  // like_predicate: rule 3
			$$->append($1);
			$$->append($2);
		}
	|	character_like_predicate_part_2
		{
			if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())  // like_predicate: rule 5
			{
				OSQLParseNode* pColumnRef = newNode("", SQLNodeType::Rule,OSQLParser::RuleID(OSQLParseNode::column_ref));
				pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQLNodeType::Name));

				$$ = SQL_NEW_RULE;
				$$->append(pColumnRef);
				$$->append($1);
				OSQLParseNode* p2nd = $1->removeAt(2);
				OSQLParseNode* p3rd = $1->removeAt(2);
				if ( !xxx_pGLOBAL_SQLPARSER->buildLikeRule($1,p2nd,p3rd) )
				{
					delete $$;
					YYABORT;
				}
				$1->append(p3rd);
			}
			else
				YYERROR;
		}
	|	other_like_predicate_part_2
		{
			if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck()) // like_predicate: rule 6
			{
				OSQLParseNode* pColumnRef = newNode("", SQLNodeType::Rule,OSQLParser::RuleID(OSQLParseNode::column_ref));
				pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQLNodeType::Name));

				$$ = SQL_NEW_RULE;
				$$->append(pColumnRef);
				$$->append($1);
				OSQLParseNode* p2nd = $1->removeAt(2);
				OSQLParseNode* p3rd = $1->removeAt(2);
				if ( !xxx_pGLOBAL_SQLPARSER->buildLikeRule($1,p2nd,p3rd) )
				{
					delete $$;
					YYABORT;
				}
				$1->append(p3rd);
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
			$$->append(newNode("{", SQLNodeType::Punctuation));
			$$->append($2);
			$$->append($3);
			$$->append(newNode("}", SQLNodeType::Punctuation));
		}
	;

null_predicate_part_2:
	  SQL_TOKEN_IS sql_not SQL_TOKEN_NULL
	  {
		$$ = SQL_NEW_RULE; // test_for_null: rule 1
		$$->append($1);
		$$->append($2);
		$$->append($3);
	  }
	| SQL_TOKEN_IS sql_not SQL_TOKEN_UNKNOWN
	  {
		$$ = SQL_NEW_RULE; // test_for_null: rule 1
		$$->append($1);
		$$->append($2);
		$$->append($3);
	  }
	;
test_for_null:
		row_value_constructor null_predicate_part_2
		{
			$$ = SQL_NEW_RULE; // test_for_null: rule 1
			$$->append($1);
			$$->append($2);
		}
	|	null_predicate_part_2
		{
			if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())// test_for_null: rule 2
			{
				OSQLParseNode* pColumnRef = newNode("", SQLNodeType::Rule,OSQLParser::RuleID(OSQLParseNode::column_ref));
				pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQLNodeType::Name));

				$$ = SQL_NEW_RULE;
				$$->append(pColumnRef);
				$$->append($1);
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
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($2);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	;
in_predicate_part_2:
	sql_not SQL_TOKEN_IN in_predicate_value
	{
		$$ = SQL_NEW_RULE;// in_predicate: rule 1
		$$->append($1);
		$$->append($2);
		$$->append($3);
	}
	;
in_predicate:
		row_value_constructor in_predicate_part_2
		{
			$$ = SQL_NEW_RULE;// in_predicate: rule 1
			$$->append($1);
			$$->append($2);
		}
	|	in_predicate_part_2
		{
			if ( xxx_pGLOBAL_SQLPARSER->inPredicateCheck() )// in_predicate: rule 2
			{
				OSQLParseNode* pColumnRef = newNode("", SQLNodeType::Rule,OSQLParser::RuleID(OSQLParseNode::column_ref));
				pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQLNodeType::Name));

				$$ = SQL_NEW_RULE;
				$$->append(pColumnRef);
				$$->append($1);
			}
			else
				YYERROR;
		}
	;
quantified_comparison_predicate_part_2:
	comparison any_all_some subquery
	{
		$$ = SQL_NEW_RULE;
		$$->append($1);
		$$->append($2);
		$$->append($3);
	}
	;
all_or_any_predicate:
		row_value_constructor quantified_comparison_predicate_part_2
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	|	quantified_comparison_predicate_part_2
		{
			if(xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
			{
				OSQLParseNode* pColumnRef = newNode("", SQLNodeType::Rule,OSQLParser::RuleID(OSQLParseNode::column_ref));
				pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQLNodeType::Name));

				$$ = SQL_NEW_RULE;
				$$->append(pColumnRef);
				$$->append($1);
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
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($2);
			$$->append(newNode(")", SQLNodeType::Punctuation));}
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
			$$->append(newNode(".", SQLNodeType::Punctuation));
			$$->append(newNode("*", SQLNodeType::Punctuation));
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
			$$ = newNode("*", SQLNodeType::Punctuation);
		}
	|	'?'
		{
			$$ = newNode("?", SQLNodeType::Punctuation);
		}
	|   op_like	'*'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("*", SQLNodeType::Punctuation));
			xxx_pGLOBAL_SQLPARSER->reduceLiteral($$, sal_False);
		}
	|	op_like '?'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("?", SQLNodeType::Punctuation));
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
				OSQLParser::reduceLiteral($$, true);
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
				OSQLParser::reduceLiteral($$, true);
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
				OSQLParser::reduceLiteral($$, true);
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
				OSQLParser::reduceLiteral($$, true);
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
		SQL_TOKEN_POSITION '(' value_exp SQL_TOKEN_IN value_exp ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append($4);
			$$->append($5);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	|	SQL_TOKEN_POSITION '(' value_exp_commalist ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	;
num_value_fct:
		position_exp
	|	extract_exp
	|	length_exp
	;
char_length_exp:
		SQL_TOKEN_CHAR_LENGTH '(' value_exp ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	|	SQL_TOKEN_SQL_TOKEN_INTNUM '(' value_exp ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}

	;
octet_length_exp:
		SQL_TOKEN_OCTET_LENGTH '(' value_exp ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	;
bit_length_exp:
		SQL_TOKEN_BIT_LENGTH '(' value_exp ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));
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
	   time_zone_field
	  | datetime_field
	  |	value_exp
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
extract_exp:
		SQL_TOKEN_EXTRACT '(' extract_field SQL_TOKEN_FROM value_exp ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append($4);
			$$->append($5);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	;
unsigned_value_spec:
		general_value_spec
	|	literal
	;
general_value_spec:
		parameter
	| SQL_TOKEN_USER
	| SQL_TOKEN_NULL
	| SQL_TOKEN_FALSE
	| SQL_TOKEN_TRUE
	| SQL_TOKEN_VALUE
	| SQL_TOKEN_CURRENT_CATALOG
	| SQL_TOKEN_CURRENT_DEFAULT_TRANSFORM_GROUP
	| SQL_TOKEN_CURRENT_PATH
	| SQL_TOKEN_CURRENT_ROLE
	| SQL_TOKEN_CURRENT_SCHEMA
	| SQL_TOKEN_CURRENT_USER
	| SQL_TOKEN_SESSION_USER
	| SQL_TOKEN_SYSTEM_USER
	;
set_fct_spec:
		general_set_fct
	|	'{' odbc_fct_spec '}'
		{
			$$ = SQL_NEW_RULE;
			$$->append(newNode("{", SQLNodeType::Punctuation));
			$$->append($2);
			$$->append(newNode("}", SQLNodeType::Punctuation));
		}
	|	function_name '(' ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	|	function_name0 '(' ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	|	function_name1 '(' function_arg ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	|	function_name2 '(' function_arg_commalist2 ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	|	function_name3 '(' function_arg_commalist3 ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	|	string_function_4Argument '(' function_arg_commalist4 ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	|	function_name '(' function_args_commalist ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	|	function_name12 '(' function_args_commalist ')'
		{
			if ( $3->count() == 1 || $3->count() == 2 )
			{
				$$ = SQL_NEW_RULE;
				$$->append($1);
				$$->append(newNode("(", SQLNodeType::Punctuation));
				$$->append($3);
				$$->append(newNode(")", SQLNodeType::Punctuation));
			}
			else
				YYERROR;
		}
	|	function_name23 '(' function_args_commalist ')'
		{
			if ( $3->count() == 2 || $3->count() == 3)
			{
				$$ = SQL_NEW_RULE;
				$$->append($1);
				$$->append(newNode("(", SQLNodeType::Punctuation));
				$$->append($3);
				$$->append(newNode(")", SQLNodeType::Punctuation));
			}
			else
				YYERROR;
		}
	;
function_name0:
		date_function_0Argument
	|	numeric_function_0Argument
	;
function_name1:
		string_function_1Argument
	|	date_function_1Argument
	|	numeric_function_1Argument
	;
function_name2:
		string_function_2Argument
	|	numeric_function_2Argument
	;
function_name12:
		SQL_TOKEN_ROUND
	|	SQL_TOKEN_WEEK
	|	SQL_TOKEN_LOGF
	|	SQL_TOKEN_LOG
	;
function_name23:
		SQL_TOKEN_LOCATE
	|	SQL_TOKEN_DATEDIFF
	;
function_name3:
		string_function_3Argument
	;
function_name:
		string_function
	|	date_function
	|	numeric_function
	|	SQL_TOKEN_NAME
	;
string_function_1Argument:
		SQL_TOKEN_LENGTH
	|	SQL_TOKEN_ASCII
	|	SQL_TOKEN_LCASE
	|	SQL_TOKEN_LTRIM
	|	SQL_TOKEN_RTRIM
	|	SQL_TOKEN_SPACE
	|	SQL_TOKEN_UCASE
	;

string_function_2Argument:
		SQL_TOKEN_REPEAT
	|	SQL_TOKEN_LEFT
	|	SQL_TOKEN_RIGHT
	;
string_function_3Argument:
		SQL_TOKEN_REPLACE
	;
string_function_4Argument:
		SQL_TOKEN_INSERT
	;

string_function:
		SQL_TOKEN_CHAR
	|	SQL_TOKEN_CONCAT
	|	SQL_TOKEN_DIFFERENCE
	|	SQL_TOKEN_LOCATE_2
	|	SQL_TOKEN_SOUNDEX
	;
date_function_0Argument:
		SQL_TOKEN_CURDATE
	|	SQL_TOKEN_CURTIME
	|	SQL_TOKEN_NOW
	;
date_function_1Argument:
		SQL_TOKEN_DAYOFWEEK
	|	SQL_TOKEN_DAYOFMONTH
	|	SQL_TOKEN_DAYOFYEAR
	|	SQL_TOKEN_MONTH
	|	SQL_TOKEN_DAYNAME
	|	SQL_TOKEN_MONTHNAME
	|	SQL_TOKEN_QUARTER
	|	SQL_TOKEN_HOUR
	|	SQL_TOKEN_MINUTE
	|	SQL_TOKEN_SECOND
	|	SQL_TOKEN_YEAR
	|	SQL_TOKEN_DAY
	|	SQL_TOKEN_TIMEVALUE
	|	SQL_TOKEN_DATEVALUE
	;

date_function:
		SQL_TOKEN_TIMESTAMPADD
	|	SQL_TOKEN_TIMESTAMPDIFF
	|	SQL_TOKEN_DATEADD
	;
numeric_function_0Argument:
		SQL_TOKEN_PI
	;
numeric_function_1Argument:
		SQL_TOKEN_ABS
	|	SQL_TOKEN_ACOS
	|	SQL_TOKEN_ASIN
	|	SQL_TOKEN_ATAN
	|	SQL_TOKEN_CEILING
	|	SQL_TOKEN_COS
	|	SQL_TOKEN_COT
	|	SQL_TOKEN_DEGREES
	|	SQL_TOKEN_FLOOR
	|	SQL_TOKEN_SIGN
	|	SQL_TOKEN_SIN
	|	SQL_TOKEN_SQRT
	|	SQL_TOKEN_TAN
	|	SQL_TOKEN_EXP
	|	SQL_TOKEN_LOG10
	|	SQL_TOKEN_LN
	|	SQL_TOKEN_RADIANS
	|	SQL_TOKEN_ROUNDMAGIC
	;
numeric_function_2Argument:
		SQL_TOKEN_ATAN2
	|	SQL_TOKEN_MOD
	|	SQL_TOKEN_POWER
	;
numeric_function:
		SQL_TOKEN_RAND
	|	SQL_TOKEN_TRUNCATE
	;

window_function:
	window_function_type SQL_TOKEN_OVER window_name_or_specification
	{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
	}
	;
window_function_type :
		rank_function_type '(' ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	|	SQL_TOKEN_ROW_NUMBER '(' ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	| general_set_fct
	| ntile_function
	| lead_or_lag_function
	| first_or_last_value_function
	| nth_value_function
;
ntile_function :
	SQL_TOKEN_NTILE '(' number_of_tiles ')'
	{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));
	}
	;
dynamic_parameter_specification:
	parameter
	;
simple_value_specification:
	literal
	;
number_of_tiles :
		simple_value_specification
	|	dynamic_parameter_specification
	;
opt_lead_or_lag_function:
	/* empty */      {$$ = SQL_NEW_RULE;}
	| ',' offset
		{
			$$ = SQL_NEW_RULE;
			$$->append(newNode(",", SQLNodeType::Punctuation));
			$$->append($2);
		}
	| ',' offset ',' default_expression
		{
			$$ = SQL_NEW_RULE;
			$$->append(newNode(",", SQLNodeType::Punctuation));
			$$->append($2);
			$$->append(newNode(",", SQLNodeType::Punctuation));
			$$->append($4);
		}
	;
opt_null_treatment:
		/* empty */      {$$ = SQL_NEW_RULE;}
	|	null_treatment
	;

lead_or_lag_function:
	lead_or_lag '(' lead_or_lag_extent opt_lead_or_lag_function ')'	opt_null_treatment
	{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append($4);
			$$->append(newNode(")", SQLNodeType::Punctuation));
			$$->append($6);
	}
	;
lead_or_lag:
		SQL_TOKEN_LEAD
	|	SQL_TOKEN_LAG
	;
lead_or_lag_extent:
	value_exp
	;
offset:
	SQL_TOKEN_INTNUM
	;
default_expression:
	value_exp
	;
null_treatment:
		SQL_TOKEN_RESPECT SQL_TOKEN_NULLS
	|	SQL_TOKEN_IGNORE SQL_TOKEN_NULLS
	;
first_or_last_value_function:
	first_or_last_value '(' value_exp ')' opt_null_treatment
	{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));
			$$->append($5);
	}
	;
first_or_last_value :
		SQL_TOKEN_FIRST_VALUE
	|	SQL_TOKEN_LAST_VALUE
	;
opt_from_first_or_last:
		/* empty */      {$$ = SQL_NEW_RULE;}
	|	from_first_or_last
	;
nth_value_function:
	SQL_TOKEN_NTH_VALUE '(' value_exp ',' nth_row ')' opt_from_first_or_last opt_null_treatment
	{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(",", SQLNodeType::Punctuation));
			$$->append($5);
			$$->append(newNode(")", SQLNodeType::Punctuation));
			$$->append($7);
			$$->append($8);
	}
	;
nth_row:
		simple_value_specification
	|	dynamic_parameter_specification
	;
from_first_or_last:
		SQL_TOKEN_FROM SQL_TOKEN_FIRST
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	|	SQL_TOKEN_FROM SQL_TOKEN_LAST
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;
window_name:
	SQL_TOKEN_NAME
	;
window_name_or_specification:
		window_name
	|	in_line_window_specification
	;
in_line_window_specification:
	window_specification
	;
opt_window_clause:
		/* empty */      {$$ = SQL_NEW_RULE;}
	|	window_clause
	;
window_clause:
	SQL_TOKEN_WINDOW window_definition_list
	{
		$$ = SQL_NEW_RULE;
		$$->append($1);
		$$->append($2);
	}
	;
window_definition_list:
		window_definition_list ',' window_definition
			{$1->append($3);
			$$ = $1;}
	|	window_definition
			{$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);}
	;
window_definition:
	new_window_name SQL_TOKEN_AS window_specification
	{
		$$ = SQL_NEW_RULE;
		$$->append($1);
		$$->append($2);
		$$->append($3);
	}
	;
new_window_name:
	window_name
	;
window_specification:
	'('
		opt_existing_window_name
		opt_window_partition_clause
		opt_order_by_clause
		opt_window_frame_clause
	')'
	{
		$$ = SQL_NEW_RULE;
		$$->append(newNode("(", SQLNodeType::Punctuation));
		$$->append($2);
		$$->append($3);
		$$->append($4);
		$$->append($5);
		$$->append(newNode(")", SQLNodeType::Punctuation));
	}
	;
opt_existing_window_name:
		/* empty */      {$$ = SQL_NEW_RULE;}
	|	existing_window_name
	;
opt_window_partition_clause:
	/* empty */      {$$ = SQL_NEW_RULE;}
	|	window_partition_clause
	;
opt_window_frame_clause:
	/* empty */      {$$ = SQL_NEW_RULE;}
	|	window_frame_clause
	;
existing_window_name:
	window_name
	;
window_partition_clause:
	SQL_TOKEN_PARTITION SQL_TOKEN_BY window_partition_column_reference_list
	{
		$$ = SQL_NEW_RULE;
		$$->append($1);
		$$->append($2);
		$$->append($3);
	}
	;
window_partition_column_reference_list:
	window_partition_column_reference_list ',' window_partition_column_reference
			{$1->append($3);
			$$ = $1;}
	|	window_partition_column_reference
			{$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);}
	;
window_partition_column_reference:
	column_ref opt_collate_clause
	{
		$$ = SQL_NEW_RULE;
		$$->append($1);
		$$->append($2);
	}
	;
opt_window_frame_exclusion:
	/* empty */      {$$ = SQL_NEW_RULE;}
	|	window_frame_exclusion
	;
window_frame_clause:
	window_frame_units window_frame_extent opt_window_frame_exclusion
	{
		$$ = SQL_NEW_RULE;
		$$->append($1);
		$$->append($2);
		$$->append($3);
	}
	;
window_frame_units:
		SQL_TOKEN_ROWS
	|	SQL_TOKEN_RANGE
	;
window_frame_extent:
		window_frame_start
	|	window_frame_between
	;
window_frame_start:
		SQL_TOKEN_UNBOUNDED SQL_TOKEN_PRECEDING
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	|	window_frame_preceding
	|	SQL_TOKEN_CURRENT SQL_TOKEN_ROW
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;
window_frame_preceding:
	unsigned_value_spec SQL_TOKEN_PRECEDING
	{
		$$ = SQL_NEW_RULE;
		$$->append($1);
		$$->append($2);
	}
	;
window_frame_between:
	SQL_TOKEN_BETWEEN window_frame_bound_1 SQL_TOKEN_AND window_frame_bound_2
	{
		$$ = SQL_NEW_RULE;
		$$->append($1);
		$$->append($2);
		$$->append($3);
		$$->append($4);
	}
	;
window_frame_bound_1:
	window_frame_bound
	;
window_frame_bound_2:
	window_frame_bound
	;
window_frame_bound:
	window_frame_start
	| SQL_TOKEN_UNBOUNDED SQL_TOKEN_FOLLOWING
	{
		$$ = SQL_NEW_RULE;
		$$->append($1);
		$$->append($2);
	}
	| window_frame_following
	;
window_frame_following:
	unsigned_value_spec SQL_TOKEN_FOLLOWING
	{
		$$ = SQL_NEW_RULE;
		$$->append($1);
		$$->append($2);
	}
	;
window_frame_exclusion:
		SQL_TOKEN_EXCLUDE SQL_TOKEN_CURRENT SQL_TOKEN_ROW
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	|	SQL_TOKEN_EXCLUDE SQL_TOKEN_GROUP
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	|	SQL_TOKEN_EXCLUDE SQL_TOKEN_TIES
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	|	SQL_TOKEN_EXCLUDE SQL_TOKEN_NO SQL_TOKEN_OTHERS
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	;
op_parameter:
		{$$ = SQL_NEW_RULE;}
	|	'?' SQL_EQUAL
		{
			$$ = SQL_NEW_RULE;
			$$->append(newNode("?", SQLNodeType::Punctuation));
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
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($2);
			$$->append(newNode(")", SQLNodeType::Punctuation));
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
	|	SQL_TOKEN_FN set_fct_spec
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;

odbc_fct_type:
		SQL_TOKEN_D
	| 	SQL_TOKEN_T
	| 	SQL_TOKEN_TS
	;

general_set_fct:
		set_fct_type '(' opt_all_distinct function_arg ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append($4);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	|	SQL_TOKEN_COUNT '(' '*' ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append(newNode("*", SQLNodeType::Punctuation));
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	|	SQL_TOKEN_COUNT '(' opt_all_distinct function_arg ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append($4);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	|	ordered_set_function
	|	array_aggregate_function
	;
set_fct_type:
		SQL_TOKEN_AVG
	|   SQL_TOKEN_MAX
	|   SQL_TOKEN_MIN
	|   SQL_TOKEN_SUM
	|   SQL_TOKEN_EVERY
	|   SQL_TOKEN_ANY
	|   SQL_TOKEN_SOME
	|   SQL_TOKEN_STDDEV_POP
	|   SQL_TOKEN_STDDEV_SAMP
	|   SQL_TOKEN_VAR_SAMP
	|   SQL_TOKEN_VAR_POP
	|   SQL_TOKEN_COLLECT
	|   SQL_TOKEN_FUSION
	|   SQL_TOKEN_INTERSECTION
	;

ordered_set_function:
		hypothetical_set_function
	|	inverse_distribution_function
	;
hypothetical_set_function:
		rank_function_type '('	hypothetical_set_function_value_expression_list ')'	within_group_specification
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));
			$$->append($5);
		}
	|	rank_function_type '('	hypothetical_set_function_value_expression_list SQL_TOKEN_BY value_exp_commalist ')'	within_group_specification
	{
		$$ = SQL_NEW_RULE;
		$$->append($1);
		$$->append(newNode("(", SQLNodeType::Punctuation));
		$$->append($3);
		$$->append($4);
		$$->append($5);
		$$->append(newNode(")", SQLNodeType::Punctuation));
		$$->append($7);
	}
	;

within_group_specification:
	{
		$$ = SQL_NEW_RULE;
	}
	|	SQL_TOKEN_WITHIN SQL_TOKEN_GROUP '(' opt_order_by_clause ')'
	{
		$$ = SQL_NEW_RULE;
		$$->append($1);
		$$->append($2);
		$$->append(newNode("(", SQLNodeType::Punctuation));
		$$->append($4);
		$$->append(newNode(")", SQLNodeType::Punctuation));
	}
	;
hypothetical_set_function_value_expression_list:
	value_exp_commalist
	;

inverse_distribution_function:
	inverse_distribution_function_type '('inverse_distribution_function_argument ')' within_group_specification
	{
		$$ = SQL_NEW_RULE;
		$$->append($1);
		$$->append(newNode("(", SQLNodeType::Punctuation));
		$$->append($3);
		$$->append(newNode(")", SQLNodeType::Punctuation));
	}
	;
inverse_distribution_function_argument:
		num_value_exp
	;
inverse_distribution_function_type:
		SQL_TOKEN_PERCENTILE_CONT
	|	SQL_TOKEN_PERCENTILE_DISC
	;

array_aggregate_function:
	SQL_TOKEN_ARRAY_AGG '(' value_exp opt_order_by_clause ')'
	{
		$$ = SQL_NEW_RULE;
		$$->append($1);
		$$->append(newNode("(", SQLNodeType::Punctuation));
		$$->append($3);
		$$->append($4);
		$$->append(newNode(")", SQLNodeType::Punctuation));
	}
	;

rank_function_type:
		SQL_TOKEN_RANK
	|	SQL_TOKEN_DENSE_RANK
	|	SQL_TOKEN_PERCENT_RANK
	|	SQL_TOKEN_CUME_DIST
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
		/* empty */ 	{$$ = SQL_NEW_RULE;}
	  | SQL_TOKEN_INNER
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
		table_ref SQL_TOKEN_CROSS SQL_TOKEN_JOIN table_ref
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	;

qualified_join:
		/* when SQL_TOKEN_NATURAL, then no join_spec */
		table_ref SQL_TOKEN_NATURAL join_type SQL_TOKEN_JOIN table_ref
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
	;
named_columns_join:
		SQL_TOKEN_USING '(' column_commalist ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));
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
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($2);
			$$->append(newNode(")", SQLNodeType::Punctuation));
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
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append($4);
			$$->append($5);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	;
value_exp_primary:
		unsigned_value_spec
	  | column_ref
	  | set_fct_spec
	  | scalar_subquery
	  | case_expression
	  | window_function
	  | '(' value_exp ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($2);
			$$->append(newNode(")", SQLNodeType::Punctuation));
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
			$$->append(newNode("-", SQLNodeType::Punctuation));
			$$->append($2);
		}
	|	'+' num_primary  %prec SQL_TOKEN_UMINUS
		{
			$$ = SQL_NEW_RULE;
			$$->append(newNode("+", SQLNodeType::Punctuation));
			$$->append($2);
		}
	;

term:
		factor
	  | term '*' factor
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("*", SQLNodeType::Punctuation));
			$$->append($3);
		}
	  | term '/' factor
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("/", SQLNodeType::Punctuation));
			$$->append($3);
		}
	  ;

num_value_exp:
		term
	  | num_value_exp '+' term
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("+", SQLNodeType::Punctuation));
			$$->append($3);
		}
	  | num_value_exp '-' term
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("-", SQLNodeType::Punctuation));
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
			$$->append(newNode("*", SQLNodeType::Punctuation));
			$$->append($3);
		}
	  | interval_term '/' factor
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("/", SQLNodeType::Punctuation));
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
			$$->append(newNode("+", SQLNodeType::Punctuation));
			$$->append($3);
		}
	  | datetime_value_exp '+' interval_term
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("+", SQLNodeType::Punctuation));
			$$->append($3);
		}
	  | datetime_value_exp '-' interval_term
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("-", SQLNodeType::Punctuation));
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
			$$->append(newNode("+", SQLNodeType::Punctuation));
			$$->append($3);
		}
	  | interval_value_exp '-' interval_term
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("-", SQLNodeType::Punctuation));
			$$->append($3);
		}
	  | '(' datetime_value_exp '-' datetime_term ')' interval_qualifier
		{
			$$ = SQL_NEW_RULE;
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($2);
			$$->append(newNode("-", SQLNodeType::Punctuation));
			$$->append($4);
			$$->append(newNode(")", SQLNodeType::Punctuation));
			$$->append($6);
		}
	;
*/
non_second_datetime_field:
		SQL_TOKEN_YEAR
	|	SQL_TOKEN_YEARDAY
	|	SQL_TOKEN_MONTH
	|	SQL_TOKEN_WEEK
	|	SQL_TOKEN_WEEKDAY
	|	SQL_TOKEN_DAY
	|	SQL_TOKEN_HOUR
	|	SQL_TOKEN_MINUTE
	|	SQL_TOKEN_MILLISECOND
	;

datetime_unit:
		SQL_TOKEN_YEAR
	|	SQL_TOKEN_MONTH
	|	SQL_TOKEN_WEEK
	|	SQL_TOKEN_DAY
	|	SQL_TOKEN_HOUR
	|	SQL_TOKEN_MINUTE
	|	SQL_TOKEN_SECOND
	|	SQL_TOKEN_MILLISECOND
	;

start_field:
		non_second_datetime_field opt_paren_precision
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;
end_field:
		non_second_datetime_field
	|	SQL_TOKEN_SECOND opt_paren_precision
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;

single_datetime_field:
		non_second_datetime_field opt_paren_precision
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	|	SQL_TOKEN_SECOND opt_paren_precision_scale
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;

interval_qualifier:
		start_field SQL_TOKEN_TO end_field
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	|	single_datetime_field
	;

function_arg_commalist2:
	    function_arg ',' function_arg
			{$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);
			$$->append($3);}
	;
function_arg_commalist3:
	    function_arg ',' function_arg ',' function_arg
		{
			$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);
			$$->append($3);
			$$->append($5);
		}
	;
function_arg_commalist4:
	    function_arg ',' function_arg ',' function_arg ',' function_arg
		{
			$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);
			$$->append($3);
			$$->append($5);
			$$->append($7);
		}
	;
value_exp_commalist:
		value_exp
			{$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);}
	|   value_exp_commalist ',' value_exp
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
function_arg:
		result
	|	value_exp comparison value_exp
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	|	value_exp SQL_TOKEN_USING value_exp comparison value_exp
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	|	value_exp SQL_TOKEN_BY value_exp_commalist
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	;
function_args_commalist:
		function_arg
			{$$ = SQL_NEW_COMMALISTRULE;
			$$->append($1);}
	|   function_args_commalist ',' function_arg
			{$1->append($3);
			$$ = $1;}
	/*	this rule is only valid if we check predicates */
	|   function_args_commalist ';' function_arg
		{
			if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
			{
				$1->append($3);
				$$ = $1;
			}
			else
				YYERROR;
		}
	|	datetime_unit ',' function_arg ',' function_arg
        {
            $$ = SQL_NEW_COMMALISTRULE;
            $$->append($1);
            $$->append($3);
            $$->append($5);
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
			$$->append(newNode("+", SQLNodeType::Punctuation));
			$$->append($3);
		}
	|	value_exp SQL_CONCAT value_exp
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	;

char_primary:
			SQL_TOKEN_STRING
	  |		string_value_fct
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
	  | bit_value_fct
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
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append($4);
			$$->append($5);
			$$->append($6);
			$$->append(newNode(")", SQLNodeType::Punctuation));
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
			$$->append(newNode("+", SQLNodeType::Punctuation));
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
	  | fold
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
	|	SQL_TOKEN_FOR value_exp
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;
char_substring_fct:
		SQL_TOKEN_SUBSTRING '(' value_exp SQL_TOKEN_FROM value_exp for_length ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append($4);
			$$->append($5);
			$$->append($6);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	|	SQL_TOKEN_SUBSTRING '(' value_exp_commalist ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	;
upper_lower:
		SQL_TOKEN_UPPER
	|	SQL_TOKEN_LOWER
	;
fold:
		upper_lower '(' value_exp ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	;
form_conversion:
		SQL_TOKEN_CONVERT '(' string_value_exp SQL_TOKEN_USING table_node ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append($4);
			$$->append($5);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	|	SQL_TOKEN_CONVERT '(' cast_operand ',' cast_target ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(",", SQLNodeType::Punctuation));
			$$->append($5);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	;
char_translation:
		SQL_TOKEN_TRANSLATE '(' string_value_exp SQL_TOKEN_USING table_node ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append($4);
			$$->append($5);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	;
trim_fct:
		SQL_TOKEN_TRIM  '(' trim_operands ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	;
trim_operands:
    trim_spec value_exp SQL_TOKEN_FROM value_exp
        {
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	| trim_spec SQL_TOKEN_FROM value_exp
        {
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	| value_exp SQL_TOKEN_FROM value_exp
        {
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	| SQL_TOKEN_FROM value_exp
	    {
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	| value_exp
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
			$$->append(newNode(".", SQLNodeType::Punctuation));
			$$->append($3);
		}
	|	SQL_TOKEN_NAME ':' schema_name
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode(":", SQLNodeType::Punctuation));
			$$->append($3);
		}
;
schema_name:
		SQL_TOKEN_NAME '.' table_name
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode(".", SQLNodeType::Punctuation));
			$$->append($3);
		}
;

table_name:
			SQL_TOKEN_NAME
			{$$ = SQL_NEW_RULE;
			$$->append($1);}
;
/* Columns */
column_ref:
			column
			{$$ = SQL_NEW_RULE;
			$$->append($1);}
/*	|       table_node '.' column_val %prec '.'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode(".", SQLNodeType::Punctuation));
			$$->append($3);}
*/
	|       SQL_TOKEN_NAME '.' column_val %prec '.'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode(".", SQLNodeType::Punctuation));
			$$->append($3);
			}
	|       SQL_TOKEN_NAME '.' SQL_TOKEN_NAME '.' column_val %prec '.'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode(".", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(".", SQLNodeType::Punctuation));
			$$->append($5);}
	|       SQL_TOKEN_NAME '.' SQL_TOKEN_NAME '.' SQL_TOKEN_NAME '.' column_val %prec '.'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode(".", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(".", SQLNodeType::Punctuation));
			$$->append($5);
			$$->append(newNode(".", SQLNodeType::Punctuation));
			$$->append($7);
			}
	|       SQL_TOKEN_NAME ':' SQL_TOKEN_NAME '.' SQL_TOKEN_NAME '.' column_val %prec '.'
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode(":", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(".", SQLNodeType::Punctuation));
			$$->append($5);
			$$->append(newNode(".", SQLNodeType::Punctuation));
			$$->append($7);
			}
/*	|       SQL_TOKEN_NAME ';' SQL_TOKEN_NAME '.' SQL_TOKEN_NAME '.' column_val
			{$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode(";", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(".", SQLNodeType::Punctuation));
			$$->append($5);
			$$->append(newNode(".", SQLNodeType::Punctuation));
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
			$$->append(newNode("*", SQLNodeType::Punctuation));
		}
	;
data_type:
	predefined_type
	;
opt_char_set_spec:
	{$$ = SQL_NEW_RULE;}
	|	SQL_TOKEN_CHARACTER SQL_TOKEN_SET SQL_TOKEN_NAME
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	;
opt_collate_clause:
	{$$ = SQL_NEW_RULE;}
	| collate_clause
	;
predefined_type:
		character_string_type opt_char_set_spec	opt_collate_clause
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	|	national_character_string_type opt_collate_clause
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	|	binary_string_type
	|	numeric_type
	|	boolean_type
	|	datetime_type
	|	interval_type
	;
character_string_type:
		SQL_TOKEN_CHARACTER opt_paren_precision
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	|	SQL_TOKEN_CHAR opt_paren_precision
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	|	SQL_TOKEN_CHARACTER SQL_TOKEN_VARYING paren_char_length
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	|	SQL_TOKEN_CHAR SQL_TOKEN_VARYING paren_char_length
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	|	SQL_TOKEN_VARCHAR paren_char_length
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	| character_large_object_type
	;
opt_paren_precision:
		{$$ = SQL_NEW_RULE;}
	|	paren_char_length
	;
paren_char_length:
	'(' SQL_TOKEN_INTNUM ')'
	{
		$$ = SQL_NEW_RULE;
		$$->append(newNode("(", SQLNodeType::Punctuation));
		$$->append($2);
		$$->append(newNode(")", SQLNodeType::Punctuation));
	}
	;
opt_paren_char_large_length:
		{$$ = SQL_NEW_RULE;}
	|	paren_character_large_object_length
	;
paren_character_large_object_length:
	'(' large_object_length ')'
	{
		$$ = SQL_NEW_RULE;
		$$->append(newNode("(", SQLNodeType::Punctuation));
		$$->append($2);
		$$->append(newNode(")", SQLNodeType::Punctuation));
	}
	;

large_object_length:
	SQL_TOKEN_INTNUM opt_multiplier
	{
		$$ = SQL_NEW_RULE;
		$$->append($1);
		$$->append($2);
	}
	;
opt_multiplier:
	{$$ = SQL_NEW_RULE;}
	|	'K'
		{
			$$ = SQL_NEW_RULE;
			$$->append(newNode("K", SQLNodeType::Punctuation));
		}
	|	'M'
		{
			$$ = SQL_NEW_RULE;
			$$->append(newNode("M", SQLNodeType::Punctuation));
		}
	|	'G'
		{
			$$ = SQL_NEW_RULE;
			$$->append(newNode("G", SQLNodeType::Punctuation));
		}
	|	'T'
		{
			$$ = SQL_NEW_RULE;
			$$->append(newNode("T", SQLNodeType::Punctuation));
		}
	|	'P'
		{
			$$ = SQL_NEW_RULE;
			$$->append(newNode("P", SQLNodeType::Punctuation));
		}
	;
character_large_object_type:
		SQL_TOKEN_CHARACTER SQL_TOKEN_LARGE SQL_TOKEN_OBJECT opt_paren_char_large_length
	{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	|	SQL_TOKEN_CHAR SQL_TOKEN_LARGE SQL_TOKEN_OBJECT opt_paren_char_large_length
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	|	SQL_TOKEN_CLOB opt_paren_char_large_length
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;
national_character_string_type:
		SQL_TOKEN_NATIONAL SQL_TOKEN_CHARACTER opt_paren_precision
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	|	SQL_TOKEN_NATIONAL SQL_TOKEN_CHAR opt_paren_precision
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	|	SQL_TOKEN_NCHAR opt_paren_precision
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	|	SQL_TOKEN_NATIONAL SQL_TOKEN_CHARACTER SQL_TOKEN_VARYING paren_char_length
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	|	SQL_TOKEN_NATIONAL SQL_TOKEN_CHAR SQL_TOKEN_VARYING paren_char_length
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	|	SQL_TOKEN_NCHAR SQL_TOKEN_VARYING paren_char_length
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	|	national_character_large_object_type
	;
national_character_large_object_type:
		SQL_TOKEN_NATIONAL SQL_TOKEN_CHARACTER SQL_TOKEN_LARGE SQL_TOKEN_OBJECT opt_paren_char_large_length
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
			$$->append($5);
		}
	|	SQL_TOKEN_NCHAR SQL_TOKEN_LARGE SQL_TOKEN_OBJECT opt_paren_char_large_length
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	|	SQL_TOKEN_NCLOB opt_paren_char_large_length
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;
binary_string_type:
		SQL_TOKEN_BINARY opt_paren_precision
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	|	SQL_TOKEN_BINARY SQL_TOKEN_VARYING paren_char_length
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	|	SQL_TOKEN_VARBINARY paren_char_length
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	|	binary_large_object_string_type
	;
binary_large_object_string_type:
		SQL_TOKEN_BINARY SQL_TOKEN_LARGE SQL_TOKEN_OBJECT opt_paren_char_large_length
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	|	SQL_TOKEN_BLOB opt_paren_char_large_length
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;
numeric_type:
		exact_numeric_type
	|	approximate_numeric_type
	;
opt_paren_precision_scale:
	{$$ = SQL_NEW_RULE;}
	|	'(' SQL_TOKEN_INTNUM ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($2);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	|	'(' SQL_TOKEN_INTNUM ',' SQL_TOKEN_INTNUM ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($2);
			$$->append(newNode(",", SQLNodeType::Punctuation));
			$$->append($4);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	;
exact_numeric_type:
		SQL_TOKEN_NUMERIC opt_paren_precision_scale
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	|	SQL_TOKEN_DECIMAL opt_paren_precision_scale
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	|	SQL_TOKEN_DEC opt_paren_precision_scale
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	|	SQL_TOKEN_SMALLINT
	|	SQL_TOKEN_INTEGER
	|	SQL_TOKEN_INT
	|	SQL_TOKEN_BIGINT
	;
approximate_numeric_type:
		SQL_TOKEN_FLOAT '(' SQL_TOKEN_INTNUM ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	|	SQL_TOKEN_FLOAT
	|	SQL_TOKEN_REAL
	|	SQL_TOKEN_DOUBLE
	|	SQL_TOKEN_DOUBLE SQL_TOKEN_PRECISION
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;
boolean_type:
	SQL_TOKEN_BOOLEAN
;
datetime_type:
		SQL_TOKEN_DATE
	|	SQL_TOKEN_TIME opt_paren_precision opt_with_or_without_time_zone
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	|	SQL_TOKEN_TIMESTAMP opt_paren_precision opt_with_or_without_time_zone
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	;
opt_with_or_without_time_zone:
		{$$ = SQL_NEW_RULE;}
	|	SQL_TOKEN_WITH SQL_TOKEN_TIME SQL_TOKEN_ZONE
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	|	SQL_TOKEN_WITHOUT SQL_TOKEN_TIME SQL_TOKEN_ZONE
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
		}
	;
interval_type:
	SQL_TOKEN_INTERVAL interval_qualifier
	{
		$$ = SQL_NEW_RULE;
		$$->append($1);
		$$->append($2);
	}
	;
	/* the various things you can name */

column:
		SQL_TOKEN_NAME
	|	SQL_TOKEN_POSITION
		{
			sal_uInt32 nNod = $$->getRuleID();
			delete $$;
			$$ = newNode(OSQLParser::TokenIDToStr(nNod), SQLNodeType::Name);
		}
	|	SQL_TOKEN_CHAR_LENGTH
		{
			sal_uInt32 nNod = $$->getRuleID();
			delete $$;
			$$ = newNode(OSQLParser::TokenIDToStr(nNod), SQLNodeType::Name);
		}
	|	SQL_TOKEN_EXTRACT
		{
			sal_uInt32 nNod = $$->getRuleID();
			delete $$;
			$$ = newNode(OSQLParser::TokenIDToStr(nNod), SQLNodeType::Name);
		}
	;
case_expression:
		case_abbreviation
	|	case_specification
	;
case_abbreviation:
		SQL_TOKEN_NULLIF '(' value_exp_commalist ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	|	SQL_TOKEN_COALESCE '(' value_exp ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	|	SQL_TOKEN_COALESCE '(' value_exp_commalist ')'
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append(newNode("(", SQLNodeType::Punctuation));
			$$->append($3);
			$$->append(newNode(")", SQLNodeType::Punctuation));
		}
	;
case_specification:
		simple_case
	|	searched_case
	;
simple_case:
	SQL_TOKEN_CASE case_operand simple_when_clause_list else_clause SQL_TOKEN_END
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
			$$->append($5);
		}
	;
searched_case:
	SQL_TOKEN_CASE searched_when_clause_list else_clause SQL_TOKEN_END
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	;
simple_when_clause_list:
		simple_when_clause
		{
			$$ = SQL_NEW_LISTRULE;
			$$->append($1);
		}
	|   searched_when_clause_list simple_when_clause
		{
			$1->append($2);
			$$ = $1;
		}
	;
simple_when_clause:
	SQL_TOKEN_WHEN when_operand_list SQL_TOKEN_THEN result
	{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	;
when_operand_list:
		when_operand
		{$$ = SQL_NEW_COMMALISTRULE;
		$$->append($1);}
	|   when_operand_list ',' when_operand
		{$1->append($3);
		$$ = $1;}
	;
when_operand:
		row_value_constructor_elem
	|	comparison_predicate_part_2        %dprec 1
	|	between_predicate_part_2
	|	in_predicate_part_2
	|	character_like_predicate_part_2
	|	null_predicate_part_2              %dprec 2
;
searched_when_clause_list:
		searched_when_clause
		{
			$$ = SQL_NEW_LISTRULE;
			$$->append($1);
		}
	|   searched_when_clause_list searched_when_clause
		{
			$1->append($2);
			$$ = $1;
		}
	;
searched_when_clause:
	SQL_TOKEN_WHEN search_condition SQL_TOKEN_THEN result
	{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
			$$->append($3);
			$$->append($4);
		}
	;
else_clause:
		{$$ = SQL_NEW_RULE;}
	|	SQL_TOKEN_ELSE result
		{
			$$ = SQL_NEW_RULE;
			$$->append($1);
			$$->append($2);
		}
	;
result:
		result_expression
	;
result_expression:
	value_exp
	;
case_operand:
	row_value_constructor_elem
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
			$$->append(newNode(":", SQLNodeType::Punctuation));
			$$->append($2);}
	|	'?'
			{$$ = SQL_NEW_RULE; // test
			$$->append(newNode("?", SQLNodeType::Punctuation));}
	|	'['	SQL_TOKEN_NAME ']'
			{$$ = SQL_NEW_RULE;
			$$->append(newNode("[", SQLNodeType::Punctuation));
			$$->append($2);
			$$->append(newNode("]", SQLNodeType::Punctuation));}
	;

/***
procedure:      SQL_TOKEN_NAME
			{$$ = SQL_NEW_RULE;
			$$->append($1);}
	;
***/

range_variable:
        {$$ = SQL_NEW_RULE;}
    |   opt_as SQL_TOKEN_NAME
        {$$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        }
	;

user:	SQL_TOKEN_NAME
	;

/* PREDICATECHECK RULES */
sql:
		search_condition /* checking predicats */
		{
			if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck()) // sql: rule 1
			{
				$$ = $1;
				if ( SQL_ISRULE($$,search_condition) )
				{
					$$->insert(0,newNode("(", SQLNodeType::Punctuation));
					$$->append(newNode(")", SQLNodeType::Punctuation));
				}
			}
			else
				YYERROR;
		}
	|   '(' sql ')' /* checking predicats */
    ;
trigger_definition:
	SQL_TOKEN_CREATE SQL_TOKEN_TRIGGER trigger_name trigger_action_time trigger_event SQL_TOKEN_ON table_name op_referencing triggered_action
	{
		$$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
        $$->append($4);
        $$->append($5);
        $$->append($6);
        $$->append($7);
        $$->append($8);
        $$->append($9);
    }
	;
op_referencing:
	{
		$$ = SQL_NEW_RULE;
	}
	|	SQL_TOKEN_REFERENCING transition_table_or_variable_list
	{
		$$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
    }
	;
trigger_action_time:
		SQL_TOKEN_BEFORE
	|	SQL_TOKEN_AFTER
	|	SQL_TOKEN_INSTEAD SQL_TOKEN_OF
	{
		$$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
    }
;
trigger_event:
		SQL_TOKEN_INSERT
	|	SQL_TOKEN_DELETE
	|	SQL_TOKEN_UPDATE op_trigger_columnlist
	{
		$$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
    }
	;
op_trigger_columnlist:
	{
		$$ = SQL_NEW_RULE;
	}
	|	SQL_TOKEN_OF trigger_column_list
	{
		$$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
    }
	;
trigger_column_list:
	column_commalist
	;
triggered_action:
	op_triggered_action_for triggered_when_clause triggered_SQL_statement
	{
		$$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
    }
	;
op_triggered_action_for:
		{
		$$ = SQL_NEW_RULE;
		}
	|	SQL_TOKEN_FOR SQL_TOKEN_EACH trigger_for
	{
		$$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
    }
	;
trigger_for:
		SQL_TOKEN_ROW
	|	SQL_TOKEN_STATEMENT
	;
triggered_when_clause:
	{
		$$ = SQL_NEW_RULE;
	}
	|	SQL_TOKEN_WHEN parenthesized_boolean_value_expression
	{
		$$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
    }
	;
triggered_SQL_statement:
		SQL_procedure_statement
	|	SQL_TOKEN_BEGIN SQL_TOKEN_ATOMIC SQL_procedure_statement_list ';' SQL_TOKEN_END
	{
		$$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
        $$->append(newNode(";", SQLNodeType::Punctuation));
        $$->append($5);
    }
	;
SQL_procedure_statement_list:
		SQL_procedure_statement
		{
			$$ = SQL_NEW_LISTRULE;
			$$->append($1);
		}
	|	SQL_procedure_statement_list ';' SQL_procedure_statement
		{
			$1->append($3);
			$$ = $1;
		}
	;
SQL_procedure_statement:
	sql
	;

transition_table_or_variable_list:
		transition_table_or_variable
		{
			$$ = SQL_NEW_LISTRULE;
			$$->append($1);
		}
	|   transition_table_or_variable_list transition_table_or_variable
		{
			$1->append($2);
			$$ = $1;
		}
	;

transition_table_or_variable:
		SQL_TOKEN_OLD opt_row opt_as old_transition_variable_name
	{
		$$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
        $$->append($4);
    }
	|	SQL_TOKEN_NEW opt_row opt_as new_transition_variable_name
	{
		$$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
        $$->append($4);
    }
	|	SQL_TOKEN_OLD SQL_TOKEN_TABLE opt_as old_transition_table_name
	{
		$$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
        $$->append($4);
    }
	|	SQL_TOKEN_NEW SQL_TOKEN_TABLE opt_as new_transition_table_name
	{
		$$ = SQL_NEW_RULE;
        $$->append($1);
        $$->append($2);
        $$->append($3);
        $$->append($4);
    }
;
old_transition_table_name:
	transition_table_name
;
new_transition_table_name:
	transition_table_name
;
transition_table_name:
	SQL_TOKEN_NAME
;
old_transition_variable_name:
	SQL_TOKEN_NAME
;
new_transition_variable_name:
	SQL_TOKEN_NAME
;
trigger_name:
	SQL_TOKEN_NAME
;
%%

#if defined _MSC_VER
#pragma warning(pop)
#endif

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::osl;
using namespace ::dbtools;

connectivity::OSQLParser* xxx_pGLOBAL_SQLPARSER;

connectivity::OSQLInternalNode* newNode(const char* pNewValue,
        const connectivity::SQLNodeType eNodeType,
        const sal_uInt32 nNodeID)
{
    return new connectivity::OSQLInternalNode(pNewValue, eNodeType, nNodeID);
}

connectivity::OSQLInternalNode* newNode(const OString& _newValue,
        const connectivity::SQLNodeType eNodeType,
        const sal_uInt32 nNodeID)
{
    return new connectivity::OSQLInternalNode(_newValue, eNodeType, nNodeID);
}

connectivity::OSQLInternalNode* newNode(const OUString& _newValue,
        const connectivity::SQLNodeType eNodeType,
        const sal_uInt32 nNodeID)
{
    return new connectivity::OSQLInternalNode(_newValue, eNodeType, nNodeID);
}

OParseContext::OParseContext()
{
}


OParseContext::~OParseContext()
{
}


OUString OParseContext::getErrorMessage(ErrorCode _eCode) const
{
    OUString aMsg;
    switch (_eCode)
    {
        case ErrorCode::General:              aMsg = "Syntax error in SQL expression"; break;
        case ErrorCode::ValueNoLike:          aMsg = "The value #1 can not be used with LIKE."; break;
        case ErrorCode::FieldNoLike:          aMsg = "LIKE can not be used with this field."; break;
        case ErrorCode::InvalidCompare:       aMsg = "The entered criterion can not be compared with this field."; break;
        case ErrorCode::InvalidIntCompare:    aMsg = "The field can not be compared with a number."; break;
        case ErrorCode::InvalidDateCompare:   aMsg = "The field can not be compared with a date."; break;
        case ErrorCode::InvalidRealCompare:   aMsg = "The field can not be compared with a floating point number."; break;
        case ErrorCode::InvalidTableNosuch:   aMsg = "The database does not contain a table named \"#\"."; break;
        case ErrorCode::InvalidTableOrQuery:  aMsg = "The database does contain neither a table nor a query named \"#\"."; break;
        case ErrorCode::InvalidColumn:        aMsg = "The column \"#1\" is unknown in the table \"#2\"."; break;
        case ErrorCode::InvalidTableExist:    aMsg = "The database already contains a table or view with name \"#\"."; break;
        case ErrorCode::InvalidQueryExist:    aMsg = "The database already contains a query with name \"#\"."; break;
        default:
            OSL_FAIL( "OParseContext::getErrorMessage: unknown error code!" );
            break;
    }
    return aMsg;
}


OString OParseContext::getIntlKeywordAscii(InternationalKeyCode _eKey) const
{
	OString aKeyword;
	switch (_eKey)
	{
		case InternationalKeyCode::Like:		aKeyword = "LIKE"; break;
		case InternationalKeyCode::Not:		aKeyword = "NOT"; break;
		case InternationalKeyCode::Null:		aKeyword = "NULL"; break;
		case InternationalKeyCode::True:		aKeyword = "True"; break;
		case InternationalKeyCode::False:		aKeyword = "False"; break;
		case InternationalKeyCode::Is:		aKeyword = "IS"; break;
		case InternationalKeyCode::Between:	aKeyword = "BETWEEN"; break;
		case InternationalKeyCode::Or:		aKeyword = "OR"; break;
		case InternationalKeyCode::And:		aKeyword = "AND"; break;
		case InternationalKeyCode::Avg:		aKeyword = "AVG"; break;
		case InternationalKeyCode::Count:		aKeyword = "COUNT"; break;
		case InternationalKeyCode::Max:		aKeyword = "MAX"; break;
		case InternationalKeyCode::Min:		aKeyword = "MIN"; break;
		case InternationalKeyCode::Sum:		aKeyword = "SUM"; break;
        case InternationalKeyCode::Every:     aKeyword = "EVERY"; break;
        case InternationalKeyCode::Any:       aKeyword = "ANY"; break;
        case InternationalKeyCode::Some:      aKeyword = "SOME"; break;
        case InternationalKeyCode::StdDevPop: aKeyword = "STDDEV_POP"; break;
        case InternationalKeyCode::StdDevSamp: aKeyword = "STDDEV_SAMP"; break;
        case InternationalKeyCode::VarSamp:  aKeyword = "VAR_SAMP"; break;
        case InternationalKeyCode::VarPop:   aKeyword = "VAR_POP"; break;
        case InternationalKeyCode::Collect:   aKeyword = "COLLECT"; break;
        case InternationalKeyCode::Fusion:    aKeyword = "FUSION"; break;
        case InternationalKeyCode::Intersection:aKeyword = "INTERSECTION"; break;
        case InternationalKeyCode::None:      break;
        default:
            OSL_FAIL( "OParseContext::getIntlKeywordAscii: unknown key!" );
            break;
	}
	return aKeyword;
}


IParseContext::InternationalKeyCode OParseContext::getIntlKeyCode(const OString& rToken) const
{
    static IParseContext::InternationalKeyCode const Intl_TokenID[] =
	{
		InternationalKeyCode::Like, InternationalKeyCode::Not, InternationalKeyCode::Null, InternationalKeyCode::True,
		InternationalKeyCode::False, InternationalKeyCode::Is, InternationalKeyCode::Between, InternationalKeyCode::Or,
		InternationalKeyCode::And, InternationalKeyCode::Avg, InternationalKeyCode::Count, InternationalKeyCode::Max,
		InternationalKeyCode::Min, InternationalKeyCode::Sum, InternationalKeyCode::Every,InternationalKeyCode::Any,InternationalKeyCode::Some,
        InternationalKeyCode::StdDevPop,InternationalKeyCode::StdDevSamp,InternationalKeyCode::VarSamp,
        InternationalKeyCode::VarPop,InternationalKeyCode::Collect,InternationalKeyCode::Fusion,InternationalKeyCode::Intersection
	};

    auto const token = std::find_if(std::cbegin(Intl_TokenID), std::cend(Intl_TokenID)
                                    , [&rToken, this](IParseContext::InternationalKeyCode const & tokenID)
                                      { return rToken.equalsIgnoreAsciiCase(getIntlKeywordAscii(tokenID)); });

    if (std::cend(Intl_TokenID) != token)
        return *token;

    return InternationalKeyCode::None;
}


static Locale& impl_getLocaleInstance( )
{
	static Locale s_aLocale( "en", "US", "" );
	return s_aLocale;
}


Locale OParseContext::getPreferredLocale( ) const
{
	return getDefaultLocale();
}


const Locale& OParseContext::getDefaultLocale()
{
	return impl_getLocaleInstance();
}

// The (unfortunately global) yylval for the handing over of
// values from the Scanner to the Parser. The global variable
// is only used for a short term, the Parser reads the variable
// immediately after the call of the Scanner into a same named own
// member variable.


OUString ConvertLikeToken(const OSQLParseNode* pTokenNode, const OSQLParseNode* pEscapeNode, bool bInternational)
{
	OUStringBuffer aMatchStr(0);
	if (pTokenNode->isToken())
	{
		sal_Unicode cEscape = 0;
		if (pEscapeNode->count())
			cEscape = pEscapeNode->getChild(1)->getTokenValue().toChar();

		// Change place holder
		aMatchStr = pTokenNode->getTokenValue();
		const sal_Int32 nLen = aMatchStr.getLength();
		OUStringBuffer sSearch,sReplace;
		if ( bInternational )
		{
		    sSearch.append("%_");
		    sReplace.append("*?");
		}
		else
		{
		    sSearch.append("*?");
		    sReplace.append("%_");
		}

		bool wasEscape = false;
		for (sal_Int32 i = 0; i < nLen; i++)
		{
			const sal_Unicode c = aMatchStr[i];
			// SQL standard requires the escape to be followed
			// by a meta-character ('%', '_' or itself), else error
			// We are more lenient here and let it escape anything.
			// Especially since some databases (e.g. Microsoft SQL Server)
			// have more meta-characters than the standard, such as e.g. '[' and ']'
			if (wasEscape)
			{
				wasEscape=false;
				continue;
			}
			if (c == cEscape)
			{
				wasEscape=true;
				continue;
			}
			int match = -1;
			if (c == sSearch[0])
				match=0;
			else if (c == sSearch[1])
				match=1;

			if (match != -1)
			{
				aMatchStr[i] = sReplace[match];
			}
		}
	}
	return aMatchStr.makeStringAndClear();
}

sal_uInt32			    OSQLParser::s_nRuleIDs[OSQLParseNode::rule_count + 1];
OSQLParser::RuleIDMap   OSQLParser::s_aReverseRuleIDLookup;
OParseContext		    OSQLParser::s_aDefaultContext;

sal_Int32			OSQLParser::s_nRefCount	= 0;
//	::osl::Mutex		OSQLParser::s_aMutex;
OSQLScanner*		OSQLParser::s_pScanner = nullptr;
OSQLParseNodesGarbageCollector*		OSQLParser::s_pGarbageCollector = nullptr;
vcl::DeleteOnDeinit<css::uno::Reference< css::i18n::XLocaleData4>> OSQLParser::s_xLocaleData(vcl::DeleteOnDeinitFlag::Empty);

void setParser(OSQLParser* _pParser)
{
	xxx_pGLOBAL_SQLPARSER = _pParser;
}

void OSQLParser::setParseTree(OSQLParseNode* pNewParseTree)
{
	m_pParseTree.reset(pNewParseTree);
}


/** Delete all comments in a query.

    See also getComment()/concatComment() implementation for
    OQueryController::translateStatement().
 */
static OUString delComment( const OUString& rQuery )
{
    // First a quick search if there is any "--" or "//" or "/*", if not then the whole
    // copying loop is pointless.
    if (rQuery.indexOf("--") < 0 && rQuery.indexOf("//") < 0 &&
            rQuery.indexOf("/*") < 0)
        return rQuery;

    const sal_Unicode* pCopy = rQuery.getStr();
    sal_Int32 nQueryLen = rQuery.getLength();
    bool bIsText1  = false;     // "text"
    bool bIsText2  = false;     // 'text'
    bool bComment2 = false;     // /* comment */
    bool bComment  = false;     // -- or // comment
    OUStringBuffer aBuf(nQueryLen);
    for (sal_Int32 i=0; i < nQueryLen; ++i)
    {
        if (bComment2)
        {
            if ((i+1) < nQueryLen)
            {
                if (pCopy[i]=='*' && pCopy[i+1]=='/')
                {
                    bComment2 = false;
                    ++i;
                }
            }
            else
            {
                // comment can't close anymore, actually an error, but...
            }
            continue;
        }
        if (pCopy[i] == '\n')
            bComment = false;
        else if (!bComment)
        {
            if (pCopy[i] == '\"' && !bIsText2)
                bIsText1 = !bIsText1;
            else if (pCopy[i] == '\'' && !bIsText1)
                bIsText2 = !bIsText2;
            if (!bIsText1 && !bIsText2 && (i+1) < nQueryLen)
            {
                if ((pCopy[i]=='-' && pCopy[i+1]=='-') || (pCopy[i]=='/' && pCopy[i+1]=='/'))
                    bComment = true;
                else if ((pCopy[i]=='/' && pCopy[i+1]=='*'))
                    bComment2 = true;
            }
        }
        if (!bComment && !bComment2)
            aBuf.append( &pCopy[i], 1);
    }
    return aBuf.makeStringAndClear();
}

std::unique_ptr<OSQLParseNode> OSQLParser::parseTree(OUString& rErrorMessage,
                                     const OUString& rStatement,
                                     bool bInternational)
{


	// Guard the parsing
	std::unique_lock aGuard(getMutex());
	// must be reset
	setParser(this);

	// delete comments before parsing
	OUString sTemp = delComment(rStatement);

	// defines how to scan
	s_pScanner->SetRule(OSQLScanner::GetSQLRule()); // initial
	s_pScanner->prepareScan(sTemp, m_pContext, bInternational);

	SQLyylval.pParseNode = nullptr;
	//	SQLyypvt = NULL;
	m_pParseTree = nullptr;
	m_sErrorMessage = "";

    // start parsing
	if (SQLyyparse() != 0)
	{
		// only set the error message, if it's not already set
		if (m_sErrorMessage.isEmpty())
			m_sErrorMessage = s_pScanner->getErrorMessage();
		if (m_sErrorMessage.isEmpty())
			m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ErrorCode::General);

		rErrorMessage = m_sErrorMessage;

		// clear the garbage collector
		(*s_pGarbageCollector)->clearAndDelete();
		// coverity[leaked_storage : FALSE] - because the garbage collector deleted it
		m_pParseTree.release();
		return nullptr;
	}
	else
	{
		(*s_pGarbageCollector)->clear();

        // return result:
        // to work around a bug in MKS YACC return the member m_pParseTree
        // instead of Sdbyyval.pParseNode

        SAL_WARN_IF(!m_pParseTree, "connectivity.parse",
            "OSQLParser: Parser did not create ParseTree");
		return std::move(m_pParseTree);
	}
}

OString OSQLParser::TokenIDToStr(sal_uInt32 nTokenID, const IParseContext* pContext)
{
	OString aStr;
	if (pContext)
	{
		IParseContext::InternationalKeyCode eKeyCode = IParseContext::InternationalKeyCode::None;
		switch( nTokenID )
		{
			case SQL_TOKEN_LIKE: eKeyCode = IParseContext::InternationalKeyCode::Like; break;
			case SQL_TOKEN_NOT: eKeyCode = IParseContext::InternationalKeyCode::Not; break;
			case SQL_TOKEN_NULL: eKeyCode = IParseContext::InternationalKeyCode::Null; break;
			case SQL_TOKEN_TRUE: eKeyCode = IParseContext::InternationalKeyCode::True; break;
			case SQL_TOKEN_FALSE: eKeyCode = IParseContext::InternationalKeyCode::False; break;
			case SQL_TOKEN_IS: eKeyCode = IParseContext::InternationalKeyCode::Is; break;
			case SQL_TOKEN_BETWEEN: eKeyCode = IParseContext::InternationalKeyCode::Between; break;
			case SQL_TOKEN_OR: eKeyCode = IParseContext::InternationalKeyCode::Or; break;
			case SQL_TOKEN_AND: eKeyCode = IParseContext::InternationalKeyCode::And; break;
			case SQL_TOKEN_AVG: eKeyCode = IParseContext::InternationalKeyCode::Avg; break;
			case SQL_TOKEN_COUNT: eKeyCode = IParseContext::InternationalKeyCode::Count; break;
			case SQL_TOKEN_MAX: eKeyCode = IParseContext::InternationalKeyCode::Max; break;
			case SQL_TOKEN_MIN: eKeyCode = IParseContext::InternationalKeyCode::Min; break;
			case SQL_TOKEN_SUM: eKeyCode = IParseContext::InternationalKeyCode::Sum; break;
		}
		if ( eKeyCode != IParseContext::InternationalKeyCode::None )
		    aStr = pContext->getIntlKeywordAscii(eKeyCode);
	}

	if (aStr.isEmpty())
	{
		// coverity[unsigned_compare : SUPPRESS] - YYTRANSLATE is out of our control
		aStr = yytname[YYTRANSLATE(nTokenID)];
		if(aStr.startsWith("SQL_TOKEN_"))
			aStr = aStr.copy(10);
		switch( nTokenID )
		{
			case SQL_TOKEN_OJ:
			case SQL_TOKEN_TS:
			case SQL_TOKEN_T:
			case SQL_TOKEN_D:
				aStr = aStr.toAsciiLowerCase();
		}
	}
	return aStr;
}

#if OSL_DEBUG_LEVEL > 0
OUString OSQLParser::RuleIDToStr(sal_uInt32 nRuleID)
{
    OSL_ENSURE(nRuleID < std::size(yytname), "OSQLParser::RuleIDToStr: Invalid nRuleId!");
	return OUString::createFromAscii(yytname[nRuleID]);
}
#endif


sal_uInt32 OSQLParser::StrToRuleID(const OString & rValue)
{
	// Search for the given name in yytname and return the index
	// (or UNKNOWN_RULE, if not found)
    static sal_uInt32 const nLen = std::size(yytname)-1;
    for (sal_uInt32 i = YYTRANSLATE(SQL_TOKEN_INVALIDSYMBOL); i < nLen; ++i)
	{
		if (rValue == yytname[i])
			return i;
	}

	// Not found
	return OSQLParseNode::UNKNOWN_RULE;
}


OSQLParseNode::Rule OSQLParser::RuleIDToRule( sal_uInt32 _nRule )
{
    OSQLParser::RuleIDMap::const_iterator i (s_aReverseRuleIDLookup.find(_nRule));
    if (i == s_aReverseRuleIDLookup.end())
    {
        SAL_INFO("connectivity.parse",
		 "connectivity::OSQLParser::RuleIDToRule cannot reverse-lookup rule. "
		 "Reverse mapping incomplete? "
		 "_nRule='" << _nRule << "' "
		 "yytname[_nRule]='" << yytname[_nRule] << "'");
        return OSQLParseNode::UNKNOWN_RULE;
    }
    else
        return i->second;
}


sal_uInt32 OSQLParser::RuleID(OSQLParseNode::Rule eRule)
{
	return s_nRuleIDs[(sal_uInt16)eRule];
}

sal_Int16 OSQLParser::buildNode(OSQLParseNode*& pAppend,OSQLParseNode* pCompare,OSQLParseNode* pLiteral,OSQLParseNode* pLiteral2)
{
	OSQLParseNode* pColumnRef = new OSQLInternalNode("", SQLNodeType::Rule,OSQLParser::RuleID(OSQLParseNode::column_ref));
	pColumnRef->append(new OSQLInternalNode(m_sFieldName,SQLNodeType::Name));
	OSQLParseNode* pComp = nullptr;
	if ( SQL_ISTOKEN( pCompare, BETWEEN) && pLiteral2 )
		pComp = new OSQLInternalNode("", SQLNodeType::Rule,OSQLParser::RuleID(OSQLParseNode::between_predicate_part_2));
	else
		pComp = new OSQLInternalNode("", SQLNodeType::Rule,OSQLParser::RuleID(OSQLParseNode::comparison_predicate));

	pComp->append(pColumnRef);
	pComp->append(pCompare);
	pComp->append(pLiteral);
	if ( pLiteral2 )
	{
		pComp->append(new OSQLInternalNode("", SQLNodeType::Keyword,SQL_TOKEN_AND));
		pComp->append(pLiteral2);
	}
	pAppend->append(pComp);
	return 1;
}

sal_Int16 OSQLParser::buildStringNodes(OSQLParseNode*& pLiteral)
{
	if(!pLiteral)
		return 1;

	if(SQL_ISRULE(pLiteral,set_fct_spec) || SQL_ISRULE(pLiteral,general_set_fct) || SQL_ISRULE(pLiteral,column_ref)
		|| SQL_ISRULE(pLiteral,subquery))
		return 1; // here I have a function that I can't transform into a string

	if(pLiteral->getNodeType() == SQLNodeType::IntNum || pLiteral->getNodeType() == SQLNodeType::ApproxNum || pLiteral->getNodeType() == SQLNodeType::AccessDate)
	{
		OSQLParseNode* pParent = pLiteral->getParent();

		OSQLParseNode* pNewNode = new OSQLInternalNode(pLiteral->getTokenValue(), SQLNodeType::String);
		pParent->replaceAndDelete(pLiteral, pNewNode);
		pLiteral = nullptr;
		return 1;
	}

	for(size_t i=0;i<pLiteral->count();++i)
	{
		OSQLParseNode* pChild = pLiteral->getChild(i);
		buildStringNodes(pChild);
	}
	if(SQL_ISRULE(pLiteral,term) || SQL_ISRULE(pLiteral,value_exp_primary))
	{
		m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ErrorCode::InvalidCompare);
		return 0;
	}
	return 1;
}

sal_Int16 OSQLParser::buildComparisonRule(OSQLParseNode*& pAppend,OSQLParseNode* pLiteral)
{
    OSQLParseNode* pComp = new OSQLInternalNode("=", SQLNodeType::Equal);
    return buildPredicateRule(pAppend,pLiteral,pComp);
}



void OSQLParser::reduceLiteral(OSQLParseNode*& pLiteral, bool bAppendBlank)
{
	OSL_ENSURE(pLiteral->isRule(), "This is no Rule");
	OSL_ENSURE(pLiteral->count() == 2, "OSQLParser::ReduceLiteral() Invalid count");
	OSQLParseNode* pTemp = pLiteral;
	OUStringBuffer aValue(pLiteral->getChild(0)->getTokenValue());
	if (bAppendBlank)
	{
		aValue.append(" ");
	}

	aValue.append(pLiteral->getChild(1)->getTokenValue());

	pLiteral = new OSQLInternalNode(aValue.makeStringAndClear(),SQLNodeType::String);
	delete pTemp;
}


void OSQLParser::error(const char *fmt)
{
	if(m_sErrorMessage.isEmpty())
	{
		OUString sStr(fmt,strlen(fmt),RTL_TEXTENCODING_UTF8);
		OUString sSQL_TOKEN("SQL_TOKEN_");

		sal_Int32 nPos1 = sStr.indexOf(sSQL_TOKEN);
		if(nPos1 != -1)
		{
			OUString sFirst  = sStr.copy(0,nPos1);
			sal_Int32 nPos2 = sStr.indexOf(sSQL_TOKEN,nPos1+1);
			if(nPos2 != -1)
			{
				sFirst += sStr.subView(nPos1+sSQL_TOKEN.getLength(),nPos2-nPos1-sSQL_TOKEN.getLength());
				sFirst  += sStr.subView(nPos2+sSQL_TOKEN.getLength());
			}
			else
				sFirst += sStr.subView(nPos1+sSQL_TOKEN.getLength());

			m_sErrorMessage = sFirst;
		}
		else
			m_sErrorMessage = sStr;

		OUString aError = s_pScanner->getErrorMessage();
		if(!aError.isEmpty())
		{
			m_sErrorMessage += ", ";
			m_sErrorMessage += aError;
		}
	}
}

int OSQLParser::SQLlex()
{
	return OSQLScanner::SQLlex();
}
