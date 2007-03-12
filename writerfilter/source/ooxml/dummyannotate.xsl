<!--
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dummyannotate.xsl,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hbrinkm $ $Date: 2007-03-12 16:10:31 $
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
    xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" 
    xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" 
    xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" 
    xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" 
    xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" 
    xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" 
    xmlns:xlink="http://www.w3.org/1999/xlink" 
    xmlns:dc="http://purl.org/dc/elements/1.1/" 
    xmlns:meta="urn:oasis:names:tc:opendocument:xmlns:meta:1.0" 
    xmlns:number="urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0" 
    xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" 
    xmlns:chart="urn:oasis:names:tc:opendocument:xmlns:chart:1.0" 
    xmlns:dr3d="urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0" 
    xmlns:math="http://www.w3.org/1998/Math/MathML" 
    xmlns:form="urn:oasis:names:tc:opendocument:xmlns:form:1.0" 
    xmlns:script="urn:oasis:names:tc:opendocument:xmlns:script:1.0" 
    xmlns:config="urn:oasis:names:tc:opendocument:xmlns:config:1.0" 
    xmlns:ooo="http://openoffice.org/2004/office" 
    xmlns:ooow="http://openoffice.org/2004/writer" 
    xmlns:oooc="http://openoffice.org/2004/calc" 
    xmlns:dom="http://www.w3.org/2001/xml-events" 
    xmlns:xforms="http://www.w3.org/2002/xforms" 
    xmlns:xsd="http://www.w3.org/2001/XMLSchema" 
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
    xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"  
    xmlns:rng="http://relaxng.org/ns/structure/1.0"
    xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#" 
    xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
    xmlns:UML = 'org.omg.xmi.namespace.UML' xml:space="default" xml:indent="true">
  <xsl:output method="xml" />
  
  <xsl:template match="rng:define" mode="propertysprm">
    <xsl:element name="resource">
      <xsl:variable name="name" select="@name"/>
      <xsl:attribute name="name">
        <xsl:value-of select="@name"/>
      </xsl:attribute>
      <xsl:attribute name="resource">Property</xsl:attribute>
      <xsl:for-each select=".//rng:attribute|.//rng:element">
        <xsl:element name="sprm">
          <xsl:attribute name="name">
            <xsl:value-of select="@name"/>
          </xsl:attribute>
          <xsl:attribute name="tokenid">0x0</xsl:attribute>
        </xsl:element>
      </xsl:for-each>
    </xsl:element>
  </xsl:template>

  <xsl:template match="rng:define" mode="propertiesattr">
    <xsl:element name="resource">
      <xsl:variable name="name" select="@name"/>
      <xsl:attribute name="name">
        <xsl:value-of select="@name"/>
      </xsl:attribute>
      <xsl:attribute name="resource">PropertySetValue</xsl:attribute>
      <xsl:for-each select=".//rng:attribute|.//rng:element">
        <xsl:element name="attribute">
          <xsl:attribute name="name">
            <xsl:value-of select="@name"/>
          </xsl:attribute>
          <xsl:attribute name="tokenid">
            <xsl:text>ooxml:</xsl:text>
            <xsl:value-of select="$name"/>
            <xsl:text>_</xsl:text>
            <xsl:value-of select="@name"/>
          </xsl:attribute>
        </xsl:element>
      </xsl:for-each>
    </xsl:element>
  </xsl:template>
  
  <xsl:template match="rng:define" mode="dummydoc">
    <xsl:element name="w:document">
      <xsl:for-each select=".//rng:attribute">
        <xsl:element name="w:p">
          <xsl:element name="w:pPr">
            <xsl:element name="w:numPr">
              <xsl:attribute name="w:{@name}"/>
            </xsl:element>
          </xsl:element>
          <xsl:element name="w:r">
            <xsl:element name="w:t">
              <xsl:value-of select="@name"/>
            </xsl:element>
          </xsl:element>
        </xsl:element>
      </xsl:for-each>
    </xsl:element>
  </xsl:template>

  <xsl:template match="rng:define" mode="attrvalue">
    <xsl:variable name="name" select="@name"/>
    <xsl:element name="resource">
      <xsl:attribute name="name">
        <xsl:value-of select="@name"/>
      </xsl:attribute>
      <xsl:attribute name="resource">AttributeValue</xsl:attribute>
      <xsl:for-each select=".//rng:value">
        <xsl:element name="value">
          <xsl:attribute name="tokenid">
            <xsl:text>ooxml:Value_</xsl:text>
            <xsl:value-of select="$name"/>
            <xsl:text>_</xsl:text>
            <xsl:value-of select="."/>
          </xsl:attribute>
          <xsl:value-of select="."/>
        </xsl:element>
      </xsl:for-each>
    </xsl:element>
  </xsl:template>

  <xsl:template match="rng:define" mode="dummydocvalues">
    <w:body>
    <xsl:for-each select=".//rng:value">
      <w:p>
        <w:pPr>
          <w:pBdr>
            <w:top>
              <xsl:attribute name="w:val"><xsl:value-of select="."/></xsl:attribute>
              <xsl:attribute name="w:sz">4</xsl:attribute>
              <xsl:attribute name="w:space">1</xsl:attribute>
              <xsl:attribute name="w:color">auto</xsl:attribute>
            </w:top>
            <w:left>
              <xsl:attribute name="w:val"><xsl:value-of select="."/></xsl:attribute>
              <xsl:attribute name="w:sz">4</xsl:attribute>
              <xsl:attribute name="w:space">4</xsl:attribute>
              <xsl:attribute name="w:color">auto</xsl:attribute>
            </w:left>
            <w:bottom>
              <xsl:attribute name="w:val"><xsl:value-of select="."/></xsl:attribute>
              <xsl:attribute name="w:sz">4</xsl:attribute>
              <xsl:attribute name="w:space">1</xsl:attribute>
              <xsl:attribute name="w:color">auto</xsl:attribute>
            </w:bottom>
            <w:between>
              <xsl:attribute name="w:val"><xsl:value-of select="."/></xsl:attribute>
              <xsl:attribute name="w:sz">4</xsl:attribute>
              <xsl:attribute name="w:space">1</xsl:attribute>
              <xsl:attribute name="w:color">auto</xsl:attribute>
            </w:between>
            <w:bar>
              <xsl:attribute name="w:val"><xsl:value-of select="."/></xsl:attribute>
              <xsl:attribute name="w:sz">4</xsl:attribute>
              <xsl:attribute name="w:color">auto</xsl:attribute>
            </w:bar>
          </w:pBdr>
        </w:pPr>
        <w:r>
          <w:t><xsl:value-of select="."/></w:t>
        </w:r>
      </w:p>
    </xsl:for-each>
    </w:body>
  </xsl:template>

  <xsl:template match="/">
    <xsl:apply-templates select="//rng:define[@name='ST_Border']" mode="dummydocvalues"/>
  </xsl:template>
</xsl:stylesheet>