<?xml version='1.0' encoding="UTF-8"?>
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
                xmlns:redirect="http://xml.apache.org/xalan/redirect"
                extension-element-prefixes="redirect">
<xsl:output method="text" indent="no" omit-xml-declaration="no" version="1.0" encoding="UTF-8" />

    <!-- OPTIONAL: output directory for the IDL
                   Have to end with path separator -->
    <xsl:param name="IDL_DIRECTORY" />

    <xsl:key name="same-context-elements"
             match="/api/element[@type='constant']"
             use="normalize-space(source/context)" />

    <xsl:template match="/">
        <!-- once for each enumeration -->
        <xsl:for-each select="/api/element[@type='enumeration']">
            <xsl:sort select="normalize-space(source/name)" />

            <redirect:write select="concat($IDL_DIRECTORY, normalize-space(source/name),'.idl')">
                <xsl:call-template name="write-idl-prefix-1" />
                <xsl:call-template name="write-idl-prefix-2" />
                <!-- write IDL content -->
                <xsl:for-each select="key('same-context-elements', normalize-space(source/name))">
                    <xsl:call-template name="write-idl-content" />
                </xsl:for-each>
                <xsl:call-template name="write-idl-suffix" />
            </redirect:write>
        </xsl:for-each>
    </xsl:template>


    <!-- writing the IDL prefix -->
    <xsl:template name="write-idl-prefix-1">
	<xsl:variable name="submod" select="source/@id"/>
	<xsl:variable name="period" select="'.'"/>
	<xsl:variable name="lcletters">abcdefghijklmnopqrstuvwxyz</xsl:variable>
	<xsl:variable name="ucletters">ABCDEFGHIJKLMNOPQRSTUVWXYZ</xsl:variable>
<xsl:text>
        module org { module openoffice { module </xsl:text><xsl:value-of select="translate(substring-before($submod,$period),$ucletters,$lcletters)"/>
    </xsl:template>
    <xsl:template name="write-idl-prefix-2">
<xsl:text> {
            constants </xsl:text><xsl:value-of select="normalize-space(source/name)" /><xsl:text> {</xsl:text>
    </xsl:template>

    <!-- writing the IDL content -->
    <xsl:template name="write-idl-content">
<xsl:text>
                const long </xsl:text><xsl:value-of select="source/name" /><xsl:text> = </xsl:text><xsl:value-of select="source/value" /><xsl:text>;</xsl:text>
    </xsl:template>

    <!-- writing the IDL suffix -->
    <xsl:template name="write-idl-suffix">
<xsl:text>
            };
        }; }; };
</xsl:text>
    </xsl:template>
</xsl:stylesheet>

