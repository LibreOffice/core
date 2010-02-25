<!--
*************************************************************************
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
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:meta="urn:oasis:names:tc:opendocument:xmlns:meta:1.0" xmlns:number="urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0" xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" xmlns:chart="urn:oasis:names:tc:opendocument:xmlns:chart:1.0" xmlns:dr3d="urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0" xmlns:math="http://www.w3.org/1998/Math/MathML" xmlns:form="urn:oasis:names:tc:opendocument:xmlns:form:1.0" xmlns:script="urn:oasis:names:tc:opendocument:xmlns:script:1.0" xmlns:config="urn:oasis:names:tc:opendocument:xmlns:config:1.0" xmlns:ooo="http://openoffice.org/2004/office" xmlns:ooow="http://openoffice.org/2004/writer" xmlns:oooc="http://openoffice.org/2004/calc" xmlns:dom="http://www.w3.org/2001/xml-events" xmlns:xforms="http://www.w3.org/2002/xforms" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"  
  xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#" xmlns:UML = 'org.omg.xmi.namespace.UML' xml:space="default" xmlns:xhtml="http://www.w3.org/1999/xhtml">
<xsl:output method="xml" indent="yes"/>

<xsl:template match="/">
<UML:Class xmi.id="dummy" name="dummy">
                    <UML:ModelElement.stereotype>
                        <UML:Stereotype xmi.idref='access'/>
                    </UML:ModelElement.stereotype>
<xsl:apply-templates select="//xhtml:body/xhtml:table"/>
</UML:Class>
</xsl:template>

<xsl:template match="xhtml:table">
<xsl:apply-templates select="xhtml:tbody/xhtml:tr[2]"/>
</xsl:template>

<xsl:template name="handleTR">
<xsl:param name="offset"/>
<xsl:param name="shift"/>
<xsl:variable name="name"><xsl:value-of select="xhtml:td[3]"/></xsl:variable>
<xsl:variable name="type">
<xsl:choose>
<xsl:when test='contains(xhtml:td[4], "[")'>
<xsl:value-of select='substring-before(xhtml:td[4], "[")'/>
</xsl:when>
<xsl:otherwise>
<xsl:value-of select="xhtml:td[4]"/>
<xsl:variable name="stereotype">attribute</xsl:variable>
</xsl:otherwise>
</xsl:choose>
</xsl:variable>
<xsl:variable name='stereotype'>
<xsl:choose>
<xsl:when test='contains(xhtml:td[4], "[")'>array</xsl:when>
<xsl:otherwise>attribute</xsl:otherwise>
</xsl:choose>
</xsl:variable>
<xsl:variable name="saltype">
<xsl:choose>
  <xsl:when test="$type='U8'">sal_uInt8</xsl:when>
  <xsl:when test="$type='S8'">sal_Int8</xsl:when>
  <xsl:when test="$type='U16'">sal_uInt16</xsl:when>
  <xsl:when test="$type='S16'">sal_Int16</xsl:when>
  <xsl:when test="$type='U32'">sal_uInt32</xsl:when>
  <xsl:when test="$type='S32'">sal_Int32</xsl:when>
  <xsl:otherwise>void *</xsl:otherwise>
</xsl:choose>
</xsl:variable>
<xsl:variable name="bits"><xsl:value-of select="xhtml:td[5]"/></xsl:variable>
<xsl:variable name="mask"><xsl:value-of select="xhtml:td[6]"/></xsl:variable>
<xsl:variable name="comment"><xsl:value-of select="xhtml:td[7]"/></xsl:variable>
					<UML:Classifier.feature>
  						<UML:Attribute><xsl:attribute name="name"><xsl:value-of select="$name"/></xsl:attribute>
    		                <UML:ModelElement.taggedValue>
                                <UML:TaggedValue>
                                    <UML:TaggedValue.dataValue><xsl:value-of select="$comment"/></UML:TaggedValue.dataValue>
                                        <UML:TaggedValue.type>
                                        <UML:TagDefinition xmi.idref = 'comment'/>
                                    </UML:TaggedValue.type>
                                </UML:TaggedValue>
		                    </UML:ModelElement.taggedValue>
    		                <UML:ModelElement.taggedValue>
                                <UML:TaggedValue>
                                    <UML:TaggedValue.dataValue><xsl:value-of select="$offset"/></UML:TaggedValue.dataValue>
                                        <UML:TaggedValue.type>
                                        <UML:TagDefinition xmi.idref = 'offset'/>
                                    </UML:TaggedValue.type>
                                </UML:TaggedValue>
		                    </UML:ModelElement.taggedValue>
    		                <UML:ModelElement.taggedValue>
                                <UML:TaggedValue>
                                    <UML:TaggedValue.dataValue><xsl:value-of select="$shift"/></UML:TaggedValue.dataValue>
                                        <UML:TaggedValue.type>
                                        <UML:TagDefinition xmi.idref = 'shift'/>
                                    </UML:TaggedValue.type>
                                </UML:TaggedValue>
		                    </UML:ModelElement.taggedValue>
    		                <UML:ModelElement.taggedValue>
                                <UML:TaggedValue>
                                    <UML:TaggedValue.dataValue><xsl:value-of select="$mask"/></UML:TaggedValue.dataValue>
                                        <UML:TaggedValue.type>
                                        <UML:TagDefinition xmi.idref = 'mask'/>
                                    </UML:TaggedValue.type>
                                </UML:TaggedValue>
		                    </UML:ModelElement.taggedValue>
    		                <UML:ModelElement.taggedValue>
                                <UML:TaggedValue>
                                    <UML:TaggedValue.dataValue><xsl:value-of select="$bits"/></UML:TaggedValue.dataValue>
                                        <UML:TaggedValue.type>
                                        <UML:TagDefinition xmi.idref = 'bits'/>
                                    </UML:TaggedValue.type>
                                </UML:TaggedValue>
		                    </UML:ModelElement.taggedValue>
    		                <UML:ModelElement.taggedValue>
                                <UML:TaggedValue>
                                    <UML:TaggedValue.dataValue>rtf:<xsl:value-of select='translate($name, "abcdefghijklmnopqrstuvwxyz", "ABCDEFGHIJKLMNOPQRSTUVWXYZ")'/></UML:TaggedValue.dataValue>
                                        <UML:TaggedValue.type>
                                        <UML:TagDefinition xmi.idref = 'attrid'/>
                                    </UML:TaggedValue.type>
                                </UML:TaggedValue>
		                    </UML:ModelElement.taggedValue>
<xsl:if test='$stereotype = "array"'>
<xsl:variable name='arraycount'><xsl:value-of select='substring-before(substring-after(xhtml:td[4], "["), "]")'/></xsl:variable>
    		                <UML:ModelElement.taggedValue>
                                <UML:TaggedValue>
                                    <UML:TaggedValue.dataValue><xsl:value-of select="$arraycount"/></UML:TaggedValue.dataValue>
                                        <UML:TaggedValue.type>
                                        <UML:TagDefinition xmi.idref = 'arraycount'/>
                                    </UML:TaggedValue.type>
                                </UML:TaggedValue>
		                    </UML:ModelElement.taggedValue>
</xsl:if>
							<UML:StructuralFeature.type>
								<UML:DataType><xsl:attribute name="xmi.idref"><xsl:value-of select="$type"/></xsl:attribute></UML:DataType>
							</UML:StructuralFeature.type>
							<UML:ModelElement.stereotype>
								<UML:Stereotype><xsl:attribute name="xmi.idref"><xsl:value-of select="$stereotype"/></xsl:attribute></UML:Stereotype>
							</UML:ModelElement.stereotype>
						</UML:Attribute>
					</UML:Classifier.feature>
</xsl:template>

<xsl:template match="xhtml:tr[./xhtml:td[position()=1]/text()]">
<xsl:variable name="offset"><xsl:value-of select="xhtml:td[2]"/></xsl:variable>
<xsl:call-template name="handleTR">
<xsl:with-param name="offset" select="$offset"/>
<xsl:with-param name="shift">0</xsl:with-param>
</xsl:call-template>
<xsl:variable name="shift1"><xsl:value-of select="substring-after(xhtml:td[5], ':')"/></xsl:variable>
<xsl:apply-templates select="following-sibling::xhtml:tr[position()=1]">
<xsl:with-param name="offset" select="$offset"/>
<xsl:with-param name="shift" select="$shift1"/>
</xsl:apply-templates>
</xsl:template>

<xsl:template match="xhtml:tr[not(./xhtml:td[position()=1]/text())]">
<xsl:param name="offset"/>
<xsl:param name="shift"/>
<xsl:call-template name="handleTR">
<xsl:with-param name="offset" select="$offset"/>
<xsl:with-param name="shift" select="$shift"/>
</xsl:call-template>
<xsl:variable name="shift1"><xsl:value-of select="$shift+substring-after(xhtml:td[5], ':')"/></xsl:variable>
<xsl:apply-templates select="following-sibling::xhtml:tr[position()=1]">
<xsl:with-param name="offset" select="$offset"/>
<xsl:with-param name="shift" select="$shift1"/>
</xsl:apply-templates>
</xsl:template>

<xsl:template match="*">
<xsl:copy-of select="."/>
</xsl:template>

</xsl:stylesheet>
