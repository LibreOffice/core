<!--
	$Id: chart.mod,v 1.1.1.1 2003-10-06 09:58:34 bm Exp $

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


<!ENTITY % chart-class "(line|area|circle|ring|scatter|radar|bar|stock|add-in)">
<!-- xxxx ::: maybe expand this for K-Office: +gantt  and +surface ? for excel -->
<!ENTITY % chart-solid-type "(cuboid|cylinder|cone|pyramid)">

<!-- Chart element -->
<!ELEMENT chart:chart ( chart:title?, chart:subtitle?, chart:legend?,
					    chart:plot-area,
						table:table? )>
<!ATTLIST chart:chart
		  chart:class %chart-class; #REQUIRED
		  chart:add-in-name %string; #IMPLIED
		  chart:table-number-list %string; #IMPLIED
		  draw:name %string; #IMPLIED
		  %draw-position;
		  %draw-size;
		  %draw-style-name;
		  chart:column-mapping %string; #IMPLIED
		  chart:row-mapping %string; #IMPLIED
		  chart:style-name %styleName; #IMPLIED>

<!ATTLIST chart:chart %presentation-class; >
<!ATTLIST chart:chart %zindex;>
<!ATTLIST chart:chart %draw-end-position; >
<!ATTLIST chart:chart draw:id %draw-shape-id; >
<!ATTLIST chart:chart draw:layer %layerName; #IMPLIED>

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
		  chart:connect-bars %boolean; "false"
		  chart:spline-order %nonNegativeInteger; "2"
		  chart:spline-resolution %nonNegativeInteger; "20"
          chart:pie-offset %nonNegativeInteger; "0">

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
<!-- xxxx ::: add this for K-Office -->
		  chart:legend-alignment (center|left-or-top|right-or-bottom)
		  svg:x %coordinate; #IMPLIED
		  svg:y %coordinate; #IMPLIED
		  chart:style-name %styleName; #IMPLIED >
<!-- Plot-Area specification -->
<!ELEMENT chart:plot-area (dr3d:light*,
						   chart:axis*,
<!-- xxxx ::: for K-Office move categories to axis-->
						   chart:categories?,
						   chart:series*,
						   chart:stock-gain-marker?,
						   chart:stock-loss-marker?,
						   chart:stock-range-line?,
						   chart:wall?,
						   chart:floor?) >
<!ATTLIST chart:plot-area
		  svg:x %coordinate; #IMPLIED
		  svg:y %coordinate; #IMPLIED
		  svg:width %length; #IMPLIED
		  svg:height %length; #IMPLIED
		  chart:style-name %styleName; #IMPLIED
		  table:cell-range-address %cell-range-address; #IMPLIED
		  chart:table-number-list %string; #IMPLIED
		  chart:data-source-has-labels (none|row|column|both) "none" >
<!-- 3d scene attributes on plot-area -->
<!ATTLIST chart:plot-area
		  dr3d:vrp %vector3D; #IMPLIED
		  dr3d:vpn %vector3D; #IMPLIED
		  dr3d:vup %vector3D; #IMPLIED
		  dr3d:projection (parallel|perspective) #IMPLIED
		  dr3d:transform CDATA #IMPLIED
		  dr3d:distance %length; #IMPLIED
		  dr3d:focal-length %length; #IMPLIED
		  dr3d:shadow-slant %nonNegativeInteger; #IMPLIED
		  dr3d:shade-mode (flat|phong|gouraud|draft) #IMPLIED
		  dr3d:ambient-color %color; #IMPLIED
		  dr3d:lighting-mode %boolean; #IMPLIED >
<!ATTLIST style:properties
		  chart:series-source (columns|rows) "columns" >
<!ELEMENT chart:wall EMPTY>
<!ATTLIST chart:wall
		  svg:width %length; #IMPLIED
		  chart:style-name %styleName; #IMPLIED >
<!ELEMENT chart:floor EMPTY>
<!ATTLIST chart:floor
		  svg:width %length; #IMPLIED
		  chart:style-name %styleName; #IMPLIED >
<!-- Stock chart elements -->
<!ELEMENT chart:stock-gain-marker EMPTY>
<!ATTLIST chart:stock-gain-marker
		  chart:style-name %styleName; #IMPLIED >
<!ELEMENT chart:stock-loss-marker EMPTY>
<!ATTLIST chart:stock-loss-marker
		  chart:style-name %styleName; #IMPLIED >
<!ELEMENT chart:stock-range-line EMPTY>
<!ATTLIST chart:stock-range-line
		  chart:style-name %styleName; #IMPLIED >
<!-- Axis -->
<!ELEMENT chart:axis (chart:title?, chart:grid*)>
<!ATTLIST chart:axis
		  chart:class (category|value|series|domain) #REQUIRED
		  chart:name %string; #IMPLIED
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
		  chart:label-arrangement (side-by-side|stagger-even|stagger-odd) "side-by-side"
		  chart:text-overlap %boolean; "false"
		  chart:visible %boolean; "true"
		  chart:link-data-style-to-source %boolean; "true" >
<!ELEMENT chart:grid EMPTY>
<!ATTLIST chart:grid
		  chart:class (major|minor) "major"
		  chart:style-name %styleName; #IMPLIED >
<!ELEMENT chart:categories EMPTY>
<!ATTLIST chart:categories
		  table:cell-range-address %cell-range-address; #IMPLIED >
<!--
	each series element must have an cell-range-address element that points
	to the underlying table data.
	Impl. Note: Internally all href elements are merged to one table range
	that represents the data for the whole chart
-->
<!ELEMENT chart:series ( chart:domain*,
						 chart:data-point* )>
<!ATTLIST chart:series
		  chart:values-cell-range-address %cell-range-address; #IMPLIED
		  chart:label-cell-address %cell-address; #IMPLIED
		  chart:class %chart-class; #IMPLIED
   	         chart:attached-axis %string; #IMPLIED
<!-- xxxx ::: replace? this for K-Office -->
		  chart:attached-axis-x %string; #IMPLIED
   	         chart:attached-axis-y %string; #IMPLIED
		  chart:attached-axis-z %string; #IMPLIED
		  chart:style-name %styleName; #IMPLIED >
<!ELEMENT chart:domain EMPTY>
<!ATTLIST chart:domain
		  table:cell-range-address %cell-range-address; #IMPLIED >
<!ELEMENT chart:data-point EMPTY>
<!ATTLIST chart:data-point
		  chart:repeated %nonNegativeInteger; #IMPLIED
		  chart:style-name %styleName; #IMPLIED >
<!-- statistical properties -->
<!ATTLIST style:properties
		  chart:mean-value %boolean; #IMPLIED
		  chart:error-category (none|variance|standard-deviation|percentage|error-margin|constant) "none"
		  chart:error-percentage %float; #IMPLIED
		  chart:error-margin %float; #IMPLIED
		  chart:error-lower-limit %float; #IMPLIED
		  chart:error-upper-limit %float; #IMPLIED
		  chart:error-upper-indicator %boolean; #IMPLIED
		  chart:error-lower-indicator %boolean; #IMPLIED
		  chart:regression-type (none|linear|logarithmic|exponential|power) "none" >
<!-- data label properties -->
<!ATTLIST style:properties
		  chart:data-label-number (none|value|percentage) "none"
		  chart:data-label-text %boolean; "false"
		  chart:data-label-symbol %boolean; "false" >
<!-- general text properties -->
<!ATTLIST style:properties
		  text:rotation-angle %integer; "0"
		  fo:direction (ltr|ttb) #IMPLIED >
<!-- symbol properties -->
<!ATTLIST style:properties
		  chart:symbol-width %nonNegativeLength; #IMPLIED
		  chart:symbol-height %nonNegativeLength; #IMPLIED
		  chart:symbol-image-name %string; #IMPLIED >