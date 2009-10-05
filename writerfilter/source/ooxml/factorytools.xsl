<!--
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: $
 *
 * $Revision: $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 *  <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
    xmlns:xalan="http://xml.apache.org/xalan"
    xmlns:UML = 'org.omg.xmi.namespace.UML' 
    xmlns:common="http://exslt.org/common"
    exclude-result-prefixes = "xalan"
    xml:space="default">
  
<!-- 
     KEYS
-->
<xsl:key name="context-resource"
         match="resource" use="@name"/>

<xsl:key name="tokenids" match="@tokenid|@sendtokenid" use="."/>

<xsl:key name="actions" match="/model/namespace/resource/action"
    use="@name"/>

<xsl:key name="definename" match="//rng:define" use="@name"/>

<xsl:key name="resources" match="/model/namespace/resource" use="@resource"/>

<xsl:key name="value-with-content" match="//rng:value"
         use="text()"/>

<xsl:key name="same-token-name" match="rng:element|rng:attribute" use="@localname"/>

<!-- license header -->
<xsl:template name="licenseheader">
  <xsl:text>
/*      

    THIS FILE IS GENERATED AUTOMATICALLY! DO NOT EDIT!
    
*/
&#xa;</xsl:text>
</xsl:template>

<!--
IDENTIFIERs
-->

<!--

generates token identifer for a namespace

NN_<namespace/@name>

-->
<xsl:template name="fastnamespace">
  <xsl:if test="string-length(@prefix) > 0">
    <xsl:text>NS_</xsl:text>
    <xsl:value-of select="@prefix"/>
  </xsl:if>
</xsl:template>

<!--

generates local part of token identifier for a rng:define

OOXML_<rng:define/@localname>

-->
<xsl:template name="fastlocalname">
  <xsl:text>OOXML_</xsl:text>
  <xsl:value-of select="@localname"/>
</xsl:template>

<!-- 

generates identifier for a token

-->
<xsl:template name="fasttoken">
  <xsl:variable name="ns">
    <xsl:call-template name="fastnamespace"/>
  </xsl:variable>
  <xsl:if test="string-length($ns) > 0">
    <xsl:value-of select="$ns"/>
    <xsl:text>|</xsl:text>
  </xsl:if>
  <xsl:call-template name="fastlocalname"/>
</xsl:template>

<!-- generates identifier for a namespace 

NN_<namespace/@name>

-->
<xsl:template name="idfornamespace">
    <xsl:text>NN_</xsl:text>
    <xsl:value-of select="translate(@name, '-', '_')"/>
</xsl:template>

<!-- generates local part of identier for rng:define 

DEFINE_<rng:define/@name>

-->
<xsl:template name="localidfordefine">
    <xsl:text>DEFINE_</xsl:text>
    <xsl:value-of select="@name"/>
</xsl:template>

<!-- generates id for a rng:define
   
NN_<namespace/@name> | DEFINE_<rng:define/@name>
   
-->
<xsl:template name="idfordefine">
    <xsl:for-each select="ancestor::namespace">
        <xsl:call-template name="idfornamespace"/>
    </xsl:for-each>
    <xsl:text>|</xsl:text>
    <xsl:call-template name="localidfordefine"/>
</xsl:template>

<!-- 

creates case label for a rng:define:

case NN_<namesapce/@name> | DEFINE_<rng:define/@name>:

-->
<xsl:template name="caselabeldefine">
  <xsl:text>case </xsl:text>
  <xsl:call-template name="idfordefine"/>
  <xsl:text>:</xsl:text>
</xsl:template>

<!--
    Generates name for a value string.
    
    Value strings are possible values for attributes in OOXML.
    
    @param string    the string as present in the according <rng:value>
-->
<xsl:template name="valuestringname">
  <xsl:param name="string"/>
  <xsl:text>OOXMLValueString_</xsl:text>
  <xsl:value-of select="translate($string, '-+ ,', 'mp__')"/>
</xsl:template>

<!--
    Generates qname for id.
    
    @param id     the id to generate qname for
    
    If id is of format <prefix>:<localname> the result is
    
    NS_<prefix>::LN_<localname>
    
    If id does not contain ":" the result is just id.
-->
<xsl:template name='idtoqname'>
  <xsl:param name='id'/>
  <xsl:choose>
    <xsl:when test="contains($id, ':')">
      <xsl:text>NS_</xsl:text>
      <xsl:value-of select='substring-before($id, ":")'/>
      <xsl:text>::LN_</xsl:text>
      <xsl:value-of select='substring-after($id, ":")'/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$id"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!--
    Returns the identifier for a namespace.

NS_<namespace/@alias>

-->
<xsl:template name="namespaceid">
  <xsl:text>NS_</xsl:text>
  <xsl:value-of select="@alias"/>
</xsl:template>

<!--
    Returns the value of the @resource attribute of the <resource>
    node according to the current <define>.
-->
<xsl:template name="contextresource">
  <xsl:variable name="name" select="@name"/>
  <xsl:variable name="nsid" select="generate-id(ancestor::namespace)"/>
  <xsl:for-each select="key('context-resource', @name)">
    <xsl:if test="generate-id(ancestor::namespace) = $nsid">
      <xsl:value-of select="@resource"/>
    </xsl:if>
  </xsl:for-each>
</xsl:template>

<xsl:template name="idforref">
    <xsl:variable name="name" select="@name"/>
    <xsl:for-each select="ancestor::namespace/rng:grammar/rng:define[@name=$name]">
        <xsl:call-template name="idfordefine"/>
    </xsl:for-each>
</xsl:template>

<xsl:template name="factorycreateelementmapfromstart">
    <xsl:for-each select="start">
        <xsl:variable name="name" select="@name"/>
        <xsl:variable name="block">
            <xsl:for-each select="ancestor::namespace/rng:grammar/rng:define[@name=$name]">
                <xsl:call-template name="factorycreateelementmapinner">
                </xsl:call-template>
            </xsl:for-each>
        </xsl:variable>
        <xsl:if test="string-length($block) > 0">
            <xsl:text>
        /* start: </xsl:text>
            <xsl:value-of select="$name"/>
            <xsl:text>*/</xsl:text>
            <xsl:value-of select="$block"/>
        </xsl:if>
    </xsl:for-each>
</xsl:template>

<!-- factoryclassname -->
<xsl:template name="factoryclassname">
    <xsl:text>OOXMLFactory_</xsl:text>
    <xsl:value-of select="translate(@name, '-', '_')"/>
</xsl:template>

<xsl:template name="factoryincludes">
    <xsl:for-each select="/model/namespace">
        <xsl:text>
#include "OOXMLFactory_</xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>.hxx"</xsl:text>
    </xsl:for-each>
</xsl:template>

<xsl:template name="factorydefineiddecls">
    <xsl:for-each select="//rng:define">
        <xsl:sort select="@name"/>
        <xsl:if test="generate-id(key('definename', @name)[1]) = generate-id(.)">
            <xsl:text>
extern const Id </xsl:text>
            <xsl:call-template name="localidfordefine"/>
            <xsl:text>;</xsl:text>
        </xsl:if>
    </xsl:for-each>
</xsl:template>

<xsl:template name="factorydefineidimpls">
    <xsl:for-each select="//rng:define">
        <xsl:sort select="@name"/>
        <xsl:if test="generate-id(key('definename', @name)[1]) = generate-id(.)">
            <xsl:text>
const Id </xsl:text>
            <xsl:call-template name="localidfordefine"/>
            <xsl:text> = </xsl:text>
            <xsl:value-of select="position()"/>
            <xsl:text>;</xsl:text>
        </xsl:if>
    </xsl:for-each>
</xsl:template>

<xsl:template name="resources">
    <xsl:for-each select="/model/namespace/resource">
        <xsl:if test="generate-id(key('resources', @resource)[1])=generate-id(.)">
            <xsl:text>RT_</xsl:text>
            <xsl:value-of select="@resource"/>
            <xsl:text>,&#xa;</xsl:text>
        </xsl:if>
    </xsl:for-each>
</xsl:template>

</xsl:stylesheet>