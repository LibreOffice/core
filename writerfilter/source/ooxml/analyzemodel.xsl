<?xml version="1.0" encoding="UTF-8"?>
<!--***********************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 ***********************************************************-->

<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:rng="http://relaxng.org/ns/structure/1.0">

<xsl:include href="factorytools.xsl"/>
<xsl:output method="xml"/>

<xsl:template match="/">
	<analyze>
		<xsl:for-each select="/model/namespace[not(@todo='ignore')]">
			<xsl:call-template name="analyzegrammar"/>
		</xsl:for-each>
	</analyze>
</xsl:template>

<xsl:template name="analyzegrammar">
	<xsl:variable name="nsname" select="@name"/>
	<xsl:for-each select="rng:grammar/rng:define">
		<xsl:variable name="defname" select="@name"/>
		<xsl:for-each select=".//rng:attribute|.//rng:element">
			<xsl:choose>
				<xsl:when test="local-name()='element'">					
					<element>
						<xsl:call-template name="defineattrs">
							<xsl:with-param name="nsname" select="$nsname"/>
							<xsl:with-param name="defname" select="$defname"/>
						</xsl:call-template>
					</element>
				</xsl:when>
				<xsl:when test="local-name()='attribute'">
					<attribute>
						<xsl:call-template name="defineattrs">
							<xsl:with-param name="nsname" select="$nsname"/>
							<xsl:with-param name="defname" select="$defname"/>
						</xsl:call-template>
					</attribute>
				</xsl:when>
			</xsl:choose>
		</xsl:for-each>
	</xsl:for-each>
</xsl:template>

<xsl:template name="defineattrs">
	<xsl:param name="nsname"/>
	<xsl:param name="defname"/>

	<xsl:variable name="localname" select="local-name()"/>
	<xsl:variable name="name" select="@name"/>

	<xsl:attribute name="id">
		<xsl:value-of select="$nsname"/>
		<xsl:text>:</xsl:text>
		<xsl:value-of select="$defname"/>
		<xsl:text>:</xsl:text>
		<xsl:value-of select="@name"/>
	</xsl:attribute>	
	<xsl:for-each select="ancestor::namespace/resource[@name=$defname]">
		<xsl:attribute name="resource"><xsl:value-of select="@resource"/></xsl:attribute>
		<xsl:choose>
			<xsl:when test="$localname='attribute'">
				<xsl:for-each select="attribute[@name=$name and @tokenid]">
					<xsl:attribute name="tokenid"><xsl:value-of select="@tokenid"/></xsl:attribute>
					<xsl:attribute name="qname">
						<xsl:call-template name="idtoqname">
							<xsl:with-param name="id" select="@tokenid"/>
						</xsl:call-template>
					</xsl:attribute>
                    <xsl:attribute name="namespace">
                        <xsl:value-of select="$nsname"/>
                    </xsl:attribute>
                    <xsl:attribute name="define">
                        <xsl:value-of select="$defname"/>
                    </xsl:attribute>
                    <xsl:attribute name="name">
                        <xsl:value-of select="@name"/>
                    </xsl:attribute>
				</xsl:for-each>
			</xsl:when>
			<xsl:when test="$localname='element'">
				<xsl:for-each select="element[@name=$name and @tokenid]">
					<xsl:attribute name="tokenid"><xsl:value-of select="@tokenid"/></xsl:attribute>
					<xsl:attribute name="qname">
						<xsl:call-template name="idtoqname">
							<xsl:with-param name="id" select="@tokenid"/>
						</xsl:call-template>
					</xsl:attribute>
                    <xsl:attribute name="namespace">
                        <xsl:value-of select="$nsname"/>
                    </xsl:attribute>
                    <xsl:attribute name="define">
                        <xsl:value-of select="$defname"/>
                    </xsl:attribute>
                    <xsl:attribute name="name">
                        <xsl:value-of select="@name"/>
                    </xsl:attribute>
				</xsl:for-each>
			</xsl:when>
		</xsl:choose>
        <xsl:copy-of select="@tag"/>
	</xsl:for-each>
</xsl:template>
</xsl:stylesheet>