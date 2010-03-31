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
    xmlns:xalan="http://xml.apache.org/xalan"
    exclude-result-prefixes = "xalan"
    xml:indent="true">
  <xsl:output method="xml" />


  <xsl:key name="resources"
           match="resource[not(@generated)]" use="@name" />

  <xsl:template name="generateresource">
    <xsl:param name="resource"/>
    <xsl:element name="resource">
      <xsl:attribute name="name">
        <xsl:value-of select="@name"/>
      </xsl:attribute>
      <xsl:attribute name="resource">
        <xsl:value-of select="$resource"/>
      </xsl:attribute>
      <xsl:attribute name="generated">
        <xsl:text>yes</xsl:text>
      </xsl:attribute>
    </xsl:element>
  </xsl:template>

  <xsl:key name="nsaliases" match="//namespace-alias" use="@name"/>

  <xsl:template name="resourcevalues">
    <xsl:variable name="definename" select="@name"/>
    <xsl:variable name="namespace" select="key('nsaliases', ancestor::namespace/rng:grammar/@ns)/@alias"/>
    <xsl:element name="resource">
      <xsl:attribute name="name">
        <xsl:value-of select="@name"/>
      </xsl:attribute>
      <xsl:attribute name="resource">List</xsl:attribute>
      <xsl:attribute name="generated">yes</xsl:attribute>
      <xsl:for-each select=".//rng:value">
        <xsl:element name="value">
          <xsl:attribute name="name">
            <xsl:value-of select="translate(., '-+ ,', 'mp__')"/>
          </xsl:attribute>
          <xsl:attribute name="tokenid">
            <xsl:text>ooxml:Value_</xsl:text>
            <xsl:value-of select="translate($namespace, '-', '_')"/>
            <xsl:text>_</xsl:text>
            <xsl:value-of select="$definename"/>
            <xsl:text>_</xsl:text>
            <xsl:value-of select="translate(., '-+ ,', 'mp__')"/>
          </xsl:attribute>
          <xsl:value-of select="."/>
        </xsl:element>
      </xsl:for-each>
    </xsl:element>
  </xsl:template>

  <xsl:template match="namespace">
    <xsl:variable name="nsid" select="generate-id(.)"/>
    <xsl:element name="namespace">
      <xsl:copy-of select="@*"/>
      <xsl:copy-of select=".//start"/>
      <xsl:copy-of select="./rng:grammar"/>

      <xsl:for-each select=".//rng:define">
        <xsl:variable name="resources" select="key('resources', @name)[generate-id(ancestor::namespace) = $nsid]"/>
        <xsl:copy-of select="$resources"/>
        <xsl:if test="count($resources) = 0">
          <xsl:if test="substring(@name, 1, 3) = 'CT_'">
            <xsl:if test="./rng:attribute[@name='val']/rng:text">
              <xsl:call-template name="generateresource">
                <xsl:with-param name="resource">StringValue</xsl:with-param>
              </xsl:call-template>
            </xsl:if>
          </xsl:if>
          <xsl:if test="substring(@name, 1, 3) = 'ST_'">
            <xsl:if test="./rng:data[@type='int']">
              <xsl:call-template name="generateresource">
                <xsl:with-param name="resource">Integer</xsl:with-param>
              </xsl:call-template>
            </xsl:if>
            <xsl:if test="./rng:data[@type='integer']">
              <xsl:call-template name="generateresource">
                <xsl:with-param name="resource">Integer</xsl:with-param>
              </xsl:call-template>
            </xsl:if>
            <xsl:if test="./rng:data[@type='long']">
              <xsl:call-template name="generateresource">
                <xsl:with-param name="resource">Integer</xsl:with-param>
              </xsl:call-template>
            </xsl:if>
            <xsl:if test="./rng:data[@type='unsignedInt']">
              <xsl:call-template name="generateresource">
                <xsl:with-param name="resource">Integer</xsl:with-param>
              </xsl:call-template>
            </xsl:if>
            <xsl:if test="./rng:data[@type='unsignedLong']">
              <xsl:call-template name="generateresource">
                <xsl:with-param name="resource">Integer</xsl:with-param>
              </xsl:call-template>
            </xsl:if>
            <xsl:if test="./rng:data[@type='boolean']">
              <xsl:call-template name="generateresource">
                <xsl:with-param name="resource">Boolean</xsl:with-param>
              </xsl:call-template>
            </xsl:if>
            <xsl:if test="./rng:data[@type='token']">
              <xsl:call-template name="generateresource">
                <xsl:with-param name="resource">String</xsl:with-param>
              </xsl:call-template>
            </xsl:if>
            <xsl:if test="./rng:data[@type='string']">
              <xsl:call-template name="generateresource">
                <xsl:with-param name="resource">String</xsl:with-param>
              </xsl:call-template>
            </xsl:if>
            <xsl:if test="./rng:data[@type='dateTime']">
              <xsl:call-template name="generateresource">
                <xsl:with-param name="resource">String</xsl:with-param>
              </xsl:call-template>
            </xsl:if>
            <xsl:if test="./rng:data[@type='hexBinary']">
              <xsl:call-template name="generateresource">
                <xsl:with-param name="resource">Hex</xsl:with-param>
              </xsl:call-template>
            </xsl:if>
            <xsl:if test="./rng:list">
              <xsl:call-template name="resourcevalues"/>
            </xsl:if>
          </xsl:if>
        </xsl:if>
      </xsl:for-each>
    </xsl:element>
  </xsl:template>
  
  <xsl:template match="namespace-alias">
    <namespace-alias>
      <xsl:for-each select="@*">
        <xsl:copy-of select="."/>
      </xsl:for-each>
    </namespace-alias>
  </xsl:template>

  <xsl:template match="token">
    <token>
      <xsl:for-each select="@*">
        <xsl:copy-of select="."/>
      </xsl:for-each>
    </token>
  </xsl:template>

  <xsl:template match="fasttoken">
    <xsl:copy-of select="."/>
  </xsl:template>

  <xsl:template match="/">
    <model>
      <xsl:apply-templates select=".//namespace-alias">
        <xsl:sort select="@id" data-type="number"/>
      </xsl:apply-templates>
      <xsl:apply-templates select=".//token"/>
      <xsl:apply-templates select=".//fasttoken"/>
      <xsl:apply-templates select=".//namespace"/>
    </model>
  </xsl:template>
</xsl:stylesheet>
