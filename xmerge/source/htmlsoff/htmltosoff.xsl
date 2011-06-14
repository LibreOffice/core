<?xml version='1.0' encoding="UTF-8"?>
<!--
 #*************************************************************************
 #
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

 #*************************************************************************
 -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
			      xmlns:office="http://openoffice.org/2000/office"
			      xmlns:style="http://openoffice.org/2000/style"
			      xmlns:text="http://openoffice.org/2000/text" 				   
			      xmlns:table="http://openoffice.org/2000/table" 				 
			      xmlns:draw="http://openoffice.org/2000/drawing"
			      xmlns:fo="http://www.w3.org/1999/XSL/Format" 
			      xmlns:xlink="http://www.w3.org/1999/xlink" 
			      xmlns:number="http://openoffice.org/2000/datastyle" 			    
			      xmlns:svg="http://www.w3.org/2000/svg" 
			      xmlns:chart="http://openoffice.org/2000/chart" 
			      xmlns:dr3d="http://openoffice.org/2000/dr3d" 
			      xmlns:math="http://www.w3.org/1998/Math/MathML" 
			      xmlns:form="http://openoffice.org/2000/form" 
			      xmlns:script="http://openoffice.org/2000/script"
			      >
<xsl:output method="xml" />


<xsl:template match="/">
	<xsl:apply-templates />
</xsl:template>

<xsl:template match="html">	
	
	<office:document-content xmlns:office="http://openoffice.org/2000/office"				 
				 xmlns:style="http://openoffice.org/2000/style" 				 
				 xmlns:text="http://openoffice.org/2000/text"
				 xmlns:table="http://openoffice.org/2000/table"
				 xmlns:draw="http://openoffice.org/2000/drawing"
				 xmlns:fo="http://www.w3.org/1999/XSL/Format" 
				 xmlns:xlink="http://www.w3.org/1999/xlink" 
				 xmlns:number="http://openoffice.org/2000/datastyle" 
				 xmlns:svg="http://www.w3.org/2000/svg" 
				 xmlns:chart="http://openoffice.org/2000/chart" 
				 xmlns:dr3d="http://openoffice.org/2000/dr3d" 
				 xmlns:math="http://www.w3.org/1998/Math/MathML" 
				 xmlns:form="http://openoffice.org/2000/form" 
				 xmlns:script="http://openoffice.org/2000/script" 
				 office:class="text" office:version="1.0">

<office:script/>
 <office:font-decls>
  <style:font-decl style:name="Letter Gothic" fo:font-family="&apos;Letter Gothic&apos;" style:font-family-generic="modern" style:font-pitch="fixed"/>
  <style:font-decl style:name="Arial Unicode MS" fo:font-family="&apos;Arial Unicode MS&apos;" style:font-pitch="variable"/>
  <style:font-decl style:name="HG Mincho Light J" fo:font-family="&apos;HG Mincho Light J&apos;" style:font-pitch="variable"/>
  <style:font-decl style:name="CG Times" fo:font-family="&apos;CG Times&apos;" style:font-family-generic="roman" style:font-pitch="variable"/>
  <style:font-decl style:name="Thorndale" fo:font-family="Thorndale" style:font-family-generic="roman" style:font-pitch="variable"/>
  <style:font-decl style:name="Antique Olive" fo:font-family="&apos;Antique Olive&apos;" style:font-family-generic="swiss" style:font-pitch="variable"/>
  <style:font-decl style:name="Arial Black" fo:font-family="&apos;Arial Black&apos;" style:font-family-generic="swiss" style:font-pitch="variable"/>
 </office:font-decls>
 <office:automatic-styles>
  <style:style style:name="Table1" style:family="table">
   <style:properties style:width="16.999cm" table:align="margins"/>
  </style:style>
  <style:style style:name="Table1.A" style:family="table-column">
   <style:properties style:column-width="3.399cm" style:rel-column-width="13107*"/>
  </style:style>
  <style:style style:name="Table1.A1" style:family="table-cell">
   <style:properties fo:padding="0.097cm" fo:border-left="0.002cm solid #000000" fo:border-right="none" fo:border-top="0.002cm solid #000000" fo:border-bottom="0.002cm solid #000000"/>
  </style:style>
  <style:style style:name="Table1.E1" style:family="table-cell">
   <style:properties fo:padding="0.097cm" fo:border="0.002cm solid #000000"/>
  </style:style>
  <style:style style:name="Table1.A2" style:family="table-cell">
   <style:properties fo:padding="0.097cm" fo:border-left="0.002cm solid #000000" fo:border-right="none" fo:border-top="none" fo:border-bottom="0.002cm solid #000000"/>
  </style:style>
  <style:style style:name="Table1.E2" style:family="table-cell">
   <style:properties fo:padding="0.097cm" fo:border-left="0.002cm solid #000000" fo:border-right="0.002cm solid #000000" fo:border-top="none" fo:border-bottom="0.002cm solid #000000"/>
  </style:style>
  <style:style style:name="P1" style:family="paragraph" style:parent-style-name="Standard">
   <style:properties fo:margin-left="0cm" fo:margin-right="0cm" fo:text-indent="0cm" style:auto-text-indent="false"/>
  </style:style>
  <style:style style:name="P2" style:family="paragraph" style:parent-style-name="Table Contents">
   <style:properties style:font-name="Arial Black" fo:font-size="20pt"/>
  </style:style>
 </office:automatic-styles>


	


 	<office:body>
		 <text:sequence-decls>
   			<text:sequence-decl text:display-outline-level="0" text:name="Illustration"/>
   			<text:sequence-decl text:display-outline-level="0" text:name="Table"/>
   			<text:sequence-decl text:display-outline-level="0" text:name="Text"/>
   			<text:sequence-decl text:display-outline-level="0" text:name="Drawing"/>
  		</text:sequence-decls>   		
  		
  		<xsl:apply-templates/>
 	</office:body>
	</office:document-content>

</xsl:template>

<xsl:template match="body">
	<xsl:apply-templates />
</xsl:template>



<xsl:template match="p">
	<xsl:for-each select=".">
		<text:p text:style-name="P1">
			<!--<xsl:value-of select="."/>-->
			<xsl:apply-templates />
		</text:p>
	</xsl:for-each>
</xsl:template>

<xsl:template match="br">
	<xsl:if test="ancestor::p">
		<xsl:text disable-output-escaping="yes">&lt;/text:p&gt; &lt;text:p text:style-name="P1"&gt;</xsl:text>
	</xsl:if>
</xsl:template>

<xsl:template match="table">
	<xsl:for-each select=".">
		<table:table table:name="Table1" table:style-name="Table1">
				 <table:table-column table:style-name="Table1.A" table:number-columns-repeated="5"/>
			<xsl:apply-templates/>
		</table:table>
	</xsl:for-each>
</xsl:template>

<xsl:template match="tr">
	<xsl:for-each select=".">
		<table:table-row>
			<xsl:apply-templates/>
		</table:table-row>
	</xsl:for-each>
</xsl:template>

<xsl:template match="td">
	<xsl:for-each select=".">
		<table:table-cell table:style-name="Table1.A1" table:value-type="string">
		<text:p text:style-name="P1">
				<xsl:value-of select="."/>
			</text:p>
		</table:table-cell>
	</xsl:for-each>
</xsl:template>



</xsl:stylesheet>
