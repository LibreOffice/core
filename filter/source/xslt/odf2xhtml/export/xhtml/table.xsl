<?xml version="1.0" encoding="UTF-8"?>
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
<!--
	For further documentation and updates visit http://xml.openoffice.org/odf2xhtml
-->
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:chart="urn:oasis:names:tc:opendocument:xmlns:chart:1.0"
	xmlns:config="urn:oasis:names:tc:opendocument:xmlns:config:1.0"
	xmlns:dc="http://purl.org/dc/elements/1.1/"
	xmlns:dom="http://www.w3.org/2001/xml-events"
	xmlns:dr3d="urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0"
	xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0"
	xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
	xmlns:form="urn:oasis:names:tc:opendocument:xmlns:form:1.0"
	xmlns:math="http://www.w3.org/1998/Math/MathML"
	xmlns:meta="urn:oasis:names:tc:opendocument:xmlns:meta:1.0"
	xmlns:number="urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0"
	xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
	xmlns:ooo="http://openoffice.org/2004/office"
	xmlns:oooc="http://openoffice.org/2004/calc"
	xmlns:ooow="http://openoffice.org/2004/writer"
	xmlns:script="urn:oasis:names:tc:opendocument:xmlns:script:1.0"
	xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
	xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
	xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0"
	xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
	xmlns:xforms="http://www.w3.org/2002/xforms"
	xmlns:xlink="http://www.w3.org/1999/xlink"
	xmlns:xsd="http://www.w3.org/2001/XMLSchema"
	xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
	exclude-result-prefixes="chart config dc dom dr3d draw fo form math meta number office ooo oooc ooow script style svg table text xforms xlink xsd xsi"
	xmlns="http://www.w3.org/1999/xhtml">



	<!-- current node is a table:table -->
	<xsl:template name="create-table-children">
		<xsl:param name="globalData" />
		<xsl:param name="allVisibleTableRows" />
		<xsl:param name="allTableColumns" />

		<xsl:element name="colgroup">
			<xsl:for-each select="$allTableColumns/table:table-column">
				<xsl:if test="not(@table:visibility = 'collapse' or @table:visibility = 'filter')">
					<xsl:element name="col">
						<xsl:variable name="value" select="$globalData/all-doc-styles/style[@style:name = current()/@table:style-name]/*/@style:column-width" />
						<xsl:if test="$value">
							<xsl:attribute name="width">
								<!-- using the absolute width, problems with the relative in browser (in OOo style:rel-column-width) -->
								<xsl:call-template name="convert2px">
									<xsl:with-param name="value" select="$globalData/all-doc-styles/style[@style:name = current()/@table:style-name]/*/@style:column-width" />
								</xsl:call-template>
							</xsl:attribute>
						</xsl:if>
					</xsl:element>
					<!-- *** the column-style ***
					<xsl:attribute name="width">
						<xsl:variable name="currentColumnStyleName" select="$allTableColumns/table:table-column[position() = $columnPosition]/@table:style-name" />
						<xsl:value-of select="$globalData/all-doc-styles/style[@style:name = $currentColumnStyleName]/*/@style:column-width" />
					</xsl:attribute>-->
				</xsl:if>
			</xsl:for-each>
		</xsl:element>

		<xsl:call-template name="create-table-rows">
			<xsl:with-param name="globalData"           select="$globalData" />
			<xsl:with-param name="allVisibleTableRows"  select="$allVisibleTableRows" />
			<xsl:with-param name="allTableColumns"      select="$allTableColumns" />
		</xsl:call-template>
	</xsl:template>



	<!-- Creating the content of a table content using CSS styles -->
	<xsl:template name="create-table-cell-content">
		<xsl:param name="tableDataType" />
		<xsl:param name="globalData" />
		<xsl:param name="allTableColumns" />
		<xsl:param name="columnPosition" />
		<xsl:param name="currentTableColumn" />

		<xsl:element name="{$tableDataType}">

			<!-- if parser reads DTD the default is set to '1' -->
			<xsl:if test="@table:number-columns-spanned and @table:number-columns-spanned > 1">
				<xsl:attribute name="colspan">
					<xsl:value-of select="@table:number-columns-spanned" />
				</xsl:attribute>
			</xsl:if>
			<!-- if parser reads DTD the default is set to '1' -->
			<xsl:if test="@table:number-rows-spanned and @table:number-rows-spanned > 1">
				<xsl:attribute name="rowspan">
					<xsl:value-of select="@table:number-rows-spanned" />
				</xsl:attribute>
			</xsl:if>


			<!-- *** the cell-style *** -->
			<!-- The cell style has no conclusion with the column style, so we switch the order/priorities due to browser issues

				The cell-style depends on two attributes:

				1) table:style-name - the style properties of cell. When they exist, a default alignement (cp. below) will be added for the
									  case of no alignment in the style exist.

				2) office:value-type - the value type of the table-cell giving the default alignments.
									  By default a string value is left aligned, all other are aligned:right.
			-->
			<xsl:choose>
				<xsl:when test="@table:style-name">
					<xsl:call-template name="set-styles">
						<xsl:with-param name="globalData" select="$globalData" />
						<xsl:with-param name="styleName" select="@table:style-name" />
						<xsl:with-param name="currentTableColumn" select="$currentTableColumn" />
					</xsl:call-template>
				</xsl:when>
				<xsl:otherwise>
					<!-- Cells without a style use the 'table:default-cell-style-name'
						 when there is no default cell style specified for the current column. -->
					<xsl:variable name="defaultCellStyleName" select="$currentTableColumn/@table:default-cell-style-name" />
					<xsl:choose>
						<xsl:when test="$defaultCellStyleName">
							<xsl:call-template name="set-styles">
								<xsl:with-param name="globalData" select="$globalData" />
								<xsl:with-param name="styleName" select="$defaultCellStyleName" />
								<xsl:with-param name="currentTableColumn" select="$currentTableColumn" />
							</xsl:call-template>
						</xsl:when>
						<xsl:otherwise>
							<!-- No cell style exists, nor a default table cell style for the column -->
							<xsl:attribute name="style">
								<!-- sets cell alignment dependent of cell value type -->
								<xsl:call-template name="set-cell-alignment" />
							</xsl:attribute>
						</xsl:otherwise>
					</xsl:choose>
				</xsl:otherwise>
			</xsl:choose>

			<xsl:if test="$debugEnabled">
				<xsl:message>A table cell '<xsl:value-of select="$tableDataType" />' element has been added!</xsl:message>
			</xsl:if>

			<!-- empty cell tags produce problems with width CSS style on itself other table cells as well
				therefore an non breakable space (&nbsp;/&#160;) have been inserted.-->
			<xsl:choose>
				<xsl:when test="node()">
					<xsl:call-template name="apply-styles-and-content">
						<xsl:with-param name="globalData" select="$globalData" />
					</xsl:call-template>
				</xsl:when>
				<xsl:otherwise>
					<xsl:call-template name="apply-styles-and-content">
						<xsl:with-param name="globalData" select="$globalData" />
					</xsl:call-template>
					<xsl:text>&#160;</xsl:text>
				</xsl:otherwise>
			</xsl:choose>

		</xsl:element>
	</xsl:template>


	<!-- Sets the cell alignment by the 'office:value-type' of the 'table:table-cell'.
		 Strings have a left alignment, other values right -->
	<xsl:template name="set-cell-alignment">
		<xsl:choose>
			<xsl:when test="@office:value-type and not(@office:value-type = 'string')">text-align:right; </xsl:when>
			<xsl:otherwise>text-align:left;</xsl:otherwise>
		</xsl:choose>
	</xsl:template>



	<!-- Sets styles of a cell -->
	<xsl:template name="set-styles">
		<xsl:param name="globalData" />
		<xsl:param name="styleName" />
		<xsl:param name="currentTableColumn" />

		<xsl:attribute name="style">
			<!-- sets cell alignment dependent of cell value type -->
			<xsl:call-template name="set-cell-alignment" />

			<!-- set column style (disjunct of cell style) -->
			<xsl:value-of select="$globalData/all-styles/style[@style:name = $currentTableColumn/@table:style-name]/final-properties" />

	   </xsl:attribute>

	   <!-- cell style header -->
	   <xsl:attribute name="class">
		   <xsl:value-of select="translate($styleName, '.,;: %()[]/\+', '_____________')"/>
	   </xsl:attribute>
	</xsl:template>
</xsl:stylesheet>

