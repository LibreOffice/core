<!-- edited with XML Spy v3.0 NT (http://www.xmlspy.com) by Jumper (Star Office GmbH) -->
<!--
	$Id: table.mod,v 1.8 2000-10-24 09:54:22 dr Exp $

   The Contents of this file are made available subject to the terms of
   either of the following licenses

          - GNU Lesser General Public License Version 2.1
          - Sun Industry Standards Source License Version 1.1

   Sun Microsystems Inc., October, 2000

   GNU Lesser General Public License Version 2.1
   =============================================
   Copyright 2000 by Sun Microsystems, Inc.
   901 San Antonio Road, Palo Alto, CA 94303, USA

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License version 2.1, as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
   MA  02111-1307  USA


   Sun Industry Standards Source License Version 1.1
   =================================================
   The contents of this file are subject to the Sun Industry Standards
   Source License Version 1.1 (the "License"); You may not use this file
   except in compliance with the License. You may obtain a copy of the
   License at http://www.openoffice.org/license.html.

   Software provided under this License is provided on an "AS IS" basis,
   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
   See the License for the specific provisions governing your rights and
   obligations concerning the Software.

   The Initial Developer of the Original Code is: Sun Microsystems, Inc..

   Copyright: 2000 by Sun Microsystems, Inc.

   All Rights Reserved.

   Contributor(s): _______________________________________

-->
<!ENTITY % table-columns "( table:table-columns | table:table-column+ )">
<!ENTITY % table-header-columns "table:table-header-columns">
<!ENTITY % table-rows "( table:table-rows | table:table-row+ )">
<!ENTITY % table-header-rows "table:table-header-rows">
<!ENTITY % table-column-groups "( %table-header-columns; |
            (%table-columns;, %table-header-columns;, %table-columns;?) |
			(%table-header-columns;?, %table-columns;) )">
<!ENTITY % table-row-groups "( %table-header-rows; |
            (%table-rows;, %table-header-rows;, %table-rows;?) |
			(%table-header-rows;?, %table-rows;) )">
<!ELEMENT table:table (table:scenario?, %table-column-groups;, %table-row-groups;)>
<!ATTLIST table:table
	table:name %string; #REQUIRED
	table:style-name %styleName; #IMPLIED
	table:use-cell-protection %boolean; "false"
	table:cell-protection-key CDATA #IMPLIED
	table:print-ranges %cell-range-address-list; #IMPLIED
>
<!ELEMENT table:scenario EMPTY>
<!ATTLIST table:scenario
	table:display-border %boolean; "true"
	table:border-color %color; #IMPLIED
	table:copy-back %boolean; "true"
	table:copy-styles %boolean; "true"
	table:copy-formulas %boolean; "true"
	table:is-active %boolean; #REQUIRED
	table:scenario-ranges %cell-range-address-list; #REQUIRED
	table:comment CDATA #IMPLIED
>
<!ELEMENT table:subtable (%table-column-groups;, %table-row-groups;)>
<!ELEMENT table:table-header-columns (table:table-column+)>
<!ELEMENT table:table-columns (table:table-column+)>
<!ELEMENT table:table-column EMPTY>
<!ATTLIST table:table-column
	table:number-columns-repeated %positiveInteger; "1"
	table:style-name %styleName; #IMPLIED
	table:visibility (visible | collapse | filter) "visible"
>
<!ELEMENT table:table-header-rows (table:table-row+)>
<!ELEMENT table:table-rows (table:table-row+)>
<!ENTITY % table-cells "(table:table-cell|table:covered-table-cell)+">
<!ELEMENT table:table-row %table-cells;>
<!ATTLIST table:table-row
	table:number-rows-repeated %positiveInteger; "1"
	table:style-name %styleName; #IMPLIED
	table:visibility (visible | collapse | filter) "visible"
>
<!ENTITY % text-wo-table "(text:h|text:p|text:ordered-list|text:unordered-list|%shapes;|chart:chart)*">
<!ENTITY % cell-content "(office:annotation?,(table:subtable|%text-wo-table;))">
<!ELEMENT table:table-cell %cell-content;>
<!ELEMENT table:covered-table-cell %cell-content;>
<!ATTLIST table:table-cell
	table:number-columns-repeated %positiveInteger; "1"
>
<!ATTLIST table:covered-table-cell
	table:number-columns-repeated %positiveInteger; "1"
>
<!ATTLIST table:table-cell
	table:number-rows-spanned %positiveInteger; "1"
	table:number-columns-spanned %positiveInteger; "1"
	table:style-name %styleName; #IMPLIED
>
<!ATTLIST table:covered-table-cell
	table:style-name %styleName; #IMPLIED
>
<!ATTLIST table:table-cell
	table:validation-name CDATA #IMPLIED
>
<!ATTLIST table:covered-table-cell
	table:validation-name CDATA #IMPLIED
>
<!ATTLIST table:table-cell
	table:formula %string; #IMPLIED
>
<!ATTLIST table:covered-table-cell
	table:formula %string; #IMPLIED
	table:number-matrix-rows-spanned %positiveInteger; #IMPLIED
>
<!ATTLIST table:table-cell
	table:number-matrix-rows-spanned %positiveInteger; #IMPLIED
>
<!ATTLIST table:covered-table-cell
	table:number-matrix-columns-spanned %positiveInteger; #IMPLIED
>
<!ATTLIST table:table-cell
	table:number-matrix-columns-spanned %positiveInteger; #IMPLIED
	table:value-type %valueType; "string"
>
<!ATTLIST table:covered-table-cell
	table:value-type %valueType; "string"
>
<!ATTLIST table:table-cell
	table:value %float; #IMPLIED
>
<!ATTLIST table:covered-table-cell
	table:value %float; #IMPLIED
>
<!ATTLIST table:table-cell
	table:date-value %date; #IMPLIED
>
<!ATTLIST table:covered-table-cell
	table:date-value %date; #IMPLIED
>
<!ATTLIST table:table-cell
	table:time-value %timeInstance; #IMPLIED
>
<!ATTLIST table:covered-table-cell
	table:time-value %timeInstance; #IMPLIED
>
<!ATTLIST table:table-cell
	table:boolean-value %boolean; #IMPLIED
>
<!ATTLIST table:covered-table-cell
	table:boolean-value %boolean; #IMPLIED
>
<!ATTLIST table:table-cell
	table:string-value %string; #IMPLIED
>
<!ATTLIST table:covered-table-cell
	table:string-value %string; #IMPLIED
>
<!ATTLIST table:table-cell
	table:currency %string; #IMPLIED
>
<!ATTLIST table:covered-table-cell
	table:currency %string; #IMPLIED
>
<!ELEMENT table:content-validations (table:content-validation)+>
<!ELEMENT table:content-validation (table:help-message?, (table:error-message | table:error-macro)?)>
<!ATTLIST table:content-validation
	table:name CDATA #REQUIRED
	table:condition CDATA #IMPLIED
	table:base-cell-address %cell-address; #IMPLIED
	table:allow-empty-cell %boolean; #IMPLIED>
<!ELEMENT table:help-message (text:p*)>
<!ATTLIST table:help-message
	table:title CDATA #IMPLIED
	table:display %boolean; #IMPLIED
>
<!ELEMENT table:error-message (text:p*)>
<!ATTLIST table:error-message
	table:title CDATA #IMPLIED
	table:message-type (stop | warning | information) #IMPLIED
	table:display %boolean; #IMPLIED
>
<!ELEMENT table:error-macro EMPTY>
<!ATTLIST table:error-macro
	table:name CDATA #IMPLIED
	table:execute %boolean; #IMPLIED
>
<!ELEMENT table:label-ranges (table:label-range)* >
<!ELEMENT table:label-range EMPTY>
<!ATTLIST table:label-range
	table:label-cell-range-address %cell-range-address; #REQUIRED
	table:data-cell-range-address %cell-range-address; #REQUIRED
	table:orientation (column|row) #REQUIRED
>
<!ELEMENT office:annotation (%cString;)>
<!ATTLIST office:annotation
	office:author %string; #REQUIRED
	office:create-date %date; #IMPLIED
	office:create-date-string %string; #IMPLIED
	office:display %boolean; "true"
>
<!ELEMENT table:sub-table (%table-column-groups; | %table-row-groups;)>
<!ELEMENT table:named-expressions (table:named-range | table:named-expression)*>
<!ELEMENT table:named-range EMPTY>
<!ATTLIST table:named-range
	table:name CDATA #REQUIRED
	table:cell-range-address %cell-range-address; #REQUIRED
	table:base-cell-address %cell-address; #IMPLIED
	table:range-usable-as CDATA "none"
>
<!ELEMENT table:named-expression EMPTY>
<!ATTLIST table:named-expression
	table:name CDATA #REQUIRED
	table:expression CDATA #REQUIRED
	table:base-cell-address %cell-address; #IMPLIED
>
<!ELEMENT table:filter (table:filter-condition | table:filter-and | table:filter-or)>
<!ATTLIST table:filter
	table:target-range-address %cell-range-address; #IMPLIED
	table:condition-source-range-address %cell-range-address; #IMPLIED
	table:condition-source (self | cell-range) "self"
	table:display-duplicates %boolean; "true"
>
<!ELEMENT table:filter-and (table:filter-or | table:filter-condition)+>
<!ELEMENT table:filter-or (table:filter-and | table:filter-condition)+>
<!ELEMENT table:filter-condition EMPTY>
<!ATTLIST table:filter-condition
	table:field-number %nonNegativeInteger; #REQUIRED
	table:case-sensitive %boolean; "false"
	table:data-type (text | number) "text"
	table:value CDATA #REQUIRED
	table:operator CDATA #REQUIRED
>
<!ELEMENT table:database-ranges (table:database-range)*>
<!ELEMENT table:database-range ((table:database-source-sql | table:database-source-table | table:database-source-query)?, table:filter?, table:sort?, table:subtotal-rules?)>
<!ATTLIST table:database-range
	table:name CDATA #IMPLIED
	table:is-selection %boolean; "false"
	table:on-update-keep-styles %boolean; "false"
	table:on-update-keep-size %boolean; "true"
	table:has-persistant-data %boolean; "true"
	table:orientation (row | column) "row"
	table:contains-header %boolean; "true"
	table:display-filter-buttons %boolean; "false"
	table:target-range-address %cell-range-address; #REQUIRED
>
<!ELEMENT table:database-source-sql EMPTY>
<!ATTLIST table:database-source-sql
	table:database-name CDATA #REQUIRED
	table:sql-statement CDATA #REQUIRED
	table:parse-sql-statements %boolean; "false"
>
<!ELEMENT table:database-source-table EMPTY>
<!ATTLIST table:database-source-table
	table:database-name CDATA #REQUIRED
	table:table-name CDATA #REQUIRED
>
<!ELEMENT table:database-source-query EMPTY>
<!ATTLIST table:database-source-query
	table:database-name CDATA #REQUIRED
	table:query-name CDATA #REQUIRED
>
<!ELEMENT table:sort (table:sort-by)+>
<!ATTLIST table:sort
	table:bind-styles-to-content %boolean; "true"
	table:target-range-address %cell-range-address; #IMPLIED
	table:case-sensitive %boolean; "false"
>
<!ELEMENT table:sort-by EMPTY>
<!ATTLIST table:sort-by
	table:field-number %nonNegativeInteger; #REQUIRED
	table:data-type (text | number | automatic | qname-but-not-ncname) "automatic"
	table:order (ascending | descending) "ascending"
>
<!ELEMENT table:subtotal-rules (table:sort-groups? | table:subtotal-rule*)?>
<!ATTLIST table:subtotal-rules
	table:bind-styles-to-content %boolean; "true"
	table:case-sensitive %boolean; "false"
	table:page-breaks-on-group-change %boolean; "false"
>
<!ELEMENT table:sort-groups EMPTY>
<!ATTLIST table:sort-groups
	table:data-type (text | number | automatic | qname-but-not-ncname) "automatic"
	table:order (ascending | descending) "ascending"
>
<!ELEMENT table:subtotal-rule (table:subtotal-field)*>
<!ATTLIST table:subtotal-rule
	table:group-by-field-number %nonNegativeInteger; #REQUIRED
>
<!ELEMENT table:subtotal-field EMPTY>
<!ATTLIST table:subtotal-field
	table:field-number %nonNegativeInteger; #REQUIRED
	table:function CDATA #REQUIRED
>
<!ELEMENT table:data-pilot-tables (table:data-pilot-table)*>
<!ELEMENT table:data-pilot-table ((table:database-source-sql | table:database-source-table | table:database-source-query | table:source-service | table:source-cell-range)?, table:data-pilot-field+)>
<!ATTLIST table:data-pilot-table
	table:name CDATA #REQUIRED
	table:application-data CDATA #IMPLIED
	table:grand-total (none | row | column | both) "both"
	table:ignore-empty-rows %boolean; "false"
	table:identify-categories %boolean; "false"
	table:target-range-address %cell-range-address; #REQUIRED
	table:buttons %cell-range-address-list; #REQUIRED
>
<!ELEMENT table:source-service EMPTY>
<!ATTLIST table:source-service
	table:name CDATA #REQUIRED
	table:source-name CDATA #REQUIRED
	table:object-name CDATA #REQUIRED
	table:username CDATA #IMPLIED
	table:password CDATA #IMPLIED
>
<!ELEMENT table:source-cell-range (table:filter)?>
<!ATTLIST table:source-cell-range
	table:cell-range-address %cell-range-address; #REQUIRED
>
<!ELEMENT table:data-pilot-field (table:data-pilot-level)?>
<!ATTLIST table:data-pilot-field
	table:source-field-name CDATA #REQUIRED
	table:is-data-layout-field %boolean; "false"
	table:function CDATA #REQUIRED
	table:orientation (row | column | data | page | hidden) #REQUIRED
	table:used-hierarchy %positiveInteger; "1"
>
<!ELEMENT table:data-pilot-level (table:data-pilot-subtotals?, table:data-pilot-members?)>
<!ATTLIST table:data-pilot-level
	table:display-empty %boolean; #IMPLIED
>
<!ELEMENT table:data-pilot-subtotals (table:data-pilot-subtotal)*>
<!ELEMENT table:data-pilot-subtotal EMPTY>
<!ATTLIST table:data-pilot-subtotal
	table:function CDATA #REQUIRED
>
<!ELEMENT table:data-pilot-members (table:data-pilot-member)*>
<!ELEMENT table:data-pilot-member EMPTY>
<!ATTLIST table:data-pilot-member
	table:name CDATA #REQUIRED
	table:display %boolean; #IMPLIED
	table:display-details %boolean; #IMPLIED
>

