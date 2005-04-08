<!--

   $RCSfile: table.xsl,v $

   $Revision: 1.3 $

   last change: $Author: hr $ $Date: 2005-04-08 16:37:45 $

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

   Copyright © 2002 by Sun Microsystems, Inc.

   All Rights Reserved.

   Contributor(s): _______________________________________

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
	xmlns:xlink="http://www.w3.org/1999/xlink"
	xmlns:xt="http://www.jclark.com/xt"
	xmlns:common="http://exslt.org/common"
	xmlns:xalan="http://xml.apache.org/xalan"
	xmlns="urn:schemas-microsoft-com:office:spreadsheet" xmlns:c="urn:schemas-microsoft-com:office:component:spreadsheet" xmlns:html="http://www.w3.org/TR/REC-html40" xmlns:o="urn:schemas-microsoft-com:office:office" xmlns:ss="urn:schemas-microsoft-com:office:spreadsheet" xmlns:x2="http://schemas.microsoft.com/office/excel/2003/xml" xmlns:x="urn:schemas-microsoft-com:office:excel" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
	exclude-result-prefixes="chart config dc dom dr3d draw fo form math meta number office ooo oooc ooow script style svg table text xlink xt common xalan">

	<xsl:param name="tableElement"          select="'Table'" />
	<xsl:param name="rowElement"            select="'Row'" />
	<xsl:param name="cellElement"           select="'Cell'" />

	<!-- ************** -->
	<!-- *** Table  *** -->
	<!-- ************** -->

	<xsl:template match="table:table" name="table:table">

		<!-- The table will only be created if the table:scenario is active -->
		<xsl:if test="not(table:scenario) or table:scenario/@table:is-active">
			<xsl:call-template name="create-table" />
		</xsl:if>
	</xsl:template>


	<xsl:template name="create-table">

		<!-- collecting all visible "table:table-row" elements of the table -->
		<xsl:variable name="allVisibleTableRows" select="table:table-row[not(@table:visibility = 'collapse' or @table:visibility = 'filter')] | table:table-header-rows/descendant::table:table-row[not(@table:visibility = 'collapse' or @table:visibility = 'filter')] | table:table-row-group/descendant::table:table-row[not(@table:visibility = 'collapse' or @table:visibility = 'filter')]" />

		<xsl:call-template name="create-table-element">
			<xsl:with-param name="allVisibleTableRows" select="$allVisibleTableRows" />
		</xsl:call-template>

	</xsl:template>


	<xsl:template name="create-table-element">
		<xsl:param name="allVisibleTableRows" />

		<xsl:element namespace="{$namespace}" name="{$tableElement}">
			<xsl:apply-templates select="@table:style-name" />

			<!-- all columns of the table -->
			<xsl:variable name="columnNodes" select="table:table-column |
													 table:table-header-columns/descendant::table:table-column |
													 table:table-column-group/descendant::table:table-column" />
			<xsl:call-template name="create-table-column">
				<xsl:with-param name="columnNodes"  select="$columnNodes" />
				<xsl:with-param name="currentColumn" select="$columnNodes[1]" />
				<xsl:with-param name="columnNodeNo" select="1" />
				<xsl:with-param name="columnCount"  select="count($columnNodes)" />
				<xsl:with-param name="columnNo"     select="1" />
			</xsl:call-template>

			<xsl:variable name="columnsRepeated" select="table:table-column/@table:number-columns-repeated |
														 table:table-header-columns/descendant::table:table-column/@table:number-columns-repeated |
														 table:table-column-group/descendant::table:table-column/@table:number-columns-repeated" />
			<xsl:variable name="columnCount">
				<xsl:choose>
					<xsl:when test="$columnNodes[last()]/@table:number-columns-repeated &gt; 99">
						<!-- This is only an approximation, in case the last column would have content this is wrong -->
						<xsl:value-of select="count($columnNodes)
											+ sum($columnsRepeated)
											- count($columnsRepeated)
											- $columnNodes[last()]/@table:number-columns-repeated
											+ 1" />
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="count($columnNodes)
											+ sum($columnsRepeated)
											- count($columnsRepeated)" />
					</xsl:otherwise>
				</xsl:choose>
			</xsl:variable>

			<xsl:apply-templates select="table:table-row">
				<xsl:with-param name="columnCount"  select="$columnCount" />
			</xsl:apply-templates>
		</xsl:element>
	</xsl:template>


	<!-- **************** -->
	<!-- *** Columns  *** -->
	<!-- **************** -->
	<xsl:template name="create-table-column">
		<xsl:param name="columnNodes"  />
		<xsl:param name="currentColumn"  />
		<xsl:param name="columnCount"  />
		<xsl:param name="columnNo"     />
		<xsl:param name="columnNodeNo" />
		<xsl:param name="index" />

		<xsl:element name="Column" namespace="{$namespace}">

			<xsl:if test="$currentColumn/@table:visibility = 'collapse' or $currentColumn/@table:visibility = 'filter'">
				<xsl:attribute name="ss:Hidden">1</xsl:attribute>
			</xsl:if>

			<xsl:if test="$currentColumn/@table:number-columns-repeated">
				<xsl:attribute name="ss:Span">
					<xsl:value-of select="$currentColumn/@table:number-columns-repeated - 1" />
				</xsl:attribute>
			</xsl:if>

		   <xsl:if test="$index">
				<xsl:attribute name="ss:Index">
					<xsl:value-of select="$columnNo" />
				</xsl:attribute>
			</xsl:if>

			<xsl:choose>
				<xsl:when test="$currentColumn/@style:use-optimal-column-width = 'true'">
					<xsl:attribute name="ss:AutoFitWidth">1</xsl:attribute>
				</xsl:when>
				<xsl:otherwise>
					<xsl:variable name="width" select="key('styles', $currentColumn/@table:style-name)/style:table-column-properties/@style:column-width" />
					<xsl:if test="$width">
						<xsl:attribute name="ss:Width">
							<!-- using the absolute width in point -->
							<xsl:call-template name="convert2pt">
								<xsl:with-param name="value" select="$width" />
							</xsl:call-template>
						</xsl:attribute>
					</xsl:if>
				</xsl:otherwise>
			</xsl:choose>

			<xsl:if test="$currentColumn/@table:number-columns-repeated">
				<xsl:attribute name="ss:Span">
					<xsl:value-of select="$currentColumn/@table:number-columns-repeated - 1" />
				</xsl:attribute>
			</xsl:if>
		</xsl:element>

		<xsl:if test="$columnNo &lt; $columnCount">
			<xsl:choose>
				<xsl:when test="@table:number-columns-repeated">
					<xsl:call-template name="create-table-column">
						<xsl:with-param name="columnNodes"  select="$columnNodes" />
						<xsl:with-param name="columnCount"  select="$columnCount" />
						<xsl:with-param name="columnNo"     select="$columnNo + $currentColumn/@table:number-columns-repeated" />
						<xsl:with-param name="columnNodeNo" select="$columnNodeNo + 1" />
						<xsl:with-param name="currentColumn" select="$columnNodes[$columnNodeNo + 1]" />
						<xsl:with-param name="index"        select="true()" />
					</xsl:call-template>
				</xsl:when>
				<xsl:otherwise>
					<xsl:call-template name="create-table-column">
						<xsl:with-param name="columnNodes"  select="$columnNodes" />
						<xsl:with-param name="columnCount"  select="$columnCount" />
						<xsl:with-param name="columnNo"     select="$columnNo + 1" />
						<xsl:with-param name="columnNodeNo" select="$columnNodeNo + 1" />
						<xsl:with-param name="currentColumn" select="$columnNodes[$columnNodeNo + 1]" />
					</xsl:call-template>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:if>
	</xsl:template>


	<!-- ************* -->
	<!-- *** Rows  *** -->
	<!-- ************* -->

	<xsl:template match="table:table-row">
		<xsl:param name="columnCount" />

		<xsl:choose>
			<xsl:when test="@table:number-rows-repeated &gt; 1">
				<xsl:choose>
					<xsl:when test="(last() or (last() - 1)) and @table:number-rows-repeated &gt; 99">
						<xsl:call-template name="write-table-row">
							<xsl:with-param name="columnCount"  select="$columnCount" />
							<xsl:with-param name="lastRow"      select="true()" />
						</xsl:call-template>
					</xsl:when>
					<xsl:otherwise>
						<!-- In case a cell is being repeated, the cell will be created
						in a variabel, which is as many times given out, as being repeated -->
						<xsl:variable name="tableRow">
							<xsl:call-template name="write-table-row">
								<xsl:with-param name="columnCount"   select="$columnCount" />
							</xsl:call-template>
						</xsl:variable>
						<xsl:call-template name="repeat-write-table-row">
							<xsl:with-param name="tableRow"     select="$tableRow" />
							<xsl:with-param name="repetition"   select="@table:number-rows-repeated" />
					   </xsl:call-template>
				   </xsl:otherwise>
			   </xsl:choose>
			</xsl:when>
			<xsl:otherwise>
				<xsl:call-template name="write-table-row">
					<xsl:with-param name="columnCount"   select="$columnCount" />
				</xsl:call-template>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name="write-table-row">
		<xsl:param name="columnCount" />
		<xsl:param name="lastRow" />


		<xsl:element namespace="{$namespace}" name="{$rowElement}">
			<xsl:if test="@table:visibility = 'collapse' or @table:visibility = 'filter'">
				<xsl:attribute name="ss:Hidden">1</xsl:attribute>
			</xsl:if>

			<!-- although valid, can not be opened with Excel - issue i31949)
			<xsl:if test="$lastRow">
				<xsl:attribute name="ss:Span"><xsl:value-of select="@table:number-rows-repeated - 1" /></xsl:attribute>
			</xsl:if>-->

			<!-- writing the style of the row -->
			<xsl:apply-templates select="@table:style-name" />


			<xsl:variable name="rowProperties" select="key('styles', @table:style-name)/*" />

			<xsl:if test="$rowProperties/@style:use-optimal-row-height = 'false'">
				<!-- default is '1', therefore write only '0' -->
				<xsl:attribute name="ss:AutoFitHeight">0</xsl:attribute>
			</xsl:if>

			<xsl:variable name="height" select="$rowProperties/@style:row-height" />
			<xsl:if test="$height">
				<xsl:attribute name="ss:Height">
					<!-- using the absolute height in point -->
					<xsl:call-template name="convert2pt">
						<xsl:with-param name="value" select="$height" />
					</xsl:call-template>
				</xsl:attribute>
			</xsl:if>

			<xsl:apply-templates select="table:table-cell">
				<xsl:with-param name="columnCount"      select="$columnCount" />
			</xsl:apply-templates>
		</xsl:element>
	</xsl:template>


	<xsl:template name="repeat-write-table-row">
		<xsl:param name="tableRow" />
		<xsl:param name="repetition" />

		<xsl:copy-of select="$tableRow" />
		<xsl:if test="$repetition &gt; 1">
			<xsl:call-template name="repeat-write-table-row">
				<xsl:with-param name="tableRow"   select="$tableRow" />
				<xsl:with-param name="repetition"  select="$repetition - 1" />
			</xsl:call-template>
		</xsl:if>
	</xsl:template>

	<!-- ************** -->
	<!-- *** Cells  *** -->
	<!-- ************** -->

	<!-- Table cells are able to be repeated by attribute in StarOffice,
		but not in Excel. If more cells are repeated
		(e.g. for emulating background) only as many cells as columns are
		allowed to be written out. -->
	<xsl:template match="table:table-cell">
		<xsl:param name="columnCount" />

		<xsl:choose>
			<xsl:when test="@table:number-columns-repeated &gt; 1">
				<!-- In case a cell is being repeated, the cell will be created
					in a variabel, which is as many times given out, as being repeated -->
				<xsl:variable name="tableCell">
					<xsl:call-template name="write-table-cell" />
				</xsl:variable>
				<xsl:choose>
				<xsl:when test="not(following-sibling::table:table-cell)">
					<xsl:call-template name="repeat-write-table-cell">
						<xsl:with-param name="tableCell"   select="$tableCell" />
						<xsl:with-param name="repetition"  select="@table:number-columns-repeated" />
						<xsl:with-param name="columnCount" select="$columnCount" />
						<xsl:with-param name="cellNo"      select="position()
								+ sum(preceding-sibling::table:table-cell/@table:number-columns-repeated)
								- count(preceding-sibling::table:table-cell/@table:number-columns-repeated)" />
				   </xsl:call-template>
				</xsl:when>
				<xsl:otherwise>
					<xsl:call-template name="repeat-write-table-cell">
						<xsl:with-param name="tableCell"   select="$tableCell" />
						<xsl:with-param name="repetition"  select="@table:number-columns-repeated" />
				   </xsl:call-template>
				</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<xsl:otherwise>
				<xsl:call-template name="write-table-cell" />
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name="repeat-write-table-cell">
		<xsl:param name="tableCell" />
		<xsl:param name="repetition" />
		<xsl:param name="columnCount" />
		<xsl:param name="cellNo" />

		<xsl:copy-of select="$tableCell" />
		<xsl:if test="$repetition &gt; 1">
			<xsl:choose>
				<xsl:when test="$cellNo">
					<xsl:if test="$cellNo  &lt; $columnCount">
						<xsl:call-template name="repeat-write-table-cell">
							<xsl:with-param name="tableCell"   select="$tableCell" />
							<xsl:with-param name="repetition"  select="$repetition - 1" />
							<xsl:with-param name="columnCount" select="$columnCount" />
							<xsl:with-param name="cellNo"      select="$cellNo + 1" />
						</xsl:call-template>
					</xsl:if>
				</xsl:when>
				<xsl:otherwise>
					<xsl:call-template name="repeat-write-table-cell">
						<xsl:with-param name="tableCell"   select="$tableCell" />
						<xsl:with-param name="repetition"  select="$repetition - 1" />
					</xsl:call-template>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:if>
	</xsl:template>


	<xsl:template name="write-table-cell">
		<xsl:element namespace="{$namespace}" name="{$cellElement}">
			 <xsl:if test="@table:number-columns-spanned &gt; 1">
				<xsl:attribute name="ss:MergeAcross">
					<xsl:value-of select="@table:number-columns-spanned - 1" />
				</xsl:attribute>
			</xsl:if>
			<xsl:if test="@table:number-rows-spanned &gt; 1">
				<xsl:attribute name="ss:MergeDown">
					<xsl:value-of select="@table:number-rows-spanned - 1" />
				</xsl:attribute>
			</xsl:if>
			<xsl:variable name="link" select="descendant::text:a/@xlink:href" />
			<xsl:if test="$link">
				<xsl:attribute name="ss:HRef">
					<xsl:value-of select="$link" />
				</xsl:attribute>
			</xsl:if>
			<xsl:choose>
				<xsl:when test="@table:style-name">
					<xsl:apply-templates select="@table:style-name" />
				</xsl:when>
				<xsl:otherwise>
					<!-- Currently it only takes the first default style, in case of multiple it fails -->
					<xsl:apply-templates select="ancestor::table:table/table:table-column/@table:default-cell-style-name" />
				</xsl:otherwise>
			</xsl:choose>
			<xsl:apply-templates select="@table:formula" />

			<xsl:choose>
				<xsl:when test="*">
				<!-- in case it is not an empty cell -->

					<!--
					  As the sequence of comment and data is opposite in Excel and Calc no match work here, in both comments exist only once
					  Possible Table Content of interest: text:h|text:p|text:list  -->
					<xsl:if test="text:h | text:p | text:list">
						<xsl:variable name="valueType">
							<xsl:choose>
								<xsl:when test="@office:value-type">
									<xsl:value-of select="@office:value-type" />
								</xsl:when>
								<xsl:otherwise>string</xsl:otherwise>
							</xsl:choose>
						</xsl:variable>
						<xsl:call-template name="ss:Data">
							<xsl:with-param name="valueType" select="$valueType" />
							<xsl:with-param name="cellStyleName" select="@table:style-name" />
						</xsl:call-template>
					</xsl:if>

					<xsl:if test="office:annotation">
						<xsl:element name="Comment">
							<xsl:if test="office:annotation/@office:author">
								<xsl:attribute name="ss:Author"><xsl:value-of select="office:annotation/@office:author" /></xsl:attribute>
							</xsl:if>
							<xsl:if test="office:annotation/@office:display = 'true'">
								<xsl:attribute name="ss:ShowAlways">1</xsl:attribute>
							</xsl:if>
							<!-- ss:Data is oblicatory, but not the same as the ss:Cell ss:Data child, as it has no attributes  -->
							<ss:Data xmlns="http://www.w3.org/TR/REC-html40">
								<xsl:for-each select="office:annotation/text:p">
									<xsl:choose>
										<xsl:when test="*">
											<!-- paragraph style have to be neglected due to Excel error,
												which does not allow shadowing their HTML attributes -->
											<xsl:for-each select="*">
												<xsl:call-template name="style-and-contents" />
											</xsl:for-each>
										</xsl:when>
										<xsl:when test="@text:style-name">
											<xsl:call-template name="style-and-contents" />
										</xsl:when>
										<xsl:otherwise>
											<!-- if no style is set, BOLD is set as default -->
											<B>
												<xsl:call-template name="style-and-contents" />
											</B>
										</xsl:otherwise>
									</xsl:choose>
								</xsl:for-each>
							</ss:Data>
						</xsl:element>
					</xsl:if>
				</xsl:when>
<!--            <xsl:otherwise>
					<!~~ Excel can not handle empty cell tags
					<Data ss:Type="String">&#160;</Data>
				</xsl:otherwise>-->
			</xsl:choose>
		</xsl:element>
	</xsl:template>

	<!-- comments are handled separately in the cell -->
	<xsl:template match="office:annotation" />
	<xsl:template match="dc:date" />

	<xsl:template name="ss:Data">
		<!-- the default value is 'String' in the office -->
		<xsl:param name="valueType" select="'string'" />
		<xsl:param name="cellStyleName" />

		<xsl:choose>
			<xsl:when test="descendant::*/@text:style-name">
				<xsl:choose>
					<xsl:when test="$valueType = 'string'">
						<ss:Data ss:Type="String" xmlns="http://www.w3.org/TR/REC-html40">
							<xsl:apply-templates>
								<xsl:with-param name="cellStyleName" select="$cellStyleName" />
							</xsl:apply-templates>
						</ss:Data>
					</xsl:when>
					<xsl:when test="$valueType = 'boolean'">
						<ss:Data ss:Type="Boolean" xmlns="http://www.w3.org/TR/REC-html40">
							<xsl:apply-templates>
								<xsl:with-param name="cellStyleName" select="$cellStyleName" />
							</xsl:apply-templates>
						</ss:Data>
					</xsl:when>
					<xsl:when test="$valueType = 'date'">
						<ss:Data ss:Type="DateTime" xmlns="http://www.w3.org/TR/REC-html40">
							<xsl:apply-templates>
								<xsl:with-param name="cellStyleName" select="$cellStyleName" />
							</xsl:apply-templates>
						</ss:Data>
					</xsl:when>
					<!-- float, time, percentage, currency (no 'Error' setting) -->
					<xsl:otherwise>
						<ss:Data ss:Type="Number" xmlns="http://www.w3.org/TR/REC-html40">
							<xsl:apply-templates>
								<xsl:with-param name="cellStyleName" select="$cellStyleName" />
							</xsl:apply-templates>
						</ss:Data>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<xsl:otherwise>
				<xsl:element name="Data">
					<xsl:call-template name="ss:Type">
						<xsl:with-param name="valueType" select="$valueType" />
					</xsl:call-template>
				</xsl:element>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>


	<xsl:template name="ss:Type">
		<xsl:param name="valueType" select="'string'" />

		<xsl:choose>
			<xsl:when test="$valueType = 'string'">
				<xsl:attribute name="ss:Type">String</xsl:attribute>
				<xsl:apply-templates select="*"/>
			</xsl:when>
			<xsl:when test="$valueType = 'boolean'">
				<xsl:attribute name="ss:Type">Boolean</xsl:attribute>
				<xsl:choose>
					<xsl:when test="@office:boolean-value = 'true'">1</xsl:when>
					<xsl:otherwise>0</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<xsl:when test="$valueType = 'date' or $valueType = 'time'">
				<!-- issue in Excel: can not have an empty 'DateTime' cell -->
				<xsl:attribute name="ss:Type">DateTime</xsl:attribute>
				<!-- Gathering information of two StarOffice date/time attributes
				Excel always needs both informations in one attribute -->
				<xsl:choose>
					<xsl:when test="@office:date-value">
					<!-- office:date-value may contain time (after 'T')-->
						<xsl:choose>
							<xsl:when test="contains(@office:date-value, 'T')">
								<!-- in case time is also part of the date -->
								<xsl:value-of select="substring-before(@office:date-value, 'T')" />
								<xsl:text>T</xsl:text>
								<xsl:value-of select="substring-after(@office:date-value,'T')" />
							   <xsl:if test="not(contains(@office:date-value,'.'))">
									<xsl:text>.</xsl:text>
								</xsl:if>
								<xsl:text>000</xsl:text>
							</xsl:when>
							<xsl:when test="@office:time-value">
							<!-- conatains date and time (time will be evaluated later -->
								<xsl:value-of select="@office:date-value" />
								<xsl:text>T</xsl:text>
								<xsl:choose>
									<xsl:when test="@table:formula or contains(@office:time-value,',')">
										<!-- customized number types not implemented yet -->
										<xsl:text>12:00:00.000</xsl:text>
									</xsl:when>
									<xsl:otherwise>
										<xsl:value-of select="translate(substring-after(@office:time-value,'PT'),'HMS','::.')" />
										<xsl:if test="not(contains(@office:time-value,'S'))">
											<xsl:text>.</xsl:text>
										</xsl:if>
										<xsl:text>000</xsl:text>
									</xsl:otherwise>
								</xsl:choose>
							</xsl:when>
							<xsl:otherwise>
								<xsl:value-of select="@office:date-value" />
								<xsl:text>T12:00:00.000</xsl:text>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:when>
					<xsl:otherwise>
						<xsl:if test="@office:time-value">
							<xsl:text>1899-12-31T</xsl:text>
							<xsl:choose>
								<xsl:when test="@table:formula or contains(@office:time-value,',')">
									<!-- customized number types not implemented yet -->
									<xsl:text>12:00:00.000</xsl:text>
								</xsl:when>
								<xsl:otherwise>
									<xsl:value-of select="translate(substring-after(@office:time-value,'PT'),'HMS','::.')" />
								   <xsl:if test="not(contains(@office:time-value,'S'))">
										<xsl:text>.</xsl:text>
									</xsl:if>
									<xsl:text>000</xsl:text>
								</xsl:otherwise>
							</xsl:choose>
						</xsl:if>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<!-- float, percentage, currency (no 'Error' setting) -->
			<xsl:otherwise>
				<xsl:attribute name="ss:Type">Number</xsl:attribute>
				<xsl:value-of select="@office:value" />
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>


	<!-- ******************** -->
	<!-- *** Common Rules *** -->
	<!-- ******************** -->

	<xsl:template match="*">
		<xsl:param name="cellStyleName" />

		<xsl:call-template name="style-and-contents">
			<xsl:with-param name="cellStyleName" select="$cellStyleName" />
		</xsl:call-template>
	</xsl:template>

	<xsl:template match="text:s">
		<xsl:call-template name="write-breakable-whitespace">
			<xsl:with-param name="whitespaces" select="@text:c" />
		</xsl:call-template>
	</xsl:template>

	<!--write the number of 'whitespaces' -->
	<xsl:template name="write-breakable-whitespace">
		<xsl:param name="whitespaces" />

		<xsl:text> </xsl:text>
		<xsl:if test="$whitespaces >= 1">
			<xsl:call-template name="write-breakable-whitespace">
				<xsl:with-param name="whitespaces" select="$whitespaces - 1" />
			</xsl:call-template>
		</xsl:if>
	</xsl:template>

	<!-- allowing all matched text nodes -->
	<xsl:template match="text()"><xsl:value-of select="." /></xsl:template>
</xsl:stylesheet>

