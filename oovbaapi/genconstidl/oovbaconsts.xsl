<?xml version='1.0' encoding="UTF-8"?>
<!-- =====================================================================

  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
  
  Copyright 2000, 2010 Oracle and/or its affiliates.
 
  OpenOffice.org - a multi-platform office productivity suite
 
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

====================================================================== -->
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

