<!--
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: resourcetools.xsl,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hbrinkm $ $Date: 2007-04-16 09:02:14 $
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
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:meta="urn:oasis:names:tc:opendocument:xmlns:meta:1.0" xmlns:number="urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0" xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" xmlns:chart="urn:oasis:names:tc:opendocument:xmlns:chart:1.0" xmlns:dr3d="urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0" xmlns:math="http://www.w3.org/1998/Math/MathML" xmlns:form="urn:oasis:names:tc:opendocument:xmlns:form:1.0" xmlns:script="urn:oasis:names:tc:opendocument:xmlns:script:1.0" xmlns:config="urn:oasis:names:tc:opendocument:xmlns:config:1.0" xmlns:ooo="http://openoffice.org/2004/office" xmlns:ooow="http://openoffice.org/2004/writer" xmlns:oooc="http://openoffice.org/2004/calc" xmlns:dom="http://www.w3.org/2001/xml-events" xmlns:xforms="http://www.w3.org/2002/xforms" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"  
  xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#" xmlns:UML = 'org.omg.xmi.namespace.UML' xml:space="default">
<xsl:output method="text" />

<xsl:template name='idtoqname'>
<xsl:param name='id'/>NS_<xsl:value-of select='substring-before($id, ":")'/>::LN_<xsl:value-of select='substring-after($id, ":")'/>
</xsl:template>

<xsl:template name="parenttype">
  <xsl:param name='type'/>
  <xsl:value-of select='//UML:Generalization[UML:Generalization.child/UML:Class/@xmi.idref=$type]/UML:Generalization.parent/UML:Class/@xmi.idref'/>
</xsl:template>

<xsl:template name='saltype'>
  <xsl:param name='type'/>
  <xsl:param name='parenttype'/>
  <xsl:choose>
    <xsl:when test="$type='U8'">sal_uInt8</xsl:when>
    <xsl:when test="$type='S8'">sal_Int8</xsl:when>
    <xsl:when test="$type='U16'">sal_uInt16</xsl:when>
    <xsl:when test="$type='S16'">sal_Int16</xsl:when>
    <xsl:when test="$type='U32'">sal_uInt32</xsl:when>
    <xsl:when test="$type='S32'">sal_Int32</xsl:when>
    <xsl:when test="$type='String'">rtl::OUString</xsl:when>
    <xsl:otherwise>
      <xsl:text>doctok::Reference &lt; </xsl:text>
      <xsl:value-of select='$parenttype'/>
      <xsl:text> &gt;::Pointer_t</xsl:text>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name='typetype'>
  <xsl:param name='type'/><xsl:choose>
  <xsl:when test="$type='U8'">simple</xsl:when>
  <xsl:when test="$type='S8'">simple</xsl:when>
  <xsl:when test="$type='U16'">simple</xsl:when>
  <xsl:when test="$type='S16'">simple</xsl:when>
  <xsl:when test="$type='U32'">simple</xsl:when>
  <xsl:when test="$type='S32'">simple</xsl:when>
  <xsl:when test="$type='String'">string</xsl:when>
  <xsl:when test="$type='Binary'">binary</xsl:when>
  <xsl:otherwise>complex</xsl:otherwise>
</xsl:choose>
</xsl:template>

<xsl:template name="valuetype">
  <xsl:param name="type"/>
  <xsl:choose>
    <xsl:when test='$type="U8"'>WW8IntValue</xsl:when>
    <xsl:when test='$type="S8"'>WW8IntValue</xsl:when>
    <xsl:when test='$type="U16"'>WW8IntValue</xsl:when>
    <xsl:when test='$type="S16"'>WW8IntValue</xsl:when>
    <xsl:when test='$type="U32"'>WW8IntValue</xsl:when>
    <xsl:when test='$type="S32"'>WW8IntValue</xsl:when>
    <xsl:otherwise>WW8PropertiesValue</xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="UML:Attribute" mode="valuetype">
  <xsl:call-template name="valuetype">
    <xsl:with-param name="type" select='.//UML:DataType/@xmi.idref'/>
  </xsl:call-template>
</xsl:template>

<xsl:template match='UML:Model' mode='qnametostr'>
<xsl:variable name='tmp'>map &lt; sal_uInt32, string &gt; </xsl:variable>
        /* Attributes */
<xsl:for-each select='.//UML:Attribute[@name!="reserved"]//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="attrid"]'>
<xsl:choose>
<xsl:when test='.//UML:Stereotype[@xmi.idref="noresolve"]'>
</xsl:when>
<xsl:otherwise>
<xsl:text>    mMap[</xsl:text>
<xsl:call-template name='idtoqname'>
<xsl:with-param name='id'><xsl:value-of select='.//UML:TaggedValue.dataValue'/></xsl:with-param>
</xsl:call-template>
<xsl:text>]= "</xsl:text>
<xsl:value-of select='.//UML:TaggedValue.dataValue'/>
<xsl:text>";
</xsl:text>
</xsl:otherwise>
</xsl:choose>
</xsl:for-each>
        /* Operations */
<xsl:for-each select='.//UML:Operation[@name!="reserved"]//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="opid"]'>
<xsl:choose>
<xsl:when test='.//UML:Stereotype[@xmi.idref="noresolve"]'>
</xsl:when>
<xsl:otherwise>
<xsl:text>    mMap[</xsl:text>
<xsl:call-template name='idtoqname'>
<xsl:with-param name='id'><xsl:value-of select='.//UML:TaggedValue.dataValue'/></xsl:with-param>
</xsl:call-template>
<xsl:text>] = "</xsl:text>
<xsl:value-of select='.//UML:TaggedValue.dataValue'/>
<xsl:text>";
</xsl:text>
</xsl:otherwise>
</xsl:choose>
</xsl:for-each>
        /* Classes */
<xsl:for-each select='.//UML:Class[@name!="reserved"]//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="classid"]'>
<xsl:text>    mMap[</xsl:text>
<xsl:call-template name='idtoqname'>
<xsl:with-param name='id'><xsl:value-of select='.//UML:TaggedValue.dataValue'/></xsl:with-param>
</xsl:call-template>
<xsl:text>] = "</xsl:text>
<xsl:value-of select='.//UML:TaggedValue.dataValue'/>
<xsl:text>";
</xsl:text>
</xsl:for-each>
</xsl:template>

<xsl:template match='UML:Model' mode='sprmcodetostr'>
SprmIdToString::SprmIdToString()
{
<xsl:variable name='tmp'>map &lt; sal_uInt32, string &gt; </xsl:variable>
<xsl:for-each select='.//UML:Class[.//UML:Stereotype/@xmi.idref="ww8sprm"]'>
    mMap[<xsl:value-of select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="sprmcode"]//UML:TaggedValue.dataValue'/>] = "<xsl:value-of select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="sprmid"]//UML:TaggedValue.dataValue'/>";</xsl:for-each>
}
</xsl:template>

</xsl:stylesheet>