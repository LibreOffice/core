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
    xmlns:uc="http://openoffice.org/2010/uno-components">
  <xsl:param name="prefix"/>
  <xsl:strip-space elements="*"/>
  <xsl:template match="/">
    <xsl:element name="components"
        namespace="http://openoffice.org/2010/uno-components">
      <xsl:for-each select="list/filename">
        <xsl:variable name="doc" select="document(concat($prefix, .))"/>
        <xsl:choose>
          <xsl:when test="count($doc/uc:component) = 1">
            <xsl:copy-of select="$doc/uc:component"/>
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
