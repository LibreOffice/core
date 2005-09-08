<?xml version="1.0" encoding="utf-8" standalone="yes" ?>
<!--***********************************************************************
 *
    OpenOffice.org - a multi-platform office productivity suite
 
    $RCSfile: resource.xsl,v $
 
    $Revision: 1.3 $
 
    last change: $Author: rt $ $Date: 2005-09-08 15:50:01 $
 
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
