<?xml version="1.0" encoding="UTF-8"?>
<!--

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

-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format" xmlns:office="http://openoffice.org/2000/office" xmlns:style="http://openoffice.org/2000/style" xmlns:text="http://openoffice.org/2000/text" xmlns:table="http://openoffice.org/2000/table" xmlns:draw="http://openoffice.org/2000/drawing" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:meta="http://openoffice.org/2000/meta" xmlns:number="http://openoffice.org/2000/datastyle" xmlns:svg="http://www.w3.org/2000/svg" xmlns:chart="http://openoffice.org/2000/chart" xmlns:dr3d="http://openoffice.org/2000/dr3d" xmlns:math="http://www.w3.org/1998/Math/MathML" xmlns:form="http://openoffice.org/2000/form" xmlns:script="http://openoffice.org/2000/script" xmlns:config="http://openoffice.org/2001/config" version="1.0" office:class="text" office:version="1.0">
    <xsl:output method="xml" indent="yes" omit-xml-declaration="no"/>
    <!--<xsl:output method="xml" version="1.0" encoding="UTF-8" doctype-public="-//OASIS//DTD DocBook XML V4.1.2//EN" doctype-system="http://www.oasis-open.org/docbook/xml/4.1.2/docbookx.dtd"/>-->
    <xsl:decimal-format name="staff" digit="D"/>

    <xsl:variable name="doc_type">
        <xsl:choose>
            <xsl:when test="/article">
                <xsl:text>article</xsl:text>
            </xsl:when>
            <xsl:when test="/chapter">
                <xsl:text>chapter</xsl:text>
            </xsl:when>
        </xsl:choose>
    </xsl:variable>


    <xsl:template match="/">
        <xsl:element name="office:document">
            <office:meta>
                <meta:generator>StarOffice 6.1 (Solaris Sparc)</meta:generator>
                <dc:title>
                    <xsl:choose>
                        <xsl:when test="contains( $doc_type, &apos;chapter&apos; )">
                            <xsl:value-of select="/chapter/chapterinfo/title"/>
                        </xsl:when>
                        <xsl:when test="contains( $doc_type, &apos;article&apos; )">
                            <xsl:value-of select="/article/articleinfo/title"/>
                        </xsl:when>
                    </xsl:choose>
                    <!--<xsl:value-of select="$doc_type"/>-->
                    <!--<xsl:value-of select="/article/articleinfo/title"/>-->

                </dc:title>
                <dc:description/>
                <dc:subject/>
                <meta:creation-date>2002-07-15T12:38:53</meta:creation-date>
                <dc:date>
                    <xsl:choose>
                        <xsl:when test="contains( $doc_type, &apos;chapter&apos; )">
                            <xsl:value-of select="/chapter/chapterinfo/pubdate"/>
                        </xsl:when>
                        <xsl:when test="contains( $doc_type, &apos;article&apos; )">
                            <xsl:value-of select="/article/articleinfo/pubdate"/>
                        </xsl:when>
                    </xsl:choose>
                    <!--<xsl:value-of select="article/articleinfo/pubdate"/>-->

                </dc:date>
                <dc:language>
                    <xsl:choose>
                        <xsl:when test="contains( $doc_type, &apos;chapter&apos; )">
                            <xsl:value-of select="chapter/@lang"/>
                        </xsl:when>
                        <xsl:when test="contains( $doc_type, &apos;article&apos; )">
                            <xsl:value-of select="article/@lang"/>
                        </xsl:when>
                    </xsl:choose>
                    <!--<xsl:value-of select="article/@lang"/>-->

                </dc:language>
                <meta:editing-cycles>21</meta:editing-cycles>
                <meta:editing-duration>P1DT0H11M54S</meta:editing-duration>
                <meta:user-defined meta:name="Info 1"/>
                <meta:user-defined meta:name="Info 2"/>
                <meta:user-defined meta:name="Info 3"/>
                <meta:user-defined meta:name="Info 4"/>
                <meta:document-statistic meta:table-count="1" meta:image-count="0" meta:object-count="0" meta:page-count="1" meta:paragraph-count="42" meta:word-count="144" meta:character-count="820"/>
            </office:meta>
            <office:automatic-styles>
                <style:style style:name="fr1" style:family="graphics" style:parent-style-name="Graphics">
                    <style:properties style:horizontal-pos="center" style:horizontal-rel="paragraph" style:mirror="none" fo:clip="rect(0cm 0cm 0cm 0cm)" draw:luminance="0%" draw:contrast="0%" draw:red="0%" draw:green="0%" draw:blue="0%" draw:gamma="1" draw:color-inversion="false" draw:transparency="-100%" draw:color-mode="standard"/>
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
   		    <xsl:call-template name="entities"/>
                <xsl:apply-templates/>
            </office:body>
        </xsl:element>
    </xsl:template>


<xsl:template name="entities">
	<xsl:element name="text:variable-decls">
	<xsl:for-each select="/descendant::entity">
		<xsl:variable name="entname"><xsl:value-of select="@name"/></xsl:variable>
			<xsl:if test="not(preceding::entity[@name = $entname])">
          			<xsl:element name="text:variable-decl">
       				<xsl:attribute name="text:value-type">
       					<xsl:text>string</xsl:text>	
       				 </xsl:attribute>
      					<xsl:attribute name="text:name">
						<xsl:text>entitydecl_</xsl:text><xsl:value-of select="@name"/>
      					</xsl:attribute>
      	 			</xsl:element>
      	 			</xsl:if>
      	  </xsl:for-each>
      	  </xsl:element>
</xsl:template>



<xsl:template match="entity">
	<xsl:variable name="entname"><xsl:value-of select="@name"/></xsl:variable>
	<xsl:choose>
		<xsl:when test="not(preceding::entity[@name = $entname])">
			<xsl:element name="text:variable-set">
	 			<xsl:attribute name="text:value-type">
             				<xsl:text>string</xsl:text>	
            		 	</xsl:attribute>
		 		<xsl:attribute name="text:name">
		 			<xsl:text>entitydecl_</xsl:text><xsl:value-of select="@name"/>
				</xsl:attribute>
				<xsl:apply-templates/>
			</xsl:element>
		</xsl:when>
		<xsl:otherwise>
			<xsl:element name="text:variable-get">
	 			<xsl:attribute name="text:value-type">
             				<xsl:text>string</xsl:text>	
            		 	</xsl:attribute>
		 		<xsl:attribute name="text:name">
		 			<xsl:text>entitydecl_</xsl:text><xsl:value-of select="@name"/>
				</xsl:attribute>
				<xsl:apply-templates/>
			</xsl:element>
		</xsl:otherwise>
	</xsl:choose>
	 
</xsl:template>



    <!-- table start -->

    <xsl:template match="table">
        <xsl:variable name="tabletitle">
            <xsl:value-of select="title"/>
        </xsl:variable>
        <xsl:element name="table:table">
            <xsl:attribute name="table:name"/>
            <xsl:attribute name="table:style-name">Table1</xsl:attribute>
            <xsl:attribute name="table:name">
                <xsl:value-of select="@id"/>
            </xsl:attribute>
            <blah>blah</blah>
            <xsl:apply-templates/>
        </xsl:element>
        <xsl:if test="not($tabletitle=&apos;&apos;)">
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
                <xsl:when test="@cols &gt;0">
                    <xsl:attribute name="table:number-columns-repeated">
                        <xsl:value-of select="@cols"/>
                    </xsl:attribute>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:attribute name="table:number-columns-repeated">
                        <xsl:value-of select="count(child::tbody/row/entry) div count(child::tbody/row) "/>
                    </xsl:attribute>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:element>
        <xsl:apply-templates/>
    </xsl:template>


    <xsl:template match="thead">
        <xsl:element name="table:table-header-rows">
            <xsl:apply-templates/>
        </xsl:element>
    </xsl:template>


    <xsl:template match="tbody">
        <xsl:apply-templates/>
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

                    <xsl:variable name="sname">
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
                    </xsl:attribute>
                </xsl:when>
                <xsl:when test="@namest and @nameend">
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
                    </xsl:attribute>
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
            <xsl:when test="parent::figure"/>
            <xsl:when test="parent::table"/>
            <xsl:when test="parent::sect1"/>
            <xsl:when test="parent::sect2"/>
            <xsl:when test="parent::sect3"/>
            <xsl:when test="parent::sect4"/>
            <xsl:when test="parent::sect5"/>
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
                <xsl:if test="/article/articleinfo/subtitle !=&apos;&apos;">
                    <xsl:element name="text:p">
                        <xsl:attribute name="text:style-name">Document SubTitle</xsl:attribute>
                        <xsl:value-of select="/article/articleinfo/subtitle"/>
                    </xsl:element>
                </xsl:if>
            <xsl:apply-templates/>
        </xsl:element>
    </xsl:template>


    <xsl:template match="chapterinfo">
        <xsl:element name="text:section">
            <xsl:attribute name="text:style-name">ChapterInfo</xsl:attribute>
            <xsl:attribute name="text:name">ChapterInfo</xsl:attribute>
            <xsl:if test="/chapter/chapterinfo/title !=&apos;&apos; ">
                <xsl:element name="text:p">
                    <xsl:attribute name="text:style-name">Document Title</xsl:attribute>
                    <xsl:value-of select="/chapter/chapterinfo/title"/>
                </xsl:element>
                <xsl:if test="/chapter/chapterinfo/subtitle !=&apos;&apos;">
                    <xsl:element name="text:p">
                        <xsl:attribute name="text:style-name">Document SubTitle</xsl:attribute>
                        <xsl:value-of select="/chapter/chapterinfo/subtitle"/>
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
</xsl:template>
-->

    <xsl:template match="articleinfo/title | chapterinfo/title">
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
    </xsl:element>
  -->
    </xsl:template>
    
    
   <xsl:template match="articleinfo/title">
	<xsl:element name="text:p">
		<xsl:attribute name="text:style-name">Document Title</xsl:attribute>
		<xsl:apply-templates/>
	</xsl:element>
</xsl:template>

<xsl:template match="date">
	<xsl:element name="text:s"/>
	<xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>date_</xsl:text><xsl:value-of select="count(preceding::date)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>date_</xsl:text><xsl:value-of select="count(preceding::date)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:element name="text:s"/>
</xsl:template>

<xsl:template match="revision">
	<xsl:element name="text:s"/>
	<xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>revision_</xsl:text><xsl:value-of select="count(preceding::revision)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>revision_</xsl:text><xsl:value-of select="count(preceding::revision)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:element name="text:s"/>
	<xsl:element name="text:s"/>
</xsl:template>

<xsl:template match="revnumber">
	<xsl:element name="text:s"/>
	<xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>revnumber_</xsl:text><xsl:value-of select="count(preceding::revnumber)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>revnumber_</xsl:text><xsl:value-of select="count(preceding::revnumber)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:element name="text:s"/>
</xsl:template>

<xsl:template match="revdescription">
	<xsl:element name="text:s"/>
		<xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>revdescription_</xsl:text><xsl:value-of select="count(preceding::revdescription)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>revdescription_</xsl:text><xsl:value-of select="count(preceding::revdescription)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:element name="text:s"/>
</xsl:template>

<xsl:template match="revhistory">
	<xsl:element name="text:p">
		<xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>revhistory_</xsl:text><xsl:value-of select="count(preceding::revhistory)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>revhistory_</xsl:text><xsl:value-of select="count(preceding::revhistory)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:element name="text:line-break"/>
	</xsl:element>
</xsl:template>

<xsl:template match="legalnotice">
	<xsl:element name="text:p">
		<xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>legalnotice_</xsl:text><xsl:value-of select="count(preceding::legalnotice)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>legalnotice_</xsl:text><xsl:value-of select="count(preceding::legalnotice)"/>
		</xsl:attribute>
	</xsl:element>
	</xsl:element>
</xsl:template>

<xsl:template match="legalnotice/title">
	<xsl:element name="text:s"/>
	<xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>title_</xsl:text><xsl:value-of select="count(preceding::legalnotice/title)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>title_</xsl:text><xsl:value-of select="count(preceding::legalnotice/title)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:element name="text:s"/>
</xsl:template>

<xsl:template match="para[ancestor::articleinfo]">
	<xsl:element name="text:s"/>

		<xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>para_</xsl:text><xsl:value-of select="count(preceding::para[ancestor::articleinfo])"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>para_</xsl:text><xsl:value-of select="count(preceding::para[ancestor::articleinfo])"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:element name="text:s"/>
</xsl:template>


<xsl:template match="articleinfo/subtitle">
	<xsl:element name="text:p">
		<xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>subtitle_</xsl:text><xsl:value-of select="count(preceding::articleinfo/subtitle)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>subtitle_</xsl:text><xsl:value-of select="count(preceding::articleinfo/subtitle)"/>
		</xsl:attribute>
	</xsl:element>
	</xsl:element>
</xsl:template>

<xsl:template match="articleinfo/edition">
		<xsl:element name="text:p">
		<xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>edition_</xsl:text><xsl:value-of select="count(preceding::articleinfo/edition)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>edition_</xsl:text><xsl:value-of select="count(preceding::articleinfo/edition)"/>
		</xsl:attribute>
	</xsl:element>
	</xsl:element>
</xsl:template>

<xsl:template match="articleinfo/releaseinfo">
      <xsl:element name="text:p">
     	<xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>releaseinfo_</xsl:text><xsl:value-of select="count(preceding::articleinfo/releaseinfo)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>releaseinfo_</xsl:text><xsl:value-of select="count(preceding::articleinfo/releaseinfo)"/>
		</xsl:attribute>
	</xsl:element>
	</xsl:element>
</xsl:template>


<xsl:template match="author/firstname">
	<xsl:element name="text:s"/>
         <xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>firstname_</xsl:text><xsl:value-of select="count(preceding::author/firstname)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>firstname_</xsl:text><xsl:value-of select="count(preceding::author/firstname)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:element name="text:s"/>

</xsl:template>



<xsl:template match="year[ancestor::articleinfo]">
	<xsl:element name="text:s"/>

        	<xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>year_</xsl:text><xsl:value-of select="count(preceding::year[ancestor::articleinfo])"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>year_</xsl:text><xsl:value-of select="count(preceding::year[ancestor::articleinfo])"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:element name="text:s"/>

</xsl:template>

<xsl:template match="authorgroup">
	<xsl:element name="text:p">
	<xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>authorgroup_</xsl:text><xsl:value-of select="count(preceding::authorgroup)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>authorgroup_</xsl:text><xsl:value-of select="count(preceding::authorgroup)"/>
		</xsl:attribute>
	</xsl:element>
	</xsl:element>
</xsl:template>

<xsl:template match="articleinfo/copyright/holder">
	<xsl:element name="text:s"/>
       <xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>holder_</xsl:text><xsl:value-of select="count(preceding::articleinfo/copyright/holder)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>holder_</xsl:text><xsl:value-of select="count(preceding::articleinfo/copyright/holder)"/>
		</xsl:attribute>
	</xsl:element>
	<!--<xsl:element name="text:s"/>-->
	<xsl:element name="text:line-break"/>
</xsl:template>

<xsl:template match="articleinfo/copyright">
	<xsl:element name="text:p">
       <xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>copyright_</xsl:text><xsl:value-of select="count(preceding::articleinfo/copyright)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>copyright_</xsl:text><xsl:value-of select="count(preceding::articleinfo/copyright)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:element name="text:s"/>
	</xsl:element>
</xsl:template>


<xsl:template name="affiliation">
	<xsl:element name="text:s"/>
	 <xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>affiliation_</xsl:text><xsl:value-of select="count(preceding::affiliation)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>affiliation_</xsl:text><xsl:value-of select="count(preceding::affiliation)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:element name="text:s"/>
</xsl:template>

<xsl:template match="author/affiliation/address">
	<xsl:element name="text:p">
		 <xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>address_</xsl:text><xsl:value-of select="count(preceding::author/affiliation/address)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>address_</xsl:text><xsl:value-of select="count(preceding::author/affiliation/address)"/>
		</xsl:attribute>
	</xsl:element>
</xsl:element>
</xsl:template>

<xsl:template match="authorgroup">
	<xsl:element name="text:p">
    		 <xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>authorgroup_</xsl:text><xsl:value-of select="count(preceding::authorgroup)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>authorgroup_</xsl:text><xsl:value-of select="count(preceding::authorgroup)"/>
		</xsl:attribute>
	</xsl:element>
</xsl:element>
</xsl:template>


<xsl:template match="author">
	<xsl:element name="text:s"/>
    	<xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>author_</xsl:text><xsl:value-of select="count(preceding::author)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>author_</xsl:text><xsl:value-of select="count(preceding::author)"/>
		</xsl:attribute>
	</xsl:element>	
	<!--<xsl:element name="text:s"/>-->
	<xsl:element name="text:line-break"/>
</xsl:template>

<xsl:template match="author/affiliation">
	<xsl:element name="text:s"/>
    	 <xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>affiliation_</xsl:text><xsl:value-of select="count(preceding::author/affiliation)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>affiliation_</xsl:text><xsl:value-of select="count(preceding::author/affiliation)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:element name="text:s"/>
</xsl:template>

<xsl:template match="author/affiliation/address">
	<xsl:element name="text:s"/>
    	 <xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>address_</xsl:text><xsl:value-of select="count(preceding::author/affiliation/address)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>address_</xsl:text><xsl:value-of select="count(preceding::author/affiliation/address)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:element name="text:s"/>
</xsl:template>


<xsl:template match="email[ancestor::articleinfo]">
	<xsl:element name="text:s"/>

    	 <xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>email_</xsl:text><xsl:value-of select="count(preceding::email[ancestor::articleinfo])"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>email_</xsl:text><xsl:value-of select="count(preceding::email[ancestor::articleinfo])"/>
		</xsl:attribute>
	</xsl:element>
		<xsl:element name="text:s"/>

</xsl:template>

<xsl:template match="author/affiliation/orgname">
	<xsl:element name="text:s"/>

    	<xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>orgname_</xsl:text><xsl:value-of select="count(preceding::author/affiliation/orgname)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>orgname_</xsl:text><xsl:value-of select="count(preceding::author/affiliation/orgname)"/>
		</xsl:attribute>
	</xsl:element>
		<xsl:element name="text:s"/>

</xsl:template>


<xsl:template match="author/surname">
	<xsl:element name="text:s"/>

   	<xsl:element name="text:bookmark-start">
		<xsl:attribute name="text:name">
			<xsl:text>surname_</xsl:text><xsl:value-of select="count(preceding::author/surname)"/>
		</xsl:attribute>
	</xsl:element>
	<xsl:apply-templates/>
	<xsl:element name="text:bookmark-end">
		<xsl:attribute name="text:name">
			<xsl:text>surname_</xsl:text><xsl:value-of select="count(preceding::author/surname)"/>
		</xsl:attribute>
	</xsl:element>
				<xsl:element name="text:s"/>


</xsl:template>


    
    <xsl:template match="para">
        <xsl:choose>
            <xsl:when test="ancestor::varlistentry">
                <xsl:element name="text:p">
                    <xsl:attribute name="text:style-name">VarList Term</xsl:attribute>
                    <xsl:apply-templates/>
                </xsl:element>
            </xsl:when>
            <xsl:otherwise>
                <xsl:element name="text:p">
                    <xsl:choose>
                        <xsl:when test="ancestor-or-self::footnote">
                            <xsl:attribute name="text:style-name">
                                <xsl:text>Footnote</xsl:text>
                            </xsl:attribute>
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
                    <xsl:choose>
                        <xsl:when test="@id">
                            <xsl:call-template name="test.id"/>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:apply-templates/>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:element>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    
    
    <xsl:template match="section">
        <xsl:element name="text:h">
            <xsl:attribute name="text:level">
                <xsl:value-of select="count(ancestor-or-self::section) "/>
            </xsl:attribute>
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
            <xsl:choose>
                <xsl:when test="@id">
                    <xsl:call-template name="test.id"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="child::title"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:element>
        <xsl:apply-templates/>
    </xsl:template>
    
    
    <xsl:template match="sect2">
        <xsl:element name="text:h">
            <xsl:attribute name="text:level">2</xsl:attribute>
            <xsl:choose>
                <xsl:when test="@id">
                    <xsl:call-template name="test.id"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="child::title"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:element>
        <xsl:apply-templates/>
    </xsl:template>
    
    
    <xsl:template match="sect3">
        <xsl:element name="text:h">
            <xsl:attribute name="text:level">3</xsl:attribute>
            <xsl:choose>
                <xsl:when test="@id">
                    <xsl:call-template name="test.id"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="child::title"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:element>
        <xsl:apply-templates/>
    </xsl:template>
    
    
    <xsl:template match="sect4">
        <xsl:element name="text:h">
            <xsl:attribute name="text:level">4</xsl:attribute>
            <xsl:choose>
                <xsl:when test="@id">
                    <xsl:call-template name="test.id"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="child::title"/>
                </xsl:otherwise>
            </xsl:choose>
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
            <xsl:attribute name="table:name"/>
            <xsl:attribute name="table:style-name">Table1</xsl:attribute>
            <xsl:attribute name="table:name">
                <xsl:value-of select="@id"/>
            </xsl:attribute>
            <xsl:apply-templates/>
        </xsl:element>
    </xsl:template>
    
    <xsl:template match="indexterm"/>


    <xsl:template match="figure">
        <xsl:apply-templates/>
    </xsl:template>
    <!--  lists          Section                                          -->

    <xsl:template match="itemizedlist">
        <xsl:element name="text:unordered-list">
            <xsl:attribute name="text:style-name">UnOrdered List</xsl:attribute>
            <xsl:attribute name="text:continue-numbering">false</xsl:attribute>
            <xsl:apply-templates/>
        </xsl:element>
    </xsl:template>


    <xsl:template match="variablelist">
        <xsl:element name="text:unordered-list">
            <xsl:attribute name="text:style-name">Var List</xsl:attribute>
            <xsl:attribute name="text:continue-numbering">false</xsl:attribute>
            <xsl:apply-templates/>
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
                    <xsl:apply-templates/>
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
        <xsl:variable name="pos"><xsl:value-of select="count(preceding::menuchoice)"/></xsl:variable>
        <xsl:element name="text:bookmark-start">
            <xsl:attribute name="text:name">
                <xsl:text>menuchoice_</xsl:text><xsl:value-of select="$pos"/>
            </xsl:attribute>
        </xsl:element>
        <xsl:apply-templates/>
        <xsl:element name="text:bookmark-end">
            <xsl:attribute name="text:name">
                <xsl:text>menuchoice_</xsl:text><xsl:value-of select="$pos"/>
            </xsl:attribute>
        </xsl:element>
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

<!-- Change Made By Kevin Fowlks (fowlks@msu.edu) June 4th, 2003 -->
    <xsl:template match="emphasis">
        <xsl:element name="text:span">
		<xsl:choose>
			<xsl:when test="@role">
				<xsl:attribute name="text:style-name">Emphasis Bold</xsl:attribute>
				<xsl:apply-templates/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:attribute name="text:style-name">Emphasis</xsl:attribute>
				<xsl:apply-templates/>	
			</xsl:otherwise>
		</xsl:choose>
        </xsl:element>
    </xsl:template>

<!-- Change Made By Kevin Fowlks (fowlks@msu.edu) June 16th, 2003 -->
    <xsl:template match="quote">
        <xsl:element name="text:span">
            <xsl:attribute name="text:style-name">Citation</xsl:attribute>
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
            <xsl:attribute name="draw:name"/>
            <xsl:attribute name="text:anchor-type"/>
            <xsl:attribute name="draw:z-index"/>
            <xsl:attribute name="xlink:href">
                <xsl:value-of select="@fileref"/>
            </xsl:attribute>
            <xsl:attribute name="xlink:type"/>
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
            <xsl:attribute name="xlink:type">
                <xsl:text>simple</xsl:text>
            </xsl:attribute>
            <xsl:attribute name="xlink:href">
                <xsl:value-of select="@url"/>
            </xsl:attribute>
            <xsl:apply-templates/>
        </xsl:element>
    </xsl:template>


    <xsl:template match="link">
        <xsl:element name="text:a">
            <xsl:attribute name="xlink:type">
                <xsl:text>simple</xsl:text>
            </xsl:attribute>
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
            <xsl:attribute name="xlink:type">
                <xsl:text>simple</xsl:text>
            </xsl:attribute>
            <xsl:attribute name="xlink:href">
                <xsl:value-of select="@targetdocent"/>
            </xsl:attribute>
            <xsl:apply-templates/>
        </xsl:element>
    </xsl:template>

    <!-- xref maps to reference-ref i.e an OOo insert reference
    xref is an empty element, reference-ref spans the object to be referenced -->
    <xsl:template match="xref">
        <xsl:element name="text:reference-ref">
            <xsl:attribute name="text:reference-format">
                <xsl:text>text</xsl:text>
            </xsl:attribute>
            <xsl:attribute name="text:ref-name">
                <xsl:value-of select="@linkend"/>
            </xsl:attribute>
        </xsl:element>
    </xsl:template>


    <!-- the DocBook elements that have been "xreffed" contain an ID attribute that is linked to by an xref @linkend or @endterm -->
    <!-- the @id can also be pointed to by an indexterm -->

    <xsl:template name="test.id">
        <xsl:if test="@id">
            <xsl:variable name="id.val">
                <xsl:value-of select="@id"/>
            </xsl:variable>

            <xsl:element name="text:reference-mark-start">
                <xsl:attribute name="text:name">
                    <xsl:value-of select="@id"/>
                </xsl:attribute>
            </xsl:element>

            <xsl:choose>
                <xsl:when test="self::para">
                    <xsl:apply-templates/>
                </xsl:when>
                <xsl:otherwise>
                    <!-- sect* -->
                    <xsl:value-of select="child::title"/>
                </xsl:otherwise>
            </xsl:choose>

            <xsl:element name="text:reference-mark-end">
                <xsl:attribute name="text:name">
                    <xsl:value-of select="@id"/>
                </xsl:attribute>
            </xsl:element>

        </xsl:if>
    </xsl:template>


    <xsl:template match="indexterm">
        <xsl:if test="@class = &apos;startofrange&apos;">
            <xsl:element name="text:alphabetical-index-mark-start">
                <xsl:attribute name="text:id">
                    <xsl:value-of select="@id"/>
                </xsl:attribute>
                <!--<xsl:if test="primary">-->

                <xsl:attribute name="text:key1">
                    <xsl:value-of select="primary"/>
                </xsl:attribute>
                <!--</xsl:if>-->

                <xsl:if test="secondary">--&gt;
                    <xsl:attribute name="text:key2">
                        <xsl:value-of select="secondary"/>
                    </xsl:attribute>
                </xsl:if>
            </xsl:element>
        </xsl:if>
        <xsl:if test="@class = &apos;endofrange&apos;">
            <xsl:element name="text:alphabetical-index-mark-end">
                <xsl:attribute name="text:id">
                    <xsl:value-of select="@startref"/>
                </xsl:attribute>
            </xsl:element>
        </xsl:if>
    </xsl:template>


    <xsl:template match="index">
        <xsl:element name="text:alphabetical-index">
            <xsl:attribute name="text:style-name">
                <xsl:text disable-output-escaping="yes">Sect1</xsl:text>
            </xsl:attribute>
            <xsl:attribute name="text:protected">
                <xsl:text disable-output-escaping="yes">true</xsl:text>
            </xsl:attribute>
            <xsl:attribute name="text:name">
                <xsl:value-of select="title"/>
            </xsl:attribute>
            <xsl:element name="text:alphabetical-index-source">
                <xsl:attribute name="text:main-entry-style-name">
                    <xsl:text disable-output-escaping="yes">Main index entry</xsl:text>
                </xsl:attribute>
                <xsl:attribute name="text:sort-algorithm">
                    <xsl:text disable-output-escaping="yes">alphanumeric</xsl:text>
                </xsl:attribute>
                <xsl:attribute name="fo:language">
                    <xsl:text disable-output-escaping="yes">en</xsl:text>
                </xsl:attribute>
                <xsl:attribute name="fo:country">
                    <xsl:text disable-output-escaping="yes">IE</xsl:text>
                </xsl:attribute>
                <xsl:element name="text:index-title-template">
                    <xsl:attribute name="text:style-name">Index Heading</xsl:attribute>
                    <xsl:value-of select="title"/>
                </xsl:element>
                <xsl:element name="text:alphabetical-index-entry-template">
                    <xsl:attribute name="text:outline-level">
                        <xsl:text disable-output-escaping="yes">separator</xsl:text>
                    </xsl:attribute>
                    <xsl:attribute name="text:style-name">
                        <xsl:text disable-output-escaping="yes">Index Separator</xsl:text>
                    </xsl:attribute>
                    <xsl:element name="text:index-entry-text"/>
                </xsl:element>
                <xsl:element name="text:alphabetical-index-entry-template">
                    <xsl:attribute name="text:outline-level">
                        <xsl:text disable-output-escaping="yes">1</xsl:text>
                    </xsl:attribute>
                    <xsl:attribute name="text:style-name">
                        <xsl:text disable-output-escaping="yes">Index 1</xsl:text>
                    </xsl:attribute>
                    <xsl:element name="text:index-entry-text"/>
                    <xsl:element name="text:index-entry-tab-stop">
                        <xsl:attribute name="style:type">
                            <xsl:text disable-output-escaping="yes">left</xsl:text>
                        </xsl:attribute>
                        <xsl:attribute name="style:position">
                            <xsl:text disable-output-escaping="yes">0cm</xsl:text>
                        </xsl:attribute>
                        <xsl:attribute name="style:leader-char">
                            <xsl:text disable-output-escaping="yes"/>
                        </xsl:attribute>
                    </xsl:element>
                </xsl:element>
                <xsl:element name="text:alphabetical-index-entry-template">
                    <xsl:attribute name="text:outline-level">
                        <xsl:text disable-output-escaping="yes">2</xsl:text>
                    </xsl:attribute>
                    <xsl:attribute name="text:style-name">
                        <xsl:text disable-output-escaping="yes">Index 2</xsl:text>
                    </xsl:attribute>
                    <xsl:element name="text:index-entry-text"/>
                    <xsl:element name="text:index-entry-tab-stop">
                        <xsl:attribute name="style:type">
                            <xsl:text disable-output-escaping="yes">left</xsl:text>
                        </xsl:attribute>
                        <xsl:attribute name="style:position">
                            <xsl:text disable-output-escaping="yes">0cm</xsl:text>
                        </xsl:attribute>
                        <xsl:attribute name="style:leader-char">
                            <xsl:text disable-output-escaping="yes"/>
                        </xsl:attribute>
                    </xsl:element>
                </xsl:element>
                <xsl:element name="text:alphabetical-index-entry-template">
                    <xsl:attribute name="text:outline-level">
                        <xsl:text disable-output-escaping="yes">3</xsl:text>
                    </xsl:attribute>
                    <xsl:attribute name="text:style-name">
                        <xsl:text disable-output-escaping="yes">Index 3</xsl:text>
                    </xsl:attribute>
                    <xsl:element name="text:index-entry-text"/>
                    <xsl:element name="text:index-entry-tab-stop">
                        <xsl:attribute name="style:type">
                            <xsl:text disable-output-escaping="yes">left</xsl:text>
                        </xsl:attribute>
                        <xsl:attribute name="style:position">
                            <xsl:text disable-output-escaping="yes">0cm</xsl:text>
                        </xsl:attribute>
                        <xsl:attribute name="style:leader-char">
                            <xsl:text disable-output-escaping="yes"/>
                        </xsl:attribute>
                    </xsl:element>
                </xsl:element>
            </xsl:element>

            <xsl:element name="text:index-body">
                <xsl:element name="text:index-title">
                    <xsl:attribute name="text:style-name">
                        <xsl:text disable-output-escaping="yes">Sect1</xsl:text>
                    </xsl:attribute>
                    <xsl:attribute name="text:name">
                        <xsl:text disable-output-escaping="yes">Alphabetical Index1_Head</xsl:text>
                    </xsl:attribute>
                    <xsl:element name="text:p">
                        <xsl:attribute name="text:style-name">
                            <xsl:text disable-output-escaping="yes">Index Heading</xsl:text>
                        </xsl:attribute>
                        <xsl:value-of select="title"/>
                    </xsl:element>
                </xsl:element>
                <xsl:apply-templates select="indexentry"/>
            </xsl:element>
        </xsl:element>
    </xsl:template>


    <xsl:template match="indexentry">
        <xsl:element name="text:p">
            <xsl:attribute name="text:style-name">
                <xsl:text disable-output-escaping="yes">Index 1</xsl:text>
            </xsl:attribute>
            <xsl:value-of select="primaryie"/>
            <xsl:element name="text:tab-stop"/>
        </xsl:element>
        <xsl:if test="secondaryie">
            <xsl:element name="text:p">
                <xsl:attribute name="text:style-name">
                    <xsl:text disable-output-escaping="yes">Index 2</xsl:text>
                </xsl:attribute>
                <xsl:value-of select="secondaryie"/>
                <xsl:element name="text:tab-stop"/>
            </xsl:element>
        </xsl:if>
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


    <xsl:template match="textobject"/>


    <xsl:template match="caption">
        <xsl:apply-templates/>
    </xsl:template>


    <xsl:template match="imagedata">
        <xsl:element name="draw:image">
            <xsl:attribute name="draw:style-name">
                <xsl:text>fr1</xsl:text>
            </xsl:attribute>
            <xsl:attribute name="draw:name"/>
            <xsl:attribute name="text:anchor-type"/>
            <xsl:attribute name="draw:z-index"/>
            <xsl:attribute name="xlink:href">
                <xsl:value-of select="@fileref"/>
            </xsl:attribute>
            <xsl:attribute name="xlink:type"/>
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
            <xsl:attribute name="draw:name"/>
            <xsl:attribute name="text:anchor-type"/>
            <xsl:attribute name="draw:z-index"/>
            <xsl:attribute name="xlink:href">
                <xsl:value-of select="@fileref"/>
            </xsl:attribute>
            <xsl:attribute name="xlink:type"/>
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
            <xsl:attribute name="text:style-name">
                <xsl:text>Mediaobject</xsl:text>
            </xsl:attribute>
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


    <xsl:template match="comment()"> 
       <xsl:element name="text:p">
           <xsl:attribute name="text:style-name">XMLComment</xsl:attribute>
           <xsl:value-of select="."/>
       </xsl:element>
    </xsl:template>
</xsl:stylesheet>
