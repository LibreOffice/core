<!--
	$Id: chart.mod,v 1.1 2000-09-29 16:16:03 mh Exp $
 
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


<!ENTITY % chart-class "(line|area|circle|ring|scatter|radar|bar|stock)">
<!ENTITY % chart-solid-type "(cuboid|cylinder|cone|pyramid)">

<!-- Chart element -->
<!ELEMENT chart:chart ( chart:title?, chart:subtitle?, chart:legend?,
					    chart:plot-area,
						table:table )>
<!ATTLIST chart:chart
		  office:class %chart-class; #REQUIRED
		  svg:width %length; #IMPLIED
		  svg:height %length; #IMPLIED
		  chart:style-name %styleName; #IMPLIED >

<!ATTLIST style:properties 
		  chart:scale-text %boolean; "true"
		  chart:stock-updown-bars %boolean; "false"
		  chart:stock-with-volume %boolean; "false"
		  chart:three-dimensional %boolean; "false"
		  chart:deep %boolean; "false"
		  chart:lines %boolean; "false"
		  chart:percentage %boolean; "false"
		  chart:solid-type %chart-solid-type; "cuboid"
		  chart:splines %nonNegativeInteger; "0"
		  chart:stacked %boolean; "false"
		  chart:symbol %integer; "-1"
		  chart:vertical %boolean; "false"
		  chart:lines-used %nonNegativeInteger; "0"
		  chart:connect-bars %boolean; "false">

<!-- Main/Sub Title -->
<!-- the cell-address attribute is currently not supported for titles -->
<!ELEMENT chart:title (text:p)?>
<!ATTLIST chart:title
		  table:cell-range %cell-address; #IMPLIED
		  svg:x %coordinate; #IMPLIED
		  svg:y %coordinate; #IMPLIED
		  chart:style-name %styleName; #IMPLIED >

<!ELEMENT chart:subtitle (text:p)?>
<!ATTLIST chart:subtitle
		  table:cell-range %cell-address; #IMPLIED
		  svg:x %coordinate; #IMPLIED
		  svg:y %coordinate; #IMPLIED
		  chart:style-name %styleName; #IMPLIED >

<!-- you must specify either a legend-position or both, x and y coordinates -->
<!ELEMENT chart:legend EMPTY>
<!ATTLIST chart:legend
		  chart:legend-position (top|left|bottom|right) "right"
		  svg:x %coordinate; #IMPLIED
		  svg:y %coordinate; #IMPLIED
		  chart:style-name %styleName; #IMPLIED >

<!-- Plot-Area specification -->

<!ELEMENT chart:plot-area (chart:axis*,
						   chart:categories?,
						   chart:series*,
						   chart:wall?,
						   chart:floor?) >

<!ATTLIST chart:plot-area
		  svg:x %coordinate; #IMPLIED
		  svg:y %coordinate; #IMPLIED
		  svg:width %length; #IMPLIED
		  svg:height %length; #IMPLIED
		  chart:style-name %styleName; #IMPLIED >

<!ELEMENT chart:wall EMPTY>
<!ATTLIST chart:wall
		  svg:width %length; #IMPLIED
		  chart:style-name %styleName; #IMPLIED >

<!ELEMENT chart:floor EMPTY>
<!ATTLIST chart:floor
		  svg:width %length; #IMPLIED
		  chart:style-name %styleName; #IMPLIED >

<!-- Axis -->

<!ELEMENT chart:axis (chart:title?, chart:grid*)>
<!ATTLIST chart:axis
		  office:class (category|value|series|domain) #REQUIRED
		  office:name %string; #IMPLIED
		  chart:style-name %styleName; #IMPLIED >

<!ATTLIST style:properties
		  chart:tick-marks-major-inner %boolean; "false"
		  chart:tick-marks-major-outer %boolean; "true"
		  chart:tick-marks-minor-inner %boolean; "false"
		  chart:tick-marks-minor-outer %boolean; "false"
		  chart:logarithmic %boolean; "false"
		  chart:maximum %float; #IMPLIED
		  chart:minimum %float; #IMPLIED
		  chart:origin %float; #IMPLIED
		  chart:interval-major %float; #IMPLIED
		  chart:interval-minor %float; #IMPLIED
		  chart:gap-width %integer; #IMPLIED
		  chart:overlap %integer; #IMPLIED
		  text:line-break %boolean; "true"
		  chart:display-label %boolean; "true"
		  chart:label-arrangement (side-by-side|stagger-even|stagger-odd) "side-by-side" >

<!ELEMENT chart:grid EMPTY>
<!ATTLIST chart:grid
		  office:class (major|minor) "major"
		  chart:style-name %styleName; #IMPLIED >


<!ELEMENT chart:categories EMPTY>
<!ATTLIST chart:categories
		  table:cell-range-address %cell-range-address; #REQUIRED >

<!--
	each series element must have an cell-range-address element that points 
	to the underlying table data.
	Impl. Note: Internally all href elements are merged to one table range
	that represents the data for the whole chart
-->
<!ELEMENT chart:series ( chart:domain*,
						 chart:data-point* )>
<!ATTLIST chart:series
		  chart:values-cell-range-address %cell-range-address; #REQUIRED
		  chart:label-cell-address %cell-address; #IMPLIED
		  office:class %chart-class; #IMPLIED
		  chart:attached-axis %string; #IMPLIED
		  chart:style-name %styleName; #IMPLIED >

<!ELEMENT chart:domain EMPTY>
<!ATTLIST chart:domain
		  table:cell-range-address %cell-range-address; #REQUIRED >

<!ELEMENT chart:data-point EMPTY>
<!ATTLIST chart:data-point
		  chart:repeated %nonNegativeInteger; #IMPLIED
		  chart:style-name %styleName; #IMPLIED >

<!-- statistical properties -->

<!ATTLIST style:properties
		  chart:mean-value %boolean; #IMPLIED
		  chart:error-margin %float; #IMPLIED
		  chart:error-lower-limit %float; #IMPLIED
		  chart:error-upper-limit %float; #IMPLIED
		  chart:error-upper-indicator %boolean; #IMPLIED
		  chart:error-lower-indicator %boolean; #IMPLIED >

<!-- data label properties -->

<!ATTLIST style:properties
		  chart:data-label-number (none|value|percentage) "none"
		  chart:data-label-text %boolean; "false"
		  chart:data-label-symbol %boolean; "false" >

<!-- general text properties -->

<!ATTLIST style:properties text:rotation-angle %integer; "0" >
