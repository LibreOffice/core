<!--
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: resources.xsl,v $
 *
 * $Revision: 1.9 $
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
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

-->
<xsl:stylesheet version="1.0" 
		xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
		xmlns:UML = 'org.omg.xmi.namespace.UML' 
		xml:space="default">
  <xsl:output method="xml"/>

  <xsl:template match="/">
    <out>
      <xsl:apply-templates select="*"/>
    </out>
  </xsl:template>

  <xsl:template name="calcsizeinner">
    <xsl:param name="size">0</xsl:param>
    <xsl:variable name="arraycount">
      <xsl:choose>
	<xsl:when test="@array-count">
	  <xsl:value-of select="@array-count"/>
	</xsl:when>
	<xsl:otherwise>1</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="mysize">
      <xsl:choose>
	<xsl:when test="@size">
	  <xsl:value-of select="number($arraycount) * number(@size)"/>
	</xsl:when>
	<xsl:otherwise>0</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:choose>
      <xsl:when test="following-sibling::*">
	<xsl:for-each select="following-sibling::*[1]">
	  <xsl:call-template name="calcsizeinner">
	    <xsl:with-param name="size" 
			    select="number($size) + number($mysize)"/>
	  </xsl:call-template>
	</xsl:for-each>
      </xsl:when>
      <xsl:otherwise>
	<xsl:value-of select="number($size) + number($mysize)"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="calcsize">
    <xsl:for-each select="*[1]">
      <xsl:call-template name="calcsizeinner"/>
    </xsl:for-each>
  </xsl:template>

  <xsl:template match="ww8resource">
    <UML:Class>
      <xsl:attribute name="xmi.id">
	<xsl:value-of select="@name"/>
      </xsl:attribute>
      <xsl:attribute name="name">
	<xsl:value-of select="@name"/>
      </xsl:attribute>
      <UML:ModelElement.stereotype>
	<UML:Stereotype xmi.idref="ww8resource"/>
      </UML:ModelElement.stereotype>
      <xsl:for-each select="stereotype">
	<UML:ModelElement.stereotype>
	  <UML:Stereotype>
	    <xsl:value-of select="@name"/>
	  </UML:Stereotype>
	</UML:ModelElement.stereotype>
      </xsl:for-each>
      <UML:ModelElement.taggedValue>
	<UML:TaggedValue>
	  <UML:TaggedValue.dataValue>
	    <xsl:call-template name="calcsize"/>
	  </UML:TaggedValue.dataValue>
	  <UML:TaggedValue.type>
	    <UML:TagDefinition xmi.idref="size"/>
	  </UML:TaggedValue.type>
	</UML:TaggedValue>
      </UML:ModelElement.taggedValue>

      <xsl:apply-templates select="*"/>

    </UML:Class>
    <UML:Generalization isSpecification="false">
      <xsl:attribute name="xmi.id">
	<xsl:value-of select="@name"/>
	<xsl:text>-</xsl:text>
	<xsl:value-of select="@resource"/>
      </xsl:attribute>
      <UML:Generalization.child>
	<UML:Class>
	  <xsl:attribute name="xmi.idref">
	    <xsl:value-of select="@name"/>
	  </xsl:attribute>
	</UML:Class>
      </UML:Generalization.child>
      <UML:Generalization.parent>
	<UML:Class xmi.idref="Properties">
	  <xsl:attribute name="xmi.idref">
	    <xsl:value-of select="@resource"/>
	  </xsl:attribute>
	</UML:Class>
      </UML:Generalization.parent>
    </UML:Generalization>
  </xsl:template>

  <xsl:template name="taggedvalue">
    <xsl:param name="type"/>
    <xsl:param name="value"/>
    <UML:ModelElement.taggedValue>
      <UML:TaggedValue>
	<UML:TaggedValue.dataValue>
	  <xsl:value-of select="$value"/>
	</UML:TaggedValue.dataValue>
	<UML:TaggedValue.type>
	  <UML:TagDefinition>
	    <xsl:attribute name="xmi.idref">
	      <xsl:value-of select="$type"/>
	    </xsl:attribute>
	  </UML:TagDefinition>
	</UML:TaggedValue.type>
      </UML:TaggedValue>
    </UML:ModelElement.taggedValue>    
  </xsl:template>

  <xsl:template match="attribute">
    <UML:Classifier.feature>
      <UML:Attribute>
	<xsl:attribute name="name">
	  <xsl:value-of select="@name"/>
	</xsl:attribute>
	<xsl:call-template name="taggedvalue">
	  <xsl:with-param name="type">comment</xsl:with-param>
	  <xsl:with-param name="value"/>
	</xsl:call-template>
	<xsl:call-template name="taggedvalue">
	  <xsl:with-param name="type">offset</xsl:with-param>
	  <xsl:with-param name="value">
	    <xsl:value-of select="sum(preceding-sibling::*/@size)"/>
	  </xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="taggedvalue">
	  <xsl:with-param name="type">shift</xsl:with-param>
	  <xsl:with-param name="value"></xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="taggedvalue">
	  <xsl:with-param name="type">mask</xsl:with-param>
	  <xsl:with-param name="value"></xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="taggedvalue">
	  <xsl:with-param name="type">bits</xsl:with-param>
	  <xsl:with-param name="value"></xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="taggedvalue">
	  <xsl:with-param name="type">attrid</xsl:with-param>
	  <xsl:with-param name="value">
	    <xsl:text>rtf:</xsl:text>
	    <xsl:value-of select="@name"/>
	  </xsl:with-param>
	</xsl:call-template>
	<xsl:if test="@array-count">
	  <xsl:call-template name="taggedvalue">
	    <xsl:with-param name="type">arraycount</xsl:with-param>
	    <xsl:with-param name="value">
	      <xsl:value-of select="@array-count"/>
	    </xsl:with-param>
	  </xsl:call-template>
	</xsl:if>
	<UML:StructuralFeature.type>
	  <UML:DataType>
	    <xsl:attribute name="xmi.idref">
	      <xsl:value-of select="@type"/>
	    </xsl:attribute>
	  </UML:DataType>
	</UML:StructuralFeature.type>
	<xsl:choose>
	  <xsl:when test="@array-count">
	    <UML:ModelElement.stereotype>
	      <UML:Stereotype xmi.idref="array"/>
	    </UML:ModelElement.stereotype>
	  </xsl:when>
	  <xsl:otherwise>
	    <UML:ModelElement.stereotype>
	      <UML:Stereotype xmi.idref="attribute"/>
	    </UML:ModelElement.stereotype>
	  </xsl:otherwise>
	</xsl:choose>
      </UML:Attribute>
    </UML:Classifier.feature>
  </xsl:template>

  <xsl:template match="bitfield">
    <xsl:variable name="offset">
      <xsl:value-of select="sum(preceding-sibling::*/@size)"/>
    </xsl:variable>
    <xsl:for-each select="bits">
      <xsl:call-template name="bits">
	<xsl:with-param name="offset" select="$offset"/>
      </xsl:call-template>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="mask">
    <xsl:param name="count"/>
    <xsl:param name="value"/>

    <xsl:choose>
      <xsl:when test="number($count) = 1">
	<xsl:value-of select="$value"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:call-template name="mask">
	  <xsl:with-param name="count" select="number($count) - 1"/>
	  <xsl:with-param name="value" select="number($value) * 2 + 1"/>
	</xsl:call-template>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="shift">
    <xsl:param name="shift"/>
    <xsl:param name="value"/>

    <xsl:choose>
      <xsl:when test="number($shift) = 0">
	<xsl:value-of select="$value"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:call-template name="shift">
	  <xsl:with-param name="shift" select="number($shift) - 1"/>
	  <xsl:with-param name="value" select="$value * 2"/>
	</xsl:call-template>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name='dectohex'>
    <xsl:param name="number"/>
    <xsl:if test="$number > 16">
      <xsl:call-template name="dectohex">
	<xsl:with-param name="number" select="floor($number div 16)"/>
      </xsl:call-template>
    </xsl:if>
    <xsl:value-of select="substring('0123456789abcdef', $number mod 16 + 1, 1)"/>
  </xsl:template>

  <xsl:template name="bits">
    <xsl:param name="offset"/>
    <xsl:variable name="shift">
      <xsl:value-of select="sum(following-sibling::*/@bits)"/>
    </xsl:variable>
    <xsl:variable name="mask">
      <xsl:call-template name="mask">
	<xsl:with-param name="count" select="@bits"/>
	<xsl:with-param name="value" select="1"/>
      </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="maskshifted">
      <xsl:call-template name="shift">
	<xsl:with-param name="shift" select="$shift"/>
	<xsl:with-param name="value" select="$mask"/>
      </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="maskhex">
      <xsl:text>0x</xsl:text>
      <xsl:call-template name="dectohex">
	<xsl:with-param name="number" select="$maskshifted"/>
      </xsl:call-template>
    </xsl:variable>
    <UML:Classifier.feature>
      <UML:Attribute>
	<xsl:attribute name="name">
	  <xsl:value-of select="@name"/>
	</xsl:attribute>
	<UML:ModelElement.taggedValue>
	  <UML:TaggedValue>
	    <UML:TaggedValue.dataValue/>
	    <UML:TaggedValue.type>
	      <UML:TagDefinition xmi.idref="comment"/>
	    </UML:TaggedValue.type>
	  </UML:TaggedValue>
	</UML:ModelElement.taggedValue>
	<UML:ModelElement.taggedValue>
	  <UML:TaggedValue>
	    <UML:TaggedValue.dataValue>
	      <xsl:value-of select="$offset"/>
	    </UML:TaggedValue.dataValue>
	    <UML:TaggedValue.type>
	      <UML:TagDefinition xmi.idref="offset"/>
	    </UML:TaggedValue.type>
	  </UML:TaggedValue>
	</UML:ModelElement.taggedValue>
	<UML:ModelElement.taggedValue>
	  <UML:TaggedValue>
	    <UML:TaggedValue.dataValue>
	      <xsl:value-of select="$shift"/>
	    </UML:TaggedValue.dataValue>
	    <UML:TaggedValue.type>
	      <UML:TagDefinition xmi.idref="shift"/>
	    </UML:TaggedValue.type>
	  </UML:TaggedValue>
	</UML:ModelElement.taggedValue>
	<UML:ModelElement.taggedValue>
	  <UML:TaggedValue>
	    <UML:TaggedValue.dataValue>
	      <xsl:value-of select="$maskhex"/>
	    </UML:TaggedValue.dataValue>
	    <UML:TaggedValue.type>
	      <UML:TagDefinition xmi.idref="mask"/>
	    </UML:TaggedValue.type>
	  </UML:TaggedValue>
	</UML:ModelElement.taggedValue>
	<UML:ModelElement.taggedValue>
	  <UML:TaggedValue>
	    <UML:TaggedValue.dataValue>
	      <xsl:value-of select="@bits"/>
	    </UML:TaggedValue.dataValue>
	    <UML:TaggedValue.type>
	      <UML:TagDefinition xmi.idref="bits"/>
	    </UML:TaggedValue.type>
	  </UML:TaggedValue>
	</UML:ModelElement.taggedValue>
	<UML:ModelElement.taggedValue>
	  <UML:TaggedValue>
	    <UML:TaggedValue.dataValue>
	      <xsl:text>rtf:</xsl:text>
	      <xsl:value-of select="@name"/>
	    </UML:TaggedValue.dataValue>
	    <UML:TaggedValue.type>
	      <UML:TagDefinition xmi.idref="attrid"/>
	    </UML:TaggedValue.type>
	  </UML:TaggedValue>
	</UML:ModelElement.taggedValue>
	<UML:StructuralFeature.type>
	  <UML:DataType>
	    <xsl:attribute name="xmi.idref">
	      <xsl:value-of select="ancestor::bitfield/@type"/>
	    </xsl:attribute>
	  </UML:DataType>
	</UML:StructuralFeature.type>
	<UML:ModelElement.stereotype>
	  <UML:Stereotype xmi.idref="attribute"/>
	</UML:ModelElement.stereotype>
      </UML:Attribute>
    </UML:Classifier.feature>
  </xsl:template>

  <xsl:template match="operation">
    <UML:Classifier.feature>
      <UML:Operation>
	<xsl:attribute name="name">
	  <xsl:value-of select="@name"/>
	</xsl:attribute>
	<xsl:call-template name="taggedvalue">
	  <xsl:with-param name="type">opid</xsl:with-param>
	  <xsl:with-param name="value">
	    <xsl:choose>
	      <xsl:when test="@token">
		<xsl:value-of select="@token"/>
	      </xsl:when>
	      <xsl:otherwise>
		<xsl:text>rtf:</xsl:text>
		<xsl:value-of select="@name"/>		
	      </xsl:otherwise>
	    </xsl:choose>
	  </xsl:with-param>
	</xsl:call-template>
	<xsl:if test="@array-count">
	  <xsl:call-template name="taggedvalue">
	    <xsl:with-param name="type">arraycount</xsl:with-param>
	    <xsl:with-param name="value">
	      <xsl:value-of select="@array-count"/>
	    </xsl:with-param>
	  </xsl:call-template>	  
	</xsl:if>
	<UML:BehavioralFeature.parameter>
	  <UML:Parameter kind="return" name="return">
	    <UML:Parameter.type>
	      <UML:Class>
		<xsl:attribute name="xmi.idref">
		  <xsl:value-of select="@type"/>
		</xsl:attribute>
	      </UML:Class>
	    </UML:Parameter.type>
	  </UML:Parameter>
	</UML:BehavioralFeature.parameter>
	<xsl:choose>
	  <xsl:when test="@array-count">
	    <UML:ModelElement.stereotype>
	      <UML:Stereotype xmi.idref="array"/>
	    </UML:ModelElement.stereotype>
	  </xsl:when>
	  <xsl:otherwise>
	    <UML:ModelElement.stereotype>
	      <UML:Stereotype xmi.idref="attribute"/>
	    </UML:ModelElement.stereotype>
	  </xsl:otherwise>
	</xsl:choose>
      </UML:Operation>
    </UML:Classifier.feature>
  </xsl:template>
</xsl:stylesheet>