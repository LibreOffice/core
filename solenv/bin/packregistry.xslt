<?xml version="1.0" encoding="UTF-8"?>
<!--***********************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 ***********************************************************-->



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
