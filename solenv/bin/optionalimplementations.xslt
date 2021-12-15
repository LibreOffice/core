<?xml version="1.0" encoding="UTF-8"?>
<!--
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * this "misuses" xsl:message to dump all optional components
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://openoffice.org/2010/uno-components"
    xmlns:uc="http://openoffice.org/2010/uno-components">
<xsl:template match="uc:component">
  <xsl:for-each select="uc:implementation">
    <xsl:if test="uc:optional">
      <xsl:message><xsl:value-of select="@name"/></xsl:message>
    </xsl:if>
  </xsl:for-each>
</xsl:template>
</xsl:stylesheet>
