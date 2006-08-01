<?xml version="1.0" encoding="UTF-8"?>
<!--

    OpenOffice.org - a multi-platform office productivity suite

    $RCSfile: sofftohtml.xsl,v $

    $Revision: 1.3 $

    last change: $Author: ihi $ $Date: 2006-08-01 12:43:18 $

    The Contents of this file are made available subject to
    the terms of GNU Lesser General Public License Version 2.1.


      GNU Lesser General Public License Version 2.1
      =============================================
      Copyright 2005 by Sun Microsystems, Inc.
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

-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:style="http://openoffice.org/2000/style" xmlns:table="http://openoffice.org/2000/table" xmlns:text="http://openoffice.org/2000/text" xmlns:office="http://openoffice.org/2000/office" xmlns:fo="http://www.w3.org/1999/XSL/Format">
	<xsl:output method="xml" omit-xml-declaration="yes" indent="yes" encoding="ISO-8859-1"/>
	<!--doctype-system=[<!ENTITY acirc "">] -->
	<xsl:strip-space elements="tokens"/>
	<xsl:template match="office:document">
		<html>
			<xsl:apply-templates/>
		</html>
	</xsl:template>
	<xsl:template match="office:document-content">
		<html>
			<xsl:apply-templates/>
		</html>
	</xsl:template>
	<xsl:template match="office:automatic-styles">
		<style type="text/css">p.Table-Heading{font-weight:bold;}<xsl:apply-templates/>
		</style>
	</xsl:template>
	<xsl:template match="office:styles"/>
	<xsl:template match="office:meta"/>
	<xsl:template match="office:settings"/>
	<xsl:template match="style:style">
		<xsl:if test="@style:family ='paragraph'">p.<xsl:value-of select="@style:name"/>{<xsl:apply-templates/>}</xsl:if>
		<xsl:if test="@style:family ='paragraph'">p.<xsl:value-of select="@style:name"/>{<xsl:if test="@style:parent-style-name='Table Heading'">font-weight:bold;font-style:italic;</xsl:if>
			<xsl:apply-templates/>}</xsl:if>
		<xsl:if test="@style:family ='table-cell'">td.<xsl:value-of select="@style:name"/>{<xsl:if test="@style:parent-style-name='Table Heading'">font-weight:bold;font-style:italic;</xsl:if>
			<xsl:apply-templates/>}</xsl:if>
	</xsl:template>
	<xsl:template match="style:properties">
		<!--<xsl:param name="style" select="@fo:font-weight"/>-->
		<xsl:if test="@fo:font-weight">font-weight:<xsl:value-of select="@fo:font-weight"/>;</xsl:if>
		<xsl:if test="@fo:font-style">font-style:<xsl:value-of select="@fo:font-style"/>;</xsl:if>
		<xsl:if test="@style:font-name">font-family:<xsl:value-of select="@style:font-name"/>;</xsl:if>
		<xsl:if test="@fo:font-size">font-size:<xsl:value-of select="@fo:font-size"/>;</xsl:if>
		<xsl:if test="@style:text-underline='single'">text-decoration:underline;</xsl:if>
		<xsl:if test="@style:text-crossing-out='single-line'">text-decoration:line-through;</xsl:if>
		<xsl:if test="@fo:text-align='start'">text-align:left</xsl:if>
		<xsl:if test="@fo:text-align='center'">text-align:center</xsl:if>
		<xsl:if test="@fo:text-align='end'">text-align:right</xsl:if>
		<!--<xsl:value-of select="$style"/>-->
	</xsl:template>
	<xsl:template match="office:body">
		<xsl:apply-templates/>
	</xsl:template>
	<xsl:template match="table:table">
		<table border="1" cellpadding="2" width="100%">
			<xsl:apply-templates/>
		</table>
	</xsl:template>
	<xsl:template match="table:table-header-rows">
		<th>
			<xsl:apply-templates/>
		</th>
	</xsl:template>
	<xsl:template match="table:table-row">
		<tr>
			<xsl:apply-templates/>
		</tr>
	</xsl:template>
	<xsl:template match="table:table-cell">
		<xsl:text disable-output-escaping="yes">&lt;td class="</xsl:text>
		<xsl:value-of select="@table:style-name"/>
		<xsl:text disable-output-escaping="yes">"&gt;</xsl:text>
		<!--<xsl:value-of select="."/>-->
		<xsl:apply-templates/>
		<xsl:text disable-output-escaping="yes">&lt;/td&gt;</xsl:text>
		<!--<td width="20%">
		<xsl:apply-templates />
		</td>-->
	</xsl:template>
	<xsl:template match="text:p">
		<xsl:if test="ancestor-or-self::table:table-cell">
			<xsl:if test=".=''">
				<br/>
			</xsl:if>
		</xsl:if>
		<xsl:text disable-output-escaping="yes">&lt;p class="</xsl:text>
		<xsl:choose>
			<xsl:when test="@text:style-name ='Table Heading'">Table-Heading</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="@text:style-name"/>
			</xsl:otherwise>
		</xsl:choose>
		<xsl:text disable-output-escaping="yes">"&gt;</xsl:text>
		<xsl:apply-templates/>
		<xsl:text disable-output-escaping="yes">&lt;/p&gt;</xsl:text>
		<!--<xsl:value-of select="."/>-->
		<!--<xsl:text disable-output-escaping="yes">&amp;nbsp;</xsl:text>-->
		<!--<br/>-->
	</xsl:template>
</xsl:stylesheet>
