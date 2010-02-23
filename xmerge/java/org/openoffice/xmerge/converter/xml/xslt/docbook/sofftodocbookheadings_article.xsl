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

<xsl:stylesheet version="1.0" xmlns:style="http://openoffice.org/2000/style" xmlns:text="http://openoffice.org/2000/text" xmlns:office="http://openoffice.org/2000/office" xmlns:table="http://openoffice.org/2000/table" xmlns:draw="http://openoffice.org/2000/drawing" xmlns:fo="http://www.w3.org/1999/XSL/Format" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:meta="http://openoffice.org/2000/meta" xmlns:number="http://openoffice.org/2000/datastyle" xmlns:svg="http://www.w3.org/2000/svg" xmlns:chart="http://openoffice.org/2000/chart" xmlns:dr3d="http://openoffice.org/2000/dr3d" xmlns:math="http://www.w3.org/1998/Math/MathML" xmlns:form="http://openoffice.org/2000/form" xmlns:script="http://openoffice.org/2000/script" xmlns:config="http://openoffice.org/2001/config" office:class="text" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" exclude-result-prefixes="office meta  table number dc fo xlink chart math script xsl draw svg dr3d form config text style">

    <xsl:key name='headchildren' match="text:p | text:alphabetical-index | table:table | text:span | text:ordered-list | office:annotation | text:unordered-list | text:footnote | text:a | text:list-item | draw:plugin | draw:text-box | text:footnote-body | text:section"
   use="generate-id((..|preceding-sibling::text:h[@text:level='1']|preceding-sibling::text:h[@text:level='2']|preceding-sibling::text:h[@text:level='3']|preceding-sibling::text:h[@text:level='4']|preceding-sibling::text:h[@text:level='5'])[last()])"/>

    <xsl:key name="children" match="text:h[@text:level='2']"
   use="generate-id(preceding-sibling::text:h[@text:level='1'][1])"/>

    <xsl:key name="children" match="text:h[@text:level='3']"
   use="generate-id(preceding-sibling::text:h[@text:level='2' or @text:level='1'][1])"/>

    <xsl:key name="children" match="text:h[@text:level='4']"
   use="generate-id(preceding-sibling::text:h[@text:level='3' or @text:level='2' or @text:level='1'][1])"/>

    <xsl:key name="children" match="text:h[@text:level='5']"
   use="generate-id(preceding-sibling::text:h[@text:level='4' or @text:level='3' or @text:level='2' or @text:level='1'][1])"/>

    <xsl:key name="secondary_children" match="text:p[@text:style-name = 'Index 2']"
   use="generate-id(preceding-sibling::text:p[@text:style-name = 'Index 1'][1])"/>
    
<!-- -->
    <xsl:template match="/office:document">
	<xsl:text disable-output-escaping="yes">&lt;!DOCTYPE article PUBLIC "-//OASIS//DTD DocBook XML V4.1.2//EN" "http://www.oasis-open.org/docbook/xml/4.1.2/docbookx.dtd" [
	</xsl:text>
	<xsl:for-each select="descendant::text:variable-decl">
		<xsl:variable name="name"><xsl:value-of select="@text:name"/></xsl:variable>
		<xsl:if test="contains(@text:name,'entitydecl')">
			<xsl:text disable-output-escaping="yes">&lt;!ENTITY </xsl:text><xsl:value-of select="substring-after(@text:name,'entitydecl_')"/><xsl:text> "</xsl:text><xsl:value-of select="/descendant::text:variable-set[@text:name= $name][1]"/><xsl:text disable-output-escaping="yes">"&gt;</xsl:text>
		</xsl:if>
	</xsl:for-each>

	<xsl:text disable-output-escaping="yes">]&gt;</xsl:text>
        <article>
            <xsl:attribute name="lang">
                <xsl:value-of select="/office:document/office:meta/dc:language"/>
            </xsl:attribute>
            <xsl:apply-templates/>
        </article>
    </xsl:template>


    <xsl:template match="text:section">
        <xsl:choose>
	<xsl:when test="@text:name='ArticleInfo'">
		<articleinfo>
			<xsl:apply-templates/>
		</articleinfo>
	</xsl:when>
	<xsl:when test="@text:name='Abstract'">
		<abstract>
			<xsl:apply-templates/>
		</abstract>
	</xsl:when>
	<xsl:when test="@text:name='Appendix'">
		<appendix>
			<xsl:apply-templates/>
		</appendix>
	</xsl:when>
	<xsl:otherwise>
		<xsl:variable name="sectvar"><xsl:text>sect</xsl:text><xsl:value-of select="count(ancestor::text:section)+1"/></xsl:variable>	
		<xsl:variable name="idvar"><xsl:text> id="</xsl:text><xsl:value-of select="@text:name"/><xsl:text>"</xsl:text></xsl:variable>
		<xsl:text disable-output-escaping="yes">&lt;</xsl:text><xsl:value-of select="$sectvar"/><xsl:value-of select="$idvar"/><xsl:text  disable-output-escaping="yes">&gt;</xsl:text>
			<xsl:apply-templates/>
		<xsl:text disable-output-escaping="yes">&lt;/</xsl:text><xsl:value-of select="$sectvar"/><xsl:text  disable-output-escaping="yes">&gt;</xsl:text>
	</xsl:otherwise>
</xsl:choose>

    </xsl:template>

    <xsl:template match="text:h[@text:level=&apos;1&apos;]">
        <xsl:choose>
            <xsl:when test=".=&apos;Abstract&apos;">
                <abstract>
                    <xsl:apply-templates select="key(&apos;headchildren&apos;, generate-id())"/>
                    <xsl:apply-templates select="key(&apos;children&apos;, generate-id())"/>
                </abstract>
            </xsl:when>
            <xsl:otherwise>
                <xsl:call-template name="make-section">
                    <xsl:with-param name="current" select="@text:level"/>
                    <xsl:with-param name="prev" select="1"/>
                </xsl:call-template>
                <!--<sect1>
                 <title>
                       <xsl:apply-templates/>
                 </title>
                     <xsl:apply-templates select="key('headchildren', generate-id())"/>
                 <xsl:apply-templates select="key('children', generate-id())"/>
           </sect1>-->

            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>


    <xsl:template match="text:h[@text:level=&apos;2&apos;] | text:h[@text:level=&apos;3&apos;]| text:h[@text:level=&apos;4&apos;] | text:h[@text:level=&apos;5&apos;]">
        <xsl:variable name="level" select="@text:level"/>
        <xsl:call-template name="make-section">
            <xsl:with-param name="current" select="$level"/>
            <xsl:with-param name="prev" select="preceding-sibling::text:h[@text:level &lt; $level][1]/@text:level "/>
        </xsl:call-template>
    </xsl:template>


    <xsl:template name="make-section">
        <xsl:param name="current"/>
        <xsl:param name="prev"/>
        <xsl:choose>
            <xsl:when test="$current &gt; $prev+1">
                <xsl:variable name="sect.element">
                    <xsl:text disable-output-escaping="yes">sect</xsl:text>
                    <xsl:value-of select="$prev +1"/>
                </xsl:variable>
                <!--<xsl:text disable-output-escaping="yes">&lt;sect</xsl:text><xsl:value-of select="$prev +1"/><xsl:text disable-output-escaping="yes">&gt;</xsl:text>-->

                <xsl:element name="{$sect.element}">
                    <xsl:call-template name="id.attribute"/>
                    <title/>
                    <xsl:call-template name="make-section">
                        <xsl:with-param name="current" select="$current"/>
                        <xsl:with-param name="prev" select="$prev +1"/>
                    </xsl:call-template>
                    <!--<xsl:text disable-output-escaping="yes">&lt;/sect</xsl:text><xsl:value-of select="$prev +1 "/><xsl:text disable-output-escaping="yes">&gt;</xsl:text>-->

                </xsl:element>
            </xsl:when>
            <xsl:otherwise>
                <xsl:variable name="sect.element">
                    <xsl:text disable-output-escaping="yes">sect</xsl:text>
                    <xsl:value-of select="$current"/>
                </xsl:variable>
                <!--<xsl:text disable-output-escaping="yes">&lt;sect</xsl:text><xsl:value-of select="$current"/><xsl:text disable-output-escaping="yes">&gt;</xsl:text>-->

                <xsl:element name="{$sect.element}">
                    <xsl:call-template name="id.attribute"/>
                    <title>
                        <xsl:apply-templates/>
                    </title>
                    <xsl:apply-templates select="key(&apos;headchildren&apos;, generate-id())"/>
                    <xsl:apply-templates select="key(&apos;children&apos;, generate-id())"/>
                    <!--<xsl:text disable-output-escaping="yes">&lt;/sect</xsl:text><xsl:value-of select="$current"/><xsl:text disable-output-escaping="yes">&gt;</xsl:text>-->

                </xsl:element>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template match="text:variable-set|text:variable-get">
	<xsl:choose>
		<xsl:when test="contains(@text:name,'entitydecl')">
			<xsl:text disable-output-escaping="yes">&amp;</xsl:text><xsl:value-of select="substring-after(@text:name,'entitydecl_')"/><xsl:text disable-output-escaping="yes">;</xsl:text>
		</xsl:when>
	</xsl:choose>
     </xsl:template>


    <xsl:template match="text:p[@text:style-name='XMLComment']">
         <xsl:comment>
             <xsl:value-of select="."/>
         </xsl:comment>
    </xsl:template>

    <xsl:template match="text:section[@text:name = 'ArticleInfo']/text:p[not(@text:style-name='XMLComment')]">
           <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="text:p">
        <!--text:span text:style-name="XrefLabel" -->
        <!--<xsl:if test="not( child::text:span[@text:style-name = 'XrefLabel'] )"> -->

        <xsl:element name="para">
            <xsl:if test="child::text:reference-mark-start">
                <xsl:value-of select="."/>
            </xsl:if>
            <xsl:apply-templates/>
        </xsl:element>
        <!-- </xsl:if> -->

    </xsl:template>


    <xsl:template match="office:meta">
        <xsl:apply-templates/>
    </xsl:template>


    <xsl:template match="text:ordered-list">
        <orderedlist>
            <xsl:apply-templates/>
        </orderedlist>
    </xsl:template>

    <xsl:template match="meta:editing-cycles"/>

    <xsl:template match="meta:user-defined"/>

    <xsl:template match="meta:editing-duration"/>

    <xsl:template match="dc:language"/>

    <xsl:template match="dc:date">
    <!--<pubdate>
        <xsl:value-of select="substring-before(.,'T')"/>
    </pubdate>-->
    </xsl:template>
    
    <xsl:template match="meta:creation-date"/>
    
    <xsl:template match="office:body">
        <xsl:apply-templates select="key(&apos;headchildren&apos;, generate-id())"/>
        <xsl:apply-templates select="text:h[@text:level=&apos;1&apos;]"/>
        <!--<xsl:apply-templates/>-->
    </xsl:template>

    <xsl:template match="office:styles">
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="office:script"/>

    <xsl:template match="office:settings"/>

    <xsl:template match="office:font-decls"/>
    
    <xsl:template match="text:bookmark-start">
	<xsl:text disable-output-escaping="yes">&lt;</xsl:text><xsl:value-of select="substring-before(@text:name,'_')"/><xsl:text disable-output-escaping="yes">&gt;</xsl:text><xsl:apply-templates/>
    </xsl:template>


    <xsl:template match="text:bookmark-end">
	<xsl:text disable-output-escaping="yes">&lt;/</xsl:text><xsl:value-of select="substring-before(@text:name,'_')"/><xsl:text disable-output-escaping="yes">&gt;</xsl:text><xsl:apply-templates/>
    </xsl:template>


    <xsl:template match="text:p[@text:style-name='Document Title']">
	<xsl:element name="title">
		<xsl:apply-templates />
	</xsl:element>
     </xsl:template>

<xsl:template match="text:p[@text:style-name='Document SubTitle']">
</xsl:template>

    <xsl:template match="text:p[@text:style-name=&apos;Document SubTitle&apos;]"/>

    <xsl:template match="text:p[@text:style-name=&apos;Section Title&apos;]">
        <title>
            <xsl:apply-templates/>
        </title>
    </xsl:template>

    <xsl:template match="text:p[@text:style-name=&apos;Appendix Title&apos;]">
        <title>
            <xsl:apply-templates/>
        </title>
    </xsl:template>


    <xsl:template match="text:p[@text:style-name=&apos;VarList Item&apos;]">
        <xsl:if test="not(preceding-sibling::text:p[@text:style-name=&apos;VarList Item&apos;])">
            <xsl:text disable-output-escaping="yes">&lt;listitem&gt;</xsl:text>
        </xsl:if>
        <para>
            <xsl:apply-templates/>
        </para>
        <xsl:if test="not(following-sibling::text:p[@text:style-name=&apos;VarList Item&apos;])">
            <xsl:text disable-output-escaping="yes">&lt;/listitem&gt;</xsl:text>
        </xsl:if>
    </xsl:template>


    <xsl:template match="text:p[@text:style-name=&apos;VarList Term&apos;]">
        <term>
            <xsl:apply-templates/>
        </term>
    </xsl:template>


    <xsl:template match="text:p[@text:style-name=&apos;Section1 Title&apos;]">
        <title>
            <xsl:apply-templates/>
        </title>
    </xsl:template>


    <xsl:template match="text:p[@text:style-name=&apos;Section2 Title&apos;]">
        <title>
            <xsl:apply-templates/>
        </title>
    </xsl:template>


    <xsl:template match="text:p[@text:style-name=&apos;Section3 Title&apos;]">
        <title>
            <xsl:apply-templates/>
        </title>
    </xsl:template>


    <xsl:template match="text:footnote-citation"/>


    <xsl:template match="text:p[@text:style-name=&apos;Mediaobject&apos;]">
        <mediaobject>
            <xsl:apply-templates/>
        </mediaobject>
    </xsl:template>


    <xsl:template match="office:annotation/text:p">
        <note>
            <remark>
                <xsl:apply-templates/>
            </remark>
        </note>
    </xsl:template>
    <!--<xsl:template match="meta:initial-creator">
    <author>
    <xsl:apply-templates />
        </author>
    </xsl:template>-->

    <xsl:template match="table:table">
        <xsl:choose>
            <xsl:when test="following-sibling::text:p[@text:style-name=&apos;Table&apos;]">
                <table frame="all">
                    <xsl:attribute name="id">
                        <xsl:value-of select="@table:name"/>
                    </xsl:attribute>
                    <title>
                        <xsl:value-of select="following-sibling::text:p[@text:style-name=&apos;Table&apos;]"/>
                    </title>
                    <xsl:call-template name="generictable"/>
                </table>
            </xsl:when>
            <xsl:otherwise>
                <informaltable frame="all">
                    <xsl:call-template name="generictable"/>
                </informaltable>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    
    
    <xsl:template name="generictable">
        <xsl:variable name="cells" select="count(descendant::table:table-cell)"/>
        <xsl:variable name="rows">
            <xsl:value-of select="count(descendant::table:table-row) "/>
        </xsl:variable>
        <xsl:variable name="cols">
            <xsl:value-of select="$cells div $rows"/>
        </xsl:variable>
        <xsl:variable name="numcols">
            <xsl:choose>
                <xsl:when test="child::table:table-column/@table:number-columns-repeated">
                    <xsl:value-of select="number(table:table-column/@table:number-columns-repeated+1)"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$cols"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:element name="tgroup">
            <xsl:attribute name="cols">
                <xsl:value-of select="$numcols"/>
            </xsl:attribute>
            <xsl:call-template name="colspec">
                <xsl:with-param name="left" select="1"/>
            </xsl:call-template>
            <xsl:apply-templates/>
        </xsl:element>
    </xsl:template>
    
    
    <xsl:template name="colspec">
        <xsl:param name="left"/>
        <xsl:if test="number($left &lt; ( table:table-column/@table:number-columns-repeated +2)  )">
            <xsl:element name="colspec">
                <xsl:attribute name="colnum">
                    <xsl:value-of select="$left"/>
                </xsl:attribute>
                <xsl:attribute name="colname">c
                    <xsl:value-of select="$left"/>
                </xsl:attribute>
            </xsl:element>
            <xsl:call-template name="colspec">
                <xsl:with-param name="left" select="$left+1"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>
    
    
    <xsl:template match="table:table-column">
        <xsl:apply-templates/>
    </xsl:template>
    
    
    <xsl:template match="table:table-header-rows">
        <thead>
            <xsl:apply-templates/>
        </thead>
    </xsl:template>
    
    
    <xsl:template match="table:table-header-rows/table:table-row">
        <row>
            <xsl:apply-templates/>
        </row>
    </xsl:template>
    
    
    <xsl:template match="table:table/table:table-row">
        <xsl:if test="not(preceding-sibling::table:table-row)">
            <xsl:text disable-output-escaping="yes">&lt;tbody&gt;</xsl:text>
        </xsl:if>
        <row>
            <xsl:apply-templates/>
        </row>
        <xsl:if test="not(following-sibling::table:table-row)">
            <xsl:text disable-output-escaping="yes">&lt;/tbody&gt;</xsl:text>
        </xsl:if>
    </xsl:template>
    
    
    <xsl:template match="table:table-cell">
        <xsl:element name="entry">
            <xsl:if test="@table:number-columns-spanned &gt;&apos;1&apos;">
                <xsl:attribute name="namest">
                    <xsl:value-of select="concat(&apos;c&apos;,count(preceding-sibling::table:table-cell[not(@table:number-columns-spanned)]) +sum(preceding-sibling::table:table-cell/@table:number-columns-spanned)+1)"/>
                </xsl:attribute>
                <xsl:attribute name="nameend">
                    <xsl:value-of select="concat(&apos;c&apos;,count(preceding-sibling::table:table-cell[not(@table:number-columns-spanned)]) +sum(preceding-sibling::table:table-cell/@table:number-columns-spanned)+ @table:number-columns-spanned)"/>
                </xsl:attribute>
            </xsl:if>
            <xsl:apply-templates/>
        </xsl:element>
    </xsl:template>
    
    
    <xsl:template match="text:p">
        <xsl:choose>
            <xsl:when test="@text:style-name=&apos;Table&apos;"/>
            <xsl:otherwise>
                <xsl:if test="not( child::text:span[@text:style-name = &apos;XrefLabel&apos;] )">
                    <para>
                        <xsl:call-template name="id.attribute"/>
                        <xsl:apply-templates/>
                    </para>
                </xsl:if>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    
    
    <xsl:template match="text:unordered-list">
        <xsl:choose>
            <xsl:when test="@text:style-name=&apos;Var List&apos;">
                <variablelist>
                    <xsl:apply-templates/>
                </variablelist>
            </xsl:when>
            <xsl:when test="@text:style-name=&apos;UnOrdered List&apos;">
                <variablelist>
                    <xsl:apply-templates/>
                </variablelist>
            </xsl:when>
            <xsl:otherwise>
                <itemizedlist>
                    <title/>
                    <xsl:apply-templates/>
                </itemizedlist>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    
    
    <xsl:template match="text:list-item">
        <xsl:if test="parent::text:unordered-list/@text:style-name=&apos;Var List&apos;">
            <varlistentry>
                <xsl:for-each select="text:p[@text:style-name=&apos;VarList Term&apos;]">
                    <xsl:apply-templates select="."/>
                </xsl:for-each>
            </varlistentry>
        </xsl:if>
        <xsl:if test="not(parent::text:unordered-list/@text:style-name=&apos;Var List&apos;)">
            <listitem>
                <xsl:apply-templates/>
            </listitem>
        </xsl:if>
    </xsl:template>
    
    
    <xsl:template match="dc:title"/>
    
    
    <xsl:template match="dc:description">
        <abstract>
            <para>
                <xsl:apply-templates/>
            </para>
        </abstract>
    </xsl:template>
    
    
    <xsl:template match="dc:subject"/>
    
    <xsl:template match="meta:generator"/>
    
    <xsl:template match="draw:plugin">
        <xsl:element name="audioobject">
            <xsl:attribute name="fileref">
                <xsl:value-of select="@xlink:href"/>
            </xsl:attribute>
            <xsl:attribute name="width"/>
        </xsl:element>
    </xsl:template>
    
    
    <xsl:template match="text:footnote">
        <footnote>
            <xsl:apply-templates/>
        </footnote>
    </xsl:template>
    
    
    <xsl:template match="text:footnote-body">
        <xsl:apply-templates/>
    </xsl:template>
    
    <xsl:template match="draw:text-box"/>
    
    <xsl:template match="draw:image">
        <xsl:choose>
            <xsl:when test="parent::text:p[@text:style-name=&apos;Mediaobject&apos;]">
                <xsl:element name="imageobject">
                    <xsl:element name="imagedata">
                        <xsl:attribute name="fileref">
                            <xsl:value-of select="@xlink:href"/>
                        </xsl:attribute>
                    </xsl:element>
                    <xsl:element name="caption">
                        <xsl:value-of select="."/>
                    </xsl:element>
                </xsl:element>
            </xsl:when>
            <xsl:otherwise>
                <xsl:element name="inlinegraphic">
                    <xsl:attribute name="fileref">
                        <xsl:value-of select="@xlink:href"/>
                    </xsl:attribute>
                    <xsl:attribute name="width"/>
                </xsl:element>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    
    
    <xsl:template match="text:span">
        <xsl:choose>
            <xsl:when test="./@text:style-name=&apos;GuiMenu&apos;">
                <xsl:element name="guimenu">
			<xsl:apply-templates/>	
                </xsl:element>
            </xsl:when>
            <xsl:when test="./@text:style-name=&apos;GuiSubMenu&apos;">
                <xsl:element name="guisubmenu">
			<xsl:apply-templates/>	
                </xsl:element>
            </xsl:when>
            <xsl:when test="@text:style-name=&apos;GuiMenuItem&apos;">
                <xsl:element name="guimenuitem">
			<xsl:apply-templates/>	
                </xsl:element>
            </xsl:when>
            <xsl:when test="@text:style-name=&apos;GuiButton&apos;">
                <xsl:element name="guibutton">
			<xsl:apply-templates/>	
                </xsl:element>
            </xsl:when>
            <xsl:when test="@text:style-name=&apos;GuiButton&apos;">
                <xsl:element name="guibutton">
			<xsl:apply-templates/>	
                </xsl:element>
            </xsl:when>
            <xsl:when test="@text:style-name=&apos;GuiLabel&apos;">
                <xsl:element name="guilabel">
			<xsl:apply-templates/>	
                </xsl:element>
            </xsl:when>
            <xsl:when test="@text:style-name=&apos;Emphasis&apos;">
                <xsl:element name="emphasis">
			<xsl:apply-templates/>	
                </xsl:element>
            </xsl:when>
<!-- Change Made By Kevin Fowlks (fowlks@msu.edu) June 4th, 2003 -->
		<xsl:when test="@text:style-name='Emphasis Bold'">
			<xsl:element name="emphasis">
				<xsl:attribute name = "role" >
					<xsl:text>bold</xsl:text>
				</xsl:attribute>
				<xsl:apply-templates/>	
			</xsl:element>
		</xsl:when>
<!-- Change Made By Kevin Fowlks (fowlks@msu.edu) June 16th, 2003 -->
		<xsl:when test="@text:style-name=&apos;Citation&apos;">
			<xsl:element name="quote">
				<xsl:apply-templates/>	
			</xsl:element>
		</xsl:when>
            <xsl:when test="@text:style-name=&apos;FileName&apos;">
                <xsl:element name="filename">
			<xsl:apply-templates/>	
                </xsl:element>
            </xsl:when>
            <xsl:when test="@text:style-name=&apos;Application&apos;">
                <xsl:element name="application">
			<xsl:apply-templates/>	
                </xsl:element>
            </xsl:when>
            <xsl:when test="@text:style-name=&apos;Command&apos;">
                <command>
                    <xsl:apply-templates/>
                </command>
            </xsl:when>
            <xsl:when test="@text:style-name=&apos;SubScript&apos;">
                <subscript>
                    <xsl:apply-templates/>
                </subscript>
            </xsl:when>
            <xsl:when test="@text:style-name=&apos;SuperScript&apos;">
                <superscript>
                    <xsl:apply-templates/>
                </superscript>
            </xsl:when>
            <xsl:when test="@text:style-name=&apos;SystemItem&apos;">
                <systemitem>
                    <xsl:apply-templates/>
                </systemitem>
            </xsl:when>
            <xsl:when test="@text:style-name=&apos;ComputerOutput&apos;">
                <computeroutput>
                    <xsl:apply-templates/>
                </computeroutput>
            </xsl:when>
            <xsl:when test="@text:style-name=&apos;Highlight&apos;">
                <highlight>
                    <xsl:apply-templates/>
                </highlight>
            </xsl:when>
            <xsl:when test="@text:style-name=&apos;KeyCap&apos;">
                <keycap>
                    <xsl:apply-templates/>
                </keycap>
            </xsl:when>
            <xsl:when test="@text:style-name=&apos;KeySym&apos;">
                <xsl:element name="keysym">
                    <xsl:apply-templates/>
                </xsl:element>
            </xsl:when>
            <xsl:when test="@text:style-name=&apos;KeyCombo&apos;">
                <keycombo>
                    <xsl:apply-templates/>
                </keycombo>
            </xsl:when>
            <xsl:otherwise>
                <xsl:apply-templates/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    
    
    <xsl:template match="text:a">
        <xsl:choose>
            <xsl:when test="contains(@xlink:href,&apos;://&apos;)">
                <xsl:element name="ulink">
                    <xsl:attribute name="url">
                        <xsl:value-of select="@xlink:href"/>
                    </xsl:attribute>
                    <xsl:apply-templates/>
                </xsl:element>
            </xsl:when>
            <xsl:when test="not(contains(@xlink:href,&apos;#&apos;))">
                <xsl:element name="olink">
                    <xsl:attribute name="targetdocent">
                        <xsl:value-of select="@xlink:href"/>
                    </xsl:attribute>
                    <xsl:apply-templates/>
                </xsl:element>
            </xsl:when>
            <xsl:otherwise>
                <xsl:variable name="linkvar" select="substring-after(@xlink:href,&apos;#&apos;)"/>
                <xsl:element name="link">
                    <xsl:attribute name="linkend">
                        <xsl:value-of select="substring-before($linkvar,&apos;%&apos;)"/>
                    </xsl:attribute>
                    <xsl:apply-templates/>
                </xsl:element>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    
    
    <xsl:template match="text:line-break">
        <xsl:text disable-output-escaping="yes"/>
    </xsl:template>
    
    
    <xsl:template match="text:tab-stop">
        <xsl:text disable-output-escaping="yes"/>
    </xsl:template>
    <!-- @endterm not retained due to problems with OOo method of displaying text in the reference-ref -->

    <xsl:template match="text:reference-ref">
        <xsl:element name="xref">
            <xsl:attribute name="linkend">
                <xsl:value-of select="@text:ref-name"/>
            </xsl:attribute>
        </xsl:element>
    </xsl:template>
    
    
    <xsl:template name="id.attribute">
        <xsl:if test="child::text:reference-mark-start">
            <xsl:attribute name="id">
                <xsl:value-of select="child::text:reference-mark-start/@text:name"/>
            </xsl:attribute>
        </xsl:if>
        <!-- Constraints imposed by OOo method of displaying reference-ref text means that xreflabel and endterm are lost -->
        <!--<xsl:if test="child::text:p/text:span[@text:style-name = 'XrefLabel']">
        <xsl:attribute name="xreflabel">
            <xsl:value-of select="text:p/text:span"/>
        </xsl:attribute>
    </xsl:if> -->

    </xsl:template>
    <!--    
        <text:h text:level="1">Part One Title
            <text:reference-mark-start text:name="part"/>
            <text:p text:style-name="Text body">
                <text:span text:style-name="XrefLabel">xreflabel_part</text:span>
            </text:p>
            <text:reference-mark-end text:name="part"/>
        </text:h>
-->
    <!--<xsl:template match="text:p/text:span[@text:style-name = 'XrefLabel']"/>-->

    <xsl:template match="text:reference-mark-start"/>
    
    <xsl:template match="text:reference-mark-end"/>
    
    <xsl:template match="comment">
        <xsl:comment>
            <xsl:value-of select="."/>
        </xsl:comment>
    </xsl:template>
    
    
    <xsl:template match="text:alphabetical-index-mark-start">
        <xsl:element name="indexterm">
            <xsl:attribute name="class">
                <xsl:text disable-output-escaping="yes">startofrange</xsl:text>
            </xsl:attribute>
            <xsl:attribute name="id">
                <xsl:value-of select="@text:id"/>
            </xsl:attribute>
            <!--<xsl:if test="@text:key1">-->

            <xsl:element name="primary">
                <xsl:value-of select="@text:key1"/>
            </xsl:element>
            <!--</xsl:if>-->

            <xsl:if test="@text:key2">
                <xsl:element name="secondary">
                    <xsl:value-of select="@text:key2"/>
                </xsl:element>
            </xsl:if>
        </xsl:element>
    </xsl:template>
    
    
    <xsl:template match="text:alphabetical-index-mark-end">
        <xsl:element name="indexterm">
            <xsl:attribute name="startref">
                <xsl:value-of select="@text:id"/>
            </xsl:attribute>
            <xsl:attribute name="class">
                <xsl:text disable-output-escaping="yes">endofrange</xsl:text>
            </xsl:attribute>
        </xsl:element>
    </xsl:template>
    
    
    <xsl:template match="text:alphabetical-index">
        <xsl:element name="index">
            <xsl:element name="title">
                <xsl:value-of select="text:index-body/text:index-title/text:p"/>
            </xsl:element>
            <xsl:apply-templates select="text:index-body"/>
        </xsl:element>
    </xsl:template>
    
    
    <xsl:template match="text:index-body">
        <xsl:for-each select="text:p[@text:style-name = &apos;Index 1&apos;]">
            <xsl:element name="indexentry">
                <xsl:element name="primaryie">
                    <xsl:value-of select="."/>
                </xsl:element>
                <xsl:if test="key(&apos;secondary_children&apos;, generate-id())">
                    <xsl:element name="secondaryie">
                        <xsl:value-of select="key(&apos;secondary_children&apos;, generate-id())"/>
                    </xsl:element>
                </xsl:if>
            </xsl:element>
        </xsl:for-each>
    </xsl:template>
    
</xsl:stylesheet>

