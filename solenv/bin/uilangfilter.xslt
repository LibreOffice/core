<?xml version="1.0" encoding="UTF-8"?>
<!--
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="xml" encoding="UTF-8"/>

<xsl:template match="/">
  <l><xsl:text>&#10;</xsl:text>
  <xsl:apply-templates select="//*[*[not(self::col)]/@translatable='yes']" />
  <xsl:apply-templates select="interface/object[data/row/col[@translatable='yes']]" />
  </l>
</xsl:template>

<xsl:template match="*/*[not(self::col)][@translatable]">
  <xsl:text> </xsl:text>
  <t r="string" g="{substring-before(../@id,':')}" l="{@name}">
  <xsl:copy-of select="text()" />
  </t><xsl:text>&#10;</xsl:text>
</xsl:template>

<xsl:template match="col">
  <xsl:text> </xsl:text>
  <xsl:variable name="groupid" select="../../../@id"/>
  <t r="stringlist" g="{substring-before($groupid. ':')}" l="{count(preceding::col[../../../@id=$groupid])}">
  <xsl:copy-of select="text()" />
  </t><xsl:text>&#10;</xsl:text>
</xsl:template>

<xsl:template match="text()"/>

</xsl:stylesheet> 
