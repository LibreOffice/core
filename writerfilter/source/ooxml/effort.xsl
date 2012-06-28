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

<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:rng="http://relaxng.org/ns/structure/1.0">

<xsl:output method="text"/>
    
<xsl:template match="/">
  <xsl:text>Namespace,Define,Name,Tag,Done,Planned,qname-count&#xa;</xsl:text>
  <xsl:for-each select="/todo/attribute|/todo/element">
    <xsl:variable name="namespace" select="@namespace"/>
    <xsl:variable name="define" select="@define"/>
    <xsl:variable name="name" select="@name"/>
    <xsl:variable name="tag" select="@tag"/>

    <xsl:for-each select=".//status">
      <xsl:value-of select="$namespace"/>
      <xsl:text>,</xsl:text>
      <xsl:value-of select="$define"/>
      <xsl:text>,</xsl:text>
      <xsl:value-of select="$name"/>
      <xsl:text>,</xsl:text>
      <xsl:value-of select="$tag"/>
      <xsl:text>,</xsl:text>
      <xsl:value-of select="@done"/>
      <xsl:text>,</xsl:text>
      <xsl:value-of select="@planned"/>
      <xsl:text>,</xsl:text>
      <xsl:value-of select="@qname-count"/>
      <xsl:text>&#xa;</xsl:text>
    </xsl:for-each>
    <xsl:if test="not(.//status)">
      <xsl:value-of select="$namespace"/>
      <xsl:text>,</xsl:text>
      <xsl:value-of select="$define"/>
      <xsl:text>,</xsl:text>
      <xsl:value-of select="$name"/>
      <xsl:text>,</xsl:text>
      <xsl:value-of select="$tag"/>
      <xsl:text>,0,0.5,1&#xa;</xsl:text>
    </xsl:if>
  </xsl:for-each>
</xsl:template>

</xsl:stylesheet>