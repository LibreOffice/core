<!--
	$Id: text.mod,v 1.1 2000-09-29 16:16:04 mh Exp $

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

<!ENTITY % fields "text:date |
				   text:time |
				   text:page-number |
				   text:page-continuation |
				   text:sender-firstname |
				   text:sender-firstname |
				   text:sender-lastname |
				   text:sender-initials | 
				   text:sender-title |
				   text:sender-position |
				   text:sender-email | 
				   text:sender-phone-private |
				   text:sender-fax | 
				   text:sender-company |
				   text:sender-phone-work |
				   text:sender-street |
				   text:sender-city |
				   text:sender-postal-code |
				   text:sender-country |
				   text:sender-state-or-province |
				   text:author-name |
				   text:author-initials |
				   text:placeholder |
				   text:variable-set | 
				   text:variable-get | 
				   text:variable-input | 
				   text:user-field-get | 
				   text:user-field-input | 
				   text:sequence | 
				   text:expression | 
				   text:text-input |
				   text:database-display |
				   text:database-next |
				   text:database-row-select |
				   text:database-row-number |
				   text:database-name |
				   text:initial-creator |
				   text:creation-date |
				   text:creation-time |
				   text:description |
				   text:user-defined |
				   text:print-time |
				   text:print-date |
				   text:printed-by |
				   text:title |
				   text:subject |
				   text:keywords |
				   text:editing-cycles |
				   text:editing-duration |
				   text:modification-time |
				   text:modification-date |
				   text:creator |
				   text:conditional-text |
				   text:hidden-text |
				   text:hide-paragraph |
				   text:chapter |
				   text:file-name |
				   text:template-name |
				   text:set-page-variable |
				   text:get-page-variable |
				   text:execute-macro |
				   text:dde-connection |
				   text:reference-ref |
				   text:sequence-ref |
				   text:bookmark-ref |
				   text:footnote-ref |
				   text:endnote-ref">


<!ENTITY % inline-text "(#PCDATA|
						 text:span|text:tab-stop|text:s|text:line-break|
						 text:footnote|text:endnote|
						 text:bookmark|text:bookmark-start|text:bookmark-end|
						 text:reference-mark|text:reference-mark-start|
						 text:reference-mark-end|%fields;|
						 draw:a|draw:text-box|draw:image)*">

<!ELEMENT text:p %inline-text;>
<!ELEMENT text:h %inline-text;>

<!ATTLIST text:p text:style-name %styleName; #IMPLIED>
<!ATTLIST text:p text:cond-style-name %styleName; #IMPLIED>

<!ATTLIST text:h text:style-name %styleName; #IMPLIED>
<!ATTLIST text:h text:cond-style-name %styleName; #IMPLIED>
<!ATTLIST text:h text:level %positiveInteger; "1">

<!ELEMENT text:span %inline-text;>
<!ATTLIST text:span text:style-name %styleName; #REQUIRED>

<!ELEMENT text:s EMPTY>
<!ATTLIST text:s text:c %positiveInteger; "1">

<!ELEMENT text:tab-stop EMPTY>

<!ELEMENT text:line-break EMPTY>


<!ENTITY % list-items "((text:list-header,text:list-item*)|text:list-item+)">
<!ELEMENT text:ordered-list %list-items;>
<!ELEMENT text:unordered-list %list-items;>


<!ATTLIST text:ordered-list text:style-name %styleName; #IMPLIED>
<!ATTLIST text:unordered-list text:style-name %styleName; #IMPLIED>

<!ATTLIST text:ordered-list text:continue-numbering %boolean; "false">

<!ELEMENT text:list-header (text:p)+>
<!ELEMENT text:list-item (text:p|text:ordered-list|text:unordered-list)+>

<!ATTLIST text:list-item text:restart-numbering %boolean; "false">
<!ATTLIST text:list-item text:start-value %positiveInteger; #IMPLIED>

<!ELEMENT text:list-style (text:list-level-style-number|
     					   text:list-level-style-bullet|
     					   text:list-level-style-image)+>

<!ATTLIST text:list-style style:name %styleName; #REQUIRED>

<!ATTLIST text:list-style text:consecutive-numbering %boolean; "false">


<!ELEMENT text:list-level-style-number (style:properties?)>

<!ATTLIST text:list-level-style-number text:level %positiveInteger;
									     		 #REQUIRED>
<!ATTLIST text:list-level-style-number text:style-name %styleName; #IMPLIED>

<!ATTLIST text:list-level-style-number style:num-format %string; #REQUIRED>
<!ATTLIST text:list-level-style-number style:num-prefix %string; #IMPLIED>
<!ATTLIST text:list-level-style-number style:num-suffix %string; #IMPLIED>
<!ATTLIST text:list-level-style-number style:num-letter-sync %boolean;
									     					"false">
<!ATTLIST text:list-level-style-number text:display-levels %positiveInteger;
									     				  "1">
<!ATTLIST text:list-level-style-number text:start-value %positiveInteger;
														   "1">


<!ELEMENT text:list-level-style-bullet (style:properties?)>

<!ATTLIST text:list-level-style-bullet text:level %positiveInteger; #REQUIRED>
<!ATTLIST text:list-level-style-bullet text:style-name %styleName; #IMPLIED>
<!ATTLIST text:list-level-style-bullet text:bullet-char %character; #REQUIRED>
<!ATTLIST text:list-level-style-bullet style:num-prefix %string; #IMPLIED>
<!ATTLIST text:list-level-style-bullet style:num-suffix %string; #IMPLIED>


<!ELEMENT text:list-level-style-image (style:properties?)>

<!ATTLIST text:list-level-style-image text:level %positiveInteger; #REQUIRED>
<!ATTLIST text:list-level-style-image xlink:type (simple) #FIXED "simple">
<!ATTLIST text:list-level-style-image xlink:href %uriReference; #REQUIRED>
<!ATTLIST text:list-level-style-image xlink:actuate (onLoad) "onLoad">
<!ATTLIST text:list-level-style-image xlink:show (embed) "embed">


<!-- list properties -->
<!ATTLIST style:properties text:space-before %nonNegativeLength; #IMPLIED>
<!ATTLIST style:properties text:min-label-width %nonNegativeLength; #IMPLIED>
<!ATTLIST style:properties text:min-label-distance %nonNegativeLength; #IMPLIED>


<!ELEMENT text:outline-style (text:outline-level-style)+>

<!ELEMENT text:outline-level-style (style:properties?)>

<!ATTLIST text:outline-level-style text:level %positiveInteger;
													 #REQUIRED>
<!ATTLIST text:outline-level-style text:style-name %styleName; #IMPLIED>

<!ATTLIST text:outline-level-style style:num-format %string; #REQUIRED>
<!ATTLIST text:outline-level-style style:num-prefix %string; #IMPLIED>
<!ATTLIST text:outline-level-style style:num-suffix %string; #IMPLIED>
<!ATTLIST text:outline-level-style style:num-letter-sync %boolean;
																"false">
<!ATTLIST text:outline-level-style text:display-levels %positiveInteger;
															  "1">
<!ATTLIST text:outline-level-style text:start-value %positiveInteger;
														   "1">

<!ENTITY % field-declarations "text:variable-decls?, 
							   text:user-field-decls?, 
							   text:sequence-decls?">

<!ENTITY % variableName "CDATA">

<!ENTITY % formula "CDATA">

<!ENTITY % valueAttr "text:value-type %valueType; #REQUIRED">

<!ENTITY % valueAndTypeAttr "%valueAttr;
		 					 text:value %float; #IMPLIED
							 text:date-value %date; #IMPLIED
							 text:time-value %timeInstance; #IMPLIED
							 text:boolean-value %boolean; #IMPLIED
							 text:string-value %string; #IMPLIED
							 text:currency CDATA #IMPLIED" >

<!ENTITY % numFormat 'style:num-format CDATA #IMPLIED 
					   style:num-letter-sync %boolean; "false"'>


<!ELEMENT text:date (#PCDATA)>
<!ATTLIST text:date text:date-value %date; #IMPLIED>
<!ATTLIST text:date text:date-adjust %dateDuration; #IMPLIED>
<!ATTLIST text:date text:fixed %boolean; "false">
<!ATTLIST text:date style:data-style-name %styleName; #IMPLIED>

<!ELEMENT text:time (#PCDATA)>
<!ATTLIST text:time text:time-value %timeInstance; #IMPLIED>
<!ATTLIST text:time text:time-adjust %timeDuration; #IMPLIED>
<!ATTLIST text:time text:fixed %boolean; "false">
<!ATTLIST text:time style:data-style-name %styleName; #IMPLIED>

<!ELEMENT text:page-number (#PCDATA)>
<!ATTLIST text:page-number text:page-adjust %positiveInteger; #IMPLIED>
<!ATTLIST text:page-number text:select-page (previous|current|next) "current">
<!ATTLIST text:page-number %numFormat;>

<!ELEMENT text:page-continuation (#PCDATA)>
<!ATTLIST text:page-continuation text:select-page (previous|next) #REQUIRED>
<!ATTLIST text:page-continuation text:string-value %string; #IMPLIED>

<!ELEMENT text:sender-firstname (#PCDATA)>
<!ATTLIST text:sender-firstname text:fixed %boolean; "true">

<!ELEMENT text:sender-lastname (#PCDATA)>
<!ATTLIST text:sender-lastname text:fixed %boolean; "true">

<!ELEMENT text:sender-initials (#PCDATA)>
<!ATTLIST text:sender-initials text:fixed %boolean; "true">

<!ELEMENT text:sender-title (#PCDATA)>
<!ATTLIST text:sender-title text:fixed %boolean; "true">

<!ELEMENT text:sender-position (#PCDATA)>
<!ATTLIST text:sender-position text:fixed %boolean; "true">

<!ELEMENT text:sender-email (#PCDATA)>
<!ATTLIST text:sender-email text:fixed %boolean; "true">

<!ELEMENT text:sender-phone-private (#PCDATA)>
<!ATTLIST text:sender-phone-private text:fixed %boolean; "true">

<!ELEMENT text:sender-fax (#PCDATA)>
<!ATTLIST text:sender-fax text:fixed %boolean; "true">

<!ELEMENT text:sender-company (#PCDATA)>
<!ATTLIST text:sender-company text:fixed %boolean; "true">

<!ELEMENT text:sender-phone-work (#PCDATA)>
<!ATTLIST text:sender-phone-work text:fixed %boolean; "true">

<!ELEMENT text:sender-street (#PCDATA)>
<!ATTLIST text:sender-street text:fixed %boolean; "true">

<!ELEMENT text:sender-city (#PCDATA)>
<!ATTLIST text:sender-city text:fixed %boolean; "true">

<!ELEMENT text:sender-postal-code (#PCDATA)>
<!ATTLIST text:sender-postal-code text:fixed %boolean; "true">

<!ELEMENT text:sender-country (#PCDATA)>
<!ATTLIST text:sender-country text:fixed %boolean; "true">

<!ELEMENT text:sender-state-or-province (#PCDATA)>
<!ATTLIST text:sender-state-or-province text:fixed %boolean; "true">

<!ELEMENT text:author-name (#PCDATA)>
<!ATTLIST text:author-name text:fixed %boolean; "true">

<!ELEMENT text:author-initials (#PCDATA)>
<!ATTLIST text:author-initials text:fixed %boolean; "true">

<!ELEMENT text:placeholder (#PCDATA)>
<!ATTLIST text:placeholder text:placeholder-type (text|table|text-box|image|object) #REQUIRED>
<!ATTLIST text:placeholder text:description %string; #IMPLIED>

<!ELEMENT text:variable-decls (text:variable-decl)*>

<!ELEMENT text:variable-decl EMPTY>
<!ATTLIST text:variable-decl text:name %variableName; #REQUIRED>
<!ATTLIST text:variable-decl %valueAndTypeAttr;>

<!ELEMENT text:variable-set (#PCDATA)>
<!ATTLIST text:variable-set text:name %variableName; #REQUIRED>
<!ATTLIST text:variable-set text:formula %formula; #IMPLIED>
<!ATTLIST text:variable-set %valueAndTypeAttr;>
<!ATTLIST text:variable-set text:display (value|none) "value">
<!ATTLIST text:variable-set style:data-style-name %styleName; #IMPLIED>

<!ELEMENT text:variable-get (#PCDATA)>
<!ATTLIST text:variable-get text:name %variableName; #REQUIRED>
<!ATTLIST text:variable-get text:display (value|formula) "value">
<!ATTLIST text:variable-get style:data-style-name %styleName; #IMPLIED>

<!ELEMENT text:variable-input (#PCDATA)>
<!ATTLIST text:variable-input text:name %variableName; #REQUIRED>
<!ATTLIST text:variable-input text:description %string; #IMPLIED>
<!ATTLIST text:variable-input %valueAndTypeAttr;>
<!ATTLIST text:variable-input text:display (value|none) "value">
<!ATTLIST text:variable-input style:data-style-name %styleName; #IMPLIED>

<!ELEMENT text:user-field-decls (text:user-field-decl)*>

<!ELEMENT text:user-field-decl EMPTY>
<!ATTLIST text:user-field-decl text:name %variableName; #REQUIRED>
<!ATTLIST text:user-field-decl text:formula %formula; #IMPLIED>
<!ATTLIST text:user-field-decl %valueAndTypeAttr;>

<!ELEMENT text:user-field-get (#PCDATA)>
<!ATTLIST text:user-field-get text:name %variableName; #REQUIRED>
<!ATTLIST text:user-field-get text:display (value|formula|none) "value">
<!ATTLIST text:user-field-get style:data-style-name %styleName; #IMPLIED>

<!ELEMENT text:user-field-input (#PCDATA)>
<!ATTLIST text:user-field-input text:name %variableName; #REQUIRED>
<!ATTLIST text:user-field-input text:description %string; #IMPLIED>
<!ATTLIST text:user-field-input style:data-style-name %styleName; #IMPLIED>

<!ELEMENT text:sequence-decls (text:sequence-decl)*>

<!ELEMENT text:sequence-decl EMPTY>
<!ATTLIST text:sequence-decl text:name %variableName; #REQUIRED>
<!ATTLIST text:sequence-decl text:display-outline-level %positiveInteger; "0">
<!ATTLIST text:sequence-decl text:separation-character %character; ".">

<!ELEMENT text:sequence (#PCDATA)>
<!ATTLIST text:sequence text:name %variableName; #REQUIRED>
<!ATTLIST text:sequence text:formula %formula; #IMPLIED>
<!ATTLIST text:sequence %numFormat;>
<!ATTLIST text:sequence text:ref-name ID #IMPLIED>

<!ELEMENT text:expression (#PCDATA)>
<!ATTLIST text:expression text:formula %formula; #IMPLIED>
<!ATTLIST text:expression text:display (value|formula ) "value">
<!ATTLIST text:expression %valueAndTypeAttr;>
<!ATTLIST text:expression style:data-style-name %styleName; #IMPLIED>

<!ELEMENT text:text-input (#PCDATA)>
<!ATTLIST text:text-input text:description %string; #IMPLIED>

<!ENTITY % database-table "text:database-name CDATA #REQUIRED 
						   text:table-name CDATA #REQUIRED">

<!ELEMENT text:database-display (#PCDATA)>
<!ATTLIST text:database-display %database-table;>
<!ATTLIST text:database-display text:column-name %string; #REQUIRED>
<!ATTLIST text:database-display %valueAttr;>
<!ATTLIST text:database-display style:data-style-name %styleName; #IMPLIED>

<!ELEMENT text:database-next (#PCDATA)>
<!ATTLIST text:database-next %database-table;>
<!ATTLIST text:database-next text:condition %formula; #IMPLIED>

<!ELEMENT text:database-row-select (#PCDATA)>
<!ATTLIST text:database-row-select %database-table;>
<!ATTLIST text:database-row-select text:condition %formula; #IMPLIED>
<!ATTLIST text:database-row-select text:row-number %integer; #REQUIRED>

<!ELEMENT text:database-row-number (#PCDATA)>
<!ATTLIST text:database-row-number %database-table;>
<!ATTLIST text:database-row-number %numFormat;>
<!ATTLIST text:database-row-number text:value %integer; #IMPLIED>

<!ELEMENT text:database-name (#PCDATA)>
<!ATTLIST text:database-name %database-table;>

<!ELEMENT text:initial-creator (#PCDATA)>
<!ATTLIST text:initial-creator text:fixed %boolean; "false">

<!ELEMENT text:creation-date (#PCDATA)>
<!ATTLIST text:creation-date text:fixed %boolean; "false">
<!ATTLIST text:creation-date text:date-value %date; #IMPLIED>
<!ATTLIST text:creation-date style:data-style-name %styleName; #IMPLIED>

<!ELEMENT text:creation-time (#PCDATA)>
<!ATTLIST text:creation-time text:fixed %boolean; "false">
<!ATTLIST text:creation-time text:time-value %timeInstance; #IMPLIED>
<!ATTLIST text:creation-time style:data-style-name %styleName; #IMPLIED>

<!ELEMENT text:description (#PCDATA)>
<!ATTLIST text:description text:fixed %boolean; "false">

<!ELEMENT text:user-defined (#PCDATA)>
<!ATTLIST text:user-defined text:fixed %boolean; "false">
<!ATTLIST text:user-defined text:name %string; #REQUIRED>

<!ELEMENT text:print-time (#PCDATA)>
<!ATTLIST text:print-time text:fixed %boolean; "false">
<!ATTLIST text:print-time text:time-value %timeInstance; #IMPLIED>
<!ATTLIST text:print-time style:data-style-name %styleName; #IMPLIED>

<!ELEMENT text:print-date (#PCDATA)>
<!ATTLIST text:print-date text:fixed %boolean; "false">
<!ATTLIST text:print-date text:date-value %date; #IMPLIED>
<!ATTLIST text:print-date style:data-style-name %styleName; #IMPLIED>

<!ELEMENT text:printed-by (#PCDATA)>
<!ATTLIST text:printed-by text:fixed %boolean; "false">

<!ELEMENT text:title (#PCDATA)>
<!ATTLIST text:title text:fixed %boolean; "false">

<!ELEMENT text:subject (#PCDATA)>
<!ATTLIST text:subject text:fixed %boolean; "false">

<!ELEMENT text:keywords (#PCDATA)>
<!ATTLIST text:keywords text:fixed %boolean; "false">

<!ELEMENT text:editing-cycles (#PCDATA)>
<!ATTLIST text:editing-cycles text:fixed %boolean; "false">

<!ELEMENT text:editing-duration (#PCDATA)>
<!ATTLIST text:editing-duration text:fixed %boolean; "false">
<!ATTLIST text:editing-duration text:duration %timeDuration; #IMPLIED>
<!ATTLIST text:editing-duration style:data-style-name %styleName; #IMPLIED>

<!ELEMENT text:modification-time (#PCDATA)>
<!ATTLIST text:modification-time text:fixed %boolean; "false">
<!ATTLIST text:modification-time text:time-value %timeInstance; #IMPLIED>
<!ATTLIST text:modification-time style:data-style-name %styleName; #IMPLIED>

<!ELEMENT text:modification-date (#PCDATA)>
<!ATTLIST text:modification-date text:fixed %boolean; "false">
<!ATTLIST text:modification-date text:date-value %date; #IMPLIED>
<!ATTLIST text:modification-date style:data-style-name %styleName; #IMPLIED>

<!ELEMENT text:creator (#PCDATA)>
<!ATTLIST text:creator text:fixed %boolean; "false">

<!ELEMENT text:conditional-text (#PCDATA)>
<!ATTLIST text:conditional-text text:condition %formula; #REQUIRED>
<!ATTLIST text:conditional-text text:string-value-if-false %string; #REQUIRED>

<!ELEMENT text:hidden-text (#PCDATA)>
<!ATTLIST text:hidden-text text:condition %formula; #REQUIRED>
<!ATTLIST text:hidden-text text:string-value %string; #REQUIRED>

<!ELEMENT text:hide-paragraph EMPTY>
<!ATTLIST text:hide-paragraph text:condition %formula; #REQUIRED>

<!ELEMENT text:chapter (#PCDATA)>
<!ATTLIST text:chapter text:display (name|number|number-and-name|
									 plain-number-and-name|plain-number) 
									 "number-and-name">
<!ATTLIST text:chapter text:outline-level %integer; "1">

<!ELEMENT text:file-name (#PCDATA)>
<!ATTLIST text:file-name text:display (full|path|name|name-and-extension) 	
									  "full">
<!ATTLIST text:file-name text:fixed %boolean; "false">

<!ELEMENT text:template-name (#PCDATA)>
<!ATTLIST text:template-name text:display (full|path|name|name-and-extension|
										  area|title) "full">

<!ELEMENT text:set-page-variable EMPTY>
<!ATTLIST text:set-page-variable text:active %boolean; "true">
<!ATTLIST text:set-page-variable text:page-adjust %integer; "0">

<!ELEMENT text:get-page-variable (#PCDATA)>
<!ATTLIST text:get-page-variable %numFormat;>

<!ELEMENT text:execute-macro (#PCDATA)>
<!ATTLIST text:execute-macro text:name %string; #REQUIRED>

<!ELEMENT text:dde-connection-decls (text:dde-connection-decl)*>

<!ELEMENT text:dde-connection-decl EMPTY>
<!ATTLIST text:dde-connection-decl text:name %string; #REQUIRED>
<!ATTLIST text:dde-connection-decl text:dde-target-name %string; #REQUIRED>
<!ATTLIST text:dde-connection-decl text:dde-file-name %string; #REQUIRED>
<!ATTLIST text:dde-connection-decl text:dde-command %string; #REQUIRED>
<!ATTLIST text:dde-connection-decl text:automatic-update %boolean; "false">

<!ELEMENT text:dde-connection (#PCDATA)>
<!ATTLIST text:dde-connection text:connection-name %string; #REQUIRED>

<!ELEMENT text:reference-ref (#PCDATA)>
<!ATTLIST text:reference-ref text:ref-name %string; #REQUIRED>
<!ATTLIST text:reference-ref text:reference-format (page|chapter|text|direction) #IMPLIED>

<!ELEMENT text:sequence-ref (#PCDATA)>
<!ATTLIST text:sequence-ref text:ref-name %string; #REQUIRED>
<!ATTLIST text:sequence-ref text:reference-format (page|chapter|text|direction|category-and-value|caption|value) #IMPLIED>

<!ELEMENT text:bookmark-ref (#PCDATA)>
<!ATTLIST text:bookmark-ref text:ref-name %string; #REQUIRED>
<!ATTLIST text:bookmark-ref text:reference-format (page|chapter|text|direction) #IMPLIED>

<!ELEMENT text:footnote-ref (#PCDATA)>
<!ATTLIST text:footnote-ref text:ref-name %string; #REQUIRED>
<!ATTLIST text:footnote-ref text:reference-format (page|chapter|text|direction) #IMPLIED>

<!ELEMENT text:endnote-ref (#PCDATA)>
<!ATTLIST text:endnote-ref text:ref-name %string; #REQUIRED>
<!ATTLIST text:endnote-ref text:reference-format (page|chapter|text|direction) #IMPLIED>

<!ELEMENT text:bookmark EMPTY>
<!ATTLIST text:bookmark text:name CDATA #REQUIRED>

<!ELEMENT text:bookmark-start EMPTY>
<!ATTLIST text:bookmark-start text:name CDATA #REQUIRED>

<!ELEMENT text:bookmark-end EMPTY>
<!ATTLIST text:bookmark-end text:name CDATA #REQUIRED>

<!ELEMENT text:reference-mark EMPTY>
<!ATTLIST text:reference-mark text:name CDATA #REQUIRED>

<!ELEMENT text:reference-mark-start EMPTY>
<!ATTLIST text:reference-mark-start text:name CDATA #REQUIRED>

<!ELEMENT text:reference-mark-end EMPTY>
<!ATTLIST text:reference-mark-end text:name CDATA #REQUIRED>

<!ELEMENT text:footnotes-configuration (text:footnote-continuation-notice-forward?,text:footnote-continuation-notice-backward?)>
<!ATTLIST text:footnotes-configuration text:num-prefix %string; #IMPLIED>
<!ATTLIST text:footnotes-configuration text:num-suffix %string; #IMPLIED>
<!ATTLIST text:footnotes-configuration style:num-format %string; #IMPLIED>
<!ATTLIST text:footnotes-configuration text:citation-body-style-name %styleName; #IMPLIED>
<!ATTLIST text:footnotes-configuration text:citation-style-name %styleName; #IMPLIED>
<!ATTLIST text:footnotes-configuration text:default-style-name  %styleName; #IMPLIED>
<!ATTLIST text:footnotes-configuration text:page-master-name %styleName; #IMPLIED>
<!ATTLIST text:footnotes-configuration text:offset %integer; #IMPLIED>
<!ATTLIST text:footnotes-configuration text:footnotes-position (document|page) "page">
<!ATTLIST text:footnotes-configuration text:start-numbering-at (document|chapter|page) "document">

<!ELEMENT text:footnote-continuation-notice-forward (#PCDATA)> 
<!ELEMENT text:footnote-continuation-notice-backward (#PCDATA)>

<!ELEMENT text:endnotes-configuration EMPTY>
<!ATTLIST text:endnotes-configuration text:num-prefix %string; #IMPLIED>
<!ATTLIST text:endnotes-configuration text:num-suffix %string; #IMPLIED>
<!ATTLIST text:endnotes-configuration style:num-format %string; #IMPLIED>
<!ATTLIST text:endnotes-configuration text:citation-style-name %styleName; #IMPLIED>
<!ATTLIST text:endnotes-configuration text:citation-body-style-name %styleName; #IMPLIED>
<!ATTLIST text:endnotes-configuration text:default-style-name %styleName; #IMPLIED>
<!ATTLIST text:endnotes-configuration text:page-master-name %styleName; #IMPLIED>
<!ATTLIST text:endnotes-configuration text:offset %integer; #IMPLIED>

<!-- Validity constraint: text:footnote and text:endnote elements may not 
	contain other text:footnote or text:endnote elements, even though the DTD
	allows this (via the %text; in the foot-/endnote-body).
	Unfortunatetly, this constraint cannot be easily specified in the DTD.
-->
<!ELEMENT text:footnote (text:footnote-citation, text:footnote-body)>
<!ATTLIST text:footnote text:id ID #IMPLIED>

<!ELEMENT text:footnote-citation (#PCDATA)>
<!ATTLIST text:footnote-citation text:label %string; #IMPLIED>

<!ELEMENT text:footnote-body (text:h|text:p|
							  text:ordered-list|text:unordered-list)*>

<!ELEMENT text:endnote (text:endnote-citation, text:endnote-body)>
<!ATTLIST text:endnote text:id ID #IMPLIED>

<!ELEMENT text:endnote-citation (#PCDATA)>
<!ATTLIST text:endnote-citation text:label %string; #IMPLIED>

<!ELEMENT text:endnote-body (text:h|text:p|
							 text:ordered-list|text:unordered-list)*>
