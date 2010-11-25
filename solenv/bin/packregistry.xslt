<?xml version="1.0" encoding="UTF-8"?>
<!--**********************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2010 Oracle and/or its affiliates.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* This file is part of OpenOffice.org.
*
* OpenOffice.org is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License version 3
* only, as published by the Free Software Foundation.
*
* OpenOffice.org is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License version 3 for more details
* (a copy is included in the LICENSE file that accompanied this code).
*
* You should have received a copy of the GNU Lesser General Public License
* version 3 along with OpenOffice.org.  If not, see
* <http://www.openoffice.org/license.html>
* for a copy of the LGPLv3 License.
*
**********************************************************************-->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns:oor="http://openoffice.org/2001/registry">
  <xsl:param name="prefix"/>
  <xsl:strip-space elements="*"/>
  <xsl:preserve-space elements="value it"/>
    <!-- TODO: strip space from "value" elements that have "it" children -->
  <xsl:template match="/">
    <oor:data xmlns:xs="http://www.w3.org/2001/XMLSchema"
       xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
      <xsl:copy-of select="list/dependency"/>
<!--
      <xsl:copy-of select="document(list/filename)/oor:component-schema"/>
      <xsl:copy-of select="document(list/filename)/oor:component-data"/>

      instead of the below for-each would only issue warnings, not errors, for
      non-existing or otherwise bad input files; it is important that the input
      filename list is already sorted in an order suitable for the configmgr
      (e.g., xcs files preceeding xcu files).
-->
      <xsl:for-each select="list/filename">
        <xsl:variable name="doc" select="document(concat($prefix, .))"/>
        <xsl:choose>
          <xsl:when test="count($doc/oor:component-schema) = 1">
            <xsl:apply-templates select="$doc/oor:component-schema"/>
          </xsl:when>
          <xsl:when test="count($doc/oor:component-data) = 1">
            <xsl:apply-templates select="$doc/oor:component-data"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:message terminate="yes">
              <xsl:text>cannot process </xsl:text>
              <xsl:value-of select="."/>
            </xsl:message>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:for-each>
    </oor:data>
  </xsl:template>
  <xsl:template
     match="oor:component-schema|oor:component-data|templates|component|group|
            set|node-ref|prop|item|value|it|unicode|node">
    <xsl:copy copy-namespaces="no">
      <!-- prune oor:component-data xmlns:install="..." namespaces (would only
           work in XSLT 2.0, however) -->
      <xsl:apply-templates select="@*"/>
      <xsl:apply-templates/>
    </xsl:copy>
  </xsl:template>
  <xsl:template match="value[it]">
    <xsl:copy copy-namespaces="no">
      <xsl:apply-templates select="@*"/>
      <xsl:apply-templates select="*"/>
        <!-- ignore text elements (which must be whitespace only) -->
    </xsl:copy>
  </xsl:template>
  <xsl:template match="info|import|uses|constraints"/>
    <!-- TODO: no longer strip elements when they are eventually read by
         configmgr implementation -->
  <xsl:template match="@*">
    <xsl:copy/>
  </xsl:template>
</xsl:stylesheet>
