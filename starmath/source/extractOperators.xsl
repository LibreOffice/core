<!-- -*- Mode: nXML; tab-width: 2; indent-tabs-mode: nil; -*- -->
<!--
    This file is part of the LibreOffice project.

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  
  <xsl:strip-space elements="*"/>

  <xsl:template match="charlist">
    <root>
      <xsl:text>&#xa;</xsl:text>
      <xsl:apply-templates select="character"/>
    </root>
  </xsl:template>

  <xsl:template match="character">
    <xsl:if test="operator-dictionary">
      <xsl:for-each select="operator-dictionary">
        <xsl:text>opDict[</xsl:text>
        <xsl:value-of select="../@id"/>
        <xsl:text>] = </xsl:text>
        <xsl:value-of select="boolean(@stretchy[.='true'])"/>
        <xsl:text>;&#xa;</xsl:text>
      </xsl:for-each>
    </xsl:if>
  </xsl:template>

</xsl:stylesheet>
