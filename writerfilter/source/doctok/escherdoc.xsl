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

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:meta="urn:oasis:names:tc:opendocument:xmlns:meta:1.0" xmlns:number="urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0" xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" xmlns:chart="urn:oasis:names:tc:opendocument:xmlns:chart:1.0" xmlns:dr3d="urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0" xmlns:math="http://www.w3.org/1998/Math/MathML" xmlns:form="urn:oasis:names:tc:opendocument:xmlns:form:1.0" xmlns:script="urn:oasis:names:tc:opendocument:xmlns:script:1.0" xmlns:config="urn:oasis:names:tc:opendocument:xmlns:config:1.0" xmlns:ooo="http://openoffice.org/2004/office" xmlns:ooow="http://openoffice.org/2004/writer" xmlns:oooc="http://openoffice.org/2004/calc" xmlns:dom="http://www.w3.org/2001/xml-events" xmlns:xforms="http://www.w3.org/2002/xforms" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"  
  xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#" xmlns:UML = 'org.omg.xmi.namespace.UML' xml:space="default" xmlns:xhtml="http://www.w3.org/1999/xhtml">
<xsl:output method="xml" indent="yes"/>

<xsl:template match="/">
  <out>    
    <xsl:for-each select="//xhtml:table">
      <xsl:apply-templates select="." mode="eschertable"/>
    </xsl:for-each>
  </out>
</xsl:template>

<xsl:template match="xhtml:table" mode="eschertable">
  <xsl:apply-templates select=".//xhtml:tr[position() > 1]" mode="eschertable"/>
</xsl:template>

<xsl:template match="xhtml:tr" mode="eschertable">
  <xsl:variable name="name"> 
    <xsl:value-of select=".//xhtml:td[1]"/>
  </xsl:variable>
  <UML:Class>
    <xsl:attribute name="xmi.id">
      <xsl:value-of select="$name"/>
    </xsl:attribute>
    <xsl:attribute name="name">
      <xsl:value-of select="$name"/>
    </xsl:attribute>
    <UML:ModelElement.stereotype>
      <UML:Stereotype xmi.idref="dffopt" />
    </UML:ModelElement.stereotype>
    <xsl:apply-templates select=".//xhtml:td[1]" mode="optname"/>
    <xsl:apply-templates select=".//xhtml:td[2]" mode="fopid"/>
    <xsl:apply-templates select=".//xhtml:td[3]" mode="type"/>
    <xsl:apply-templates select=".//xhtml:td[4]" mode="default"/>
    <xsl:apply-templates select=".//xhtml:td[5]" mode="comment"/>
  </UML:Class>
</xsl:template>

<xsl:template match="xhtml:td" mode="optname">
  <UML:ModelElement.taggedValue>
    <UML:TaggedValue>
      <UML:TaggedValue.dataValue>rtf:shp<xsl:value-of select="."/></UML:TaggedValue.dataValue>
      <UML:TaggedValue.type>
        <UML:TagDefinition xmi.idref="optname" />
      </UML:TaggedValue.type>
    </UML:TaggedValue>
  </UML:ModelElement.taggedValue>
</xsl:template>

<xsl:template match="xhtml:td" mode="fopid">
  <UML:ModelElement.taggedValue>
    <UML:TaggedValue>
      <UML:TaggedValue.dataValue><xsl:value-of select="."/></UML:TaggedValue.dataValue>
      <UML:TaggedValue.type>
        <UML:TagDefinition xmi.idref="fopid" />
      </UML:TaggedValue.type>
    </UML:TaggedValue>
  </UML:ModelElement.taggedValue>
</xsl:template>

<xsl:template match="xhtml:td" mode="type">
  <UML:ModelElement.taggedValue>
    <UML:TaggedValue>
      <UML:TaggedValue.dataValue>
        <xsl:call-template name="saltype">
          <xsl:with-param name="type" select="."/>
        </xsl:call-template>
      </UML:TaggedValue.dataValue>
      <UML:TaggedValue.type>
        <UML:TagDefinition xmi.idref="type" />
      </UML:TaggedValue.type>
    </UML:TaggedValue>
  </UML:ModelElement.taggedValue>
  <xsl:if test=".='BOOL'">
    <UML:ModelElement.taggedValue>
      <UML:TaggedValue>
        <UML:TaggedValue.dataValue>1</UML:TaggedValue.dataValue>
        <UML:TaggedValue.type>
          <UML:TagDefinition xmi.idref="isbool" />
        </UML:TaggedValue.type>
      </UML:TaggedValue>
    </UML:ModelElement.taggedValue>
  </xsl:if>
</xsl:template>

<xsl:template match="xhtml:td" mode="default">
  <UML:ModelElement.taggedValue>
    <UML:TaggedValue>
      <UML:TaggedValue.dataValue><xsl:value-of select="."/></UML:TaggedValue.dataValue>
      <UML:TaggedValue.type>
        <UML:TagDefinition xmi.idref="default" />
      </UML:TaggedValue.type>
    </UML:TaggedValue>
  </UML:ModelElement.taggedValue>
</xsl:template>

<xsl:template match="xhtml:td" mode="comment">
  <UML:ModelElement.taggedValue>
    <UML:TaggedValue>
      <UML:TaggedValue.dataValue><xsl:value-of select="."/></UML:TaggedValue.dataValue>
      <UML:TaggedValue.type>
        <UML:TagDefinition xmi.idref="comment" />
      </UML:TaggedValue.type>
    </UML:TaggedValue>
  </UML:ModelElement.taggedValue>
</xsl:template>

<xsl:template name="saltype">
  <xsl:param name="type"/>
  <xsl:choose>
    <xsl:when test="$type='LONG'">U32</xsl:when>
    <xsl:when test="$type='BOOL'">U8</xsl:when>
    <xsl:when test="$type='WCHAR*'">String</xsl:when>
    <xsl:otherwise>unknown</xsl:otherwise>
  </xsl:choose>
</xsl:template>

</xsl:stylesheet>