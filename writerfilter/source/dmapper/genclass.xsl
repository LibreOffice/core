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
      <xsl:call-template name="classdecl"/>
      <xsl:call-template name="classimpl"/>
    </xsl:for-each>
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
    <xsl:text>&#xa;    FFData();</xsl:text>
    <xsl:text>&#xa;    // destructor</xsl:text>
    <xsl:text>&#xa;    virtual ~FFData();&#xa;</xsl:text>
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
	<xsl:value-of select="@type"/>
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
    <xsl:text>void </xsl:text>
    <xsl:if test="string-length($classname) > 0">
      <xsl:value-of select="$classname"/>
      <xsl:text>::</xsl:text>
    </xsl:if>
    <xsl:call-template name="settername"/>
    <xsl:text>(</xsl:text>
    <xsl:call-template name="cctype"/>
    <xsl:text> </xsl:text>
    <xsl:call-template name="paramname"/>
    <xsl:text>)</xsl:text>
  </xsl:template>

  <xsl:template name="gettersig">
    <xsl:param name="classname"/>    
    <xsl:variable name="type" select="@type"/>
    <xsl:call-template name="cctype"/>
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
	<xsl:call-template name="cctype"/>
	<xsl:text>r_Element</xsl:text>
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
      <xsl:value-of select="@type"/>
      <xsl:text> </xsl:text>
      <xsl:call-template name="memberid"/>
      <xsl:text>;</xsl:text>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="cctype">
    <xsl:variable name="type" select="@type"/>
    <xsl:variable name="try1">
      <xsl:for-each select="ancestor::class/typedef[@name=$type]">
	<xsl:for-each select="vector">
	  <xsl:text>const </xsl:text>
	  <xsl:value-of select="$type"/>
	  <xsl:text> &amp;</xsl:text>
	</xsl:for-each>
      </xsl:for-each>
    </xsl:variable>
    <xsl:choose>
      <xsl:when test="string-length($try1) > 0">
	<xsl:value-of select="$try1"/>
      </xsl:when>
      <xsl:when test="@type='OUString'">
	<xsl:text>const ::rtl::OUString &amp;</xsl:text>
      </xsl:when>
      <xsl:otherwise>
	<xsl:value-of select="@type"/>
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
      <xsl:text>&#xa;    dmapper_logger->chars(rSprm.toString());</xsl:text>
      <xsl:text>&#xa;#endif</xsl:text>
      <xsl:text>&#xa;    switch(r_Sprm.getId())</xsl:text>
      <xsl:text>&#xa;    {</xsl:text>
      <xsl:for-each select="element">
	<xsl:text>&#xa;    case </xsl:text>
	<xsl:value-of select="@id"/>
	<xsl:text>:</xsl:text>
	<xsl:text>&#xa;    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */</xsl:text>
	<xsl:call-template name="sprmaction">
	  <xsl:with-param name="valuePrefix">r_Sprm.getValue-&gt;</xsl:with-param>
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
      <xsl:text>&#xa;    writerfilter::Reference&lt;Properties&gt;::Pointer_t pProperties = rSprm.getProps();</xsl:text>
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
    
  <xsl:template name="classimpl">
    <xsl:variable name="classname" select="@name"/>
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
