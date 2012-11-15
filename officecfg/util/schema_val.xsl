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

<xsl:transform  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
		xmlns:xs="http://www.w3.org/2001/XMLSchema"		
		xmlns:oor="http://openoffice.org/2001/registry"		
		version="1.0">

<!-- Get the correct format -->
<xsl:output method="xml" indent="yes"/>
<xsl:namespace-alias stylesheet-prefix="xs" result-prefix="xs"></xsl:namespace-alias>

<!-- Parameter -->
<xsl:param name="root">../registry/schema</xsl:param>
<xsl:param name="schemaRoot"/>
<xsl:param name="componentName"/>

	<xsl:template match = "/">
		<xsl:apply-templates/>
	</xsl:template>
	
<!-- activate attribute templates -->
	<xsl:template match="group|set|prop">
		<xsl:apply-templates select="*|@*"/>
	</xsl:template>

<!-- make sure that missing features are not invoked -->
	<xsl:template match = "item">
		<xsl:message terminate="yes">ERROR: multiple template types for sets are NOT supported!</xsl:message>
	</xsl:template>

	<xsl:template match = "set[@oor:extensible='true']">
		<xsl:message terminate="yes">ERROR: extensible sets are currently NOT supported!</xsl:message>
	</xsl:template>

	<xsl:template match = "group[@oor:extensible='true']">
		<xsl:if test="count(child::set) or count(child::group) or count(child::node-ref)">
			<xsl:message terminate="yes">ERROR: extensible groups with children are currently NOT supported!</xsl:message>
		</xsl:if>
		<xsl:apply-templates select="*|@*"/>
	</xsl:template>

<!-- Localized info elements (desc/label) are not supported currently -->
	<xsl:template match="info//*[@xml:lang]">
		<xsl:message terminate="yes">ERROR: Info elements (desc/label) are currently not localized. Remove xml:lang attributes!</xsl:message>
	</xsl:template>

<!-- check for duplicate child names -->
	<xsl:template match="@oor:name">
        <xsl:variable name="item-name" select="."/>
		<xsl:if test="../following-sibling::*[@oor:name = $item-name]">
			<xsl:message terminate="yes">ERROR: Duplicate node name '<xsl:value-of select="$item-name"/>'!</xsl:message>
		</xsl:if>			
	</xsl:template>

<!-- check if properties of type 'any' do not have a value -->
	<xsl:template match="prop[@oor:type='oor:any']">
		<xsl:if test="count(value)"> 
			<xsl:message terminate="yes">ERROR: Properties of type 'oor:any' MUST NOT have a value!</xsl:message>
		</xsl:if>
		<xsl:apply-templates select="*|@*"/>
	</xsl:template>

<!-- inhibit (explicit) NIL values -->
	<xsl:template match="value[@xsi:nil]">
		<xsl:message terminate="yes">ERROR: xsi:nil is not allowed in schemas !</xsl:message>
	</xsl:template>

<!-- validate for correct node references -->
	<xsl:template match="@oor:node-type">
		<xsl:choose>
			<xsl:when test="../@oor:component">
				<xsl:variable name ="file">
					<xsl:call-template name="locateFile"><xsl:with-param name="componentName" select="../@oor:component"/></xsl:call-template>
				</xsl:variable>
				<xsl:if test="not(document($file)/oor:component-schema/templates/*[@oor:name=current()])">
					<xsl:message terminate="yes">ERROR: node-type '<xsl:value-of select="current()"/>' not found!</xsl:message>
				</xsl:if>				
			</xsl:when>
			<xsl:when test="not(/oor:component-schema/templates/*[@oor:name=current()])">
				<xsl:message terminate="yes">ERROR: node-type '<xsl:value-of select="current()"/>' not found!</xsl:message>
			</xsl:when>
		</xsl:choose>		
	</xsl:template>

<!-- validate if file name matches component-name -->
	<xsl:template match="oor:component-schema">
		<xsl:variable name ="fullName"><xsl:value-of select="@oor:package"/>.<xsl:value-of select="@oor:name"/></xsl:variable>
		<xsl:if test="$fullName != $componentName">
			<xsl:message terminate="yes">ERROR: Component name '<xsl:value-of select="$fullName"/>' does not match with file name!</xsl:message>
		</xsl:if>			
		<xsl:apply-templates select="*|@*"/>
	</xsl:template>


<!-- locate a component file -->
	<xsl:template name="locateFile">
		<xsl:param name="componentName"/>
		<xsl:variable name="path"><xsl:value-of select="translate($componentName,'.','/')"/>.xcs</xsl:variable>
		<xsl:variable name ="file">
			<xsl:variable name ="tryfile" select="concat($root,'/',$path)"/>
			<xsl:choose>
				<xsl:when test="document($tryfile)">
					<xsl:value-of select="$tryfile"/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:if test="$schemaRoot">
						<xsl:variable name="globalfile" select="concat($schemaRoot,'/',$path)"/>
						<xsl:if test="document($globalfile)">
							<xsl:value-of select="$globalfile"/>
						</xsl:if>
					</xsl:if>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<xsl:if test="not($file)">
			<xsl:message terminate ="yes">**Error: unable to locate document '<xsl:value-of select="$path"/>'</xsl:message>
		</xsl:if>
		<xsl:value-of select="$file"/>
	</xsl:template>

</xsl:transform>
