<!--
	$Id: office.mod,v 1.7 2000-11-02 15:51:17 dvo Exp $

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

<!ELEMENT office:document ( office:meta?,
							office:styles?,
							office:automatic-styles?,
							office:master-styles?,
							office:body ) >

<!ATTLIST office:document xmlns:office	CDATA #FIXED "&nOffice;">
<!ATTLIST office:document xmlns:meta	CDATA #FIXED "&nMeta;">
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
						  (text|online-text|
						   drawing|presentation|
						   spreadsheet|chart) #REQUIRED>

<!ATTLIST office:document office:version	%string; #IMPLIED>


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
				  meta:user-defined*)">
<!ELEMENT office:meta %meta;>


<!ENTITY % styles "(style:style|text:list-style|number:number-style|
		number:currency-style|number:percentage-style|number:date-style|
		number:time-style|number:boolean-style|number:text-style|
		draw:gradient|draw:hatch|draw:fill-image|draw:marker)">

<!-- Validity constraint: The elements
		text:outline-style,
		text:footnotes-configuration and
		text:endnotes-configuration
	may appear only once!
	Unfortunatetly, this constraint cannot be easily specified in the DTD.
-->
<!ELEMENT office:styles (%styles;|text:outline-style|
		text:footnotes-configuration|text:endnotes-configuration)*>

<!ELEMENT office:automatic-styles (%styles;|style:page-master)*>

<!ELEMENT office:master-styles (style:master-page)* >

<!ENTITY % text-decls "text:variable-decls?, text:sequence-decls?,
					   text:user-field-decls?, text:dde-connection-decls?" >

<!ENTITY % body "(%text-decls;,table:content-validations?,table:label-ranges?,
		(text:h|text:p|text:ordered-list|
		text:unordered-list|table:table|chart:chart|draw:page|
		draw:a|draw:text-box|draw:image|text:section|text:table-of-content)*,
		table:named-expressions?,
		table:database-ranges?,table:data-pilot-tables?,
		table:consolidation?)">
<!ELEMENT office:body %body;>
