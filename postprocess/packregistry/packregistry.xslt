<?xml version="1.0" encoding="UTF-8"?>
<!--**********************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: code,v $
*
* $Revision: 1.4 $
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
**********************************************************************-->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:oor="http://openoffice.org/2001/registry">
  <xsl:template match="/">
    <xsl:element name="data">
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
        <xsl:choose>
          <xsl:when test="count(document(.)/oor:component-schema) = 1">
            <xsl:copy-of select="document(.)/oor:component-schema"/>
          </xsl:when>
          <xsl:when test="count(document(.)/oor:component-data) = 1">
            <xsl:copy-of select="document(.)/oor:component-data"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:message terminate="yes">
              <xsl:text>cannot process </xsl:text>
              <xsl:value-of select="."/>
            </xsl:message>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:for-each>
    </xsl:element>
  </xsl:template>
</xsl:stylesheet>
