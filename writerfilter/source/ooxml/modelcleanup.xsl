<!--
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: modelcleanup.xsl,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: hbrinkm $ $Date: 2007-04-20 14:57:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
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
           match="resource" use="@name" />

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
              <xsl:element name="resource">
                <xsl:attribute name="name">
                  <xsl:value-of select="@name"/>
                </xsl:attribute>
                <xsl:attribute name="resource">
                  <xsl:text>StringValue</xsl:text>
                </xsl:attribute>
                <xsl:attribute name="generated">
                  <xsl:text>yes</xsl:text>
                </xsl:attribute>
              </xsl:element>
            </xsl:if>
          </xsl:if>
        </xsl:if>
      </xsl:for-each>
    </xsl:element>
  </xsl:template>

  <xsl:template match="/">
    <model>
      <xsl:apply-templates select=".//namespace"/>
    </model>
  </xsl:template>
</xsl:stylesheet>
