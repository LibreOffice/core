<!--

  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
  
  Copyright 2000, 2010 Oracle and/or its affiliates.
 
  OpenOffice.org - a multi-platform office productivity suite
 
  This file is part of OpenOffice.org.
 
  OpenOffice.org is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License version 3
  only, as published by the Free Software Foundation.
 
  OpenOffice.org is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License version 3 for more details
  (a copy is included in the LICENSE file that accompanied this code).
 
  You should have received a copy of the GNU Lesser General Public License
  version 3 along with OpenOffice.org.  If not, see
  <http://www.openoffice.org/license.html>
  for a copy of the LGPLv3 License.

-->

<!ELEMENT office:document ( office:meta?,
							office:settings?,
							office:script?,
							office:font-decls?,
							office:styles?,
							office:automatic-styles?,
							office:master-styles?,
							office:body ) >

<!ATTLIST office:document xmlns:office	CDATA #FIXED "&nOffice;">
<!ATTLIST office:document xmlns:meta	CDATA #FIXED "&nMeta;">
<!ATTLIST office:document xmlns:script	CDATA #FIXED "&nScript;">
<!ATTLIST office:document xmlns:style	CDATA #FIXED "&nStyle;">
<!ATTLIST office:document xmlns:text	CDATA #FIXED "&nText;">
<!ATTLIST office:document xmlns:table	CDATA #FIXED "&nTable;">
<!ATTLIST office:document xmlns:draw	CDATA #FIXED "&nDraw;">
<!ATTLIST office:document xmlns:chart	CDATA #FIXED "&nChart;">
<!ATTLIST office:document xmlns:number	CDATA #FIXED "&nNumber;">
<!ATTLIST office:document xmlns:fo		CDATA #FIXED "&nFO;">
<!ATTLIST office:document xmlns:xlink	CDATA #FIXED "&nXLink;">
<!ATTLIST office:document xmlns:svg		CDATA #FIXED "&nSVG;">
<!ATTLIST office:document xmlns:dc		CDATA #FIXED "&nDC;">

<!ATTLIST office:document office:class
						  (text|text-global|
						   drawing|presentation|
						   spreadsheet|chart) #REQUIRED>

<!ATTLIST office:document office:version	%string; #IMPLIED>

<!-- document-styles -->
<!ELEMENT office:document-styles (
							office:font-decls?,
							office:styles?,
							office:automatic-styles?,
							office:master-styles? ) >

<!ATTLIST office:document-styles xmlns:office	CDATA #FIXED "&nOffice;">
<!ATTLIST office:document-styles xmlns:meta	CDATA #FIXED "&nMeta;">
<!ATTLIST office:document-styles xmlns:script	CDATA #FIXED "&nScript;">
<!ATTLIST office:document-styles xmlns:style	CDATA #FIXED "&nStyle;">
<!ATTLIST office:document-styles xmlns:text	CDATA #FIXED "&nText;">
<!ATTLIST office:document-styles xmlns:table	CDATA #FIXED "&nTable;">
<!ATTLIST office:document-styles xmlns:draw	CDATA #FIXED "&nDraw;">
<!ATTLIST office:document-styles xmlns:chart	CDATA #FIXED "&nChart;">
<!ATTLIST office:document-styles xmlns:number	CDATA #FIXED "&nNumber;">
<!ATTLIST office:document-styles xmlns:fo		CDATA #FIXED "&nFO;">
<!ATTLIST office:document-styles xmlns:xlink	CDATA #FIXED "&nXLink;">
<!ATTLIST office:document-styles xmlns:svg		CDATA #FIXED "&nSVG;">
<!ATTLIST office:document-styles xmlns:dc		CDATA #FIXED "&nDC;">

<!ATTLIST office:document-styles office:version	%string; #IMPLIED>

<!-- document-content -->

<!ELEMENT office:document-content (
							office:script?,
							office:font-decls?,
							office:automatic-styles?,
							office:body ) >

<!ATTLIST office:document-content xmlns:office	CDATA #FIXED "&nOffice;">
<!ATTLIST office:document-content xmlns:meta	CDATA #FIXED "&nMeta;">
<!ATTLIST office:document-content xmlns:script	CDATA #FIXED "&nScript;">
<!ATTLIST office:document-content xmlns:style	CDATA #FIXED "&nStyle;">
<!ATTLIST office:document-content xmlns:text	CDATA #FIXED "&nText;">
<!ATTLIST office:document-content xmlns:table	CDATA #FIXED "&nTable;">
<!ATTLIST office:document-content xmlns:draw	CDATA #FIXED "&nDraw;">
<!ATTLIST office:document-content xmlns:chart	CDATA #FIXED "&nChart;">
<!ATTLIST office:document-content xmlns:number	CDATA #FIXED "&nNumber;">
<!ATTLIST office:document-content xmlns:fo		CDATA #FIXED "&nFO;">
<!ATTLIST office:document-content xmlns:xlink	CDATA #FIXED "&nXLink;">
<!ATTLIST office:document-content xmlns:svg		CDATA #FIXED "&nSVG;">
<!ATTLIST office:document-content xmlns:dc		CDATA #FIXED "&nDC;">

<!ATTLIST office:document-content office:class
						  (text|text-global|
						   drawing|presentation|
						   spreadsheet|chart) #REQUIRED>

<!ATTLIST office:document-content office:version	%string; #IMPLIED>

<!-- document-content -->

<!ELEMENT office:document-meta ( office:meta? ) >

<!ATTLIST office:document-meta xmlns:office	CDATA #FIXED "&nOffice;">
<!ATTLIST office:document-meta xmlns:meta	CDATA #FIXED "&nMeta;">
<!ATTLIST office:document-meta xmlns:script	CDATA #FIXED "&nScript;">
<!ATTLIST office:document-meta xmlns:style	CDATA #FIXED "&nStyle;">
<!ATTLIST office:document-meta xmlns:text	CDATA #FIXED "&nText;">
<!ATTLIST office:document-meta xmlns:table	CDATA #FIXED "&nTable;">
<!ATTLIST office:document-meta xmlns:draw	CDATA #FIXED "&nDraw;">
<!ATTLIST office:document-meta xmlns:chart	CDATA #FIXED "&nChart;">
<!ATTLIST office:document-meta xmlns:number	CDATA #FIXED "&nNumber;">
<!ATTLIST office:document-meta xmlns:fo		CDATA #FIXED "&nFO;">
<!ATTLIST office:document-meta xmlns:xlink	CDATA #FIXED "&nXLink;">
<!ATTLIST office:document-meta xmlns:svg		CDATA #FIXED "&nSVG;">
<!ATTLIST office:document-meta xmlns:dc		CDATA #FIXED "&nDC;">

<!ATTLIST office:document-meta office:version	%string; #IMPLIED>

<!ELEMENT office:document-settings (office:settings) >
<!ATTLIST office:document-settings xmlns:office	CDATA #FIXED "&nOffice;">
<!ATTLIST office:document-settings xmlns:xlink	CDATA #FIXED "&nXLink;">
<!ATTLIST office:document-settings xmlns:config	CDATA #FIXED "&nConfig;">

<!ATTLIST office:document-settings office:version	%string; #IMPLIED>

<!ENTITY % meta "(meta:generator?,
				  dc:title?,
				  dc:description?,
				  dc:subject?,
				  meta:initial-creator?,
				  meta:creation-date?,
				  dc:creator?,
				  dc:date?,
				  meta:printed-by?,
				  meta:print-date?,
				  meta:keywords?,
				  dc:language?,
				  meta:editing-cycles?,
				  meta:editing-duration?,
				  meta:hyperlink-behaviour?,
				  meta:auto-reload?,
				  meta:template?,
				  meta:user-defined*,
				  meta:document-statistic?)">
<!ELEMENT office:meta %meta;>

<!ENTITY % script	"(script:library-embedded |
					  script:library-linked)*">
<!ELEMENT office:script %script;>

<!ELEMENT office:font-decls (style:font-decl)*>

<!ENTITY % styles "(style:default-style|style:style|text:list-style|
		number:number-style|number:currency-style|number:percentage-style|
		number:date-style|number:time-style|number:boolean-style|
		number:text-style|
		draw:gradient|draw:hatch|draw:fill-image|draw:marker|draw:stroke-dash|
		style:presentation-page-layout|draw:transparency)">

<!-- Validity constraint: The elements
		text:outline-style,
		text:footnotes-configuration,
		text:endnotes-configuration,
		text:bibliography-configuration and
		text:linenumbering-configuration
	may appear only once!
	Unfortunatetly, this constraint cannot be easily specified in the DTD.
-->
<!ELEMENT office:styles (%styles;|text:outline-style|
		text:footnotes-configuration|text:endnotes-configuration|
		text:bibliography-configuration|text:linenumbering-configuration)*>

<!ELEMENT office:automatic-styles (%styles;|style:page-master)*>

<!ELEMENT office:master-styles (draw:layer-set?,style:handout-master?,style:master-page*) >

<!ENTITY % text-decls "text:variable-decls?, text:sequence-decls?,
					   text:user-field-decls?, text:dde-connection-decls?, 
					   text:alphabetical-index-auto-mark-file?" >

<!ENTITY % change-marks "text:change | text:change-start | text:change-end">

<!ENTITY % body "(office:forms?,(text:tracked-changes|table:tracked-changes)?,%text-decls;,table:calculation-settings?,table:content-validations?,table:label-ranges?,
		(text:h|text:p|text:ordered-list|
		text:unordered-list|table:table|draw:page|
		draw:a|%shape;|text:section|text:table-of-content|
		text:illustration-index|text:table-index|text:object-index|
		text:user-index|text:alphabetical-index|text:bibliography|
		%change-marks;)*,
		table:named-expressions?,
		table:database-ranges?,table:data-pilot-tables?,
		table:consolidation?,
		table:dde-links?,
		presentation:settings?)">
<!ELEMENT office:body %body;>
<!ATTLIST office:body table:structure-protected %boolean; "false"
			table:protection-key CDATA #IMPLIED>

<!ELEMENT office:events (script:event|presentation:event)*>

<!-- DDE source: for text sections and tables -->
<!ELEMENT office:dde-source EMPTY>
<!ATTLIST office:dde-source office:dde-application CDATA #IMPLIED>
<!ATTLIST office:dde-source office:dde-topic CDATA #IMPLIED>
<!ATTLIST office:dde-source office:dde-item CDATA #IMPLIED>
<!ATTLIST office:dde-source office:automatic-update %boolean; "false">
<!ATTLIST office:dde-source office:name CDATA #IMPLIED>
<!ATTLIST office:dde-source table:conversion-mode (into-default-style-data-style|into-english-number|let-text) "into-default-style-data-style" >

<!-- annotations -->
<!-- limitation: in the current implementation, only plain text inside of
     paragraphs is supported -->
<!ELEMENT office:annotation (text:p)*>
<!ATTLIST office:annotation office:author %string; #IMPLIED>
<!ATTLIST office:annotation office:create-date %date; #IMPLIED>
<!ATTLIST office:annotation office:create-date-string %string; #IMPLIED>
<!ATTLIST office:annotation office:display %boolean; "false">

<!ELEMENT office:change-info (text:p)*>
<!ATTLIST office:change-info office:chg-author %string; #REQUIRED>
<!ATTLIST office:change-info office:chg-date-time %timeInstance; #REQUIRED>

<!ELEMENT office:binary-data (#PCDATA)>
