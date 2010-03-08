<xsl:stylesheet 
    version="1.0" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
    xmlns:rng="http://relaxng.org/ns/structure/1.0"
    xmlns:xalan="http://xml.apache.org/xalan"
    exclude-result-prefixes = "xalan"
    xml:indent="true">
  <xsl:output method="text" version="1.0" encoding="UTF-8" indent="yes" omit-xml-declaration="no"/>

  <xsl:template match="/">
    <xsl:for-each select="/model">
      <xsl:apply-templates/>
    </xsl:for-each>
  </xsl:template>

  <xsl:template match="class">
    <xsl:text>
class </xsl:text>
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
<xsl:text>
{
public:</xsl:text>
<xsl:call-template name="typedefs"/>

<xsl:text>
    FFData();
    ~FFData();
</xsl:text>
<xsl:call-template name="memberfuncdecls"/>
<xsl:text>
};
</xsl:text>
  </xsl:template>

  <xsl:template name="typedefs">
    <xsl:for-each select="typedef">
      <xsl:text>
    typedef </xsl:text>
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
    <xsl:text>void </xsl:text>
    <xsl:call-template name="settername"/>
    <xsl:text>(</xsl:text>
    <xsl:call-template name="cctype"/>
    <xsl:text> </xsl:text>
    <xsl:call-template name="paramname"/>
    <xsl:text>)</xsl:text>
  </xsl:template>

  <xsl:template name="gettersig">
    <xsl:variable name="type" select="@type"/>
    <xsl:call-template name="cctype"/>
    <xsl:text> </xsl:text>
    <xsl:call-template name="gettername"/>
    <xsl:text>() const</xsl:text>
  </xsl:template>

  <xsl:template name="pushbacksig">
    <xsl:variable name="pushback">
      <xsl:call-template name="pushbackname"/>
    </xsl:variable>
    <xsl:variable name="type" select="@type"/>
    <xsl:for-each select="ancestor::class/typedef[@name=$type]">
      <xsl:for-each select="vector">
	<xsl:text>void </xsl:text>
	<xsl:value-of select="$pushback"/>
	<xsl:text>(</xsl:text>
	<xsl:call-template name="cctype"/>
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
      <xsl:text>&#xa;    </xsl:text>
      <xsl:call-template name="pushbacksig"/>
      <xsl:text>;&#xa;</xsl:text>
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
</xsl:stylesheet>
