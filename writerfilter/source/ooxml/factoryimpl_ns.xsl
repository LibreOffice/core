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

<!--
    Returns resource for attribute.
-->

<xsl:template name="resourceforattribute">
  <xsl:variable name="mynsid" select="generate-id(ancestor::namespace)"/>
  <xsl:for-each select="rng:ref">
    <xsl:variable name="name" select="@name"/>
    <xsl:variable name="resource1">
      <xsl:for-each select="key('context-resource', @name)[generate-id(ancestor::namespace) = $mynsid]">
        <xsl:value-of select="@resource"/>
      </xsl:for-each>
    </xsl:variable>
    <xsl:choose>
      <xsl:when test="string-length($resource1) > 0">
        <xsl:value-of select="$resource1"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:for-each select="ancestor::namespace/rng:grammar/rng:define[@name=$name]">
          <xsl:call-template name="resourceforattribute"/>
        </xsl:for-each>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:for-each>
  <xsl:for-each select=".//rng:text">
    <xsl:text>String</xsl:text>
  </xsl:for-each>
  <xsl:for-each select=".//rng:data[@type='base64Binary']">
    <xsl:text>String</xsl:text>
  </xsl:for-each>
  <xsl:for-each select=".//rng:data[@type='boolean']">
    <xsl:text>Boolean</xsl:text>
  </xsl:for-each>
  <xsl:for-each select=".//rng:data[@type='unsignedInt']">
    <xsl:text>Integer</xsl:text>
  </xsl:for-each>
  <xsl:for-each select=".//rng:data[@type='int']">
    <xsl:text>Integer</xsl:text>
  </xsl:for-each>
  <xsl:for-each select=".//rng:data[@type='integer']">
    <xsl:text>Integer</xsl:text>
  </xsl:for-each>
  <xsl:for-each select=".//rng:data[@type='string']">
    <xsl:text>String</xsl:text>
  </xsl:for-each>
</xsl:template>

<!-- 

creates code block in OOXMLFactory_<namespace>::createAttributeToResourceMap 
for a rng:define

 -->
<xsl:template name="factoryattributetoresourcemapinner">
  <xsl:variable name="defname" select="@name"/>
  <xsl:for-each select=".//rng:ref[not(ancestor::rng:element or ancestor::rng:attribute)]">
    <xsl:variable name="name" select="@name"/>
    <xsl:for-each select="ancestor::namespace/rng:grammar/rng:define[@name=$name]">
        <xsl:call-template name="factoryattributetoresourcemapinner"/>
    </xsl:for-each>
  </xsl:for-each>

  <xsl:for-each select=".//rng:attribute">
    <xsl:if test="position()=1">
      <xsl:text>
      // </xsl:text>
      <xsl:value-of select="$defname"/>
    </xsl:if>
    <xsl:variable name="resource">
      <xsl:call-template name="resourceforattribute"/>
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="string-length($resource) > 0">
        <xsl:variable name="refdefine1">
          <xsl:for-each select="rng:ref">
            <xsl:variable name="refname" select="@name"/>
            <xsl:for-each select="ancestor::rng:grammar/rng:define[@name=$refname]">
              <xsl:call-template name="idfordefine"/>
            </xsl:for-each>
          </xsl:for-each>
        </xsl:variable>
        <xsl:variable name="refdefine">
          <xsl:choose>
            <xsl:when test="string-length($refdefine1) > 0">
              <xsl:value-of select="$refdefine1"/>
            </xsl:when>
            <xsl:otherwise>
              <xsl:text>0</xsl:text>
            </xsl:otherwise>
          </xsl:choose>
        </xsl:variable>
        <xsl:text>
        (*pMap)[</xsl:text>
        <xsl:call-template name="fasttoken"/>
        <xsl:text>] = AttributeInfo(RT_</xsl:text>
        <xsl:value-of select="$resource"/>
        <xsl:text>, </xsl:text>
        <xsl:value-of select="$refdefine"/>
        <xsl:text>);</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>
      // empty resource: </xsl:text>
      <xsl:call-template name="fasttoken"/>
      </xsl:otherwise>
    </xsl:choose>
    
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

<!-- factoryelementtoresorucemapinner -->
<xsl:template name="factorycreateelementmapinner">
    <xsl:for-each select=".//rng:ref[not(ancestor::rng:element or ancestor::rng:attribute)]">
        <xsl:variable name="name" select="@name"/>
        <xsl:variable name="block">
            <xsl:for-each select="ancestor::namespace/rng:grammar/rng:define[@name=$name]">
                <xsl:call-template name="factorycreateelementmapinner"/>
            </xsl:for-each>
        </xsl:variable>
	<xsl:variable name="block1">
	  <xsl:choose>
	    <xsl:when test="string-length($block) = 0">
	      <xsl:for-each select="ancestor::model/namespace/rng:grammar/rng:define[@name=$name]">
		<xsl:call-template name="factorycreateelementmapinner"/>
	      </xsl:for-each>
	    </xsl:when>
	    <xsl:otherwise>
	      <xsl:value-of select="$block"/>
	    </xsl:otherwise>
	  </xsl:choose>
	</xsl:variable>
        <xsl:if test="string-length($block1) > 0">
            <xsl:text>
         /* ref: </xsl:text>
            <xsl:value-of select="$name"/>
            <xsl:text>*/</xsl:text>
            <xsl:value-of select="$block1"/>
        </xsl:if>
    </xsl:for-each>
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
        <xsl:text>
        /* start: </xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>*/</xsl:text>
        <xsl:if test="string-length($block) > 0">
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
            @action='handleOLE' or
            @action='handleFontRel'">
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
        <xsl:when test="@action='startRow' or @action='endRow'">
            <xsl:text>
    dynamic_cast&lt;OOXMLFastContextHandlerTextTableRow*&gt;(pHandler)-&gt;</xsl:text>
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
        <xsl:when test="@action='positionOffset'">
    pHandler-&gt;positionOffset(sText);
        </xsl:when>
        <xsl:when test="@action='alignH'">
    pHandler-&gt;alignH(sText);
        </xsl:when>
        <xsl:when test="@action='alignV'">
    pHandler-&gt;alignV(sText);
        </xsl:when>
        <xsl:when test="@action='newProperty'">
          <xsl:text>
    OOXMLFastHelper&lt;OOXMLIntegerValue&gt;::newProperty(pHandler, </xsl:text>
    <xsl:call-template name="idtoqname">
      <xsl:with-param name="id" select="@tokenid"/>
    </xsl:call-template>
    <xsl:text>, "</xsl:text>
    <xsl:value-of select="@value"/>
    <xsl:text>");</xsl:text>
        </xsl:when>
        <xsl:when test="@action='mark'">
          <xsl:text>
    OOXMLFastHelper&lt;OOXMLIntegerValue&gt;::mark(pHandler, </xsl:text>
    <xsl:call-template name="idtoqname">
      <xsl:with-param name="id" select="@sendtokenid"/>
    </xsl:call-template>
    <xsl:text>, "</xsl:text>
    <xsl:value-of select="@value"/>
    <xsl:text>");</xsl:text>
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
  <xsl:variable name="switchblock1">
    <xsl:for-each select="resource[action/@name=$action]">
      <xsl:text>
      </xsl:text>
      <xsl:call-template name="caselabeldefine"/>
      <xsl:for-each select="action[@name=$action]">
        <xsl:call-template name="factorychooseaction"/>
      </xsl:for-each>
      <xsl:text>    break;&#xa;</xsl:text>
    </xsl:for-each>
  </xsl:variable>
  <xsl:variable name="switchblock2">
    <xsl:if test="$action='characters'">
      <xsl:for-each select="resource[@resource='Value']">
        <xsl:if test="count(attribute) = 0">
          <xsl:variable name="name" select="@name"/>
          <xsl:text>    </xsl:text>
          <xsl:call-template name="caselabeldefine"/>
          <xsl:text>&#xa;</xsl:text>
          <xsl:for-each select="ancestor::namespace/rng:grammar/rng:define[@name=$name]">
            <xsl:for-each select="rng:ref">
              <xsl:call-template name="charactersactionforvalues"/>
            </xsl:for-each>
          </xsl:for-each>
          <xsl:text>        break;&#xa;</xsl:text>
      </xsl:if>
      </xsl:for-each>
    </xsl:if>
  </xsl:variable>
  <xsl:text>&#xa;&#xa;</xsl:text>
  <xsl:text>void </xsl:text>
  <xsl:call-template name="factoryclassname"/>
  <xsl:text>::</xsl:text>
  <xsl:value-of select="$action"/>
  <xsl:text>Action(OOXMLFastContextHandler*</xsl:text>
  <xsl:if test="string-length($switchblock1) &gt; 0 or string-length($switchblock2) &gt; 0">
    <xsl:text> pHandler</xsl:text>
  </xsl:if>
  <xsl:if test="$action='characters'">
    <xsl:text>, const OUString &amp;</xsl:text>
    <xsl:if test="contains($switchblock1, 'sText') or contains($switchblock2, 'sText')">
      <xsl:text> sText</xsl:text>
    </xsl:if>
  </xsl:if>
  <xsl:text>)&#xa;</xsl:text>
  <xsl:text>{&#xa;</xsl:text>
  <xsl:if test="string-length($switchblock1) > 0 or string-length($switchblock2) > 0">
    <xsl:text>    sal_uInt32 nDefine = pHandler->getDefine();&#xa;</xsl:text>
  </xsl:if>
  <xsl:if test="string-length($switchblock1) > 0">
    <xsl:text>    switch (nDefine)&#xa;</xsl:text>
    <xsl:text>    {&#xa;</xsl:text>
    <xsl:value-of select="$switchblock1"/>
    <xsl:text>    default:&#xa;</xsl:text>
    <xsl:text>        break;&#xa;</xsl:text>
    <xsl:text>    }&#xa;</xsl:text>
  </xsl:if>
  <xsl:if test="string-length($switchblock2) > 0">
    <xsl:text>    switch (nDefine)&#xa;</xsl:text>
    <xsl:text>    {&#xa;</xsl:text>
    <xsl:value-of select="$switchblock2"/>
    <xsl:text>    default:&#xa;</xsl:text>
    <xsl:text>        break;&#xa;</xsl:text>
    <xsl:text>    }&#xa;</xsl:text>
  </xsl:if>
  <xsl:text>}&#xa;</xsl:text>
</xsl:template>

<xsl:template name="charactersactionforvalues">
  <xsl:variable name="name" select="@name"/>
  <xsl:for-each select="ancestor::namespace/rng:grammar/rng:define[@name=$name]">
    <xsl:text>        {&#xa;</xsl:text>
    <xsl:text>            // </xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text>&#xa;</xsl:text>
    <xsl:for-each select="rng:data[@type='int']">
      <xsl:text>            OOXMLValue::Pointer_t pValue(new OOXMLIntegerValue(sText));&#xa;</xsl:text>
      <xsl:text>            pValueHandler->setValue(pValue);&#xa;</xsl:text>
    </xsl:for-each>
    <xsl:for-each select="rng:list">
      <xsl:text>            ListValueMapPointer pListValueMap = getListValueMap(nDefine);&#xa;</xsl:text>
      <xsl:text>            if (pListValueMap.get() != NULL)&#xa;</xsl:text>
      <xsl:text>            {&#xa;</xsl:text>
      <xsl:text>                OOXMLValue::Pointer_t pValue(new OOXMLIntegerValue((*pListValueMap)[sText]));&#xa;</xsl:text>
      <xsl:text>                pValueHandler->setValue(pValue);</xsl:text>
      <xsl:text>            }&#xa;</xsl:text>
    </xsl:for-each>
    <xsl:text>        }&#xa;</xsl:text>
  </xsl:for-each>
</xsl:template>

<!-- factoryactions -->
<xsl:template name="factoryactions">
    <xsl:variable name="ns" select="@name"/>
    <xsl:for-each select="resource/action[not(@name='characters')]">
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
    <xsl:call-template name="factoryaction">
      <xsl:with-param name="action">characters</xsl:with-param>
    </xsl:call-template>
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

<xsl:template name="factorytokentoidmapinner">
    <xsl:variable name="name" select="@name"/>
    <xsl:for-each select=".//rng:ref[not(ancestor::rng:element or ancestor::rng:attribute)]">
        <xsl:variable name="refname" select="@name"/>
	<xsl:variable name="refblock1">
	    <xsl:for-each 
		select="ancestor::rng:grammar/rng:define[@name=$refname]">
	      <xsl:call-template name="factorytokentoidmapinner"/>
	    </xsl:for-each>
	</xsl:variable>
	<xsl:choose>
	  <xsl:when test="string-length($refblock1) = 0">
	    <xsl:for-each select="ancestor::model/namespace/rng:grammar/rng:define[@name=$refname]">
		<xsl:call-template name="factorytokentoidmapinner"/>
	      </xsl:for-each>
	  </xsl:when>
	  <xsl:otherwise>
	    <xsl:value-of select="$refblock1"/>
	  </xsl:otherwise>
	</xsl:choose>
    </xsl:for-each>
    <xsl:variable name="body">
        <xsl:for-each select="ancestor::namespace/resource[@name=$name]">
            <xsl:for-each select="element[@tokenid]|attribute[@tokenid]">
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
    default:</xsl:text>
    <xsl:for-each select="start">
      <xsl:variable name="name" select="@name"/>
      <xsl:text>
        // </xsl:text>
        <xsl:value-of select="$name"/>
      <xsl:for-each select="ancestor::namespace/rng:grammar/rng:define[@name=$name]">
        <xsl:call-template name="factorytokentoidmapinner"/>
      </xsl:for-each>
    </xsl:for-each>
    <xsl:text>
        break;
    }
    
    return pMap;
}
</xsl:text>
</xsl:template>

<xsl:template name="factoryattributeactiondefineinner">
  <xsl:variable name="name" select="@name"/>
  <xsl:variable name="block">
    <xsl:for-each select="ancestor::namespace/resource[@name=$name]">
      <xsl:for-each select="attribute[@action]">
        <xsl:text>
        case </xsl:text>
        <xsl:call-template name="fasttoken"/>
        <xsl:text>:
            pHandler-></xsl:text>
            <xsl:value-of select="@action"/>
            <xsl:text>(pValue);
            break;</xsl:text>
      </xsl:for-each>
    </xsl:for-each>
  </xsl:variable>
  <xsl:if test="string-length($block) > 0">
    <xsl:variable name="handlertype">
      <xsl:text>OOXMLFastContextHandler</xsl:text>
        <xsl:for-each select="ancestor::namespace/resource[@name=$name]">
          <xsl:value-of select="@resource"/>
        </xsl:for-each>      
    </xsl:variable>
    <xsl:text>
    {
        </xsl:text>
        <xsl:value-of select="$handlertype"/>
        <xsl:text> * pHandler = dynamic_cast&lt;</xsl:text>
        <xsl:value-of select="$handlertype"/>
        <xsl:text> * &gt;(_pHandler);

        switch(nToken)
        {</xsl:text>
        <xsl:value-of select="$block"/>
        <xsl:text>
        default:
            break;
        }
    }</xsl:text>
  </xsl:if>
</xsl:template>

<xsl:template name="factoryattributeactioninner">
    <xsl:for-each select="rng:grammar/rng:define">
      <xsl:variable name="inner">
        <xsl:call-template name="factoryattributeactiondefineinner"/>
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
</xsl:template>

<xsl:template name="factoryattributeaction">
  <xsl:variable name="inner">
    <xsl:call-template name="factoryattributeactioninner"/>
  </xsl:variable>
  <xsl:choose>
    <xsl:when test="string-length($inner) > 0">
      <xsl:text>
void </xsl:text>
<xsl:call-template name="factoryclassname"/>
<xsl:text>::attributeAction(OOXMLFastContextHandler * _pHandler, Token_t nToken, OOXMLValue::Pointer_t pValue)
{
    switch(_pHandler->getDefine())
    {</xsl:text>
    <xsl:value-of select="$inner"/>
    <xsl:text>
    default:
        break;
    }
}
</xsl:text>
    </xsl:when>
    <xsl:otherwise>
      <xsl:text>
void </xsl:text>
<xsl:call-template name="factoryclassname"/>
<xsl:text>::attributeAction(OOXMLFastContextHandler *, Token_t, OOXMLValue::Pointer_t)
{
}
</xsl:text>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="factorygetname">
  <xsl:param name="ns"/>
  <xsl:text>
#ifdef DEBUG_FACTORY
string </xsl:text>
<xsl:call-template name="factoryclassname"/>
<xsl:text>::getName() const
{
    return "</xsl:text>
    <xsl:value-of select="$ns"/>
    <xsl:text>";
}
#endif
</xsl:text>
</xsl:template>

  <xsl:template match="/">
    <xsl:variable name="ns" select="substring-before(substring-after($file, 'OOXMLFactory_'), '.cxx')"/>
    <xsl:text>
#include "doctok/sprmids.hxx"
#include "doctok/resourceids.hxx"
#include "ooxml/resourceids.hxx"
#include "OOXMLFactory_values.hxx"
#include "OOXMLFactory_</xsl:text>
    <xsl:value-of select="$ns"/>
    <xsl:text>.hxx"
#include "ooxml/OOXMLFastHelper.hxx"
    
#ifdef _MSC_VER
#pragma warning(disable:4065) // switch statement contains 'default' but no 'case' labels
#endif

namespace writerfilter {
namespace ooxml {

/// @cond GENERATED
    </xsl:text>
    <xsl:for-each select="/model/namespace[@name=$ns]">
        <xsl:call-template name="factorymutexdecl"/>
        <xsl:call-template name="factoryconstructor"/>
        <xsl:call-template name="factorydestructor"/>
        <xsl:call-template name="factorygetinstance"/>
        <xsl:call-template name="factoryattributetoresourcemap"/>
        <xsl:call-template name="factorylistvaluemap"/>
        <xsl:call-template name="factorycreateelementmap"/>
        <xsl:call-template name="factoryactions"/>
        <xsl:call-template name="factorygetdefinename"/>
        <xsl:call-template name="factorytokentoidmap"/>
        <xsl:call-template name="factoryattributeaction"/>
        <xsl:call-template name="factorygetname">
          <xsl:with-param name="ns" select="$ns"/>
        </xsl:call-template>
    </xsl:for-each>
    <xsl:text>
/// @endcond
}}
</xsl:text>
</xsl:template>

</xsl:stylesheet>
