<?xml version="1.0" encoding="UTF-8"?>
<!--<!DOCTYPE xsl:stylesheet [
		<!~~ width of list symbol in 'cm' ~~>
		<!ENTITY list-label-width "0.5">
]>-->
<!--

  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
  
  Copyright 2008 by Sun Microsystems, Inc.
 
  OpenOffice.org - a multi-platform office productivity suite
 
  $RCSfile: body.xsl,v $
 
  $Revision: 1.2 $
 
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
<!--
	For further documentation and updates visit http://xml.openoffice.org/odf2xhtml
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:chart="urn:oasis:names:tc:opendocument:xmlns:chart:1.0" xmlns:config="urn:oasis:names:tc:opendocument:xmlns:config:1.0" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dom="http://www.w3.org/2001/xml-events" xmlns:dr3d="urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0" xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" xmlns:form="urn:oasis:names:tc:opendocument:xmlns:form:1.0" xmlns:math="http://www.w3.org/1998/Math/MathML" xmlns:meta="urn:oasis:names:tc:opendocument:xmlns:meta:1.0" xmlns:number="urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0" xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" xmlns:ooo="http://openoffice.org/2004/office" xmlns:oooc="http://openoffice.org/2004/calc" xmlns:ooow="http://openoffice.org/2004/writer" xmlns:script="urn:oasis:names:tc:opendocument:xmlns:script:1.0" xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" xmlns:xforms="http://www.w3.org/2002/xforms" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" exclude-result-prefixes="chart config dc dom dr3d draw fo form math meta number office ooo oooc ooow script style svg table text xlink" xmlns="http://www.w3.org/1999/xhtml">



	<!--+++++ INCLUDED XSL MODULES +++++-->

	<!-- helper collection, to convert measures (e.g. inch to pixel using DPI (dots per inch) parameter)-->
	<xsl:import href="../../common/measure_conversion.xsl"/>

	<!-- common office body element handling -->
	<xsl:import href="../common/body.xsl"/>

	<!-- common table handling -->
	<xsl:import href="../common/table/table.xsl"/>

	<!-- xhtml table handling -->
	<xsl:include href="table.xsl"/>

	<!-- Useful in case of 'style:map', conditional formatting, where a style references to another -->
	<xsl:key name="styles" match="/*/office:styles/style:style | /*/office:automatic-styles/style:style" use="@style:name"/>


	<!-- ************ -->
	<!-- *** body *** -->
	<!-- ************ -->

	<xsl:template name="create-body">
		<xsl:param name="globalData"/>

		<!-- approximation as attribute belongs to a page style, which won't work in XHTML -->
		<xsl:variable name="pageProperties" select="$globalData/styles-file/*/office:automatic-styles/style:page-layout[1]/style:page-layout-properties"/>

		<xsl:element name="body">
			<!-- direction of text flow -->
			<xsl:variable name="writingMode" select="$pageProperties/@style:writing-mode"/>
			<xsl:if test="$writingMode">
				<xsl:choose>
					<xsl:when test="contains($writingMode, 'lr')">
						<xsl:attribute name="dir">ltr</xsl:attribute>
					</xsl:when>
					<xsl:when test="contains($writingMode, 'rl')">
						<xsl:attribute name="dir">rtl</xsl:attribute>
					</xsl:when>
				</xsl:choose>
			</xsl:if>

			<!-- multiple backgroundimages for different page styles (never used in html) -->
			<xsl:variable name="backgroundImage" select="$pageProperties/style:background-image"/>

			<!-- page margins & background image  -->
			<xsl:if test="$pageProperties/@fo:* or $backgroundImage/@xlink:href">
				<xsl:attribute name="style">
					<xsl:apply-templates select="$pageProperties/@fo:*"/>
					<xsl:if test="$backgroundImage/@xlink:href">
						<xsl:text>background-image:url(</xsl:text>
						<xsl:call-template name="create-href">
							<xsl:with-param name="href" select="$backgroundImage/@xlink:href"/>
						</xsl:call-template>
						<xsl:text>);</xsl:text>

						<xsl:if test="$backgroundImage/@style:repeat">
							<xsl:choose>
								<xsl:when test="$backgroundImage/@style:repeat = 'no-repeat'">
									<xsl:text>background-repeat:no-repeat;</xsl:text>
								</xsl:when>
								<xsl:otherwise>
									<xsl:text>background-repeat:repeat;</xsl:text>
								</xsl:otherwise>
							</xsl:choose>
						</xsl:if>
						<xsl:if test="$backgroundImage/@style:position">
							<xsl:text>background-position:</xsl:text>
							<xsl:value-of select="$backgroundImage/@style:position"/>
							<xsl:text>;</xsl:text>
						</xsl:if>
					</xsl:if>
				</xsl:attribute>
			</xsl:if>
			<!-- processing the content of the xml file -->
			<xsl:apply-templates select="/*/office:body/*">
				<xsl:with-param name="globalData" select="$globalData"/>
			</xsl:apply-templates>
		</xsl:element>
	</xsl:template>


	<xsl:template match="office:body/*">
		<xsl:param name="globalData"/>

		<!-- not using of 'apply-styles-and-content' as the content table information migth have been added to 'globalData' variable -->
		<xsl:apply-templates select="@text:style-name | @draw:style-name | @draw:text-style-name | @table:style-name"><!-- | @presentation:style-name -->
			<xsl:with-param name="globalData" select="$globalData"/>
		</xsl:apply-templates>

		<xsl:apply-templates>
			<xsl:with-param name="globalData" select="$globalData"/>
		</xsl:apply-templates>

		<!-- writing the footer- and endnotes beyond the body -->
		<xsl:call-template name="write-text-nodes">
			<xsl:with-param name="globalData" select="$globalData"/>
		</xsl:call-template>

	</xsl:template>

	<!-- ******************************* -->
	<!-- *** User Field Declarations *** -->
	<!-- ******************************* -->

	<xsl:template match="text:user-field-get">
		<xsl:param name="globalData"/>

		<xsl:value-of select="."/>
	</xsl:template>

	<xsl:template match="text:conditional-text">
		<xsl:param name="globalData"/>

		<xsl:value-of select="."/>
	</xsl:template>

	<!-- ODF text fields -->
	<xsl:template match="text:author-initials |
						text:author-name |
						text:chapter |
						text:character-count |
						text:creation-date |
						text:creation-time |
						text:creator |
						text:date |
						text:description |
						text:editing-cycles |
						text:editing-duration |
						text:file-name |
						text:image-count |
						text:initial-creator |
						text:keywords |
						text:modification-date |
						text:modification-time |
						text:object-count |
						text:page-continuation |
						text:page-count |
						text:page-number |
						text:paragraph-count |
						text:print-date |
						text:print-time |
						text:printed-by |
						text:sender-city |
						text:sender-company |
						text:sender-country |
						text:sender-email |
						text:sender-fax |
						text:sender-firstname |
						text:sender-initials |
						text:sender-lastname |
						text:sender-phone-private |
						text:sender-phone-work |
						text:sender-position |
						text:sender-postal-code |
						text:sender-state-or-province |
						text:sender-street |
						text:sender-title |
						text:sheet-name |
						text:subject |
						text:table-count |
						text:time |
						text:title |
						text:user-defined |
						text:word-count">
		<xsl:param name="globalData"/>

		<xsl:element name="span">
			<xsl:attribute name="title"><xsl:value-of select="local-name()"/></xsl:attribute>
			<xsl:apply-templates>
				<xsl:with-param name="globalData" select="$globalData"/>
			</xsl:apply-templates>
		</xsl:element>
	</xsl:template>



	<!-- *************** -->
	<!-- *** Textbox *** -->
	<!-- *************** -->

	<xsl:template match="draw:text-box">
		<xsl:param name="globalData"/>

		<xsl:element name="div">
			<xsl:if test="@svg:height | @svg:width">
				<xsl:attribute name="style">
					<xsl:choose>
						<xsl:when test="not(@svg:width)">
							<xsl:call-template name="svg:height"/>
						</xsl:when>
						<xsl:when test="not(@svg:height)">
							<xsl:call-template name="svg:width"/>
						</xsl:when>
						<xsl:otherwise>
							<xsl:call-template name="svg:height"/>
							<xsl:call-template name="svg:width"/>
						</xsl:otherwise>
					</xsl:choose>
				</xsl:attribute>
			</xsl:if>
			<xsl:apply-templates select="@draw:name">
				<xsl:with-param name="globalData" select="$globalData"/>
			</xsl:apply-templates>

			<xsl:call-template name="apply-styles-and-content">
				<xsl:with-param name="globalData" select="$globalData"/>
			</xsl:call-template>
		</xsl:element>
	</xsl:template>

	<!-- inline style helper for the 'div' boxes -->
	<xsl:template name="svg:height">
		<xsl:text>height: </xsl:text>
		<xsl:choose>
			<!-- changing the distance measure: inch to in -->
			<xsl:when test="contains(@svg:height, 'inch')">
				<xsl:value-of select="substring-before(@svg:height, 'ch')"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="@svg:height"/>
			</xsl:otherwise>
		</xsl:choose>
		<xsl:text>;</xsl:text>
	</xsl:template>

	<!-- inline style helper for the 'div' boxes -->
	<xsl:template name="svg:width">
		<xsl:text>width: </xsl:text>
		<xsl:choose>
			<!-- changing the distance measure: inch to in -->
			<xsl:when test="contains(@svg:width, 'inch')">
				<xsl:value-of select="substring-before(@svg:width, 'ch')"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="@svg:width"/>
			</xsl:otherwise>
		</xsl:choose>
		<xsl:text>;</xsl:text>
	</xsl:template>



	<!-- ****************** -->
	<!-- *** Paragraphs *** -->
	<!-- ****************** -->

	<xsl:template match="text:p | draw:page">
		<xsl:param name="globalData"/>
		<!-- the footnote symbol is the prefix for a footnote in the footer -->
		<xsl:param name="footnotePrefix"/>

		<!-- 1) A draw:text-box my include paragraphs (text:p) itself and in HTML a 'p' can only have inline documents (no other 'p' as children'),
				a 'div' will be given out instead.
			 2) A images are embedded in a paragraph, but are in CSS not able to express a horizontal alignment for themself (text:align is only valid for block elements).
				A surrounding 'div' element taking over the image style solves that problem, but is invalid as child of a paragraph
		-->
		<xsl:choose>
			<xsl:when test="draw:frame">
				<xsl:element name="div">
					<xsl:call-template name="apply-styles-and-content">
						<xsl:with-param name="globalData" select="$globalData" />
						<xsl:with-param name="footnotePrefix" select="$footnotePrefix" />
					</xsl:call-template>
				</xsl:element>
			</xsl:when>
			<xsl:otherwise>
				<xsl:element name="p">
					<xsl:choose>
						<!-- in ODF borders of paragraphs will be merged by default. Merging means the adjactend paragraphs are building a unit,
							where only the first and the last will have have a border to the surrounding (top / bottom border)
														<xsl:variable name="precedingParagraphStyle" select="preceding-sibling::*[1][name() = 'text:p']/@text:style-name"/>
							<xsl:variable name="followingParagraphStyle" select="following-sibling::*[1][name() = 'text:p']/@text:style-name"/>
							-->
						<xsl:when test="$globalData/all-styles/style[@style:name = current()/@text:style-name]/@mergedBorders">
							<xsl:variable name="precedingParagraphStyle" select="preceding-sibling::*[1][name() = 'text:p']/@text:style-name"/>
							<xsl:variable name="followingParagraphStyle" select="following-sibling::*[1][name() = 'text:p']/@text:style-name"/>
							<xsl:choose>
								<xsl:when test="$precedingParagraphStyle or $followingParagraphStyle">
									<xsl:variable name="isPrecedingBorderParagraph" select="$globalData/all-styles/style[@style:name = $precedingParagraphStyle]/@mergedBorders"/>
									<xsl:variable name="isFollowingBorderParagraph" select="$globalData/all-styles/style[@style:name = $followingParagraphStyle]/@mergedBorders"/>
									<xsl:choose>
										<xsl:when test="not($isPrecedingBorderParagraph) and $isFollowingBorderParagraph">
											<xsl:attribute name="class">
												<xsl:value-of select="concat(translate(@text:style-name, '.,;: %()[]/\+', '_____________'), '_borderStart')"/>
											</xsl:attribute>
											<xsl:apply-templates>
												<xsl:with-param name="globalData" select="$globalData"/>
											</xsl:apply-templates>
										</xsl:when>
										<xsl:when test="$isPrecedingBorderParagraph and not($isFollowingBorderParagraph)">
											<xsl:attribute name="class">
												<xsl:value-of select="concat(translate(@text:style-name, '.,;: %()[]/\+', '_____________'), '_borderEnd')"/>
											</xsl:attribute>
											<xsl:apply-templates>
												<xsl:with-param name="globalData" select="$globalData"/>
											</xsl:apply-templates>
										</xsl:when>
										<xsl:otherwise>
											<xsl:attribute name="class">
												<xsl:value-of select="translate(@text:style-name, '.,;: %()[]/\+', '_____________')"/>
											</xsl:attribute>
											<xsl:apply-templates>
												<xsl:with-param name="globalData" select="$globalData"/>
											</xsl:apply-templates>
										</xsl:otherwise>
									</xsl:choose>
								</xsl:when>
								<xsl:otherwise>
									<xsl:call-template name="write-paragraph">
										<xsl:with-param name="globalData" select="$globalData"/>
									</xsl:call-template>
								</xsl:otherwise>
							</xsl:choose>
						</xsl:when>
						<xsl:otherwise>
							<xsl:call-template name="write-paragraph">
								<xsl:with-param name="globalData" select="$globalData" />
								<xsl:with-param name="footnotePrefix" select="$footnotePrefix" />
							</xsl:call-template>
						</xsl:otherwise>
					</xsl:choose>
				</xsl:element>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name="write-paragraph">
		<xsl:param name="globalData"/>
		<!-- the footnote symbol is the prefix for a footnote in the footer -->
		<xsl:param name="footnotePrefix" />

			<!-- empty paragraph tags does not provoke an carridge return,
				therefore an non breakable space (&nbsp) have been inserted.-->
		<xsl:choose>
			<xsl:when test="node()">
				<xsl:call-template name="apply-styles-and-content">
					<xsl:with-param name="globalData" select="$globalData" />
					<xsl:with-param name="footnotePrefix" select="$footnotePrefix" />
				</xsl:call-template>
			</xsl:when>
			<xsl:otherwise>
				<xsl:call-template name="apply-styles-and-content">
					<xsl:with-param name="globalData" select="$globalData" />
					<xsl:with-param name="footnotePrefix" select="$footnotePrefix" />
				</xsl:call-template>
				<xsl:text>&#160;</xsl:text>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template match="draw:frame">
		<xsl:param name="globalData"/>

		<xsl:element name="div">
			<xsl:if test="@svg:height | @svg:width">
				<xsl:attribute name="style">
					<xsl:choose>
						<xsl:when test="not(@svg:width)">
							<xsl:call-template name="svg:height"/>
						</xsl:when>
						<xsl:when test="not(@svg:height)">
							<xsl:call-template name="svg:width"/>
						</xsl:when>
						<xsl:otherwise>
							<xsl:call-template name="svg:height"/>
							<xsl:call-template name="svg:width"/>
						</xsl:otherwise>
					</xsl:choose>
				</xsl:attribute>
			</xsl:if>
			<xsl:apply-templates select="@draw:name">
				<xsl:with-param name="globalData" select="$globalData"/>
			</xsl:apply-templates>

			<xsl:call-template name="apply-styles-and-content">
				<xsl:with-param name="globalData" select="$globalData"/>
			</xsl:call-template>
		</xsl:element>
	</xsl:template>

	<!-- ***************** -->
	<!-- *** Text Span *** -->
	<!-- ***************** -->

	<xsl:template match="text:span">
		<xsl:param name="globalData"/>

		<xsl:element name="span">
			<xsl:call-template name="apply-styles-and-content">
				<xsl:with-param name="globalData" select="$globalData"/>
			</xsl:call-template>
		</xsl:element>
	</xsl:template>



	<!-- **************** -->
	<!-- *** Headings *** -->
	<!-- **************** -->

	<xsl:template match="text:h">
		<xsl:param name="globalData"/>

		<!-- no creation of empty headings (without text content)   -->
		<xsl:if test="text()">
			<!-- The URL linking of an table-of-content is due to a bug (cp. bug id# 102311) not mapped as URL in the XML.
				 Linking of the table-of-content can therefore only be archieved by a work-around in HTML -->
			<xsl:call-template name="create-heading">
				<xsl:with-param name="globalData" select="$globalData"/>
			</xsl:call-template>
		</xsl:if>
	</xsl:template>

	<!-- default matching for header elements -->
	<xsl:template name="create-heading">
		<xsl:param name="globalData"/>

		<xsl:variable name="headingNumber">
			<xsl:choose>
			<xsl:when test="@text:outline-level &lt; 6">
				<xsl:value-of select="@text:outline-level"/>
			</xsl:when>
			<xsl:otherwise>6</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<xsl:variable name="headertyp" select="concat('h', $headingNumber)"/>
		<xsl:element name="{$headertyp}">
			<!-- outline style 'text:min-label-width' is interpreted as a CSS 'margin-right' attribute
			NOTE: Should be handled as CSS style in style header -->
			<xsl:variable name="min-label" select="$globalData/office:styles/text:outline-style/text:outline-level-style[@text:level = current()/@text:outline-level]/*/@text:min-label-width"/>
			<xsl:attribute name="class">
				<xsl:value-of select="translate(@text:style-name, '.,;: %()[]/\+', '_____________')"/>
			</xsl:attribute>


			<!-- writing out a heading number if desired.-->
			<!-- if a corresponding 'text:outline-style' exist or is not empty -->
			<xsl:choose>
				<xsl:when test="$globalData/office:styles/text:outline-style/text:outline-level-style[@text:level = current()/@text:outline-level]/@style:num-format != ''">

					<!-- Every heading element will get an unique anchor for its file, from its hiearchy level and name:
						 For example:  The heading title 'My favorite heading' might get <a name="1_2_2_My_favorite_heading" /> -->
					<!-- creating an anchor for referencing the heading (e.g. from content table) -->
					<xsl:variable name="headingNumber">
						<xsl:call-template name="write-heading-number">
							<xsl:with-param name="globalData" select="$globalData"/>
						</xsl:call-template>
					</xsl:variable>
					<xsl:call-template name="create-heading-anchor">
						<xsl:with-param name="globalData" select="$globalData"/>
						<xsl:with-param name="headingNumber" select="$headingNumber"/>
					</xsl:call-template>

					<xsl:element name="span">
						<!-- outline style 'text:min-label-width' is interpreted as a CSS 'margin-right' attribute
						NOTE: Should be handled as CSS style in style header -->
						<xsl:variable name="minLabelDistance" select="$globalData/office:styles/text:outline-style/text:outline-level-style[@text:level = current()/@text:outline-level]/*/@text:min-label-distance"/>
						<xsl:variable name="minLabelWidth" select="$globalData/office:styles/text:outline-style/text:outline-level-style[@text:level = current()/@text:outline-level]/*/@text:min-label-width"/>

						<xsl:if test="$minLabelDistance | $minLabelWidth">
							<xsl:attribute name="style">
								<xsl:if test="$minLabelDistance">
										<xsl:text>margin-right:</xsl:text>
										<xsl:call-template name="convert2cm">
											<xsl:with-param name="value" select="$minLabelDistance"/>
										</xsl:call-template>
										<xsl:text>cm;</xsl:text>
								</xsl:if>
								<xsl:if test="$minLabelWidth">
										<xsl:text>min-width:</xsl:text>
										<xsl:call-template name="convert2cm">
											<xsl:with-param name="value" select="$minLabelWidth"/>
										</xsl:call-template>
										<xsl:text>cm;</xsl:text>
								</xsl:if>
								</xsl:attribute>
						</xsl:if>
						<xsl:copy-of select="$headingNumber"/>
					</xsl:element>
				</xsl:when>
				<xsl:otherwise>
					<xsl:call-template name="create-heading-anchor">
						<xsl:with-param name="globalData" select="$globalData"/>
					</xsl:call-template>
				</xsl:otherwise>
			</xsl:choose>

			<xsl:apply-templates>
				<xsl:with-param name="globalData" select="$globalData"/>
			</xsl:apply-templates>
		</xsl:element>

	</xsl:template>


	<!-- creating an anchor for referencing the heading -->
	<xsl:template name="create-heading-anchor">
		<xsl:param name="globalData"/>
		<xsl:param name="headingNumber" />

		<!-- The URL linking of an table-of-content is due to a bug (cp. bug id# 102311) not mapped as URL in the XML.
			 Linking of the table-of-content can therefore only be archieved by a work-around in HTML -->
		<xsl:call-template name="create-default-heading-anchor">
			<xsl:with-param name="globalData" select="$globalData"/>
			<xsl:with-param name="headingNumber" select="$headingNumber"/>
		</xsl:call-template>
	</xsl:template>

	<!-- default matching for header elements -->
	<xsl:template name="create-default-heading-anchor">
		<xsl:param name="headingNumber" />
		<xsl:variable name="title">
			<xsl:apply-templates mode="concatenate"/>
		</xsl:variable>
		<xsl:element namespace="{$namespace}" name="a">
			<xsl:attribute name="name">
				<xsl:value-of select="translate(concat($headingNumber, '_', $title), '.,;: %()[]/\+', '_____________')"/>
			</xsl:attribute>
		</xsl:element>
	</xsl:template>

	<xsl:template name="write-heading-number">
		<xsl:param name="globalData"/>

		<!-- By default heading start with '1', the parameter 'textStartValue' will only be set, if the attribute @text:start-value exist -->
		<xsl:choose>
			<xsl:when test="$globalData/office:styles/text:outline-style/text:outline-level-style[@text:level = current()/@text:outline-level]/@text:start-value">
				<xsl:call-template name="calc-heading-number">
					<xsl:with-param name="globalData" select="$globalData"/>
					<xsl:with-param name="outlineLevel" select="@text:outline-level"/>
					<xsl:with-param name="textStartValue" select="$globalData/office:styles/text:outline-style/text:outline-level-style[@text:level = current()/@text:outline-level]/@text:start-value"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:otherwise>
				<xsl:call-template name="calc-heading-number">
					<xsl:with-param name="globalData" select="$globalData"/>
					<xsl:with-param name="outlineLevel" select="@text:outline-level"/>
				</xsl:call-template>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<!--
	Find the correct heading no, which is the sum of 'text:start-value'
	and preceding siblings of 'text:h' with the same 'text:outline-level' (until a text:outline-level with lower value is found).
	If the 'text:start-value is not set the default value of '1' has to be taken.
	If a heading number is found (e.g. text:outline-level='3') all heading numbers
	for the higher levels have to be written out -->
	<xsl:template name="calc-heading-number">
		<xsl:param name="globalData"/>
		<xsl:param name="outlineLevel"/><!-- text level of the heading -->
		<xsl:param name="iOutlineLevel" select="1"/><!-- iterator, counts from 1 to the text level of the heading -->
		<xsl:param name="textStartValue" select="1"/><!-- text level to start with, default is '1' -->

		<xsl:choose>
			<!-- iText levels counts up from '1' to outlineLevel
				Which means writing a heading number from left to right -->
			<xsl:when test="$iOutlineLevel &lt; $outlineLevel">

			<!-- Write preceding heading numbers -->
				<xsl:call-template name="writeNumber">
					<xsl:with-param name="numberDigit">
						<xsl:call-template name="calc-heading-digit">
							<xsl:with-param name="value" select="0"/>
							<xsl:with-param name="currentoutlineLevel" select="$iOutlineLevel"/>
						</xsl:call-template>
					</xsl:with-param>
					<xsl:with-param name="numberSuffix" select="$globalData/office:styles/text:outline-style/text:outline-level-style[@text:level = ($outlineLevel)]/@style:num-suffix"/>
					<xsl:with-param name="numberFormat" select="$globalData/office:styles/text:outline-style/text:outline-level-style[@text:level = ($outlineLevel)]/@style:num-format"/>
				</xsl:call-template>
				<xsl:choose>
					<xsl:when test="$globalData/office:styles/text:outline-style/text:outline-level-style[@text:level = ($iOutlineLevel + 1)]/@text:start-value">
						<xsl:call-template name="calc-heading-number">
							<xsl:with-param name="globalData" select="$globalData"/>
							<xsl:with-param name="outlineLevel" select="$outlineLevel"/>
							<xsl:with-param name="iOutlineLevel" select="$iOutlineLevel + 1"/>
							<xsl:with-param name="textStartValue" select="$globalData/office:styles/text:outline-style/text:outline-level-style[@text:level = ($iOutlineLevel + 1)]/@text:start-value"/>
						</xsl:call-template>
					</xsl:when>
					<xsl:otherwise>
						<xsl:call-template name="calc-heading-number">
							<xsl:with-param name="globalData" select="$globalData"/>
							<xsl:with-param name="outlineLevel" select="$outlineLevel"/>
							<xsl:with-param name="iOutlineLevel" select="$iOutlineLevel + 1"/>
						</xsl:call-template>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<xsl:otherwise>
				<!-- Write preceding heading numbers -->
				<xsl:call-template name="writeNumber">
					<xsl:with-param name="numberDigit">
						<xsl:call-template name="calc-heading-digit">
							<xsl:with-param name="value" select="$textStartValue"/>
							<xsl:with-param name="currentoutlineLevel" select="$iOutlineLevel"/>
						</xsl:call-template>
					</xsl:with-param>
					<xsl:with-param name="numberSuffix" select="$globalData/office:styles/text:outline-style/text:outline-level-style[@text:level = $outlineLevel]/@style:num-suffix"/>
					<xsl:with-param name="numberFormat" select="$globalData/office:styles/text:outline-style/text:outline-level-style[@text:level = $outlineLevel]/@style:num-format"/>
					<xsl:with-param name="last" select="true()"/>
				</xsl:call-template>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name="writeNumber">
		<xsl:param name="numberDigit"/>
		<xsl:param name="numberFormat"/>
		<xsl:param name="numberSuffix"/>
		<xsl:param name="last"/>

		<xsl:choose>
			<xsl:when test="not($numberFormat) and not($numberSuffix)">
				<xsl:number value="$numberDigit" format="1."/>
			</xsl:when>
			<xsl:when test="not($numberSuffix)">
				<xsl:choose>
					<xsl:when test="$last">
						<xsl:number value="$numberDigit" format="{$numberFormat}"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:number value="$numberDigit" format="{$numberFormat}."/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<xsl:otherwise>
				<xsl:number value="$numberDigit" format="{$numberFormat}{$numberSuffix}"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name="calc-heading-digit">
		<xsl:param name="value"/>
		<xsl:param name="currentoutlineLevel"/>
		<xsl:param name="i" select="1"/>

		<xsl:variable name="precedingoutlineLevel" select="preceding-sibling::text:h[$i]/@text:outline-level"/>
		<xsl:choose>
			<xsl:when test="$currentoutlineLevel = $precedingoutlineLevel">
				<xsl:call-template name="calc-heading-digit">
					<xsl:with-param name="value" select="$value + 1"/>
					<xsl:with-param name="currentoutlineLevel" select="$currentoutlineLevel"/>
					<xsl:with-param name="i" select="$i + 1"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="$currentoutlineLevel &lt; $precedingoutlineLevel">
				<xsl:call-template name="calc-heading-digit">
					<xsl:with-param name="value" select="$value"/>
					<xsl:with-param name="currentoutlineLevel" select="$currentoutlineLevel"/>
					<xsl:with-param name="i" select="$i + 1"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="$value"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<!-- Neglect Annotations -->
	<xsl:template match="office:annotation" mode="concatenate"/>

	<!-- Match text:placeholder child nodes (e.g. text) -->
	<xsl:template match="text:placeholder">
		<xsl:param name="globalData"/>

		<xsl:call-template name="apply-styles-and-content">
			<xsl:with-param name="globalData" select="$globalData"/>
		</xsl:call-template>
	</xsl:template>

	<!-- ************* -->
	<!-- *** Link  *** -->
	<!-- ************* -->

	<xsl:template match="text:a | draw:a">
		<xsl:param name="globalData"/>

		<xsl:call-template name="create-common-anchor-link">
			<xsl:with-param name="globalData" select="$globalData"/>
		</xsl:call-template>
	</xsl:template>


	<xsl:template name="create-common-anchor-link">
		<xsl:param name="globalData"/>

		<xsl:element name="a">
			<xsl:attribute name="href">
				<xsl:call-template name="create-href">
					<xsl:with-param name="href" select="@xlink:href"/>
				</xsl:call-template>
			</xsl:attribute>
			<xsl:call-template name="apply-styles-and-content">
				<xsl:with-param name="globalData" select="$globalData"/>
			</xsl:call-template>
		</xsl:element>
	</xsl:template>



	<!-- ******************* -->
	<!-- *** Image Link  *** -->
	<!-- ******************* -->

	<!-- currently suggesting that all draw:object-ole elements are images -->
	<xsl:template match="draw:image | draw:object-ole">
		<xsl:param name="globalData"/>

		<xsl:choose>
			<xsl:when test="parent::text:p or parent::text:span or parent::text:h or parent::draw:a or parent::text:a or text:ruby-base">
				<!-- XHTML does not allow the mapped elements to contain paragraphs -->
				<xsl:call-template name="create-image-element">
					<xsl:with-param name="globalData" select="$globalData"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:otherwise>
				<!-- images are embedded in a paragraph, but are in CSS not able to express a horizontal alignment for themself.
					A 'div' element taking over the image style would solve that problem, but is invalid as child of a paragraph -->
				<xsl:element name="p">
					<xsl:apply-templates select="@draw:style-name">
						<xsl:with-param name="globalData" select="$globalData"/>
					</xsl:apply-templates>

					<xsl:call-template name="create-image-element">
						<xsl:with-param name="globalData" select="$globalData"/>
					</xsl:call-template>
				</xsl:element>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name="create-image-element">
		<xsl:param name="globalData"/>

		<xsl:element name="img">
			<xsl:if test="../@svg:width">
				<xsl:attribute name="width">
					<xsl:call-template name="convert2px">
						<xsl:with-param name="value" select="../@svg:width"/>
					</xsl:call-template>
				</xsl:attribute>
			</xsl:if>
			<xsl:if test="../@svg:height">
				<xsl:attribute name="height">
					<xsl:call-template name="convert2px">
						<xsl:with-param name="value" select="../@svg:height"/>
					</xsl:call-template>
				</xsl:attribute>
			</xsl:if>
			<xsl:attribute name="alt">
				<xsl:choose>
					<xsl:when test="../svg:desc">
						<xsl:value-of select="../svg:desc"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:message>
	Accessibility Warning:
		 No alternate text ('svg:desc' element) set for
		 image '<xsl:value-of select="@xlink:href"/>'!</xsl:message>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:attribute>

			<xsl:attribute name="src">
				<xsl:call-template name="create-href">
					<xsl:with-param name="href" select="@xlink:href"/>
				</xsl:call-template>
			</xsl:attribute>

			<!-- style interpretation only, as no subelements are allowed for img in XHTML -->
			<xsl:apply-templates select="@draw:style-name">
				<xsl:with-param name="globalData" select="$globalData"/>
			</xsl:apply-templates>
		</xsl:element>
	</xsl:template>

	<!-- ************ -->
	<!-- *** list *** -->
	<!-- ************ -->
	<!--
		Some things have to be considered to create XHTML lists with a similar view as in the Office:

		A)
		One significant difference between XHTML and Office List elements is that an list element without text but only child lists
		would not shown a list symbol in the Office, but do in XHTML.

		Workaround:
			- Contiguous child elements of 'text:list' elements of the same list type will be taken as a single XHTML list item
			  In other words XHTML list element ('ol, 'ul' or 'li') will only be created if the ODF XML elements 'text:list' or
			'text:list-item' elements have a child different to them.

		B)
		In OASIS Open Document XML (OOo2.0) only one element exists for list items, during a mapping to OOo XML
		it is always mapped to a 'text:list' list element, even with it is a unordered list.

		Workaround:
			- XHTML element 'ol', 'ul' will not be choosen upon the 'text:list' element,
			but on the list style type.

		C)
		An Office Lists may be spread over the whole document. Linked by their style and text:continue-numbering='true'.
		Futhermore it can have any provided character as List label.
		Workaround:
			- comlete list emulation without XHTML list elements (currently neglected)
	-->
	<xsl:key name="listStyles" match="/*/office:styles/text:list-style | /*/office:automatic-styles/text:list-style" use="@style:name"/>

	<xsl:template match="text:list">
		<xsl:param name="globalData"/>
		<xsl:param name="listLevel" select="1"/>
		<xsl:param name="listStyleName" select="@text:style-name"/>
		<!-- The left margin is in XHTML messured till the beginning of the text in the Office to the beginning of the list symbol (label)
			a left-margin of 0 hides all list symbol. This assumption of an indent make them visible
			Note: 	instead of 0.5, which is the width of list symbol in 'cm', the constant &list-label-width; should be used,
					but problems occure with Xalan 2.4.1 used in JRE 1.4 -->
		<xsl:param name="listIndentTotalBefore" select="-0.5"/>

		<!-- get the list style, with the same 'text:style-name' and same 'text:level' -->
		<xsl:variable name="listLevelStyle" select="key('listStyles', $listStyleName)/*[@text:level = number($listLevel)]"/>
		<xsl:variable name="listType">
			<xsl:choose>
				<!-- ordered list -->
				<xsl:when test="name($listLevelStyle) = 'text:list-level-style-number'">
					<xsl:text>ol</xsl:text>
				</xsl:when>
				<!-- unordered list -->
				<xsl:otherwise>
					<xsl:text>ul</xsl:text>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<xsl:variable name="isEmptyList" select="not(*[name() = 'text:list-item'or name() = 'text:list-header']/*[not(name() = 'text:list-item') and not(name() = 'text:list-header') and not(name() = 'text:list') and not(name() = 'text:list')])"/>
		<xsl:variable name="listMargin">
			<xsl:variable name="minLabelWidth">
				<xsl:choose>
					<xsl:when test="$listLevelStyle/*/@text:min-label-width">
						<xsl:call-template name="convert2cm">
							<xsl:with-param name="value" select="$listLevelStyle/*/@text:min-label-width"/>
						</xsl:call-template>
					</xsl:when>
					<xsl:otherwise>0</xsl:otherwise>
				</xsl:choose>
			</xsl:variable>
			<xsl:variable name="minLabelDistance">
				<xsl:choose>
					<xsl:when test="$listLevelStyle/*/@text:min-label-distance">
						<xsl:call-template name="convert2cm">
							<xsl:with-param name="value" select="$listLevelStyle/*/@text:min-label-distance"/>
						</xsl:call-template>
					</xsl:when>
					<xsl:otherwise>0</xsl:otherwise>
				</xsl:choose>
			</xsl:variable>
			<xsl:choose>
				<xsl:when test="$minLabelDistance &gt; $minLabelWidth">
					<xsl:value-of select="$minLabelDistance"/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select="$minLabelWidth"/>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<xsl:variable name="listIndent">
			<xsl:choose>
				<xsl:when test="$listLevelStyle/*/@text:space-before">
					<xsl:call-template name="convert2cm">
						<xsl:with-param name="value" select="$listLevelStyle/*/@text:space-before"/>
					</xsl:call-template>
				</xsl:when>
				<xsl:otherwise>0</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<xsl:element name="{$listType}">
			<xsl:call-template name="create-list-style-attribute">
				<!-- the list is empty, if it just contains further lists. No element exist, than one of the three list elements -->
				<xsl:with-param name="isEmptyList" select="$isEmptyList"/>
				<xsl:with-param name="listLevelStyle" select="$listLevelStyle"/>
				<xsl:with-param name="listIndent" select="$listIndent - $listIndentTotalBefore"/>
				<xsl:with-param name="listIndentTotalBefore" select="$listIndentTotalBefore"/>
			</xsl:call-template>
			<xsl:apply-templates>
				<xsl:with-param name="globalData" select="$globalData"/>
				<xsl:with-param name="isEmptyList" select="$isEmptyList"/>
				<xsl:with-param name="listIndent" select="$listIndent - $listIndentTotalBefore"/>
				<xsl:with-param name="listIndentTotalBefore" select="$listIndentTotalBefore"/>
				<xsl:with-param name="listLevel" select="$listLevel"/>
				<xsl:with-param name="listLevelStyle" select="$listLevelStyle"/>
				<xsl:with-param name="listMargin" select="$listMargin"/>
				<xsl:with-param name="listStyleName" select="$listStyleName"/>
			</xsl:apply-templates>
		</xsl:element>
	</xsl:template>

	<xsl:template name="create-list-style-attribute">
		<xsl:param name="listLevelStyle"/>
		<xsl:param name="isEmptyList"/>
		<xsl:param name="listIndent"/>

		<xsl:if test="$isEmptyList or $listIndent or $listLevelStyle/@style:num-format">
			<xsl:attribute name="style">
				<!-- rounding the indent to 4 digits -->
				<xsl:if test="$listIndent">margin-left:<xsl:value-of select="round($listIndent * 4) div 4"/>cm;</xsl:if>
				<xsl:choose>
					<xsl:when test="$listLevelStyle/@style:num-format or $isEmptyList">
						<xsl:text>list-style-type:</xsl:text>
						<xsl:choose>
							<xsl:when test="$isEmptyList">none</xsl:when>
							<xsl:when test="$listLevelStyle/@style:num-format = '1'">
								<xsl:text>decimal</xsl:text>
							</xsl:when>
							<xsl:when test="$listLevelStyle/@style:num-format = 'I'">
								<xsl:text>upper-roman</xsl:text>
							</xsl:when>
							<xsl:when test="$listLevelStyle/@style:num-format = 'i'">
								<xsl:text>lower-roman</xsl:text>
							</xsl:when>
							<xsl:when test="$listLevelStyle/@style:num-format = 'A'">
								<xsl:text>upper-alpha</xsl:text>
							</xsl:when>
							<xsl:when test="$listLevelStyle/@style:num-format = 'a'">
								<xsl:text>lower-alpha</xsl:text>
							</xsl:when>
						</xsl:choose>
						<xsl:text>; </xsl:text>
					</xsl:when>
				</xsl:choose>
			</xsl:attribute>
		</xsl:if>
	</xsl:template>


	<!-- ****************** -->
	<!-- *** list item  *** -->
	<!-- ****************** -->
<!--
	Left margin of the complete list:
	The space between left page and the list symbol (left-margin) is in the Office implemented by
	the sum of three values:
		1) 'text:space-before', which is part of the 'text:list-style' element.
		2) 'margin:left' from the style of the first child (e.g. paragraph).
		3) 'fo:text-indent' the indent of the first line of some child (e.g. paragraph) (currently neglected)


	Different left-margin behaviors between Office and XHTML:

	a) margin in XHTML is messured till the beginning of the text in the Office
		till the label.
		As workaround the assumed list label width is added/subtracted.

	b) margin in XHTML of li, ol, ul elements is cumulative, in the Office absolute
		As workaround the earlier level value is substracted from the new given.

	c) In the Office the 'margin-left' of the first list child element (e.g. paragraph) is added
		as space BEFORE the list label.	In XHTML it would only result in an indent of
		the first paragraph after the list label.
		As workaround the list-item receives a margin-left from the first child with style

	Possible list children:
	<!ELEMENT text:list-item (text:p|text:h|text:list|text:list)+>

	In the Office the indent after the list label before the text depends on two attributes:
		- 'text:min-label-width': the distance between list label and all text of the list item.
		- 'text:min-label-distance': the distance between list label and text of the first line.
		As workaround the first child gets the higher margin of both as margin-left style.
-->
	<xsl:template match="text:list-item | text:list-header">
		<xsl:param name="isEmptyList"/>
		<xsl:param name="globalData"/>
		<xsl:param name="listIndent"/>
		<xsl:param name="listIndentTotalBefore"/>
		<xsl:param name="listLevel"/>
		<xsl:param name="listLevelStyle"/>
		<xsl:param name="listMargin"/>
		<xsl:param name="listStyleName"/>


		<xsl:variable name="firstElementStyleName" select="descendant::*[@text:style-name][1]/@text:style-name"/>
		<xsl:variable name="firstElementStyles" select="$globalData/all-styles/style[@style:name = $firstElementStyleName]/final-properties"/>
		<xsl:variable name="firstElementMargin">
			<xsl:choose>
				<xsl:when test="contains($firstElementStyles, 'margin-left:')">
					<xsl:call-template name="convert2cm">
						<xsl:with-param name="value" select="normalize-space(substring-before(substring-after($firstElementStyles, 'margin-left:'), ';'))"/>
					</xsl:call-template>
				</xsl:when>
				<xsl:otherwise>0</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>

		<xsl:element name="li">
			<!-- The left-margin of the first list child is being added to the whole list left-margin -->
			<xsl:call-template name="create-list-style">
				<xsl:with-param name="globalData" select="$globalData"/>
				<xsl:with-param name="listLevelStyle" select="$listLevelStyle"/>
				<xsl:with-param name="marginInCentimeter" select="$firstElementMargin"/>
				<xsl:with-param name="styleName" select="$firstElementStyleName"/>
			</xsl:call-template>
			<xsl:choose>
				<xsl:when test="$isEmptyList">
					<xsl:apply-templates>
						<xsl:with-param name="globalData" select="$globalData"/>
						<xsl:with-param name="listIndentTotalBefore" select="$listIndent + $firstElementMargin + $listIndentTotalBefore"/>
						<xsl:with-param name="listLevel" select="$listLevel + 1"/>
						<xsl:with-param name="listStyleName" select="$listStyleName"/>
					</xsl:apply-templates>
				</xsl:when>
				<xsl:otherwise>
					<xsl:apply-templates mode="first-element-margin-adaption" select="*">
						<xsl:with-param name="globalData" select="$globalData"/>
						<xsl:with-param name="listIndentTotalBefore" select="$listIndent + $firstElementMargin + $listIndentTotalBefore"/>
						<xsl:with-param name="marginInCentimeter" select="$listMargin"/>
						<xsl:with-param name="listLevel" select="$listLevel + 1"/>
						<xsl:with-param name="listStyleName" select="$listStyleName"/>
					</xsl:apply-templates>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:element>
	</xsl:template>

	<xsl:template match="text:p" mode="first-element-margin-adaption">
		<xsl:param name="globalData"/>
		<xsl:param name="marginInCentimeter"/>

		<xsl:element name="p">
			<!-- empty paragraph tags does not provoke an carridge return,
				therefore an non breakable space (&nbsp) have been inserted.-->
			<xsl:choose>
				<xsl:when test="*">
					<xsl:call-template name="create-list-style">
						<xsl:with-param name="globalData" select="$globalData"/>
						<!-- Note: 	instead of 0.5, which is the width of list symbol in 'cm', the constant &list-label-width; should be used,
									but problems occure with Xalan 2.4.1 used in JRE 1.4 -->
						<xsl:with-param name="marginInCentimeter" select="$marginInCentimeter - 0.5"/>
						<xsl:with-param name="styleName" select="@text:style-name"/>
					</xsl:call-template>
					<xsl:apply-templates>
						<xsl:with-param name="globalData" select="$globalData"/>
					</xsl:apply-templates>
				</xsl:when>
				<xsl:otherwise>
					<xsl:call-template name="create-list-style">
						<xsl:with-param name="globalData" select="$globalData"/>
						<!-- Note: 	instead of 0.5, which is the width of list symbol in 'cm', the constant &list-label-width; should be used,
									but problems occure with Xalan 2.4.1 used in JRE 1.4 -->
						<xsl:with-param name="marginInCentimeter" select="$marginInCentimeter - 0.5"/>
						<xsl:with-param name="styleName" select="@text:style-name"/>
					</xsl:call-template>
					<xsl:apply-templates>
						<xsl:with-param name="globalData" select="$globalData"/>
					</xsl:apply-templates>
					<xsl:text>&#160;</xsl:text>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:element>
	</xsl:template>


	<!-- Neglecting the left margin behavior for headings for now -->
	<xsl:template match="text:h" mode="first-element-margin-adaption">
		<xsl:param name="globalData"/>

		<xsl:apply-templates select="self::*">
			<xsl:with-param name="globalData" select="$globalData"/>
		</xsl:apply-templates>
	</xsl:template>


	<xsl:template match="*" mode="first-element-margin-adaption">
		<xsl:param name="globalData"/>
		<xsl:param name="listIndentTotalBefore"/>
		<xsl:param name="listLevel"/>
		<xsl:param name="listStyleName"/>

		<xsl:apply-templates select="self::*">
			<xsl:with-param name="globalData" select="$globalData"/>
			<xsl:with-param name="listIndentTotalBefore" select="$listIndentTotalBefore"/>
			<xsl:with-param name="listLevel" select="$listLevel"/>
			<xsl:with-param name="listStyleName" select="$listStyleName"/>
		</xsl:apply-templates>
	</xsl:template>


	<xsl:template match="text()" mode="first-element-margin-adaption">
		<xsl:value-of select="."/>
	</xsl:template>


	<xsl:template name="create-list-style">
		<xsl:param name="globalData"/>
		<xsl:param name="marginInCentimeter"/>
		<xsl:param name="styleName"/>

		<xsl:if test="$styleName">
			<xsl:attribute name="class">
				<xsl:value-of select="translate($styleName, '.,;: %()[]/\+', '_____________')"/>
			</xsl:attribute>
		</xsl:if>
		<xsl:attribute name="style">
			<xsl:text>margin-left:</xsl:text>
			<xsl:choose>
				<xsl:when test="$marginInCentimeter">
					<xsl:value-of select="round($marginInCentimeter * 4) div 4"/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:text>0</xsl:text>
				</xsl:otherwise>
			</xsl:choose>
			<xsl:text>cm;</xsl:text>
		</xsl:attribute>
	</xsl:template>



	<!-- ********************************************** -->
	<!-- *** Text Section (contains: draw:text-box) *** -->
	<!-- ********************************************** -->

	<xsl:template match="text:section">
		<xsl:param name="globalData"/>

		<xsl:if test="not(contains(@text:display, 'none'))">
			<xsl:element name="div">
				<xsl:call-template name="apply-styles-and-content">
					<xsl:with-param name="globalData" select="$globalData"/>
				</xsl:call-template>
			</xsl:element>
		</xsl:if>
	</xsl:template>


	<!-- Hidden text dependend on Office variables:
		 The text is not shown, if condition is 'true'.
		 Implemented solely for conditons as '<VARIABLE>==0' or '<VARIABLE>==1'
	-->
	<xsl:key match="text:variable-set" name="varSet" use="@text:name"/>
	<xsl:template match="text:hidden-text">
		<xsl:param name="globalData"/>

		<xsl:variable name="varName" select="substring-before(@text:condition, '==')"/>
		<xsl:variable name="varValue" select="substring-after(@text:condition, '==')"/>
		<xsl:choose>
			<xsl:when test="key('varSet', $varName)/@text:value != $varValue">
				<xsl:value-of select="@text:string-value"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:comment>
					<xsl:value-of select="$varName"/>
					<xsl:value-of select="@text:string-value"/>
					<xsl:value-of select="$varName"/>
				</xsl:comment>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>


	<xsl:template match="@text:style-name | @draw:style-name | @draw:text-style-name | @table:style-name"><!-- | @presentation:style-name-->
		<xsl:param name="globalData"/>

		<xsl:attribute name="class">
			<xsl:value-of select="translate(., '.,;: %()[]/\+', '_____________')"/>
		</xsl:attribute>
	</xsl:template>


	<!-- ***************** -->
	<!-- *** Footnotes *** -->
	<!-- ***************** -->

	<xsl:template match="text:note">
		<xsl:param name="globalData"/>

		<!-- get style configuration -->
		<xsl:variable name="footnoteConfig" select="$globalData/office:styles/text:notes-configuration[@text:note-class=current()/@text:note-class]" />

		<xsl:variable name="titlePrefix">
			<xsl:choose>
				<xsl:when test="@text:note-class = 'footnote'">
					<xsl:text>Footnote: </xsl:text>
				</xsl:when>
				<xsl:otherwise>
					<xsl:text>Endnote: </xsl:text>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>

		<!-- write anchor -->
		<xsl:element name="span">
			<xsl:attribute name="class"><xsl:value-of select="$footnoteConfig/@text:citation-body-style-name"/></xsl:attribute>
			<xsl:attribute name="title"><xsl:value-of select="$titlePrefix"/><xsl:apply-templates mode="textOnly" select="text:note-body"/></xsl:attribute>
			<xsl:element name="a">
				<xsl:attribute name="href"><xsl:value-of select="concat('#', @text:id)"/></xsl:attribute>
				<xsl:attribute name="id"><xsl:value-of select="concat('body_', @text:id)"/></xsl:attribute>
				<xsl:apply-templates  mode="textOnly" select="text:note-citation"/>
			</xsl:element>
		</xsl:element>
	</xsl:template>

	<xsl:template match="*" mode="textOnly">
		<xsl:apply-templates select="* | text()" mode="textOnly" />
	</xsl:template>

	<xsl:template match="text()" mode="textOnly">
		<xsl:value-of select="."/>
	</xsl:template>

	<!-- Useful in case of 'style:map', conditional formatting, where a style references to another -->
	<xsl:key name="textNotes" match="text:note" use="@text:note-class"/>

	<!-- writing the footer- and endnotes beyond the body -->
	<xsl:template name="write-text-nodes">
		<xsl:param name="globalData"/>

		<!-- write footnote body -->
		<xsl:for-each select="key('textNotes', 'footnote')">
			<xsl:call-template name="write-text-node">
				<xsl:with-param name="globalData" select="$globalData"/>
				<xsl:with-param name="footnoteConfig" select="$globalData/office:styles/text:notes-configuration[@text:note-class=current()/@text:note-class]" />
			</xsl:call-template>
		</xsl:for-each>

		<!-- write endnote body -->
		<xsl:for-each select="key('textNotes', 'endnote')">
			<xsl:call-template name="write-text-node">
				<xsl:with-param name="globalData" select="$globalData"/>
				<xsl:with-param name="footnoteConfig" select="$globalData/office:styles/text:notes-configuration[@text:note-class=current()/@text:note-class]" />
			</xsl:call-template>
		</xsl:for-each>
	</xsl:template>

	<xsl:template name="write-text-node">
		<xsl:param name="globalData"/>
		<xsl:param name="footnoteConfig"/>

		<xsl:apply-templates select="text:note-body/node()">
				<xsl:with-param name="globalData" select="$globalData" />
				<xsl:with-param name="footnotePrefix">
					<xsl:element name="a">
						<xsl:attribute name="class"><xsl:value-of select="$footnoteConfig/@text:citation-style-name"/></xsl:attribute>
						<xsl:attribute name="id"><xsl:value-of select="@text:id"/></xsl:attribute>
						<xsl:attribute name="href"><xsl:value-of select="concat('#body_', @text:id)"/></xsl:attribute>
						<xsl:apply-templates  mode="textOnly" select="text:note-citation"/>
					</xsl:element>
				</xsl:with-param>
		</xsl:apply-templates>
	</xsl:template>


	<!-- tabulator -->
	<xsl:template match="text:tab">
		<xsl:param name="globalData"/>

		<xsl:element name="span">
			<!-- using as heuristic the first tab width of the 0paragraph style as margin-left -->
			<xsl:attribute name="style">margin-left:<xsl:value-of select="$globalData/all-doc-styles/style[@style:name = current()/ancestor::*/@text:style-name]/*/style:tab-stops/style:tab-stop/@style:position"/>;</xsl:attribute>
		</xsl:element>

	</xsl:template>

</xsl:stylesheet>
