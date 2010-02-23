<?xml version="1.0" encoding="utf-8" standalone="yes" ?>
<!--***********************************************************************
 *
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

 ************************************************************************ -->

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
