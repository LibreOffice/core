<?xml version="1.0" encoding="utf-8" standalone="yes" ?>
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



<xsl:transform  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
		xmlns:xs="http://www.w3.org/2001/XMLSchema"		
		xmlns:oor="http://openoffice.org/2001/registry"		
		version="1.0">

    <!-- Set correct output format -->
    <xsl:output method="text" indent="no" encoding="UTF-8"/>

    <xsl:param name="locale"/>
    <xsl:param name="fallback-locale">en-US</xsl:param>

    <xsl:namespace-alias stylesheet-prefix="xs" result-prefix="xs"></xsl:namespace-alias>

    <!-- overwrite default rule for text & attribute nodes to be silent -->
    <xsl:template match="text()|@*">
    </xsl:template>

    <!-- match root -->
    <xsl:template match="/">
        <xsl:apply-templates/>
    </xsl:template>

    <!-- match all elements that could possibly contain info elements -->
    <xsl:template match="info/desc[@xml:lang=$locale]|info/label[@xml:lang=$locale]">

        <!-- print TPF's path to current node -->
        <xsl:for-each select="ancestor-or-self::*">
            <xsl:if test="local-name(.)!='component-schema'"><xsl:value-of select="local-name(.)"/>
                <xsl:choose>
                    <xsl:when test="@oor:name">[<xsl:value-of select="@oor:name"/>]</xsl:when>
                    <xsl:when test="@oor:value">[<xsl:value-of select="@oor:value"/>]</xsl:when>
                </xsl:choose>
                <xsl:if test="position()!=last()">.</xsl:if>
            </xsl:if>
        </xsl:for-each>

        <!-- print separator (equals sign) -->
        <xsl:text>=</xsl:text>

        <!-- print info/desc|label element's content -->
        <xsl:value-of select="normalize-space(.)"/>

        <!-- print linefeed -->
        <xsl:text>&#10;</xsl:text>

        <xsl:apply-templates/>

    </xsl:template>

</xsl:transform>
