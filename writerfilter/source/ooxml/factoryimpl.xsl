<!--
/*************************************************************************
 *
  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
  
  Copyright 2008 by Sun Microsystems, Inc.
 
  OpenOffice.org - a multi-platform office productivity suite
 
  $RCSfile: fastresources_wml.xsl,v $
 
  $Revision: 1.3 $
 
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
    xmlns:UML = 'org.omg.xmi.namespace.UML' xml:space="default">
  <xsl:output method="text" />
  <xsl:param name="prefix"/>
  
  <xsl:include href="factorytools.xsl"/>
  
  <xsl:key name="resources-with-kind" match="//resource[kind]" use="kind/@name"/>
  <xsl:key name="sprms-with-code" match="//resource/element[@tokenid]" use="@tokenid"/>

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

            snprintf(buffer, sizeof(buffer), "0x%08" SAL_PRIuUINT32, nDefine);
            debug_logger->attribute("definenum", buffer);
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
                        <xsl:text>&gt;::createAndSetParentAndDefine(pHandler, Element, nId, aCreateElement.m_nId));
                break;</xsl:text>
                    </xsl:if>
                </xsl:if>
            </xsl:for-each>
            <xsl:text>
	    case RT_Any:
	        aResult.set(createFastChildContextFromStart(pHandler, Element));
		break;
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

<xsl:template name="factorycreatefromstart">
    <xsl:text>
uno::Reference&lt; xml::sax::XFastContextHandler &gt; OOXMLFactory::createFastChildContextFromStart
(OOXMLFastContextHandler * pHandler, Token_t Element)
{
#ifdef DEBUG_CREATE
    debug_logger->startElement("createFastChildContextFromStart");
#endif

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
    
#ifdef DEBUG_CREATE
    debug_logger->endElement("createFastChildContextFromStart");
#endif
    return aResult;
}
</xsl:text>
</xsl:template>

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

  <xsl:template match="/">
    <xsl:text>    
#include "OOXMLFactory.hxx"
#include "OOXMLFastHelper.hxx"
#include "OOXMLStreamImpl.hxx"
#include "doctok/sprmids.hxx"
#include "doctok/resourceids.hxx"
</xsl:text>
    <xsl:call-template name="factoryincludes"/>
    <xsl:text>
namespace writerfilter {
namespace ooxml {

/// @cond GENERATED
    </xsl:text>
    <xsl:call-template name="factorycreatecontextfromfactory"/>
    <xsl:call-template name="factoryfornamespace"/>
    <xsl:call-template name="factorycreatefromstart"/>
    <xsl:call-template name="fasttokentoid"/>
    <xsl:call-template name="sprmkind"/>
    <xsl:call-template name="getfastparser"/>
    <xsl:text>
/// @endcond
}}
</xsl:text>
</xsl:template>

</xsl:stylesheet>