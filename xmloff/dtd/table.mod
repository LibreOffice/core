<!-- edited with XML Spy v3.5 NT (http://www.xmlspy.com) by Sascha Ballach (Star Office GmbH) -->
<!--
	$Id: table.mod,v 1.25 2001-03-01 15:17:04 sab Exp $

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
   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
   See the License for the specific provisions governing your rights and
   obligations concerning the Software.

   The Initial Developer of the Original Code is: Sun Microsystems, Inc.

   Copyright: 2000 by Sun Microsystems, Inc.

   All Rights Reserved.

   Contributor(s): _______________________________________

-->
<!ELEMENT table:tracked-changes (table:cell-content-change | table:insertion | table:deletion | table:movement | table:rejection)+>
<!ELEMENT table:dependences (table:dependence)+>
<!ELEMENT table:dependence EMPTY>
<!ATTLIST table:dependence
	table:id CDATA #REQUIRED
>
<!ELEMENT table:deletions (table:cell-content-deletion | table:change-deletion)+>
<!ELEMENT table:cell-content-deletion (table:cell-address?, table:table-cell?)>
<!ATTLIST table:cell-content-deletion
	table:id CDATA #IMPLIED
>
<!ELEMENT table:change-deletion EMPTY>
<!ATTLIST table:change-deletion
	table:id CDATA #IMPLIED
>
<!ELEMENT table:insertion (office:change-info, table:dependences?, table:deletions?)>
<!ATTLIST table:insertion
	table:id CDATA #REQUIRED
	table:acceptance-state (accepted | rejected | pending) "pending"
	table:rejecting-change-id %positiveInteger; #IMPLIED
	table:type (row | column | table) #REQUIRED
	table:position %integer; #REQUIRED
	table:count %positiveInteger; "1"
	table:table %integer; #IMPLIED
>
<!ELEMENT table:deletion (office:change-info, table:dependences?, table:deletions?, table:cut-offs?)>
<!ATTLIST table:deletion
	table:id CDATA #REQUIRED
	table:acceptance-state (accepted | rejected | pending) "pending"
	table:rejecting-change-id %positiveInteger; #IMPLIED
	table:type (row | column | table) #REQUIRED
	table:position %integer; #REQUIRED
	table:count %positiveInteger; "1"
	table:table %integer; #IMPLIED
	table:multi-spanned-deletion %integer; #IMPLIED
>
<!ELEMENT table:cut-offs (table:insertion-cut-off | table:movement-cut-off+ | (table:insertion-cut-off, table:movement-cut-off+))>
<!ELEMENT table:insertion-cut-off EMPTY>
<!ATTLIST table:insertion-cut-off
	table:id CDATA #REQUIRED
	table:position %integer; #REQUIRED
>
<!ELEMENT table:movement-cut-off EMPTY>
<!ATTLIST table:movement-cut-off
	table:id CDATA #REQUIRED
	table:start-position %integer; #IMPLIED
	table:end-position %integer; #IMPLIED
	table:position %integer; #IMPLIED
>
<!ELEMENT table:movement (table:source-range-address, table:target-range-address, office:change-info, table:dependences?, table:deletions?)>
<!ATTLIST table:movement
	table:id CDATA #REQUIRED
	table:acceptance-state (accepted | rejected | pending) "pending"
	table:rejecting-change-id %positiveInteger; #IMPLIED
>
<!ELEMENT table:target-range-address EMPTY>
<!ATTLIST table:target-range-address
	table:column %integer; #IMPLIED
	table:row %integer; #IMPLIED
	table:table %integer; #IMPLIED
	table:start-column %integer; #IMPLIED
	table:start-row %integer; #IMPLIED
	table:start-table %integer; #IMPLIED
	table:end-column %integer; #IMPLIED
	table:end-row %integer; #IMPLIED
	table:end-table %integer; #IMPLIED
>
<!ELEMENT table:source-range-address EMPTY>
<!ATTLIST table:source-range-address
	table:column %integer; #IMPLIED
	table:row %integer; #IMPLIED
	table:table %integer; #IMPLIED
	table:start-column %integer; #IMPLIED
	table:start-row %integer; #IMPLIED
	table:start-table %integer; #IMPLIED
	table:end-column %integer; #IMPLIED
	table:end-row %integer; #IMPLIED
	table:end-table %integer; #IMPLIED
>
<!ELEMENT table:cell-content-change (table:cell-address, office:change-info, table:dependences?, table:deletions?, table:previous)>
<!ATTLIST table:cell-content-change
	table:id CDATA #REQUIRED
	table:acceptance-state (accepted | rejected | pending) "pending"
	table:rejecting-change-id %positiveInteger; #IMPLIED
>
<!ELEMENT table:cell-address EMPTY>
<!ATTLIST table:cell-address
	table:column %integer; #IMPLIED
	table:row %integer; #IMPLIED
	table:table %integer; #IMPLIED
>
<!ELEMENT table:previous (table:table-cell)>
<!ATTLIST table:previous
	table:id CDATA #IMPLIED
>
<!ELEMENT table:rejection (office:change-info, table:dependences?, table:deletions?)>
<!ATTLIST table:rejection
	table:id CDATA #REQUIRED
	table:acceptance-state (accepted | rejected | pending) "pending"
	table:rejecting-change-id %positiveInteger; #IMPLIED
>
<!ENTITY % table-columns "( table:table-columns | ( table:table-column | table:table-column-group )+ )">
<!ENTITY % table-header-columns "table:table-header-columns">
<!ENTITY % table-rows "( table:table-rows | ( table:table-row | table:table-row-group )+ )">
<!ENTITY % table-header-rows "table:table-header-rows">
<!ENTITY % table-column-groups "( (%table-columns;,(%table-header-columns;,%table-columns;?)?) | (%table-header-columns;,%table-columns;?) )">
<!ENTITY % table-row-groups "( (%table-rows;,(%table-header-rows;,%table-rows;?)?) | (%table-header-rows;,%table-rows;?) )">
<!ELEMENT table:table (table:table-source?, table:scenario?, office:forms?, table:shapes?, %table-column-groups;, %table-row-groups;)>
<!ATTLIST table:table
	table:name %string; #REQUIRED
	table:style-name %styleName; #IMPLIED
	table:protected %boolean; "false"
	table:protection-key CDATA #IMPLIED
	table:print-ranges %cell-range-address-list; #IMPLIED
>
<!ELEMENT table:table-source EMPTY>
<!ATTLIST table:table-source
	table:mode (copy-all | copy-results-only) "copy-all"
	xlink:type (simple) #FIXED "simple"
	xlink:actuate (onRequest) "onRequest"
	xlink:href %uriReference; #REQUIRED
	table:filter-name CDATA #IMPLIED
	table:table-name CDATA #REQUIRED
	table:filter-options CDATA #IMPLIED
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
<!ELEMENT table:shapes %shapes;>
<!ELEMENT table:table-column-group (table:table-header-columns | table:table-column | table:table-column-group)+>
<!ATTLIST table:table-column-group
	table:display %boolean; "true"
>
<!ELEMENT table:table-header-columns (table:table-column | table:table-column-group)+>
<!ELEMENT table:table-columns (table:table-column | table:table-column-group)+>
<!ELEMENT table:table-column EMPTY>
<!ATTLIST table:table-column
	table:number-columns-repeated %positiveInteger; "1"
	table:style-name %styleName; #IMPLIED
	table:visibility (visible | collapse | filter) "visible"
>
<!ELEMENT table:table-row-group (table:table-header-rows | table:table-row | table:table-row-group)+>
<!ATTLIST table:table-row-group
	table:display %boolean; "true"
>
<!ELEMENT table:table-header-rows (table:table-row | table:table-row-group)+>
<!ELEMENT table:table-rows (table:table-row | table:table-row-group)+>
<!ENTITY % table-cells "(table:table-cell|table:covered-table-cell)+">
<!ELEMENT table:table-row %table-cells;>
<!ATTLIST table:table-row
	table:number-rows-repeated %positiveInteger; "1"
	table:style-name %styleName; #IMPLIED
	table:visibility (visible | collapse | filter) "visible"
>
<!ELEMENT table:subtable (%table-column-groups;, %table-row-groups;)>
<!ENTITY % text-wo-table "(text:h|text:p|text:ordered-list|text:unordered-list|%shapes;|chart:chart)*">
<!ENTITY % cell-content "(table:cell-range-source?,office:annotation?,table:detective?,(table:subtable|%text-wo-table;))">
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
<!ELEMENT table:calculation-settings (table:null-date?, table:iteration?)>
<!ATTLIST table:calculation-settings
	table:case-sensitive %boolean; "true"
	table:precision-as-shown %boolean; "false"
	table:search-criteria-must-apply-to-whole-cell %boolean; "true"
	table:automatic-find-labels %boolean; "true"
	table:null-year %positiveInteger; "1930"
>
<!ELEMENT table:null-date EMPTY>
<!ATTLIST table:null-date
	table:value-type %valueType; #FIXED "date"
	table:date-value %date; "1899-12-30"
>
<!ELEMENT table:iteration EMPTY>
<!ATTLIST table:iteration
	table:status (enable | disable) "disable"
	table:steps %positiveInteger; "100"
	table:maximum-difference %float; "0.001"
>
<!ELEMENT table:content-validations (table:content-validation)+>
<!ELEMENT table:content-validation (table:help-message?, (table:error-message | table:error-macro)?)>
<!ATTLIST table:content-validation
	table:name CDATA #REQUIRED
	table:condition CDATA #IMPLIED
	table:base-cell-address %cell-address; #IMPLIED
	table:allow-empty-cell %boolean; #IMPLIED
>
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
<!ELEMENT table:label-ranges (table:label-range)*>
<!ELEMENT table:label-range EMPTY>
<!ATTLIST table:label-range
	table:label-cell-range-address %cell-range-address; #REQUIRED
	table:data-cell-range-address %cell-range-address; #REQUIRED
	table:orientation (column | row) #REQUIRED
>
<!ELEMENT table:cell-range-source EMPTY>
<!ATTLIST table:cell-range-source
	table:name %string; #REQUIRED
	xlink:type (simple) #FIXED "simple"
	xlink:actuate (onRequest) #FIXED "onRequest"
	xlink:href %uriReference; #REQUIRED
	table:filter-name %string; #REQUIRED
	table:filter-options %string; #IMPLIED
	table:last-column-spanned %positiveInteger; #REQUIRED
	table:last-row-spanned %positiveInteger; #REQUIRED
>
<!ELEMENT table:detective (table:highlighted-range*, table:operation*)>
<!ELEMENT table:highlighted-range EMPTY>
<!ATTLIST table:highlighted-range
	table:cell-range-address %cell-range-address; #IMPLIED
	table:direction (from-another-table | to-another-table | from-same-table | to-same-table) #REQUIRED
	table:contains-error %boolean; "false"
>
<!ELEMENT table:operation EMPTY>
<!ATTLIST table:operation
	table:name (trace-dependents | remove-dependents | trace-precedents | remove-precedents | trace-errors) #REQUIRED
	table:index %nonNegativeInteger; #REQUIRED
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
<!ELEMENT table:consolidation EMPTY>
<!ATTLIST table:consolidation
	table:function CDATA #REQUIRED
	table:source-cell-range-addresses %cell-range-address-list; #REQUIRED
	table:target-cell-address %cell-address; #REQUIRED
	table:use-label (none | column | row | both) "none"
	table:link-to-source-data %boolean; "false"
>
<!ELEMENT table:dde-links (table:dde-link)+>
<!ELEMENT table:dde-link (office:dde-source, table:table)>
