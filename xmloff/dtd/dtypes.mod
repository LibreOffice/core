<!--
	$Id: dtypes.mod,v 1.2 2000-10-10 10:42:02 cl Exp $

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

<!-- datatypes corresponding to XML Schema Part 2 W3C Working draft of	-->
<!-- 07 April 2000														-->

<!-- string -->
<!ENTITY % string				"CDATA">
<!ENTITY % cString				"#PCDATA">

<!-- boolean (values are "true" and "false" -->
<!ENTITY % boolean				"CDATA">

<!-- integer ( ..., -2, -1, 0, 1, 2, ...) -->
<!ENTITY % integer				"CDATA">

<!-- non negative integer ( 0, 1, 2, ...) -->
<!ENTITY % nonNegativeInteger	"CDATA">

<!-- positive integer ( 1, 2, ...) -->
<!ENTITY % positiveInteger		"CDATA">
<!ENTITY % cPositiveInteger		"#PCDATA">

<!-- time duration as specified by ISO8601, section 5.5.3.2 -->
<!ENTITY % timeDuration			"CDATA">
<!ENTITY % cTimeDuration		"#PCDATA">

<!-- time instance as specified by ISO8601, section 5.4 -->
<!ENTITY % timeInstance			"CDATA">
<!ENTITY % cTimeInstance		"#PCDATA">

<!-- date instance as specified by ISO8601, section 5.2.1.1, extended format-->
<!ENTITY % date					"CDATA">
<!ENTITY % cDate				"#PCDATA">

<!-- date duration, like timDuration but truncated to full dates -->
<!ENTITY % dateDuration			"CDATA">
<!ENTITY % cDateDuration		"#PCDATA">

<!-- URI reference -->
<!ENTITY % uriReference			"CDATA">

<!-- language code as specified by RFC1766 -->
<!ENTITY % language				"CDATA">
<!ENTITY % cLanguage			"#PCDATA">

<!-- float -->
<!ENTITY % float "CDATA">

<!-- Some other common used data types -->

<!-- a single UNICODE character -->
<!ENTITY % character			"CDATA">

<!-- a style name -->
<!ENTITY % styleName			"CDATA">

<!-- a target frame mame -->
<!ENTITY % targetFrameName			"CDATA">

<!-- a language without a country as specified by ISO639 -->
<!ENTITY % languageOnly			"CDATA">

<!-- a country as specified by ISO3166 -->
<!ENTITY % country				"CDATA">

<!-- a color value having the format #rrggbb -->
<!ENTITY % color				"CDATA">

<!-- a percentage -->
<!ENTITY % percentage 			"CDATA">

<!-- a length (i.e. 1cm or .6inch) -->
<!ENTITY % length				"CDATA">
<!ENTITY % positiveLength		"CDATA">
<!ENTITY % nonNegativeLength	"CDATA">

<!-- a length or a percentage -->
<!ENTITY % lengthOrPercentage	"CDATA">
<!ENTITY % positiveLengthOrPercentage	"CDATA">

<!-- a text encoding -->
<!ENTITY % textEncoding	"CDATA">

<!-- cell address and cell range address -->
<!ENTITY % cell-address "CDATA">
<!ENTITY % cell-range-address "CDATA">
<!ENTITY % cell-range-address-list "CDATA">

<!-- value types -->
<!ENTITY % valueType "(float|time|date|percentage|currency|boolean|string)">

<!-- an svg coordinate in different distance formats -->
<!ENTITY % coordinate "CDATA">

<!ENTITY % shapes "(draw:rect|draw:line|draw:polyline|draw:polygon|draw:polygonpath|draw:circle|draw:ellipse|draw:g|draw:page-thumbnail|text:text-box)" >

<!ENTITY % anchorType "(page|frame|paragraph|char|as-char)">
