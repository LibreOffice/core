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

<xsl:template name="factoryactiondecls">
    <xsl:variable name="ns" select="@name"/>
    <xsl:for-each select="resource/action[not(@name='characters')]">
        <xsl:sort select="@name"/>
        <xsl:if test="generate-id(key('actions', @name)[ancestor::namespace/@name=$ns][1]) = generate-id(.)">
            <xsl:text>
    void </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>Action(OOXMLFastContextHandler * pHandler</xsl:text>
            <xsl:text>);</xsl:text>
        </xsl:if>
    </xsl:for-each>
    <xsl:text>
    virtual void charactersAction(OOXMLFastContextHandler * pHandler, const ::rtl::OUString &amp; sText);</xsl:text>
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
    virtual void attributeAction(OOXMLFastContextHandler * pHandler, Token_t nToken, OOXMLValue::Pointer_t pValue);
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

  <xsl:template match="/">
    <xsl:variable name="ns" select="substring-before(substring-after($file, 'OOXMLFactory_'), '.hxx')"/>
    <xsl:variable name="incguard">
      <xsl:text>INCLUDED_OOXML_FACTORY_</xsl:text>
      <xsl:value-of select="translate($ns, 'abcdefghijklmnopqrstuvwxyz-', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ_')"/>
      <xsl:text>_HXX</xsl:text>
    </xsl:variable>
    <xsl:call-template name="licenseheader"/>
    <xsl:text>
#ifndef </xsl:text>
    <xsl:value-of select="$incguard"/>
    <xsl:text>
#define </xsl:text>
    <xsl:value-of select="$incguard"/>
    <xsl:text>
#include "OOXMLFactory.hxx"
#include "OOXMLFactory_generated.hxx"
#include "OOXMLnamespaceids.hxx"
#include "resourceids.hxx"

namespace writerfilter {
namespace ooxml {

/// @cond GENERATED
    </xsl:text>
    <xsl:for-each select="/model/namespace[@name=$ns]">
        <xsl:call-template name="factorydecl"/>
    </xsl:for-each>
    <xsl:text>
/// @endcond
}}
#endif //</xsl:text>
    <xsl:value-of select="$incguard"/>
    <xsl:text>&#xa;</xsl:text>
</xsl:template>

</xsl:stylesheet>