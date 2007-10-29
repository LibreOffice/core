<!--
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: resourcestools.xsl,v $
 *
 *  $Revision: 1.43 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-29 15:30:15 $
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
    xmlns:xalan="http://xml.apache.org/xalan"
    xmlns:UML = 'org.omg.xmi.namespace.UML' 
    exclude-result-prefixes = "xalan"
    xml:space="default">
  <xsl:output method="text" />

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

  <!-- 
       Returns <define> for the current <ref>. 

       The current node must be a <ref>!
  -->
  <xsl:template name="defineforref">
    <xsl:variable name="mygrammarid" select="generate-id(ancestor::rng:grammar)"/>
    <xsl:value-of select="key('defines-with-name', @name)[generate-id(ancestor::rng:grammar) = $mygrammarid]"/>
  </xsl:template>

  <!-- 
       Generates a context name for the current node.

       The name is determined by the @name attribute of the current
       node. A unique <define> with the name @name is searched. If
       there is a unique one contextnamefordefine is used to determine
       the context name. If there are multiple <define>s with name
       @name the one with the same application as as the current node
       determines the context name.

       The application of the current node is the @application
       attribute of the ancestor <rng:grammar>.
  -->
  <xsl:template name="contextnameforname">
    <xsl:variable name="application" select="ancestor::rng:grammar/@application"/>
    <xsl:variable name="name" select="@name"/>
    <xsl:choose>
      <xsl:when test="count(key('defines-with-name', @name)) = 1">
        <xsl:for-each select="key('defines-with-name', @name)">
          <xsl:call-template name="contextnamefordefine"/>
        </xsl:for-each>
      </xsl:when>
      <xsl:otherwise>
        <xsl:for-each select="key('defines-with-application', $application)[@name=$name]">
          <xsl:call-template name="contextnamefordefine"/>
        </xsl:for-each>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!--
      Generates statement to generate a context for the current node.

      Uses contextnameforname.
  -->
  <xsl:template name="contextnew">
    <xsl:text> new </xsl:text>
    <xsl:call-template name="contextnameforname"/>
    <xsl:text>(*this)</xsl:text>
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

  <!--
      Create name for element enumeration value.

      The result is
      
        OOXML_ELEMENT_<name>

      where all occurrences of ':' are replaced by '_'.

     @param name the name of the element
  -->
  <xsl:template name="elementname">
    <xsl:param name="name"/>
    <xsl:text>OOXML_ELEMENT_</xsl:text>
    <xsl:value-of select="translate($name, ':', '_')"/>
  </xsl:template>

  <!--
      Create name for attribute enumeration value.

      The result is
      
         OOXML_ATTRIBUTE_<name>

      where all occurrences of ':' are replaced by '_'.
  -->
  <xsl:template name="attrname">
    <xsl:param name="name"/>
    <xsl:text>OOXML_ATTRIBUTE_</xsl:text>
    <xsl:value-of select="translate($name, ':', '_')"/>
  </xsl:template>

  <!--
      Create entries in enum definition for elements.

      For each occurrence of rng:element an entry 

               OOXML_ELEMENT_<name> 

      is generated, but only if the element is the first named <name>.
  -->
  <xsl:template name="enumelement">
    <xsl:param name="prefix"/>
    <xsl:for-each select=".//rng:element[@enumname]">
      <xsl:if test="generate-id(.) = generate-id(key('same-element-enum', @enumname)[1])">
        <xsl:call-template name="elementname">
          <xsl:with-param name="name" select="@enumname"/>
        </xsl:call-template>
        <xsl:text>, &#xa; </xsl:text>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>

  <!--
      Create entries in enum definition for attributes.

      For each occurrence of rng:attribute an entry

               OOXML_ATTRIBUTE_<name>

      is generated, but only if the attribute is the first named <name>.
  -->
  <xsl:template name="enumattribute">
    <xsl:param name="prefix"/>
    <xsl:for-each select=".//rng:attribute[@enumname]">
      <xsl:if test="generate-id(.) = generate-id(key('same-attribute-enum', @enumname)[1])">
        <xsl:call-template name="attrname">
          <xsl:with-param name="name" select="@enumname"/>
        </xsl:call-template>
        <xsl:text>, &#xa;</xsl:text>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>

  <!--
      Create entry in reverse element map of TokenMap.
  -->
  <xsl:template name="hashelement">
    <xsl:param name="prefix"/>
    <xsl:for-each select=".//rng:element[@enumname]">
      <xsl:choose>
        <xsl:when test="generate-id(.) = generate-id(key('same-element-enum', @enumname)[1])">
          <xsl:variable name="element">
            <xsl:call-template name="elementname">
              <xsl:with-param name="name" select="@enumname"/>
            </xsl:call-template>
          </xsl:variable>
          <xsl:text>
    {
        rtl::OUString aStr(RTL_CONSTASCII_USTRINGPARAM("</xsl:text>
        <xsl:value-of select="@qname"/>
          <xsl:text>"));</xsl:text>
          <xsl:text>
        mElementReverseMap[</xsl:text>
        <xsl:value-of select="$element"/>
        <xsl:text>] = aStr;
    } &#xa;</xsl:text>
        </xsl:when>
      </xsl:choose>
    </xsl:for-each>
  </xsl:template>

  <!-- 
       Create entry in reverse attribute map of TokenMap.
  -->
  <xsl:template name="hashattribute">
    <xsl:param name="prefix"/>
    <xsl:for-each select=".//rng:attribute[@name]">
      <xsl:choose>
        <xsl:when test="generate-id(.) = generate-id(key('same-attribute-enum', @enumname)[1])">
          <xsl:variable name="attribute">
            <xsl:call-template name="attrname">
              <xsl:with-param name="name" select="@enumname"/>
            </xsl:call-template>
          </xsl:variable>
          <xsl:text>
    {
        rtl::OUString aStr(RTL_CONSTASCII_USTRINGPARAM("</xsl:text>
          <xsl:value-of select="@qname"/>
          <xsl:text>"));</xsl:text>
          <xsl:text>
        mAttributeReverseMap[</xsl:text>
          <xsl:value-of select="$attribute"/>
          <xsl:text>] = aStr;        
    } &#xa;</xsl:text>
        </xsl:when>
      </xsl:choose>
    </xsl:for-each>
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
      Generates context name.

      The resulting name is

          OOXMLContext_$prefix_$name

      @param prefix
      @param name
  -->
  <xsl:template name="contextname">
    <xsl:param name="prefix"/>
    <xsl:param name="name"/>
    <xsl:text>OOXMLContext_</xsl:text>
    <xsl:value-of select="$prefix"/>
    <xsl:text>_</xsl:text>
    <xsl:value-of select="$name"/>
  </xsl:template>

  <!--
      Returns the context name for the current <define> node.

      The current node has to be the <define> node.
  -->
  <xsl:template name="contextnamefordefine">
    <xsl:variable name="myprefix">
      <xsl:apply-templates select="." mode="grammar-prefix"/>
    </xsl:variable>
    <xsl:call-template name="contextname">
      <xsl:with-param name="prefix" select="$myprefix"/>
      <xsl:with-param name="name" select="@name"/>
    </xsl:call-template>
  </xsl:template>

  <!--
      Returns the value of the @resource attribute of the <resource>
      node according to the current <define>.
  -->
  <xsl:template name="contextresource">
    <xsl:variable name="mynsid" select="generate-id(ancestor::namespace)"/>
    <xsl:value-of select="key('context-resource', @name)[generate-id(ancestor::namespace)=$mynsid]/@resource"/>
  </xsl:template>

  <!--
      Generates contructor declaration for a <define>.
  -->
  <xsl:template name="contextconstructordecl">
    <xsl:variable name="classname">
      <xsl:call-template name="contextnamefordefine"/>
    </xsl:variable>
    <xsl:value-of select="$classname"/>
    <xsl:text>(const OOXMLContext &amp; rContext);&#xa;</xsl:text>
  </xsl:template>

  <!--
      Returns the parent class for the context class representing the
      current <define>.
  -->
  <xsl:template name="contextparent">
    <xsl:variable name="resource">
      <xsl:call-template name="contextresource"/>
    </xsl:variable>
    <xsl:text>OOXMLContext</xsl:text>
    <xsl:value-of select="$resource"/>
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
    <xsl:variable name="myprefix">
      <xsl:apply-templates select="." mode="grammar-prefix"/>
    </xsl:variable>
    <xsl:call-template name="valuename">
      <xsl:with-param name="prefix" select="$myprefix"/>
      <xsl:with-param name="name" select="@name"/>
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
      Generates declaration of a context class.

      Precondition: <resource> for current <define> indicates that
      class is directly or indirectly derived from OOXMLContext.
  -->
  <xsl:template name="contextdecl">
    <xsl:variable name="classname">
      <xsl:call-template name="contextnamefordefine"/>
    </xsl:variable>
    <xsl:variable name="resource">
      <xsl:call-template name="contextresource"/>
    </xsl:variable>
    <xsl:text>
class </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text> : public </xsl:text>
    <xsl:call-template name="contextparent"/>
    <xsl:text>
{
public:
    </xsl:text>
    <xsl:call-template name="contextconstructordecl"/>
    <xsl:text>
    virtual </xsl:text>
    <xsl:text>~</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>();&#xa;</xsl:text>
    <xsl:text>
    virtual OOXMLContext::Pointer_t element(TokenEnum_t nToken);
    virtual OOXMLContext::Pointer_t elementFromRefs(TokenEnum_t nToken);
    virtual bool lcl_attribute(TokenEnum_t nToken, 
                               const rtl::OUString &amp; rValue);
    virtual doctok::Id getId(TokenEnum_t nToken);
    virtual doctok::Id getIdFromRefs(TokenEnum_t nToken);
    virtual void lcl_characters(const rtl::OUString &amp; str); 
    virtual void startAction();
    virtual void endAction();

    virtual string getType() const { return "</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>"; }</xsl:text>
    <xsl:text>
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
    <xsl:variable name="name" select="@name"/>
    <xsl:choose>
      <xsl:when test="ancestor::namespace//start[@name=$name]">1</xsl:when>
      <xsl:when test="(starts-with(@name, 'CT_') or starts-with(@name, 'EG_'))">1</xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
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

  <!--
      Generates declarations of classes (derived from OOXMLContext and 
      OOXMLValue).
  -->
  <xsl:template name="contextdecls">
    <xsl:param name="prefix"/>
    <xsl:for-each select=".//rng:define">
      <xsl:variable name="do">
        <xsl:call-template name="classfordefine"/>
      </xsl:variable>
      <xsl:if test="$do = '1'">
        <xsl:call-template name="contextdecl"/>
      </xsl:if>
      <xsl:variable name="dovalue">
        <xsl:call-template name="valuefordefine"/>
      </xsl:variable>
      <xsl:if test="$dovalue = '1'">
        <xsl:call-template name="valuedecl"/>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>

  <!--
      Generates the inner of OOXMLContext...::elementByRef.
  -->
  <xsl:template name="refsimplondefine">
    <xsl:variable name="do">
      <xsl:call-template name="classfordefine">
      </xsl:call-template>
    </xsl:variable>
    <xsl:if test="$do = '1'">
      <xsl:text>
    {       
        OOXMLContext::Pointer_t pSubContext = OOXMLContext::Pointer_t
            (</xsl:text>
      <xsl:call-template name="contextnew"/>
      <xsl:text>);</xsl:text>
        OOXMLContext::Pointer_t pContext(pSubContext->element(nToken));

        if (pContext.get() != NULL)
            return pContext;
     }
    </xsl:if>
  </xsl:template>

  <!--
      Generates definition of getIdFromRefs for the current <define>.
  -->
  <xsl:template name="contextrefsidimpl">
    <xsl:param name="prefix"/>
    <xsl:variable name="classname">
      <xsl:call-template name="contextnamefordefine"/>
    </xsl:variable>
    <xsl:text>
doctok::Id </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>::getIdFromRefs(TokenEnum_t nToken)
{
    TokenEnum_t tmpToken;
    tmpToken = nToken; // prevent warning</xsl:text>
   <xsl:variable name="myid" select="generate-id(.)"/>
    <xsl:for-each select=".//rng:ref[not(ancestor::rng:element or ancestor::rng:attribute) and @name]">
      <xsl:if test="generate-id(.) != $myid">
        <xsl:variable name="mydefine" select="key('defines-with-name', @name)"/>
        <xsl:for-each select="$mydefine">
          <xsl:call-template name="refsidimplondefine"/>
        </xsl:for-each>
      </xsl:if>
    </xsl:for-each>    
    <xsl:text>
    return 0x0;
}
    </xsl:text>
  </xsl:template>

  <!--
      Generates definition of elementFromRefs for the current <define>.
  -->
  <xsl:template name="contextrefsimpl">
    <xsl:param name="prefix"/>
    <xsl:variable name="classname">
      <xsl:call-template name="contextnamefordefine"/>
    </xsl:variable>
    <xsl:text>
OOXMLContext::Pointer_t </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>::elementFromRefs(TokenEnum_t nToken)
{
    TokenEnum_t tmpToken;
    tmpToken = nToken; // prevent warning
    </xsl:text>
   <xsl:variable name="myid" select="generate-id(.)"/>
    <xsl:for-each select=".//rng:ref[not(ancestor::rng:element or ancestor::rng:attribute) and @name]">
      <xsl:if test="generate-id(.) != $myid">
        <xsl:variable name="mydefine" select="key('defines-with-name', @name)"/>
        <xsl:for-each select="$mydefine">
          <xsl:call-template name="refsimplondefine"/>
        </xsl:for-each>
      </xsl:if>
    </xsl:for-each>    
    <xsl:text>
    return OOXMLContext::Pointer_t();
}
    </xsl:text>
  </xsl:template>

  <!--
      Generates the inner of getIdFromRefs for the current <define>.
  -->
  <xsl:template name="refsidimplondefine">
    <xsl:variable name="do">
      <xsl:call-template name="classfordefine">
      </xsl:call-template>
    </xsl:variable>
    <xsl:if test="$do = '1'">
      <xsl:text>
    {       
        OOXMLContext::Pointer_t pSubContext = OOXMLContext::Pointer_t
            (</xsl:text>
      <xsl:call-template name="contextnew"/>
      <xsl:text>);</xsl:text>
        doctok::Id nId = pSubContext->getId(nToken);

        if (nId != 0x0)
            return nId;
     }
    </xsl:if>
  </xsl:template>

  <!--
      Generates the definition of lcl_characters for the current <define>.
  -->
  <xsl:template name="contextcharactersimpl">
    <xsl:variable name="classname">
      <xsl:call-template name="contextnamefordefine"/>
    </xsl:variable>
    <xsl:variable name="mynsid" select="generate-id(ancestor::namespace)"/>
    <xsl:variable name="resource" select="key('context-resource', @name)[generate-id(ancestor::namespace) = $mynsid]"/>
    <xsl:text>
void </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>::lcl_characters(const rtl::OUString &amp; sText) 
{
</xsl:text>
    <xsl:choose>
      <xsl:when test="$resource/action[@name='characters' and @action='text']">
        <xsl:text>
    mrStream.utext(reinterpret_cast &lt; const sal_uInt8 * &gt; 
                   (sText.getStr()), 
                   sText.getLength());</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="contextparent"/>
        <xsl:text>::lcl_characters(sText);</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:text>
}</xsl:text>
  </xsl:template>

  <!--
      Generates an element case statement.

      Precondition: the current node is a <rng:element> node in a
      <rng:define>.
  -->
  <xsl:template name="caselabelelement">
          <xsl:text>
     case </xsl:text>
           <xsl:call-template name="elementname">
             <xsl:with-param name="name" select="@enumname"/>
           </xsl:call-template>
           <xsl:text>:</xsl:text>
  </xsl:template>

  <!--
      Generates an attribute case statement.

      Precondition the current node is a <rng:attribute> node in a
      <rng:define>
  -->
  <xsl:template name="caselabelattribute">
          <xsl:text>
     case </xsl:text>
           <xsl:call-template name="attrname">
             <xsl:with-param name="name" select="@enumname"/>
           </xsl:call-template>
           <xsl:text>:</xsl:text>
  </xsl:template>

  <!--
      Generates definition of method element for current <define>.
  -->
  <xsl:template name="contextelementimpl">
    <xsl:param name="prefix"/>
    <xsl:variable name="classname">
      <xsl:call-template name="contextnamefordefine"/>
    </xsl:variable>
    <xsl:variable name="resource">
      <xsl:call-template name="contextresource"/>
    </xsl:variable>
    <xsl:variable name="mydefine" select="@name"/>
    <xsl:variable name="switchbody">
      <!--
          This variable contains the body of the switch over the given token.

          For each <rng:element> in <define> that contains a <rng:ref>
          and has an attribute @enumname a case is generated. In the
          case a context for the according OOXMLContext... is created
          and assigned as the result of the method.

          Exceptions:

          - For <rng:ref>s to BUILT_IN_ANY_TYPE OOXMLContext is used
            to create the context.
            
          - If classfordefine indicates that no class will be
            generated for <rng:ref> nothing will hapen and the result
            is null.
      -->
      <xsl:for-each select=".//rng:element[@enumname and rng:ref]">
        <xsl:variable name="do">
          <xsl:for-each select="key('defines-with-name', ./rng:ref/@name)[1]">
            <xsl:call-template name="classfordefine"/>
          </xsl:for-each>
        </xsl:variable>
        <xsl:if test="$do = '1'">
          <xsl:variable name="myelement" select="@name"/>
          <xsl:call-template name="caselabelelement"/>
          <xsl:choose>
            <xsl:when test="./rng:ref/@name='BUILT_IN_ANY_TYPE'">
              <xsl:text>
        {
            OOXMLContext aContext(*this);
            pResult = aContext.element(nToken);
        }
        break;</xsl:text>
               </xsl:when>
               <xsl:otherwise>
           <xsl:text>
        {
            pResult = OOXMLContext::Pointer_t
               (</xsl:text>
               <xsl:for-each select="./rng:ref">
                 <xsl:call-template name="contextnew"/>
               </xsl:for-each>
           <xsl:text>);</xsl:text>
         <xsl:text>
         }
             break;</xsl:text>
               </xsl:otherwise>
             </xsl:choose>
         </xsl:if>
       </xsl:for-each>       
    </xsl:variable>
    <xsl:text>
OOXMLContext::Pointer_t
</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>::element(TokenEnum_t nToken)
{
    OOXMLContext::Pointer_t pResult;
    </xsl:text>
    <xsl:choose>
      <xsl:when test="string-length($switchbody) > 0">
        <xsl:text>
    switch (nToken)
    {</xsl:text>
    <xsl:value-of select="$switchbody"/>
       <xsl:text>
     case OOXML_TOKENS_END:
         break;
     default:
         pResult = elementFromRefs(nToken);
              break;
     }</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>
     pResult = elementFromRefs(nToken);</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
     <xsl:text>

    if (pResult.get() != NULL)
        pResult->setToken(nToken);

    return pResult;
}
     </xsl:text>
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

  <!--
      Generated the definition of getId for current <define>.
  -->
  <xsl:template name="contextgetid">
    <xsl:param name="prefix"/>
    <xsl:variable name="classname">
      <xsl:call-template name="contextnamefordefine"/>
    </xsl:variable>
    <xsl:variable name="resource">
      <xsl:call-template name="contextresource"/>
    </xsl:variable>
    <xsl:variable name="mydefine" select="@name"/>
    <xsl:variable name="switchbody">
      <!--
          Generates the body for the switch over the token id.
      -->
      <xsl:if test="$resource = 'Properties' or $resource = 'Property' or $resource = 'SingleElement'">
        <xsl:for-each select="//resource[@name=$mydefine]">
          <xsl:for-each select="attribute|sprm">
            <xsl:variable name="sprmname" select="@name"/>
        // <xsl:value-of select="$mydefine"/> : <xsl:value-of select="$sprmname"/>
            <xsl:for-each select="key('defines-with-name', $mydefine)//rng:element[@name=$sprmname]">
              <xsl:call-template name="caselabelelement"/>
            </xsl:for-each>
            <xsl:for-each select="key('defines-with-name', $mydefine)//rng:attribute[@name=$sprmname]">
              <xsl:call-template name="caselabelattribute"/>
            </xsl:for-each>
            <xsl:text>
        nResult = </xsl:text>
        <xsl:call-template name="processtokenid"/>
        <xsl:text>;
        break;</xsl:text>
          </xsl:for-each>
        </xsl:for-each>
      </xsl:if>
    </xsl:variable>
    <xsl:text>
doctok::Id
</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>::getId(TokenEnum_t nToken)
{
    doctok::Id nResult = 0x0;
    </xsl:text>
    <xsl:choose>
      <xsl:when test="string-length($switchbody) > 0">
        <xsl:text>
    switch (nToken)
    {</xsl:text>
    <xsl:value-of select="$switchbody"/>
    <xsl:text>
    case OOXML_TOKENS_END: // prevent warning
        break;
    default:
        nResult = getIdFromRefs(nToken);
        break;
    }</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>
    nResult = getIdFromRefs(nToken);</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:text>
    return nResult;  
}
    </xsl:text>
  </xsl:template>

  <!--
      Generates case block for current <rng:attribute> node.

      Precondition: The current <define> is labeled as "Boolean" by
      the according <resource>.      
  -->
 <xsl:template name="contextattributeimplbool">
    <xsl:call-template name="caselabelattribute"/>
    <xsl:for-each select=".//rng:ref">
      <xsl:text>
      {</xsl:text>
      <xsl:variable name="mynsid" select="generate-id(ancestor::namespace)"/>
      <xsl:for-each select="key('defines-with-name', @name)[generate-id(ancestor::namespace)=$mynsid]">
          <xsl:text>
          mbValue = </xsl:text>
          <xsl:call-template name="valuenamefordefine"/>
          <xsl:text>(rValue).getBool();
          bResult = true;
      }</xsl:text>
      </xsl:for-each>
    </xsl:for-each>
    <xsl:text>
      break;</xsl:text>
  </xsl:template>

  <!--      
      Generates case block for current <rng:attribute> node.

      Precondition: The current <define> is labeled as "Integer" by
      the according <resource>.      
  -->
  <xsl:template name="contextattributeimplint">
    <xsl:call-template name="caselabelattribute"/>
    <xsl:text>
        mnValue = rValue.toInt32();
        bResult = true;
        break;</xsl:text>
  </xsl:template>

  <!--
      Generates inner of case block for current <rng:attribute> node.
  -->
  <xsl:template name="contextattributeimplpropcaseinner">
    <xsl:variable name="mynsid" select="generate-id(ancestor::namespace)"/>
    <xsl:if test=".//rng:text">
      <xsl:text>
        {
            OOXMLValue::Pointer_t pVal(new OOXMLStringValue(rValue));
            newProperty(nToken, pVal);
        }</xsl:text>
    </xsl:if>
    <xsl:for-each select=".//rng:ref">
      <xsl:variable name="refclassname">
        <xsl:for-each select="key('defines-with-name', @name)[generate-id(ancestor::namespace) = $mynsid]">
          <xsl:call-template name="valuenamefordefine"/>
        </xsl:for-each>
      </xsl:variable>
      <xsl:text>
        {
          OOXMLValue::Pointer_t pVal(new </xsl:text>
          <xsl:value-of select="$refclassname"/>
          <xsl:text>(rValue));
          newProperty(nToken, pVal);

          bResult = true;
        }</xsl:text>
    </xsl:for-each>
  </xsl:template>

  <!--      
      Generates case block for current <rng:attribute> node.

      Precondition: The current <define> is labeled as "XNotes" by
      the according <resource>.      
  -->
  <xsl:template name="contextattributeimplxnote">
    <xsl:variable name="mynsid" select="generate-id(ancestor::namespace)"/>
    <xsl:variable name="definename" select="ancestor::rng:define/@name"/>
    <xsl:variable name="resource" select="key('context-resource', $definename)[generate-id(ancestor::namespace)=$mynsid]"/>
    <xsl:variable name="name" select="@name"/>
    <xsl:call-template name="caselabelattribute"/>
    <xsl:if test="$resource//attribute[@name=$name and @action='checkId']">
      <xsl:value-of select="@action"/>
      <xsl:text>
        checkId(rValue);</xsl:text>
    </xsl:if>
    <xsl:call-template name="contextattributeimplpropcaseinner"/>
    <xsl:text>
        bResult = true;
        break;</xsl:text>
  </xsl:template>

  <!--      
      Generates case block for current <rng:attribute> node.

      Precondition: The current <define> is labeled as "Hex" by
      the according <resource>.      
  -->
  <xsl:template name="contextattributeimplhex">
    <xsl:call-template name="caselabelattribute"/>
    <xsl:text>
        mnValue = rValue.toInt32(16);
        bResult = true;
        break;</xsl:text>
  </xsl:template>

  <!--      
      Generates case block for current <rng:attribute> node.

      Precondition: The current <define> is labeled as "String" by
      the according <resource>.      
  -->
   <xsl:template name="contextattributeimplstring">
    <xsl:call-template name="caselabelattribute"/>
    <xsl:text>
        msValue = OOXMLStringValue(rValue).getString();
        bResult = true;
        break;</xsl:text>
  </xsl:template>

  <!--      
      Generates case block for current <rng:attribute> node.

      Precondition: The current <define> is labeled as "List" by
      the according <resource>.      
  -->
 <xsl:template name="contextattributeimpllist">
    <xsl:call-template name="caselabelattribute"/>
    <xsl:for-each select=".//rng:ref">
      <xsl:text>
      {</xsl:text>
      <xsl:variable name="mynsid" select="generate-id(ancestor::namespace)"/>
      <xsl:for-each select="key('defines-with-name', @name)[generate-id(ancestor::namespace)=$mynsid]">
          <xsl:text>
          mnValue = </xsl:text>
          <xsl:call-template name="valuenamefordefine"/>
          <xsl:text>(rValue).getInt();
          bResult = true;
      }</xsl:text>
      </xsl:for-each>
    </xsl:for-each>
    <xsl:text>
      break;</xsl:text>
  </xsl:template>

  <xsl:template name="attributeproptype">
    <xsl:variable name="mynsid" select="generate-id(ancestor::namespace)"/>
    <xsl:variable name="name" select="@name"/>
    <xsl:variable name="resource" select="key('context-resource', @name)[generate-id(ancestor::namespace)=$mynsid]"/>
    <xsl:choose>
      <xsl:when test="$resource/sprm[@name=$name]">
        <xsl:text>OOXMLPropertyImpl::SPRM</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>OOXMLPropertyImpl::ATTRIBUTE</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!--      
      Generates case block for current <rng:attribute> node.

      Precondition: The current <define> is labeled as "Properties" by
      the according <resource>.      
  -->
  <xsl:template name="contextattributeimplprops">
    <xsl:variable name="mynsid" select="generate-id(ancestor::namespace)"/>
    <xsl:variable name="name" select="@name"/>
    <xsl:variable name="definename" select="ancestor::rng:define/@name"/>
    <xsl:variable name="resource" select="key('context-resource', $definename)[generate-id(ancestor::namespace)=$mynsid]"/>
    <xsl:variable name="proptype">
      <xsl:call-template name="attributeproptype"/>
    </xsl:variable>
    <xsl:call-template name="caselabelattribute"/>
    <xsl:call-template name="contextattributeimplpropcaseinner"/>
    <xsl:text>
      break;</xsl:text>
  </xsl:template>

  <!--
      Generates the inner of the switch over the given token id in the
      ::attribute method of the class for the current <define>.

      For each <rng:attribute> in the current <define> the according
      template generating the case block is called. Which template is
      called depends on the <resource>/@resource for the current
      <define>.
  -->
  <xsl:template name="contextattributeswitchbody">
    <xsl:variable name="resource">
      <xsl:call-template name="contextresource"/>
    </xsl:variable>
    <xsl:for-each select=".//rng:attribute[@name]">
      <xsl:choose>
        <xsl:when test="$resource = 'BooleanValue'">
          <xsl:call-template name="contextattributeimplbool"/>
        </xsl:when>
        <xsl:when test="$resource = 'IntegerValue'">
          <xsl:call-template name="contextattributeimplint"/>
        </xsl:when>
        <xsl:when test="$resource = 'HexValue'">
          <xsl:call-template name="contextattributeimplhex"/>
        </xsl:when>
        <xsl:when test="$resource = 'StringValue'">
          <xsl:call-template name="contextattributeimplstring"/>
        </xsl:when>
        <xsl:when test="$resource = 'ListValue'">
          <xsl:call-template name="contextattributeimpllist"/>
        </xsl:when>
        <xsl:when test="$resource = 'Properties'">
          <xsl:call-template name="contextattributeimplprops"/>
        </xsl:when>
        <xsl:when test="$resource = 'SingleElement'">
          <xsl:call-template name="contextattributeimplprops"/>
        </xsl:when>
        <xsl:when test="$resource = 'XNote'">
          <xsl:call-template name="contextattributeimplxnote"/>
        </xsl:when>
        <xsl:otherwise/>
      </xsl:choose>
    </xsl:for-each>
  </xsl:template>

  <!--
      Generates the code in ::lcl_attribute handling direct <rng:ref>s in
      the current define.

      Direct <rng:ref>s have a path from the <define> that does not
      contain neither <rng:attribute> nor <rng:element>.

      For each those <rng:ref>s an instance of the according context
      class is generated. The handling is then passed to the
      ::attribute method of that instance.
  -->
  <xsl:template name="contextattributerefs">
    <xsl:variable name="resource">
      <xsl:call-template name="contextresource"/>
    </xsl:variable>
      <xsl:for-each select="./rng:ref">
        <xsl:choose>
          <xsl:when test="ancestor::rng:attribute|ancestor::rng:element"/>
          <xsl:otherwise>
            <xsl:variable name="do">
              <xsl:call-template name="classfordefine"/>
            </xsl:variable>
            <xsl:if test="$do = '1'">
              <xsl:text>
    if (! bResult)
    {</xsl:text>
    <xsl:variable name="refresource">
      <xsl:call-template name="contextresource"/>
    </xsl:variable>
    <xsl:variable name="refclass">
      <xsl:choose>
        <xsl:when test="$refresource = 'Properties'">
          <xsl:text>OOXMLContextProperties</xsl:text>
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>OOXMLContext</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
      <xsl:text>
        </xsl:text>
        <xsl:value-of select="$refclass"/>
        <xsl:text>::Pointer_t pRefContext(</xsl:text>
        <xsl:call-template name="contextnew"/>
        <xsl:text>);
        pRefContext->setParentResource(getResource());
        </xsl:text>
        <xsl:if test="$refresource='Properties'">
          <xsl:choose>
            <xsl:when test="$resource = 'Properties'">
              <xsl:text>
        pRefContext->setPropertySet(mpPropertySet);</xsl:text>
            </xsl:when>
            <xsl:when test="$resource='Stream' or $resource='XNote'">
              <xsl:text>
        pRefContext->setPropertySet(getPropertySetAttrs());</xsl:text>
            </xsl:when>
          </xsl:choose>
        </xsl:if>
        <xsl:text>
        bResult = pRefContext->attribute(nToken, rValue);
        </xsl:text>
        <xsl:text>
    }</xsl:text>
            </xsl:if>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:for-each>
  </xsl:template>

  <!--
      Generates the definition of ::lcl_attribute for the current
      <define>.
  -->
  <xsl:template name="contextattributeimpl">
    <xsl:param name="prefix"/>
    <xsl:variable name="classname">
      <xsl:call-template name="contextnamefordefine"/>
    </xsl:variable>
    <xsl:variable name="switchbody">
      <xsl:call-template name="contextattributeswitchbody"/>
    </xsl:variable>
    <xsl:variable name="refshandling">
      <xsl:call-template name="contextattributerefs"/>
    </xsl:variable>
    <xsl:text>
bool </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:choose>
      <xsl:when test="string-length($switchbody) > 0 or string-length($refshandling) > 0">
    <xsl:text>::lcl_attribute(TokenEnum_t nToken, const rtl::OUString &amp; rValue)</xsl:text>
    </xsl:when>
    <xsl:otherwise>
    <xsl:text>::lcl_attribute(TokenEnum_t /*nToken*/, const rtl::OUString &amp; /*rValue*/)</xsl:text>
    </xsl:otherwise>
    </xsl:choose>
    <xsl:text>
{
    bool bResult = false;</xsl:text>
    <xsl:if test="string-length($switchbody) > 0">
      <xsl:text>
    switch (nToken)
    {</xsl:text>
    <xsl:value-of select="$switchbody"/>
    <xsl:text>
    default:
      ;
    }</xsl:text>
    </xsl:if>
    <xsl:value-of select="$refshandling"/>
    <xsl:text>
    
    return bResult;
}</xsl:text>
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
      <xsl:when test="ancestor::namespace//resource[@name=$name]//default">1</xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!--
      Generates the definition of the constructor for the context
      class for the current <define>.
  -->
  <xsl:template name="contextconstructorimpl">
    <xsl:param name="prefix"/>
    <xsl:variable name="me" select="node()"/>
    <xsl:variable name="classname">
      <xsl:call-template name="contextnamefordefine"/>
    </xsl:variable>
    <xsl:variable name="resource" select="key('context-resource', @name)"/>
    <xsl:value-of select="$classname"/>
    <xsl:text>::</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>(const OOXMLContext &amp; rContext)
: </xsl:text>
    <xsl:call-template name="contextparent"/>
    <xsl:text>(rContext)
{</xsl:text>
    <xsl:text>
}&#xa;</xsl:text>
  </xsl:template>

  <!--
      Chooses the action for the current <action> element.
  -->
  <xsl:template name="chooseaction">
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
      <xsl:when test="@action='ftnednref'">
        <xsl:text>
    if (isForwardEvents())        
        mrStream.utext(sFtnEdnRef, 1);</xsl:text>
      </xsl:when>
      <xsl:when test="@action='ftnednsep'">
        <xsl:text>
    if (isForwardEvents())        
        mrStream.utext(sFtnEdnSep, 1);</xsl:text>
      </xsl:when>
      <xsl:when test="@action='ftnedncont'">
        <xsl:text>
    if (isForwardEvents())        
        mrStream.utext(sFtnEdnCont, 1);</xsl:text>
      </xsl:when>
      <xsl:when test="@action='pgNum'">
        <xsl:text>
    if (isForwardEvents())        
        mrStream.utext(sPgNum, 1);</xsl:text>
      </xsl:when>
      <xsl:when test="@action='tab'">
        <xsl:text>
    if (isForwardEvents())        
        mrStream.utext(sTab, 1);</xsl:text>
      </xsl:when>
      <xsl:when test="@action='endOfParagraph'">
        <xsl:text>
    if (isForwardEvents())        
        mrStream.utext(sCR, 1);</xsl:text>
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
          <xsl:text>
    switch (meToken)
    {
    case OOXML_ELEMENT_wordprocessingml_footnoteReference:
        {
            OOXMLFootnoteHandler aFootnoteHandler(this);
            mpPropertySet->resolve(aFootnoteHandler);
        }
        break;
    case OOXML_ELEMENT_wordprocessingml_endnoteReference:
        {
            OOXMLEndnoteHandler aEndnoteHandler(this);
            mpPropertySet->resolve(aEndnoteHandler);
        }
        break;
    default:
        break;
    }</xsl:text>
        </xsl:when>
        <xsl:when test="@action='handleHdrFtr'">
          <xsl:text>
    switch (meToken)
    {
    case OOXML_ELEMENT_wordprocessingml_footerReference:
        {
            OOXMLFooterHandler aFooterHandler(this);
            mpPropertySet->resolve(aFooterHandler);
        }
        break;
    case OOXML_ELEMENT_wordprocessingml_headerReference:
        {
            OOXMLHeaderHandler aHeaderHandler(this);
            mpPropertySet->resolve(aHeaderHandler);
        }
        break;
    default:
        break;
    }</xsl:text>
        </xsl:when>
        <xsl:when test="@action='handleComment'">
    {
        OOXMLCommentHandler aCommentHandler(this);
        mpPropertySet->resolve(aCommentHandler);
    }
        </xsl:when>
    </xsl:choose>
  </xsl:template>

  <!--
      Generates the definition of ::startAction of the context class
      for the current <define>.
  -->
  <xsl:template name="contextstartimpl">
    <xsl:param name="prefix"/>
    <xsl:variable name="me" select="node()"/>
    <xsl:variable name="classname">
      <xsl:call-template name="contextnamefordefine"/>
    </xsl:variable>
    <xsl:variable name="resource" select="key('context-resource', @name)"/>
    <xsl:text>
void </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>::startAction()
{</xsl:text>
    <xsl:for-each select="$resource/action[@name='start']">
      <xsl:call-template name="chooseaction"/>
    </xsl:for-each>
    <xsl:if test="$resource/@resource='Properties'">
      <xsl:for-each select="$resource//attribute[@default]">
        /*<xsl:value-of select="@name"/>*/
        <xsl:variable name="attrname" select="@name"/>
        <xsl:variable name="enum" select="$me//rng:attribute[@name=$attrname]/@enumname"/>
        <xsl:variable name="token">
          <xsl:call-template name="attrname">
            <xsl:with-param name="name" select="$enum"/>
          </xsl:call-template>
        </xsl:variable>
        <xsl:text>
    attribute(</xsl:text>
    <xsl:value-of select="$token"/>
    <xsl:text>, </xsl:text>
    <xsl:call-template name="valuestringname">
      <xsl:with-param name="string" select="@default"/>
    </xsl:call-template>
    <xsl:text>);</xsl:text>
      </xsl:for-each>
    </xsl:if>
    <xsl:if test="$resource/@resource='Table'">
      <xsl:text>
    setId(</xsl:text>
    <xsl:call-template name="idtoqname">
      <xsl:with-param name="id" select="$resource/@tokenid"/>
    </xsl:call-template>
    <xsl:text>);</xsl:text>
    </xsl:if>    
    <xsl:text>
}</xsl:text>
  </xsl:template>

  <!--
      Genertes the definition of ::endAction of the context class for
      the current <define>.
  -->
  <xsl:template name="contextendimpl">
    <xsl:param name="prefix"/>
    <xsl:variable name="classname">
      <xsl:call-template name="contextnamefordefine"/>
    </xsl:variable>
    <xsl:variable name="resource" select="key('context-resource', @name)"/>
    <xsl:text>
void </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>::endAction()
{</xsl:text>
    <xsl:for-each select="$resource/action[@name='end']">
      <xsl:call-template name="chooseaction"/>
    </xsl:for-each>
    <xsl:text>
}</xsl:text>
  </xsl:template>

  <!--
      Generates the definition of the destructor of the context class
      for the current <define>.
  -->
  <xsl:template name="contextdestructorimpl">
    <xsl:param name="prefix"/>
    <xsl:variable name="classname">
      <xsl:call-template name="contextnamefordefine"/>
    </xsl:variable>
    <xsl:variable name="resource" select="key('context-resource', @name)"/>
    <xsl:value-of select="$classname"/>
    <xsl:text>::~</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>()
{</xsl:text>
    <xsl:text>
}&#xa;</xsl:text>
  </xsl:template>

  <!--
      Generates the definitions of the methods of the context class
      for the current <define>.
  -->
  <xsl:template name="contextimpls">
    <xsl:call-template name="maincontextrefs"/>
    <xsl:call-template name="mainidrefs"/>
    <xsl:for-each select=".//rng:define">
      <xsl:variable name="do">
        <xsl:call-template name="classfordefine"/>
      </xsl:variable>
      <xsl:variable name="classname">
        <xsl:call-template name="contextnamefordefine"/>
<!--
          <xsl:with-param name="name" select="@name"/>
        </xsl:call-template>
-->
      </xsl:variable>
      <xsl:if test="$do = '1'">
        <xsl:text>
/* 
    class: </xsl:text>
        <xsl:value-of select="$classname"/>
        <xsl:text>*/&#xa;</xsl:text>
        <xsl:call-template name="contextconstructorimpl"/>
        <xsl:call-template name="contextdestructorimpl"/>
        <xsl:call-template name="contextrefsimpl"/>
        <xsl:call-template name="contextelementimpl"/>
        <xsl:call-template name="contextattributeimpl"/>
        <xsl:call-template name="contextgetid"/>
        <xsl:call-template name="contextrefsidimpl"/>
        <xsl:call-template name="contextcharactersimpl"/>
        <xsl:call-template name="contextstartimpl"/>
        <xsl:call-template name="contextendimpl"/>
      </xsl:if>
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
    <xsl:value-of select="translate($string, '-+', 'mp')"/>
  </xsl:template>
  
  <!--
      Generates constant definitions for attribute values.
  -->
  <xsl:template name="valueconstants">
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
    <xsl:variable name="classname">
      <xsl:call-template name="valuenamefordefine"/>
    </xsl:variable>
    <xsl:variable name="resource">
      <xsl:call-template name="contextresource"/>
    </xsl:variable>
    <xsl:choose>
      <xsl:when test="$resource = 'List'">
        <xsl:text>&#xa;</xsl:text>
        <xsl:value-of select="$classname"/>
        <xsl:text>::</xsl:text>
        <xsl:value-of select="$classname"/>
        <xsl:text>(const rtl::OUString &amp; rValue)
: OOXMLListValue()
{</xsl:text>
    <xsl:for-each select=".//default">
      <xsl:text>
        mnValue = </xsl:text>
      <xsl:value-of select="."/>
    </xsl:for-each>
    <xsl:for-each select="ancestor::namespace//resource[@name=$name]/value">
      <xsl:text>
  </xsl:text>
    <xsl:text>if (rValue.compareTo(</xsl:text>
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
    <xsl:for-each select=".//rng:define">
      <xsl:variable name="do">
        <xsl:call-template name="valuefordefine"/>
      </xsl:variable>
      <xsl:variable name="classname">
        <xsl:call-template name="valuenamefordefine"/>
      </xsl:variable>
      <xsl:if test="$do = 1">
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

  <!--
      Generates OOXMLContext::elementForRefs.
  -->
  <xsl:template name="maincontextrefs">
    <xsl:param name="prefix"/>
    <xsl:text>
OOXMLContext::Pointer_t OOXMLContext::elementFromRefs(TokenEnum_t nToken)
{</xsl:text>
    <xsl:for-each select=".//start">
      <xsl:variable name="name" select="@name"/>
      <xsl:for-each select="ancestor::namespace//rng:define[@name=$name]">
        <xsl:text>
    {
        OOXMLContext::Pointer_t pTopContext
            (</xsl:text>
        <xsl:call-template name="contextnew"/>
      <xsl:text>);
        OOXMLContext::Pointer_t pContext(pTopContext->element(nToken));

        if (pContext.get() != NULL)
            return pContext;
    }
        </xsl:text>
      </xsl:for-each>
    </xsl:for-each>
    <xsl:text>

    return OOXMLContext::Pointer_t();
}
    </xsl:text>  
  </xsl:template>

  <!--
      Generates OOXMLContext::getIdFromRefs.
  -->
  <xsl:template name="mainidrefs">
    <xsl:param name="prefix"/>
    <xsl:text>
doctok::Id OOXMLContext::getIdFromRefs(TokenEnum_t nToken)
{</xsl:text>
    <xsl:for-each select=".//start">
      <xsl:variable name="name" select="@name"/>
      <xsl:for-each select="ancestor::namespace//rng:define[@name=$name]">
        <xsl:text>
    {
        OOXMLContext::Pointer_t pTopContext
            ( </xsl:text>
        <xsl:call-template name="contextnew"/>
        <xsl:text>);
        doctok::Id nId = pTopContext->getId(nToken);

        if (nId != 0x0)
            return nId;
    }
        </xsl:text>
      </xsl:for-each>
    </xsl:for-each>
    <xsl:text>

    return 0x0;
}
    </xsl:text>  
  </xsl:template>

  <!--
      Generates getAnyContext.
  -->
  <xsl:template name="elementimplany">
    <xsl:variable name="switchbody">
    <xsl:for-each select=".//namespace">
      <xsl:variable name="nsname" select="@name"/>
    <xsl:for-each select=".//rng:element[@name]">
      <xsl:if test="(generate-id(.) = generate-id(key('same-element', @name)[1])) and ./rng:ref">
      <xsl:text>
    case </xsl:text>
      <xsl:call-template name="elementname">
        <xsl:with-param name="prefix" select="$nsname"/>
        <xsl:with-param name="name" select="@name"/>
      </xsl:call-template>
      <xsl:text>:
        pResult = OOXMLContext::Pointer_t
            (new </xsl:text>
      <xsl:variable name="referreddefine">
        <xsl:for-each select="./rng:ref">
          <xsl:call-template name="defineforref"/>
        </xsl:for-each>
      </xsl:variable>
      <xsl:for-each select="$referreddefine">        
        <xsl:call-template name="contextnamefordefine"/>
      </xsl:for-each>
      <xsl:text>());
        break;</xsl:text>
      </xsl:if>
    </xsl:for-each>
    </xsl:for-each>
    </xsl:variable>
    <xsl:text>
OOXMLContext::Pointer_t getAnyContext(TokenEnum_t nToken)
{
    OOXMLContext::Pointer pResult;
    </xsl:text>
    <xsl:if test="string-length($switchbody) > 0">
    <xsl:text>
    switch (nToken)
    { </xsl:text>
    <xsl:text>
    default:
       break;
    }
    </xsl:text>
    </xsl:if>
    <xsl:text>
    return pResult;
}&#xa;</xsl:text>
  </xsl:template>

  <!--
      Generates contant definitions for tokenids.
  -->
  <xsl:template name="defineooxmlids">
    <xsl:text>
namespace NS_ooxml
{</xsl:text>
    <xsl:for-each select="//resource//attribute|//resource//element|//resource//sprm|//resource//value">
      <xsl:if test="contains(@tokenid, 'ooxml:')">
        <xsl:text>
    const QName_t LN_</xsl:text>
    <xsl:value-of select="substring-after(@tokenid, 'ooxml:')"/>
    <xsl:text> = </xsl:text>
    <xsl:value-of select="90000 + position()"/>
    <xsl:text>;</xsl:text>
      </xsl:if>
    </xsl:for-each>
}
  </xsl:template>

  <!--
      Generates mapping from tokenids to strings. (DEBUG)
  -->
  <xsl:template name="qnametostr">
    <xsl:text>
    /* ooxml */
    </xsl:text>
    <xsl:for-each select="//resource">
      <xsl:variable name="name" select="@name"/>
      <xsl:for-each select="attribute|element|sprm">
        <xsl:if test="contains(@tokenid, 'ooxml:')">
          <xsl:text>
    mMap[</xsl:text>
          <xsl:call-template name="idtoqname">
            <xsl:with-param name="id" select="@tokenid"/>
          </xsl:call-template>
          <xsl:text>] = "</xsl:text>
          <xsl:value-of select="@tokenid"/>
          <xsl:text>";</xsl:text>
        </xsl:if>
      </xsl:for-each>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="qnametostrfunc">
    <xsl:text>
string qnameToString(writerfilter::QName_t nToken)
{
    string sResult;

    switch (nToken)
    {</xsl:text>
    <xsl:for-each select="//resource">
      <xsl:variable name="name" select="@name"/>
      <xsl:for-each select="attribute|element|sprm">
        <xsl:if test="contains(@tokenid, 'ooxml:')">
          <xsl:text>
    case </xsl:text>
    <xsl:call-template name="idtoqname">
      <xsl:with-param name="id" select="@tokenid"/>
    </xsl:call-template>
    <xsl:text>:
        sResult = "</xsl:text>
        <xsl:value-of select="@tokenid"/>
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
  
  <xsl:key name="sprms-with-code" match="sprm" use="@tokenid"/>

  <!--
      Generates case labels for mapping from token ids to a single kind
      of sprm.

      @param kind     the sprm kind for which to generate the case labels
  -->
  <xsl:template name="sprmkindcase">
    <xsl:param name="kind"/>
    <xsl:for-each select="key('resources-with-kind', $kind)/sprm">
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

  <!--
      Generates input for gperf to generate hash map for elements.
  -->
  <xsl:template name="gperfinputelements">
    <xsl:text>
%{
#include "OOXMLtokens.hxx"

namespace ooxml { namespace tokenmap { namespace elements {
%}
struct token { const char * name; TokenEnum_t nToken; };
%%</xsl:text>
    <xsl:for-each select=".//rng:element[@enumname]">
      <xsl:if test="generate-id(.) = generate-id(key('same-element-enum', @enumname)[1])">
        <xsl:text>&#xa;</xsl:text>
        <xsl:value-of select="@qname"/>
        <xsl:text>, </xsl:text>
        <xsl:call-template name="elementname">
          <xsl:with-param name="name" select="@enumname"/>
        </xsl:call-template>
      </xsl:if>
    </xsl:for-each>
    <xsl:text>
%%&#xa;</xsl:text>
}}}&#xa;</xsl:template>

<!--
    Generates input for gperf to generate hash map for attributes.
-->
<xsl:template name="gperfinputattributes">
    <xsl:text>
%{
#include "OOXMLtokens.hxx"

namespace ooxml { namespace tokenmap { namespace attributes {
%}
struct token { const char * name; TokenEnum_t nToken; };
%%</xsl:text>
    <xsl:for-each select=".//rng:attribute[@enumname]">
      <xsl:if test="generate-id(.) = generate-id(key('same-attribute-enum', @enumname)[1])">
        <xsl:text>&#xa;</xsl:text>
        <xsl:value-of select="@qname"/>
        <xsl:text>, </xsl:text>
        <xsl:call-template name="attrname">
          <xsl:with-param name="name" select="@enumname"/>
        </xsl:call-template>
      </xsl:if>
    </xsl:for-each>
    <xsl:text>
%%&#xa;
}}}&#xa;</xsl:text></xsl:template>

</xsl:stylesheet>
