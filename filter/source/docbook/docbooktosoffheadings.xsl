<?xml version="1.0" encoding="UTF-8"?>
<!--
 #  The Contents of this file are made available subject to the terms of
 #  either of the following licenses
 #
 #         - GNU Lesser General Public License Version 2.1
 #         - Sun Industry Standards Source License Version 1.1
 #
 #  Sun Microsystems Inc., October, 2000
 #
 #  GNU Lesser General Public License Version 2.1
 #  =============================================
 #  Copyright 2000 by Sun Microsystems, Inc.
 #  901 San Antonio Road, Palo Alto, CA 94303, USA
 #
 #  This library is free software; you can redistribute it and/or
 #  modify it under the terms of the GNU Lesser General Public
 #  License version 2.1, as published by the Free Software Foundation.
 #
 #  This library is distributed in the hope that it will be useful,
 #  but WITHOUT ANY WARRANTY; without even the implied warranty of
 #  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 #  Lesser General Public License for more details.
 #
 #  You should have received a copy of the GNU Lesser General Public
 #  License along with this library; if not, write to the Free Software
 #  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 #  MA  02111-1307  USA
 #
 #
 #  Sun Industry Standards Source License Version 1.1
 #  =================================================
 #  The contents of this file are subject to the Sun Industry Standards
 #  Source License Version 1.1 (the "License"); You may not use this file
 #  except in compliance with the License. You may obtain a copy of the
 #  License at http://www.openoffice.org/license.html.
 #
 #  Software provided under this License is provided on an "AS IS" basis,
 #  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 #  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 #  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 #  See the License for the specific provisions governing your rights and
 #  obligations concerning the Software.
 #
 #  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 #
 #  Copyright: 2000 by Sun Microsystems, Inc.
 #
 #  All Rights Reserved.
 #
 #  Contributor(s): _______________________________________
 #
 #
 -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format"
xmlns:office="http://openoffice.org/2000/office" xmlns:style="http://openoffice.org/2000/style" xmlns:text="http://openoffice.org/2000/text" xmlns:table="http://openoffice.org/2000/table" xmlns:draw="http://openoffice.org/2000/drawing"  xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:meta="http://openoffice.org/2000/meta" xmlns:number="http://openoffice.org/2000/datastyle" xmlns:svg="http://www.w3.org/2000/svg" xmlns:chart="http://openoffice.org/2000/chart" xmlns:dr3d="http://openoffice.org/2000/dr3d" xmlns:math="http://www.w3.org/1998/Math/MathML" xmlns:form="http://openoffice.org/2000/form" xmlns:script="http://openoffice.org/2000/script" xmlns:config="http://openoffice.org/2001/config" office:class="text" office:version="1.0">
	<xsl:decimal-format name="staff" digit="D" />
<xsl:template match="/">
	<xsl:element name="office:document">
			<office:meta>
		<meta:generator>StarOffice 6.1 (Solaris Sparc)</meta:generator>
		<dc:title><xsl:value-of select="/article/articleinfo/title"/></dc:title>
		<dc:description></dc:description>
		<dc:subject></dc:subject>
		<meta:creation-date>2002-07-15T12:38:53</meta:creation-date>
		<dc:date><xsl:value-of select="article/articleinfo/pubdate"/></dc:date>
		<dc:language><xsl:value-of select="article/@lang"/></dc:language>
		<meta:editing-cycles>21</meta:editing-cycles>
		<meta:editing-duration>P1DT0H11M54S</meta:editing-duration>
		<meta:user-defined meta:name="Info 1"/>
		<meta:user-defined meta:name="Info 2"/>
		<meta:user-defined meta:name="Info 3"/>
		<meta:user-defined meta:name="Info 4"/>
		<meta:document-statistic meta:table-count="1" meta:image-count="0" meta:object-count="0" meta:page-count="1" meta:paragraph-count="42" meta:word-count="144" meta:character-count="820"/>
	</office:meta>
	<office:automatic-styles>
		<style:style style:name="fr1" style:family="graphics" style:parent-style-name="Graphics" ><style:properties style:horizontal-pos="center" style:horizontal-rel="paragraph" style:mirror="none" fo:clip="rect(0cm 0cm 0cm 0cm)" draw:luminance="0%" draw:contrast="0%" draw:red="0%" draw:green="0%" draw:blue="0%" draw:gamma="1" draw:color-inversion="false" draw:transparency="-100%" draw:color-mode="standard" ></style:properties>
</style:style>

		<style:style style:name="Table1" style:family="table">
			<style:properties style:width="14.649cm" table:align="margins"/>
		</style:style>
		<style:style style:name="Table1.A" style:family="table-column">
			<style:properties style:column-width="2.93cm" style:rel-column-width="13107*"/>
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
		<style:style style:name="P1" style:family="paragraph" style:parent-style-name="Text body" style:list-style-name="Ordered List"/>
		<style:style style:name="T1" style:family="text" style:parent-style-name="Source Text">
			<style:properties fo:font-style="normal"/>
		</style:style>
		
		<style:page-master style:name="pm1">
			<style:properties fo:page-width="20.999cm" fo:page-height="29.699cm" style:num-format="1" style:print-orientation="portrait" fo:margin-top="2.54cm" fo:margin-bottom="2.54cm" fo:margin-left="3.175cm" fo:margin-right="3.175cm" style:writing-mode="lr-tb" style:footnote-max-height="0cm">
				<style:footnote-sep style:width="0.018cm" style:distance-before-sep="0.101cm" style:distance-after-sep="0.101cm" style:adjustment="left" style:rel-width="25%" style:color="#000000"/>
			</style:properties>
			<style:header-style/>
			<style:footer-style/>
		</style:page-master>
	</office:automatic-styles>
	<office:master-styles>
		<style:master-page style:name="Standard" style:page-master-name="pm1"/>
	</office:master-styles>
			<office:body>				 
				<xsl:apply-templates/>
			</office:body>
	</xsl:element>
</xsl:template>

<xsl:template match="subtitle">
<xsl:choose>
	<xsl:when test="parent::table">
			<xsl:apply-templates/>
	</xsl:when>
	<xsl:when test="parent::informaltable">
			<xsl:apply-templates/>
	</xsl:when>
	<xsl:otherwise>
		<xsl:element name="text:p">				
					<xsl:attribute name="text:style-name">Section SubTitle</xsl:attribute>
		</xsl:element>
	</xsl:otherwise>
</xsl:choose>
</xsl:template>




<xsl:template match="title">
<xsl:choose>
	<xsl:when test="parent::figure">
	</xsl:when>
	<xsl:when test="parent::table">
	</xsl:when>
	<xsl:when test="parent::sect1">
	</xsl:when>
<xsl:when test="parent::sect2">
	</xsl:when>
<xsl:when test="parent::sect3">
	</xsl:when>
<xsl:when test="parent::sect4">
	</xsl:when>
<xsl:when test="parent::sect5">
	</xsl:when>
	<xsl:when test="parent::informaltable">
			<xsl:apply-templates/>
	</xsl:when>
	
	<xsl:otherwise>
		<xsl:element name="text:p">				
			<xsl:choose>	
				
				<xsl:when test="parent::appendix">
								 <xsl:attribute name="text:style-name">Appendix Title</xsl:attribute>	 
				</xsl:when>
		 	</xsl:choose>
			<xsl:apply-templates/>
		</xsl:element>
	</xsl:otherwise>
</xsl:choose>
</xsl:template>

<xsl:template match="articleinfo">
	<xsl:element name="text:section">
		<xsl:attribute name="text:style-name">ArticleInfo</xsl:attribute>
		<xsl:attribute name="text:name">ArticleInfo</xsl:attribute>
			<xsl:if test="/article/articleinfo/title !=''">
				 	    <xsl:element name="text:p">
							<xsl:attribute name="text:style-name">Document Title</xsl:attribute>
							<xsl:value-of select="/article/articleinfo/title"/>
						</xsl:element>
						<xsl:if test="/article/articleinfo/subtitle !=''">
							<xsl:element name="text:p">
								<xsl:attribute name="text:style-name">Document SubTitle</xsl:attribute>
								<xsl:value-of select="/article/articleinfo/subtitle"/>
							</xsl:element>
						</xsl:if>
				 </xsl:if>
			<xsl:apply-templates/>
	</xsl:element>
	
</xsl:template>

<xsl:template match="appendix">
	<xsl:element name="text:section">
		<xsl:attribute name="text:style-name">Appendix</xsl:attribute>
		<xsl:attribute name="text:name">Appendix</xsl:attribute>
			<xsl:apply-templates/>
	</xsl:element>
	
</xsl:template>

<!--
<xsl:template match="author">
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="firstname">
	 <xsl:element name="text:variable-set">
		 <xsl:attribute name="text:name">
		 	<xsl:if test="ancestor::articleinfo/author">
		 		<xsl:text disable-output-escaping="yes">articleinfo.author</xsl:text><xsl:value-of select="count(parent::author[preceding-sibling::author])"/><xsl:text disable-output-escaping="yes">.firstname</xsl:text><xsl:value-of select="count(preceding-sibling::firstname)"/>
		 	</xsl:if>
		</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>

</xsl:template>-->

<xsl:template match="articleinfo/title">
	<!-- <xsl:element name="text:variable-decls">
             	<xsl:element name="text:variable-decl">
             		<xsl:attribute name="text:value-type">
             			<xsl:text>string</xsl:text>	
            		 </xsl:attribute>
      			<xsl:attribute name="text:name">
		 			<xsl:text disable-output-escaping="yes">articleinfo.title</xsl:text>
      				</xsl:attribute>
      			 </xsl:element>
      			 
      	</xsl:element>
      	<xsl:element name="text:p">
	 <xsl:element name="text:variable-set">
	 	<xsl:attribute name="text:value-type">
             			<xsl:text>string</xsl:text>	
            		 </xsl:attribute>
		 <xsl:attribute name="text:name">
		 	<xsl:text disable-output-escaping="yes">articleinfo.title</xsl:text>
		</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>	
	</xsl:element>-->
</xsl:template>

<xsl:template match="articleinfo/subtitle">
	 <xsl:element name="text:variable-decls">
             	<xsl:element name="text:variable-decl">
             		<xsl:attribute name="text:value-type">
             			<xsl:text>string</xsl:text>	
            		 </xsl:attribute>
      			<xsl:attribute name="text:name">
		 			<xsl:text disable-output-escaping="yes">articleinfo.subtitle</xsl:text>
      				</xsl:attribute>
      			 </xsl:element>
      	</xsl:element>
      	<xsl:element name="text:p">
	 <xsl:element name="text:variable-set">
	 	<xsl:attribute name="text:value-type">
             			<xsl:text>string</xsl:text>	
            		 </xsl:attribute>
		 <xsl:attribute name="text:name">
		 	<xsl:text disable-output-escaping="yes">articleinfo.subtitle</xsl:text>
		</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
	</xsl:element>
</xsl:template>

<xsl:template match="articleinfo/edition">
	 <xsl:element name="text:variable-decls">
             	<xsl:element name="text:variable-decl">
             		<xsl:attribute name="text:value-type">
             			<xsl:text>string</xsl:text>	
            		 </xsl:attribute>
      			<xsl:attribute name="text:name">
		 			<xsl:text disable-output-escaping="yes">articleinfo.edition</xsl:text>
      				</xsl:attribute>
      			 </xsl:element>
      	</xsl:element>
      	<xsl:element name="text:p">
	 <xsl:element name="text:variable-set">
	 	<xsl:attribute name="text:value-type">
             			<xsl:text>string</xsl:text>	
            	 </xsl:attribute>
		 <xsl:attribute name="text:name">
		 	<xsl:text disable-output-escaping="yes">articleinfo.edition</xsl:text>
		</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
	</xsl:element>
</xsl:template>

<xsl:template match="articleinfo/releaseinfo">
      <xsl:element name="text:variable-decls">
             	<xsl:element name="text:variable-decl">
             		<xsl:attribute name="text:value-type">
             			<xsl:text>string</xsl:text>	
            		 </xsl:attribute>
      			<xsl:attribute name="text:name">
		 			<xsl:text disable-output-escaping="yes">articleinfo.releaseinfo_</xsl:text><xsl:value-of select="count(preceding-sibling::releaseinfo)"/>
      				</xsl:attribute>
      			 </xsl:element>
      	</xsl:element>
      	<xsl:element name="text:p">
	 <xsl:element name="text:variable-set">
	 	<xsl:attribute name="text:value-type">
             			<xsl:text>string</xsl:text>	
            	</xsl:attribute>
		 <xsl:attribute name="text:name">
		 	<xsl:text disable-output-escaping="yes">articleinfo.releaseinfo_</xsl:text><xsl:value-of select="count(preceding-sibling::releaseinfo)"/>
		</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
	</xsl:element>
</xsl:template>


<xsl:template match="author/firstname">
      <xsl:element name="text:variable-decls">
             	<xsl:element name="text:variable-decl">
             		<xsl:attribute name="text:value-type">
             			<xsl:text>string</xsl:text>	
            		 </xsl:attribute>
            		
      			<xsl:attribute name="text:name">
      				<xsl:if test="ancestor::articleinfo">
		 			<xsl:text disable-output-escaping="yes">articleinfo.author_</xsl:text><xsl:value-of select="count(parent::author[preceding-sibling::author])"/><xsl:text disable-output-escaping="yes">.firstname_</xsl:text><xsl:value-of select="count(preceding-sibling::firstname)"/>
		 		</xsl:if>
      				</xsl:attribute>
      			 </xsl:element>
      	</xsl:element>
      	<xsl:element name="text:p">
	 <xsl:element name="text:variable-set">
	 	<xsl:attribute name="text:value-type">
             			<xsl:text>string</xsl:text>	
            	</xsl:attribute>
		 <xsl:attribute name="text:name">
		 	<xsl:if test="ancestor::articleinfo">
		 		<xsl:text disable-output-escaping="yes">articleinfo.author_</xsl:text><xsl:value-of select="count(parent::author[preceding-sibling::author])"/><xsl:text disable-output-escaping="yes">.firstname_</xsl:text><xsl:value-of select="count(preceding-sibling::firstname)"/>
			</xsl:if>
		</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
	</xsl:element>
</xsl:template>



<xsl:template match="articleinfo/copyright/year">
      <xsl:element name="text:variable-decls">
             	<xsl:element name="text:variable-decl">
             		<xsl:attribute name="text:value-type">
             			<xsl:text>string</xsl:text>	
            		 </xsl:attribute>
            		
      			<xsl:attribute name="text:name">
      				<xsl:if test="ancestor::articleinfo/copyright">
		 			<xsl:text disable-output-escaping="yes">articleinfo.copyright_</xsl:text><xsl:value-of select="count(parent::copyright[preceding-sibling::copyright])"/><xsl:text disable-output-escaping="yes">.year_</xsl:text><xsl:value-of select="count(preceding-sibling::year)"/>
		 		</xsl:if>
      				</xsl:attribute>
      			 </xsl:element>
      	</xsl:element>
      	<xsl:element name="text:p">
	 <xsl:element name="text:variable-set">
	 	<xsl:attribute name="text:value-type">
             			<xsl:text>string</xsl:text>	
            	</xsl:attribute>
		 <xsl:attribute name="text:name">
		 	<xsl:if test="ancestor::articleinfo/copyright">
		 			<xsl:text disable-output-escaping="yes">articleinfo.copyright_</xsl:text><xsl:value-of select="count(parent::copyright[preceding-sibling::copyright])"/><xsl:text disable-output-escaping="yes">.year_</xsl:text><xsl:value-of select="count(preceding-sibling::year)"/>
		 		</xsl:if>
		</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
	</xsl:element>
</xsl:template>

<xsl:template match="authorgroup">
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="articleinfo/copyright/holder">
      <xsl:element name="text:variable-decls">
             	<xsl:element name="text:variable-decl">
             		<xsl:attribute name="text:value-type">
             			<xsl:text>string</xsl:text>	
            		 </xsl:attribute>
            		
      			<xsl:attribute name="text:name">
      				<xsl:if test="ancestor::articleinfo/copyright">
		 			<xsl:text disable-output-escaping="yes">articleinfo.copyright_</xsl:text><xsl:value-of select="count(parent::copyright[preceding-sibling::copyright])"/><xsl:text disable-output-escaping="yes">.holder_</xsl:text><xsl:value-of select="count(preceding-sibling::holder)"/>
		 		</xsl:if>
      				</xsl:attribute>
      			 </xsl:element>
      	</xsl:element>
      	<xsl:element name="text:p">
	 <xsl:element name="text:variable-set">
	 	<xsl:attribute name="text:value-type">
             			<xsl:text>string</xsl:text>	
            	</xsl:attribute>
		 <xsl:attribute name="text:name">
		 	<xsl:if test="ancestor::articleinfo/copyright">
		 			<xsl:text disable-output-escaping="yes">articleinfo.copyright_</xsl:text><xsl:value-of select="count(parent::copyright[preceding-sibling::copyright])"/><xsl:text disable-output-escaping="yes">.holder_</xsl:text><xsl:value-of select="count(preceding-sibling::holder)"/>
		 		</xsl:if>
		</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
	</xsl:element>
</xsl:template>




<xsl:template name="affiliation">
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="author/affiliation/address">
      <xsl:element name="text:variable-decls">
             	<xsl:element name="text:variable-decl">
             		<xsl:attribute name="text:value-type">
             			<xsl:text>string</xsl:text>	
            		 </xsl:attribute>
      			<xsl:attribute name="text:name">
		 			<xsl:text disable-output-escaping="yes">articleinfo.author_</xsl:text><xsl:value-of select="count(ancestor::author[preceding-sibling::author])"/><xsl:text disable-output-escaping="yes">.affiliation_</xsl:text><xsl:value-of select="count(parent::affiliation[preceding-sibling::affiliation])"/><xsl:text disable-output-escaping="yes">.address_</xsl:text><xsl:value-of select="count(preceding-sibling::address)"/>
      				</xsl:attribute>
      			 </xsl:element>
      	</xsl:element>
      	<xsl:element name="text:p">
	 <xsl:element name="text:variable-set">
	 	<xsl:attribute name="text:value-type">
             			<xsl:text>string</xsl:text>	
            	</xsl:attribute>
		 <xsl:attribute name="text:name">
		 	<xsl:text disable-output-escaping="yes">articleinfo.author_</xsl:text><xsl:value-of select="count(ancestor::author[preceding-sibling::author])"/><xsl:text disable-output-escaping="yes">.affiliation_</xsl:text><xsl:value-of select="count(parent::affiliation[preceding-sibling::affiliation])"/><xsl:text disable-output-escaping="yes">.address_</xsl:text><xsl:value-of select="count(preceding-sibling::address)"/>

		</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
	</xsl:element>
</xsl:template>

<xsl:template match="author/affiliation/orgname">
      <xsl:element name="text:variable-decls">
             	<xsl:element name="text:variable-decl">
             		<xsl:attribute name="text:value-type">
             			<xsl:text>string</xsl:text>	
            		 </xsl:attribute>
            		 <xsl:if test="ancestor::articleinfo">
      			<xsl:attribute name="text:name">
		 			<xsl:text disable-output-escaping="yes">articleinfo.author_</xsl:text><xsl:value-of select="count(ancestor::author[preceding-sibling::author])"/><xsl:text disable-output-escaping="yes">.affiliation_</xsl:text><xsl:value-of select="count(parent::affiliation[preceding-sibling::affiliation])"/><xsl:text disable-output-escaping="yes">.orgname_</xsl:text><xsl:value-of select="count(preceding-sibling::orgname)"/>
      				</xsl:attribute>
      			</xsl:if>
      			 </xsl:element>
      	</xsl:element>
      	<xsl:element name="text:p">
	 <xsl:element name="text:variable-set">
	 	<xsl:attribute name="text:value-type">
             			<xsl:text>string</xsl:text>	
            	</xsl:attribute>
            	 <xsl:if test="ancestor::articleinfo">
		 <xsl:attribute name="text:name">
		 		<xsl:text disable-output-escaping="yes">articleinfo.author_</xsl:text><xsl:value-of select="count(ancestor::author[preceding-sibling::author])"/><xsl:text disable-output-escaping="yes">.affiliation_</xsl:text><xsl:value-of select="count(parent::affiliation[preceding-sibling::affiliation])"/><xsl:text disable-output-escaping="yes">.orgname_</xsl:text><xsl:value-of select="count(preceding-sibling::orgname)"/>
		</xsl:attribute>
		</xsl:if>
		<xsl:apply-templates/>		
	</xsl:element>
	</xsl:element>
</xsl:template>



<xsl:template match="author/surname">
      <xsl:element name="text:variable-decls">
             	<xsl:element name="text:variable-decl">
             		<xsl:attribute name="text:value-type">
             			<xsl:text>string</xsl:text>	
            		 </xsl:attribute>
            		
      			<xsl:attribute name="text:name">
		 			<xsl:text disable-output-escaping="yes">articleinfo.author_</xsl:text><xsl:value-of select="count(parent::author[preceding-sibling::author])"/><xsl:text disable-output-escaping="yes">.surname_</xsl:text><xsl:value-of select="count(preceding-sibling::surname)"/>
      				</xsl:attribute>
      			 </xsl:element>
      	</xsl:element>
      	<xsl:element name="text:p">
	 <xsl:element name="text:variable-set">
		<xsl:attribute name="text:value-type">
             			<xsl:text>string</xsl:text>	
            	</xsl:attribute>
		 <xsl:attribute name="text:name">
		 		<xsl:text disable-output-escaping="yes">articleinfo.author_</xsl:text><xsl:value-of select="count(parent::author[preceding-sibling::author])"/><xsl:text disable-output-escaping="yes">.surname_</xsl:text><xsl:value-of select="count(preceding-sibling::surname)"/>

		
		</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
	</xsl:element>
</xsl:template>





<xsl:template match="para">
<xsl:choose>
<xsl:when test="ancestor::varlistentry">
		<xsl:element name="text:p">
			<xsl:attribute name="text:style-name">VarList Term</xsl:attribute>
			<xsl:apply-templates />
		</xsl:element>
</xsl:when>
<xsl:otherwise>
	<xsl:element name="text:p">
<xsl:choose>
	<xsl:when test="ancestor-or-self::footnote">
	 	<xsl:attribute name= "text:style-name"><xsl:text>Footnote</xsl:text></xsl:attribute>
	</xsl:when>
	<xsl:when test="ancestor-or-self::listitem">
	 	<xsl:attribute name= "text:style-name"><xsl:text>VarList Item</xsl:text></xsl:attribute>
	</xsl:when>
	<xsl:when test="ancestor-or-self::informaltable">
		<xsl:if test="ancestor-or-self::informaltable">
			<xsl:attribute name="text:style-name">Table Contents</xsl:attribute>
		</xsl:if>
		<xsl:if test="ancestor-or-self::thead ">
			<xsl:attribute name="text:style-name">Table Heading</xsl:attribute>
		</xsl:if>
	</xsl:when>
	<xsl:when test="ancestor-or-self::table">
	<xsl:if test="ancestor-or-self::table">
			<xsl:attribute name="text:style-name">Table Contents</xsl:attribute>
		</xsl:if>
		<xsl:if test="ancestor-or-self::thead ">
			<xsl:attribute name="text:style-name">Table Heading</xsl:attribute>
		</xsl:if>
	</xsl:when>
	<xsl:otherwise>
			<xsl:attribute name="text:style-name">Text body</xsl:attribute>
	</xsl:otherwise>
</xsl:choose>
		<xsl:apply-templates/>	
</xsl:element>

</xsl:otherwise>
</xsl:choose>
</xsl:template>

<xsl:template match="section">
	<xsl:element name="text:h">
		<xsl:attribute name="text:level"><xsl:value-of select="count(ancestor-or-self::section) "/></xsl:attribute> 
		<xsl:value-of select="child::title"/> 
	</xsl:element>
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="abstract">
	<xsl:element name="text:h">
		<xsl:attribute name="text:level">1</xsl:attribute> 
		<xsl:text>abstract</xsl:text>
	</xsl:element>
	<xsl:apply-templates/>
</xsl:template>


<xsl:template match="appendix">
	<xsl:element name="text:h">
		<xsl:attribute name="text:level">1</xsl:attribute> 
		<xsl:text>appendix</xsl:text>
	</xsl:element>
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="sect1">
	<xsl:element name="text:h">
		<xsl:attribute name="text:level">1</xsl:attribute>
		<xsl:value-of select="child::title"/> 
	</xsl:element>
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="sect2">
	<xsl:element name="text:h">
		<xsl:attribute name="text:level">2</xsl:attribute> 
			<xsl:value-of select="child::title"/> 
	</xsl:element>
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="sect3">
	<xsl:element name="text:h">
		<xsl:attribute name="text:level">3</xsl:attribute> 
			<xsl:value-of select="child::title"/> 
	</xsl:element>
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="sect4">
	<xsl:element name="text:h">
		<xsl:attribute name="text:level">4</xsl:attribute> 
			<xsl:value-of select="child::title"/> 
	</xsl:element>
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="sect5">
	<xsl:element name="text:h">
		<xsl:attribute name="text:level">5</xsl:attribute> 
			<xsl:value-of select="child::title"/> 
	</xsl:element>
	<xsl:apply-templates/>
</xsl:template>

<!--<xsl:template match="sect5">
	<xsl:element name="text:section">
		<xsl:attribute name="text:style-name">Sect1</xsl:attribute> 
		<xsl:attribute name="text:name"><xsl:value-of select="@id"/></xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
</xsl:template>-->


<xsl:template match="informaltable">
	<xsl:element name="table:table">
		<xsl:attribute name="table:name"></xsl:attribute>
		<xsl:attribute name="table:style-name">Table1</xsl:attribute>
		<xsl:attribute name="table:name"><xsl:value-of select="@id"/></xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
</xsl:template>


<xsl:template match="table">
	<xsl:variable name="tabletitle"><xsl:value-of select="title"/></xsl:variable>
	<xsl:element name="table:table">
		<xsl:attribute name="table:name"></xsl:attribute>
		<xsl:attribute name="table:style-name">Table1</xsl:attribute>
		<xsl:attribute name="table:name"><xsl:value-of select="@id"/></xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
	<xsl:if test="not($tabletitle='')">
		<xsl:element name="text:p">
			<xsl:attribute name="text:style-name">Table</xsl:attribute>
				<xsl:value-of select="$tabletitle"/>
	  	</xsl:element>
	</xsl:if>
</xsl:template>

<xsl:template match="tgroup">
	<xsl:element name="table:table-column">
		<xsl:attribute name="table:style-name">Table1.A</xsl:attribute>
		<xsl:choose>
			<xsl:when test="@cols >0">
				<xsl:attribute name="table:number-columns-repeated"><xsl:value-of select="@cols"/></xsl:attribute>
			</xsl:when>
			<xsl:otherwise>
				<xsl:attribute name="table:number-columns-repeated"><xsl:value-of select="count(child::tbody/row/entry) div count(child::tbody/row) "/></xsl:attribute>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:element>
	<xsl:apply-templates/>
</xsl:template>


<xsl:template match="indexterm">
</xsl:template>

<xsl:template match="thead">
	<xsl:element name="table:table-header-rows">
		<xsl:apply-templates />
	</xsl:element>
</xsl:template>

<xsl:template match="tbody">
		<xsl:apply-templates />
</xsl:template>

<xsl:template match="row">
	<xsl:element name="table:table-row">
		<xsl:apply-templates/>
	</xsl:element>
</xsl:template>

<xsl:template match="entry">
	<xsl:element name="table:table-cell">
		<xsl:if test="ancestor-or-self::thead">
			<xsl:attribute name="table:style-name">Table1.A1</xsl:attribute>
		</xsl:if>
		<xsl:if test="not(ancestor-or-self::thead)">
			<xsl:attribute name="table:style-name">Table1.A2</xsl:attribute>
		</xsl:if>
		
		<xsl:choose>
			<xsl:when test="@spanname">
		<!--<xsl:if test="@spanname">-->
				<xsl:variable name="sname" >
					<xsl:value-of select="@spanname"/>
				</xsl:variable>
				<xsl:attribute name="table:number-columns-spanned">
					<xsl:variable name="colnamestart">
						<xsl:value-of select="ancestor::tgroup/spanspec[@spanname=$sname]/@namest"/>
					</xsl:variable>
					<xsl:variable name="colnameend">
						<xsl:value-of select="ancestor::tgroup/spanspec[@spanname=$sname]/@nameend"/>
					</xsl:variable>
					<xsl:variable name="colnumstart">
						<xsl:value-of select="ancestor::tgroup/colspec[@colname=$colnamestart]/@colnum"/>
					</xsl:variable>
					<xsl:variable name="colnumend">
						<xsl:value-of select="ancestor::tgroup/colspec[@colname=$colnameend]/@colnum"/>
					</xsl:variable>
					<xsl:value-of select="$colnumend - $colnumstart + 1"/>
			  	</xsl:attribute >
		</xsl:when>
		<xsl:when  test="@namest and @nameend">
		<!--<xsl:if test="@namest and @nameend">-->
			<xsl:variable name="colnamestart">
					<xsl:value-of select="@namest"/>
			</xsl:variable>
			<xsl:variable name="colnameend">
					<xsl:value-of select="@nameend"/>
			</xsl:variable>

			<xsl:attribute name="table:number-columns-spanned">
			<xsl:variable name="colnumstart">
						<xsl:value-of select="ancestor::tgroup/colspec[@colname=$colnamestart]/@colnum"/>
					</xsl:variable>
					<xsl:variable name="colnumend">
						<xsl:value-of select="ancestor::tgroup/colspec[@colname=$colnameend]/@colnum"/>
					</xsl:variable>
					<xsl:value-of select="$colnumend - $colnumstart + 1"/>

			</xsl:attribute >
		</xsl:when>
		</xsl:choose>
		<!--
		<xsl:if test="not(@namest = '' ) ">
		     <xsl:attribute name="table:number-columns-spanned">
				 <xsl:value-of select="(substring-after(@nameend,'c')-substring-after(@namest,'c'))+1"/>
			 	
			 </xsl:attribute>
		</xsl:if>
		-->
		<xsl:choose>
			<xsl:when test="not(child::para)">
				<xsl:element name="text:p">
						<xsl:if test="ancestor-or-self::thead">
							<xsl:attribute name="text:style-name">Table Heading</xsl:attribute>
						</xsl:if>
						<xsl:if test="ancestor-or-self::tbody">
							<xsl:attribute name="text:style-name">Table Contents</xsl:attribute>
						</xsl:if>
						<xsl:apply-templates/>
				</xsl:element>
			</xsl:when>
			<xsl:otherwise>
				<xsl:apply-templates/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:element>
</xsl:template>






<xsl:template match="figure">
	<xsl:apply-templates/>
</xsl:template>

<!--  lists          Section                                          -->



<xsl:template match="itemizedlist">
	<xsl:element name="text:unordered-list">
	<xsl:attribute name="text:style-name">Itemized List</xsl:attribute>
			<xsl:attribute name="text:continue-numbering">false</xsl:attribute>
			<xsl:apply-templates/>
	</xsl:element>
</xsl:template>

<xsl:template match="variablelist">
	<xsl:element name="text:unordered-list">
		<xsl:attribute name="text:style-name">Var List</xsl:attribute>
			<xsl:attribute name="text:continue-numbering">false</xsl:attribute>
		<xsl:apply-templates />
	</xsl:element>
</xsl:template>

<xsl:template match="orderedlist">
<xsl:element name="text:ordered-list">
	<xsl:attribute name="text:style-name">Ordered List</xsl:attribute>
	<xsl:attribute name="text:continue-numbering">false</xsl:attribute>
	<xsl:apply-templates/>	
</xsl:element>
</xsl:template>

<xsl:template match="term">
	<xsl:if test="parent::varlistentry">
	<text:list-item>
	<xsl:element name="text:p">
		<xsl:attribute name="text:style-name">VarList Term</xsl:attribute>
		<xsl:apply-templates />
	</xsl:element>
		</text:list-item>
	</xsl:if>
</xsl:template>


<xsl:template match="listitem">
	<xsl:choose>
		<xsl:when test="parent::varlistentry">
					<text:list-item>
						<xsl:apply-templates/>
						</text:list-item>
		</xsl:when>
		<xsl:otherwise>
				<text:list-item>
						<xsl:apply-templates/>
				</text:list-item>
		</xsl:otherwise>	
	</xsl:choose>
</xsl:template>


<!--  end of lists-->

<xsl:template match="menuchoice">
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="guimenuitem">
	<xsl:element name="text:span">
			<xsl:attribute name="text:style-name">GuiMenuItem</xsl:attribute>
			<xsl:apply-templates/>
	</xsl:element>
</xsl:template>

<xsl:template match="guibutton">
	<xsl:element name="text:span">
			<xsl:attribute name="text:style-name">GuiButton</xsl:attribute>
			<xsl:apply-templates/>
	</xsl:element>
</xsl:template>

<xsl:template match="guisubmenu">
	<xsl:element name="text:span">
			<xsl:attribute name="text:style-name">GuiSubMenu</xsl:attribute>
			<xsl:apply-templates/>
	</xsl:element>
</xsl:template>

<xsl:template match="emphasis">
		<xsl:element name="text:span">
			<xsl:attribute name="text:style-name">Emphasis</xsl:attribute>
			<xsl:apply-templates/>
	</xsl:element>
</xsl:template>


<xsl:template match="guimenu">
		<xsl:element name="text:span">
			<xsl:attribute name="text:style-name">GuiMenu</xsl:attribute>
			<xsl:apply-templates/>
	</xsl:element>
</xsl:template>

<xsl:template match="guisubmenu">
		<xsl:element name="text:span">
			<xsl:attribute name="text:style-name">GuiSubMenu</xsl:attribute>
			<xsl:apply-templates/>
	</xsl:element>
</xsl:template>


<xsl:template match="guilabel">
	<xsl:element name="text:span">
			<xsl:attribute name="text:style-name">GuiLabel</xsl:attribute>
			<xsl:apply-templates/>
	</xsl:element>
</xsl:template>

<xsl:template match="guibutton">
	<xsl:element name="text:span">
			<xsl:attribute name="text:style-name">GuiButton</xsl:attribute>
			<xsl:apply-templates/>
	</xsl:element>
</xsl:template>

<xsl:template match="keycap">
	<xsl:element name="text:span">
		<xsl:attribute name="text:style-name">KeyCap</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
</xsl:template>


<xsl:template match="keysym">
	<xsl:element name="text:span">
		<xsl:attribute name="text:style-name">KeySym</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
</xsl:template>


<xsl:template match="keycombo">
	<xsl:element name="text:span">
		<xsl:attribute name="text:style-name">KeyCombo</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
</xsl:template>

<xsl:template match="command">
	<xsl:element name="text:span">
		<xsl:attribute name="text:style-name">Command</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
</xsl:template>

<xsl:template match="application">
	<xsl:element name="text:span">
		<xsl:attribute name="text:style-name">Application</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
</xsl:template>

<xsl:template match="filename">
	<xsl:element name="text:span">
		<xsl:attribute name="text:style-name">FileName</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
</xsl:template>

<xsl:template match="systemitem">
	<xsl:element name="text:span">
		<xsl:attribute name="text:style-name">SystemItem</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
</xsl:template>

<xsl:template match="computeroutput">
	<xsl:element name="text:span">
		<xsl:attribute name="text:style-name">ComputerOutput</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
</xsl:template>

<xsl:template match="inlinegraphic">
	<xsl:element name="draw:image">
		<xsl:attribute name="draw:style-name">
			<xsl:text>fr1</xsl:text>
		</xsl:attribute>
		<xsl:attribute name="draw:name">
		</xsl:attribute>
		<xsl:attribute name="text:anchor-type">
		</xsl:attribute>
		<xsl:attribute name="draw:z-index">
		</xsl:attribute>
		<xsl:attribute name="xlink:href">
			<xsl:value-of select="@fileref"/>
		</xsl:attribute>
		<xsl:attribute name="xlink:type">
		</xsl:attribute>
		<xsl:attribute name="svg:width">
			<!--<xsl:value-of select="@width"/>-->
			<xsl:text>1cm</xsl:text>
		</xsl:attribute> 
		<xsl:attribute name="svg:height">	
			<xsl:text>1cm</xsl:text>
		</xsl:attribute>
		<xsl:attribute name="xlink:show">
			<xsl:text>embed</xsl:text>
		</xsl:attribute>
		<xsl:attribute name="xlink:actuate">
			<xsl:text>onLoad</xsl:text>
		</xsl:attribute>
		<xsl:attribute name="draw:filter-name">
			<xsl:text disable-output-escaping="yes">&lt;All formats&gt;</xsl:text>
		</xsl:attribute>
</xsl:element>
</xsl:template> 


<xsl:template match="footnote">
	<xsl:element name="text:footnote">
		<!--<xsl:element name="text:footnote-citation">Aidan</xsl:element>-->
		<xsl:element name="text:footnote-body">
				<xsl:apply-templates/>
		</xsl:element>
	</xsl:element>
</xsl:template>

<xsl:template match="highlight">
	<xsl:element name="text:span">
		<xsl:attribute name="text:style-name">
			<xsl:text>Highlight</xsl:text>
		</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
</xsl:template>

<xsl:template match="ulink">
		<xsl:element name="text:a">
		<xsl:attribute name="xlink:type"><xsl:text>simple</xsl:text></xsl:attribute>
		<xsl:attribute name="xlink:href">
			<xsl:value-of select="@url"/>
		</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>

</xsl:template>

<xsl:template match="link">
	<xsl:element name="text:a">
		<xsl:attribute name="xlink:type"><xsl:text>simple</xsl:text></xsl:attribute>
		<xsl:attribute name="xlink:href">
			<xsl:text>#</xsl:text>
				<xsl:value-of select="@linkend"/>
			<xsl:text>%7Cregion</xsl:text>
		</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>

</xsl:template>

<xsl:template match="olink">
<xsl:element name="text:a">
		<xsl:attribute name="xlink:type"><xsl:text>simple</xsl:text></xsl:attribute>
		<xsl:attribute name="xlink:href">
			<xsl:value-of select="@targetdocent"/>
		</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
</xsl:template>

<xsl:template match="note">
	<office:annotation>
		<text:p>
			<xsl:apply-templates/>
		</text:p>
	</office:annotation>
</xsl:template>

<xsl:template match="imageobject">
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="textobject">
</xsl:template>

<xsl:template match="caption">
	<xsl:apply-templates/>
</xsl:template>


<xsl:template match="imagedata">
	<xsl:element name="draw:image">
		<xsl:attribute name="draw:style-name">
			<xsl:text>fr1</xsl:text>
		</xsl:attribute>
		<xsl:attribute name="draw:name">
		</xsl:attribute>
		<xsl:attribute name="text:anchor-type">
		</xsl:attribute>
		<xsl:attribute name="draw:z-index">
		</xsl:attribute>
		<xsl:attribute name="xlink:href">
			<xsl:value-of select="@fileref"/>
		</xsl:attribute>
		<xsl:attribute name="xlink:type">
		</xsl:attribute>
		<xsl:attribute name="svg:width">
			<!--<xsl:value-of select="@width"/>-->
			<xsl:text>1cm</xsl:text>
		</xsl:attribute> 
		<xsl:attribute name="svg:height">	
			<xsl:text>1cm</xsl:text>
		</xsl:attribute>
		<xsl:attribute name="xlink:show">
			<xsl:text>embed</xsl:text>
		</xsl:attribute>
		<xsl:attribute name="xlink:actuate">
			<xsl:text>onLoad</xsl:text>
		</xsl:attribute>
		<xsl:attribute name="draw:filter-name">
			<xsl:text disable-output-escaping="yes">&lt;All formats&gt;</xsl:text>
		</xsl:attribute>
	</xsl:element>
</xsl:template>

<xsl:template match="audioobject">
	<xsl:element name="draw:plugin">
		<xsl:attribute name="draw:style-name">
			<xsl:text>fr1</xsl:text>
		</xsl:attribute>
		<xsl:attribute name="draw:name">
		</xsl:attribute>
		<xsl:attribute name="text:anchor-type">
		</xsl:attribute>
		<xsl:attribute name="draw:z-index">
		</xsl:attribute>
		<xsl:attribute name="xlink:href">
			<xsl:value-of select="@fileref"/>
		</xsl:attribute>
		<xsl:attribute name="xlink:type">
		</xsl:attribute>
		<xsl:attribute name="svg:width">
			<!--<xsl:value-of select="@width"/>-->
			<xsl:text>1cm</xsl:text>
		</xsl:attribute> 
		<xsl:attribute name="svg:height">	
			<xsl:text>1cm</xsl:text>
		</xsl:attribute>
		<xsl:attribute name="xlink:show">
			<xsl:text>embed</xsl:text>
		</xsl:attribute>
		<xsl:attribute name="xlink:actuate">
			<xsl:text>onLoad</xsl:text>
		</xsl:attribute>
		<xsl:attribute name="draw:filter-name">
			<xsl:text disable-output-escaping="yes">&lt;All formats&gt;</xsl:text>
		</xsl:attribute>
	</xsl:element>
</xsl:template>

<xsl:template match="remark">
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="mediaobject">
	<xsl:element name="text:p">
		<xsl:attribute name="text:style-name"><xsl:text>Mediaobject</xsl:text></xsl:attribute>
	<xsl:apply-templates/>
	</xsl:element>
</xsl:template>

<xsl:template match="superscript">
	<xsl:element name="text:span">
		<xsl:attribute name="text:style-name">
			<xsl:text>SuperScript</xsl:text>
		</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
</xsl:template>

<xsl:template match="subscript">
	<xsl:element name="text:span">
		<xsl:attribute name="text:style-name">
			<xsl:text>SubScript</xsl:text>
		</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
</xsl:template>



</xsl:stylesheet>
