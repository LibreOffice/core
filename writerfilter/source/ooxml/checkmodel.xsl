<!--
/*************************************************************************
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

 ************************************************************************/

-->
<xsl:stylesheet 
    version="1.0" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
    xmlns:rng="http://relaxng.org/ns/structure/1.0"
    xml:space="default" xml:indent="true">
  <xsl:output method="text" />
  

  <xsl:template name="noannotation">
    <xsl:for-each select="//namespace">
      <xsl:variable name="nsname" select="@name"/>
      <xsl:value-of select="./rng:grammar/@ns"/>
      <xsl:text>&#xa;</xsl:text>
      <xsl:for-each select=".//rng:define">
        <xsl:variable name="name" select="@name"/>
        <xsl:if test="not(ancestor::namespace//resource[@name=$name])">
          <xsl:text>noannotation: </xsl:text>
          <xsl:value-of select="$nsname"/>
          <xsl:text>::</xsl:text>
          <xsl:value-of select="$name"/>
          <xsl:text>&#xa;</xsl:text>
        </xsl:if>
      </xsl:for-each>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="checkmodel">
    <xsl:for-each select="//namespace">
      <xsl:variable name="nsname" select="@name"/>
      <xsl:for-each select=".//rng:define">
        <xsl:call-template name="orphanedprops"/>
        <xsl:variable name="definename" select="@name"/>
        <xsl:if test="not(ancestor::namespace//resource[@name=$definename])">
          <xsl:text>no annotation: </xsl:text>
          <xsl:value-of select="$nsname"/>
          <xsl:text>::</xsl:text>
          <xsl:value-of select="$definename"/>
          <xsl:text>&#xa;</xsl:text>
        </xsl:if>
        <xsl:for-each select=".//rng:element">
          <xsl:variable name="elementname" select="@name"/>
          <xsl:for-each select="ancestor::namespace//resource[@name=$definename and @resource='Properties']">
            <xsl:if test="not(.//element[@name=$elementname])">
              <xsl:text>missing element: </xsl:text>
              <xsl:value-of select="$nsname"/>
              <xsl:text>::</xsl:text>
              <xsl:value-of select="$definename"/>
              <xsl:text>::</xsl:text>
              <xsl:value-of select="$elementname"/>
              <xsl:text>&#xa;</xsl:text>
            </xsl:if>
            <xsl:call-template name="orphanedprops"/>
          </xsl:for-each>
        </xsl:for-each>
        <xsl:for-each select=".//rng:attribute">
          <xsl:variable name="attributename" select="@name"/>
          <xsl:for-each select="ancestor::namespace//resource[@name=$definename and @resource='Properties']">
            <xsl:if test="not(.//attribute[@name=$attributename])">
              <xsl:text>missing attribute: </xsl:text>
              <xsl:value-of select="$nsname"/>
              <xsl:text>::</xsl:text>
              <xsl:value-of select="$definename"/>
              <xsl:text>::</xsl:text>
              <xsl:value-of select="$attributename"/>
              <xsl:text>&#xa;</xsl:text>
            </xsl:if>
            <xsl:call-template name="orphanedprops"/>
          </xsl:for-each>
        </xsl:for-each>
      </xsl:for-each>
    </xsl:for-each>
  </xsl:template>

  <xsl:key name="defines-with-name" match="rng:define" use="@name"/>

  <xsl:template name="defineforref">
    <xsl:variable name="grammarid" select="generate-id(ancestor::grammar)"/>
    <xsl:variable name="defineingrammar" select="key('defines-with-name', @name)[generate-id(ancestor::grammar) = $grammarid]"/>
    <xsl:choose>
      <xsl:when test="$defineingrammar">
        <xsl:value-of select="$defineingrammar"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="key('defines-with-name', @name)"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="contextresource">
    <xsl:variable name="name" select="@name"/>
    <xsl:value-of select="ancestor::namespace//resource[@name=$name]/@resource"/>
  </xsl:template>

  <xsl:template name="orphanedprops">
    <xsl:variable name="nsname" select="ancestor::namespace/@name"/>
    <xsl:for-each select=".//rng:ref[not (ancestor::rng:element or ancestor::rng:attribute)]">
      <xsl:variable name="defineresource">
        <xsl:for-each select="ancestor::rng:define">
          <xsl:call-template name="contextresource"/>
        </xsl:for-each>
      </xsl:variable>
      <xsl:variable name="definename" select="ancestor::rng:define/@name"/>
      <xsl:variable name="mydefine">
        <xsl:copy>
          <xsl:call-template name="defineforref"/>
        </xsl:copy>
      </xsl:variable>
      <xsl:variable name="myresource">
        <xsl:call-template name="contextresource"/>
      </xsl:variable>
      <xsl:if test="$myresource = 'Properties'">
        <xsl:choose>
          <xsl:when test="$defineresource = 'Properties'"/>
          <xsl:when test="$defineresource = 'Stream'"/>
          <xsl:otherwise>
            <xsl:text>orphaned properties: </xsl:text>
            <xsl:value-of select="$nsname"/>
            <xsl:text>:</xsl:text>
            <xsl:value-of select="$definename"/>
            <xsl:text>(</xsl:text>
            <xsl:value-of select="$defineresource"/>
            <xsl:text>)-&gt;</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>(</xsl:text>
            <xsl:value-of select="$myresource"/>
            <xsl:text>)&#xa;</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>

  <xsl:template match="/">
    <out>
      <xsl:call-template name="checkmodel"/>
    </out>
  </xsl:template>
</xsl:stylesheet>