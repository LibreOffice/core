<!--

   $Id: datastyl.mod,v 1.2 2000-10-18 17:24:56 sab Exp $

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

<!-- data styles -->
<!ENTITY % any-number "( number:number | number:scientific-number | number:fraction )">
<!ENTITY % number-style-content "( number:text | (number:text?,%any-number;,number:text?) )">
<!ELEMENT number:number-style ( style:properties?, %number-style-content;, style:map* )>
<!ELEMENT number:number EMPTY>
<!ELEMENT number:scientific-number EMPTY>
<!ELEMENT number:fraction EMPTY>

<!ENTITY % currency-symbol-and-text "number:currency-symbol,number:text?">
<!ENTITY % number-and-text "number:number,number:text?">
<!ENTITY % currency-style-content "( number:text | (number:text?,%number-and-text;,
	(number:text?,%currency-symbol-and-text;)?) | (%currency-symbol-and-text;,(%number-and-text;)?))">
<!ELEMENT number:currency-style ( style:properties?, %currency-style-content;, style:map* )>
<!ELEMENT number:currency-symbol (#PCDATA)>
<!ATTLIST number:currency-symbol number:language CDATA #IMPLIED>
<!ATTLIST number:currency-symbol number:country CDATA #IMPLIED>

<!ENTITY % percentage-style-content "( number:text | (number:text?,%number-and-text;) )">
<!ELEMENT number:percentage-style ( style:properties?, %percentage-style-content;, style:map* )>

<!ENTITY % any-date "( number:day | number:month | number:year | number:day-of-week | number:week-of-year | number:quarter| number:hours | number:am-pm | number:minutes | number:seconds )">
<!ENTITY % date-style-content "( number:text | (number:text?,(%any-date;,number:text?)+) )">
<!ELEMENT number:date-style ( style:properties?, %date-style-content;, style:map* )>
<!ELEMENT number:day EMPTY>
<!ATTLIST number:day number:style (short|long) "short">
<!ELEMENT number:month EMPTY>
<!ATTLIST number:month number:textual %boolean; "false">
<!ATTLIST number:month number:style (short|long) "short">
<!ELEMENT number:year EMPTY>
<!ATTLIST number:year number:style (short|long) "short">
<!ELEMENT number:day-of-week EMPTY>
<!ATTLIST number:day-of-week number:style (short|long) "short">
<!ELEMENT number:week-of-year EMPTY>
<!ELEMENT number:quarter EMPTY>
<!ATTLIST number:quarter number:style (short|long) "short">

<!ENTITY % any-time "( number:hours | number:am-pm | number:minutes | number:seconds )">
<!ENTITY % time-style-content "( number:text | (number:text?,(%any-time;,number:text?)+) )">
<!ELEMENT number:time-style ( style:properties?, %time-style-content;, style:map* )>
<!ELEMENT number:hours EMPTY>
<!ATTLIST number:hours number:style (short|long) "short">
<!ELEMENT number:minutes EMPTY>
<!ATTLIST number:minutes number:style (short|long) "short">
<!ELEMENT number:seconds EMPTY>
<!ATTLIST number:seconds number:style (short|long) "short">
<!ATTLIST number:seconds number:decimal-places %integer; "0">
<!ELEMENT number:am-pm EMPTY>

<!ENTITY % boolean-style-content "(number:text| (number:text?,number:boolean,number:text?))">
<!ELEMENT number:boolean-style ( style:properties?,%boolean-style-content;, style:map* )>
<!ELEMENT number:boolean EMPTY>

<!ENTITY % text-style-content "(number:text| (number:text?,number:text-content, number:text?))">
<!ELEMENT number:text-style ( style:properties?,%text-style-content;, style:map* )>
<!ELEMENT number:text (#PCDATA)>
<!ELEMENT number:text-content EMPTY>

<!ATTLIST number:number-style style:name %styleName; #REQUIRED>
<!ATTLIST number:currency-style style:name %styleName; #REQUIRED>
<!ATTLIST number:percentage-style style:name %styleName; #REQUIRED>
<!ATTLIST number:date-style style:name %styleName; #REQUIRED>
<!ATTLIST number:time-style style:name %styleName; #REQUIRED>
<!ATTLIST number:boolean-style style:name %styleName; #REQUIRED>
<!ATTLIST number:text-style style:name %styleName; #REQUIRED>

<!ATTLIST number:number-style style:family CDATA #REQUIRED>
<!ATTLIST number:currency-style style:family CDATA #REQUIRED>
<!ATTLIST number:percentage-style style:family CDATA #REQUIRED>
<!ATTLIST number:date-style style:family CDATA #REQUIRED>
<!ATTLIST number:time-style style:family CDATA #REQUIRED>
<!ATTLIST number:boolean-style style:family CDATA #REQUIRED>
<!ATTLIST number:text-style style:family CDATA #REQUIRED>

<!ATTLIST number:number-style number:language CDATA #IMPLIED>
<!ATTLIST number:currency-style number:language CDATA #IMPLIED>
<!ATTLIST number:percentage-style number:language CDATA #IMPLIED>
<!ATTLIST number:date-style number:language CDATA #IMPLIED>
<!ATTLIST number:time-style number:language CDATA #IMPLIED>
<!ATTLIST number:boolean-style number:language CDATA #IMPLIED>
<!ATTLIST number:text-style number:language CDATA #IMPLIED>

<!ATTLIST number:number-style number:country CDATA #IMPLIED>
<!ATTLIST number:currency-style number:country CDATA #IMPLIED>
<!ATTLIST number:percentage-style number:country CDATA #IMPLIED>
<!ATTLIST number:date-style number:country CDATA #IMPLIED>
<!ATTLIST number:time-style number:country CDATA #IMPLIED>
<!ATTLIST number:boolean-style number:country CDATA #IMPLIED>
<!ATTLIST number:text-style number:country CDATA #IMPLIED>

<!ATTLIST number:number-style number:title CDATA #IMPLIED>
<!ATTLIST number:currency-style number:title CDATA #IMPLIED>
<!ATTLIST number:percentage-style number:title CDATA #IMPLIED>
<!ATTLIST number:date-style number:title CDATA #IMPLIED>
<!ATTLIST number:time-style number:title CDATA #IMPLIED>
<!ATTLIST number:boolean-style number:title CDATA #IMPLIED>
<!ATTLIST number:text-style number:title CDATA #IMPLIED>

<!ATTLIST number:number-style style:volatile %boolean; #IMPLIED>
<!ATTLIST number:currency-style style:volatile %boolean; #IMPLIED>
<!ATTLIST number:percentage-style style:volatile %boolean; #IMPLIED>
<!ATTLIST number:date-style style:volatile %boolean; #IMPLIED>
<!ATTLIST number:time-style style:volatile %boolean; #IMPLIED>
<!ATTLIST number:boolean-style style:volatile %boolean; #IMPLIED>
<!ATTLIST number:text-style style:volatile %boolean; #IMPLIED>

<!ATTLIST number:currency-style number:automatic-order %boolean; "false">
<!ATTLIST number:date-style number:automatic-order %boolean; "false">

<!ATTLIST number:date-style number:format-source (fixed|language) "fixed">
<!ATTLIST number:time-style number:format-source (fixed|language) "fixed">

<!ATTLIST number:time-style number:truncate-on-overflow %boolean; "true">

<!ATTLIST number:number number:decimal-places %integer; #IMPLIED>
<!ATTLIST number:scientific-number number:decimal-places %integer; #IMPLIED>

<!ATTLIST number:number number:min-integer-digits %integer; #IMPLIED>
<!ATTLIST number:scientific-number number:min-integer-digits %integer; #IMPLIED>
<!ATTLIST number:fraction number:min-integer-digits %integer; #IMPLIED>

<!ATTLIST number:number number:grouping %boolean; "false">
<!ATTLIST number:scientific-number number:grouping %boolean; "false">
<!ATTLIST number:fraction number:grouping %boolean; "false">

<!ATTLIST number:number number:decimal-replacement CDATA #IMPLIED>

<!ATTLIST number:scientific-number number:min-exponent-digits %integer; #IMPLIED>

<!ATTLIST number:fraction number:min-numerator-digits %integer; #IMPLIED>

<!ATTLIST number:fraction number:min-denominator-digits %integer; #IMPLIED>
