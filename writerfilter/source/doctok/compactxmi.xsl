<xsl:stylesheet 
    version="1.0" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
    xmlns:UML = 'org.omg.xmi.namespace.UML' 
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
      <xsl:for-each select="UML:Namespace.ownedElement">
	<xsl:comment>Stereotypes</xsl:comment>
	<xsl:apply-templates select="./UML:Stereotype"/>
	<xsl:comment>Stereotypes</xsl:comment>
	<xsl:comment>Datatypes</xsl:comment>
	<xsl:apply-templates select="./UML:DataType"/>
	<xsl:comment>Datatypes</xsl:comment>
	<xsl:comment>Tag definitions</xsl:comment>
	<xsl:apply-templates select="./UML:TagDefinition"/>
	<xsl:comment>Tag definitions</xsl:comment>
	<xsl:comment>Classes</xsl:comment>
	<xsl:apply-templates select="./UML:Class"/>
	<xsl:comment>Classes</xsl:comment>
      </xsl:for-each>
      <xsl:for-each select="key('parentforchild', 'DffRecord')">
	<test/>
      </xsl:for-each>
    </model>
  </xsl:template>

  <xsl:template match="UML:Stereotype">
    <stereotype>
      <xsl:copy-of select="@*"/>
      <xsl:value-of select="UML:Stereotype.baseClass"/>
    </stereotype>
  </xsl:template>

  <xsl:template match="UML:DataType">
    <datatype>
      <xsl:copy-of select="@*"/>
    </datatype>
  </xsl:template>

  <xsl:template match="UML:TagDefinition">
    <tagDefinition>
      <xsl:copy-of select="@*"/>
    </tagDefinition>
  </xsl:template>

  <xsl:template match="UML:Class">
    <xsl:variable name="id" select="@xmi.id"/>
    <class>
      <xsl:attribute name="name">
	<xsl:value-of select="@name"/>
      </xsl:attribute>
      <xsl:for-each select="//UML:Generalization[UML:Generalization.child/UML:Class/@xmi.idref=$id]">
	<parent>
	  <xsl:value-of select="UML:Generalization.parent/UML:Class/@xmi.idref"/>	
	</parent>
      </xsl:for-each>
      <xsl:for-each select="UML:ModelElement.stereotype/UML:Stereotype">
	<stereotype>
	  <xsl:value-of select="@xmi.idref"/>
	</stereotype>
      </xsl:for-each>
      <xsl:for-each select="UML:ModelElement.taggedValue/UML:TaggedValue">
	<taggedValue>
	  <xsl:attribute name="type">
	    <xsl:value-of select="UML:TaggedValue.type/UML:TagDefinition/@xmi.idref"/>
	  </xsl:attribute>
	  <xsl:value-of select="UML:TaggedValue.dataValue"/>
	</taggedValue>
      </xsl:for-each>
      <xsl:apply-templates select="UML:Classifier.feature/UML:Attribute"/>
    </class>
  </xsl:template>

  <xsl:template match="UML:Attribute">
    <attribute>
      <xsl:attribute name="name">
	<xsl:value-of select="@name"/>
      </xsl:attribute>
      <xsl:for-each select="UML:ModelElement.taggedValue">
	<xsl:for-each select="UML:TaggedValue[UML:TaggedValue.type/UML:TagDefinition/@xmi.idref='offset']">
	  <xsl:attribute name="offset">
	    <xsl:value-of select="UML:TaggedValue.dataValue"/>
	  </xsl:attribute>
	</xsl:for-each>
	<xsl:for-each select="UML:TaggedValue[UML:TaggedValue.type/UML:TagDefinition/@xmi.idref='mask']">
	  <xsl:attribute name="mask">
	    <xsl:value-of select="UML:TaggedValue.dataValue"/>
	  </xsl:attribute>
	</xsl:for-each>
	<xsl:for-each select="UML:TaggedValue[UML:TaggedValue.type/UML:TagDefinition/@xmi.idref='shift']">
	  <xsl:attribute name="shift">
	    <xsl:value-of select="UML:TaggedValue.dataValue"/>
	  </xsl:attribute>
	</xsl:for-each>
	<xsl:for-each select="UML:TaggedValue[UML:TaggedValue.type/UML:TagDefinition/@xmi.idref='bits']">
	  <xsl:attribute name="bits">
	    <xsl:value-of select="UML:TaggedValue.dataValue"/>
	  </xsl:attribute>
	</xsl:for-each>
	<xsl:for-each select="UML:TaggedValue[UML:TaggedValue.type/UML:TagDefinition/@xmi.idref='attrid']">
	  <xsl:attribute name="attrid">
	    <xsl:value-of select="UML:TaggedValue.dataValue"/>
	  </xsl:attribute>
	</xsl:for-each>
      </xsl:for-each>
      <xsl:for-each select="UML:StructuralFeature.type/UML:DataType">
	<type>
	  <xsl:value-of select="@xmi.idref"/>
	</type>
      </xsl:for-each>
      <xsl:for-each select="UML:ModelElement.stereotype/UML:Stereotype">
	<stereotype>
	  <xsl:value-of select="@xmi.idref"/>
	</stereotype>
      </xsl:for-each>
      <xsl:for-each select="UML:ModelElement.taggedValue">
	<xsl:for-each select="UML:TaggedValue[UML:TaggedValue.type/UML:TagDefinition/@xmi.idref='comment']">
	  <xsl:element name="comment">
	    <xsl:value-of select="UML:TaggedValue.dataValue"/>
	  </xsl:element>
	</xsl:for-each>
      </xsl:for-each>
    </attribute>
  </xsl:template>
</xsl:stylesheet>