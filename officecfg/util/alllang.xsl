<?xml version="1.0" encoding="utf-8" standalone="yes" ?>
<!--***********************************************************************
 *
 *  $RCSfile: alllang.xsl,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 17:08:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************ -->

<xsl:transform 	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
		xmlns:xs="http://www.w3.org/2001/XMLSchema"
		xmlns:oor="http://openoffice.org/2001/registry"
		xmlns:filehelper="http://www.jclark.com/xt/java/org.openoffice.configuration.FileHelper"
		extension-element-prefixes="filehelper">

<!-- Get the correct format -->
<xsl:output method="xml" indent="yes" />

<!--************************** PARAMETER ******************************** -->
<xsl:param name="locale"/>
<xsl:param name="xcs"/>
<xsl:param name="schemaRoot">.</xsl:param>
<xsl:param name="fallback-locale">en-US</xsl:param>

<xsl:variable name="schemaRootURL"><xsl:value-of select="filehelper:makeAbs($schemaRoot)"/></xsl:variable>
<xsl:variable name="schemaURL"><xsl:value-of select="filehelper:makeAbs($xcs)"/></xsl:variable>

<!--************************** TEMPLATES ******************************** -->
<!-- ensure that at least root is available -->
	<xsl:template match="/oor:node">
		<xsl:copy>
			<xsl:choose>
				<xsl:when test="string-length($locale)">
			        <xsl:apply-templates select = "@*" mode="locale"/>
					<xsl:apply-templates mode="locale"/>
				</xsl:when>
				<xsl:otherwise>
			        <xsl:apply-templates select = "@*" mode="non-locale"/>
					<xsl:for-each select="node|prop">
						<xsl:variable name="component-schema" select="document($schemaURL)/oor:component-schema"/>
						<xsl:apply-templates select="." mode="non-locale">
							<xsl:with-param name="component-schema" select="$component-schema"/>
							<xsl:with-param name="context" select="$component-schema/component/*[@oor:name = current()/@oor:name]"/>
						</xsl:apply-templates>
					</xsl:for-each>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:copy>
	</xsl:template>

<!-- locale dependent data -->
	<xsl:template match="node|prop" mode = "locale">
        <xsl:choose>
            <xsl:when test="$locale=$fallback-locale">
                <xsl:if test="count(descendant::value[@xml:lang=$locale]/../value[not (@xml:lang)])">
  			        <xsl:copy>
				        <xsl:apply-templates select = "@*" mode="locale"/>
				        <xsl:apply-templates mode = "locale"/>
			        </xsl:copy>
                </xsl:if>
            </xsl:when>
            <xsl:otherwise>
                <xsl:if test="count(descendant::value[@xml:lang = $locale])">
  			        <xsl:copy>
				        <xsl:apply-templates select = "@*" mode="locale"/>
				        <xsl:apply-templates mode = "locale"/>
			        </xsl:copy>
                </xsl:if>
            </xsl:otherwise>
        </xsl:choose>
	</xsl:template>

	<xsl:template match="value" mode="locale">
		<xsl:if test="@xml:lang = $locale">
			<xsl:copy>
				<xsl:apply-templates select = "@*" mode="locale"/>
				<xsl:value-of select="."/>
			</xsl:copy>
		</xsl:if>
	</xsl:template>

	<xsl:template match = "@*" mode="locale">
		<xsl:copy/>
	</xsl:template>

    <!-- suppress all merge instructions -->
	<xsl:template match = "@oor:op" mode="locale"/>

<!-- locale independent data -->
	<xsl:template match="node" mode="non-locale">
		<xsl:param name = "context"/>
		<xsl:param name = "component-schema"/>

		<xsl:if test="count(descendant::value[(not (@xml:lang)) or (@xml:lang=$fallback-locale)]) or count(descendant-or-self::*[(@oor:finalized='true') or (@oor:mandatory='true') or (@oor:op='replace') or (@oor:op='remove')])">
			<xsl:copy>
				<xsl:apply-templates select = "@*"  mode="non-locale"/>
				<xsl:choose>
					<!-- look for matching templates in other components -->
					<xsl:when test="$context/@oor:node-type and $context/@oor:component">
						<xsl:variable name="fileURL">
							<xsl:call-template name="composeFileURL">
								<xsl:with-param name="component-schema" select="$component-schema"/>
								<xsl:with-param name="componentName"><xsl:value-of select="$context/@oor:component"/></xsl:with-param>
							</xsl:call-template>
						</xsl:variable>
						<xsl:for-each select="node|prop">
							<xsl:apply-templates select="." mode="non-locale">
								<xsl:with-param name="component-schema" select="$component-schema"/>
								<xsl:with-param name="context" select="document($fileURL)/oor:component-schema/templates/*[@oor:name = $context/@oor:node-type]"/>
							</xsl:apply-templates>
						</xsl:for-each>
					</xsl:when>
					<!-- look for matching templates within the same component -->
					<xsl:when test="$context/@oor:node-type">
						<xsl:for-each select="node|prop">
							<xsl:apply-templates select="." mode="non-locale">
								<xsl:with-param name="component-schema" select="$component-schema"/>
								<xsl:with-param name="context" select="$component-schema/templates/*[@oor:name = $context/@oor:node-type]"/>
							</xsl:apply-templates>
						</xsl:for-each>
					</xsl:when>
					<xsl:otherwise>
						<xsl:for-each select="node|prop">
							<xsl:apply-templates select="." mode="non-locale">
								<xsl:with-param name="component-schema" select="$component-schema"/>
								<xsl:with-param name="context" select="$context/*[@oor:name = current()/@oor:name]"/>
							</xsl:apply-templates>
						</xsl:for-each>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:copy>
		</xsl:if>
	</xsl:template>

	<xsl:template match="prop" mode="non-locale">
		<xsl:param name = "context"/>
		<xsl:choose>
			<xsl:when test="not ($context) or @oor:finalized='true'">
				<xsl:copy>
					<xsl:apply-templates select = "@*" mode="non-locale"/>
					<xsl:apply-templates select = "value" mode="non-locale"/>
				</xsl:copy>
			</xsl:when>
			<xsl:when test="count (value[not (@xml:lang)])">
			    <!-- copy locale independent values only, if the values differ -->
				<xsl:variable name="isRedundant">
					<xsl:call-template name="isRedundant">
						<xsl:with-param name="schemaval"  select="$context/value"/>
						<xsl:with-param name="dataval" select="value[not (@xml:lang)]"/>
					</xsl:call-template>
				</xsl:variable>
				<xsl:if test="$isRedundant ='false'">
					<xsl:copy>
						<xsl:apply-templates select = "@*" mode="non-locale"/>
						<xsl:apply-templates select = "value" mode="non-locale"/>
					</xsl:copy>
				</xsl:if>
			</xsl:when>
			<xsl:otherwise>
				<xsl:copy>
					<xsl:apply-templates select = "@*" mode="non-locale"/>
					<xsl:apply-templates select = "value" mode="fallback-locale"/>
				</xsl:copy>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template match="value" mode="non-locale">
		<xsl:if test="not (@xml:lang)">
			<xsl:copy>
				<xsl:apply-templates select = "@*" mode="non-locale"/>
				<xsl:value-of select="."/>
			</xsl:copy>
		</xsl:if>
	</xsl:template>

	<xsl:template match="value" mode="fallback-locale">
		<xsl:if test="@xml:lang = $fallback-locale">
			<xsl:copy>
				<xsl:apply-templates select = "@*" mode="non-locale"/>
				<xsl:value-of select="."/>
			</xsl:copy>
		</xsl:if>
	</xsl:template>

	<xsl:template match = "@*" mode="non-locale">
		<xsl:copy/>
	</xsl:template>

<!-- compares two values -->
	<xsl:template name="isRedundant">
		<xsl:param name = "schemaval"/>
		<xsl:param name = "dataval"/>
		<xsl:choose>
			<xsl:when test="not ($dataval)">
				<xsl:value-of select="true()"/>
			</xsl:when>
			<xsl:when test="not ($schemaval)">
				<xsl:choose>
					<xsl:when test="$dataval/@xsi:nil='true'">
						<xsl:value-of select="true()"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="false()"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<xsl:when test="$schemaval = $dataval">
				<xsl:value-of select="true()"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="false()"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name="composeFileURL">
		<xsl:param name="componentName"/>
		<xsl:variable name="fileURL">
			<xsl:value-of select="$schemaRootURL"/>/<xsl:value-of select="translate($componentName,'.','/')"/>.xcs
		</xsl:variable>
		<xsl:value-of select="$fileURL"/>
	</xsl:template>

</xsl:transform>
