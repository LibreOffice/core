<?xml version="1.0" encoding="utf-8" standalone="yes" ?>
<!--
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 -->

<xsl:transform 	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0"
		xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
		xmlns:xs="http://www.w3.org/2001/XMLSchema"
		xmlns:oor="http://openoffice.org/2001/registry"
		xmlns:install="http://openoffice.org/2004/installation"
        	exclude-result-prefixes="install">

<!-- Get the correct format -->
<xsl:output method="xml" indent="yes" encoding="UTF-8"/>

<!--************************** PARAMETER ******************************** -->
<xsl:param name="locale"/>
<xsl:param name="module"/>
<xsl:param name="xcs"/>
<xsl:param name="schemaRoot">.</xsl:param>
<xsl:param name="fallback-locale">en-US</xsl:param>

<xsl:param name="LIBO_SHARE_FOLDER"/>

<xsl:variable name="schemaRootURL">
    <xsl:value-of select="$schemaRoot"/>
</xsl:variable>
<xsl:variable name="schemaURL">
    <xsl:value-of select="$xcs"/>
</xsl:variable>

<!--************************** TEMPLATES ******************************** -->
<!-- ensure that at least root is available -->
	<xsl:template match="/oor:component-data">
		<xsl:copy>
			<xsl:choose>
				<xsl:when test="string-length($locale)">
			        <xsl:apply-templates select = "@*" mode="locale"/>
					<xsl:apply-templates select = "node|prop" mode="locale"/>
				</xsl:when>
				<xsl:otherwise>
			        <xsl:apply-templates select = "@*"/>
					<xsl:for-each select="node|prop">
						<xsl:variable name="component-schema" select="document($schemaURL)/oor:component-schema"/>
						<xsl:apply-templates select=".">
							<xsl:with-param name="component-schema" select="$component-schema"/>
							<xsl:with-param name="context" select="$component-schema/component/*[@oor:name = current()/@oor:name]"/>
							<xsl:with-param name="find-module" select="$module"/>
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
                <xsl:if test="descendant::value[@xml:lang=$locale]/../value[not (@xml:lang)]">
  			        <xsl:copy>
				        <xsl:apply-templates select = "@*" mode="locale"/>
				        <xsl:apply-templates select = "node|prop|value" mode = "locale"/>
			        </xsl:copy>
                </xsl:if>
            </xsl:when>
            <xsl:otherwise>
                <xsl:if test="descendant::value[@xml:lang = $locale]">
  			        <xsl:copy>
				        <xsl:apply-templates select = "@*" mode="locale"/>
				        <xsl:apply-templates select = "node|prop|value" mode = "locale"/>
			        </xsl:copy>
                </xsl:if>
            </xsl:otherwise>
        </xsl:choose>
	</xsl:template>

	<xsl:template match="value" mode="locale">
		<xsl:if test="@xml:lang=$locale and not(@install:module)">
			<xsl:copy>
				<xsl:apply-templates select = "@*" mode="locale"/>
				<xsl:apply-templates select="node()"/>
			</xsl:copy>
		</xsl:if>
	</xsl:template>

	<xsl:template match = "@*" mode="locale">
		<xsl:copy/>
	</xsl:template>

    <!-- suppress all merge instructions -->
	<xsl:template match = "@oor:op" mode="locale"/>

    <!-- suppress all module markers -->
	<xsl:template match = "@install:module" mode="locale"/>

<!-- locale independent data -->

    <!-- handle template references      -->
	<xsl:template name="copy-resolve-template">
		<xsl:param name = "node-type"/>
		<xsl:param name = "schema-type"/>
		<xsl:param name = "component-schema"/>
		
        <xsl:choose>
            <xsl:when test="$schema-type='node-ref'">
                <xsl:apply-templates select=".">				
                    <xsl:with-param name="context" select="$component-schema/templates/*[@oor:name = $node-type]"/>	
                    <xsl:with-param name="component-schema" select="$component-schema"/>							
                </xsl:apply-templates>
            </xsl:when>
            <xsl:when test="$schema-type='set'">
                <xsl:copy>
                    <xsl:apply-templates select = "@*" />
                    <xsl:for-each select="node|prop">						
                        <xsl:apply-templates select=".">				
                            <xsl:with-param name="context" select="$component-schema/templates/*[@oor:name = $node-type]"/>	
                            <xsl:with-param name="component-schema" select="$component-schema"/>							
                        </xsl:apply-templates>
                    </xsl:for-each>
                </xsl:copy>
            </xsl:when>
            <xsl:otherwise>
                <xsl:message terminate="yes">ERROR: The schema element for a <xsl:value-of select="$schema-type"/> 
                                                should not have a node-type.
                </xsl:message>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    
	<xsl:template name="copy-node">
		<xsl:param name = "context"/>
		<xsl:param name = "component-schema"/>

        <xsl:choose>
            <!-- look for matching templates in other components -->
            <xsl:when test="$context/@oor:node-type and $context/@oor:component">
                <xsl:variable name="fileURL">
                    <xsl:call-template name="composeFileURL">
                        <xsl:with-param name="component-schema" select="$component-schema"/>
                        <xsl:with-param name="componentName"><xsl:value-of select="$context/@oor:component"/></xsl:with-param>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:call-template name="copy-resolve-template">
                    <xsl:with-param name="node-type" select="$context/@oor:node-type"/>
                    <xsl:with-param name="schema-type" select="local-name($context)"/>
                    <xsl:with-param name="component-schema" select="document($fileURL)/oor:component-schema"/>
                </xsl:call-template>
            </xsl:when>
            <!-- look for matching templates within the same component -->
            <xsl:when test="$context/@oor:node-type">
                <xsl:call-template name="copy-resolve-template">
                    <xsl:with-param name="node-type" select="$context/@oor:node-type"/>
                    <xsl:with-param name="schema-type" select="local-name($context)"/>
                    <xsl:with-param name="component-schema" select="$component-schema"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <xsl:copy>
                    <xsl:apply-templates select = "@*" />
                    <xsl:for-each select="node|prop">
                        <xsl:apply-templates select=".">
                            <xsl:with-param name="component-schema" select="$component-schema"/>
                            <xsl:with-param name="context" select="$context/*[@oor:name = current()/@oor:name]"/>
                        </xsl:apply-templates>
                    </xsl:for-each>
                </xsl:copy>
            </xsl:otherwise>
        </xsl:choose>
	</xsl:template>
    
	<xsl:template match="node">
		<xsl:param name = "context"/>
		<xsl:param name = "component-schema"/>

        <xsl:variable name="applicable-values" select="descendant::value[not (@xml:lang) or (@xml:lang=$fallback-locale) or (@install:module=$module)]"/>
        <xsl:variable name="substantive-nodes" select="descendant-or-self::*[(@oor:finalized='true') or (@oor:mandatory='true') or (@oor:op!='modify')]"/>

        <xsl:choose>
            <!-- go ahead, if we are in the active module -->
            <xsl:when test="ancestor-or-self::*/@install:module=$module">
                <xsl:if test="$applicable-values | $substantive-nodes">
                    <xsl:call-template name="copy-node">
                        <xsl:with-param name="component-schema" select="$component-schema"/>
                        <xsl:with-param name="context" select="$context"/>
                    </xsl:call-template>
                </xsl:if>
			</xsl:when>
            <!-- strip data from wrong module -->
            <xsl:when test="ancestor-or-self::*/@install:module"/>
            <!-- looking for module -->
            <xsl:when test="$module">
                <xsl:if test="($applicable-values | $substantive-nodes)/ancestor-or-self::*/@install:module=$module">
                    <xsl:call-template name="copy-node">
                        <xsl:with-param name="component-schema" select="$component-schema"/>
                        <xsl:with-param name="context" select="$context"/>
                    </xsl:call-template>
                </xsl:if>
			</xsl:when>
            <!-- copying non-module data -->
            <xsl:otherwise>
                <xsl:if test="($applicable-values | $substantive-nodes)[not(ancestor-or-self::*/@install:module)]">
                    <xsl:call-template name="copy-node">
                        <xsl:with-param name="component-schema" select="$component-schema"/>
                        <xsl:with-param name="context" select="$context"/>
                    </xsl:call-template>
                </xsl:if>
            </xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template match="prop">
		<xsl:param name = "context"/>
		<xsl:choose>
            <xsl:when test="$module and not((ancestor-or-self::* | child::value)/@install:module=$module)"/>
            <xsl:when test="not($module) and ancestor-or-self::*/@install:module"/>
			<xsl:when test="not ($context) or @oor:finalized='true' or @oor:op!='modify'">
				<xsl:copy>
					<xsl:apply-templates select = "@*"/>
					<xsl:apply-templates select = "value"/>
				</xsl:copy>
			</xsl:when>
			<xsl:when test="value[not (@xml:lang) or @install:module]">
                <xsl:if test="value[not(@install:module) or @install:module=$module]">
                    <!-- copy locale independent values only, if the values differ -->
                    <xsl:variable name="isRedundant">
                        <xsl:call-template name="isRedundant">
                            <xsl:with-param name="schemaval"  select="$context/value"/>
                            <xsl:with-param name="dataval" select="value[(not(@xml:lang) or @install:module) and (not(@install:module) or @install:module=$module)]"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:if test="$isRedundant ='false'">
                        <xsl:copy>
                            <xsl:apply-templates select = "@*"/>
                            <xsl:apply-templates select = "value"/>
                        </xsl:copy>
                        </xsl:if>
				</xsl:if>
			</xsl:when>
			<xsl:otherwise>
				<xsl:copy>
					<xsl:apply-templates select = "@*"/>
					<xsl:apply-templates select = "value" mode="fallback-locale"/>
				</xsl:copy>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template match="value">
		<xsl:choose>
            <xsl:when test="@xml:lang and not(@install:module)"/>
            <xsl:when test="$module and not(ancestor-or-self::*/@install:module=$module)"/>
            <xsl:when test="not($module) and ancestor-or-self::*/@install:module"/>
            <xsl:otherwise>
                <xsl:copy>
                    <xsl:apply-templates select = "@*|node()"/>
                </xsl:copy>
            </xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template match="value" mode="fallback-locale">
		<xsl:if test="@xml:lang=$fallback-locale and not(@install:module)">
            <xsl:copy>
                <xsl:apply-templates select = "@*|node()"/>
            </xsl:copy>
		</xsl:if>
	</xsl:template>

        <xsl:template match ="it|unicode">
          <xsl:copy>
            <xsl:apply-templates select = "@*|node()"/>
          </xsl:copy>
        </xsl:template>

	<xsl:template match = "@*">
		<xsl:copy/>
	</xsl:template>

    <!-- suppress all merge instructions, that are out-of-module -->
	<xsl:template match = "@oor:op">
        <xsl:if test="not($module) or ancestor::*/@install:module">
            <xsl:copy/>
		</xsl:if>
	</xsl:template>

    <!-- suppress all module markers -->
	<xsl:template match = "@install:module"/>

<!-- compares two values -->
	<xsl:template name="isRedundant">
		<xsl:param name = "schemaval"/>
		<xsl:param name = "dataval"/>
		<xsl:choose>
			<xsl:when test="not ($dataval)">
				<xsl:value-of select="true()"/>
			</xsl:when>
            <xsl:when test="$dataval/@oor:external">
                <xsl:value-of select="false()"/>
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
			<xsl:when test="$schemaval != $dataval">
				<xsl:value-of select="false()"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="true()"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name="composeFileURL">
		<xsl:param name="componentName"/>
		<xsl:variable name="fileURL">
			<xsl:value-of select="$schemaRootURL"/>/<xsl:value-of select="translate($componentName,'.','/')"/><xsl:text>.xcs</xsl:text>
		</xsl:variable>
		<xsl:value-of select="$fileURL"/>
	</xsl:template>

        <xsl:template match="@oor:name">
          <xsl:attribute name="oor:name">
            <xsl:call-template name="replace">
              <xsl:with-param name="input" select="current()"/>
              <xsl:with-param name="pattern" select="'@LIBO_SHARE_FOLDER@'"/>
              <xsl:with-param name="replace" select="$LIBO_SHARE_FOLDER"/>
            </xsl:call-template>
          </xsl:attribute>
        </xsl:template>

        <xsl:template match="text()">
          <xsl:call-template name="replace">
            <xsl:with-param name="input" select="current()"/>
            <xsl:with-param name="pattern" select="'@LIBO_SHARE_FOLDER@'"/>
            <xsl:with-param name="replace" select="$LIBO_SHARE_FOLDER"/>
          </xsl:call-template>
        </xsl:template>

        <xsl:template name="replace">
          <xsl:param name="input"/>
          <xsl:param name="pattern"/>
          <xsl:param name="replace"/>
          <xsl:choose>
            <xsl:when test="contains($input, $pattern)">
              <xsl:value-of select="substring-before($input, $pattern)"/>
              <xsl:value-of select="$replace"/>
              <xsl:call-template name="replace">
                <xsl:with-param
                    name="input" select="substring-after($input, $pattern)"/>
                <xsl:with-param name="pattern" select="$pattern"/>
                <xsl:with-param name="replace" select="$replace"/>
              </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
              <xsl:value-of select="$input"/>
            </xsl:otherwise>
          </xsl:choose>
        </xsl:template>

</xsl:transform>
