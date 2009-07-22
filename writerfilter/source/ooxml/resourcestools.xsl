<!--
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: resourcestools.xsl,v $
 *
 * $Revision: 1.49 $
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

 <xsl:key name="same-attribute"
           match="rng:attribute" use="@name" />

  <xsl:key name="same-element"
           match="rng:element" use="@name" />

  <xsl:key name="defines-with-name"
           match="rng:define" use="@name" />

  <xsl:key name="same-element-enum"
           match="rng:element" use="@enumname"/>

  <xsl:key name="same-attribute-enum"
           match="rng:attribute" use="@enumname"/>

  <xsl:key name="same-element-or-attribute-enum"
           match="rng:attribute|rng:element" use="@enumname"/>

  <xsl:key name="context-resource"
           match="resource" use="@name"/>

  <xsl:key name="defines-with-application"
           match="rng:define" use="ancestor::rng:grammar/@application"/>

  <xsl:key name="namespace-aliases" match="//namespace-alias" use="@name"/>

  <xsl:template name="licenseheader">
    <xsl:text>
/*      

  THIS FILE IS GENERATED AUTOMATICALLY! DO NOT EDIT!

*/
&#xa;</xsl:text>
  </xsl:template>

  <xsl:template name="hasrefs">
    <xsl:choose>
      <xsl:when test=".//rng:ref[not(ancestor::rng:element or ancestor::rng:attribute)]">
        <xsl:text>1</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>0</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- 
       Returns <define> for the current <ref>. 

       The current node must be a <ref>!
  -->
  <xsl:template name="defineforref">
    <xsl:variable name="mygrammarid" select="generate-id(ancestor::rng:grammar)"/>
    <xsl:value-of select="key('defines-with-name', @name)[generate-id(ancestor::rng:grammar) = $mygrammarid]"/>
  </xsl:template>

  <!-- 
       Create name with prefix.

       The result is <prefix>:<name>. 

       Exception: If <name> contains a ':' the prefix is ignored.

       @param prefix    the prefix
       @param name      the name
  -->
  <xsl:template name="prefixname">
    <xsl:param name="prefix"/>
    <xsl:param name="name"/>
    <xsl:choose>
      <xsl:when test="contains($name, ':')">
        <xsl:value-of select="$name"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$prefix"/>
        <xsl:text>:</xsl:text>
        <xsl:value-of select="$name"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="fastnamespace">
    <xsl:if test="string-length(@prefix) > 0">
      <xsl:text>NS_</xsl:text>
      <xsl:value-of select="@prefix"/>
    </xsl:if>
  </xsl:template>

  <xsl:template name="fastlocalname">
    <xsl:text>OOXML_</xsl:text>
    <xsl:value-of select="@localname"/>
  </xsl:template>

  <!--
      Create entries in enum definition for fast tokens.

      For each occurrence of rng:element an entry 

               OOXML_<name> 

      is generated, but only if the element is the first named <name>.
  -->
  <xsl:template name="enumfasttokens">
    <xsl:param name="prefix"/>
    <xsl:for-each select=".//rng:element|.//rng:attribute">
      <xsl:if test="generate-id(.) = generate-id(key('same-token-name', @localname)[1])">
        <xsl:call-template name="fastlocalname"/>
        <xsl:text>, &#xa; </xsl:text>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="fasttokens">
    <xsl:text>
typedef sal_Int32 Token_t;
    </xsl:text>
    <xsl:for-each select="/model/fasttoken">
      <xsl:text>
const Token_t OOXML_</xsl:text>
<xsl:value-of select="translate(., '-', '_')"/>
<xsl:text> = </xsl:text>
<xsl:value-of select="position() - 1"/>
<xsl:text>;</xsl:text>
    </xsl:for-each>
    <xsl:text>
const Token_t OOXML_FAST_TOKENS_END =</xsl:text>
<xsl:value-of select="count(/model/fasttoken)"/>
<xsl:text>;&#xa;</xsl:text>
  </xsl:template>

  <!--
      Returns prefix for the rng:grammar of the current node.
  -->
  <xsl:template name="prefixforgrammar">
    <xsl:variable name="ns" select="ancestor::rng:grammar/@ns"/>
    <xsl:variable name="nsalias"><xsl:value-of select="key('namespace-aliases', $ns)/@alias"/></xsl:variable>
    <!--<xsl:variable name="nsalias">test</xsl:variable>-->
    <xsl:choose>
      <xsl:when test="string-length($nsalias) > 0">
        <xsl:value-of select="$nsalias"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="translate(substring-after($ns, 'http://'), '/.', '__')"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!--
      Returns prefix for the rng:grammar of the matching nodes.
  -->
  <xsl:template match="*" mode="grammar-prefix">
    <xsl:call-template name="prefixforgrammar"/>
  </xsl:template>

  <!--
      Generates name for fast context class for this <define>
  -->
  <xsl:template name="fastcontextname">
    <xsl:variable name="do">
      <xsl:call-template name="classfordefine"/>
    </xsl:variable>
    <xsl:if test="$do = '1'">
      <xsl:variable name="definename">
        <xsl:call-template name="searchdefinenamespace">
          <xsl:with-param name="name" select="@name"/>
        </xsl:call-template>
      </xsl:variable>
      <xsl:for-each select="/model/namespace[@name=substring-before($definename, ':')]">
        <xsl:text>OOXMLFastContextHandler_</xsl:text>
        <xsl:value-of select="@namespacealias"/>
        <xsl:text>_</xsl:text>
        <xsl:value-of select="substring-after($definename, ':')"/>
      </xsl:for-each>
    </xsl:if>
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

  <!--
      Returns a value name.

      The resulting value name is
          
          OOXMLValue_$prefix_$name

      @prefix     the prefix
      @name       the name
  -->
  <xsl:template name="valuename">
    <xsl:param name="prefix"/>
    <xsl:param name="name"/>
    <xsl:text>OOXMLValue_</xsl:text>
    <xsl:value-of select="$prefix"/>
    <xsl:text>_</xsl:text>
    <xsl:value-of select="$name"/>
  </xsl:template>

  <!--
      Returns the value name for the current <define>.
  -->
  <xsl:template name="valuenamefordefine">
    <xsl:call-template name="valuename">
      <xsl:with-param name="prefix" select="key('namespace-aliases', ancestor::namespace/rng:grammar/@ns)/@alias"/>
      <xsl:with-param name="name" select="translate(@name, '-', '_')"/>
    </xsl:call-template>
  </xsl:template>

  <!--
      Returns the name of the parent class of the class for the
      current <define>.

      Precondition: The class for the current <define> is derived
      indirectly from OOXMLValue.

  -->
  <xsl:template name="valueparent">
    <xsl:variable name="resource">
      <xsl:call-template name="contextresource"/>
    </xsl:variable>
    <xsl:text>OOXML</xsl:text>
    <xsl:value-of select="$resource"/>
    <xsl:text>Value</xsl:text>
  </xsl:template>

  <!--
      Generates declaration for a value class.

      Precondition: <resource> for current <define> indicates that the
      class is derived directly or indirectly from OOXMLValue.
  -->
  <xsl:template name="valuedecl">
    <xsl:variable name="classname">
     <xsl:call-template name="valuenamefordefine"/>
    </xsl:variable>
    <xsl:variable name="resource">
      <xsl:call-template name="contextresource"/>
    </xsl:variable>
    <xsl:text>
class </xsl:text>
    <xsl:value-of select="$classname"/>
     <xsl:text> : public </xsl:text>
     <xsl:call-template name="valueparent"/>
     <xsl:text>
{
public:
     </xsl:text>
     <xsl:choose>
       <xsl:when test="$resource='List'">
         <xsl:value-of select="$classname"/>
         <xsl:text>(</xsl:text>
         <xsl:text>const rtl::OUString &amp; rValue</xsl:text>
         <xsl:text>);</xsl:text>
       </xsl:when>
       <xsl:otherwise>
         <xsl:value-of select="$classname"/>
         <xsl:text>(const rtl::OUString &amp; rValue) : </xsl:text>
         <xsl:call-template name="valueparent"/>
         <xsl:text>(rValue) {}</xsl:text>
       </xsl:otherwise>
     </xsl:choose>
     <xsl:text>
     virtual ~</xsl:text>
     <xsl:value-of select="$classname"/>
     <xsl:text>() {}
};
</xsl:text>
  </xsl:template>

  <!--
      Checks if a class derived from OOXMLContex shall be defined for
      the current <define>.

      @retval 1    the class shall be defined.
      @retval 0    otherwise
  -->
  <xsl:template name="classfordefine">
    <!--
    <xsl:variable name="name" select="@name"/>
    <xsl:choose>
      <xsl:when test="(starts-with(@name, 'CT_') or starts-with(@name, 'EG_') or starts-with(@name, 'AG_'))">1</xsl:when>
      <xsl:when test="ancestor::namespace//start[@name=$name]">1</xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
    -->
    <xsl:value-of select="@classfordefine"/>
  </xsl:template>

  <!--
      Checks if a class derived from OOXMLValue shall be defined for
      the current <define>.

      @retval 1    the class shall be defined
      @retval 0    otherwise
  -->
  <xsl:template name="valuefordefine">
    <xsl:choose>
      <xsl:when test="starts-with(@name, 'ST_')">1</xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="valuedecls">
    <xsl:for-each select=".//rng:grammar/rng:define">
      <xsl:variable name="dovalue">
        <xsl:call-template name="valuefordefine"/>
      </xsl:variable>
      <xsl:if test="$dovalue = '1'">
        <xsl:call-template name="valuedecl"/>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>

  <!-- 
       Generate switch body for createFastChildContext
  -->
  <xsl:template name="switchbodycreatechildcontext">
    <xsl:for-each select=".//rng:element[@name]">
        <xsl:call-template name="caselabelfasttoken"/>
        <xsl:variable name="createstatement">
          <xsl:call-template name="fastelementcreatestatement"/>
        </xsl:variable>
        <xsl:if test="string-length($createstatement) > 0">
          <xsl:text>
          xContextHandler.set(</xsl:text>
          <xsl:value-of select="$createstatement"/>
          <xsl:text>);
          </xsl:text>
        </xsl:if>
        <xsl:text>
          break;
        </xsl:text>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="fastelementcreatefromrefstatement">
    <xsl:variable name="definename">
      <xsl:call-template name="searchdefinenamespace">
        <xsl:with-param name="name" select="@name"/>
      </xsl:call-template>
    </xsl:variable>
    <xsl:for-each select="/model/namespace[@name=substring-before($definename, ':')]">
      <xsl:for-each select="./rng:grammar/rng:define[@name=substring-after($definename, ':')]">
        <xsl:variable name="do">
          <xsl:call-template name="classfordefine"/>
        </xsl:variable>
        <xsl:if test="$do = '1'">
          <xsl:text>OOXMLFastHelper &lt;</xsl:text>
          <xsl:call-template name="fastcontextname"/>
          <xsl:text>&gt;::createAndSetParentRef(this, Element, Attribs)</xsl:text>
        </xsl:if>
      </xsl:for-each>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="switchbodycreatechildcontextrefs">
    <xsl:for-each select=".//rng:ref[not (ancestor::rng:element or ancestor::rng:attribute)]">
      <xsl:variable name="createstatement">
        <xsl:call-template name="fastelementcreatefromrefstatement"/>
      </xsl:variable>
      <xsl:if test="string-length($createstatement) > 0">
        <xsl:text>
      if (! xContextHandler.is() || dynamic_cast&lt;OOXMLFastContextHandler *&gt;(xContextHandler.get())->isFallback())
      {
          xContextHandler.set(</xsl:text>
          <xsl:value-of select="$createstatement"/>
          <xsl:text>);                    
      }
          </xsl:text>
      </xsl:if>
    </xsl:for-each>
    <xsl:for-each select=".//rng:element[rng:anyName]">
      <xsl:variable name="createstatement">
        <xsl:call-template name="fastelementcreatestatement"/>
      </xsl:variable>
      <xsl:if test="string-length($createstatement) > 0">
        <xsl:text>
      if (! xContextHandler.is() || dynamic_cast&lt;OOXMLFastContextHandler *&gt;(xContextHandler.get())->isFallback())
      {
          xContextHandler.set(</xsl:text>
          <xsl:value-of select="$createstatement"/>
          <xsl:text>);                    
      } 
          </xsl:text>        
      </xsl:if>
    </xsl:for-each>
  </xsl:template>

  <!--
      Generates definition of method createFastChildContext for current <define>
  -->
  <xsl:template name="createfastchildcontext">
    <xsl:variable name="resource">
      <xsl:call-template name="contextresource"/>
    </xsl:variable>
    <xsl:if test="not($resource='Shape')">
      <xsl:variable name="switchbody">
        <xsl:call-template name="switchbodycreatechildcontext"/>
      </xsl:variable>
      <xsl:variable name="switchbodyrefs">
        <xsl:call-template name="switchbodycreatechildcontextrefs"/>
      </xsl:variable>
      <xsl:if test="string-length($switchbody) > 0 or string-length($switchbodyrefs) > 0">
        <xsl:text>
uno::Reference &lt; xml::sax::XFastContextHandler &gt;
</xsl:text>
<xsl:call-template name="fastcontextname"/>
<xsl:text>::lcl_createFastChildContext
(::sal_Int32 Element, 
 const uno::Reference &lt; xml::sax::XFastAttributeList &gt; &amp;</xsl:text>
<xsl:if test="contains($switchbody, 'Attribs') or contains($switchbodyrefs, 'Attribs')">
  <xsl:text> Attribs</xsl:text>
</xsl:if>
<xsl:text>) 
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    uno::Reference &lt; xml::sax::XFastContextHandler &gt; xContextHandler;
</xsl:text>
<xsl:if test="string-length($switchbody) > 0">
  <xsl:text>
    switch (Element)
    {
    </xsl:text>
    <xsl:value-of select="$switchbody"/>
    <xsl:text>
    default:
       break;
    }
  </xsl:text>
</xsl:if>
<xsl:if test="string-length($switchbodyrefs) > 0">
  <xsl:value-of select="$switchbodyrefs"/>
</xsl:if>
<xsl:text>

    return xContextHandler;
}
</xsl:text>
      </xsl:if>
    </xsl:if>
  </xsl:template>

  <xsl:template name="idforattr">
    <xsl:variable name="name" select="@name"/>
    <xsl:for-each select="ancestor::rng:define">
      <xsl:variable name="definename" select="@name"/>
      <xsl:for-each select="ancestor::namespace/resource[@name=$definename]">
        <xsl:for-each select="./attribute[@name=$name]">
          <xsl:call-template name="idtoqname">
            <xsl:with-param name="id" select="@tokenid"/>
          </xsl:call-template>
        </xsl:for-each>
      </xsl:for-each>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="idforelement">
    <xsl:variable name="name" select="@name"/>
    <xsl:for-each select="ancestor::rng:define">
      <xsl:variable name="definename" select="@name"/>
      <xsl:for-each select="ancestor::namespace/resource[@name=$definename]">
        <xsl:for-each select="./element[@name=$name]">
          <xsl:call-template name="idtoqname">
            <xsl:with-param name="id" select="@tokenid"/>
          </xsl:call-template>
        </xsl:for-each>
      </xsl:for-each>
    </xsl:for-each>
  </xsl:template>

  <!--
      Processes token id given in <resource> elements.

      The result is the identifier for the tokenid.      
  -->
  <xsl:template name="processtokenid">
    <xsl:choose>
      <xsl:when test="contains(@tokenid, ':')">
        <xsl:call-template name="idtoqname">
          <xsl:with-param name="id" select="@tokenid"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="@tokenid"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="getidcaseimpl">
    <xsl:for-each select="attribute|element">
      <xsl:text>
      </xsl:text>
      <xsl:call-template name="caselabelfasttoken"/>
      <xsl:text>
        nResult = </xsl:text>
         <xsl:call-template name="processtokenid"/>
         <xsl:text>;
        break;</xsl:text>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="attributeproptype">
    <xsl:variable name="mynsid" select="generate-id(ancestor::namespace)"/>
    <xsl:variable name="name" select="@name"/>
    <xsl:variable name="resource" select="key('context-resource', @name)[generate-id(ancestor::namespace)=$mynsid]"/>
    <xsl:choose>
      <xsl:when test="$resource/element[@name=$name]">
        <xsl:text>OOXMLPropertyImpl::SPRM</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>OOXMLPropertyImpl::ATTRIBUTE</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!--
      Check if a default is defined for a define.

      Returns if the <resource> for the current <define> has a
      <default> child.

      @retval 0     there is no default
      @retval 1     there is a default
  -->
  <xsl:template name="contexthasdefault">
    <xsl:variable name="name" select="@name"/>
    <xsl:choose>
      <xsl:when test="ancestor::namespace/resource[@name=$name]//default">1</xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!--
      Chooses the action for the current <action> element.
  -->
  <xsl:template name="chooseaction">
    <xsl:if test="@tokenid">
      <xsl:text>
        if (sal::static_int_cast&lt;Id&gt;(getId()) == </xsl:text>
      <xsl:call-template name="idtoqname">
        <xsl:with-param name="id" select="@tokenid"/>
      </xsl:call-template>
      <xsl:text>)
      {</xsl:text>
    </xsl:if>
    <xsl:for-each select="./cond">
      <xsl:text>
    {
        OOXMLPropertySetEntryToInteger aHandler(</xsl:text>
        <xsl:call-template name="idtoqname">
          <xsl:with-param name="id" select="@tokenid"/>
        </xsl:call-template>
        <xsl:text>);
        getPropertySetAttrs()->resolve(aHandler);
        
        if (sal::static_int_cast&lt;Id&gt;(aHandler.getValue()) == </xsl:text>
        <xsl:call-template name="idtoqname">
          <xsl:with-param name="id" select="@value"/>
        </xsl:call-template>
        <xsl:text>)
        {</xsl:text>
    </xsl:for-each>
    <xsl:choose>
      <xsl:when test="@action='sendTableDepth'">
        <xsl:text>
    sendTableDepth();</xsl:text>
      </xsl:when>
      <xsl:when test="@action='startCell'">
        <xsl:text>
    startCell();</xsl:text>
      </xsl:when>
      <xsl:when test="@action='startParagraphGroup'">
        <xsl:text>
    startParagraphGroup();</xsl:text>
      </xsl:when>
      <xsl:when test="@action='startCharacterGroup'">
        <xsl:text>
    startCharacterGroup();</xsl:text>
      </xsl:when>
      <xsl:when test="@action='startSectionGroup'">
        <xsl:text>
    startSectionGroup();</xsl:text>
      </xsl:when>
      <xsl:when test="@action='fieldstart'">
        <xsl:text>
    startField();</xsl:text>
      </xsl:when>
      <xsl:when test="@action='fieldsep'">
        <xsl:text>
    fieldSeparator();</xsl:text>
      </xsl:when>
      <xsl:when test="@action='fieldend'">
        <xsl:text>
    endField();</xsl:text>
      </xsl:when>
      <xsl:when test="@action='ftnednref'">
        <xsl:text>
    ftnednref();</xsl:text>
      </xsl:when>
      <xsl:when test="@action='ftnednsep'">
        <xsl:text>
    ftnednsep();</xsl:text>
      </xsl:when>
      <xsl:when test="@action='ftnedncont'">
        <xsl:text>
    ftnedncont();</xsl:text>
      </xsl:when>
      <xsl:when test="@action='pgNum'">
        <xsl:text>
    pgNum();</xsl:text>
      </xsl:when>
      <xsl:when test="@action='tab'">
        <xsl:text>
    tab();</xsl:text>
      </xsl:when>
      <xsl:when test="@action='cr'">
        <xsl:text>
    cr();</xsl:text>
      </xsl:when>
      <xsl:when test="@action='noBreakHyphen'">
        <xsl:text>
    noBreakHyphen();</xsl:text>
      </xsl:when>
      <xsl:when test="@action='softHyphen'">
        <xsl:text>
    softHyphen();</xsl:text>
      </xsl:when>
      <xsl:when test="@action='endOfParagraph'">
        <xsl:text>
    endOfParagraph();</xsl:text>
      </xsl:when>
      <xsl:when test="@action='handleLastParagraphInSection'">
        <xsl:text>
    handleLastParagraphInSection();</xsl:text>
      </xsl:when>
      <xsl:when test="@action='setLastParagraphInSection'">
        <xsl:text>
    setLastParagraphInSection();</xsl:text>
      </xsl:when>
      <xsl:when test="@action='endCell'">
        <xsl:text>
    endCell();</xsl:text>
      </xsl:when>
      <xsl:when test="@action='endParagraphGroup'">
        <xsl:text>
    endParagraphGroup();</xsl:text>
      </xsl:when>
      <xsl:when test="@action='endCharacterGroup'">
        <xsl:text>
    endCharacterGroup();</xsl:text>
      </xsl:when>
      <xsl:when test="@action='endSectionGroup'">
        <xsl:text>
    endSectionGroup();</xsl:text>
      </xsl:when>
        <xsl:when test="@action='handleXNotes'">
    handleXNotes();</xsl:when>
        <xsl:when test="@action='handleHdrFtr'">
    handleHdrFtr();</xsl:when>
        <xsl:when test="@action='handleComment'">
    handleComment();</xsl:when>
        <xsl:when test="@action='handlePicture'">
    handlePicture();</xsl:when>
        <xsl:when test="@action='handleHyperlink'">
    handleHyperlink();</xsl:when>
        <xsl:when test="@action='handleBreak'">
    handleBreak();</xsl:when>
        <xsl:when test="@action='handleOLE'">
    handleOLE();</xsl:when>
        <xsl:when test="@action='printproperty'">
          <xsl:text>
    sendProperty(</xsl:text>
        <xsl:call-template name="idtoqname">
          <xsl:with-param name="id" select="@sendtokenid"/>
        </xsl:call-template>
        <xsl:text>);</xsl:text>
        </xsl:when>
        <xsl:when test="@action='propagateCharacterProperties'">
    propagateCharacterProperties();
        </xsl:when>
        <xsl:when test="@action='propagateCharacterPropertiesAsSet'">
          <xsl:text>
    propagateCharacterPropertiesAsSet(</xsl:text>
    <xsl:call-template name="idtoqname">
      <xsl:with-param name="id" select="@sendtokenid"/>
    </xsl:call-template>
    <xsl:text>);</xsl:text>
        </xsl:when>
        <xsl:when test="@action='propagateTableProperties'">
    propagateTableProperties();
        </xsl:when>
        <xsl:when test="@action='sendPropertiesWithId'">
          <xsl:text>
    sendPropertiesWithId(</xsl:text>
          <xsl:call-template name="idtoqname">
            <xsl:with-param name="id" select="@sendtokenid"/>
          </xsl:call-template>
          <xsl:text>);</xsl:text>
        </xsl:when>
        <xsl:when test="@action='clearProps'">
    clearProps();
        </xsl:when>
        <xsl:when test="@action='text'">
    text(sText);
        </xsl:when>
        <xsl:when test="@action='setHandle'">
    setHandle();
        </xsl:when>

        <xsl:when test="@action='footnoteSeparator'">
    footnoteSeparator();
        </xsl:when>
        <xsl:when test="@action='footnoteCont'">
    footnoteCont();
        </xsl:when>
        <xsl:when test="@action='endnoteSeparator'">
    endnoteSeparator();
        </xsl:when> 
        <xsl:when test="@action='endnoteCont'">
    endnoteCont();
        </xsl:when>
        <xsl:when test="@action='newProperty'">
          <xsl:text>
    OOXMLFastHelper&lt;OOXMLIntegerValue&gt;::newProperty(this, </xsl:text>
    <xsl:call-template name="idtoqname">
      <xsl:with-param name="id" select="@tokenid"/>
    </xsl:call-template>
    <xsl:text>, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("</xsl:text>
    <xsl:value-of select="@value"/>
    <xsl:text>")));</xsl:text>
        </xsl:when>
        <xsl:when test="@action='mark'">
          <xsl:text>
    OOXMLFastHelper&lt;OOXMLIntegerValue&gt;::mark(this, </xsl:text>
    <xsl:call-template name="idtoqname">
      <xsl:with-param name="id" select="@sendtokenid"/>
    </xsl:call-template>
    <xsl:text>, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("</xsl:text>
    <xsl:value-of select="@value"/>
    <xsl:text>")));</xsl:text>
        </xsl:when>
        <xsl:when test="@action='tokenproperty'">
          <xsl:text>
    OOXMLFastHelper&lt;OOXMLIntegerValue&gt;::newProperty(this, </xsl:text>
    <xsl:call-template name="idtoqname">
      <xsl:with-param name="id">ooxml:token</xsl:with-param>
    </xsl:call-template>
    <xsl:text>, getToken());</xsl:text>
        </xsl:when>
    </xsl:choose>
    <xsl:for-each select="./cond">
      <xsl:text>
        }
    }</xsl:text>
    </xsl:for-each>
    <xsl:if test="@tokenid">
      <xsl:text>
    }</xsl:text>
    </xsl:if>
  </xsl:template>

  <!-- 
      Generates the definitions of the methods of the fast context class
      for the current <define>.
  -->
  <xsl:template name="fastcontextimpls">
    <xsl:param name="ns"/>
    <xsl:for-each select=".//namespace[@name=$ns]">
    <xsl:text>
</xsl:text>
      <xsl:for-each select="./rng:grammar/rng:define">
        <xsl:variable name="do">
          <xsl:call-template name="classfordefine"/>
        </xsl:variable>
        <xsl:if test="$do = '1'">
          <xsl:variable name="classname">
            <xsl:call-template name="fastcontextname"/>
          </xsl:variable>
          <xsl:text>
/* 
    class: </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>
*/
</xsl:text>
<xsl:call-template name="fastcontextconstructor"/>
<xsl:call-template name="fastcontextdestructor"/>
<xsl:call-template name="faststartaction"/>
<xsl:call-template name="fastendaction"/>
<xsl:call-template name="createfastchildcontext"/>
<xsl:call-template name="fastattribute"/>
<xsl:call-template name="fastcharacters"/>
<xsl:call-template name="propagatesproperties"/>
        </xsl:if>
      </xsl:for-each>
    </xsl:for-each>
  </xsl:template>

  <xsl:key name="value-with-content" match="//rng:value"
           use="text()"/>

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
      Generates constant definitions for attribute values.
  -->
  <xsl:template name="valueconstants">
    <xsl:text>
rtl::OUString </xsl:text>
<xsl:call-template name="valuestringname">
  <xsl:with-param name="string"></xsl:with-param>
</xsl:call-template>
<xsl:text>(RTL_CONSTASCII_USTRINGPARAM(""));</xsl:text>
<xsl:for-each select="//rng:value[generate-id(key('value-with-content', text())[1]) = generate-id(.)]">
  <xsl:text>
rtl::OUString </xsl:text>
<xsl:call-template name="valuestringname">
  <xsl:with-param name="string" select="."/>
</xsl:call-template>
<xsl:text>(RTL_CONSTASCII_USTRINGPARAM("</xsl:text>
<xsl:value-of select="."/> 
<xsl:text>"));</xsl:text>
</xsl:for-each>
  </xsl:template>

  <!--
      Generates constant declarations for attribute values.
  -->
  <xsl:template name="valueconstantdecls">
    <xsl:text>
extern rtl::OUString 
    </xsl:text>
    <xsl:call-template name="valuestringname">
      <xsl:with-param name="string"></xsl:with-param>
    </xsl:call-template>
    <xsl:text>;</xsl:text>    
    <xsl:for-each select="//rng:value[generate-id(key('value-with-content', text())[1]) = generate-id(.)]">
      <xsl:text>
extern rtl::OUString </xsl:text>
      <xsl:call-template name="valuestringname">
        <xsl:with-param name="string" select="."/>
      </xsl:call-template>
      <xsl:text>;</xsl:text>
    </xsl:for-each>
  </xsl:template>

  <!--
      Generates definition of constructor for attribute value class
      for current <define>.
  -->
  <xsl:template name="valueconstructorimpl">
    <xsl:variable name="name" select="@name"/>
    <xsl:variable name="resource">
      <xsl:call-template name="contextresource"/>
    </xsl:variable>
    <xsl:choose>
      <xsl:when test="$resource = 'List'">
        <xsl:variable name="classname">
          <xsl:call-template name="valuenamefordefine"/>
        </xsl:variable>
        <xsl:text>&#xa;</xsl:text>
        <xsl:value-of select="$classname"/>
        <xsl:text>::</xsl:text>
        <xsl:value-of select="$classname"/>
        <xsl:text>(const rtl::OUString &amp; </xsl:text>
        <xsl:choose>
          <xsl:when test="count(ancestor::namespace/resource[@name=$name]/value) > 0">
            <xsl:text>rValue</xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>/* rValue */</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
        <xsl:text>)
: OOXMLListValue()
{</xsl:text>
    <xsl:for-each select="ancestor::namespace/resource[@name=$name]">
      <xsl:for-each select="./default">
        <xsl:text>
  mnValue = </xsl:text>
  <xsl:choose>
    <xsl:when test="@tokenid">
      <xsl:call-template name="idtoqname">
        <xsl:with-param name="id" select="@tokenid"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="."/>
    </xsl:otherwise>
  </xsl:choose>
  <xsl:text>;</xsl:text>
      </xsl:for-each>
      <xsl:for-each select="./value">
        <xsl:text>
  if (rValue.compareTo(</xsl:text>
  <xsl:call-template name="valuestringname">
    <xsl:with-param name="string" select="text()"/>
  </xsl:call-template>
  <xsl:text>) == 0)
    {
        mnValue = </xsl:text>
        <xsl:call-template name="idtoqname">
          <xsl:with-param name="id" select="@tokenid"/>
        </xsl:call-template>
        <xsl:text>; 
        return; 
    }</xsl:text>
      </xsl:for-each>
    </xsl:for-each>
  <xsl:text>
}
</xsl:text>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <!--
      Generates definition of destructor of attribute value class for
      current <define>.
  -->
  <xsl:template name="valuedestructorimpl">
    <xsl:variable name="classname">
      <xsl:call-template name="valuenamefordefine"/>
    </xsl:variable>
    <xsl:text>&#xa;</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>::~</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>()
{
}
    </xsl:text>
  </xsl:template>

  <!--
      Generates definitions for attribute value class for current
      <define>.
  -->
  <xsl:template name="valueimpls">
    <xsl:for-each select=".//rng:grammar/rng:define">
      <xsl:variable name="do">
        <xsl:call-template name="valuefordefine"/>
      </xsl:variable>
      <xsl:if test="$do = 1">
        <xsl:variable name="classname">
          <xsl:call-template name="valuenamefordefine"/>
        </xsl:variable>
        <xsl:text>
/*
    class: </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>
*/
    </xsl:text>
    <xsl:call-template name="valueconstructorimpl"/>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>

  <xsl:key name="tokenids" match="@tokenid|@sendtokenid" use="."/>

  <!--
      Generates contant definitions for tokenids.
  -->
  <xsl:template name="defineooxmlids">
    <xsl:text>
namespace NS_ooxml
{</xsl:text>
<xsl:for-each select="//@tokenid|//@sendtokenid">
  <xsl:if test="contains(., 'ooxml:') and generate-id(.) = generate-id(key('tokenids', .)[1])">
    <xsl:text>
    const Id LN_</xsl:text>
    <xsl:value-of select="substring-after(., 'ooxml:')"/>
    <xsl:text> = </xsl:text>
    <xsl:value-of select="90000 + position()"/>
    <xsl:text>;</xsl:text>
  </xsl:if>
</xsl:for-each>
}
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

  <xsl:template name="qnametostrfunc">
    <xsl:text>
string qnameToString(sal_uInt32 nToken)
{
    string sResult;

    switch (nToken)
    {</xsl:text>
    <xsl:for-each select="//resource">
      <xsl:variable name="name" select="@name"/>
      <xsl:for-each select="attribute|element">
        <xsl:if test="contains(@tokenid, 'ooxml:')">
          <xsl:text>
    case </xsl:text>
    <xsl:call-template name="idtoqname">
      <xsl:with-param name="id" select="@tokenid"/>
    </xsl:call-template>
    <xsl:text>:
        sResult = "</xsl:text>
        <xsl:value-of select="@token"/>
        <xsl:text>";
        break;</xsl:text>        
        </xsl:if>
      </xsl:for-each>
    </xsl:for-each>
    <xsl:text>
    default:
        ;
    }

    return sResult;
}</xsl:text>
  </xsl:template>

  <xsl:key name="resources-with-kind" match="resource[.//kind]"
           use=".//kind/@name"/>
  
  <xsl:key name="sprms-with-code" match="element" use="@tokenid"/>

  <!--
      Generates case labels for mapping from token ids to a single kind
      of sprm.

      @param kind     the sprm kind for which to generate the case labels
  -->
  <xsl:template name="sprmkindcase">
    <xsl:param name="kind"/>
    <xsl:for-each select="key('resources-with-kind', $kind)/element">
      <xsl:if test="generate-id(.) = generate-id(key('sprms-with-code', @tokenid))">
      <xsl:text>
     case </xsl:text>
     <xsl:call-template name="idtoqname">
       <xsl:with-param name="id" select="@tokenid"/>
     </xsl:call-template>
     <xsl:text>: //</xsl:text>
     <xsl:value-of select="ancestor::resource/@name"/>
     <xsl:text>, </xsl:text>
     <xsl:value-of select="@name"/>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>

  <!--
      Generates SprmKind.
  -->
  <xsl:template name="sprmkind">
    <xsl:text>
Sprm::Kind SprmKind(sal_uInt32 nSprmCode)
{
    Sprm::Kind nResult = Sprm::UNKNOWN;

    switch (nSprmCode)
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
}</xsl:text>
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

  <xsl:key name="same-token-name" match="rng:element|rng:attribute" use="@localname"/>

  <!--
      Generates input for gperf to genreate hash map for OOXMLFastTokenHandler
  -->
  <xsl:template name="gperfinputfasttokenhandler">
    <xsl:text>
%{
#include "OOXMLFastTokens.hxx"

namespace writerfilter { namespace ooxml { namespace tokenmap {
%}
struct token { const char * name; Token_t nToken; };
%%</xsl:text>
    <xsl:for-each select=".//rng:element|.//rng:attribute">
      <xsl:if test="generate-id(.) = generate-id(key('same-token-name', @localname)[1])">
        <xsl:text>&#xa;</xsl:text>
        <xsl:value-of select="@localname"/>
        <xsl:text>, </xsl:text>
        <xsl:call-template name="fastlocalname"/>
      </xsl:if>
    </xsl:for-each>
    <xsl:text>
%%&#xa;</xsl:text>
}}}&#xa;</xsl:template>

<xsl:template name="namespaceid">
  <xsl:text>NS_</xsl:text>
  <xsl:value-of select="@alias"/>
</xsl:template>

<xsl:template name="namespaceids">
  <xsl:for-each select="//namespace-alias">
    <xsl:text>
const sal_uInt32 </xsl:text>
<xsl:call-template name="namespaceid"/>
<xsl:text> = </xsl:text>
<xsl:value-of select="@id"/>
<xsl:text> &lt;&lt; 16;</xsl:text>
  </xsl:for-each>
</xsl:template>

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

<xsl:template name="fasttokenwithattr">
  <xsl:if test="local-name(.)='attribute'">
    <xsl:text>F_Attribute|</xsl:text>
  </xsl:if>
  <xsl:call-template name="fasttoken"/>
</xsl:template>

<xsl:template name="caselabelfasttoken">
  <xsl:text>case </xsl:text>
  <xsl:call-template name="fasttokenwithattr"/>
  <xsl:text>:</xsl:text>
</xsl:template>

<xsl:key name="elementsattrsbynamespace" 
         match="rng:element[@name]|rng:attribute[@name]" use="ancestor::rng:grammar/@ns"/>

<xsl:template name="fasttokentoid">
  <xsl:text>
namespace tokenmap {
struct token { const char * name; Token_t nToken; };
class Perfect_Hash
{
private:
  static inline unsigned int hash (const char *str, unsigned int len);
public:
  static struct token *in_word_set (const char *str, unsigned int len);
};
}

string fastTokenToId(sal_uInt32 nToken)
{
  </xsl:text>
  <xsl:text>    
    string sResult;

    switch (nToken &amp; 0xffff0000)
    {</xsl:text>
    <xsl:for-each select="//namespace-alias">
      <xsl:text>
    case NS_</xsl:text>
    <xsl:value-of select="@alias"/>
    <xsl:text>:
        sResult += "</xsl:text>
        <xsl:value-of select="@alias"/>
        <xsl:text>:";
        break;</xsl:text>
    </xsl:for-each>
    <xsl:text>
    }

    switch (nToken &amp; 0xffff)
    {</xsl:text>
  <xsl:for-each select=".//rng:element[@localname]|.//rng:attribute[@localname]">
    <xsl:variable name="localname" select="@localname"/>
    <xsl:if test="generate-id(.) = generate-id(key('same-token-name', $localname)[1])">
      <xsl:text>
    case </xsl:text>    
    <xsl:call-template name="fastlocalname"/>
    <xsl:text>:
        sResult +=  "</xsl:text>
        <xsl:value-of select="$localname"/>
        <xsl:text>";
        break;</xsl:text>        
    </xsl:if>
  </xsl:for-each>
<xsl:text>
    }

    return sResult;
}
  </xsl:text>
</xsl:template>

<xsl:template name="getfastparser">
<xsl:text>
uno::Reference &lt; xml::sax::XFastParser &gt; OOXMLStreamImpl::getFastParser()
{
    if (! mxFastParser.is())
    {
        uno::Reference &lt; lang::XMultiComponentFactory &gt; xFactory = 
            uno::Reference &lt; lang::XMultiComponentFactory &gt;
            (mxContext->getServiceManager());

        mxFastParser.set(xFactory->createInstanceWithContext
            ( ::rtl::OUString::createFromAscii
                ( "com.sun.star.xml.sax.FastParser" ), 
                    mxContext ), uno::UNO_QUERY_THROW);
</xsl:text>
<xsl:for-each select="//namespace-alias">
  <xsl:text>
        mxFastParser->registerNamespace(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("</xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text>")), </xsl:text>
    <xsl:call-template name="namespaceid"/>
    <xsl:text>);</xsl:text>
</xsl:for-each>
<xsl:text>
    }

    return mxFastParser;
}
</xsl:text>
</xsl:template>

<xsl:template name="searchdefinenamespace">
  <xsl:param name="name"/>
  <xsl:variable name="tmp">
    <xsl:for-each select="ancestor::namespace">
      <xsl:call-template name="searchdefinenamespacerec">
        <xsl:with-param name="name" select="$name"/>
      </xsl:call-template>
    </xsl:for-each>
  </xsl:variable>
  <xsl:choose>
    <xsl:when test="string-length($tmp) > 0">
      <xsl:value-of select="$tmp"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:for-each select="key('defines-with-name', $name)[1]">
        <xsl:value-of select="ancestor::namespace/@name"/>
        <xsl:text>:</xsl:text>
        <xsl:value-of select="@name"/>
      </xsl:for-each>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="searchdefinenamespacerec">
  <xsl:param name="name"/>
  <xsl:variable name="nsname" select="@name"/>
  <xsl:variable name="nsid" select="generate-id(.)"/>
  <xsl:variable name="tmp">
    <xsl:for-each select="key('defines-with-name', $name)">
      <xsl:if test="generate-id(ancestor::namespace)=$nsid">
        <xsl:value-of select="$nsname"/>
        <xsl:text>:</xsl:text>
        <xsl:value-of select="$name"/>
      </xsl:if>
    </xsl:for-each>
  </xsl:variable>
  <xsl:choose>
    <xsl:when test="string-length($tmp) > 0">
      <xsl:value-of select="$tmp"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:for-each select="./rng:include">
        <xsl:for-each select="/model/namespace[@file=@href]">
          <xsl:call-template name="searchdefinenamespacerec">
            <xsl:with-param name="name" select="$name"/>
          </xsl:call-template>
        </xsl:for-each>
      </xsl:for-each>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

  <xsl:template name="fastattributescheckattr">
    <xsl:text>if (Attribs->hasAttribute(</xsl:text>
    <xsl:call-template name="fastlocalname"/>
    <xsl:text>))</xsl:text>
  </xsl:template>
    
  <xsl:template name="fastattributescheckattrwithns">
    <xsl:text>if (Attribs->hasAttribute(</xsl:text>
    <xsl:call-template name="fasttoken"/>
    <xsl:text>))</xsl:text>
  </xsl:template>
    
  <xsl:template name="fastcontextnameforattribute">
    <xsl:choose>      
      <xsl:when test="./rng:ref">
        <xsl:for-each select="./rng:ref">
          <xsl:call-template name="valuenameforref"/>
        </xsl:for-each>
      </xsl:when>
      <xsl:when test="./rng:data/@type = 'boolean'">
        <xsl:text>OOXMLBooleanValue</xsl:text>
      </xsl:when>
      <xsl:when test="./rng:data/@type = 'unsignedInt'">
        <xsl:text>OOXMLIntegerValue</xsl:text>
      </xsl:when>
      <xsl:when test="./rng:text or ./rng:data/@type = 'string' or ./rng:data/@type = 'token'">
        <xsl:text>OOXMLStringValue</xsl:text>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="fastattributesproperties">
    <xsl:variable name="definename" select="@name"/>
    <xsl:for-each select=".//rng:attribute">
      <xsl:variable name="attrname" select="@name"/>
      <xsl:variable name="contextname">
        <xsl:call-template name="fastcontextnameforattribute"/>
      </xsl:variable>
      <xsl:variable name="attrid">
        <xsl:call-template name="idforattr"/>
      </xsl:variable>      
      <xsl:if test="string-length($contextname) > 0 and string-length($attrid) > 0">
        <xsl:text>
        </xsl:text>
        <xsl:call-template name="fastattributescheckattrwithns"/>
        <xsl:text>
        {
           ::rtl::OUString aValue(Attribs->getValue(</xsl:text>
           <xsl:call-template name="fasttoken"/>
           <xsl:text>));
           OOXMLFastHelper &lt; </xsl:text>
           <xsl:value-of select="$contextname"/>
           <xsl:text> &gt;::newProperty(this, </xsl:text>
           <xsl:value-of select="$attrid"/>
           <xsl:text>, aValue);</xsl:text>
           <xsl:for-each select="ancestor::namespace/resource[@name=$definename]">
             <xsl:for-each select="./attribute[@name=$attrname]">
               <xsl:choose>
                 <xsl:when test="@action='checkId'">
               <xsl:text>
           checkId(aValue);</xsl:text>
                 </xsl:when>
                 <xsl:when test="@action='checkXNoteType'">
                 </xsl:when>
                 <xsl:when test="@action='setXNoteId'">
               <xsl:text>
           setXNoteId(aValue);</xsl:text>
                 </xsl:when>
               </xsl:choose>
             </xsl:for-each>
           </xsl:for-each>
           <xsl:text>
        }</xsl:text>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="fastattributesstringvalue">
    <xsl:for-each select=".//rng:attribute">
      <xsl:text>
      </xsl:text>
      <xsl:call-template name="fastattributescheckattrwithns"/>
      <xsl:text>
        msValue = Attribs->getValue(</xsl:text>
        <xsl:call-template name="fasttoken"/>
        <xsl:text>);</xsl:text>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="fastattributesintvalue">
    <xsl:for-each select=".//rng:attribute">
      <xsl:text>
      </xsl:text>
      <xsl:call-template name="fastattributescheckattrwithns"/>
      <xsl:text>
        mnValue = Attribs->getValue(</xsl:text>
        <xsl:call-template name="fasttoken"/>
        <xsl:text>).toInt32();</xsl:text>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="fastattributeshexvalue">
    <xsl:for-each select=".//rng:attribute">
      <xsl:text>
      </xsl:text>
      <xsl:call-template name="fastattributescheckattrwithns"/>
      <xsl:text>
        mnValue = Attribs->getValue(</xsl:text>
        <xsl:call-template name="fasttoken"/>
        <xsl:text>).toInt32(16);</xsl:text>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="fastattributesboolvalue">
    <xsl:for-each select=".//rng:attribute">
      <xsl:text>
      </xsl:text>
      <xsl:call-template name="fastattributescheckattrwithns"/>
      <xsl:text>
        setValue(Attribs->getValue(</xsl:text>
        <xsl:call-template name="fasttoken"/>
        <xsl:text>));</xsl:text>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="fastattributeslistvalue">
    <xsl:for-each select=".//rng:attribute">
      <xsl:variable name="myfasttoken">
        <xsl:call-template name="fastlocalname"/>
      </xsl:variable>
      <xsl:variable name="myfasttokenwithns">
        <xsl:call-template name="fasttoken"/>
      </xsl:variable>
      <xsl:variable name="bodywithns">        
        <xsl:for-each select="rng:ref">
          <xsl:variable name="refname" select="@name"/>
          <xsl:variable name="refns">
            <xsl:call-template name="searchdefinenamespace">
              <xsl:with-param name="name" select="@name"/>
            </xsl:call-template>
          </xsl:variable>
          <xsl:variable name="valname">
            <xsl:for-each select="/model/namespace[@name=substring-before($refns, ':')]">
              <xsl:for-each select="./rng:grammar/rng:define[@name=substring-after($refns, ':')]">
                <xsl:call-template name="valuenamefordefine"/>
              </xsl:for-each>
            </xsl:for-each>
          </xsl:variable>
          <xsl:text>
            mpValue = OOXMLValue::Pointer_t (new </xsl:text>
          <xsl:value-of select="$valname"/>
          <xsl:text>(Attribs->getValue(</xsl:text>
          <xsl:value-of select="$myfasttokenwithns"/>
          <xsl:text>)));</xsl:text>
        </xsl:for-each>
      </xsl:variable>
      <xsl:if test="string-length($bodywithns) > 0">
        <xsl:text>
        </xsl:text>
        <xsl:call-template name="fastattributescheckattrwithns"/>
        <xsl:value-of select="$bodywithns"/>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="fastattributebody">
    <xsl:variable name="resource">
      <xsl:call-template name="contextresource"/>
    </xsl:variable>
    <xsl:choose>
      <xsl:when test="$resource = 'Properties' or $resource = 'Stream' or $resource='XNote' or $resource='Shape'" >
        <xsl:call-template name="fastattributesproperties"/>
      </xsl:when>
      <xsl:when test="$resource = 'StringValue'">
        <xsl:call-template name="fastattributesstringvalue"/>
      </xsl:when>
      <xsl:when test="$resource = 'IntegerValue'">
        <xsl:call-template name="fastattributesintvalue"/>
      </xsl:when>
      <xsl:when test="$resource = 'HexValue'">
        <xsl:call-template name="fastattributeshexvalue"/>
      </xsl:when>
      <xsl:when test="$resource = 'BooleanValue'">
        <xsl:call-template name="fastattributesboolvalue"/>
      </xsl:when>
      <xsl:when test="$resource = 'ListValue'">
        <xsl:call-template name="fastattributeslistvalue"/>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="valuenameforref">
    <xsl:variable name="definename">
      <xsl:call-template name="searchdefinenamespace">
        <xsl:with-param name="name" select="@name"/>
      </xsl:call-template>
    </xsl:variable>
    <xsl:for-each select="/model/namespace[@name=substring-before($definename, ':')]">
      <xsl:for-each select="./rng:grammar/rng:define[@name=substring-after($definename, ':')]">
        <xsl:call-template name="valuenamefordefine"/>
      </xsl:for-each>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="fastcontextnameforref">
    <xsl:variable name="definename">
      <xsl:call-template name="searchdefinenamespace">
        <xsl:with-param name="name" select="@name"/>
      </xsl:call-template>
    </xsl:variable>
    <!--
    <xsl:text>/* </xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text>, </xsl:text>
    <xsl:value-of select="$definename"/>
    <xsl:text> */</xsl:text>
    -->
    <xsl:for-each select="/model/namespace[@name=substring-before($definename, ':')]">
      <xsl:for-each select="./rng:grammar/rng:define[@name=substring-after($definename, ':')]">
        <xsl:call-template name="fastcontextname"/>
      </xsl:for-each>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="fastattributebodyrefs">
    <xsl:for-each select=".//rng:ref[not(ancestor::rng:element or ancestor::rng:attribute)]">
      <xsl:variable name="contextname">
        <xsl:call-template name="fastcontextnameforref"/>
      </xsl:variable>
      <xsl:if test="string-length($contextname) > 0">
        <xsl:text>
      OOXMLFastHelper &lt;</xsl:text>
      <xsl:value-of select="$contextname"/>
      <xsl:text>&gt;::attributes(this, Attribs);</xsl:text>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>

  <!--
      Generates definition of method attributes for current <define>
  -->
  <xsl:template name="fastattribute">
    <xsl:variable name="resource">
      <xsl:call-template name="contextresource"/>
    </xsl:variable>
    <!--<xsl:if test="not($resource='Shape')"> -->
      <xsl:variable name="body">
        <xsl:call-template name="fastattributebody"/>
      </xsl:variable>
      <xsl:variable name="bodyrefs">
        <xsl:call-template name="fastattributebodyrefs"/>
      </xsl:variable>
      <xsl:if test="string-length($body) > 0 or string-length($bodyrefs) > 0">
        <xsl:text>
void </xsl:text>
<xsl:call-template name="fastcontextname"/>
<xsl:text>::attributes
(const uno::Reference &lt; xml::sax::XFastAttributeList &gt; &amp; Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_DUMP_ATTRIBUTES
    dumpAttribs(Attribs);
#endif
</xsl:text>
<xsl:value-of select="$body"/>
<xsl:value-of select="$bodyrefs"/>
<xsl:text>
}
</xsl:text>
      </xsl:if>
        <!-- </xsl:if> -->
  </xsl:template>

  <xsl:template name="fastelementcreatestatement">
    <xsl:for-each select=".//rng:ref">
      <xsl:choose>
        <xsl:when test="@name='BUILT_IN_ANY_TYPE'">
          <xsl:text>createFromStart(Element, Attribs)</xsl:text>
        </xsl:when>
        <xsl:otherwise>
          <xsl:variable name="classname">
            <xsl:call-template name="fastcontextname"/>
          </xsl:variable>
          <xsl:if test="string-length($classname) > 0">
            <xsl:text>OOXMLFastHelper &lt;</xsl:text>
            <xsl:value-of select="$classname"/>
            <xsl:text>&gt;::createAndSetParent(this, Element, </xsl:text>
            <xsl:for-each select="ancestor::rng:element">
              <xsl:variable name="id">
                <xsl:call-template name="idforelement"/>
              </xsl:variable>
              <xsl:choose>
                <xsl:when test="string-length($id) > 0">
                  <xsl:value-of select="$id"/>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:text>0</xsl:text>
                </xsl:otherwise>
              </xsl:choose>
            </xsl:for-each>
            <xsl:text>)</xsl:text>
          </xsl:if>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:for-each>
  </xsl:template>

  <!--
      Generates name for parent class of fast context.
  -->
  <xsl:template name="fastresourceclass">
    <xsl:text>OOXMLFastContextHandler</xsl:text>
    <xsl:call-template name="contextresource"/>    
  </xsl:template>

  <!--
      Generates the declaration of the fast context for the current <define>
  -->
  <xsl:template name="fastcontextdecl">
    <xsl:variable name="classname">
      <xsl:call-template name="fastcontextname"/>
    </xsl:variable>
    <xsl:text>
class WRITERFILTER_DLLPRIVATE </xsl:text>
<xsl:value-of select="$classname"/>
<xsl:text>: public </xsl:text>
<xsl:call-template name="fastresourceclass"/>
<xsl:text>
{
public:
    explicit </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>
    (OOXMLFastContextHandler * context);
    virtual ~</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>();
    </xsl:text>
    <xsl:variable name="createchildcontextbody">
      <xsl:call-template name="createfastchildcontext"/>
    </xsl:variable>
    <xsl:if test="string-length($createchildcontextbody) > 0">
    virtual uno::Reference &lt; xml::sax::XFastContextHandler &gt;
    lcl_createFastChildContext
    (::sal_Int32 Element, 
     const uno::Reference &lt; xml::sax::XFastAttributeList &gt; &amp; Attribs) 
        throw (uno::RuntimeException, xml::sax::SAXException);
    </xsl:if>
    <xsl:variable name="fastattributebody">
      <xsl:call-template name="fastattribute"/>
    </xsl:variable>
    <xsl:if test="string-length($fastattributebody) > 0">
    virtual void attributes
    (const uno::Reference &lt; xml::sax::XFastAttributeList &gt; &amp; Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException);
    </xsl:if>
    <xsl:variable name="faststartactionbody">
      <xsl:call-template name="faststartaction"/>
    </xsl:variable>
    <xsl:if test="string-length($faststartactionbody)">
     virtual void lcl_startAction(Token_t nElement);
    </xsl:if>
    <xsl:variable name="fastendactionbody">
      <xsl:call-template name="fastendaction"/>
    </xsl:variable>
    <xsl:if test="string-length($fastendactionbody)">
     virtual void lcl_endAction(Token_t nElement);
    </xsl:if>
    <xsl:variable name="fastcharactersbody">
      <xsl:call-template name="fastcharacters"/>
    </xsl:variable>
    <xsl:if test="string-length($fastcharactersbody)">
    virtual void lcl_characters(const ::rtl::OUString &amp; aChars) 
        throw (uno::RuntimeException, xml::sax::SAXException);
    </xsl:if>
    <xsl:text>
    virtual string getType() const { return "</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>"; }</xsl:text>
    <xsl:variable name="propagatespropsbody">
      <xsl:call-template name="propagatesproperties"/>
    </xsl:variable>
    <xsl:if test="string-length($propagatespropsbody)">
      <xsl:text>
    virtual bool propagatesProperties() const;</xsl:text>
    </xsl:if>
    <xsl:text>
};
    </xsl:text>
  </xsl:template>

  <xsl:template name="fastcontextdecls">
    <xsl:param name="namespace"/>
    <xsl:for-each select="/model/namespace[@name=$namespace]">
      <xsl:for-each select="./rng:grammar/rng:define">
        <xsl:variable name="do">
          <xsl:call-template name="classfordefine"/>
        </xsl:variable>
        <xsl:text>
/*
        </xsl:text>
        <xsl:value-of select="ancestor::namespace/@name"/>
        <xsl:text>:</xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>
*/</xsl:text>
<xsl:if test="$do = '1'">
  <xsl:call-template name="fastcontextdecl"/>
</xsl:if>
      </xsl:for-each>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="fastcontextconstructor">
    <xsl:variable name="classname">
      <xsl:call-template name="fastcontextname"/>
    </xsl:variable>
    <xsl:value-of select="$classname"/>
    <xsl:text>::</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>
(OOXMLFastContextHandler * pContext)
: </xsl:text>
<xsl:call-template name="fastresourceclass"/>
<xsl:text>(pContext)
{}
</xsl:text>
  </xsl:template>

  <xsl:template name="fastcontextdestructor">
    <xsl:variable name="classname">
      <xsl:call-template name="fastcontextname"/>
    </xsl:variable>
    <xsl:value-of select="$classname"/>
    <xsl:text>::~</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>
()
{}
</xsl:text>
  </xsl:template>

  <xsl:template name="faststartactionbodysetid">
      <xsl:if test="@resource = 'Table' or @resource='PropertyTable'">
        <xsl:text>
    setId(</xsl:text>
    <xsl:call-template name="idtoqname">
      <xsl:with-param name="id" select="@tokenid"/>
    </xsl:call-template>
    <xsl:text>);</xsl:text>
      </xsl:if>
  </xsl:template>

  <xsl:template name="faststartactionbodychooseaction">
    <xsl:for-each select="./action[@name='start']">
      <xsl:call-template name="chooseaction"/>
    </xsl:for-each>    
  </xsl:template>

  <xsl:template name="faststartactionbody">
    <xsl:variable name="name" select="@name"/>
    <xsl:for-each select="ancestor::namespace/resource[@name = $name]">
      <xsl:call-template name="faststartactionbodysetid"/>
      <xsl:call-template name="faststartactionbodychooseaction"/>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="faststartaction">
    <xsl:variable name="body">
      <xsl:call-template name="faststartactionbody"/>
    </xsl:variable>
    <xsl:variable name="name" select="@name"/>
    <xsl:if test="string-length($body) > 0">
      <xsl:variable name="classname">
        <xsl:call-template name="fastcontextname"/>
      </xsl:variable>
      <xsl:text>
void </xsl:text>
<xsl:value-of select="$classname"/>
<xsl:text>::lcl_startAction(Token_t</xsl:text>
<xsl:for-each select="ancestor::namespace/resource[@name=$name]">
  <xsl:if test="./element/action[@name='start']">
    <xsl_text> nElement</xsl_text>
  </xsl:if>
</xsl:for-each>
<xsl:text>)
{</xsl:text>
<xsl:value-of select="$body"/>
<xsl:text>
}
</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template name="fastendactionbody">
    <xsl:variable name="name" select="@name"/>
    <xsl:for-each select="ancestor::namespace/resource[@name = $name]">
      <xsl:for-each select="./action[@name='end']">
        <xsl:call-template name="chooseaction"/>
      </xsl:for-each>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="fastendaction">
    <xsl:variable name="body">
      <xsl:call-template name="fastendactionbody"/>
    </xsl:variable>
    <xsl:variable name="name" select="@name"/>
    <xsl:if test="string-length($body) > 0">
      <xsl:variable name="classname">
        <xsl:call-template name="fastcontextname"/>
      </xsl:variable>
      <xsl:text>
void </xsl:text>
<xsl:value-of select="$classname"/>
<xsl:text>::lcl_endAction(Token_t</xsl:text>
<xsl:for-each select="ancestor::namespace/resource[@name=$name]">
  <xsl:if test="./element/action[@name='end']">
    <xsl_text> nElement</xsl_text>
  </xsl:if>
</xsl:for-each>
<xsl:text>)
{</xsl:text>
<xsl:value-of select="$body"/>
<xsl:text>
}
</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template name="fastcharactersbody">
    <xsl:variable name="name" select="@name"/>
    <xsl:for-each select="ancestor::namespace/resource[@name = $name]//action[@name='characters']">
      <xsl:call-template name="chooseaction"/>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="fastcharacters">
    <xsl:variable name="body">
      <xsl:call-template name="fastcharactersbody"/>
    </xsl:variable>
    <xsl:if test="string-length($body) > 0">
      <xsl:variable name="classname">
        <xsl:call-template name="fastcontextname"/>
      </xsl:variable>
      <xsl:text>
void </xsl:text>
<xsl:value-of select="$classname"/>
<xsl:text>::lcl_characters(const ::rtl::OUString &amp; sText) 
    throw (uno::RuntimeException, xml::sax::SAXException)
{</xsl:text>
<xsl:value-of select="$body"/>
<xsl:text>
}
</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template name="propagatespropertiesbody">
    <xsl:variable name="name" select="@name"/>
    <xsl:for-each select="ancestor::namespace/resource[@name=$name]">
      <xsl:for-each select=".//action">
        <xsl:choose>          
          <xsl:when test="@name='propagateCharacterProperties'">
            <xsl:text>
    return true;</xsl:text>
          </xsl:when>
          <xsl:when test="@name='propagateTableProperties'">
            <xsl:text>
    return true;</xsl:text>
          </xsl:when>
        </xsl:choose>
      </xsl:for-each>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="propagatesproperties">
    <xsl:variable name="body">
      <xsl:call-template name="propagatespropertiesbody"/>
    </xsl:variable>
    <xsl:if test="string-length($body) > 0">
      <xsl:variable name="classname">
        <xsl:call-template name="fastcontextname"/>
      </xsl:variable>
      <xsl:text>
bool </xsl:text>
<xsl:value-of select="$classname"/>
<xsl:text>::propagatesProperties() const
{</xsl:text>
      <xsl:value-of select="$body"/>
      <xsl:text>
}
      </xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template name="fastcontextcreatefromstart">
    <xsl:text>
uno::Reference &lt; xml::sax::XFastContextHandler &gt; 
OOXMLFastContextHandler::createFromStart
(::sal_Int32 Element,
 const uno::Reference &lt; xml::sax::XFastAttributeList &gt; &amp; Attribs) 
{
#ifdef DEBUG_CREATE
    debug_logger-&gt;startElement("createfromstart");
    debug_logger-&gt;attribute("element", fastTokenToId(Element));
#endif
    uno::Reference &lt; xml::sax::XFastContextHandler &gt; xResult;</xsl:text>

    <xsl:for-each select="//namespace/start">
      <xsl:variable name="name" select="@name"/>
      <xsl:for-each select="ancestor::namespace/rng:grammar/rng:define[@name=$name]">
        <xsl:text>    
    if (! xResult.is() || dynamic_cast&lt;OOXMLFastContextHandler *&gt;(xResult.get())->isFallback())
    {
        xResult = OOXMLFastHelper &lt; </xsl:text>
    <xsl:call-template name="fastcontextname"/>
    <xsl:text> &gt;::createAndSetParentRef(this, Element, Attribs);
    }</xsl:text>
      </xsl:for-each>
    </xsl:for-each>
    <xsl:text>

#ifdef DEBUG_CREATE
    debug_logger-&gt;endElement("createfromstart");
#endif

    return xResult;
}
    </xsl:text>
  </xsl:template>
  
  <xsl:key name="attribs-qnames" match="rng:attribute" use="@qname"/>

  <xsl:template name="dumpattribs">
    <xsl:text>
void dumpAttrib(const char * sToken, sal_uInt32 nToken, 
                const uno::Reference &lt; xml::sax::XFastAttributeList 
                &gt; &amp; Attribs)
{
   try
   {
       if (Attribs->hasAttribute(nToken))
       {
           debug_logger-&gt;startElement("attrib");
           debug_logger-&gt;attribute("id", sToken);
           debug_logger-&gt;chars(Attribs->getValue(nToken));
           debug_logger-&gt;endElement("attrib"); 
       }
   }
   catch (...)
   {
      debug_logger-&gt;startElement("error");
      debug_logger-&gt;chars(sToken);
      debug_logger-&gt;endElement("error");
   }
}

void dumpAttribs
(const uno::Reference &lt; xml::sax::XFastAttributeList &gt; &amp; Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException)
{
    debug_logger-&gt;startElement("attribs");
    </xsl:text>
    <xsl:for-each select="//rng:attribute[@name]">
      <xsl:if test="generate-id(.) = generate-id(key('attribs-qnames', @qname)[1]
)">
        <xsl:text>
        dumpAttrib("</xsl:text>
        <xsl:call-template name="fastlocalname"/>
        <xsl:text>", </xsl:text>
        <xsl:call-template name="fastlocalname"/>
        <xsl:text>, Attribs);</xsl:text>
        <xsl:text>
        dumpAttrib("</xsl:text>
        <xsl:call-template name="fasttoken"/>
        <xsl:text>", </xsl:text>
        <xsl:call-template name="fasttoken"/>
        <xsl:text>, Attribs);</xsl:text>
      </xsl:if>
    </xsl:for-each>
    <xsl:text>
    debug_logger-&gt;endElement("attribs");
}</xsl:text>
  </xsl:template>

<xsl:template name="createfastchildcontextname">
  <xsl:text>createFastContextHandler_</xsl:text>
  <xsl:value-of select="@name"/>
</xsl:template>

<xsl:template name="idfornamespace">
    <xsl:text>NN_</xsl:text>
    <xsl:value-of select="translate(@name, '-', '_')"/>
</xsl:template>

<xsl:template name="localidfordefine">
    <xsl:text>DEFINE_</xsl:text>
    <xsl:value-of select="@name"/>
</xsl:template>

<xsl:template name="idfordefine">
    <xsl:for-each select="ancestor::namespace">
        <xsl:call-template name="idfornamespace"/>
    </xsl:for-each>
    <xsl:text>|</xsl:text>
    <xsl:call-template name="localidfordefine"/>
</xsl:template>

<xsl:template name="caselabeldefine">
  <xsl:text>case </xsl:text>
  <xsl:call-template name="idfordefine"/>
  <xsl:text>:</xsl:text>
</xsl:template>

<xsl:template name="createfastchildcontextswitchbodyref">
  <xsl:variable name="definename">
    <xsl:call-template name="searchdefinenamespace">
      <xsl:with-param name="name" select="@name"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:for-each select="/model/namespace[@name=substring-before($definename, ':')]">
    <xsl:for-each select="./rng:grammar/rng:define[@name=substring-after($definename, ':')]">
      <xsl:call-template name="createfastchildcontextswitchbody"/>
    </xsl:for-each>
  </xsl:for-each>
</xsl:template>

<xsl:template name="createfastchildcontextswitchbody">
  <xsl:for-each select=".//rng:element">
    <xsl:text>
        </xsl:text>
        <xsl:call-template name="caselabelfasttoken"/>
        <xsl:text>
            xResult.set(</xsl:text>
            <xsl:call-template name="fastelementcreatestatement"/>
            <xsl:text>);
            break;</xsl:text>
  </xsl:for-each>
  <xsl:for-each 
      select=".//rng:ref[not(ancestor::rng:element or ancestor::rng:attribute)]">
    <xsl:call-template name="createfastchildcontextswitchbodyref"/>
  </xsl:for-each>
</xsl:template>

<xsl:template name="createfastchildcontextswitch">
  <xsl:variable name="body">
    <xsl:call-template name="createfastchildcontextswitchbody"/>
  </xsl:variable>
  <xsl:if test="string-length($body) > 0">
    <xsl:text>
        switch(Element)
        {</xsl:text>
        <xsl:value-of select="$body"/>
        <xsl:text>
        default:
            ;
        }</xsl:text>
  </xsl:if>
</xsl:template>

<xsl:template name="createfastchildcontextlookup">
  <xsl:text>
uno::Reference&lt;XFastContextHandler&gt; </xsl:text>
<xsl:call-template name="createfastchildcontextname"/>
<xsl:text>(Id parent, Token_t Element)
{
    uno::Reference&lt;XFastContextHandler&gt; xResult;
    switch (parent)
    {</xsl:text>
<xsl:for-each select="./rng:grammar/rng:define">
  <xsl:variable name="do">
    <xsl:call-template name="classfordefine"/>
  </xsl:variable>
  <xsl:if test="$do='1'">
    <xsl:variable name="casebody">
      <xsl:call-template name="createfastchildcontextswitch"/>
    </xsl:variable>
    <xsl:if test="string-length($casebody) > 0">
      <xsl:text>
    </xsl:text>
    <xsl:call-template name="caselabeldefine"/>
    <xsl:value-of select="$casebody"/>
    <xsl:text>
        break;</xsl:text>
    </xsl:if>
  </xsl:if>
</xsl:for-each>
<xsl:text>
    default:
        ;
    }

    return xResult;
}
</xsl:text>
</xsl:template>

<!-- *** Factory *** -->

<xsl:template name="factoryattributetoresourcemapinner">
    <xsl:for-each select=".//rng:attribute">
        <xsl:variable name="resource">
            <xsl:for-each select="rng:ref">
                <xsl:value-of select="key('context-resource', @name)/@resource"/>
            </xsl:for-each>
        </xsl:variable>
        <xsl:if test="string-length($resource) > 0">
            <xsl:text>
            (*pMap)[</xsl:text>
            <xsl:call-template name="fasttoken"/>
            <xsl:text>] = RT_</xsl:text>
            <xsl:value-of select="$resource"/>
            <xsl:text>;</xsl:text>
        </xsl:if>
    </xsl:for-each>
    <xsl:for-each select=".//rng:ref[not(ancestor::rng:element or ancestor::rng:attribute)]">
        <xsl:call-template name="factoryattributetoresourcemapinner"/>
    </xsl:for-each>
</xsl:template>

<!-- factoryattributetoresourcemap -->
<xsl:template name="factoryattributetoresourcemap">
    <xsl:text>
AttributeToResourceMapPointer </xsl:text>
    <xsl:call-template name="factoryclassname"/>
    <xsl:text>::createAttributeToResourceMap(Id nId)
{
    AttributeToResourceMapPointer pMap(new AttributeToResourceMap());
    
    switch (nId)
    {</xsl:text>
    <xsl:for-each select="rng:grammar/rng:define">
        <xsl:variable name="inner">
            <xsl:call-template name="factoryattributetoresourcemapinner"/>
        </xsl:variable>
        <xsl:if test="string-length($inner) > 0">
            <xsl:text>
    </xsl:text>
            <xsl:call-template name="caselabeldefine"/>
            <xsl:value-of select="$inner"/>
            <xsl:text>
        break;</xsl:text>
        </xsl:if>
    </xsl:for-each>
    default:
        break;
    }
    
    return pMap;
}
</xsl:template>

<!-- factorylistvaluemap -->
<xsl:template name="factorylistvaluemap">
    <xsl:text>
ListValueMapPointer </xsl:text>
    <xsl:call-template name="factoryclassname"/>
    <xsl:text>::createListValueMap(Id nId)
{
    ListValueMapPointer pMap(new ListValueMap());
    
    switch (nId)
    {</xsl:text>
    <xsl:for-each select="resource[@resource='List']">
        <xsl:text>
    </xsl:text>
        <xsl:call-template name="caselabeldefine"/>
        <xsl:for-each select="value">
            <xsl:text>
        (*pMap)[</xsl:text>
            <xsl:call-template name="valuestringname">
                <xsl:with-param name="string" select="text()"/>
            </xsl:call-template>
            <xsl:text>] = </xsl:text>
            <xsl:call-template name="idtoqname">
                <xsl:with-param name="id" select="@tokenid"/>
            </xsl:call-template>
            <xsl:text>;</xsl:text>
        </xsl:for-each>
        <xsl:text>
        break;</xsl:text>
    </xsl:for-each>
    <xsl:text>
    default:
        break;
    }
    
    return pMap;
}</xsl:text>
</xsl:template>

<xsl:template name="idforref">
    <xsl:variable name="name" select="@name"/>
    <xsl:for-each select="ancestor::namespace/rng:grammar/rng:define[@name=$name]">
        <xsl:call-template name="idfordefine"/>
    </xsl:for-each>
</xsl:template>

<!-- factoryelementtoresorucemapinner -->
<xsl:template name="factorycreateelementmapinner">
    <xsl:for-each select=".//rng:element">
        <xsl:variable name="resource">
            <xsl:for-each select="rng:ref">
                <xsl:call-template name="contextresource"/>
            </xsl:for-each>
        </xsl:variable>
        <xsl:if test="string-length($resource) > 0">
            <xsl:text>
        (*pMap)[</xsl:text>
            <xsl:call-template name="fasttoken"/>
            <xsl:text>] = CreateElement(RT_</xsl:text>
            <xsl:value-of select="$resource"/>
            <xsl:text>, </xsl:text>
            <xsl:for-each select="rng:ref">
                <xsl:call-template name="idforref"/>
            </xsl:for-each>
            <xsl:text>);</xsl:text>
        </xsl:if>
    </xsl:for-each>
    <xsl:for-each select=".//rng:ref[not(ancestor::rng:element or ancestor::rng:attribute)]">
        <xsl:variable name="name" select="@name"/>
        <xsl:variable name="block">
            <xsl:for-each select="ancestor::namespace/rng:grammar/rng:define[@name=$name]">
                <xsl:call-template name="factorycreateelementmapinner"/>
            </xsl:for-each>
        </xsl:variable>
        <xsl:if test="string-length($block) > 0">
            <xsl:text>
         /* ref: </xsl:text>
            <xsl:value-of select="$name"/>
            <xsl:text>*/</xsl:text>
            <xsl:value-of select="$block"/>
        </xsl:if>
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

<!-- factoryelementtoresourcemap -->
<xsl:template name="factorycreateelementmap">
    <xsl:text>
CreateElementMapPointer </xsl:text>
    <xsl:call-template name="factoryclassname"/>
    <xsl:text>::createCreateElementMap(Id nId)
{
    CreateElementMapPointer pMap(new CreateElementMap());
    
    switch (nId)
    {</xsl:text>
    <xsl:for-each select="rng:grammar/rng:define">
        <xsl:variable name="inner">
            <xsl:call-template name="factorycreateelementmapinner"/>
        </xsl:variable>
        <xsl:if test="string-length($inner) > 0">
            <xsl:text>
    </xsl:text>
            <xsl:call-template name="caselabeldefine"/>
            <xsl:value-of select="$inner"/>             
            <xsl:text>
        break;</xsl:text>
        </xsl:if>
    </xsl:for-each>
    <xsl:text>
    default:</xsl:text>
        <xsl:call-template name="factorycreateelementmapfromstart"/>
        <xsl:text>
        break;
    }
    
    return pMap;
}</xsl:text>
</xsl:template>

<xsl:template name="factorytokentoidmapinner">
    <xsl:variable name="name" select="@name"/>
    <xsl:variable name="body">
        <xsl:for-each select="ancestor::namespace/resource[@name=$name]">
            <xsl:for-each select="element[@tokenid]">
                <xsl:text>
        (*pMap)[</xsl:text>
                <xsl:call-template name="fasttoken"/>
                <xsl:text>] = </xsl:text>
                <xsl:call-template name="idtoqname">
                    <xsl:with-param name="id" select="@tokenid"/>
                </xsl:call-template>
                <xsl:text>;</xsl:text>
            </xsl:for-each>
        </xsl:for-each>
    </xsl:variable>
    <xsl:if test="string-length($body) > 0">
        <xsl:text>
        // </xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:value-of select="$body"/>
    </xsl:if>
    <xsl:for-each select=".//rng:ref[not(ancestor::rng:element or ancestor::rng:attribute)]">
        <xsl:variable name="refname" select="@name"/>
        <xsl:for-each select="ancestor::rng:grammar/rng:define[@name=$refname]">
            <xsl:call-template name="factorytokentoidmapinner"/>
        </xsl:for-each>
    </xsl:for-each>
</xsl:template>

<xsl:template name="factorytokentoidmap">
    <xsl:text>
TokenToIdMapPointer </xsl:text>
    <xsl:call-template name="factoryclassname"/>
    <xsl:text>::createTokenToIdMap(Id nId)
{
    TokenToIdMapPointer pMap(new TokenToIdMap());
    
    switch (nId)
    {</xsl:text>
    <xsl:for-each select="rng:grammar/rng:define">
        <xsl:variable name="inner">
            <xsl:call-template name="factorytokentoidmapinner"/>
        </xsl:variable>
        <xsl:if test="string-length($inner) > 0">
            <xsl:text>
    </xsl:text>
            <xsl:call-template name="caselabeldefine"/>
            <xsl:value-of select="$inner"/>
                <xsl:text>
        break;</xsl:text>
        </xsl:if>
    </xsl:for-each>
    <xsl:text>
    default:
        break;
    }
    
    return pMap;
}</xsl:text>
</xsl:template>

  <!--
      Chooses the action for the current <action> element.
  -->
  <xsl:template name="factorychooseaction">
    <xsl:if test="@tokenid">
      <xsl:text>
        if (sal::static_int_cast&lt;Id&gt;(pHandler-&gt;getId()) == </xsl:text>
      <xsl:call-template name="idtoqname">
        <xsl:with-param name="id" select="@tokenid"/>
      </xsl:call-template>
      <xsl:text>)
      {</xsl:text>
    </xsl:if>
    <xsl:for-each select="./cond">
      <xsl:text>
    {
        OOXMLPropertySetEntryToInteger aHandler(</xsl:text>
        <xsl:call-template name="idtoqname">
          <xsl:with-param name="id" select="@tokenid"/>
        </xsl:call-template>
        <xsl:text>);
        dynamic_cast&lt;OOXMLFastContextHandlerStream*&gt;(pHandler)-&gt;getPropertySetAttrs()-&gt;resolve(aHandler);
        
        if (sal::static_int_cast&lt;Id&gt;(aHandler.getValue()) == </xsl:text>
        <xsl:call-template name="idtoqname">
          <xsl:with-param name="id" select="@value"/>
        </xsl:call-template>
        <xsl:text>)
        {</xsl:text>
    </xsl:for-each>
    <xsl:choose>
        <xsl:when test="@action='handleXNotes' or 
            @action='handleHdrFtr' or 
            @action='handleComment' or 
            @action='handlePicture' or 
            @action='handleBreak' or 
            @action='handleOLE'">
            <xsl:text>
    dynamic_cast&lt;OOXMLFastContextHandlerProperties*&gt;(pHandler)-&gt;</xsl:text>
            <xsl:value-of select="@action"/>
            <xsl:text>();</xsl:text>
        </xsl:when>
        <xsl:when test="@action='propagateCharacterPropertiesAsSet'">
          <xsl:text>
    pHandler-&gt;propagateCharacterPropertiesAsSet(</xsl:text>
    <xsl:call-template name="idtoqname">
      <xsl:with-param name="id" select="@sendtokenid"/>
    </xsl:call-template>
    <xsl:text>);</xsl:text>
        </xsl:when>
        <xsl:when test="@action='startCell' or @action='endCell'">
            <xsl:text>
    dynamic_cast&lt;OOXMLFastContextHandlerTextTableCell*&gt;(pHandler)-&gt;</xsl:text>
            <xsl:value-of select="@action"/>
            <xsl:text>();</xsl:text>
        </xsl:when>
        <xsl:when test="@action='sendProperty' or @action='handleHyperlink'">
            <xsl:text>
    dynamic_cast&lt;OOXMLFastContextHandlerStream*&gt;(pHandler)-&gt;</xsl:text>
            <xsl:value-of select="@action"/>
            <xsl:text>();</xsl:text>
        </xsl:when>
        <xsl:when test="@action='fieldstart'">
            <xsl:text>
    pHandler-&gt;startField();</xsl:text>
        </xsl:when>
        <xsl:when test="@action='fieldsep'">
            <xsl:text>
    pHandler-&gt;fieldSeparator();</xsl:text>
        </xsl:when>
        <xsl:when test="@action='fieldend'">
            <xsl:text>
    pHandler-&gt;endField();</xsl:text>
        </xsl:when>
        <xsl:when test="@action='printproperty'">
          <xsl:text>
    dynamic_cast&lt;OOXMLFastContextHandlerStream*&gt;(pHandler)-&gt;sendProperty(</xsl:text>
        <xsl:call-template name="idtoqname">
          <xsl:with-param name="id" select="@sendtokenid"/>
        </xsl:call-template>
        <xsl:text>);</xsl:text>
        </xsl:when>
        <xsl:when test="@action='sendPropertiesWithId'">
          <xsl:text>
    pHandler-&gt;sendPropertiesWithId(</xsl:text>
          <xsl:call-template name="idtoqname">
            <xsl:with-param name="id" select="@sendtokenid"/>
          </xsl:call-template>
          <xsl:text>);</xsl:text>
        </xsl:when>
        <xsl:when test="@action='text'">
    pHandler-&gt;text(sText);
        </xsl:when>
        <xsl:when test="@action='newProperty'">
          <xsl:text>
    OOXMLFastHelper&lt;OOXMLIntegerValue&gt;::newProperty(pHandler, </xsl:text>
    <xsl:call-template name="idtoqname">
      <xsl:with-param name="id" select="@tokenid"/>
    </xsl:call-template>
    <xsl:text>, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("</xsl:text>
    <xsl:value-of select="@value"/>
    <xsl:text>")));</xsl:text>
        </xsl:when>
        <xsl:when test="@action='mark'">
          <xsl:text>
    OOXMLFastHelper&lt;OOXMLIntegerValue&gt;::mark(pHandler, </xsl:text>
    <xsl:call-template name="idtoqname">
      <xsl:with-param name="id" select="@sendtokenid"/>
    </xsl:call-template>
    <xsl:text>, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("</xsl:text>
    <xsl:value-of select="@value"/>
    <xsl:text>")));</xsl:text>
        </xsl:when>
        <xsl:when test="@action='tokenproperty'">
          <xsl:text>
    OOXMLFastHelper&lt;OOXMLIntegerValue&gt;::newProperty(pHandler, </xsl:text>
    <xsl:call-template name="idtoqname">
      <xsl:with-param name="id">ooxml:token</xsl:with-param>
    </xsl:call-template>
    <xsl:text>, pHandler-&gt;getToken());</xsl:text>
        </xsl:when>
        <xsl:otherwise>
            <xsl:text>
    pHandler-&gt;</xsl:text>
        <xsl:value-of select="@action"/>
        <xsl:text>();</xsl:text>
        </xsl:otherwise>
    </xsl:choose>
    <xsl:for-each select="./cond">
      <xsl:text>
        }
    }</xsl:text>
    </xsl:for-each>
    <xsl:if test="@tokenid">
      <xsl:text>
    }</xsl:text>
    </xsl:if>
  </xsl:template>

<!-- factoryaction -->
<xsl:template name="factoryaction">
    <xsl:param name="action"/>
    <xsl:text>
void </xsl:text>
    <xsl:call-template name="factoryclassname"/>
    <xsl:text>::</xsl:text>
    <xsl:value-of select="$action"/>
    <xsl:text>Action(OOXMLFastContextHandler * pHandler</xsl:text>
    <xsl:if test="$action='characters'">
        <xsl:text>, const ::rtl::OUString &amp; sText</xsl:text>
    </xsl:if>
    <xsl:text>)
{
    switch (pHandler->getDefine())
    {</xsl:text>
    <xsl:for-each select="resource[action/@name=$action]">
        <xsl:text>
    </xsl:text>
        <xsl:call-template name="caselabeldefine"/>
        <xsl:for-each select="action[@name=$action]">
            <xsl:call-template name="factorychooseaction"/>
        </xsl:for-each>
        <xsl:text>
        break;</xsl:text>
    </xsl:for-each>
    <xsl:text>
    default:
        break;
    }
}</xsl:text>
</xsl:template>

<!-- Key: actions" -->
<xsl:key name="actions" match="/model/namespace/resource/action"
    use="@name"/>

<!-- factoryactions -->
<xsl:template name="factoryactions">
    <xsl:variable name="ns" select="@name"/>
    <xsl:for-each select="resource/action">
        <xsl:sort select="@name"/>
        <xsl:if test="generate-id(key('actions', @name)[ancestor::namespace/@name=$ns][1]) = generate-id(.)">
            <xsl:variable name="name" select="@name"/>
            <xsl:for-each select="ancestor::namespace">
            <xsl:call-template name="factoryaction">
                <xsl:with-param name="action" select="$name"/>
            </xsl:call-template>
            </xsl:for-each>
        </xsl:if>
    </xsl:for-each>
</xsl:template>

<xsl:template name="factoryactiondecls">
    <xsl:variable name="ns" select="@name"/>
    <xsl:for-each select="resource/action">
        <xsl:sort select="@name"/>
        <xsl:if test="generate-id(key('actions', @name)[ancestor::namespace/@name=$ns][1]) = generate-id(.)">
            <xsl:text>
    void </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>Action(OOXMLFastContextHandler * pHandler</xsl:text>
            <xsl:if test="@name='characters'">
                <xsl:text>, const ::rtl::OUString &amp; sText</xsl:text>
            </xsl:if>
            <xsl:text>);</xsl:text>
        </xsl:if>
    </xsl:for-each>
</xsl:template>

<!-- factoryclassname -->
<xsl:template name="factoryclassname">
    <xsl:text>OOXMLFactory_</xsl:text>
    <xsl:value-of select="translate(@name, '-', '_')"/>
</xsl:template>

<!-- factorydecl -->
<xsl:template name="factorydecl">
    <xsl:variable name="classname">
        <xsl:call-template name="factoryclassname"/>
    </xsl:variable>
    <xsl:text>
class </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text> : public OOXMLFactory_ns
{
public:
    typedef boost::shared_ptr &lt; OOXMLFactory_ns &gt; Pointer_t;
    
    static Pointer_t getInstance();
    
    virtual AttributeToResourceMapPointer createAttributeToResourceMap(Id nId);
    virtual ListValueMapPointer createListValueMap(Id nId);
    virtual CreateElementMapPointer createCreateElementMap(Id nId);
    virtual TokenToIdMapPointer createTokenToIdMap(Id nId);
    virtual string getDefineName(Id nId) const;</xsl:text>
    <xsl:call-template name="factoryactiondecls"/>
    <xsl:text>
    
    virtual ~</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>();
    
protected:
    static Pointer_t m_pInstance;

    </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>();
};
    </xsl:text>
</xsl:template>

<xsl:template name="factoryincludes">
    <xsl:for-each select="/model/namespace">
        <xsl:text>
#include "OOXMLFactory_</xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>.hxx"</xsl:text>
    </xsl:for-each>
</xsl:template>

<xsl:template name="factoryfornamespace">
    <xsl:text>
OOXMLFactory_ns::Pointer_t OOXMLFactory::getFactoryForNamespace(Id nId)
{
    OOXMLFactory_ns::Pointer_t pResult;
    
    switch (nId &amp; 0xffff0000)
    {</xsl:text>
    <xsl:for-each select="/model/namespace">
        <xsl:text>
    case </xsl:text>
        <xsl:call-template name="idfornamespace"/>
        <xsl:text>:
        pResult = </xsl:text>
        <xsl:call-template name="factoryclassname"/>
        <xsl:text>::getInstance();
        break;</xsl:text>
    </xsl:for-each>
    <xsl:text>
        default:
            break;
    }
    
    return pResult;
}
</xsl:text>
</xsl:template>

<xsl:template name="factorynamespaceiddecls">
    <xsl:for-each select="/model/namespace">
        <xsl:sort select="@name"/>
        <xsl:text>
extern const Id </xsl:text>
        <xsl:call-template name="idfornamespace"/>
        <xsl:text>;</xsl:text>
    </xsl:for-each>
</xsl:template>

<xsl:template name="factorynamespaceidimpls">
    <xsl:for-each select="/model/namespace">
        <xsl:sort select="@name"/>
        <xsl:text>
const Id </xsl:text>
        <xsl:call-template name="idfornamespace"/>
        <xsl:text> = </xsl:text>
        <xsl:value-of select="position()"/>
        <xsl:text> &lt;&lt; 16;</xsl:text>
    </xsl:for-each>
</xsl:template>

<xsl:key name="definename" match="//rng:define" use="@name"/>

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

<xsl:template name="factorymutex">
    <xsl:call-template name="factoryclassname"/>
    <xsl:text>_Mutex</xsl:text>
</xsl:template>

<xsl:template name="factorymutexdecl">
    <xsl:variable name="classname">
        <xsl:call-template name="factoryclassname"/>
    </xsl:variable>
    <xsl:text>
typedef rtl::Static &lt; osl::Mutex, </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text> &gt; </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>_Mutex;
</xsl:text>
</xsl:template>

<xsl:template name="factoryconstructor">
    <xsl:variable name="classname">
        <xsl:call-template name="factoryclassname"/>
    </xsl:variable>
    <xsl:text>
</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>::</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>()
{
    // multi-thread-safe mutex for all platforms
    
    osl::MutexGuard aGuard(</xsl:text>
    <xsl:call-template name="factorymutex"/>
    <xsl:text>::get());
}
</xsl:text>
</xsl:template>

<xsl:template name="factorydestructor">
    <xsl:variable name="classname">
        <xsl:call-template name="factoryclassname"/>
    </xsl:variable>
    <xsl:text>
</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>::~</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>()
{
}
</xsl:text>
</xsl:template>

<xsl:template name="factorygetinstance">
    <xsl:variable name="classname">
        <xsl:call-template name="factoryclassname"/>
    </xsl:variable>
    <xsl:text>
OOXMLFactory_ns::Pointer_t </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>::m_pInstance;
    
OOXMLFactory_ns::Pointer_t </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>::getInstance()
{
    if (m_pInstance.get() == NULL)
        m_pInstance.reset(new </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>());
    
    return m_pInstance;
}
</xsl:text>
</xsl:template>

<xsl:key name="resources" match="/model/namespace/resource" use="@resource"/>

<xsl:template name="resources">
    <xsl:for-each select="/model/namespace/resource">
        <xsl:if test="generate-id(key('resources', @resource)[1])=generate-id(.)">
            <xsl:text>RT_</xsl:text>
            <xsl:value-of select="@resource"/>
            <xsl:text>,&#xa;</xsl:text>
        </xsl:if>
    </xsl:for-each>
</xsl:template>

<xsl:template name="factorycreatecontextfromfactory">
    <xsl:text>
uno::Reference&lt; xml::sax::XFastContextHandler &gt; OOXMLFactory::createFastChildContextFromFactory
(OOXMLFastContextHandler * pHandler, OOXMLFactory_ns::Pointer_t pFactory, Token_t Element)
{
    uno::Reference &lt; xml::sax::XFastContextHandler &gt; aResult;
    Id nDefine = pHandler->getDefine();
    
    if (pFactory.get() != NULL)
    {
        CreateElementMapPointer pMap = pFactory-&gt;getCreateElementMap(nDefine);
        TokenToIdMapPointer pTokenMap = pFactory-&gt;getTokenToIdMap(nDefine);
        
        if (pMap.get() != NULL)
        {
            Id nId = (*pTokenMap)[Element];
#ifdef DEBUG_CREATE
            string sDefine(pFactory->getDefineName(nDefine));
            string sElement(fastTokenToId(Element));
            string sQName((*QNameToString::Instance())(nId));
            
            debug_logger->startElement("createFastChildContextFromFactory");            
            debug_logger->attribute("define", sDefine);
            debug_logger->attribute("element", sElement);
            debug_logger->attribute("qname", sQName);
            
            static char buffer[16];
            snprintf(buffer, sizeof(buffer), "0x%08" SAL_PRIuUINT32, nId);
            debug_logger->attribute("idnum", buffer);
#endif
        
            CreateElement aCreateElement = (*pMap)[Element];
            
            switch (aCreateElement.m_nResource)
            {</xsl:text>
            <xsl:for-each select="/model/namespace/resource">
                <xsl:if test="generate-id(key('resources', @resource)) = generate-id(.)">
                    <xsl:if test="not(@resource = 'Hex' or 
                                      @resource = 'Integer' or 
                                      @resource = 'Boolean' or
                                      @resource = 'List' or
                                      @resource = 'String')">
                        <xsl:text>
            case RT_</xsl:text>
                        <xsl:value-of select="@resource"/>
                        <xsl:text>:
                aResult.set(OOXMLFastHelper&lt;OOXMLFastContextHandler</xsl:text>
                        <xsl:value-of select="@resource"/>
                        <xsl:text>&gt;::createAndSetParentAndDefine(pHandler, Element, (*pTokenMap)[Element], aCreateElement.m_nId));
                break;</xsl:text>
                    </xsl:if>
                </xsl:if>
            </xsl:for-each>
            <xsl:text>
            default:
                break;
            }

#ifdef DEBUG_CREATE
            debug_logger->endElement("createFastChildContextFromFactory");        
#endif
        }
    }
    
    return aResult;
}
</xsl:text>
</xsl:template>

<xsl:template name="factorycreatefromstart">
    <xsl:text>
uno::Reference&lt; xml::sax::XFastContextHandler &gt; OOXMLFactory::createFastChildContextFromStart
(OOXMLFastContextHandler * pHandler, Token_t Element)
{
    uno::Reference &lt; xml::sax::XFastContextHandler &gt; aResult;
    OOXMLFactory_ns::Pointer_t pFactory;    
    
</xsl:text>
    <xsl:for-each select="/model/namespace">
        <xsl:text>
    if (! aResult.is())
    {
        pFactory = getFactoryForNamespace(</xsl:text>
        <xsl:call-template name="idfornamespace"/>
        <xsl:text>);
        aResult.set(createFastChildContextFromFactory(pHandler, pFactory, Element));
    }</xsl:text>
    </xsl:for-each>
    <xsl:text>
    
    return aResult;
}
</xsl:text>
</xsl:template>

<xsl:template name="factorygetdefinename">
    <xsl:text>
string </xsl:text>
    <xsl:call-template name="factoryclassname"/>
    <xsl:text>::getDefineName(Id nId) const
{
    static IdToStringMapPointer pMap;
    
    if (pMap.get() == NULL)
    {
        pMap = IdToStringMapPointer(new IdToStringMap());
        
</xsl:text>
        <xsl:for-each select="rng:grammar/rng:define">
            <xsl:text>
        (*pMap)[</xsl:text>
            <xsl:call-template name="idfordefine"/>
            <xsl:text>] = "</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>";</xsl:text>
        </xsl:for-each>
    }
    
    return (*pMap)[nId];
}
    
</xsl:template>

</xsl:stylesheet>
