<!--
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:meta="urn:oasis:names:tc:opendocument:xmlns:meta:1.0" xmlns:number="urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0" xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" xmlns:chart="urn:oasis:names:tc:opendocument:xmlns:chart:1.0" xmlns:dr3d="urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0" xmlns:math="http://www.w3.org/1998/Math/MathML" xmlns:form="urn:oasis:names:tc:opendocument:xmlns:form:1.0" xmlns:script="urn:oasis:names:tc:opendocument:xmlns:script:1.0" xmlns:config="urn:oasis:names:tc:opendocument:xmlns:config:1.0" xmlns:ooo="http://openoffice.org/2004/office" xmlns:ooow="http://openoffice.org/2004/writer" xmlns:oooc="http://openoffice.org/2004/calc" xmlns:dom="http://www.w3.org/2001/xml-events" xmlns:xforms="http://www.w3.org/2002/xforms" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"  
  xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#" xmlns:UML = 'org.omg.xmi.namespace.UML' xml:space="default">
<xsl:output method="text" />

  <xsl:include href="resourcetools.xsl"/>

<xsl:template match="/">
  <xsl:call-template name="licenseheader"/>
  <xsl:text>
#include &lt;doctok/resources.hxx&gt;

#ifndef INCLUDED_SPRMIDS_HXX
#include &lt;doctok/sprmids.hxx&gt;
#endif

namespace writerfilter {
namespace doctok {

using namespace ::std;
</xsl:text>
  <xsl:apply-templates select='/XMI/XMI.content/UML:Model' mode="sprmkind"/>
  <xsl:apply-templates select='.//UML:Class' mode='class'/>
  <xsl:apply-templates select='//UML:Model' mode='createsprmprops'/>
  <xsl:apply-templates select='/XMI/XMI.content/UML:Model' 
                       mode='createsprmbinary'/>
  <xsl:apply-templates select='/XMI/XMI.content/UML:Model' 
                       mode='createdffrecord'/>
  <xsl:apply-templates select='/XMI/XMI.content/UML:Model' mode='ww8foptename'/>
  <xsl:apply-templates select='/XMI/XMI.content/UML:Model' 
                       mode='isbooleandffopt'/>
  <xsl:text>
}}
</xsl:text>
</xsl:template>

  <xsl:template match="UML:Class" mode="class">
  </xsl:template>

  <xsl:template match="UML:Class" mode="resolveProperties">
    <xsl:param name="classname"/>
    <xsl:variable name="rHandler">
      <xsl:choose>
        <xsl:when test='.//UML:Attribute[@name!="reserved"]'>
          <xsl:if test='.//UML:Stereotype[@xmi.idref != "noresolve"]'>
            <xsl:text>rHandler</xsl:text>
          </xsl:if>
        </xsl:when>
        <xsl:when test='.//UML:Operation[@name!="reserved"]'>
          <xsl:if test='.//UML:Stereotype[@xmi.idref != "noresolve"]'>
            <xsl:text>rHandler</xsl:text>
          </xsl:if>
        </xsl:when>
        <xsl:when test='.//UML:Stereotype[@xmi.idref = "resolvenoauto"]'>
          <xsl:text>rHandler</xsl:text>
        </xsl:when>
      </xsl:choose>
    </xsl:variable>
  </xsl:template>

  <xsl:template match='UML:Attribute' mode='resolveAttribute'>
    <xsl:variable name="attrid">
      <xsl:for-each select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="attrid"]'>
      <xsl:value-of select='UML:TaggedValue.dataValue'/>
      </xsl:for-each>
    </xsl:variable>
    <xsl:variable name="qname">
      <xsl:call-template name='idtoqname'>
        <xsl:with-param name='id'>
          <xsl:value-of select='$attrid'/>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:variable>
    <xsl:choose>
      <xsl:when test='.//UML:Stereotype/@xmi.idref = "attribute"'>
        <xsl:text>
            {
                WW8Value::Pointer_t pVal = createValue(get_</xsl:text>
              <xsl:value-of select="@name"/>
              <xsl:text>());
                rHandler.attribute(</xsl:text>
                <xsl:value-of select="$qname"/>
              <xsl:text>, *pVal);
            }</xsl:text>
      </xsl:when>
      <xsl:when test='.//UML:Stereotype/@xmi.idref = "array"'>
        <xsl:variable name="elementtype">
          <xsl:value-of select='.//UML:DataType/@xmi.idref'/>
        </xsl:variable>
        <xsl:variable name="parentclass">
          <xsl:call-template name='parenttype'>
            <xsl:with-param name='type'>
              <xsl:value-of select='$elementtype'/>
            </xsl:with-param>
          </xsl:call-template>
        </xsl:variable>
        <xsl:text>
            {
                sal_uInt32 nCount = get_</xsl:text>
              <xsl:value-of select="@name"/>
              <xsl:text>_count();              
                for (sal_uInt32 n = 0; n &lt; nCount; ++n)
                {
                    WW8Value::Pointer_t pVal = createValue(get_</xsl:text>
                    <xsl:value-of select="@name"/>
                    <xsl:text>(n));
                    rHandler.attribute(</xsl:text>
                    <xsl:value-of select="$qname"/>
                    <xsl:text>, *pVal);
                }
            }</xsl:text>
      </xsl:when>
      <xsl:when test='.//UML:Stereotype/@xmi.idref = "string"'>
        <xsl:text>
            {
                WW8StringValue aVal(get_</xsl:text>
                <xsl:value-of select='@name'/>
                <xsl:text>());
                rHandler.attribute(</xsl:text>
                <xsl:value-of select="$qname"/>
                <xsl:text>, aVal);
            }</xsl:text>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template match='UML:Attribute' mode='dumpAttribute'>
    <xsl:variable name="type">
      <xsl:value-of select='.//UML:DataType/@xmi.idref'/>
      </xsl:variable><xsl:variable name="saltype">
      <xsl:call-template name='saltype'>
        <xsl:with-param name='type'>
          <xsl:value-of select='$type'/>
        </xsl:with-param>
        <xsl:with-param name='parenttype'>
          <xsl:apply-templates select="." mode="parentclass"/>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="name" select="@name"/>
    <xsl:for-each select=".//UML:Stereotype">      
      <xsl:choose>
        <xsl:when test='@xmi.idref = "attributeremainder"'/>
        <xsl:when test='@xmi.idref = "array"'>
          <xsl:text>
          {
            sal_uInt32 nCount = get_</xsl:text>
            <xsl:value-of select='$name'/>
            <xsl:text>_count();
            
            for (sal_uInt32 n = 0; n &lt; nCount; ++n)
            {
                writerfilter::dump(o, "</xsl:text>
                <xsl:value-of select='$name'/>
                <xsl:text>", get_</xsl:text>
                <xsl:value-of select="$name"/>
                <xsl:text>(n));
            }
        }</xsl:text>
        </xsl:when>
        <xsl:when test='@xmi.idref = "string"'>
          <xsl:text>
        {
            WW8StringValue aVal(get_</xsl:text>
            <xsl:value-of select='$name'/>
            <xsl:text>());
            
            o.addItem("</xsl:text>
            <xsl:value-of select='$name'/>
            <xsl:text>" &lt;&lt; "=\"" + aVal.toString() + "\"");
        }</xsl:text>
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>
        writerfilter::dump(o, "</xsl:text>
        <xsl:value-of select='$name'/>
        <xsl:text>", get_</xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>());</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="resolveOperationAttribute">
    <xsl:variable name="opid">
      <xsl:for-each select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="opid"]'>
        <xsl:value-of select='./UML:TaggedValue.dataValue'/>
      </xsl:for-each>
    </xsl:variable>
    <xsl:text>
          {
              WW8Value::Pointer_t pVal = createValue(get_</xsl:text>
              <xsl:value-of select="@name"/>
              <xsl:text>());
              rHandler.attribute(</xsl:text>
              <xsl:call-template name='idtoqname'>
                <xsl:with-param name='id'><xsl:value-of select='$opid'/></xsl:with-param>
              </xsl:call-template>
              <xsl:text>, *pVal);
          }&#xa;</xsl:text>
  </xsl:template>

  <xsl:template name="resolveOperationArray">
    <xsl:variable name="elementtype">
      <xsl:value-of select='.//UML:Parameter.type/@xmi.idref'/>
    </xsl:variable>
    <xsl:variable name="opid">
      <xsl:for-each select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="opid"]'>
        <xsl:value-of select='./UML:TaggedValue.dataValue'/>
      </xsl:for-each>
    </xsl:variable>
    <xsl:variable name="parentclass">
      <xsl:for-each select='/XMI/XMI.content/UML:Model/UML:Namespace.ownedElement/UML:Generalization[UML:Generalization.child/UML:Class/@xmi.idref=$elementtype]'>
        <xsl:value-of select='./UML:Generalization.parent/UML:Class/@xmi.idref'/>
      </xsl:for-each>
    </xsl:variable>
    <xsl:text>
          {
              sal_uInt32 nCount = get_</xsl:text>
              <xsl:value-of select="@name"/>
              <xsl:text>_count();
              
              for (sal_uInt32 n = 0; n &lt; nCount; ++n)
              {
                  WW8Value::Pointer_t pVal = createValue(get_</xsl:text>
                  <xsl:value-of select="@name"/>
                  <xsl:text>(n));
                  rHandler.attribute(</xsl:text>
                  <xsl:call-template name='idtoqname'>
                    <xsl:with-param name='id'><xsl:value-of select='$opid'/></xsl:with-param>
                  </xsl:call-template>
                  <xsl:text>, *pVal);
             }
         }&#xa;</xsl:text>
  </xsl:template>

  <xsl:template name="resolveOperationBinary">
    <xsl:text>
          {
              WW8BinaryObjReference::Pointer_t pRef(get_</xsl:text>
              <xsl:value-of select="@name"/>
              <xsl:text>());
              WW8Sprm aSprm(pRef);
              
              rHandler.sprm(aSprm);
          }&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match='UML:Operation' mode='resolveOperation'>
    <xsl:choose>
      <xsl:when test='.//UML:Stereotype/@xmi.idref = "attribute"'>
        <xsl:call-template name="resolveOperationAttribute"/>
      </xsl:when>
      <xsl:when test='.//UML:Stereotype/@xmi.idref = "array"'>
        <xsl:call-template name="resolveOperationArray"/>
      </xsl:when>
      <xsl:when test='.//UML:Stereotype/@xmi.idref = "binary"'>
        <xsl:call-template name="resolveOperationBinary"/>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="UML:Class" mode="resolveTable">
    <xsl:param name="classname"/>
    <xsl:text>void </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>::resolve(Table &amp;)
    {
   }
    </xsl:text>
  </xsl:template>

<xsl:template match='UML:Model' mode='createsprmprops'>
</xsl:template>

<xsl:template match='UML:Model' mode='createsprmbinary'>
</xsl:template>

<!-- returns optname of UML:Class -->
<xsl:template name="optname">
  <xsl:for-each select="./UML:ModelElement.taggedValue/UML:TaggedValue">
    <xsl:if test=".//UML:TagDefinition/@xmi.idref='optname'">
      <xsl:value-of select="./UML:TaggedValue.dataValue"/>
    </xsl:if>
  </xsl:for-each>
</xsl:template>

<xsl:template name="ww8foptenamecase">
  <xsl:text>
      case </xsl:text>
      <xsl:variable name="optname">
        <xsl:call-template name="optname"/>
      </xsl:variable>
      <xsl:call-template name="idtoqname">
        <xsl:with-param name="id" select="$optname"/>
      </xsl:call-template>
      <xsl:text>:
      result = "</xsl:text>
      <xsl:value-of select="$optname"/>
      <xsl:text>";
      break;</xsl:text>
</xsl:template>

<xsl:template name="getdffoptname">
  <xsl:text>
OUString getDffOptName(sal_uInt32)
{
    OUString result;
    return result;
}</xsl:text>
</xsl:template>

<xsl:template name="ww8foptegetvalue">
  <xsl:text>
</xsl:text>
</xsl:template>

<xsl:template match='UML:Model/UML:Namespace.ownedElement' mode='ww8foptename'>
  <xsl:text>
</xsl:text>
<xsl:call-template name="getdffoptname"/>
<xsl:text>
</xsl:text>
<xsl:call-template name="ww8foptegetvalue"/>
</xsl:template>

<xsl:template match="UML:Model" mode="createdffrecord">
<xsl:text>
    </xsl:text>
</xsl:template>

<xsl:template match="UML:Attribute" mode="initmembers">
  <xsl:variable name="stereotype"><xsl:value-of select=".//UML:Stereotype/@xmi.idref"/></xsl:variable>
  <xsl:variable name="offset_"><xsl:value-of select='translate(.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="offset"]/UML:TaggedValue.dataValue, "ABCDEFX", "abcdefx")'/></xsl:variable>
  <xsl:variable name="offset">
    <xsl:if test='$offset_ != "" and not(starts-with($offset_, "0x"))'>0x</xsl:if>
    <xsl:value-of select='$offset_'/>
  </xsl:variable>
  <xsl:variable name="shift"><xsl:value-of select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="shift"]/UML:TaggedValue.dataValue'/></xsl:variable>
  <xsl:variable name="mask_"><xsl:value-of select='translate(.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="mask"]/UML:TaggedValue.dataValue, "ABCDEFX", "abcdefx")'/></xsl:variable>
  <xsl:variable name="mask">
    <xsl:if test='$mask_ != "" and not(starts-with($mask_, "0x"))'>0x</xsl:if>
    <xsl:value-of select='$mask_'/>
  </xsl:variable>
  <xsl:variable name="type"><xsl:value-of select='.//UML:DataType/@xmi.idref'/></xsl:variable><xsl:variable name="saltype">
  <xsl:call-template name='saltype'>
    <xsl:with-param name='type'>
      <xsl:value-of select='$type'/>
    </xsl:with-param>
  </xsl:call-template>
</xsl:variable>

/*
<xsl:value-of select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="comment"]/UML:TaggedValue.dataValue'/>

Name: <xsl:value-of select='@name'/>(<xsl:value-of select='$stereotype'/>)
Offset: <xsl:value-of select='$offset'/>
Mask: <xsl:value-of select='$mask'/>
Shift:  <xsl:value-of select='$shift'/>
*/

<xsl:choose>
  <xsl:when test='$stereotype = "attribute"'>
    m<xsl:value-of select='@name'/> =  (get<xsl:value-of select="$type"/>(<xsl:value-of select="$offset"/>)<xsl:if test="string-length($mask)>0"> &amp; <xsl:value-of select="$mask"/></xsl:if>)<xsl:if test="$shift>0"> &gt;&gt; <xsl:value-of select="$shift"/></xsl:if>;
  </xsl:when>
  <xsl:when test='$stereotype = "array"'>
    <xsl:text>
      {
    sal_uInt32 nCount = </xsl:text>
    <xsl:value-of select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref = "arraycount"]/UML:TaggedValue.dataValue'/>
    <xsl:text>;
    m</xsl:text>
    <xsl:value-of select='@name'/>
    <xsl:text> = new </xsl:text><xsl:value-of select='$saltype'/><xsl:text>[nCount];

    for (sal_uInt32 n = 0; n &lt; nCount; ++n)
    m</xsl:text><xsl:value-of select='@name'/><xsl:text>[n] = get</xsl:text>
    <xsl:value-of select='$type'/>
    <xsl:text>(</xsl:text>
    <xsl:value-of select='$offset'/>
    <xsl:text> + nIndex * sizeof(</xsl:text>
    <xsl:value-of select='$saltype'/>
    <xsl:text>));
    }
    </xsl:text>
  </xsl:when>
  <xsl:when test='$stereotype = "string"'>
    m<xsl:value-of select='@name'/> = getString(<xsl:value-of select='$offset'/>)
  </xsl:when>
</xsl:choose>
</xsl:template>

<xsl:template match="UML:Model" mode="isbooleandffopt">
</xsl:template>

</xsl:stylesheet>

