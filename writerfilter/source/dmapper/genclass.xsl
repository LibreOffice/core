<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet 
    version="1.0" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
    xmlns:rng="http://relaxng.org/ns/structure/1.0"
    xmlns:xalan="http://xml.apache.org/xalan"
    exclude-result-prefixes = "xalan"
    xml:indent="true">
  <xsl:output method="text" version="1.0" encoding="UTF-8" indent="yes" omit-xml-declaration="no"/>

  <xsl:template match="/">
    <xsl:for-each select="/model/class">
      <xsl:choose>
	<xsl:when test="$filetype='inc'">
	  <xsl:call-template name="incfile"/>
	</xsl:when>
	<xsl:when test="$filetype='impl'">
	  <xsl:call-template name="implfile"/>
	</xsl:when>
      </xsl:choose>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="license">
    <xsl:text>/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PropertyMap.hxx,v $
 * $Revision: 1.18 $
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
 * &lt;http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/&#xa;</xsl:text>
  </xsl:template>

  <xsl:template name="memberid">
    <xsl:variable name="member">
      <xsl:choose>
	<xsl:when test="@member">
	  <xsl:value-of select="@member"/>
	</xsl:when>
	<xsl:otherwise>
	  <xsl:value-of select="@name"/>
	</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:for-each select="ancestor::class/member[@name=$member]">
      <xsl:choose>
	<xsl:when test="@type='bool'">
	  <xsl:text>m_b</xsl:text>
	</xsl:when>
	<xsl:when test="@type='OUString'">
	  <xsl:text>m_s</xsl:text>
	</xsl:when>
	<xsl:when test="@type='sal_uInt32'">
	  <xsl:text>m_n</xsl:text>
	</xsl:when>
	<xsl:otherwise>
	  <xsl:text>m_</xsl:text>
	</xsl:otherwise>
      </xsl:choose>
    </xsl:for-each>
    <xsl:call-template name="capfirst">
      <xsl:with-param name="string" select="$member"/>
    </xsl:call-template>
  </xsl:template>
  
  <xsl:template name="incfile">
    <xsl:call-template name="license"/>
    <xsl:for-each select="//class">
      <xsl:variable name="includeguard">
	<xsl:text>INCLUDED_</xsl:text>
	<xsl:value-of select="@name"/>
	<xsl:text>_HXX</xsl:text>
	</xsl:variable>
	<xsl:text>#ifndef </xsl:text>
	<xsl:value-of select="$includeguard"/>
	<xsl:text>&#xa;#define </xsl:text>
	<xsl:value-of select="$includeguard"/>
	<xsl:text>&#xa;#include &lt;resourcemodel/WW8ResourceModel.hxx></xsl:text>
	<xsl:text>&#xa;#include &lt;rtl/ustring.hxx></xsl:text>
	<xsl:text>&#xa;namespace writerfilter {</xsl:text>
	<xsl:text>&#xa;namespace dmapper {</xsl:text>
	<xsl:call-template name="classdecl"/>
	<xsl:text>&#xa;}}</xsl:text>
	<xsl:text>&#xa;#endif //</xsl:text>
	<xsl:value-of select="$includeguard"/>
	<xsl:text>&#xa;</xsl:text>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="implfile">
    <xsl:call-template name="license"/>
    <xsl:for-each select="//class">
      <xsl:text>#include "</xsl:text>
      <xsl:value-of select="@name"/>
      <xsl:text>.hxx"&#xa;</xsl:text>
      <xsl:text>&#xa;#include &lt;ooxml/resourceids.hxx></xsl:text>
      <xsl:text>&#xa;#ifdef DEBUG_DOMAINMAPPER</xsl:text>
      <xsl:text>&#xa;#include &lt;resourcemodel/QNameToString.hxx></xsl:text>
      <xsl:text>&#xa;#include "dmapperLoggers.hxx"</xsl:text>
      <xsl:text>&#xa;#endif</xsl:text>
      <xsl:text>&#xa;namespace writerfilter {</xsl:text>
      <xsl:text>&#xa;namespace dmapper {&#xa;</xsl:text>
      <xsl:call-template name="classimpl"/>
      <xsl:text>}}&#xa;</xsl:text>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="classdecl">
    <xsl:text>&#xa;class </xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:if test="parent">
      <xsl:text> : public </xsl:text>
    </xsl:if>
    <xsl:for-each select="parent">
      <xsl:if test="position() > 1">
	<xsl:text>, </xsl:text>
      </xsl:if>
      <xsl:value-of select="@name"/>
    </xsl:for-each>
    <xsl:text>&#xa;{</xsl:text>
    <xsl:text>&#xa;public:</xsl:text>
    <xsl:call-template name="typedefs"/>
    <xsl:text>&#xa;    // constructor</xsl:text>
    <xsl:text>&#xa;    </xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text>();</xsl:text>
    <xsl:text>&#xa;    // destructor</xsl:text>
    <xsl:text>&#xa;    virtual ~</xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text>();&#xa;</xsl:text>
    <xsl:call-template name="memberfuncdecls"/>
    <xsl:for-each select="sprm">
      <xsl:text>&#xa;    // sprm</xsl:text>
      <xsl:text>&#xa;    void sprm(Sprm &amp; r_sprm);</xsl:text>
      <xsl:text>&#xa;    void resolveSprm(Sprm &amp; r_sprm);&#xa;</xsl:text>
    </xsl:for-each>
    <xsl:for-each select="attribute">
      <xsl:text>&#xa;    // attribute</xsl:text>
      <xsl:text>&#xa;    void attribute(Id name, Value &amp; val);&#xa;</xsl:text>
    </xsl:for-each>
    <xsl:text>&#xa;private:</xsl:text>
    <xsl:call-template name="memberdecls"/>
    <xsl:text>&#xa;};&#xa;&#xa;</xsl:text>
  </xsl:template>

  <xsl:template name="type">
    <xsl:param name="name"/>
    <xsl:choose>
      <xsl:when test="$name='OUString'">
	<xsl:text>::rtl::OUString</xsl:text>
      </xsl:when>
      <xsl:otherwise>
	<xsl:value-of select="$name"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="typedefs">
    <xsl:text>&#xa;    // typedefs</xsl:text>
    <xsl:for-each select="typedef">
      <xsl:text>&#xa;    typedef </xsl:text>
      <xsl:choose>
	<xsl:when test="@type = 'shared_ptr'">
	  <xsl:text>::boost::shared_ptr&lt;</xsl:text>
	  <xsl:value-of select="ancestor::class/@name"/>
	  <xsl:text>&gt;</xsl:text>
	</xsl:when>
	<xsl:otherwise>
	</xsl:otherwise>
      </xsl:choose>
      <xsl:for-each select="vector">
	<xsl:text>::std::vector&lt;</xsl:text>
	<xsl:call-template name="cctype"/>
	<xsl:text>&gt;</xsl:text>
      </xsl:for-each>
      <xsl:text> </xsl:text>
      <xsl:value-of select="@name"/>
      <xsl:text>;</xsl:text>
    </xsl:for-each>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template name="settername">
    <xsl:text>set</xsl:text>
    <xsl:call-template name="capfirst">
      <xsl:with-param name="string" select="@name"/>
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="gettername">
    <xsl:text>get</xsl:text>
    <xsl:call-template name="capfirst">
      <xsl:with-param name="string" select="@name"/>
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="pushbackname">
    <xsl:value-of select="@name"/>
    <xsl:text>PushBack</xsl:text>
  </xsl:template>

  <xsl:template name="settersig">    
    <xsl:param name="classname"/>
    <xsl:variable name="impl">
      <xsl:if test="string-length($classname) > 0">
	<xsl:text>true</xsl:text>
      </xsl:if>
    </xsl:variable>
    <xsl:text>void </xsl:text>
    <xsl:if test="string-length($classname) > 0">
      <xsl:value-of select="$classname"/>
      <xsl:text>::</xsl:text>
    </xsl:if>
    <xsl:call-template name="settername"/>
    <xsl:text>(</xsl:text>
    <xsl:call-template name="constcctyperef">
      <xsl:with-param name="impl" select="$impl"/>
    </xsl:call-template>
    <xsl:text> </xsl:text>
    <xsl:call-template name="paramname"/>
    <xsl:text>)</xsl:text>
  </xsl:template>

  <xsl:template name="gettersig">
    <xsl:param name="classname"/>    
    <xsl:variable name="impl">
      <xsl:if test="string-length($classname) > 0">
	<xsl:text>true</xsl:text>
      </xsl:if>
    </xsl:variable>
    <xsl:variable name="type" select="@type"/>
    <xsl:call-template name="constcctyperef">
      <xsl:with-param name="impl" select="$impl"/>
    </xsl:call-template>
    <xsl:text> </xsl:text>
    <xsl:if test="string-length($classname) > 0">
      <xsl:value-of select="$classname"/>
      <xsl:text>::</xsl:text>
    </xsl:if>    
    <xsl:call-template name="gettername"/>
    <xsl:text>() const</xsl:text>
  </xsl:template>
  
  <xsl:template name="pushbacksig">
    <xsl:param name="classname"/>    
    <xsl:variable name="pushback">
      <xsl:call-template name="pushbackname"/>
    </xsl:variable>
    <xsl:variable name="type" select="@type"/>
    <xsl:for-each select="ancestor::class/typedef[@name=$type]">
      <xsl:for-each select="vector">
	<xsl:text>void </xsl:text>
	<xsl:if test="string-length($classname) > 0">
	  <xsl:value-of select="$classname"/>
	  <xsl:text>::</xsl:text>
	</xsl:if>    
	<xsl:value-of select="$pushback"/>
	<xsl:text>(</xsl:text>
	<xsl:call-template name="constcctyperef">
	  <xsl:with-param name="deep">true</xsl:with-param>
	</xsl:call-template>
	<xsl:text> r_Element</xsl:text>
	<xsl:text>)</xsl:text>
      </xsl:for-each>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="memberfuncdecls">
    <xsl:for-each select="member">
      <xsl:text>&#xa;    // member: </xsl:text>
      <xsl:value-of select="@name"/>
      <xsl:text>&#xa;    </xsl:text>
      <xsl:call-template name="settersig"/>
      <xsl:text>;</xsl:text>
      <xsl:text>&#xa;    </xsl:text>
      <xsl:call-template name="gettersig"/>
      <xsl:text>;&#xa;    </xsl:text>
      <xsl:variable name="pushback">
	<xsl:call-template name="pushbacksig"/>
      </xsl:variable>
      <xsl:if test="string-length($pushback) > 0">
	<xsl:value-of select="$pushback"/>
	<xsl:text>;&#xa;</xsl:text>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="memberdecls">
    <xsl:for-each select="member">
      <xsl:text>&#xa;    </xsl:text>
      <xsl:call-template name="cctype"/>
      <xsl:text> </xsl:text>
      <xsl:call-template name="memberid"/>
      <xsl:text>;</xsl:text>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="cctype">
    <xsl:param name="deep"/>
    <xsl:param name="impl"/>
    <xsl:variable name="type" select="@type"/>
    <xsl:variable name="try1">
      <xsl:if test="$deep='true'">
	<xsl:for-each select="ancestor::class/typedef[@name=$type]">
	  <xsl:for-each select="vector">
	    <xsl:call-template name="cctype">
	      <xsl:with-param name="impl" select="$impl"/>
	      <xsl:with-param name="deep" select="$deep"/>
	    </xsl:call-template>
	  </xsl:for-each>
	</xsl:for-each>
      </xsl:if>
    </xsl:variable>
    <xsl:choose>
      <xsl:when test="string-length($try1) > 0">
	<xsl:value-of select="$try1"/>
      </xsl:when>
      <xsl:when test="@type='OUString'">
	<xsl:text>rtl::OUString</xsl:text>
      </xsl:when>
      <xsl:otherwise>
	<xsl:if test="$impl='true'">
	  <xsl:for-each select="ancestor::class">
	    <xsl:variable name="classname" select="@name"/>
	    <xsl:for-each select="typedef[@name=$type]">
	      <xsl:value-of select="$classname"/>
	      <xsl:text>::</xsl:text>
	    </xsl:for-each>
	  </xsl:for-each>
	</xsl:if>
	<xsl:value-of select="@type"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="constcctype">
    <xsl:param name="impl"/>
    <xsl:choose>
      <xsl:when test="@type='bool' or @type='sal_uInt32'">
	<xsl:value-of select="@type"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:text>const </xsl:text>
	<xsl:call-template name="cctype">
	  <xsl:with-param name="impl" select="$impl"/>
	</xsl:call-template>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="constcctyperef">
    <xsl:param name="impl"/>
    <xsl:choose>
      <xsl:when test="@type='bool' or @type='sal_uInt32'">
	<xsl:value-of select="@type"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:call-template name="constcctype">
	  <xsl:with-param name="impl" select="$impl"/>
	</xsl:call-template>
	<xsl:text> &amp;</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="capfirst">
    <xsl:param name="string"/>
    <xsl:variable name="first" select="translate(substring($string, 1,1), 'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ')"/>
    <xsl:value-of select="concat($first, substring($string, 2))"/>
  </xsl:template>

  <xsl:template name="paramname">
    <xsl:choose>
      <xsl:when test="@type='OUString'">
	<xsl:text>r_s</xsl:text>
	<xsl:call-template name="capfirst">
	  <xsl:with-param name="string" select="@name"/>
	</xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
	<xsl:text>r_</xsl:text>
	<xsl:value-of select="@name"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="setterimpl">
    <xsl:call-template name="settersig">
      <xsl:with-param name="classname" select="ancestor::class/@name"/>
    </xsl:call-template>
    <xsl:text>&#xa;{</xsl:text>
    <xsl:text>&#xa;    </xsl:text>
    <xsl:call-template name="memberid"/>
    <xsl:text> = </xsl:text>
    <xsl:call-template name="paramname"/>
    <xsl:text>;</xsl:text>
    <xsl:text>&#xa;}&#xa;&#xa;</xsl:text>
  </xsl:template>

  <xsl:template name="getterimpl">
    <xsl:call-template name="gettersig">
      <xsl:with-param name="classname" select="ancestor::class/@name"/>
    </xsl:call-template>
    <xsl:text>&#xa;{</xsl:text>
    <xsl:text>&#xa;    return </xsl:text>
    <xsl:call-template name="memberid"/>
    <xsl:text>;</xsl:text>
    <xsl:text>&#xa;}&#xa;&#xa;</xsl:text>
  </xsl:template>

  <xsl:template name="pushbackimpl">
    <xsl:variable name="sig">
      <xsl:call-template name="pushbacksig">
	<xsl:with-param name="classname" select="ancestor::class/@name"/>
      </xsl:call-template>
    </xsl:variable>
    <xsl:if test="string-length($sig) > 0">
      <xsl:value-of select="$sig"/>
      <xsl:text>&#xa;{</xsl:text>
      <xsl:text>&#xa;    </xsl:text>
      <xsl:call-template name="memberid"/>
      <xsl:text>.push_back(r_Element);</xsl:text>
      <xsl:text>&#xa;}&#xa;&#xa;</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template name="getvalue">
    <xsl:param name="valuePrefix"/>
    <xsl:choose>
      <xsl:when test="@type='OUString'">
	<xsl:value-of select="$valuePrefix"/>
	<xsl:text>getString()</xsl:text>
      </xsl:when>
      <xsl:when test="@type='bool' or @type='sal_uInt32'">
	<xsl:value-of select="$valuePrefix"/>
	<xsl:text>getInt()</xsl:text>
      </xsl:when>
      <xsl:when test="@type">
	<xsl:variable name="type" select="@type"/>
	<xsl:for-each select="ancestor::class/typedef[@name=$type]">
	  <xsl:call-template name="getvalue">
	    <xsl:with-param name="valuePrefix" select="$valuePrefix"/>
	  </xsl:call-template>
	</xsl:for-each>
      </xsl:when>
      <xsl:otherwise>
	<xsl:variable name="member">
	  <xsl:choose>
	    <xsl:when test="@member">
	      <xsl:value-of select="@member"/>
	    </xsl:when>
	    <xsl:otherwise>
	      <xsl:value-of select="@name"/>
	    </xsl:otherwise>
	  </xsl:choose>
	</xsl:variable>
	<xsl:for-each select="ancestor::class/member[@name=$member]">
	  <xsl:call-template name="getvalue">
	    <xsl:with-param name="valuePrefix" select="$valuePrefix"/>
	  </xsl:call-template>
	</xsl:for-each>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:for-each select="vector">
      <xsl:call-template name="getvalue">
	<xsl:with-param name="valuePrefix" select="$valuePrefix"/>
      </xsl:call-template>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="sprmactionset">
    <xsl:param name="valuePrefix"/>
    <xsl:text>&#xa;        {</xsl:text>
    <xsl:text>&#xa;            </xsl:text>
    <xsl:call-template name="memberid"/>
    <xsl:text> = </xsl:text>
    <xsl:call-template name="getvalue">
      <xsl:with-param name="valuePrefix" select="$valuePrefix"/>
    </xsl:call-template>
    <xsl:text>;&#xa;        }</xsl:text>
  </xsl:template>

  <xsl:template name="sprmactionpushback">
    <xsl:param name="valuePrefix"/>
    <xsl:text>&#xa;        {</xsl:text>
    <xsl:text>&#xa;            </xsl:text>
    <xsl:call-template name="memberid"/>
    <xsl:text>.push_back(</xsl:text>
    <xsl:call-template name="getvalue">
      <xsl:with-param name="valuePrefix" select="$valuePrefix"/>
    </xsl:call-template>
    <xsl:text>);</xsl:text>
    <xsl:text>;&#xa;        }</xsl:text>
  </xsl:template>

  <xsl:template name="sprmactionresolve">
    <xsl:text>&#xa;        {</xsl:text>
    <xsl:text>&#xa;            resolveSprm(r_Sprm);</xsl:text>
    <xsl:text>&#xa;        }</xsl:text>
  </xsl:template>

  <xsl:template name="sprmaction">
    <xsl:param name="valuePrefix"/>
    <xsl:choose>
      <xsl:when test="@action='set'">
	<xsl:call-template name="sprmactionset">
	  <xsl:with-param name="valuePrefix" select="$valuePrefix"/>
	</xsl:call-template>
      </xsl:when>
      <xsl:when test="@action='pushback'">
	<xsl:call-template name="sprmactionpushback">
	  <xsl:with-param name="valuePrefix" select="$valuePrefix"/>
	</xsl:call-template>
      </xsl:when>
      <xsl:when test="@action='resolve'">
	<xsl:call-template name="sprmactionresolve"/>
      </xsl:when>
      <xsl:otherwise/>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="sprmimpl">
    <xsl:for-each select="sprm">
      <xsl:text>&#xa;void </xsl:text>
      <xsl:variable name="classname" select="ancestor::class/@name"/>
      <xsl:value-of select="$classname"/>
      <xsl:text>::sprm(Sprm &amp; r_Sprm)</xsl:text>
      <xsl:text>&#xa;{</xsl:text>
      <xsl:text>&#xa;#ifdef DEBUG_DOMAINMAPPER</xsl:text>
      <xsl:text>&#xa;    dmapper_logger->startElement("</xsl:text>
      <xsl:value-of select="$classname"/>
      <xsl:text>.sprm");</xsl:text>
      <xsl:text>&#xa;    dmapper_logger->chars(r_Sprm.toString());</xsl:text>
      <xsl:text>&#xa;#endif</xsl:text>
      <xsl:text>&#xa;    switch(r_Sprm.getId())</xsl:text>
      <xsl:text>&#xa;    {</xsl:text>
      <xsl:for-each select="element">
	<xsl:text>&#xa;    case </xsl:text>
	<xsl:value-of select="@id"/>
	<xsl:text>:</xsl:text>
	<xsl:text>&#xa;    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */</xsl:text>
	<xsl:call-template name="sprmaction">
	  <xsl:with-param name="valuePrefix">r_Sprm.getValue()-&gt;</xsl:with-param>
	</xsl:call-template>
	<xsl:text>&#xa;        break;</xsl:text>
      </xsl:for-each>
      <xsl:text>&#xa;    default:</xsl:text>
      <xsl:text>&#xa;#ifdef DEBUG_DOMAINMAPPER</xsl:text>
      <xsl:text>&#xa;        dmapper_logger->element("unhandled");</xsl:text>
      <xsl:text>&#xa;#endif DEBUG_DOMAINMAPPER</xsl:text>
      <xsl:text>&#xa;        break;</xsl:text>
      <xsl:text>&#xa;    }</xsl:text>
      <xsl:text>&#xa;#ifdef DEBUG_DOMAINMAPPER</xsl:text>
      <xsl:text>&#xa;    dmapper_logger->endElement("</xsl:text>
      <xsl:value-of select="$classname"/>
      <xsl:text>.sprm");</xsl:text>
      <xsl:text>&#xa;#endif</xsl:text>
      <xsl:text>&#xa;}&#xa;</xsl:text>
      <xsl:text>&#xa;void </xsl:text>
      <xsl:value-of select="ancestor::class/@name"/>
      <xsl:text>::resolveSprm(Sprm &amp; r_Sprm)</xsl:text>
      <xsl:text>&#xa;{</xsl:text>
      <xsl:text>&#xa;    writerfilter::Reference&lt;Properties&gt;::Pointer_t pProperties = r_Sprm.getProps();</xsl:text>
      <xsl:text>&#xa;    if( pProperties.get())</xsl:text>
      <xsl:text>&#xa;        pProperties->resolve(*this);</xsl:text>
      <xsl:text>&#xa;}&#xa;</xsl:text>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="attrimpl">
    <xsl:for-each select="attribute">
      <xsl:text>&#xa;void </xsl:text>
      <xsl:variable name="classname" select="ancestor::class/@name"/>
      <xsl:value-of select="$classname"/>
      <xsl:text>::</xsl:text>
      <xsl:text>attribute(Id name, Value &amp; val)</xsl:text>
      <xsl:text>&#xa;{</xsl:text>
      <xsl:text>&#xa;#ifdef DEBUG_DOMAINMAPPER</xsl:text>
      <xsl:text>&#xa;    dmapper_logger->startElement("</xsl:text>
      <xsl:value-of select="$classname"/>
      <xsl:text>.attribute");</xsl:text>
      <xsl:text>&#xa;    dmapper_logger->attribute("name", (*QNameToString::Instance())(name));</xsl:text>
      <xsl:text>&#xa;    dmapper_logger->attribute("value", val.toString());</xsl:text>
      <xsl:text>&#xa;    dmapper_logger->endElement("</xsl:text>
      <xsl:value-of select="$classname"/>
      <xsl:text>.attribute");</xsl:text>
      <xsl:text>&#xa;#endif</xsl:text>
      <xsl:text>&#xa;    switch (name)</xsl:text>
      <xsl:text>&#xa;    {</xsl:text>
      <xsl:for-each select="attribute">
	<xsl:text>&#xa;    case </xsl:text>
	<xsl:value-of select="@id"/>
	<xsl:text>:</xsl:text>
	<xsl:text>&#xa;    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */</xsl:text>
	<xsl:call-template name="sprmactionset">
	  <xsl:with-param name="valuePrefix">val.</xsl:with-param>
	</xsl:call-template>
	<xsl:text>&#xa;        break;</xsl:text>
      </xsl:for-each>
      <xsl:text>&#xa;    default:</xsl:text>
      <xsl:text>&#xa;#ifdef DEBUG_DOMAINMAPPER</xsl:text>
      <xsl:text>&#xa;        dmapper_logger->element("unhandled");</xsl:text>
      <xsl:text>&#xa;#endif DEBUG_DOMAINMAPPER</xsl:text>
      <xsl:text>&#xa;        break;</xsl:text>
      <xsl:text>&#xa;    }</xsl:text>
      <xsl:text>&#xa;}&#xa;&#xa;</xsl:text>
    </xsl:for-each>
  </xsl:template>
  
  <xsl:template name="constructorimpl">
    <xsl:text>&#xa;</xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text>::</xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text>()</xsl:text>
    <xsl:variable name="memberinits">
      <xsl:for-each select="member[@type='sal_uInt32' or @type='bool']">
	<xsl:if test="position() > 1">
	  <xsl:text>,</xsl:text>
	  <xsl:text>&#xa;  </xsl:text>
	</xsl:if>
	<xsl:call-template name="memberid"/>	
	<xsl:choose>
	  <xsl:when test="@type='sal_uInt32'">
	    <xsl:text>(0)</xsl:text>
	  </xsl:when>
	  <xsl:when test="@type='bool'">
	    <xsl:text>(false)</xsl:text>
	  </xsl:when>
	</xsl:choose>
      </xsl:for-each>
    </xsl:variable>
    <xsl:if test="string-length($memberinits) > 0">
      <xsl:text>&#xa;: </xsl:text>
      <xsl:value-of select="$memberinits"/>
    </xsl:if>
    <xsl:text>&#xa;{</xsl:text>
    <xsl:text>&#xa;}&#xa;&#xa;</xsl:text>
  </xsl:template>
  
  <xsl:template name="destructorimpl">
    <xsl:text>&#xa;</xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text>::~</xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text>()</xsl:text>
    <xsl:text>&#xa;{</xsl:text>
    <xsl:text>&#xa;}&#xa;&#xa;</xsl:text>
  </xsl:template>
  
  <xsl:template name="classimpl">
    <xsl:variable name="classname" select="@name"/>
    <xsl:variable name="comment">
      <xsl:text>class: </xsl:text>
      <xsl:value-of select="$classname"/>
    </xsl:variable>
    <xsl:variable name="commentfill">
      <xsl:value-of select="translate($comment, 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ: ', 
			                        '******************************************************')"/>
    </xsl:variable>
    <xsl:text>&#xa;/***</xsl:text>
    <xsl:value-of select="$commentfill"/>
    <xsl:text>*</xsl:text>
    <xsl:text>&#xa; * </xsl:text>
    <xsl:value-of select="$comment"/>
    <xsl:text> *</xsl:text>
    <xsl:text>&#xa; **</xsl:text>
    <xsl:value-of select="$commentfill"/>
    <xsl:text>**/&#xa;</xsl:text>
    <xsl:call-template name="constructorimpl"/>
    <xsl:call-template name="destructorimpl"/>
    <xsl:for-each select="member">
      <xsl:text>// member: </xsl:text>
      <xsl:value-of select="$classname"/>
      <xsl:text>::</xsl:text>
      <xsl:value-of select="@name"/>
      <xsl:text>&#xa;</xsl:text>
      <xsl:call-template name="setterimpl"/>
      <xsl:call-template name="getterimpl"/>
      <xsl:call-template name="pushbackimpl"/>
    </xsl:for-each>
    <xsl:call-template name="sprmimpl"/>
    <xsl:call-template name="attrimpl"/>
  </xsl:template>
</xsl:stylesheet>
