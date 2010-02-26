<!--
/*************************************************************************
 *
  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
  
  Copyright 2000, 2010 Oracle and/or its affiliates.
 
  OpenOffice.org - a multi-platform office productivity suite
 
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
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:UML = 'org.omg.xmi.namespace.UML' xml:space="default">
<xsl:output indent="yes"/>
<xsl:template match="/">
<XMI xmi.version="1.2">
  <XMI.header>
    <XMI.documentation/>
  </XMI.header>
  <XMI.content>
    <UML:Model xmlns:UML="org.omg.xmi.namespace.UML" name="WW8Document" >
      <UML:Namespace.ownedElement>
        <xsl:apply-templates select="XMI/XMI.content/UML:Model/UML:Namespace.ownedElement"/>
      </UML:Namespace.ownedElement>
    </UML:Model>
  </XMI.content>
</XMI>

</xsl:template>

<xsl:template match="UML:Namespace.ownedElement">
  <xsl:comment>Stereotypes </xsl:comment>
  <xsl:copy-of select="UML:Stereotype"/>
  <xsl:comment>Stereotypes</xsl:comment>
  <xsl:comment>Datatypes </xsl:comment>
  <xsl:copy-of select="UML:DataType"/>
  <xsl:comment>Datatypes</xsl:comment>
  <xsl:comment>Tag definitions </xsl:comment>
  <xsl:copy-of select="UML:TagDefinition"/>
  <xsl:comment>Tag definitions </xsl:comment>
  <xsl:comment>Classes without stereotype</xsl:comment>
  <xsl:apply-templates select="UML:Class[count(./UML:ModelElement.stereotype) = 0]"/>  
  <xsl:comment>Classes without stereotype</xsl:comment>
  <xsl:comment>Resources</xsl:comment>
  <xsl:copy-of select="UML:Class[UML:ModelElement.stereotype/UML:Stereotype/@xmi.idref='resource']"/>
  <xsl:comment>Resources</xsl:comment>
  <xsl:comment>WW8 resources</xsl:comment>
  <xsl:apply-templates select="UML:Class[UML:ModelElement.stereotype/UML:Stereotype/@xmi.idref='ww8resource' and count(UML:ModelElement.stereotype/UML:Stereotype[@xmi.idref='ww8sprm']) = 0]"/>
  <xsl:comment>WW8 resources</xsl:comment>
  <xsl:comment>SPRMS</xsl:comment>
  <xsl:apply-templates select="UML:Class[UML:ModelElement.stereotype/UML:Stereotype/@xmi.idref='ww8sprm']"/>
  <xsl:comment>SPRMS</xsl:comment>
  <xsl:comment>DFFS</xsl:comment>
  <xsl:apply-templates select="UML:Class[UML:ModelElement.stereotype/UML:Stereotype/@xmi.idref='dffrecord']"/>
  <xsl:comment>DFFS</xsl:comment>
  <xsl:comment>DFFOPT</xsl:comment>
  <xsl:apply-templates select="UML:Class[UML:ModelElement.stereotype/UML:Stereotype/@xmi.idref='dffopt']"/>
  <xsl:comment>DFFOPT</xsl:comment>
</xsl:template>

<xsl:key name="generalization" match="UML:Generalization" 
         use="UML:Generalization.child/UML:Class/@xmi.idref"/>

<xsl:template match="UML:Class[count(UML:ModelElement.stereotype) = 0]">
  <xsl:variable name="theid"><xsl:value-of select="@xmi.id"/></xsl:variable>
  <xsl:comment>Class <xsl:value-of select="@xmi.id"/></xsl:comment>
  <xsl:copy-of select="."/>
  <xsl:copy-of select="key('generalization', $theid)"/>
  <xsl:comment>Class <xsl:value-of select="@xmi.id"/></xsl:comment>
</xsl:template>

<xsl:template match="UML:Class[UML:ModelElement.stereotype/UML:Stereotype/@xmi.idref='ww8resource' and count(UML:ModelElement.stereotype/UML:Stereotype[@xmi.idref='ww8sprm']) = 0]">
  <xsl:variable name="theid"><xsl:value-of select="@xmi.id"/></xsl:variable>
  <xsl:comment>Class <xsl:value-of select="@xmi.id"/></xsl:comment>
  <xsl:copy-of select="."/>
  <xsl:copy-of select="key('generalization', $theid)"/>
  <xsl:comment>Class <xsl:value-of select="@xmi.id"/></xsl:comment>
</xsl:template>

<xsl:template match="UML:Class[UML:ModelElement.stereotype/UML:Stereotype/@xmi.idref='ww8sprm']">
  <xsl:variable name="theid"><xsl:value-of select="@xmi.id"/></xsl:variable>
  <xsl:comment>SPRM <xsl:value-of select="@xmi.id"/></xsl:comment>
  <xsl:copy-of select="."/>
  <xsl:copy-of select="key('generalization', $theid)"/>
  <xsl:comment>SPRM <xsl:value-of select="@xmi.id"/></xsl:comment>
</xsl:template>

<xsl:template match="UML:Class[UML:ModelElement.stereotype/UML:Stereotype/@xmi.idref='dffrecord']">
  <xsl:variable name="theid"><xsl:value-of select="@xmi.id"/></xsl:variable>
  <xsl:comment>DFF <xsl:value-of select="@xmi.id"/></xsl:comment>
  <xsl:copy-of select="."/>
  <xsl:copy-of select="key('generalization', $theid)"/>
  <xsl:comment>DFF <xsl:value-of select="@xmi.id"/></xsl:comment>
</xsl:template>

<xsl:template match="UML:Class[UML:ModelElement.stereotype/UML:Stereotype/@xmi.idref='dffopt']">
  <xsl:variable name="theid"><xsl:value-of select="@xmi.id"/></xsl:variable>
  <xsl:comment>DFFOPT <xsl:value-of select="@xmi.id"/></xsl:comment>
  <xsl:copy-of select="."/>
  <xsl:copy-of select="key('generalization', $theid)"/>
  <xsl:comment>DFFOPT <xsl:value-of select="@xmi.id"/></xsl:comment>
</xsl:template>

</xsl:stylesheet>