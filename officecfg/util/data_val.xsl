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

<!-- ************************************************************************************** -->
<!-- * Transformation from New Format XCS & XCU to schema description                   *** -->
<!-- ************************************************************************************** -->
<xsl:transform version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:xsi="http://www.w3.org/1999/XMLSchema-instance"
                xmlns:xs="http://www.w3.org/2001/XMLSchema"
                xmlns:oor="http://openoffice.org/2001/registry"
                xmlns:install="http://openoffice.org/2004/installation">

<!-- Get the correct format -->
<xsl:output method="xml" indent="yes" />

<xsl:param name="xcs"/>
<xsl:param name="schemaRoot">.</xsl:param>

<xsl:variable name="schemaRootURL">
    <xsl:value-of select="$schemaRoot"/>
</xsl:variable>
<xsl:variable name="schemaURL">
    <xsl:value-of select="$xcs"/>
</xsl:variable>

<!-- ************************************** -->
<!-- * oor:component-data                           *** -->
<!-- ************************************** -->
    <xsl:template match="/oor:component-data">
        <xsl:variable name="component-schema" select="document($schemaURL)/oor:component-schema"/>
        <xsl:for-each select="node|prop">
            <xsl:apply-templates select=".">
                <xsl:with-param name="context" select="$component-schema/component/*[@oor:name = current()/@oor:name]"/>
                <xsl:with-param name="component-schema" select="$component-schema"/>
            </xsl:apply-templates>
        </xsl:for-each>
    </xsl:template>

<!-- ****************************************** -->
<!-- * handle template references           *** -->
<!-- ****************************************** -->
    <xsl:template name="resolve-template">
        <xsl:param name = "node-type"/>
        <xsl:param name = "schema-type"/>
        <xsl:param name = "component-schema"/>
        <xsl:variable name = "path">
            <xsl:call-template name="collectPath"/>
        </xsl:variable>

        <xsl:if test="not ($component-schema)">
            <xsl:message terminate="yes">ERROR: Template '<xsl:value-of select="$node-type"/>',
                                         referenced from node '<xsl:value-of select="$path"/>'
                                         does not exist in schema!
            </xsl:message>
        </xsl:if>

        <xsl:choose>
            <xsl:when test="$schema-type='node-ref'">
                <xsl:apply-templates select=".">
                    <xsl:with-param name="context" select="$component-schema/templates/*[@oor:name = $node-type]"/>
                    <xsl:with-param name="component-schema" select="$component-schema"/>
                </xsl:apply-templates>
            </xsl:when>
            <xsl:when test="$schema-type='set'">
                <xsl:for-each select="node|prop">
                    <xsl:apply-templates select=".">
                        <xsl:with-param name="context" select="$component-schema/templates/*[@oor:name = $node-type]"/>
                        <xsl:with-param name="component-schema" select="$component-schema"/>
                    </xsl:apply-templates>
                </xsl:for-each>
            </xsl:when>
            <xsl:otherwise>
                <xsl:message terminate="yes">ERROR: The schema element for node <xsl:value-of select="$path"/>
                                              is a <xsl:value-of select="$schema-type"/> and should not have a node-type.
                </xsl:message>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

<!-- ****************************************** -->
<!-- * node                                 *** -->
<!-- ****************************************** -->
    <xsl:template match="node">
        <xsl:param name = "context"/>
        <xsl:param name = "component-schema"/>
        <xsl:variable name = "path">
            <xsl:call-template name="collectPath"/>
        </xsl:variable>

        <xsl:if test="not ($context)">
            <xsl:message terminate="yes">ERROR: Node '<xsl:value-of select="$path"/>' does not exist in schema!</xsl:message>
        </xsl:if>

        <xsl:call-template name="checkModule"/>
        <xsl:call-template name="checkDuplicates"/>

        <xsl:choose>
            <!-- look for matching templates in other components -->
            <xsl:when test="$context/@oor:node-type and $context/@oor:component">
                <xsl:variable name="fileURL">
                    <xsl:call-template name="composeFileURL">
                        <xsl:with-param name="componentName"><xsl:value-of select="$context/@oor:component"/></xsl:with-param>
                    </xsl:call-template>
                </xsl:variable>

                <xsl:call-template name="resolve-template">
                    <xsl:with-param name="node-type" select="$context/@oor:node-type"/>
                    <xsl:with-param name="schema-type" select="local-name($context)"/>
                    <xsl:with-param name="component-schema" select="document($fileURL)/oor:component-schema"/>
                </xsl:call-template>
            </xsl:when>
            <!-- look for matching templates within the same component -->
            <xsl:when test="$context/@oor:node-type">
                <xsl:call-template name="resolve-template">
                    <xsl:with-param name="node-type" select="$context/@oor:node-type"/>
                    <xsl:with-param name="schema-type" select="local-name($context)"/>
                    <xsl:with-param name="component-schema" select="$component-schema"/>
                </xsl:call-template>
            </xsl:when>
            <!-- is the node extensible ? -->
            <xsl:when test="$context/@oor:extensible='true'">
                <xsl:for-each select="node|prop">
                    <xsl:apply-templates select="." mode="extensible">
                        <xsl:with-param name="context" select="$context/*[@oor:name = current()/@oor:name]"/>
                        <xsl:with-param name="component-schema" select="$component-schema"/>
                    </xsl:apply-templates>
                </xsl:for-each>
            </xsl:when>
            <xsl:otherwise>
                <xsl:for-each select="node|prop">
                    <xsl:apply-templates select=".">
                        <xsl:with-param name="context" select="$context/*[@oor:name = current()/@oor:name]"/>
                        <xsl:with-param name="component-schema" select="$component-schema"/>
                    </xsl:apply-templates>
                </xsl:for-each>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

<!-- ****************************************** -->
<!-- * prop                                 *** -->
<!-- ****************************************** -->
    <xsl:template match="prop">
        <xsl:param name = "context"/>
        <xsl:variable name = "path">
            <xsl:call-template name="collectPath"/>
        </xsl:variable>


        <xsl:if test="not ($context)">
            <xsl:message terminate="yes">ERROR: Property '<xsl:value-of select="$path"/>' does not exist in schema !</xsl:message>
        </xsl:if>

        <xsl:if test="@oor:op">
            <xsl:message terminate="yes">ERROR: Property '<xsl:value-of select="$path"/>' has unexpected operation '<xsl:value-of select="@oor:op"/>' !</xsl:message>
        </xsl:if>

        <xsl:if test="not($context/@oor:localized='true')">
            <xsl:for-each select="value">
                <xsl:if test="@xml:lang">
                    <xsl:message terminate="yes">ERROR: Non-localized property '<xsl:value-of select="$path"/>' has value for xml:lang='<xsl:value-of select="@xml:lang"/>'!</xsl:message>
                </xsl:if>
            </xsl:for-each>
        </xsl:if>

        <xsl:call-template name="checkModule"/>
        <xsl:call-template name="checkDuplicates"/>

        <xsl:apply-templates />
    </xsl:template>

<!-- ****************************************** -->
<!-- * value                                    *** -->
<!-- ****************************************** -->
    <xsl:template match="value">
        <xsl:call-template name="checkModule"/>
        <xsl:variable name="path">
          <xsl:call-template name="collectPath"/>
        </xsl:variable>
        <xsl:variable name="lang" select="@xml:lang"/>
        <xsl:variable name="module" select="@install:module"/>
        <xsl:if test="$module and $lang='x-no-translate'">
          <xsl:message terminate="yes">ERROR: Property '<xsl:value-of select="$path"/>' has value for special xml:lang="x-no-translate" in module <xsl:value-of select="$module"/>.</xsl:message>
        </xsl:if>
        <xsl:choose>
          <xsl:when
              test="preceding-sibling::value[($lang and not(@xml:lang)) or (not($lang) and @xml:lang)]">
            <xsl:message terminate="yes">ERROR: Property '<xsl:value-of select="$path"/>' has values with and without xml:lang attributes.</xsl:message>
          </xsl:when>
          <xsl:when
              test="preceding-sibling::value[((not($lang) and not(@xml:lang)) or $lang=@xml:lang) and ((not($module) and not(@install:module)) or $module=@install:module)]">
            <xsl:message terminate="yes">ERROR: Property '<xsl:value-of select="$path"/>' has values with matching xml:lang attribute <xsl:value-of select="$lang"/> and inconsistent install:module attributes.</xsl:message>
          </xsl:when>
        </xsl:choose>
    </xsl:template>

<!-- ****************************************** -->
<!-- * node (mode:extensible) - not supported * -->
<!-- ****************************************** -->
    <xsl:template match="node" mode="extensible">
        <xsl:variable name = "path">
            <xsl:call-template name="collectPath"/>
        </xsl:variable>

        <xsl:message terminate="yes">ERROR: Node '<xsl:value-of select="$path"/>' is within an extensible node!</xsl:message>
    </xsl:template>
<!-- ****************************************** -->
<!-- * prop (mode:extensible)               *** -->
<!-- ****************************************** -->
    <xsl:template match="prop" mode="extensible">
        <xsl:param name = "context"/>
        <xsl:variable name = "path">
            <xsl:call-template name="collectPath"/>
        </xsl:variable>

        <xsl:choose>
            <xsl:when test="not(@oor:op)">
                <xsl:if test="not ($context)">
                    <xsl:message terminate="yes">ERROR: Property '<xsl:value-of select="$path"/>' does not exist in schema!</xsl:message>
                </xsl:if>
            </xsl:when>
            <xsl:when test="@oor:op='replace'">
                <xsl:if test="not (@oor:type)">
                    <xsl:message terminate="yes">ERROR: Property '<xsl:value-of select="$path"/>' does not have a type!</xsl:message>
                </xsl:if>
            </xsl:when>
            <xsl:otherwise>
                <xsl:message terminate="yes">ERROR: Property '<xsl:value-of select="$path"/>' has unexpected operation '<xsl:value-of select="@oor:op"/>'!</xsl:message>
            </xsl:otherwise>
        </xsl:choose>

        <!-- Extension properties are always non-localized: -->
        <xsl:for-each select="value">
            <xsl:if test="@xml:lang">
                <xsl:message terminate="yes">ERROR: Non-localized extension property '<xsl:value-of select="$path"/>' has value for xml:lang='<xsl:value-of select="@xml:lang"/>'!</xsl:message>
            </xsl:if>
        </xsl:for-each>

        <xsl:call-template name="checkModule"/>
        <xsl:call-template name="checkDuplicates"/>
    </xsl:template>


<!-- ************************************* -->
<!-- * checkDuplicates                 *** -->
<!-- ************************************* -->
    <xsl:template name="checkDuplicates">
        <xsl:variable name="item-name" select="@oor:name"/>
        <xsl:if test="following-sibling::*[@oor:name = $item-name]">
            <xsl:variable name = "path">
                <xsl:call-template name="collectPath"/>
            </xsl:variable>
            <xsl:message terminate="yes">ERROR: Duplicate node/prop '<xsl:value-of select="$path"/>'!</xsl:message>
        </xsl:if>
    </xsl:template>


<!-- ************************************* -->
<!-- * checkModule                     *** -->
<!-- ************************************* -->
    <xsl:template name="checkModule">
        <xsl:if test="@install:module">
            <xsl:if test="ancestor::*[@install:module]">
                <xsl:message terminate="yes">ERROR: Nested modules are not supported.  Found module '<xsl:value-of select="@install:module"/>' within module '<xsl:value-of select="ancestor::*/@install:module"/>'!
                </xsl:message>
            </xsl:if>
        </xsl:if>
    </xsl:template>

<!-- ************************************* -->
<!-- * collectPath                     *** -->
<!-- ************************************* -->
    <xsl:template name="collectPath">
        <xsl:for-each select="ancestor-or-self::node()[@oor:name]">
            <xsl:text>/</xsl:text><xsl:value-of select="@oor:name"/>
        </xsl:for-each>
    </xsl:template>


<!-- ****************************** -->
<!-- * composeFileURL           *** -->
<!-- ****************************** -->
    <xsl:template name="composeFileURL">
        <xsl:param name="componentName"/>
        <xsl:variable name="fileURL">
            <xsl:value-of select="$schemaRootURL"/>/<xsl:value-of select="translate($componentName,'.','/')"/><xsl:text>.xcs</xsl:text>
        </xsl:variable>
        <xsl:value-of select="$fileURL"/>
    </xsl:template>


</xsl:transform>

