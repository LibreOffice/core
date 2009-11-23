<xsl:stylesheet 
    version="1.0" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
    xmlns:UML = 'org.omg.xmi.namespace.UML' 
    xmlns:exslt="http://exslt.org/common"
    xml:space="default">
  <xsl:output method="xml"/>

  <xsl:template match="/">
    <xsl:apply-templates select="//UML:Model"/>
  </xsl:template>

  <xsl:template match="UML:Model">
    <model>
      <xsl:attribute name="name">
	<xsl:value-of select="@name"/>
      </xsl:attribute>
      <xsl:call-template name="nostereotype"/>
      <xsl:call-template name="ww8resources"/>
      <xsl:call-template name="sprms"/>
    </model>
  </xsl:template>

  <xsl:template name="nostereotype">
    <xsl:for-each select="UML:Namespace.ownedElement/UML:Class[not(UML:ModelElement.stereotype)]">
      <xsl:variable name="myid" select="@xmi.id"/>
      <class>
	<xsl:attribute name="name">
	  <xsl:value-of select="$myid"/>
	</xsl:attribute>
	<xsl:attribute name="resource">
	<xsl:for-each select="//UML:Generalization[UML:Generalization.child/UML:Class/@xmi.idref=$myid]">
	  <xsl:value-of select="UML:Generalization.parent/UML:Class/@xmi.idref"/>
	</xsl:for-each>
      </xsl:attribute>
      <xsl:apply-templates select=".//UML:Attribute|.//UML:Operation" mode="ww8resource"/>
      </class>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="ww8resources">
    <xsl:for-each select="UML:Namespace.ownedElement/UML:Class[UML:ModelElement.stereotype/UML:Stereotype/@xmi.idref='ww8resource' and not(UML:ModelElement.stereotype/UML:Stereotype/@xmi.idref='ww8sprm')]">
      <xsl:sort select="@xmi.id"/>
      <xsl:call-template name="ww8resource"/>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="ww8resource">
    <xsl:variable name="myid" select="@xmi.id"/>
    <xsl:variable name="classid">
      <xsl:call-template name="gettaggedvalue">
	<xsl:with-param name="name">classid</xsl:with-param>
      </xsl:call-template>
    </xsl:variable>
    <ww8resource>
      <xsl:attribute name="name">
	<xsl:value-of select="$myid"/>
      </xsl:attribute>
      <xsl:attribute name="resource">
	<xsl:for-each select="//UML:Generalization[UML:Generalization.child/UML:Class/@xmi.idref=$myid]">
	  <xsl:value-of select="UML:Generalization.parent/UML:Class/@xmi.idref"/>
	</xsl:for-each>
      </xsl:attribute>
      <xsl:if test="string-length($classid) &gt; 0">
	<xsl:attribute name="classid">
	  <xsl:value-of select="$classid"/>
	</xsl:attribute>
      </xsl:if>
      <xsl:for-each select="UML:ModelElement.stereotype/UML:Stereotype">
	<xsl:if test="@xmi.idref != 'ww8resource'">
	  <stereotype>
	    <xsl:attribute name="name">
	      <xsl:value-of select="@xmi.idref"/>
	    </xsl:attribute>
	  </stereotype>
	</xsl:if>
      </xsl:for-each>
      <xsl:apply-templates select=".//UML:Attribute|.//UML:Operation" mode="ww8resource"/>
    </ww8resource>
  </xsl:template>

  <xsl:template name="gettaggedvalue">
    <xsl:param name="name"/>
    <xsl:for-each select="UML:ModelElement.taggedValue/UML:TaggedValue[UML:TaggedValue.type/UML:TagDefinition/@xmi.idref=$name]">
      <xsl:value-of select="UML:TaggedValue.dataValue"/>
    </xsl:for-each>
  </xsl:template>
  
  <xsl:template name="sizefortype">
    <xsl:param name="type"/>
    <xsl:choose>
      <xsl:when test="$type='S8'">1</xsl:when>
      <xsl:when test="$type='U8'">1</xsl:when>
      <xsl:when test="$type='S16'">2</xsl:when>
      <xsl:when test="$type='U16'">2</xsl:when>
      <xsl:when test="$type='S32'">4</xsl:when>
      <xsl:when test="$type='U32'">4</xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="bits">
    <xsl:param name="bits"/>
    <bits>
      <xsl:attribute name="name">
	<xsl:value-of select="@name"/>
      </xsl:attribute>
      <xsl:attribute name="bits">
	<xsl:value-of select="$bits"/>
      </xsl:attribute>
      <xsl:attribute name="token">
	<xsl:call-template name="gettaggedvalue">
	  <xsl:with-param name="name">attrid</xsl:with-param>
	</xsl:call-template>
      </xsl:attribute>
    </bits>
  </xsl:template>

  <xsl:template match="UML:Attribute" mode="ww8resource">
    <xsl:variable name="mask">
      <xsl:call-template name="gettaggedvalue">
	<xsl:with-param name="name">mask</xsl:with-param>
      </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="offset">
      <xsl:call-template name="gettaggedvalue">
	<xsl:with-param name="name">offset</xsl:with-param>
      </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="type">
      <xsl:value-of select="UML:StructuralFeature.type/UML:DataType/@xmi.idref"/>
    </xsl:variable>
    <xsl:variable name="size">
      <xsl:call-template name="sizefortype">
	<xsl:with-param name="type" select="$type"/>
      </xsl:call-template>
    </xsl:variable>
    <xsl:choose>
      <xsl:when test="string-length($mask) = 0">
	<attribute>
	  <xsl:attribute name="name">
	    <xsl:value-of select="@name"/>
	  </xsl:attribute>
	  <xsl:attribute name="type">
	    <xsl:value-of select="$type"/>
	  </xsl:attribute>
	  <xsl:attribute name="size">
	    <xsl:value-of select="$size"/>
	  </xsl:attribute>
	  <xsl:attribute name="token">
	    <xsl:call-template name="gettaggedvalue">
	      <xsl:with-param name="name">attrid</xsl:with-param>
	    </xsl:call-template>
	  </xsl:attribute>
	</attribute>
      </xsl:when>
      <xsl:otherwise>
	<xsl:variable name="attrswithoffset">
	  <xsl:for-each select="ancestor::UML:Class//UML:Attribute[UML:ModelElement.taggedValue/UML:TaggedValue[UML:TaggedValue.type/UML:TagDefinition/@xmi.idref='offset' and UML:TaggedValue.dataValue=$offset]]">
	    <xsl:sort select="UML:ModelElement.taggedValue/UML:TaggedValue[UML:TaggedValue.type/UML:TagDefinition/@xmi.idref='shift']/UML:TaggedValue.dataValue" data-type="number" order="descending"/>
	    <xsl:copy-of select="."/>
	  </xsl:for-each>
	</xsl:variable>
	<xsl:text>&#xa;</xsl:text>
	<xsl:if test="@name = exslt:node-set($attrswithoffset)/UML:Attribute[1]/@name">
	  <bitfield>
	    <xsl:attribute name="size">
	      <xsl:value-of select="$size"/>
	    </xsl:attribute>
	    <xsl:attribute name="type">
	      <xsl:value-of select="$type"/>
	    </xsl:attribute>
	    <xsl:variable name="allbits" select="8 * $size"/>
	    <xsl:for-each select="exslt:node-set($attrswithoffset)/UML:Attribute">
	      <xsl:variable name="lastshift">
		<xsl:choose>
		  <xsl:when test="count(preceding-sibling::UML:Attribute) = 0">
		    <xsl:value-of select="$allbits"/>
		  </xsl:when>
		  <xsl:otherwise>
		    <xsl:value-of select="preceding-sibling::UML:Attribute[1]/UML:ModelElement.taggedValue/UML:TaggedValue[UML:TaggedValue.type/UML:TagDefinition/@xmi.idref='shift']/UML:TaggedValue.dataValue"/>
		  </xsl:otherwise>
		</xsl:choose>
	      </xsl:variable>
	      <xsl:variable name="bits" select="$lastshift - UML:ModelElement.taggedValue/UML:TaggedValue[UML:TaggedValue.type/UML:TagDefinition/@xmi.idref='shift']/UML:TaggedValue.dataValue"/>
	      <xsl:call-template name="bits">
		<xsl:with-param name="bits" select="$bits"/>
	      </xsl:call-template>
	    </xsl:for-each>
	  </bitfield>
	</xsl:if>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="UML:Operation" mode="ww8resource">
    <operation>
      <xsl:attribute name="name">
	<xsl:value-of select="@name"/>
      </xsl:attribute>
      <xsl:attribute name="type">
	<xsl:for-each select="UML:BehavioralFeature.parameter/UML:Parameter[@kind='return']">
	  <xsl:value-of select="UML:Parameter.type/UML:Class/@xmi.idref"/>
	</xsl:for-each>
      </xsl:attribute>
      <xsl:attribute name="token">
	<xsl:call-template name="gettaggedvalue">
	  <xsl:with-param name="name">opid</xsl:with-param>
	</xsl:call-template>
      </xsl:attribute>
      <xsl:for-each select="UML:ModelElement.stereotype">
	<stereotype>
	  <xsl:value-of select="UML:Stereotype/@xmi.idref"/>
	</stereotype>
      </xsl:for-each>
    </operation>
  </xsl:template>

  <xsl:template match="*" mode="ww8resources">
    <xsl:apply-templates select="*" mode="ww8resources"/>
  </xsl:template>

  <xsl:template name="sprms">
    <xsl:for-each select="UML:Namespace.ownedElement/UML:Class[UML:ModelElement.stereotype/UML:Stereotype/@xmi.idref='ww8sprm']">
      <xsl:sort select="@xmi.id"/>
      <sprm>
	<xsl:attribute name="name">
	  <xsl:value-of select="@xmi.id"/>
	</xsl:attribute>
	<xsl:attribute name="code">
	  <xsl:call-template name="gettaggedvalue">
	    <xsl:with-param name="name">sprmcode</xsl:with-param>
	  </xsl:call-template>
	</xsl:attribute>
	<xsl:attribute name="kind">
	  <xsl:call-template name="gettaggedvalue">
	    <xsl:with-param name="name">kind</xsl:with-param>
	  </xsl:call-template>
	</xsl:attribute>
	<xsl:attribute name="token">
	  <xsl:call-template name="gettaggedvalue">
	    <xsl:with-param name="name">sprmid</xsl:with-param>
	  </xsl:call-template>
	</xsl:attribute>
	<xsl:if test="UML:ModelElement.stereotype/UML:Stereotype/@xmi.idref='ww8resource'">
	  <xsl:apply-templates select=".//UML:Attribute|.//UML:Operation" mode="ww8resource"/>
	</xsl:if>
      </sprm>
    </xsl:for-each>
  </xsl:template>

</xsl:stylesheet>