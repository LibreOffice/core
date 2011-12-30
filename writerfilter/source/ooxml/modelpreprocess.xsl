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

  <xsl:key name="namespace-aliases" match="//namespace-alias" use="@name"/>

  <xsl:template match="/">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template name="prefixfromurl">
    <xsl:param name="url"/>
    <xsl:variable name="prefix" select="key('namespace-aliases', $url)/@alias"/>
    <xsl:choose>
      <xsl:when test="string-length($prefix) > 0">
        <xsl:value-of select="$prefix"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="translate(substring-after($url, 'http://'), '/.', '__')"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="prefixforgrammar">
    <xsl:variable name="ns" select="ancestor::namespace/rng:grammar/@ns"/>
    <xsl:variable name="prefix" select="key('namespace-aliases', $ns)/@alias"/>
    <xsl:choose>
      <xsl:when test="string-length($prefix) > 0">
        <xsl:value-of select="$prefix"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="prefixfromurl">
          <xsl:with-param name="url" select="$ns"/>
        </xsl:call-template>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="nsforgrammar">
    <xsl:value-of select="ancestor::namespace/rng:grammar/@ns"/>
  </xsl:template>

  <xsl:template match="rng:element[@name|./rng:anyName] | rng:attribute[@name] | element | attribute">
    <xsl:variable name="prefix">
      <xsl:choose>
        <xsl:when test="contains(@name, ':')">
          <xsl:variable name="myname" select="@name"/>
          <xsl:call-template name="prefixfromurl">
            <xsl:with-param name="url" select="string(namespace::*[local-name(.) = substring-before($myname, ':')])"/>
          </xsl:call-template>
        </xsl:when>
        <xsl:when test="name(.)='attribute'">
          <xsl:if test="ancestor::namespace/rng:grammar/@attributeFormDefault='qualified'">
            <xsl:call-template name="prefixforgrammar"/>
          </xsl:if>
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
          <xsl:value-of select="string(namespace::*[local-name(.) = substring-before($myname, ':')])"/>
        </xsl:when>
        <xsl:when test="name(.)='attribute'">
          <xsl:if test="ancestor::rng:grammar/@attributeFormDefault='qualified'">
            <xsl:call-template name="nsforgrammar"/>
          </xsl:if>
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
	<xsl:when test="./rng:anyName">
	  <xsl:text>FAST_TOKENS_END</xsl:text>
	</xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="@name"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:copy>
      <xsl:apply-templates select="@*"/>
      <xsl:attribute name="enumname">
        <xsl:if test="string-length($prefix) > 0">
          <xsl:value-of select="$prefix"/>
          <xsl:text>:</xsl:text>
        </xsl:if>
        <xsl:value-of select="$localname"/>
      </xsl:attribute>
      <xsl:attribute name="qname">
        <xsl:if test="string-length($ns) > 0">
          <xsl:value-of select="$ns"/>
          <xsl:text>:</xsl:text>
        </xsl:if>
        <xsl:value-of select="$localname"/>
      </xsl:attribute>
      <xsl:attribute name="prefix">
        <xsl:value-of select="$prefix"/>
      </xsl:attribute>
      <xsl:attribute name="localname">
        <xsl:value-of select="$localname"/>
      </xsl:attribute>      
      <xsl:apply-templates/>      
    </xsl:copy>    
  </xsl:template>

  <xsl:template match="rng:grammar">
   <xsl:copy>
     <xsl:apply-templates select="@*"/>
     <xsl:attribute name="application">
       <xsl:value-of select="substring-before(substring-after(@ns, 'http://schemas.openxmlformats.org/'), '/')"/>
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

  <xsl:template match="rng:define|rng:ref">
   <xsl:copy>
     <xsl:apply-templates select="@*"/>
     <xsl:attribute name="classfordefine">
       <xsl:variable name="name" select="@name"/>
       <xsl:choose>
         <xsl:when test="(starts-with(@name, 'CT_') or starts-with(@name, 'EG_') or starts-with(@name, 'AG_'))">1</xsl:when>
         <xsl:when test="ancestor::namespace//start[@name=$name]">1</xsl:when>
         <xsl:otherwise>0</xsl:otherwise>
       </xsl:choose>
     </xsl:attribute>
     <xsl:apply-templates/>
   </xsl:copy>
  </xsl:template>

  <xsl:template match="namespace">
    <xsl:variable name="ns" select=".//rng:grammar/@ns"/>
   <xsl:copy>
     <xsl:apply-templates select="@*"/>
     <xsl:attribute name="namespacealias">
       <xsl:value-of select="key('namespace-aliases', $ns)/@alias"/>
     </xsl:attribute>
     <xsl:attribute name="prefix"><xsl:value-of select="translate(substring-after($ns, 'http://schemas.openxmlformats.org/'), '/-', '__')"/></xsl:attribute>
     <xsl:apply-templates/>
   </xsl:copy>
  </xsl:template>
</xsl:stylesheet>
