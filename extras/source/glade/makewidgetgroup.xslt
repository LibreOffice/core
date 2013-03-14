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
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="xml" version="1.0" encoding="UTF-8"/>

  <!-- work arround apparently broken output indenting in libxslt -->
  <xsl:param name="indent" select="'  '"/>

  <xsl:template match="/glade-catalog">
    <xsl:copy>
      <xsl:apply-templates select="@*|node()"/>

      <xsl:text>&#10;</xsl:text>
      <xsl:value-of select="$indent"/>
      <glade-widget-group name="libreoffice" title="LibreOffice">
      <xsl:text>&#10;</xsl:text>

        <xsl:for-each select="//glade-widget-class">
          <xsl:value-of select="concat($indent,$indent)"/>
          <glade-widget-class-ref name="{@name}"/>
          <xsl:text>&#10;</xsl:text>
        </xsl:for-each>

      <xsl:value-of select="$indent"/>
      </glade-widget-group>
      <xsl:text>&#10;</xsl:text>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="@*|node()">
    <xsl:copy><xsl:apply-templates select="@*|node()"/></xsl:copy>
  </xsl:template>

</xsl:stylesheet>
