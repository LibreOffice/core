<?xml version="1.0" encoding="UTF-8"?>
<!--

  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

  Copyright 2008 by Sun Microsystems, Inc.

  OpenOffice.org - a multi-platform office productivity suite

  $RCSfile: body.xsl,v $

  $Revision: 1.3 $

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
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:chart="urn:oasis:names:tc:opendocument:xmlns:chart:1.0" xmlns:config="urn:oasis:names:tc:opendocument:xmlns:config:1.0" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dom="http://www.w3.org/2001/xml-events" xmlns:dr3d="urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0" xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" xmlns:form="urn:oasis:names:tc:opendocument:xmlns:form:1.0" xmlns:math="http://www.w3.org/1998/Math/MathML" xmlns:meta="urn:oasis:names:tc:opendocument:xmlns:meta:1.0" xmlns:number="urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0" xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" xmlns:ooo="http://openoffice.org/2004/office" xmlns:oooc="http://openoffice.org/2004/calc" xmlns:ooow="http://openoffice.org/2004/writer" xmlns:script="urn:oasis:names:tc:opendocument:xmlns:script:1.0" xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" xmlns:xforms="http://www.w3.org/2002/xforms" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" exclude-result-prefixes="chart config dc dom dr3d draw fo form math meta number office ooo oooc ooow script style svg table text xforms xlink xsd xsi" xmlns="http://www.w3.org/1999/xhtml">



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

			<!-- adapt page size -->
			<xsl:variable name="pageWidth" select="$pageProperties/@fo:page-width"/>

			<!-- multiple backgroundimages for different page styles (never used in html) -->
			<xsl:variable name="backgroundImage" select="$pageProperties/style:background-image"/>

			<!-- page margins & background image  -->
			<xsl:if test="$pageWidth or $pageProperties/@fo:* or $backgroundImage/@xlink:href">
				<xsl:attribute name="style">
					<xsl:if test="$pageWidth">
						 <xsl:text>max-width:</xsl:text><xsl:value-of select="$pageWidth"/><xsl:text>;</xsl:text>
					</xsl:if>
					<xsl:if test="$pageProperties/@fo:* or $backgroundImage/@xlink:href">
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

	<xsl:template match="text:user-field-get | text:user-field-input">
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
						<!-- write number prefix -->
						<xsl:value-of select="$globalData/office:styles/text:outline-style/text:outline-level-style[@text:level = current()/@text:outline-level]/@style:num-prefix"/>
						<xsl:call-template name="write-heading-number">
							<xsl:with-param name="globalData" select="$globalData"/>
						</xsl:call-template>
						<!-- write number suffix -->
						<xsl:value-of select="$globalData/office:styles/text:outline-style/text:outline-level-style[@text:level = current()/@text:outline-level]/@style:num-suffix"/>
					</xsl:variable>
					<xsl:call-template name="create-heading-anchor">
						<xsl:with-param name="globalData" select="$globalData"/>
						<xsl:with-param name="headingNumber" select="$headingNumber"/>
					</xsl:call-template>
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

		<xsl:variable name="title">
			<xsl:apply-templates mode="concatenate"/>
		</xsl:variable>
		<xsl:element namespace="{$namespace}" name="a">
			<xsl:attribute name="name">
				<xsl:value-of select="translate(concat($headingNumber, '_', $title), '.,;: %()[]/\+', '_____________')"/>
			</xsl:attribute>

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
	Find the correct heading no., which is the sum of 'text:start-value'
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
					</xsl:with-param><xsl:with-param name="numberFormat" select="$globalData/office:styles/text:outline-style/text:outline-level-style[@text:level = $outlineLevel]/@style:num-format"/>
					<xsl:with-param name="last" select="true()"/>
				</xsl:call-template>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name="writeNumber">
		<xsl:param name="numberDigit"/>
		<xsl:param name="numberFormat"/>
		<xsl:param name="last"/>

		<xsl:choose>
			<xsl:when test="not($numberFormat)">
				<xsl:number value="$numberDigit" format="1."/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:choose>
					<xsl:when test="$last">
						<xsl:number value="$numberDigit" format="{$numberFormat}"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:number value="$numberDigit" format="{$numberFormat}."/>
					</xsl:otherwise>
				</xsl:choose>
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
		Due to the requirements below the ODF list functionality is not handled by CSS, but the list labels calculated and written by XSLT.

		REQUIREMENTS:
		=============

		A)
		One significant difference between XHTML and Office List elements is that a list without text nodes but only further list children
		would not show a list symbol in the Office, but in the browser from XHTML.

		B)
		Since OASIS Open Document XML (implemented in OOo2.0) only one parent type exists for list items
		the 'text:list' element. The XHTML element 'ol', 'ul' will be choosen upon the list style type.

		C)
		An Office list may be spread over the whole document. Linked by their style and text:continue-numbering='true'.

		D)
		An Office list can use characters or images as list label.

		E)
		An Office list can have a prefix and suffix around the list label.

		F)
		An Office list style may have the attribute consecutive numbering, which resolves in a list counting for all levels

		G)
		An Office list may (re)start on any arbitrary value by using @text:start-value on the text:list-item

		INDENDATION:
		============

		The indent of a list label is not only calculated by using the text:space-before of the list level (listLevelStyle), but
		as well taking the left margin of the first paragraph (or heading) of the list into account as long it is not negative.

		|           MARGIN LEFT                 |        LABEL           |

		|   text:space-before (listlevelstyle)  | text:min-label-width   |
		| + fo:left-margin (firstParagraph)     |                        |

		Further details beyond text:list-list..
	-->
	<xsl:key name="listStyles" match="/*/office:styles/text:list-style | /*/office:automatic-styles/text:list-style  |
									  /*/office:styles/style:graphic-properties/text:list-style | /*/office:automatic-styles/style:graphic-properties/text:list-style" use="@style:name"/>

	<!--
		A text list may only have text:list-item and text:list-header as children.
	-->
	<xsl:template match="text:list">
		<xsl:param name="globalData"/>
		<xsl:param name="isListNumberingReset"/>
		<xsl:param name="isNextLevelNumberingReset"/>
		<xsl:param name="listLevel" select="1"/>
		<xsl:param name="listRestart" select="false()"/>
		<xsl:param name="itemLabel" select="''"/>
		<xsl:param name="listStyle"/>
		<xsl:param name="listStyleName" select="@text:style-name"/>

		<!-- To choose list type - get the list style, with the same 'text:style-name' and same 'text:level' -->
		<xsl:variable name="listStyle" select="key('listStyles', $listStyleName)"/>
		<xsl:variable name="listLevelStyle" select="$listStyle/*[@text:level = number($listLevel)]"/>
		<xsl:variable name="listIndent">
			<xsl:call-template name="getListIndent">
				<xsl:with-param name="globalData" select="$globalData"/>
				<xsl:with-param name="listLevelStyle" select="$listLevelStyle"/>
				<xsl:with-param name="firstPara" select="*[1]/*[name() = 'text:p' or name() = 'text:h'][1]"/>
			</xsl:call-template>
		</xsl:variable>
		<xsl:variable name="isEmptyList" select="not(*[1]/*[name() = 'text:h' or name() = 'text:p'])"/>
		<xsl:variable name="listType">
			<xsl:choose>
				<!-- ordered list -->
				<xsl:when test="name($listLevelStyle) = 'text:list-level-style-number'">
					<xsl:text>ol</xsl:text>
				</xsl:when>
				<!-- unordered list (bullet or image) -->
				<xsl:otherwise>
					<xsl:text>ul</xsl:text>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<xsl:element name="{$listType}">
			<xsl:apply-templates select="*[1]" mode="listItemSibling">
				<xsl:with-param name="globalData" select="$globalData"/>
				<xsl:with-param name="isEmptyList" select="$isEmptyList"/>
				<xsl:with-param name="isListNumberingReset" select="$isNextLevelNumberingReset"/>
				<xsl:with-param name="isNextLevelNumberingReset">
					<xsl:choose>
						<xsl:when test="$isListNumberingReset">
							<xsl:value-of select="true()"/>
						</xsl:when>
						<xsl:otherwise>
							<!-- A list is empty if a text:list does not have a text:list-header or text:list-item (wildcard as only those can exist beyond a text:list), which contains a text:h or text:p -->
							<xsl:value-of select="not($isEmptyList)"/>
						</xsl:otherwise>
					</xsl:choose>
				</xsl:with-param>
				<xsl:with-param name="itemLabel" select="$itemLabel"/>
				<xsl:with-param name="listIndent" select="$listIndent"/>
				<xsl:with-param name="listLevel" select="$listLevel"/>
				<xsl:with-param name="listLevelStyle" select="$listLevelStyle"/>
				<xsl:with-param name="listRestart">
					  <xsl:choose>
						<xsl:when test="$listRestart">
							<xsl:value-of select="$listRestart"/>
						</xsl:when>
						<xsl:otherwise>
							<!-- descdendants restart their list numbering, when an ancestor is not empty -->
							<xsl:value-of select="not($isEmptyList)"/>
						</xsl:otherwise>
					</xsl:choose>
				</xsl:with-param>
				<xsl:with-param name="listStyle" select="$listStyle"/>
				<xsl:with-param name="listStyleName" select="$listStyleName"/>
				<xsl:with-param name="minLabelDist">
					<xsl:choose>
						<xsl:when test="$listLevelStyle/*/@text:min-label-distance">
							<xsl:call-template name="convert2cm">
								<xsl:with-param name="value" select="$listLevelStyle/*/@text:min-label-distance"/>
							</xsl:call-template>
						</xsl:when>
						<xsl:otherwise>0</xsl:otherwise>
					</xsl:choose>
				</xsl:with-param>
				<xsl:with-param name="minLabelWidth">
					<xsl:choose>
						<xsl:when test="$listLevelStyle/*/@text:min-label-width">
							<xsl:call-template name="convert2cm">
								<xsl:with-param name="value" select="$listLevelStyle/*/@text:min-label-width"/>
							</xsl:call-template>
						</xsl:when>
						<xsl:otherwise>0</xsl:otherwise>
					</xsl:choose>
				</xsl:with-param>
			</xsl:apply-templates>
		</xsl:element>
	</xsl:template>

	<!-- See comment before text:list template -->
	<xsl:template name="getListIndent">
		<xsl:param name="globalData"/>
		<xsl:param name="listLevelStyle"/>
		<!-- The first paragraph of the list item (heading is special paragraph in ODF) -->
		<xsl:param name="firstPara" />

		<!-- Styles of first paragraph in list item, including ancestor styles (inheritance) -->
		<xsl:variable name="firstParaStyles" select="$globalData/all-styles/style[@style:name = $firstPara/@text:style-name]/final-properties"/>

		<!-- Only the left margin of the first paragraph of a list item will be added to the margin of the complete list (all levels)-->
		<xsl:variable name="firstParaLeftMargin">
			<xsl:choose>
				<xsl:when test="contains($firstParaStyles, 'margin-left:')">
					<xsl:call-template name="convert2cm">
						<xsl:with-param name="value" select="normalize-space(substring-before(substring-after($firstParaStyles, 'margin-left:'), ';'))"/>
					</xsl:call-template>
				</xsl:when>
				<xsl:otherwise>0</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<xsl:variable name="spaceBefore">
			<xsl:choose>
				<xsl:when test="$listLevelStyle/*/@text:space-before">
					<xsl:call-template name="convert2cm">
						<xsl:with-param name="value" select="$listLevelStyle/*/@text:space-before"/>
					</xsl:call-template>
				</xsl:when>
				<xsl:otherwise>0</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<!-- Only if the left-margin of the first paragraph is positive the sum
		text:space-before and fo:left-margin is taken as list indent -->
		<xsl:choose>
			<xsl:when test="$firstParaLeftMargin &gt; 0">
				<xsl:value-of select="$firstParaLeftMargin + $spaceBefore"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="$spaceBefore"/>
			</xsl:otherwise>
		</xsl:choose>
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
		3) 'fo:text-indent' the indent of the first line of some child (e.g. paragraph) (applied by CSS class style)

	Possible list children:
	<!ELEMENT text:list-item (text:p|text:h|text:list|text:list)+>

	In the Office the list label before the text depends on two attributes:
		- 'text:min-label-width': the distance between list label and all text of the list item.
		- 'text:min-label-distance': the distance between list label and text of the first line,
			only used, when text does not fit in text:min-label-width (ignored)

-->
	<xsl:template match="text:list-item | text:list-header" mode="listItemSibling">
		<xsl:param name="globalData"/>
		<xsl:param name="firstitemLabelWidth"/>
		<xsl:param name="isEmptyList" select="not(*[name() = 'text:h' or name() = 'text:p'])"/>
		<xsl:param name="isListNumberingReset"/>
		<xsl:param name="isNextLevelNumberingReset"/>
		<xsl:param name="itemNumber"/>
		<xsl:param name="itemLabel"/>
		<xsl:param name="listLevel"/>
		<xsl:param name="listLevelStyle"/>
		<xsl:param name="listRestart"/>
		<xsl:param name="listStyle"/>
		<xsl:param name="listStyleName"/>
		<xsl:param name="minLabelDist"/>
		<xsl:param name="minLabelWidth"/>
		<xsl:param name="listIndent" />


		<xsl:variable name="listIndentNew">
			<xsl:choose>
				<xsl:when test="$listIndent">
					<xsl:value-of select="$listIndent"/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:call-template name="getListIndent">
						<xsl:with-param name="globalData" select="$globalData"/>
						<xsl:with-param name="listLevelStyle" select="$listLevelStyle"/>
						<xsl:with-param name="firstPara" select="*[name() = 'text:p' or name() = 'text:h'][1]" />
					</xsl:call-template>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<xsl:variable name="itemNumberNew">
			<xsl:if test="$listStyle/text:list-level-style-number">
				<xsl:choose>
					<xsl:when test="$isEmptyList">
						<!--  An empty list item (no text:h/text:p as child), will not count as item and does not increment the count.  -->
						<xsl:variable name="tempItemNumber">
							<xsl:choose>
								<!-- siblings will be incremented by one -->
								<xsl:when test="$itemNumber">
									<xsl:value-of select="$itemNumber + 1"/>
								</xsl:when>
								<!-- if a higher list level had content the numbering starts with 1 -->
								<xsl:when test="$isListNumberingReset and $listLevel &gt; 1">
									<xsl:value-of select="1"/>
								</xsl:when>
								<xsl:otherwise>
									<xsl:call-template name="getItemNumber">
										<xsl:with-param name="isEmptyList" select="$isEmptyList"/>
										<xsl:with-param name="isListNumberingReset" select="$isListNumberingReset"/>
										<xsl:with-param name="itemNumber" select="$itemNumber"/>
										<xsl:with-param name="listStyleName" select="$listStyleName"/>
										<xsl:with-param name="listLevel" select="$listLevel"/>
										<xsl:with-param name="listLevelStyle" select="$listLevelStyle"/>
										<xsl:with-param name="listStyle" select="$listStyle"/>
									</xsl:call-template>
								</xsl:otherwise>
							</xsl:choose>
						</xsl:variable>
						<xsl:choose>
							<!-- in case the empty list-item is the first list-item in document -->
							<xsl:when test="$tempItemNumber = 1">
								<xsl:value-of select="1"/>
							</xsl:when>
							<xsl:otherwise>
								<xsl:value-of select="$tempItemNumber - 1"/>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:when>
					<xsl:otherwise>
						<xsl:choose>
							<xsl:when test="@text:start-value">
								<xsl:value-of select="@text:start-value"/>
							</xsl:when>
							<xsl:when test="$listLevelStyle/@text:start-value">
								<xsl:value-of select="$listLevelStyle/@text:start-value"/>
							</xsl:when>
							<!-- siblings will be incremented by one -->
							<xsl:when test="$itemNumber">
								<xsl:value-of select="$itemNumber + 1"/>
							</xsl:when>
							<!-- if a higher list level had content the numbering starts with 1 -->
							<xsl:when test="$isListNumberingReset and $listLevel &gt; 1">
								<xsl:value-of select="1"/>
							</xsl:when>
							<xsl:otherwise>
								<xsl:call-template name="getItemNumber">
									<xsl:with-param name="isEmptyList" select="$isEmptyList"/>
									<xsl:with-param name="isListNumberingReset" select="$isListNumberingReset"/>
									<xsl:with-param name="itemNumber" select="$itemNumber"/>
									<xsl:with-param name="listStyleName" select="$listStyleName"/>
									<xsl:with-param name="listLevel" select="$listLevel"/>
									<xsl:with-param name="listLevelStyle" select="$listLevelStyle"/>
									<xsl:with-param name="listStyle" select="$listStyle"/>
								</xsl:call-template>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:if>
		</xsl:variable>
		<xsl:variable name="itemLabelNew">
		   <xsl:if test="$listStyle/text:list-level-style-number">
				<!--
					A numbered label (e.g. 2.C.III) is created for every text:list-item/header.
					Above list levels are listed in the label, if the list-style requires this. Levels are separated by a '.'
					Formatation is dependent for every list level depth.
					The label is passed from anchestor text:list-item/header and if requrired truncated.
					The prefix/suffix (as well list level dependent) comes before and after the complete label (after truncation)
				-->
				<!-- Numbered label will be generated  -->
				<xsl:call-template name="createItemLabel">
					<xsl:with-param name="itemNumber" select="$itemNumberNew"/>
					<xsl:with-param name="itemLabel" select="$itemLabel"/>
					<xsl:with-param name="listLevelsToDisplay">
						<xsl:variable name="display" select="$listLevelStyle/@text:display-levels"/>
						<xsl:choose>
							<xsl:when test="$display">
								<xsl:value-of select="$display"/>
							</xsl:when>
							<xsl:otherwise>1</xsl:otherwise>
						</xsl:choose>
					</xsl:with-param>
					<xsl:with-param name="listLevel" select="$listLevel"/>
					<xsl:with-param name="listStyleName" select="$listStyleName"/>
					<xsl:with-param name="listLevelStyle" select="$listLevelStyle"/>
					<xsl:with-param name="listStyle" select="$listStyle"/>
				</xsl:call-template>
			</xsl:if>
		</xsl:variable>
		<xsl:element name="li">
			<xsl:choose>
				<xsl:when test="$isEmptyList">
					<xsl:apply-templates>
						<xsl:with-param name="globalData" select="$globalData"/>
						<xsl:with-param name="itemLabel" select="$itemLabelNew"/>
						<xsl:with-param name="listLevel" select="$listLevel + 1"/>
						<xsl:with-param name="listStyleName" select="$listStyleName"/>
					</xsl:apply-templates>
				</xsl:when>
				<xsl:otherwise>
					<xsl:variable name="listLabelElement">
				<!-- Numbering is being done by this transformation creating a HTML span representing the number label
					 The html:span represents the list item/header label (e.g. 1.A.III)
					 As the html:span is usually a inline element is formated by CSS as block element to use width upon it,
					 to disable the caridge return float:left is used and later neglected -->
					<span class="{$listLevelStyle/@text:style-name}"  style="display:block;float:left;min-width:{$minLabelWidth}cm;">
						<xsl:choose>
							<xsl:when test="$listStyle/text:list-level-style-bullet">
								<xsl:value-of select="$listLevelStyle/@style:num-prefix"/>
								<xsl:value-of select="$listStyle/text:list-level-style-bullet/@text:bullet-char"/>
								<xsl:value-of select="$listLevelStyle/@style:num-suffix"/>
							</xsl:when>
							<xsl:when test="$listStyle/text:list-level-style-number">
								<xsl:value-of select="$listLevelStyle/@style:num-prefix"/>
								<xsl:value-of select="$itemLabelNew"/>
								<xsl:value-of select="$listLevelStyle/@style:num-suffix"/>
							</xsl:when>
							<xsl:otherwise>
								<!-- Listing with image as bullets, taken from the list style's href -->
								<xsl:value-of select="$listStyle/text:list-level-style-image/@xlink:href"/>
							</xsl:otherwise>
						</xsl:choose>
					</span>
					</xsl:variable>
					<!-- Maybe the following children: text:h, text:p, list:text, text:soft-page-break -->
					<xsl:apply-templates mode="first-list-child-margin-adaption" select="*[1]">
						<xsl:with-param name="globalData" select="$globalData"/>
						<xsl:with-param name="isEmptyList" select="$isEmptyList"/>
						<xsl:with-param name="isNextLevelNumberingReset" select="$isNextLevelNumberingReset"/>
						<!-- The new created label is given to the children -->
						<xsl:with-param name="itemLabel" select="$itemLabelNew"/>
						<xsl:with-param name="listLabelElement" select="$listLabelElement"/>
						<xsl:with-param name="listLevel" select="$listLevel + 1"/>
						<xsl:with-param name="listLevelStyle" select="$listLevelStyle"/>
						<xsl:with-param name="listRestart" select="$listRestart"/>
						<xsl:with-param name="listStyle" select="$listStyle"/>
						<xsl:with-param name="listStyleName" select="$listStyleName"/>
						<xsl:with-param name="listIndent" select="$listIndentNew"/>
						<xsl:with-param name="minLabelWidth" select="$minLabelWidth"/>
					</xsl:apply-templates>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:element>
		<xsl:apply-templates select="following-sibling::*[1]" mode="listItemSibling">
			<xsl:with-param name="globalData" select="$globalData"/>
			<xsl:with-param name="isListNumberingReset" select="$isListNumberingReset"/>
			<xsl:with-param name="isNextLevelNumberingReset" select="$isNextLevelNumberingReset"/>
			<xsl:with-param name="itemNumber" select="$itemNumberNew"/>
			<xsl:with-param name="listIndent">
				<xsl:choose>
					<xsl:when test="not($isEmptyList)"><xsl:value-of select="$listIndentNew"/></xsl:when>
				</xsl:choose>
			</xsl:with-param>
			<!-- Receives the same parent label -->
			<xsl:with-param name="itemLabel" select="$itemLabel"/>
			<xsl:with-param name="listLevel" select="$listLevel"/>
			<xsl:with-param name="listLevelStyle" select="$listLevelStyle"/>
			<xsl:with-param name="listStyle" select="$listStyle"/>
			<xsl:with-param name="listStyleName" select="$listStyleName"/>
			<xsl:with-param name="minLabelDist" select="$minLabelDist"/>
			<xsl:with-param name="minLabelWidth" select="$minLabelWidth"/>
		</xsl:apply-templates>
	</xsl:template>


	<!-- Each key element holds the set of all text:list-item/text:list-header of a certain level and a certain style -->
	<xsl:key name="getListItemsByLevelAndStyle" use="concat(count(ancestor::text:list), ancestor::text:list/@text:style-name)" match="text:list-item | text:list-header"/>
	<!-- Each key element holds the set of all text:list-item/text:list-header of a certain style -->
	<xsl:key name="getListItemsByStyle" use="ancestor::text:list/@text:style-name" match="text:list-item | text:list-header"/>


	<!-- The Numbering start value (or offset from regular counteing) is used at the first item of offset,
	but have to be reused on following item/headers with no text:start-value -->
	<xsl:template name="getItemNumber">
		<xsl:param name="listLevel"/>
		<xsl:param name="listLevelStyle"/>
		<xsl:param name="listStyleName"/>
		<xsl:param name="listStyle"/>

		<xsl:call-template name="countListItemTillStartValue">
			<xsl:with-param name="listLevel" select="$listLevel"/>
			<xsl:with-param name="listLevelStyle" select="$listLevelStyle"/>
			<xsl:with-param name="listStyleName" select="$listStyleName"/>
			<xsl:with-param name="listStyle" select="$listStyle"/>
			<xsl:with-param name="precedingListItemsOfSameLevelAndStyle" select="preceding::text:list-item[generate-id(key('getListItemsByLevelAndStyle', concat($listLevel, $listStyleName))) =
																							   generate-id(key('getListItemsByLevelAndStyle', concat(count(ancestor::text:list), ancestor::text:list/@text:style-name)))]"/>
			<xsl:with-param name="precedingListItemsOfSameStyle" select="preceding::text:list-item[generate-id(key('getListItemsByStyle', $listStyleName)) =
																							   generate-id(key('getListItemsByStyle', ancestor::text:list/@text:style-name))]"/>
		</xsl:call-template>
	</xsl:template>

	<!-- When there is a text:start-value the last have to be found and added to the number -->
	<xsl:template name="countListItemTillStartValue">
		<xsl:param name="IteratorSameLevelAndStyle" select="1"/>
		<xsl:param name="IteratorSameStyle" select="1"/>
		<xsl:param name="itemNumber" select="1"/>
		<xsl:param name="listLevel"/>
		<xsl:param name="listLevelStyle"/>
		<xsl:param name="listStyle"/>
		<xsl:param name="listStyleName"/>
		<xsl:param name="precedingListItemsOfSameLevelAndStyle" />
		<xsl:param name="precedingListItemsOfSameLevelAndStyleCount" select="count($precedingListItemsOfSameLevelAndStyle)"/>
		<xsl:param name="precedingListItemsOfSameStyle" />
		<xsl:param name="precedingListItemsOfSameStyleCount" select="count($precedingListItemsOfSameStyle)"/>
		<!-- E.g.: If a list level 2 number is searched, a level 3 with content found with only a level 1 parent with content,
			the level 3 gets a 'pseudoLevel' -->
		<xsl:param name="pseudoLevel" select="0" />

		<!-- set the next of preceding list items. Starting from the current to the next previous text:list-item -->
		<xsl:variable name="precedingListItemOfSameLevelAndStyle" select="$precedingListItemsOfSameLevelAndStyle[$precedingListItemsOfSameLevelAndStyleCount - $IteratorSameLevelAndStyle + 1]"/>
		<xsl:variable name="precedingListItemOfSameStyle" select="$precedingListItemsOfSameStyle[$precedingListItemsOfSameStyleCount - $IteratorSameStyle + 1]"/>
		<xsl:choose>
			<xsl:when test="($precedingListItemOfSameStyle and $precedingListItemOfSameLevelAndStyle) or ($precedingListItemOfSameStyle and $listStyle/@text:consecutive-numbering)">
				<xsl:for-each select="$precedingListItemOfSameStyle">
					<xsl:choose>
						<!-- if it is a higher list level element  -->
						<xsl:when test="$listStyle/@text:consecutive-numbering">

							<xsl:call-template name="countListItem">
								<xsl:with-param name="IteratorSameLevelAndStyle" select="$IteratorSameLevelAndStyle" />
								<xsl:with-param name="IteratorSameStyle" select="$IteratorSameStyle"/>
								<xsl:with-param name="itemNumber" select="$itemNumber"/>
								<xsl:with-param name="listLevel" select="$listLevel"/>
								<xsl:with-param name="listLevelStyle" select="$listLevelStyle"/>
								<xsl:with-param name="listStyle" select="$listStyle"/>
								<xsl:with-param name="precedingListItemsOfSameLevelAndStyle" select="$precedingListItemsOfSameLevelAndStyle"/>
								<xsl:with-param name="precedingListItemsOfSameLevelAndStyleCount" select="$precedingListItemsOfSameLevelAndStyleCount"/>
								<xsl:with-param name="precedingListItemsOfSameStyle" select="$precedingListItemsOfSameStyle"/>
								<xsl:with-param name="precedingListItemsOfSameStyleCount" select="$precedingListItemsOfSameStyleCount"/>
								<xsl:with-param name="pseudoLevel" select="$pseudoLevel" />
							</xsl:call-template>
						</xsl:when>
						<xsl:otherwise>
							<!-- NOT CONSECUTIVE NUMBERING -->
							<xsl:variable name="currentListLevel" select="count(ancestor::text:list)"/>
							<xsl:choose>
								<!-- IF IT IS A HIGHER LIST LEVEL ELEMENT -->
								<xsl:when test="$currentListLevel &lt; $listLevel">
									<xsl:choose>
										<!-- if it has content the counting is ended -->
										<xsl:when test="*[name() = 'text:h' or name() = 'text:p']">
											<!-- 2DO: Perhaps the children still have to be processed -->
											<xsl:value-of select="$itemNumber + $pseudoLevel"/>
										</xsl:when>
										<xsl:otherwise>
										<!-- if it is empty the counting continues -->
											<xsl:call-template name="countListItemTillStartValue">
												<xsl:with-param name="IteratorSameLevelAndStyle" select="$IteratorSameLevelAndStyle" />
												<xsl:with-param name="IteratorSameStyle" select="$IteratorSameStyle + 1"/>
												<xsl:with-param name="itemNumber" select="$itemNumber"/>
												<xsl:with-param name="listLevel" select="$listLevel"/>
												<xsl:with-param name="listLevelStyle" select="$listLevelStyle"/>
												<xsl:with-param name="listStyle" select="$listStyle"/>
												<xsl:with-param name="precedingListItemsOfSameLevelAndStyle" select="$precedingListItemsOfSameLevelAndStyle"/>
												<xsl:with-param name="precedingListItemsOfSameLevelAndStyleCount" select="$precedingListItemsOfSameLevelAndStyleCount"/>
												<xsl:with-param name="precedingListItemsOfSameStyle" select="$precedingListItemsOfSameStyle"/>
												<xsl:with-param name="precedingListItemsOfSameStyleCount" select="$precedingListItemsOfSameStyleCount"/>
												<xsl:with-param name="pseudoLevel" select="$pseudoLevel" />
											</xsl:call-template>
										</xsl:otherwise>
									</xsl:choose>
								</xsl:when>
								<!-- IF IT IS A LIST LEVEL ELEMENT OF THE COUNTING LEVEL -->
								<xsl:when test="$currentListLevel = $listLevel">
									<xsl:call-template name="countListItem">
										<xsl:with-param name="IteratorSameLevelAndStyle" select="$IteratorSameLevelAndStyle" />
										<xsl:with-param name="IteratorSameStyle" select="$IteratorSameStyle"/>
										<xsl:with-param name="itemNumber" select="$itemNumber"/>
										<xsl:with-param name="listLevel" select="$listLevel"/>
										<xsl:with-param name="listLevelStyle" select="$listLevelStyle"/>
										<xsl:with-param name="listStyle" select="$listStyle"/>
										<xsl:with-param name="precedingListItemsOfSameLevelAndStyle" select="$precedingListItemsOfSameLevelAndStyle"/>
										<xsl:with-param name="precedingListItemsOfSameLevelAndStyleCount" select="$precedingListItemsOfSameLevelAndStyleCount"/>
										<xsl:with-param name="precedingListItemsOfSameStyle" select="$precedingListItemsOfSameStyle"/>
										<xsl:with-param name="precedingListItemsOfSameStyleCount" select="$precedingListItemsOfSameStyleCount"/>
										<xsl:with-param name="pseudoLevel" select="$pseudoLevel" />
									</xsl:call-template>
								</xsl:when>
								<xsl:otherwise>
									<!-- list item below the current level does not count -->
									<xsl:call-template name="countListItemTillStartValue">
										<xsl:with-param name="IteratorSameLevelAndStyle" select="$IteratorSameLevelAndStyle" />
										<xsl:with-param name="IteratorSameStyle" select="$IteratorSameStyle + 1"/>
										<xsl:with-param name="itemNumber" select="$itemNumber"/>
										<xsl:with-param name="listLevel" select="$listLevel"/>
										<xsl:with-param name="listLevelStyle" select="$listLevelStyle"/>
										<xsl:with-param name="listStyle" select="$listStyle"/>
										<xsl:with-param name="listStyleName" select="$listStyleName"/>
										<xsl:with-param name="precedingListItemsOfSameLevelAndStyle" select="$precedingListItemsOfSameLevelAndStyle"/>
										<xsl:with-param name="precedingListItemsOfSameLevelAndStyleCount" select="$precedingListItemsOfSameLevelAndStyleCount"/>
										<xsl:with-param name="precedingListItemsOfSameStyle" select="$precedingListItemsOfSameStyle"/>
										<xsl:with-param name="precedingListItemsOfSameStyleCount" select="$precedingListItemsOfSameStyleCount"/>
										<xsl:with-param name="pseudoLevel">
											<xsl:choose>
												<!-- empty list item does not count -->
												<xsl:when test="not(*[name() = 'text:h' or name() = 'text:p'])">
													<xsl:value-of select="$pseudoLevel"/>
												</xsl:when>
												<xsl:otherwise>1</xsl:otherwise>
											</xsl:choose>
										</xsl:with-param>
									</xsl:call-template>
								</xsl:otherwise>
							</xsl:choose>
						</xsl:otherwise>
					</xsl:choose>
				</xsl:for-each>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="$itemNumber"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name="countListItem">
		<xsl:param name="IteratorSameLevelAndStyle"/>
		<xsl:param name="IteratorSameStyle"/>
		<xsl:param name="itemNumber"/>
		<xsl:param name="listLevel"/>
		<xsl:param name="listLevelStyle"/>
		<xsl:param name="listStyle"/>
		<xsl:param name="listStyleName"/>
		<xsl:param name="precedingListItemsOfSameLevelAndStyle"/>
		<xsl:param name="precedingListItemsOfSameLevelAndStyleCount"/>
		<xsl:param name="precedingListItemsOfSameStyle"/>
		<xsl:param name="precedingListItemsOfSameStyleCount"/>
		<xsl:param name="pseudoLevel" />

		<xsl:choose>
			<xsl:when test="@text:start-value">
				<xsl:choose>
					<xsl:when test="not(*[name() = 'text:h' or name() = 'text:p'])">
						<!-- empty list item does not count -->
						<xsl:call-template name="countListItemTillStartValue">
							<xsl:with-param name="IteratorSameLevelAndStyle" select="$IteratorSameLevelAndStyle + 1" />
							<xsl:with-param name="IteratorSameStyle" select="$IteratorSameStyle + 1"/>
							<xsl:with-param name="itemNumber" select="$itemNumber"/>
							<xsl:with-param name="listLevel" select="$listLevel"/>
							<xsl:with-param name="listLevelStyle" select="$listLevelStyle"/>
							<xsl:with-param name="listStyle" select="$listStyle"/>
							<xsl:with-param name="precedingListItemsOfSameLevelAndStyle" select="$precedingListItemsOfSameLevelAndStyle"/>
							<xsl:with-param name="precedingListItemsOfSameLevelAndStyleCount" select="$precedingListItemsOfSameLevelAndStyleCount"/>
							<xsl:with-param name="precedingListItemsOfSameStyle" select="$precedingListItemsOfSameStyle"/>
							<xsl:with-param name="precedingListItemsOfSameStyleCount" select="$precedingListItemsOfSameStyleCount"/>
							<xsl:with-param name="pseudoLevel" select="$pseudoLevel" />
						</xsl:call-template>
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="$itemNumber + @text:start-value"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<xsl:when test="$listLevelStyle/@text:start-value">
				<xsl:choose>
					<xsl:when test="not(*[name() = 'text:h' or name() = 'text:p'])">
						<!-- empty list item does not count -->
						<xsl:call-template name="countListItemTillStartValue">
							<xsl:with-param name="IteratorSameLevelAndStyle" select="$IteratorSameLevelAndStyle + 1" />
							<xsl:with-param name="IteratorSameStyle" select="$IteratorSameStyle + 1"/>
							<xsl:with-param name="itemNumber" select="$itemNumber"/>
							<xsl:with-param name="listLevel" select="$listLevel"/>
							<xsl:with-param name="listLevelStyle" select="$listLevelStyle"/>
							<xsl:with-param name="listStyle" select="$listStyle"/>
							<xsl:with-param name="precedingListItemsOfSameLevelAndStyle" select="$precedingListItemsOfSameLevelAndStyle"/>
							<xsl:with-param name="precedingListItemsOfSameLevelAndStyleCount" select="$precedingListItemsOfSameLevelAndStyleCount"/>
							<xsl:with-param name="precedingListItemsOfSameStyle" select="$precedingListItemsOfSameStyle"/>
							<xsl:with-param name="precedingListItemsOfSameStyleCount" select="$precedingListItemsOfSameStyleCount"/>
							<xsl:with-param name="pseudoLevel" select="$pseudoLevel" />
						</xsl:call-template>
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="$itemNumber + $listLevelStyle/@text:start-value"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<xsl:otherwise>
				<xsl:choose>
					<xsl:when test="not(*[name() = 'text:h' or name() = 'text:p'])">
						<!-- empty list item does not count -->
						<xsl:call-template name="countListItemTillStartValue">
							<xsl:with-param name="IteratorSameLevelAndStyle" select="$IteratorSameLevelAndStyle + 1" />
							<xsl:with-param name="IteratorSameStyle" select="$IteratorSameStyle + 1"/>
							<xsl:with-param name="itemNumber" select="$itemNumber"/>
							<xsl:with-param name="listLevel" select="$listLevel"/>
							<xsl:with-param name="listLevelStyle" select="$listLevelStyle"/>
							<xsl:with-param name="listStyle" select="$listStyle"/>
							<xsl:with-param name="precedingListItemsOfSameLevelAndStyle" select="$precedingListItemsOfSameLevelAndStyle"/>
							<xsl:with-param name="precedingListItemsOfSameLevelAndStyleCount" select="$precedingListItemsOfSameLevelAndStyleCount"/>
							<xsl:with-param name="precedingListItemsOfSameStyle" select="$precedingListItemsOfSameStyle"/>
							<xsl:with-param name="precedingListItemsOfSameStyleCount" select="$precedingListItemsOfSameStyleCount"/>
							<xsl:with-param name="pseudoLevel" select="$pseudoLevel" />
						</xsl:call-template>
					</xsl:when>
					<xsl:otherwise>
						<!-- count on till you find a start-value or the end is reached -->
						<xsl:call-template name="countListItemTillStartValue">
							<xsl:with-param name="IteratorSameLevelAndStyle" select="$IteratorSameLevelAndStyle + 1" />
							<xsl:with-param name="IteratorSameStyle" select="$IteratorSameStyle + 1"/>
							<xsl:with-param name="itemNumber" select="$itemNumber + 1"/>
							<xsl:with-param name="listLevel" select="$listLevel"/>
							<xsl:with-param name="listLevelStyle" select="$listLevelStyle"/>
							<xsl:with-param name="listStyle" select="$listStyle"/>
							<xsl:with-param name="precedingListItemsOfSameLevelAndStyle" select="$precedingListItemsOfSameLevelAndStyle"/>
							<xsl:with-param name="precedingListItemsOfSameLevelAndStyleCount" select="$precedingListItemsOfSameLevelAndStyleCount"/>
							<xsl:with-param name="precedingListItemsOfSameStyle" select="$precedingListItemsOfSameStyle"/>
							<xsl:with-param name="precedingListItemsOfSameStyleCount" select="$precedingListItemsOfSameStyleCount"/>
							<xsl:with-param name="pseudoLevel" select="0" />
						</xsl:call-template>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>


	<!-- Creates the list label containing the number, which is separated by '.' between the levels.
		Depending on the levels to display (listLevelsToDisplay) -->
	<xsl:template name="createItemLabel">
		<xsl:param name="itemLabel" select="''"/>
		<xsl:param name="itemNumber" />
		<xsl:param name="listLevel" />
		<xsl:param name="listLevelStyle" />
		<xsl:param name="listLevelsToDisplay" />

		<xsl:choose>
			<xsl:when test="$listLevelsToDisplay &lt; $listLevel">
				<xsl:call-template name="truncLabel">
					<xsl:with-param name="itemLabel" select="$itemLabel"/>
					<xsl:with-param name="itemNumber" select="$itemNumber" />
					<xsl:with-param name="listLevel" select="$listLevel"/>
					<xsl:with-param name="listLevelStyle" select="$listLevelStyle" />
					<xsl:with-param name="listLevelsToDisplay" select="$listLevelsToDisplay"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:otherwise>
				<xsl:variable name="numberedSymbol">
					<xsl:number value="$itemNumber" format="{$listLevelStyle/@style:num-format}"/>
				</xsl:variable>
				<xsl:choose>
					<xsl:when test="$listLevelsToDisplay != 1">
						<xsl:value-of select="concat($itemLabel, '.' , $numberedSymbol)"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="$numberedSymbol"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name="truncLabel">
		<xsl:param name="itemLabel" />
		<xsl:param name="itemNumber" />
		<xsl:param name="listLevel" />
		<xsl:param name="listLevelStyle" />
		<xsl:param name="listLevelsToDisplay" />
		<xsl:param name="listStyle" />
		<xsl:param name="listStyleName" />

		<xsl:call-template name="createItemLabel">
			<xsl:with-param name="itemLabel">
				<xsl:if test="contains($itemLabel, '.')">
					<xsl:value-of select="substring-after($itemLabel, '.')"/>
				</xsl:if>
			</xsl:with-param>
			<xsl:with-param name="itemNumber" select="$itemNumber"/>
			<xsl:with-param name="listLevel" select="$listLevel - 1"/>
			<xsl:with-param name="listLevelStyle" select="$listLevelStyle"/>
			<xsl:with-param name="listLevelsToDisplay" select="$listLevelsToDisplay"/>
			<xsl:with-param name="listStyle" select="$listStyle"/>
			<xsl:with-param name="listStyleName" select="$listStyleName"/>
		</xsl:call-template>
	</xsl:template>


	<xsl:template match="text:p" mode="first-list-child-margin-adaption">
		<xsl:param name="globalData"/>
		<xsl:param name="listLabelElement"/>
		<xsl:param name="isEmptyList"/>
		<xsl:param name="isListNumberingReset"/>
		<xsl:param name="isNextLevelNumberingReset"/>
		<xsl:param name="itemLabel"/>
		<xsl:param name="itemNumber"/>
		<xsl:param name="listIndent"/>
		<xsl:param name="listLevel"/>
		<xsl:param name="listRestart"/>
		<xsl:param name="listStyle"/>
		<xsl:param name="listStyleName"/>
		<xsl:param name="minLabelWidth"/>

		<xsl:element name="p">
			<xsl:call-template name="create-list-style">
				<xsl:with-param name="globalData" select="$globalData"/>
				<xsl:with-param name="listIndent" select="$listIndent"/>
				<xsl:with-param name="styleName" select="@text:style-name"/>
			</xsl:call-template>
			<xsl:if test="$listLabelElement">
				<xsl:copy-of select="$listLabelElement"/>
			</xsl:if>
			<xsl:apply-templates>
				<xsl:with-param name="globalData" select="$globalData"/>
				<xsl:with-param name="listIndent" select="$minLabelWidth"/>
			</xsl:apply-templates>
			<xsl:if test="$listLabelElement">
				<!-- this span disables the float necessary to bring two block elements on one line. It contains a space as IE6 bug workaround -->
				<span class="odfLiEnd"></span>
			</xsl:if>
			<xsl:text>&#160;</xsl:text>
		</xsl:element>

		<xsl:apply-templates mode="first-list-child-margin-adaption" select="following-sibling::*[1]">
			<xsl:with-param name="globalData" select="$globalData"/>
			<xsl:with-param name="isEmptyList" select="$isEmptyList"/>
			<xsl:with-param name="isListNumberingReset" select="$isListNumberingReset"/>
			<xsl:with-param name="isNextLevelNumberingReset" select="$isNextLevelNumberingReset"/>
			<xsl:with-param name="itemLabel" select="$itemLabel"/>
			<xsl:with-param name="itemNumber" select="$itemNumber"/>
			<xsl:with-param name="listLevel" select="$listLevel"/>
			<xsl:with-param name="listRestart" select="$listRestart"/>
			<xsl:with-param name="listStyle" select="$listStyle"/>
			<xsl:with-param name="listStyleName" select="$listStyleName"/>
			<xsl:with-param name="listIndent" select="$listIndent"/>
			<xsl:with-param name="minLabelWidth" select="$minLabelWidth"/>
		</xsl:apply-templates>
	</xsl:template>


	<!-- Neglecting the left margin behavior for headings for now -->
	<xsl:template match="text:h" mode="first-list-child-margin-adaption">
		<xsl:param name="globalData"/>
		<xsl:param name="listLabelElement"/>
		<xsl:param name="isEmptyList"/>
		<xsl:param name="isListNumberingReset"/>
		<xsl:param name="isNextLevelNumberingReset"/>
		<xsl:param name="itemLabel"/>
		<xsl:param name="itemNumber"/>
		<xsl:param name="listIndent"/>
		<xsl:param name="listLevel"/>
		<xsl:param name="listRestart"/>
		<xsl:param name="listStyle"/>
		<xsl:param name="listStyleName"/>
		<xsl:param name="minLabelWidth"/>

		<xsl:element name="h">
			<xsl:call-template name="create-list-style">
				<xsl:with-param name="globalData" select="$globalData"/>
				<xsl:with-param name="listIndent" select="$listIndent"/>
				<xsl:with-param name="styleName" select="@text:style-name"/>
			</xsl:call-template>
			<xsl:if test="$listLabelElement">
				<xsl:copy-of select="$listLabelElement"/>
			</xsl:if>
			<xsl:apply-templates>
				<xsl:with-param name="globalData" select="$globalData"/>
				<xsl:with-param name="listIndent" select="$minLabelWidth"/>
			</xsl:apply-templates>
			<xsl:if test="$listLabelElement">
				<!-- this span disables the float necessary to bring two block elements on one line. It contains a space as IE6 bug workaround -->
				<span class="odfLiEnd"></span>
			</xsl:if>
			<xsl:text>&#160;</xsl:text>
		</xsl:element>

		<xsl:apply-templates mode="first-list-child-margin-adaption" select="following-sibling::*[1]">
			<xsl:with-param name="globalData" select="$globalData"/>
			<xsl:with-param name="isEmptyList" select="$isEmptyList"/>
			<xsl:with-param name="isListNumberingReset" select="$isListNumberingReset"/>
			<xsl:with-param name="isNextLevelNumberingReset" select="$isNextLevelNumberingReset"/>
			<xsl:with-param name="itemLabel" select="$itemLabel"/>
			<xsl:with-param name="itemNumber" select="$itemNumber"/>
			<xsl:with-param name="listLevel" select="$listLevel"/>
			<xsl:with-param name="listRestart" select="$listRestart"/>
			<xsl:with-param name="listStyle" select="$listStyle"/>
			<xsl:with-param name="listStyleName" select="$listStyleName"/>
			<xsl:with-param name="listIndent" select="$listIndent"/>
			<xsl:with-param name="minLabelWidth" select="$minLabelWidth"/>
		</xsl:apply-templates>
	</xsl:template>


	<xsl:template match="*" mode="first-list-child-margin-adaption">
		<xsl:param name="globalData"/>
		<xsl:param name="isEmptyList"/>
		<xsl:param name="isListNumberingReset"/>
		<xsl:param name="isNextLevelNumberingReset"/>
		<xsl:param name="itemLabel"/>
		<xsl:param name="itemNumber"/>
		<xsl:param name="listIndent"/>
		<xsl:param name="listLevel"/>
		<xsl:param name="listRestart"/>
		<xsl:param name="listStyle"/>
		<xsl:param name="listStyleName"/>
		<xsl:param name="minLabelWidth"/>

		<xsl:apply-templates select="self::*" mode="listItemSibling">
			<xsl:with-param name="globalData" select="$globalData"/>
			<xsl:with-param name="isEmptyList" select="$isEmptyList"/>
			<xsl:with-param name="isListNumberingReset" select="$isListNumberingReset"/>
			<xsl:with-param name="isNextLevelNumberingReset" select="$isNextLevelNumberingReset"/>
			<xsl:with-param name="itemLabel" select="$itemLabel"/>
			<xsl:with-param name="itemNumber" select="$itemNumber"/>
			<xsl:with-param name="listIndent" select="$listIndent"/>
			<xsl:with-param name="listLevel" select="$listLevel"/>
			<xsl:with-param name="listRestart" select="$listRestart"/>
			<xsl:with-param name="listStyle" select="$listStyle"/>
			<xsl:with-param name="listStyleName" select="$listStyleName"/>
		</xsl:apply-templates>

		<xsl:apply-templates mode="first-list-child-margin-adaption" select="following-sibling::*[1]">
			<xsl:with-param name="globalData" select="$globalData"/>
			<xsl:with-param name="isEmptyList" select="$isEmptyList"/>
			<xsl:with-param name="isListNumberingReset" select="$isListNumberingReset"/>
			<xsl:with-param name="isNextLevelNumberingReset" select="$isNextLevelNumberingReset"/>
			<xsl:with-param name="itemLabel" select="$itemLabel"/>
			<xsl:with-param name="itemNumber" select="$itemNumber"/>
			<xsl:with-param name="listLevel" select="$listLevel"/>
			<xsl:with-param name="listRestart" select="$listRestart"/>
			<xsl:with-param name="listStyle" select="$listStyle"/>
			<xsl:with-param name="listStyleName" select="$listStyleName"/>
			<xsl:with-param name="listIndent" select="$listIndent"/>
			<xsl:with-param name="minLabelWidth" select="$minLabelWidth"/>
		</xsl:apply-templates>
	</xsl:template>

	<xsl:template match="*" mode="listItemSibling">
		<xsl:param name="globalData"/>
		<xsl:param name="isEmptyList"/>
		<xsl:param name="isListNumberingReset"/>
		<xsl:param name="isNextLevelNumberingReset"/>
		<xsl:param name="itemLabel"/>
		<xsl:param name="itemNumber"/>
		<xsl:param name="listIndent"/>
		<xsl:param name="listLevel"/>
		<xsl:param name="listRestart"/>
		<xsl:param name="listStyle"/>
		<xsl:param name="listStyleName"/>

		<xsl:apply-templates select="self::*">
			<xsl:with-param name="globalData" select="$globalData"/>
			<xsl:with-param name="isEmptyList" select="$isEmptyList"/>
			<xsl:with-param name="isListNumberingReset" select="$isListNumberingReset"/>
			<xsl:with-param name="isNextLevelNumberingReset" select="$isNextLevelNumberingReset"/>
			<xsl:with-param name="itemNumber" select="$itemNumber"/>
			<xsl:with-param name="listIndent" select="$listIndent"/>
			<!-- receives the same parent label, only with a different itemNumber -->
			<xsl:with-param name="itemLabel" select="$itemLabel"/>
			<xsl:with-param name="listLevel" select="$listLevel"/>
			<xsl:with-param name="listStyle" select="$listStyle"/>
			<xsl:with-param name="listStyleName" select="$listStyleName"/>
		</xsl:apply-templates>
		<xsl:apply-templates select="following-sibling::*[1]" mode="listItemSibling">
			<xsl:with-param name="globalData" select="$globalData"/>
			<xsl:with-param name="isEmptyList" select="$isEmptyList"/>
			<xsl:with-param name="isListNumberingReset" select="$isListNumberingReset"/>
			<xsl:with-param name="isNextLevelNumberingReset" select="$isNextLevelNumberingReset"/>
			<xsl:with-param name="itemNumber" select="$itemNumber"/>
			<xsl:with-param name="listIndent" select="$listIndent"/>
			<!-- receives the same parent label, only with a different itemNumber -->
			<xsl:with-param name="itemLabel" select="$itemLabel"/>
			<xsl:with-param name="listLevel" select="$listLevel"/>
			<xsl:with-param name="listStyle" select="$listStyle"/>
			<xsl:with-param name="listStyleName" select="$listStyleName"/>
		</xsl:apply-templates>
	</xsl:template>

	<xsl:template match="text()" mode="first-list-child-margin-adaption">
		<xsl:value-of select="."/>
	</xsl:template>


	<xsl:template name="create-list-style">
		<xsl:param name="globalData"/>
		<xsl:param name="listIndent" select="0"/>
		<xsl:param name="styleName"/>

		<xsl:if test="$styleName">
			<xsl:attribute name="class">
				<xsl:value-of select="translate($styleName, '.,;: %()[]/\+', '_____________')"/>
			</xsl:attribute>
		</xsl:if>
		<xsl:attribute name="style">
			<xsl:text>margin-left:</xsl:text>
			<xsl:value-of select="$listIndent"/>
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
					<xsl:element name="span">
						<xsl:attribute name="class">footnodeNumber</xsl:attribute>
						<xsl:element name="a">
							<xsl:attribute name="class"><xsl:value-of select="$footnoteConfig/@text:citation-style-name"/></xsl:attribute>
							<xsl:attribute name="id"><xsl:value-of select="@text:id"/></xsl:attribute>
							<xsl:attribute name="href"><xsl:value-of select="concat('#body_', @text:id)"/></xsl:attribute>
							<xsl:apply-templates  mode="textOnly" select="text:note-citation"/>
						</xsl:element>
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

	<!-- MathML -->
	<xsl:template match="draw:object[math:math]">
		<math xmlns="http://www.w3.org/1998/Math/MathML">
			<xsl:apply-templates select="math:math/math:semantics/*" mode="math"/>
		</math>
	</xsl:template>

	<xsl:template match="*" mode="math">
		<xsl:element name="{local-name()}" namespace="http://www.w3.org/1998/Math/MathML">
			<xsl:apply-templates select="@*|node()" mode="math"/>
		</xsl:element>
	</xsl:template>

	<xsl:template match="@*" mode="math">
		<xsl:attribute name="{local-name()}">
			<xsl:value-of select="."/>
		</xsl:attribute>
	</xsl:template>

	<xsl:template match="math:annotation" mode="math"/>

</xsl:stylesheet>