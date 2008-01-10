<!--
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: resourcetools.xsl,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:54:23 $
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
  <xsl:param name='id'/>
  <xsl:text>NS_</xsl:text>
  <xsl:value-of select='substring-before($id, ":")'/>
  <xsl:text>::LN_</xsl:text>
  <xsl:value-of select='substring-after($id, ":")'/>
</xsl:template>

<xsl:template name="parenttype">
  <xsl:param name='type'/>
  <xsl:for-each select='/XMI/XMI.content/UML:Model/UML:Namespace.ownedElement/UML:Generalization[UML:Generalization.child/UML:Class/@xmi.idref=$type]'>
    <xsl:value-of select='./UML:Generalization.parent/UML:Class/@xmi.idref'/>
  </xsl:for-each>
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
      <xsl:text>writerfilter::Reference &lt; </xsl:text>
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

<xsl:template name="qnametostrattrs">
  <xsl:text>
    /* Attributes */</xsl:text>
    <xsl:for-each select='.//UML:Attribute[@name!="reserved"]'>
      <xsl:for-each select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="attrid"]'>
        <xsl:choose>
          <xsl:when test='.//UML:Stereotype[@xmi.idref="noresolve"]'>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>
    mMap[</xsl:text>
    <xsl:call-template name='idtoqname'>
      <xsl:with-param name='id'><xsl:value-of select='.//UML:TaggedValue.dataValue'/></xsl:with-param>
    </xsl:call-template>
    <xsl:text>] = "</xsl:text>
    <xsl:value-of select='.//UML:TaggedValue.dataValue'/>
    <xsl:text>";</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:for-each>
    </xsl:for-each>
</xsl:template>

<xsl:template name="qnametostrops">
  <xsl:text>
    /* Operations */</xsl:text>
    <xsl:for-each select='.//UML:Operation[@name!="reserved"]'>
      <xsl:for-each select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="opid"]'>
        <xsl:choose>
          <xsl:when test='.//UML:Stereotype[@xmi.idref="noresolve"]'>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>
    mMap[</xsl:text>
    <xsl:call-template name='idtoqname'>
      <xsl:with-param name='id'><xsl:value-of select='.//UML:TaggedValue.dataValue'/></xsl:with-param>
    </xsl:call-template>
    <xsl:text>] = "</xsl:text>
    <xsl:value-of select='.//UML:TaggedValue.dataValue'/>
    <xsl:text>";</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:for-each>
    </xsl:for-each>
</xsl:template>

<xsl:template name="qnametostrclasses">
  <xsl:text>
    /* Classes */</xsl:text>
    <xsl:for-each select='.//UML:Class[@name!="reserved"]'>
      <xsl:for-each select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="classid"]'>
        <xsl:text>
    mMap[</xsl:text>
    <xsl:call-template name='idtoqname'>
      <xsl:with-param name='id'><xsl:value-of select='.//UML:TaggedValue.dataValue'/></xsl:with-param>
    </xsl:call-template>
    <xsl:text>] = "</xsl:text>
    <xsl:value-of select='.//UML:TaggedValue.dataValue'/>
    <xsl:text>";</xsl:text>
      </xsl:for-each>
    </xsl:for-each>
</xsl:template>

<xsl:template match='UML:Model' mode='qnametostr'>
  <xsl:text>
void QNameToString::init_doctok()
{</xsl:text>
<xsl:call-template name="qnametostrattrs"/>
<xsl:call-template name="qnametostrops"/>
<xsl:call-template name="qnametostrclasses"/>
<xsl:text>
}
</xsl:text>
</xsl:template>

<xsl:key name="ids" match='UML:Attribute[@name!="reserved"]//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="attrid"]|UML:Operation[@name!="reserved"]//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="opid"]' use=".//UML:TaggedValue.dataValue"/>

<xsl:template match="UML:Model" mode="qnametostrfunc">
  <xsl:text>
string qnameToString(sal_uInt32 nToken)
{
    string sResult;

    switch (nToken)
    {
       // Attributes</xsl:text>
       <xsl:for-each select='.//UML:Attribute[@name!="reserved"]//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="attrid"]'>
         <xsl:choose>
           <xsl:when test='.//UML:Stereotype[@xmi.idref="noresolve"]'>
           </xsl:when>
           <xsl:otherwise>
             <xsl:if test="generate-id(key('ids', .//UML:TaggedValue.dataValue)[1])=generate-id(.)">
               <xsl:text>
    case </xsl:text>
    <xsl:call-template name='idtoqname'>
      <xsl:with-param name='id'><xsl:value-of select='.//UML:TaggedValue.dataValue'/></xsl:with-param>
    </xsl:call-template>
    <xsl:text>:
        sResult = "</xsl:text>
        <xsl:value-of select='.//UML:TaggedValue.dataValue'/>
        <xsl:text>";
        break;</xsl:text>
             </xsl:if>
           </xsl:otherwise>
         </xsl:choose>
       </xsl:for-each>
       <xsl:text>
       // Operations</xsl:text>
       <xsl:for-each select='.//UML:Operation[@name!="reserved"]//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="opid"]'>
         <xsl:choose>
           <xsl:when test='.//UML:Stereotype[@xmi.idref="noresolve"]'>
           </xsl:when>
           <xsl:otherwise>
             <xsl:if test="generate-id(key('ids', .//UML:TaggedValue.dataValue)[1])=generate-id(.)">
               <xsl:text>
    case </xsl:text>
    <xsl:call-template name='idtoqname'>
      <xsl:with-param name='id'><xsl:value-of select='.//UML:TaggedValue.dataValue'/></xsl:with-param>
    </xsl:call-template>
    <xsl:text>:
        sResult = "</xsl:text>
        <xsl:value-of select='.//UML:TaggedValue.dataValue'/>
        <xsl:text>";
        break;</xsl:text>
             </xsl:if>
           </xsl:otherwise>
         </xsl:choose>
       </xsl:for-each>
       <xsl:text>
       // Classes:</xsl:text>
       <xsl:for-each select='.//UML:Class[@name!="reserved"]//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="classid"]'>
         <xsl:text>
    case </xsl:text>
       <xsl:call-template name='idtoqname'>
         <xsl:with-param name='id'><xsl:value-of select='.//UML:TaggedValue.dataValue'/></xsl:with-param>
       </xsl:call-template>
       <xsl:text>:
        sResult = "</xsl:text>
        <xsl:value-of select='.//UML:TaggedValue.dataValue'/>
        <xsl:text>";
        break;</xsl:text>
       </xsl:for-each>
       <xsl:text>
    default:
        ;
    }
    
    return sResult;
}</xsl:text>
</xsl:template>

<xsl:template match='UML:Model' mode='sprmcodetostr'>
<xsl:text>
SprmIdToString::SprmIdToString()
{</xsl:text>
<xsl:variable name='tmp'>map &lt; sal_uInt32, string &gt; </xsl:variable>
<xsl:for-each select='.//UML:Class[.//UML:Stereotype/@xmi.idref="ww8sprm"]'>
  <xsl:variable name="sprmconst">
    <xsl:call-template name="sprmCodeOfClass"/>
  </xsl:variable>
  <xsl:text>
    mMap[</xsl:text>    
  <xsl:value-of select="$sprmconst"/>
  <xsl:text>] = "</xsl:text>
  <xsl:value-of select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="sprmid"]//UML:TaggedValue.dataValue'/>
  <xsl:text>";</xsl:text>
</xsl:for-each>
<xsl:text>
}
</xsl:text>
</xsl:template>

<xsl:key name="classes-with-kind" match="UML:TagDefinition[@xmi.idref='kind']"
         use="ancestor::UML:TaggedValue/UML:TaggedValue.dataValue"/>

<xsl:template name="sprmCodeOfClass">
  <xsl:variable name="tmp">
    <xsl:text>sprm:</xsl:text>
    <xsl:value-of select="substring-after(@name, 'sprm')"/>
  </xsl:variable>
  <xsl:call-template name="idtoqname">
    <xsl:with-param name="id" select="$tmp"/>
  </xsl:call-template>
</xsl:template>

<xsl:template name="sprmkindcase">
  <xsl:param name="kind"/>
  <xsl:for-each select="key('classes-with-kind', $kind)/ancestor::UML:Class">
    <xsl:text>
    case </xsl:text>
    <xsl:call-template name="sprmCodeOfClass"/>
    <xsl:text>:</xsl:text>
  </xsl:for-each>
</xsl:template>

<xsl:template match="UML:Model" mode='sprmkind'>
<xsl:text>
Sprm::Kind SprmKind(sal_uInt32 sprmCode)
{
    Sprm::Kind nResult = Sprm::UNKNOWN;

    switch(sprmCode)
    {</xsl:text>
    <xsl:call-template name="sprmkindcase">
      <xsl:with-param name="kind">paragraph</xsl:with-param>
    </xsl:call-template>
    <xsl:text>
      nResult = Sprm::PARAGRAPH;
      break;</xsl:text>
    <xsl:call-template name="sprmkindcase">
      <xsl:with-param name="kind">character</xsl:with-param>
    </xsl:call-template>
    <xsl:text>
      nResult = Sprm::CHARACTER;
      break;</xsl:text>
    <xsl:call-template name="sprmkindcase">
      <xsl:with-param name="kind">table</xsl:with-param>
    </xsl:call-template>
    <xsl:text>
      nResult = Sprm::TABLE;
      break;</xsl:text>
    <xsl:text>
    default:
      break;
    }

    return nResult;
}
</xsl:text>
</xsl:template>

<xsl:template match="UML:Model" mode='sprmids'>
  <xsl:text>
namespace NS_sprm { </xsl:text>
  <xsl:for-each select=".//UML:Class[.//UML:Stereotype/@xmi.idref='ww8sprm']">
    <xsl:variable name="sprmcode">
      <xsl:value-of select=".//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref = 'sprmcode']/UML:TaggedValue.dataValue"/>
    </xsl:variable>
    <xsl:variable name="sprmcodelower">
      <xsl:value-of select="translate($sprmcode, 'ABCDEF', 'abcdef')"/>
    </xsl:variable>
    <xsl:variable name="sprmidname">
      <xsl:text>LN_</xsl:text>
      <xsl:value-of select="substring-after(@name, 'sprm')"/>
    </xsl:variable>
    <xsl:text>
    const sal_uInt16 </xsl:text>
    <xsl:value-of select="$sprmidname"/>
    <xsl:text> = </xsl:text>
    <xsl:value-of select="$sprmcodelower"/>
    <xsl:text>;</xsl:text>
  </xsl:for-each>
  <xsl:text>
}
</xsl:text>
</xsl:template>

<xsl:template match="UML:Model" mode='sprmidsed'>
  <xsl:text>#!/bin/sh
  cat $1 \&#xa;</xsl:text>
  <xsl:for-each select=".//UML:Class[.//UML:Stereotype/@xmi.idref='ww8sprm']">
    <xsl:variable name="sprmcode">
      <xsl:value-of select=".//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref = 'sprmcode']/UML:TaggedValue.dataValue"/>
    </xsl:variable>
    <xsl:variable name="sprmcodelower">
      <xsl:value-of select="translate($sprmcode, 'ABCDEF', 'abcdef')"/>
    </xsl:variable>
    <xsl:variable name="sprmidname">
      <xsl:text>NS_sprm::LN_</xsl:text>
      <xsl:value-of select="substring-after(@name, 'sprm')"/>
    </xsl:variable>
    <xsl:text>| sed "s/</xsl:text>
<xsl:value-of select="$sprmcode"/>
<xsl:text>/</xsl:text>
<xsl:value-of select="$sprmidname"/>
<xsl:text>/g" \&#xa;</xsl:text>
    <xsl:text>| sed "s/</xsl:text>
<xsl:value-of select="$sprmcodelower"/>
<xsl:text>/</xsl:text>
<xsl:value-of select="$sprmidname"/>
<xsl:text>/g" \&#xa;</xsl:text>
  </xsl:for-each>
  <xsl:text> | cat&#xa;</xsl:text>
</xsl:template>

<xsl:template match="UML:Model" mode="sprmidstoxml">
  <xsl:text>
void sprmidsToXML(::std::ostream &amp; out)
{
  </xsl:text>
  <xsl:for-each select=".//UML:Class[.//UML:Stereotype/@xmi.idref='ww8sprm']">
    <xsl:variable name="sprmcode">
      <xsl:value-of select=".//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref = 'sprmcode']/UML:TaggedValue.dataValue"/>
    </xsl:variable>
    <xsl:variable name="sprmcodelower">
      <xsl:value-of select="translate($sprmcode, 'ABCDEF', 'abcdef')"/>
    </xsl:variable>
    <xsl:variable name="sprmidname">
      <xsl:text>sprm:</xsl:text>
      <xsl:value-of select="substring-after(@name, 'sprm')"/>
    </xsl:variable>
    <xsl:text>
    out &lt;&lt; "&lt;theid name=\"</xsl:text>
    <xsl:value-of select="$sprmidname"/>
    <xsl:text>\"&gt;</xsl:text>
    <xsl:value-of select="$sprmcodelower"/>
    <xsl:text>&lt;/theid&gt;" &lt;&lt; endl;</xsl:text>
  </xsl:for-each>
  <xsl:text>
}
  </xsl:text>
</xsl:template>

<xsl:template match="UML:Model" mode='sprmreplace'>
  <xsl:for-each select=".//UML:Class[.//UML:Stereotype/@xmi.idref='ww8sprm']">
    <xsl:variable name="pattern">
      <xsl:value-of select=".//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref = 'sprmcode']/UML:TaggedValue.dataValue"/>
    </xsl:variable>
    <xsl:variable name="lowerpattern">
      <xsl:value-of select="translate($pattern, 'ABCDEF', 'abcdef')"/>
    </xsl:variable>
    <xsl:variable name="upperpattern">
      <xsl:value-of select="translate($pattern, 'abcdef', 'ABCDEF')"/>
    </xsl:variable>
    <xsl:variable name="tmp">
      <xsl:text>sprm:</xsl:text>
      <xsl:value-of select="substring-after(@name, 'sprm')"/>
    </xsl:variable>
    <xsl:variable name="constname">
      <xsl:call-template name="idtoqname">
        <xsl:with-param name="id" select="$tmp"/>
      </xsl:call-template>
    </xsl:variable>
    <xsl:text>
sed "s/</xsl:text>
    <xsl:value-of select="$lowerpattern"/>
    <xsl:text>/</xsl:text>
    <xsl:value-of select="$constname"/>
    <xsl:text>/g" &lt; $1 > $1.out &amp;&amp; mv $1.out $1 </xsl:text>
    <xsl:text>
sed "s/</xsl:text>
    <xsl:value-of select="$upperpattern"/>
    <xsl:text>/</xsl:text>
    <xsl:value-of select="$constname"/>
    <xsl:text>/g" &lt; $1 > $1.out &amp;&amp; mv $1.out $1 </xsl:text>
  </xsl:for-each>
  <xsl:text>&#xa;</xsl:text>
</xsl:template>

<!-- Key all attributes with the same name and same value -->
<xsl:key name="same-valued-tagged-data"
         match="UML:TaggedValue.dataValue" use="." />

<xsl:template name="analyzerdoctokidsattrs">
  <xsl:text>
  /* Attributes */</xsl:text>
  <xsl:for-each select='.//UML:Attribute[@name!="reserved"]'>
    <xsl:if test='count(.//UML:Stereotype[@xmi.idref="noqname"]) = 0'>
      <xsl:for-each select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="attrid"]'>
        <xsl:choose>
          <xsl:when test='generate-id(UML:TaggedValue.dataValue) != generate-id(key("same-valued-tagged-data", UML:TaggedValue.dataValue)[1])'/>
          <xsl:otherwise>
            <xsl:text>
    out &lt;&lt; "&lt;theid name=\"</xsl:text>
    <xsl:value-of select=".//UML:TaggedValue.dataValue"/>
    <xsl:text>\"&gt;</xsl:text>
    <xsl:value-of select='10000 + position()'/>
    <xsl:text>&lt;/theid&gt;" &lt;&lt; endl;</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:for-each>
    </xsl:if>
  </xsl:for-each>
</xsl:template>

<xsl:template name="analyzerdoctokidsops">
  <xsl:text>
  /* Operations */</xsl:text>
  <xsl:for-each select='.//UML:Operation[@name!="reserved"]'>
    <xsl:if test='count(.//UML:Stereotype[@xmi.idref="noqname"]) = 0'>
      <xsl:for-each select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="opid"]'>
        <xsl:choose>
          <xsl:when test='generate-id(UML:TaggedValue.dataValue) != generate-id(key("same-valued-tagged-data", UML:TaggedValue.dataValue)[1])'/>
          <xsl:otherwise>
            <xsl:text>
    out &lt;&lt; "&lt;theid name=\"</xsl:text>
    <xsl:value-of select=".//UML:TaggedValue.dataValue"/>
    <xsl:text>\"&gt;</xsl:text>
    <xsl:value-of select='20000 + position()'/>
    <xsl:text>&lt;/theid&gt;" &lt;&lt; endl;</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:for-each>
    </xsl:if>
  </xsl:for-each>
</xsl:template>

<xsl:template name="analyzerdoctokidsclasses">
  <xsl:text>
  /* clases */</xsl:text>
  <xsl:for-each select='.//UML:Class[@name!="reserved"]'>
    <xsl:for-each select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="classid"]'>
      <xsl:choose>
        <xsl:when test='.//UML:Stereotype[@xmi.idref="noqname"]'/>
        <xsl:when test='generate-id(UML:TaggedValue.dataValue) != generate-id(key("same-valued-tagged-data", UML:TaggedValue.dataValue)[1])'/>
        <xsl:otherwise>
          <xsl:text>
    out &lt;&lt; "&lt;theid name=\"</xsl:text>
    <xsl:value-of select=".//UML:TaggedValue.dataValue"/>
    <xsl:text>\"&gt;</xsl:text>
    <xsl:value-of select='30000 + position()'/>
    <xsl:text>&lt;/theid&gt;" &lt;&lt; endl;</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
  </xsl:for-each>
  </xsl:for-each>
</xsl:template>

<xsl:template match="UML:Model" mode="analyzerdoctokids">
  <xsl:text>
void doctokidsToXML(::std::ostream &amp; out)
{</xsl:text>
<xsl:call-template name="analyzerdoctokidsattrs"/>
<xsl:call-template name="analyzerdoctokidsops"/>
<xsl:call-template name="analyzerdoctokidsclasses"/>
<xsl:text>
}
  </xsl:text>
</xsl:template>

  <xsl:template name="licenseheader">
    <xsl:text>
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: resourcetools.xsl,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:54:23 $
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
/*      

  THIS FILE IS GENERATED AUTOMATICALLY! DO NOT EDIT!

*/
&#xa;</xsl:text>
  </xsl:template>
</xsl:stylesheet>
