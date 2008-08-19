<?xml version="1.0" encoding="UTF-8"?>
<!--

  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
  
  Copyright 2008 by Sun Microsystems, Inc.
 
  OpenOffice.org - a multi-platform office productivity suite
 
  $RCSfile: table.xsl,v $
 
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
	exclude-result-prefixes="chart config dc dom dr3d draw fo form math meta number office ooo oooc ooow script style svg table text xforms xlink xsd xsi">


	<!-- table row handling -->
	<xsl:include href="table_rows.xsl" />
	<!-- table column handling -->
	<xsl:include href="table_columns.xsl" />
	<!-- table cell handling -->
	<xsl:include href="table_cells.xsl" />

	<xsl:param name="tableElement" select="'table'" />

	<!-- ******************* -->
	<!-- *** main table  *** -->
	<!-- ******************* -->


	<xsl:template match="table:table" name="table:table">
		<xsl:param name="globalData" />

		<!-- The table will only be created if the table:scenario is active -->
		<xsl:if test="not(table:scenario) or table:scenario/@table:is-active">
			<xsl:call-template name="create-table">
				<xsl:with-param name="globalData" select="$globalData" />
			</xsl:call-template>
		</xsl:if>
	</xsl:template>



	<xsl:template name="create-table">
		<xsl:param name="globalData" />

		<!-- by default '1', for each new sub/inner/nested table the number counts one up -->
		<xsl:variable name="tableLevel" select="count(ancestor-or-self::table:table)" />
		<!-- collecting all visible "table:table-row" elements of the table -->
		<xsl:variable name="allVisibleTableRows" select="table:table-row[not(@table:visibility = 'collapse' or @table:visibility = 'filter')][count(ancestor-or-self::table:table) = $tableLevel] |
														 table:table-header-rows/descendant::table:table-row[not(@table:visibility = 'collapse' or @table:visibility = 'filter')][count(ancestor-or-self::table:table) = $tableLevel] |
														 table:table-row-group/descendant::table:table-row[not(@table:visibility = 'collapse' or @table:visibility = 'filter')][count(ancestor-or-self::table:table) = $tableLevel]" />
		<!-- As the alignment of a table is by 'align' attribut is deprecated and as the CSS 'float' attribute not well displayed,
			 we do a trick by encapsulating the table with a aligned 'div' element-->
		<xsl:variable name="table-alignment" select="key('styles', @style:name = current()/@table:style-name)/*/@table:align" />
		<xsl:choose>
			<xsl:when test="string-length($table-alignment) != 0">
				<xsl:element namespace="{$namespace}" name="div">
					<xsl:attribute name="style">
						<xsl:choose>
							<xsl:when test='$table-alignment="left" or $table-alignment="margins"'>
								<xsl:text>text-align:left</xsl:text>
							</xsl:when>
							<xsl:when test='$table-alignment="right"'>
								<xsl:text>text-align:right</xsl:text>
							</xsl:when>
							<xsl:when test='$table-alignment="center"'>
								<xsl:text>text-align:center</xsl:text>
							</xsl:when>
						</xsl:choose>
					</xsl:attribute>
					<xsl:call-template name="create-table-element">
						<xsl:with-param name="globalData" select="$globalData" />
						<xsl:with-param name="allVisibleTableRows" select="$allVisibleTableRows" />
					</xsl:call-template>
				</xsl:element>
			</xsl:when>
			<xsl:otherwise>
				<xsl:call-template name="create-table-element">
					<xsl:with-param name="globalData" select="$globalData" />
					<xsl:with-param name="allVisibleTableRows" select="$allVisibleTableRows" />
				</xsl:call-template>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>


	<xsl:template name="create-table-element">
		<xsl:param name="globalData" />
		<xsl:param name="allVisibleTableRows" />

		<xsl:element namespace="{$namespace}" name="{$tableElement}">
			<xsl:attribute name="border">0</xsl:attribute>
			<xsl:attribute name="cellspacing">0</xsl:attribute>
			<xsl:attribute name="cellpadding">0</xsl:attribute>
			<xsl:choose>
				<xsl:when test='name()="table:table"'>
					<xsl:variable name="value" select="$globalData/all-doc-styles/style[@style:name = current()/@table:style-name]/*/@style:rel-width" />
					<xsl:if test="$value">
						<xsl:attribute name="width">
							<xsl:value-of select="$value" />
						</xsl:attribute>
					</xsl:if>
				</xsl:when>
				<xsl:otherwise>
					<xsl:attribute name="width">100%</xsl:attribute>
				</xsl:otherwise>
			</xsl:choose>

			<xsl:apply-templates select="@table:style-name">
				<xsl:with-param name="globalData" select="$globalData" />
			</xsl:apply-templates>

			<xsl:call-template name="create-column-style-variable">
				<xsl:with-param name="globalData" select="$globalData" />
				<xsl:with-param name="allVisibleTableRows" select="$allVisibleTableRows" />
			</xsl:call-template>
		</xsl:element>
	</xsl:template>

</xsl:stylesheet>
