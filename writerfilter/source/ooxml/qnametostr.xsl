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
    exclude-result-prefixes = "xalan"
    xml:space="default">
  <xsl:output method="text" />

  <xsl:include href="factorytools.xsl"/>

  <!--
      Generates mapping from tokenids to strings. (DEBUG)
  -->
  <xsl:template name="qnametostr">
    <xsl:text>
void QNameToString::init_ooxml()
{
    /* ooxml */
    </xsl:text>
    <xsl:for-each select="//@tokenid">
      <xsl:if test="generate-id(.) = generate-id(key('tokenids', .)[1]) and contains(., 'ooxml:')">
        <xsl:text>
    mMap[</xsl:text>
    <xsl:call-template name="idtoqname">
      <xsl:with-param name="id" select="."/>
    </xsl:call-template>
    <xsl:text>] = "</xsl:text>
    <xsl:value-of select="."/>
    <xsl:text>";</xsl:text>
      </xsl:if>
    </xsl:for-each>
    <xsl:text>
}
    </xsl:text>
  </xsl:template>

  <xsl:template name="ooxmlidstoxml">
    <xsl:text>
void ooxmlsprmidsToXML(::std::ostream &amp; out)
{</xsl:text>
    <xsl:for-each select="//@tokenid">
      <xsl:if test="contains(., 'ooxml:') and generate-id(.) = generate-id(key('tokenids', .)[1]) and ancestor::element">
        <xsl:text>
    out &lt;&lt; "&lt;theid name=\"</xsl:text>
    <xsl:value-of select="."/>
    <xsl:text>\"&gt;</xsl:text>
    <xsl:value-of select="90000 + position()"/>
    <xsl:text>&lt;/theid&gt;" &lt;&lt; endl; </xsl:text>
      </xsl:if> 
    </xsl:for-each>
    <xsl:text>
}</xsl:text>
    <xsl:text>
void ooxmlidsToXML(::std::ostream &amp; out)
{</xsl:text>
    <xsl:for-each select="//@tokenid">
      <xsl:if test="contains(., 'ooxml:') and generate-id(.) = generate-id(key('tokenids', .)[1]) and ancestor::attribute">
        <xsl:text>
    out &lt;&lt; "&lt;theid name=\"</xsl:text>
    <xsl:value-of select="."/>
    <xsl:text>\"&gt;</xsl:text>
    <xsl:value-of select="90000 + position()"/>
    <xsl:text>&lt;/theid&gt;" &lt;&lt; endl; </xsl:text>
      </xsl:if> 
    </xsl:for-each>
    <xsl:text>
}</xsl:text>
  </xsl:template>

  <xsl:template match="/">
#include "ooxml/resourceids.hxx"
#include "resourcemodel/QNameToString.hxx"

namespace writerfilter
{
    <xsl:call-template name="qnametostr"/>
    <xsl:call-template name="ooxmlidstoxml"/>
}
  </xsl:template>

</xsl:stylesheet>
