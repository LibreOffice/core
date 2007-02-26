<!--
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: modelpreprocess.xsl,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hbrinkm $ $Date: 2007-02-26 15:34:50 $
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
    xmlns:UML = 'org.omg.xmi.namespace.UML' xml:space="default">
  <xsl:output method="xml" />

  <xsl:template match="/">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template name="prefixfromurl">
    <xsl:param name="url"/>
    <xsl:value-of select="translate(substring-after($url, 'http://'), '/.', '__')"/>
  </xsl:template>

  <xsl:template name="prefixforgrammar">
    <xsl:variable name="ns" select="ancestor::rng:grammar/@ns"/>
    <xsl:call-template name="prefixfromurl">
      <xsl:with-param name="url" select="$ns"/>
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="nsforgrammar">
    <xsl:value-of select="ancestor::rng:grammar/@ns"/>
  </xsl:template>

  <xsl:template match="rng:element[@name] | rng:attribute[@name]">
    <xsl:variable name="prefix">
      <xsl:choose>
        <xsl:when test="contains(@name, ':')">
          <xsl:variable name="myname" select="@name"/>
          <xsl:call-template name="prefixfromurl">
            <xsl:with-param name="url" select="string(ancestor::rng:grammar/namespace::*[local-name(.) = substring-before($myname, ':')])"/>
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="prefixforgrammar"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="ns">
      <xsl:choose>
        <xsl:when test="contains(@name, ':')">
          <xsl:variable name="myname" select="@name"/>
          <xsl:value-of select="string(ancestor::rng:grammar/namespace::*[local-name(.) = substring-before($myname, ':')])"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="nsforgrammar"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="localname">
      <xsl:choose>
        <xsl:when test="contains(@name, ':')">
          <xsl:value-of select="substring-after(@name, ':')"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="@name"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:copy>
      <xsl:apply-templates select="@*"/>
      <xsl:attribute name="enumname">
        <xsl:value-of select="$prefix"/>
        <xsl:text>:</xsl:text>
        <xsl:value-of select="$localname"/>
      </xsl:attribute>
      <xsl:attribute name="qname">
        <xsl:value-of select="$ns"/>
        <xsl:text>:</xsl:text>
        <xsl:value-of select="$localname"/>
      </xsl:attribute>
      <xsl:apply-templates/>      
    </xsl:copy>    
  </xsl:template>

  <xsl:template match="node()|@*">
   <xsl:copy>
   <xsl:apply-templates select="@*"/>
   <xsl:apply-templates/>
   </xsl:copy>
  </xsl:template>

  <xsl:template match="model">
   <xsl:copy>
   <xsl:apply-templates select="@*"/>
   <xsl:apply-templates/>
   </xsl:copy>
  </xsl:template>

  <xsl:template match="namespace">
    <xsl:variable name="ns" select=".//rng:grammar/@ns"/>
   <xsl:copy>
     <xsl:apply-templates select="@*"/>
     <xsl:attribute name="prefix"><xsl:value-of select="translate(substring-after($ns, 'http://schemas.openxmlformats.org/'), '/', '_')"/></xsl:attribute>
     <xsl:apply-templates/>
   </xsl:copy>
  </xsl:template>
</xsl:stylesheet>