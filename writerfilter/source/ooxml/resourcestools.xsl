<!--
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: resourcestools.xsl,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hbrinkm $ $Date: 2007-02-26 15:37:20 $
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

  <xsl:key name="context-resource"
           match="resource" use="@name"/>

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
          <xsl:text>"));
        mElementMap[aStr] = </xsl:text>
        <xsl:value-of select="$element"/>
          <xsl:text>;
        mElementReverseMap[</xsl:text>
        <xsl:value-of select="$element"/>
        <xsl:text>] = aStr;
    } &#xa;</xsl:text>
        </xsl:when>
      </xsl:choose>
    </xsl:for-each>
  </xsl:template>

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
          <xsl:text>"));
        mAttributeMap[aStr] = </xsl:text>
          <xsl:value-of select="$attribute"/>
          <xsl:text>;
        mAttributeReverseMap[</xsl:text>
          <xsl:value-of select="$attribute"/>
          <xsl:text>] = aStr;        
    } &#xa;</xsl:text>
        </xsl:when>
      </xsl:choose>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="prefixforgrammar">
    <xsl:variable name="ns" select="ancestor::rng:grammar/@ns"/>
    <xsl:value-of select="translate(substring-after($ns, 'http://'), '/.', '__')"/>
  </xsl:template>

  <xsl:template match="*" mode="grammar-prefix">
    <xsl:call-template name="prefixforgrammar"/>
  </xsl:template>

  <xsl:template name="contextname">
    <xsl:param name="prefix"/>
    <xsl:param name="name"/>
    <xsl:text>OOXMLContext_</xsl:text>
    <xsl:value-of select="$prefix"/>
    <xsl:text>_</xsl:text>
    <xsl:value-of select="$name"/>
  </xsl:template>

  <xsl:template name="contextnamefordefine">
    <xsl:param name="name"/>
    <xsl:choose>
      <xsl:when test="$name">
        <xsl:variable name="mydefine" select="key('defines-with-name', $name)"/>
        <xsl:variable name="myprefix">
          <xsl:apply-templates select="$mydefine" mode="grammar-prefix"/>
        </xsl:variable>
        <xsl:call-template name="contextname">
          <xsl:with-param name="prefix" select="$myprefix"/>
          <xsl:with-param name="name" select="$mydefine/@name"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:variable name="mydefine" select="key('defines-with-name', @name)"/>
        <xsl:variable name="myprefix">
          <xsl:apply-templates select="$mydefine" mode="grammar-prefix"/>
        </xsl:variable>
        <xsl:call-template name="contextname">
          <xsl:with-param name="prefix" select="$myprefix"/>
          <xsl:with-param name="name" select="$mydefine/@name"/>
        </xsl:call-template>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="contextresource">
    <xsl:value-of select="key('context-resource', @name)/@resource"/>
  </xsl:template>

  <xsl:template name="contextmembers">
  </xsl:template>

  <xsl:template name="contextconstructordecl">
    <xsl:variable name="classname">
      <xsl:call-template name="contextnamefordefine"/>
    </xsl:variable>
    <xsl:value-of select="$classname"/>
    <xsl:text>(const OOXMLContext &amp; rContext);&#xa;</xsl:text>
  </xsl:template>

  <xsl:template name="contextcharactersdecl">
    <xsl:variable name="resource">
      <xsl:call-template name="contextresource"/>
    </xsl:variable>
    <xsl:choose>
      <xsl:when test="$resource = 'Stream'">
        <xsl:text>
    virtual void  characters(const rtl::OUString&amp; /*str*/) 
        throw (xml::sax::SAXException,uno::RuntimeException);
        </xsl:text>
      </xsl:when>
      <xsl:otherwise/>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="contextcharactersimpl">
    <xsl:param name="prefix"/>
    <xsl:variable name="resource">
      <xsl:call-template name="contextresource"/>
    </xsl:variable>
    <xsl:variable name="classname">
      <xsl:call-template name="contextnamefordefine"/>
    </xsl:variable>
    <xsl:choose>
      <xsl:when test="$resource = 'Stream'">
        <xsl:text>
void </xsl:text>
        <xsl:value-of select="$classname"/>
        <xsl:text>::characters(const rtl::OUString&amp; str) 
throw (xml::sax::SAXException,uno::RuntimeException)
{
    mrStream.utext(reinterpret_cast &lt; const sal_uInt8 * &gt; (sText.getStr()), sText.getLength());
}
        </xsl:text>
      </xsl:when>
      <xsl:otherwise/>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="contextparent">
    <xsl:variable name="resource">
      <xsl:call-template name="contextresource"/>
    </xsl:variable>
    <xsl:text>OOXMLContext</xsl:text>
    <xsl:value-of select="$resource"/>
  </xsl:template>

  <xsl:template name="contextdecl">
    <xsl:variable name="classname">
      <xsl:call-template name="contextnamefordefine"/>
    </xsl:variable>
    <xsl:text>
class </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text> : public </xsl:text>
    <xsl:call-template name="contextparent"/>
    <xsl:text>
{</xsl:text>
    <xsl:call-template name="contextmembers"/>
    <xsl:text>

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
    //virtual bool attribute(TokenEnum_t nToken, const rtl::OUString &amp; rValue); 
    virtual string getType() { return "</xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text>"; }</xsl:text>
    <xsl:text>
};
    </xsl:text>
  </xsl:template>
  
  <xsl:template name="classfordefine">
    <xsl:choose>
      <xsl:when test="(.//rng:ref or .//rng:element or .//rng:attribute) and (generate-id(.) = generate-id(key('defines-with-name', @name)[1]))">1</xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="referreddefine">
    <xsl:param name="name"/>
    <xsl:variable name="prefix" select="key('defines-with-name', @name)[1]/ancestor::namespace/@name"/>
    <xsl:value-of select="$prefix"/>
    <xsl:text>:</xsl:text>
    <xsl:value-of select="$name"/>
  </xsl:template>

  <xsl:template name="contextdecls">
    <xsl:param name="prefix"/>
    <xsl:for-each select=".//rng:define">
      <xsl:variable name="do">
        <xsl:call-template name="classfordefine"/>
      </xsl:variable>
      <xsl:text>
/* </xsl:text>
      <xsl:value-of select="@name"/>
      <xsl:value-of select="$do"/>
      <xsl:text>*/ </xsl:text>
      <xsl:if test="$do = '1'">
        <xsl:call-template name="contextdecl"/>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>

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
      <xsl:call-template name="contextnew">
        <xsl:with-param name="define" select="@name"/>
      </xsl:call-template>
      <xsl:text>);</xsl:text>
        OOXMLContext::Pointer_t pContext(pSubContext->element(nToken));

        if (pContext.get() != NULL)
            return pContext;
     }
    </xsl:if>
  </xsl:template>

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

  <xsl:template name="contextnew">
    <xsl:param name="define"/>
    <xsl:for-each select="key('defines-with-name', $define)[1]">
      <xsl:text> new </xsl:text>
      <xsl:call-template name="contextnamefordefine">
        <xsl:with-param name="name" select="$define"/>
      </xsl:call-template>
      <xsl:text>(*this)</xsl:text>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="contextelementimpl">
    <xsl:param name="prefix"/>
    <xsl:variable name="classname">
      <xsl:call-template name="contextnamefordefine"/>
    </xsl:variable>
    <xsl:text>
OOXMLContext::Pointer_t
</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>::element(TokenEnum_t nToken)
{
#ifdef DEBUG_OOXML_ELEMENT
    clog &lt;&lt; "-->" &lt;&lt; "</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text> "&lt;&lt; nToken &lt;&lt; endl;
#endif

    OOXMLContext::Pointer_t pResult;
    </xsl:text>
      <xsl:text>
    switch (nToken)
    {</xsl:text>
      <xsl:for-each select=".//rng:element[@enumname and rng:ref]">
        <xsl:variable name="do">
          <xsl:for-each select="key('defines-with-name', ./rng:ref/@name)[1]">
            <xsl:call-template name="classfordefine"/>
          </xsl:for-each>
        </xsl:variable>
        <xsl:if test="$do = '1'">
          <xsl:text>
     case </xsl:text>
           <xsl:call-template name="elementname">
             <xsl:with-param name="name" select="@enumname"/>
           </xsl:call-template>
           <xsl:text>:</xsl:text>
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
         pResult = OOXMLContext::Pointer_t
               (</xsl:text>
           <xsl:call-template name="contextnew">
             <xsl:with-param name="define" select="./rng:ref/@name"/>
           </xsl:call-template>
         <xsl:text>);
             break;</xsl:text>
               </xsl:otherwise>
             </xsl:choose>
         </xsl:if>
       </xsl:for-each>       
       <xsl:text>
     case OOXML_TOKENS_END:
        // prevent warning
         break;
     default:
         pResult = elementFromRefs(nToken);
              break;
     }
       </xsl:text>
     <xsl:text>

#ifdef DEBUG_OOXML_ELEMENT
    clog &lt;&lt; "&lt;--" &lt;&lt; "</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text> " &lt;&lt; (pResult.get() != NULL) &lt;&lt; endl;
#endif

    return pResult;
}
     </xsl:text>
  </xsl:template>

  <xsl:template name="contextconstructorimpl">
    <xsl:param name="prefix"/>
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
{
#ifdef DEBUG_OOXML_MEMORY
   clog &lt;&lt; "--></xsl:text>
   <xsl:value-of select="$classname"/>
   <xsl:text>" &lt;&lt; endl;
#endif
</xsl:text>
    <xsl:choose>
      <xsl:when test="$resource/action[@name='start' and @action='startParagraphGroup']">
        <xsl:text>
    mrStream.startParagraphGroup();</xsl:text>
      </xsl:when>
      <xsl:when test="$resource/action[@name='start' and @action='startCharacterGroup']">
        <xsl:text>
    mrStream.startCharacterGroup();</xsl:text>
      </xsl:when>
    </xsl:choose>
    <xsl:text>
}&#xa;</xsl:text>
  </xsl:template>

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
    <xsl:choose>
      <xsl:when test="$resource/action[@name='end' and @action='endParagraphGroup']">
        <xsl:text>
    mrStream.endParagraphGroup();</xsl:text>
      </xsl:when>
      <xsl:when test="$resource/action[@name='end' and @action='endCharacterGroup']">
        <xsl:text>
    mrStream.endCharacterGroup();</xsl:text>
      </xsl:when>
    </xsl:choose>
    <xsl:text>
#ifdef DEBUG_OOXML_MEMORY
   clog &lt;&lt; "&lt;--</xsl:text>
   <xsl:value-of select="$classname"/>
   <xsl:text>" &lt;&lt; endl;
#endif
}&#xa;</xsl:text>
  </xsl:template>

  <xsl:template name="contextimpls">
    <xsl:call-template name="maincontextrefs">
      <xsl:with-param name="prefix" select="$prefix"/>
    </xsl:call-template>
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
      </xsl:if>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="maincontextrefs">
    <xsl:param name="prefix"/>
    <xsl:text>
OOXMLContext::Pointer_t OOXMLContext::elementFromRefs(TokenEnum_t nToken)
{</xsl:text>
    <xsl:for-each select=".//start">
      <xsl:variable name="mydefine" select="key('defines-with-name', @name)"/>
      <xsl:text>
    {
        OOXMLContext::Pointer_t pTopContext
            (</xsl:text>
    <xsl:call-template name="contextnew">
      <xsl:with-param name="define" select="@name"/>
    </xsl:call-template>
    <xsl:text>);
        OOXMLContext::Pointer_t pContext(pTopContext->element(nToken));

        if (pContext.get() != NULL)
            return pContext;
    }
    </xsl:text>
    </xsl:for-each>
    <xsl:text>

    return OOXMLContext::Pointer_t();
}
    </xsl:text>  
  </xsl:template>

  <xsl:template name="elementimplany">
    <xsl:text>
OOXMLContext::Pointer_t getAnyContext(TokenEnum_t nToken)
{
    OOXMLContext::Pointer pResult;

    switch (nToken)
    { </xsl:text>
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
      <xsl:variable name="referred">
        <xsl:call-template name="referreddefine">
          <xsl:with-param name="name" select="./rng:ref/@name"/>
        </xsl:call-template>
      </xsl:variable>
      <xsl:call-template name="contextname">
        <xsl:with-param name="prefix" select="$nsname"/>
        <xsl:with-param name="name" select="$referred"/>
      </xsl:call-template>
      <xsl:text>());
        break;</xsl:text>
      </xsl:if>
    </xsl:for-each>
    </xsl:for-each>
    <xsl:text>
    default:
       break;
    }
    
    return pResult;
}&#xa;</xsl:text>
  </xsl:template>

</xsl:stylesheet>