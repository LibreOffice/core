<!--
	$Id: style.mod,v 1.8 2000-10-19 10:13:55 dr Exp $

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


<!ELEMENT style:style ( style:properties?,style:map*)>

<!ATTLIST style:style style:name %styleName; #REQUIRED>

<!ENTITY % styleFamily "(paragraph|text|section|
						 table|table-column|table-row|table-cell|table-page|chart|graphics|default|drawing-page|presentation)">
<!ATTLIST style:style style:family %styleFamily; #REQUIRED>

<!ATTLIST style:style style:parent-style-name %styleName; #IMPLIED>
<!ATTLIST style:style style:next-style-name %styleName; #IMPLIED>
<!ATTLIST style:style style:list-style-name %styleName; #IMPLIED>
<!ATTLIST style:style style:data-style-name %styleName; #IMPLIED>

<!ATTLIST style:style style:auto-update %boolean; "false">

<!ATTLIST style:style style:class %string; #IMPLIED>


<!ELEMENT style:map EMPTY>

<!ATTLIST style:map style:condition %string; #REQUIRED>
<!ATTLIST style:map style:apply-style-name %styleName; #REQUIRED>
<!ATTLIST style:map style:base-cell-address %cell-address; #IMPLIED>

<!ELEMENT style:properties ANY>

<!-- number format properties -->
<!ATTLIST style:properties style:num-prefix %string; #IMPLIED>
<!ATTLIST style:properties style:num-suffix %string; #IMPLIED>
<!ATTLIST style:properties style:num-format %string; #IMPLIED>
<!ATTLIST style:properties style:num-letter-sync %boolean; #IMPLIED>

<!-- frame properties -->
<!ATTLIST style:properties fo:width %positiveLength; #IMPLIED>
<!ATTLIST style:properties fo:height %positiveLength; #IMPLIED>
<!ATTLIST style:properties style:vertical-pos (top|middle|bottom|from-top) #IMPLIED>
<!ATTLIST style:properties style:vertical-rel (page|page-content|
											   frame|frame-content|
											   paragraph|paragraph-content|
											   line|baseline|char) #IMPLIED>
<!ATTLIST style:properties style:horizontal-pos (left|center|right|from-left|inside|outside|from-inside) #IMPLIED>
<!ATTLIST style:properties style:horizontal-rel (page|page-content|
											   frame|frame-content|
											   paragraph|paragraph-content|
											   char) #IMPLIED>
<!ATTLIST style:properties svg:width %lengthOrPercentage; #IMPLIED>
<!ATTLIST style:properties svg:height %lengthOrPercentage; #IMPLIED>
<!ATTLIST style:properties fo:min-height %lengthOrPercentage; #IMPLIED>
<!ATTLIST style:properties text:anchor-type %anchorType; #IMPLIED>
<!ATTLIST style:properties text:anchor-page-number %positiveInteger; #IMPLIED>
<!ATTLIST style:properties svg:x %coordinate; #IMPLIED>
<!ATTLIST style:properties svg:y %coordinate; #IMPLIED>
<!ATTLIST style:properties style:print-content %boolean; #IMPLIED>
<!ATTLIST style:properties style:protect %boolean; #IMPLIED>
<!ATTLIST style:properties style:wrap (none|left|right|parallel|dynamic|run-trough) #IMPLIED>
<!ENTITY % noLimitOrPositiveInteger "CDATA">
<!ATTLIST style:properties style:number-wrapped-paragraphs %noLimitOrPositiveInteger; #IMPLIED>
<!ATTLIST style:properties style:wrap-contour %boolean; #IMPLIED>
<!ATTLIST style:properties style:wrap-contour-mode (full|outside) #IMPLIED>
<!ATTLIST style:properties style:run-through (foreground|background) #IMPLIED>
<!ATTLIST style:properties style:editable %boolean; #IMPLIED>
<!ATTLIST style:properties style:mirror CDATA #IMPLIED>
<!ATTLIST style:properties style:clip CDATA #IMPLIED>

<!-- text properties -->
<!ATTLIST style:properties fo:font-variant (normal|small-caps) #IMPLIED>
<!ATTLIST style:properties fo:text-transform (none|lowercase|
											  uppercase|capitalize) #IMPLIED>
<!ATTLIST style:properties fo:color %color; #IMPLIED>
<!ATTLIST style:properties style:text-outline %boolean; #IMPLIED>
<!ATTLIST style:properties style:text-crossing-out
						   (none|single-line|double-line|thick-line|slash|X)
						   #IMPLIED>
<!ATTLIST style:properties style:text-position CDATA #IMPLIED>
<!ATTLIST style:properties fo:font-family %string; #IMPLIED>
<!ATTLIST style:properties style:font-family-generic
						   (roman|swiss|modern|decorative|script|system)
						   #IMPLIED>
<!ATTLIST style:properties style:font-style-name %string; #IMPLIED>
<!ATTLIST style:properties style:font-pitch (fixed|variable) #IMPLIED>
<!ATTLIST style:properties style:font-charset %textEncoding; #IMPLIED>
<!ATTLIST style:properties fo:font-size %positiveLengthOrPercentage; #IMPLIED>
<!ENTITY % normalOrLength "CDATA">
<!ATTLIST style:properties fo:letter-spacing %normalOrLength; #IMPLIED>
<!ATTLIST style:properties fo:language %languageOnly; #IMPLIED>
<!ATTLIST style:properties fo:country %country; #IMPLIED>
<!ATTLIST style:properties fo:font-style (normal|italic|oblique) #IMPLIED>
<!ATTLIST style:properties fo:text-shadow CDATA #IMPLIED>
<!ATTLIST style:properties style:text-underline
						   (none|single|double|dotted|dash|long-dash|dot-dash|
							dot-dot-dash|wave|bold|bold-dotted|bold-dash|
							bold-long-dash|bold-dot-dash|bold-dot-dot-dash|
							bold-wave|double-wave|small-wave) #IMPLIED>
<!ATTLIST style:properties fo:font-weight CDATA #IMPLIED>
<!ATTLIST style:properties fo:score-spaces %boolean; #IMPLIED>
<!ATTLIST style:properties style:letter-kerning %boolean; #IMPLIED>
<!ATTLIST style:properties style:text-blinking %boolean; #IMPLIED>
<!ATTLIST style:properties style:text-background-color %transparentOrColor;
						   #IMPLIED>

<!-- paragraph properties -->
<!ENTITY % nonNegativeLengthOrPercentageOrNormal "CDATA">
<!ATTLIST style:properties fo:line-height
						   %nonNegativeLengthOrPercentageOrNormal; #IMPLIED>
<!ATTLIST style:properties style:line-height-at-least %nonNegativeLength;
						   #IMPLIED>
<!ATTLIST style:properties style:line-spacing %length; #IMPLIED>
<!ATTLIST style:properties fo:text-align (start|end|center|justify) #IMPLIED>
<!ATTLIST style:properties fo:text-align-last (start|center|justify) #IMPLIED>
<!ATTLIST style:properties style:text-align-source (fix|value-type) #IMPLIED>
<!ATTLIST style:properties style:justify-single-word %boolean; #IMPLIED>
<!ATTLIST style:properties style:break-inside (auto|avoid) #IMPLIED>
<!ATTLIST style:properties fo:widows %nonNegativeInteger; #IMPLIED>
<!ATTLIST style:properties fo:orphans %nonNegativeInteger; #IMPLIED>

<!ATTLIST style:properties fo:hyphenate %boolean; #IMPLIED>
<!ATTLIST style:properties fo:hyphenate-keep (none|page) #IMPLIED>
<!ATTLIST style:properties fo:hyphenation-remain-char-count %positiveInteger;
						   #IMPLIED>
<!ATTLIST style:properties fo:hyphenation-push-char-count %positiveInteger;
						   #IMPLIED>
<!ATTLIST style:properties fo:hyphenation-ladder-count
						   %noLimitOrPositiveInteger;  #IMPLIED>

<!ELEMENT style:tab-stops (style:tab-stop)*>
<!ELEMENT style:tab-stop EMPTY>
<!ATTLIST style:tab-stop style:position %nonNegativeLength; #REQUIRED>
<!ATTLIST style:tab-stop style:type (left|center|right|char) "left">
<!ATTLIST style:tab-stop style:char %character; #IMPLIED>
<!ATTLIST style:tab-stop style:leader-char %character; " ">

<!ELEMENT style:drop-cap EMPTY>
<!ENTITY % wordOrPositiveInteger "CDATA">
<!ATTLIST style:drop-cap style:length %wordOrPositiveInteger; "1">
<!ATTLIST style:drop-cap style:lines %positiveInteger; "1">
<!ATTLIST style:drop-cap style:distance %length; "0cm">
<!ATTLIST style:drop-cap style:style-name %styleName; #IMPLIED>

<!ATTLIST style:properties style:register-true %boolean; #IMPLIED>
<!ATTLIST style:properties fo:margin-left %positiveLengthOrPercentage; #IMPLIED>
<!ATTLIST style:properties fo:margin-right %positiveLengthOrPercentage;
						   #IMPLIED>
<!ATTLIST style:properties fo:text-indent %lengthOrPercentage; #IMPLIED>
<!ATTLIST style:properties style:auto-text-indent %boolean; #IMPLIED>
<!ATTLIST style:properties fo:margin-top %positiveLengthOrPercentage; #IMPLIED>
<!ATTLIST style:properties fo:margin-bottom %positiveLengthOrPercentage; #IMPLIED>
<!ATTLIST style:properties fo:break-before (auto|column|page) #IMPLIED>
<!ATTLIST style:properties fo:break-after (auto|column|page) #IMPLIED>
<!ATTLIST style:properties fo:background-color %transparentOrColor; #IMPLIED>

<!ELEMENT style:background-image EMPTY>
<!ATTLIST style:background-image xlink:type (simple) #IMPLIED>
<!ATTLIST style:background-image xlink:href %uriReference; #IMPLIED>
<!ATTLIST style:background-image xlink:show (embed) #IMPLIED>
<!ATTLIST style:background-image xlink:actuate (onLoad) #IMPLIED>
<!ATTLIST style:background-image style:repeat (no-repeat|repeat|stretch)
								 "repeat">
<!ATTLIST style:background-image style:position CDATA "center">
<!ATTLIST style:background-image style:filter-name %string; #IMPLIED>

<!ATTLIST style:properties fo:border CDATA #IMPLIED>
<!ATTLIST style:properties fo:border-top CDATA #IMPLIED>
<!ATTLIST style:properties fo:border-bottom CDATA #IMPLIED>
<!ATTLIST style:properties fo:border-left CDATA #IMPLIED>
<!ATTLIST style:properties fo:border-right CDATA #IMPLIED>
<!ATTLIST style:properties style:border-line-width CDATA #IMPLIED>
<!ATTLIST style:properties style:border-line-width-top CDATA #IMPLIED>
<!ATTLIST style:properties style:border-line-width-bottom CDATA #IMPLIED>
<!ATTLIST style:properties style:border-line-width-left CDATA #IMPLIED>
<!ATTLIST style:properties style:border-line-width-right CDATA #IMPLIED>
<!ATTLIST style:properties fo:padding %nonNegativeLength; #IMPLIED>
<!ATTLIST style:properties fo:padding-top %nonNegativeLength; #IMPLIED>
<!ATTLIST style:properties fo:padding-bottom %nonNegativeLength; #IMPLIED>
<!ATTLIST style:properties fo:padding-left %nonNegativeLength; #IMPLIED>
<!ATTLIST style:properties fo:padding-right %nonNegativeLength; #IMPLIED>
<!ATTLIST style:properties style:shadow CDATA #IMPLIED>
<!ATTLIST style:properties fo:keep-with-next %boolean; #IMPLIED>

<!-- table properties -->
<!ATTLIST style:properties style:width %positiveLength; #IMPLIED>
<!ATTLIST style:properties style:rel-width %percentage; #IMPLIED>
<!ATTLIST style:properties style:may-break-between-rows %boolean; #IMPLIED>
<!ATTLIST style:properties table:page-style-name %styleName; #IMPLIED>
<!ATTLIST style:properties table:display %boolean; #IMPLIED>

<!-- table column properties -->
<!ATTLIST style:properties style:column-width %positiveLength; #IMPLIED>
<!ENTITY % relWidth "CDATA">
<!ATTLIST style:properties style:rel-column-width %relWidth; #IMPLIED>
<!ATTLIST style:properties style:use-optimal-column-width %boolean; #IMPLIED>

<!-- table row properties -->
<!ATTLIST style:properties style:row-height %positiveLength; #IMPLIED>
<!ATTLIST style:properties style:min-row-height %nonNegativeLength; #IMPLIED>
<!ATTLIST style:properties style:use-optimal-row-height %boolean; #IMPLIED>

<!-- table cell properties -->
<!ATTLIST style:properties
	table:align (left | center | right | margins) #IMPLIED
	table:border-model (collapsing | separating) #IMPLIED
	fo:vertical-align (top | middle | bottom) #IMPLIED
	fo:direction (ltr | ttb) #IMPLIED
	style:rotation-angle %nonNegativeInteger; #IMPLIED
	style:rotation-align (none | bottom | top | center) #IMPLIED
	style:cell-protect CDATA #IMPLIED
	fo:wrap-option (no-wrap | wrap) #IMPLIED
>
<!ELEMENT style:columns (style:column*)>
<!ATTLIST style:columns fo:column-count %nonNegativeInteger; #IMPLIED>

<!ELEMENT style:column EMPTY>
<!ATTLIST style:column style:rel-width CDATA #IMPLIED>
<!ATTLIST style:column fo:margin-left %positiveLength; #IMPLIED>
<!ATTLIST style:column fo:margin-right %positiveLength; #IMPLIED>

<!-- page master properties -->
<!ELEMENT style:page-master (style:properties?, style:header-style?, style:footer-style?, style:footnote-layout?)>
<!ATTLIST style:page-master style:name %styleName; #REQUIRED>
<!ATTLIST style:page-master style:page-usage (all|left|right|mirrored) "all">

<!ELEMENT style:header-style (style:properties?)>
<!ELEMENT style:footer-style (style:properties?)>

<!ATTLIST style:properties fo:page-width %length; #IMPLIED>
<!ATTLIST style:properties fo:page-height %length; #IMPLIED>
<!ATTLIST style:properties style:paper-tray-number %positiveNumberOrDefault; #IMPLIED>
<!ATTLIST style:properties style:print-orientation (portrait|landscape) #IMPLIED>
<!ATTLIST style:properties style:register-truth-ref-style-name %styleName; #IMPLIED>
<!ATTLIST style:properties style:print CDATA #IMPLIED>
<!ATTLIST style:properties style:print-page-order (ttb|ltr) #IMPLIED>
<!ATTLIST style:properties style:first-page-number %positiveInteger; #IMPLIED>
<!ATTLIST style:properties style:scale-to %percentage; #IMPLIED>
<!ATTLIST style:properties style:scale-to-pages %positiveInteger; #IMPLIED>

<!ELEMENT style:footnote-layout (style:properties?,style:footnote-sep?)>
<!ATTLIST style:properties style:max-height %lengthOrNoLimit; #IMPLIED>
<!ATTLIST style:properties style:distance-before-sep %length; #IMPLIED>
<!ATTLIST style:properties style:distance-after-sep %length; #IMPLIED>

<!ELEMENT style:footnote-sep EMPTY>
<!ATTLIST style:footnote-sep style:width %length; #REQUIRED>
<!ATTLIST style:footnote-sep style:length %percentage; "100%">
<!ATTLIST style:footnote-sep style:horizontal-align (left|center|right) "left">

<!-- master page properties -->
<!ELEMENT style:master-page ( (style:header, style:header-left?)?, (style:footer, style:footer-left?)?,
								style:style*, (%shapes;)*, presentation:notes? )>
<!ATTLIST style:master-page style:name %styleName; #REQUIRED>
<!ATTLIST style:master-page style:page-master-name %styleName; #REQUIRED>
<!ATTLIST style:master-page style:next-style-name %styleName; #IMPLIED>

<!ENTITY % hd-ft-content "( text:p | (style:region-left?|style:region-center?| style:region-right?) )">
<!ELEMENT style:header %hd-ft-content;>
<!ELEMENT style:footer %hd-ft-content;>
<!ELEMENT style:header-left %hd-ft-content;>
<!ELEMENT style:footer-left %hd-ft-content;>

<!ENTITY % region-content "(text:p*)">
<!ELEMENT style:region-left %region-content;>
<!ELEMENT style:region-center %region-content;>
<!ELEMENT style:region-right %region-content;>
