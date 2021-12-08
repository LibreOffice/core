<?xml version="1.0" encoding="UTF-8"?>
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

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    exclude-result-prefixes="uc"
    xmlns="http://openoffice.org/2010/uno-components"
    xmlns:uc="http://openoffice.org/2010/uno-components">
  <xsl:param name="uri"/>
  <xsl:param name="cppu_env"/>
  <xsl:param name="features"/>
  <xsl:strip-space elements="*"/>
  <xsl:template match="uc:component">
    <components>
      <xsl:copy>
        <xsl:apply-templates select="@*"/>
        <xsl:attribute name="uri">
          <xsl:value-of select="$uri"/>
        </xsl:attribute>
        <xsl:apply-templates/>
      </xsl:copy>
    </components>
  </xsl:template>
  <xsl:template match="uc:implementation[@CONDITION]">
    <xsl:variable name="feature" select="@CONDITION"/>
    <xsl:variable name="feature2" select="concat('(',$feature,')')"/>
    <xsl:if test="contains($features,$feature2)">
      <xsl:copy>
        <xsl:apply-templates select="@*"/>
        <xsl:apply-templates/>
      </xsl:copy>
    </xsl:if>
  </xsl:template>
  <xsl:template match="*">
    <xsl:copy>
      <xsl:apply-templates select="@*"/>
      <xsl:apply-templates/>
    </xsl:copy>
  </xsl:template>
  <xsl:template match="@environment">
    <xsl:attribute name="environment">
      <xsl:call-template name="replace">
        <xsl:with-param name="input" select="current()"/>
        <xsl:with-param name="pattern" select="'@CPPU_ENV@'"/>
        <xsl:with-param name="replace" select="$cppu_env"/>
      </xsl:call-template>
    </xsl:attribute>
  </xsl:template>
  <xsl:template match="@CONDITION"/>
  <xsl:template match="@*">
    <xsl:copy/>
  </xsl:template>
  <xsl:template name="replace">
    <xsl:param name="input"/>
    <xsl:param name="pattern"/>
    <xsl:param name="replace"/>
    <xsl:choose>
      <xsl:when test="contains($input, $pattern)">
        <xsl:value-of select="substring-before($input, $pattern)"/>
        <xsl:value-of select="$replace"/>
        <xsl:call-template name="replace">
          <xsl:with-param
              name="input" select="substring-after($input, $pattern)"/>
          <xsl:with-param name="pattern" select="$pattern"/>
          <xsl:with-param name="replace" select="$replace"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$input"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
</xsl:stylesheet>
