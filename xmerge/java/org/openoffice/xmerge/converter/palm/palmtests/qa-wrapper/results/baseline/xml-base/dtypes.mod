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

<!ENTITY % positiveNumberOrDefault "CDATA">

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
<!-- a color value having the format #rrggbb or "transparent" -->
<!ENTITY % transparentOrColor			"CDATA">

<!-- a percentage -->
<!ENTITY % percentage 			"CDATA">

<!-- a length (i.e. 1cm or .6inch) -->
<!ENTITY % length				"CDATA">
<!ENTITY % positiveLength		"CDATA">
<!ENTITY % nonNegativeLength	"CDATA">
<!ENTITY % lengthOrNoLimit "CDATA">

<!-- a length or a percentage -->
<!ENTITY % lengthOrPercentage	"CDATA">
<!ENTITY % positiveLengthOrPercentage	"CDATA">

<!-- a pixel length (i.e. 2px) -->
<!ENTITY % nonNegativePixelLength	"CDATA">

<!-- a float or a percentage -->
<!ENTITY % floatOrPercentage	"CDATA">

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

<!ENTITY % coordinateOrPercentage	"CDATA">

<!ENTITY % shape "draw:rect|draw:line|draw:polyline|draw:polygon|draw:path|
				   draw:circle|draw:ellipse|draw:g|draw:page-thumbnail|
				   draw:text-box|draw:image|draw:object|draw:object-ole|
				   draw:applet|draw:floating-frame|draw:plugin|
				   draw:measure|draw:caption|draw:connector|chart:chart|
				   dr3d:scene|draw:control" >
<!ENTITY % shapes "(%shape;)" >

<!ENTITY % anchorType "(page|frame|paragraph|char|as-char)">

<!ENTITY % control-id "form:id CDATA #REQUIRED">
