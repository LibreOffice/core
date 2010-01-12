<?xml version="1.0" encoding="UTF-8"?>

<!-- 
	odt2wiki: OpenDocument to WikiMedia transformation
    Copyright (C) 2007  Bernhard Haumacher (haui at haumacher dot de)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.	
-->
<stylesheet version="1.0" 
	xmlns="http://www.w3.org/1999/XSL/Transform"

	xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" 
	xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" 
	xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" 
	xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" 
	xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" 
	xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" 
	xmlns:meta="urn:oasis:names:tc:opendocument:xmlns:meta:1.0" 
	xmlns:number="urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0" 
	xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" 
	xmlns:chart="urn:oasis:names:tc:opendocument:xmlns:chart:1.0" 
	xmlns:dr3d="urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0" 
	xmlns:form="urn:oasis:names:tc:opendocument:xmlns:form:1.0" 
	xmlns:script="urn:oasis:names:tc:opendocument:xmlns:script:1.0" 
	xmlns:xlink="http://www.w3.org/1999/xlink" 
	xmlns:dc="http://purl.org/dc/elements/1.1/" 
	xmlns:math="http://www.w3.org/1998/Math/MathML" 
	xmlns:dom="http://www.w3.org/2001/xml-events" 
	xmlns:xforms="http://www.w3.org/2002/xforms" 
	xmlns:xsd="http://www.w3.org/2001/XMLSchema" 
	xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
>
	
	<!-- 
		== Customization options ==
	-->

	<!-- Constant defining the newline token. -->
	<param name="NL" select="'&#10;'"/>

	<!-- String that a tabulator is expanded with in preformatted paragraphs. -->

		<variable name="codetabdocument-value"
			select="/office:document/office:meta/meta:user-defined[@meta:name='CODE_TAB_REPLACEMENT']"/>

	<param name="CODE_TAB_REPLACEMENT">

		<choose>

			<when test="boolean($codetabdocument-value)">
				<value-of select="$codetabdocument-value"/>
			</when>
			
			<otherwise>
				<value-of select="'    '"/>
			</otherwise>
		</choose>
	</param>
	
	<!-- 
		Switch that suppresses separation of paragraphs with empty lines. 
		(Set to 1 to activate) -->
	<param name="CODE_JOIN_PARAGRAPHS" 
		select="boolean(string(/office:document/office:meta/meta:user-defined[@meta:name='CODE_JOIN_PARAGRAPHS']) != 'false')"/>
	
		<variable name="document-value"
			select="/office:document/office:meta/meta:user-defined[@meta:name='CODE_STYLES']"/>

	<param name="CODE_STYLES">

		<choose>
			<when test="boolean($document-value)">
				<value-of select="$document-value"/>
			</when>
			
			<otherwise>
				<value-of select="''"/>
			</otherwise>
		</choose>
	</param>

		<variable name="table-class"
			select="/office:document/office:meta/meta:user-defined[@meta:name='TABLE_CLASS']"/>

	<param name="TABLE_CLASS">
		<choose>
			<when test="boolean($table-class)">
				<value-of select="$table-class"/>
			</when>
			
			<otherwise>
				<value-of select="''"/>
			</otherwise>
		</choose>
	</param>
	
	<variable name="USE_DEFAULT_TABLE_CLASS" select="string-length($TABLE_CLASS) &gt; 0"/>


	<!-- 
		== Wiki style constants == 
	-->
	
	<!-- Bold character style. -->
	<variable name="BOLD_BIT" select="1"/>

	<!-- Italic character style. -->
	<variable name="ITALIC_BIT" select="2"/>

	<!-- Subscript character style. -->
	<variable name="SUBSCRIPT_BIT" select="4"/>

	<!-- Superscript character style. -->
	<variable name="SUPERSCRIPT_BIT" select="8"/>

	<!-- Typewriter character style. -->
	<variable name="TYPEWRITER_BIT" select="16"/>
	
	<!-- Preformatted text paragraph style. -->
	<variable name="CODE_BIT" select="32"/>

	<!-- Centered paragraph style. -->
	<variable name="CENTER_BIT" select="64"/>

	<!-- Right aligned paragraph style. -->
	<variable name="RIGHT_BIT" select="128"/>
	
	<!-- Constant defining the empty style. -->
	<variable name="NO_STYLE" select="0"/>



	<output 
		method="text" 
		media-type="text/plain" 
		encoding="UTF-8"
	/>


	<!-- 
		== Reference resolution == 
	-->

	<key
		name="style-ref"
		match="//style:style"
		use="@style:name"
	/>

	<key
		name="list-style-ref"
		match="//text:list-style"
		use="@style:name"
	/>
	
	<key
		name="font-face-ref"
		match="//style:font-face"
		use="@style:name"
	/>
	
	<key
		name="reference-resolution"
		match="//text:reference-mark | //text:reference-mark-start"
		use="@text:name"
	/>


	<!-- 
		Multiple pages (draw only)
	-->

	<template match="draw:page">
		<value-of select="concat('&#10;&lt;!-- Page ', @draw:name, '--&gt;&#10;')"/>
		<apply-templates/>
		<value-of select="'&#10;----&#10;&#10;'"/>
	</template>


	<!-- 
		== Lists == 
	-->

	<template match="text:list">
		<!-- 
			Check, whether this list is used to implement the outline numbering 
			for headings. Such list must not be exported, because within the wiki, 
			automatic outline numbering is performed. An outline list has a single 
			text:h element as its single leaf grandchild. 
			
			This method of section numbering seems not to be used when creating new
			documents with OpenOffice.org 2.2, but the document containing the 
			OpenDocument specification version 1.1 uses such numbering through nested 
			lists.
			-->
		<choose>
			<when test="boolean(./descendant::node()[not(./self::text:list) and not(./self::text:list-item) and not(./ancestor-or-self::text:h)])">
				<apply-templates/>
			</when>
			
			<otherwise>
				<apply-templates select=".//text:h"/>
			</otherwise>
		</choose>
	</template>
	
	<template match="text:list-item">
		<if test="position() &gt; 1 or boolean(ancestor::text:list-item)">
			<value-of select="$NL"/>
		</if>
		<variable name="list-style" 
			select="key('list-style-ref',ancestor::text:list[boolean(@text:style-name)][1]/@text:style-name)"/>
		<call-template name="mk-list-token">
			<with-param name="list-style" select="$list-style"/>
			<with-param name="level" select="count(ancestor::text:list)"/>
		</call-template>
		<text> </text>
		<apply-templates/>
		<if test="position() = last() and not(boolean(ancestor::text:list-item))">
			<!-- End of (potentially nested) list is marked with a double newline. -->
			<value-of select="$NL"/>
			<value-of select="$NL"/>
		</if>
	</template>
	
	<template name="mk-list-token">
		<param name="list-style"/>
		<param name="level"/>
		
		<if test="$level &gt; 1">
			<call-template name="mk-list-token">
				<with-param name="list-style" select="$list-style"/>
				<with-param name="level" select="$level - 1"/>
			</call-template>
		</if>

		<variable name="number-style" select="$list-style/text:list-level-style-number[@text:level=$level]"/>
		<variable name="bullet-style" select="$list-style/text:list-level-style-bullet[@text:level=$level]"/>
		<choose>
			<when test="boolean($number-style)">
				<choose>
					<when test="string-length($number-style/@style:num-format) &gt; 0">
						<text>#</text>
					</when>
					<otherwise>
						<text>:</text>
					</otherwise>
				</choose>
			</when>
			<when test="boolean($bullet-style)">
				<text>*</text>
			</when>
		</choose>
	</template>


	<!-- 
		== Headings ==
	-->

	<template match="text:h">
		<if test="string-length(.) &gt; 0">
			<variable name="token">
				<call-template name="mk-heading-prefix">
					<with-param name="level" select="@text:outline-level"/>
				</call-template>
			</variable>
			<value-of select="$token"/>
			<text> </text>
			<apply-templates/>
			<text> </text>
			<value-of select="$token"/>
			<value-of select="$NL"/>
		</if>
	</template>

	<template match="text:index-title">
		<text>== </text>
		<apply-templates/>
		<text> ==</text>
		<value-of select="$NL"/>
	</template>

	<!-- 
		Function generating a wiki heading prefix.
		
		@param level
			The heading level. The value must be between 1 and 5.
	-->
	<template name="mk-heading-prefix">
		<param name="level"/>
		<choose>
			<when test="$level &gt; 5">
				<call-template name="mk-heading-prefix">
					<with-param name="level" select="5"/>
				</call-template>
			</when>
			<when test="$level &gt; 0">
				<text>=</text>
				<call-template name="mk-heading-prefix">
					<with-param name="level" select="$level - 1"/>
				</call-template>
			</when>
		</choose>
	</template>

	<!-- 
		Funktion generating a token consisting of the given character 
		repeated 'level' times.
		
		@param level
			The lengh of the result.
		@param char
			The character that should be repeated 'level' times.
	-->
	<template name="mk-token">
		<param name="level"/>
		<param name="char"/>
		<if test="$level &gt; 0">
			<value-of select="$char"/>
			<call-template name="mk-token">
				<with-param name="level" select="$level - 1"/>
				<with-param name="char" select="$char"/>
			</call-template>
		</if>
	</template>
	
	
	<!-- 
		== Tables ==
	 -->

	<template match="table:table">
		<text>&#10;</text>
		<text>{|</text>
		
		<choose>
			<when test="$USE_DEFAULT_TABLE_CLASS">
				<text> class="</text>
				<value-of select="$TABLE_CLASS"/>
				<text>"</text>
			</when>
			
			<otherwise>
				<!-- Default setting to translate detailed office table cell styles correctly. -->
				<text> style="border-spacing:0;"</text>
			</otherwise>
		</choose>
		
		<text>&#10;</text>
		<apply-templates/>
		<text>&#10;</text>
		<text>|}</text>
		<text>&#10;</text>
	</template>

	<template match="table:table-header-rows">
		<apply-templates/>
	</template>

	<template match="table:table-row[position() &lt; last()] | table:table-header-rows/table:table-row">
		<apply-templates/>
		<text>&#10;</text>
		<text>|-</text>
		<text>&#10;</text>
	</template>

	<template match="table:table-row">
		<apply-templates/>
	</template>

	<template match="table:table-header-rows//table:table-cell">
		<text>! </text>
		<if test="@table:number-columns-spanned">
			<text>colspan="</text>
			<value-of select="@table:number-columns-spanned"/>
			<text>" | </text>
		</if>
		<apply-templates/>
		<value-of select="$NL"/>
	</template>

	<template match="table:table-cell">
		<text>|</text>
		<if test="@table:number-columns-spanned">
			<text> colspan="</text>
			<value-of select="@table:number-columns-spanned"/>
			<text>" </text>
		</if>
		<if test="not($USE_DEFAULT_TABLE_CLASS) and boolean(@table:style-name)">
	 		<variable name="style-element" select="key('style-ref', @table:style-name)"/>
			
			<variable name="style">
				<call-template name="translate-style-property">
					<with-param name="style-name" select="'background-color'"/>
					<with-param name="style-property" select="$style-element/style:table-cell-properties/@fo:background-color"/>
				</call-template>
				
				<call-template name="translate-style-property">
					<with-param name="style-name" select="'border'"/>
					<with-param name="style-property" select="$style-element/style:table-cell-properties/@fo:border"/>
				</call-template>
				<call-template name="translate-style-property">
					<with-param name="style-name" select="'border-top'"/>
					<with-param name="style-property" select="$style-element/style:table-cell-properties/@fo:border-top"/>
				</call-template>
				<call-template name="translate-style-property">
					<with-param name="style-name" select="'border-bottom'"/>
					<with-param name="style-property" select="$style-element/style:table-cell-properties/@fo:border-bottom"/>
				</call-template>
				<call-template name="translate-style-property">
					<with-param name="style-name" select="'border-left'"/>
					<with-param name="style-property" select="$style-element/style:table-cell-properties/@fo:border-left"/>
				</call-template>
				<call-template name="translate-style-property">
					<with-param name="style-name" select="'border-right'"/>
					<with-param name="style-property" select="$style-element/style:table-cell-properties/@fo:border-right"/>
				</call-template>
				
				<call-template name="translate-style-property">
					<with-param name="style-name" select="'padding'"/>
					<with-param name="style-property" select="$style-element/style:table-cell-properties/@fo:padding"/>
				</call-template>
				<call-template name="translate-style-property">
					<with-param name="style-name" select="'padding-top'"/>
					<with-param name="style-property" select="$style-element/style:table-cell-properties/@fo:padding-top"/>
				</call-template>
				<call-template name="translate-style-property">
					<with-param name="style-name" select="'padding-bottom'"/>
					<with-param name="style-property" select="$style-element/style:table-cell-properties/@fo:padding-bottom"/>
				</call-template>
				<call-template name="translate-style-property">
					<with-param name="style-name" select="'padding-left'"/>
					<with-param name="style-property" select="$style-element/style:table-cell-properties/@fo:padding-left"/>
				</call-template>
				<call-template name="translate-style-property">
					<with-param name="style-name" select="'padding-right'"/>
					<with-param name="style-property" select="$style-element/style:table-cell-properties/@fo:padding-right"/>
				</call-template>
			</variable>
			
			<if test="string-length($style) &gt; 0">
				<text> style="</text>
				<value-of select="$style"/>
				<text>"</text>
			</if>
		</if>
		<text>| </text>
		<apply-templates/>
		<value-of select="$NL"/>
	</template>

	<template name="translate-style-property">
		<param name="style-name"/>
		<param name="style-property"/>
		
		<if test="boolean($style-property)">
			<value-of select="$style-name"/>
			<text>:</text>
			<value-of select="string($style-property)"/>
			<text>;</text>
		</if>
	</template>

	<!-- 
		== WikiMath == 
	 -->

	<template match="text:span[@text:style-name='WikiMath']">
		<value-of select="'&lt;math&gt;'"/>
		<apply-templates/>
		<value-of select="'&lt;/math&gt;'"/>
	</template>


	<!-- 
		== Native links == 
	 -->

	<template match="text:a">
		<variable name="link-ref" select="@xlink:href"/>
		<choose>
			<when test="string-length($link-ref) &gt; 0">
				<variable name="link-label" select="string(.)"/>
				<text>[</text>
				<value-of select="$link-ref"/>
				<text> </text>
				<value-of select="$link-label"/>
				<text>]</text>
			</when>
			
			<otherwise>
				<apply-templates/>
			</otherwise>
		</choose>
	</template>


	<!-- 
		== WikiLink == 
	 -->

	<template match="text:span[@text:style-name='WikiLink']">
		<value-of select="'[['"/>
		<variable name="link-def" select="string(.)"/>
		<variable name="link-label" select="normalize-space(substring-before($link-def, '['))"/>
		<variable name="link-ref" select="normalize-space(substring-before(substring-after($link-def, '['), ']'))"/>
		<choose>
			<when test="boolean($link-ref)">
			<value-of select="concat($link-ref, '|', $link-label)"/>
		</when>
		<otherwise>
			<value-of select="$link-def"/>
		</otherwise>
		</choose>
		<value-of select="']]'"/>
	</template>
	
	
	<!-- 
		== Paragraphs == 
	 -->

	<template match="text:p[string-length(.) &gt; 0]">
		<variable name="style">
			<call-template name="mk-style-set">
				<with-param name="node" select="."/>
			</call-template>
		</variable>

		<variable name="code" 
			select="($style mod (2 * $CODE_BIT)) - ($style mod ($CODE_BIT)) != 0"/>
		<variable name="center" 
			select="($style mod (2 * $CENTER_BIT)) - ($style mod ($CENTER_BIT)) != 0"/>
		<variable name="right" 
			select="($style mod (2 * $RIGHT_BIT)) - ($style mod ($RIGHT_BIT)) != 0"/>
		
		<choose>
			<when test="$center">
				<text>&lt;center&gt;</text>
			</when>
			<when test="$right">
				<text>&lt;div align="right"&gt;</text>
			</when>
			<when test="$code">
				<value-of select="' '"/>
			</when>
		</choose>
	
		<apply-templates/>

		<choose>
			<when test="$center">
				<text>&lt;/center&gt;</text>
			</when>
			<when test="$right">
				<text>&lt;/div&gt;</text>
			</when>
		</choose>

		<variable name="paragraph-right" 
 			select="./following-sibling::*[1]/self::text:p"/>

 		<choose>
 		<when test="boolean($paragraph-right)">
			<!-- 
				Insert end of line only if not within a list. Within wiki lists, 
				a line break leaves the current list item. 
			-->
			<choose>
				<when test="boolean(ancestor::text:list-item)">
					<text>&lt;br/&gt; </text>
				</when>
				<when test="$code">
					<variable name="style-right">
						<call-template name="mk-style-set">
							<with-param name="node" select="$paragraph-right"/>
						</call-template>
					</variable>
			
					<variable name="code-right" 
						select="($style-right mod (2 * $CODE_BIT)) - ($style-right mod ($CODE_BIT)) != 0"/>
				
					<choose>
						<when test="$code-right">
							<choose>
								<when test="$CODE_JOIN_PARAGRAPHS">
									<value-of select="$NL"/>
								</when>
								
								<otherwise>
									<value-of select="$NL"/>
									<value-of select="' '"/>
									<value-of select="$NL"/>
								</otherwise>
							</choose>
						</when>
						<otherwise>
							<value-of select="$NL"/>
							<value-of select="$NL"/>
						</otherwise>
					</choose>
				</when>
				<otherwise>
					<value-of select="$NL"/>
					<value-of select="$NL"/>
				</otherwise>
			</choose>
 		</when>
 		<when test="boolean(./following-sibling::*[1]/self::text:h) or boolean(./following-sibling::*[1]/self::table:table) or boolean(./following-sibling::*[1]/self::text:bibliography)">
 			<!-- Newline before following heading or table. -->
 			<value-of select="$NL"/>
 			<value-of select="$NL"/>
 		</when>
 		<when test="./following-sibling::*[1]/self::text:list and not(ancestor::text:list-item)">
 			<value-of select="$NL"/>
 			<value-of select="$NL"/>
 		</when>
 		</choose>
	</template>

 	<template match="text:p[string-length(.) = 0 and string-length(preceding-sibling::*[1]/self::text:p) &gt; 0]">
		<value-of select="$NL"/>
	</template>


	<!-- 
		== Preformatted text == 
	-->
	
	<template match="text:s">
		<variable name="style">
			<call-template name="mk-style-set">
				<with-param name="node" select="."/>
			</call-template>
		</variable>

		<variable name="code" 
			select="($style mod (2 * $CODE_BIT)) - ($style mod ($CODE_BIT)) != 0"/>
		
		<if test="$code">
			<choose>
				<when test="@text:c">
					<call-template name="mk-token">
						<with-param name="level" select="@text:c"/>
						<with-param name="char" select="' '"/>
					</call-template>
				</when>
				<otherwise>
					<value-of select="' '"/>
				</otherwise>
			</choose>
		</if>
	</template>
	
	<template match="text:tab">
		<variable name="style">
			<call-template name="mk-style-set">
				<with-param name="node" select="."/>
			</call-template>
		</variable>

		<variable name="code" 
			select="($style mod (2 * $CODE_BIT)) - ($style mod ($CODE_BIT)) != 0"/>
		
		<if test="$code">
			<value-of select="$CODE_TAB_REPLACEMENT"/>
		</if>
	</template>
	
	<template match="text:line-break">
		<variable name="style">
			<call-template name="mk-style-set">
				<with-param name="node" select="."/>
			</call-template>
		</variable>

		<variable name="code" 
			select="($style mod (2 * $CODE_BIT)) - ($style mod ($CODE_BIT)) != 0"/>
		
		<if test="$code">
			<value-of select="$NL"/>
			<value-of select="' '"/>
		</if>
	</template>

	<!-- 
		Footnotes
	-->
	
	<template match="text:note-body">
		<variable name="note" select="./parent::text:note"/>
		
		<if test="$note/@text:note-class = 'footnote'">
			<text>&lt;ref name="</text>
			<value-of select="$note/@text:id"/>
			<text>"&gt;</text>
			<apply-templates/>
			<text>&lt;/ref&gt;</text>
		</if>
	</template>
	
	<template match="text:note-ref[@text:note-class='footnote']">
		<text>&lt;ref name="</text>
		<value-of select="@text:ref-name"/>
		<text>"/&gt;</text>
	</template>
	
	 
	<!-- 
		== Images == 
	-->
	
 	<template match="draw:text-box[boolean(.//draw:image)]">
 		<variable name="image" select=".//draw:image[1]"/>
 		
 		<variable name="image-description">
 			<apply-templates/>
 		</variable>
 		
 		<text>[[</text>
		<call-template name="mk-image-name">
			<with-param name="image" select="$image"/>
		</call-template>
 		<text>|thumb|</text>
		<value-of select="normalize-space($image-description)"/>
 		<text>]]</text>
 	</template>
 	
 	<template match="draw:image[not(boolean(ancestor::draw:text-box))]">
 		<text>[[</text>
		<call-template name="mk-image-name">
			<with-param name="image" select="."/>
		</call-template>
 		<text>]]</text>
 	</template>

	<template name="mk-image-name">
		<param name="image"/>
		
 		<variable name="base-name">
 			<call-template name="mk-base-name">
 				<with-param name="href" select="$image/@xlink:href"/>
 			</call-template>
 		</variable>
 		
		<if test="not(starts-with($base-name, 'Image:'))">
			<value-of select="'Image:'"/>
		</if>
		<value-of select="$base-name"/>
	</template>	
 	
 	<template name="mk-base-name">
 		<param name="href"/>
 		
 		<variable name="result" select="substring-after($href, '/')"/>
 		<choose>
 			<when test="boolean($result)">
 				<call-template name="mk-base-name">
 					<with-param name="href" select="$result"/>
 				</call-template>
 			</when>
 			<otherwise>
 				<value-of select="$href"/>
 			</otherwise>
 		</choose>
 	</template>
 	
  	<!-- Frames -->
 	
 	<template match="draw:frame">
 		<choose>
 			<when test="draw:object/math:math">
 				<apply-templates select="draw:object/math:math[1]"/>
 			</when>

 			<when test="draw:image">
 				<apply-templates select="draw:image[1]"/>
 			</when>
 			
 			<otherwise>
 				<apply-templates select="./*[1]"/>
 			</otherwise>
 		</choose>
 	
 	</template>
 	
 	<!-- Formulas (Objects) -->
 	
	<include href="math/mmltex.xsl"/>
	
	<template match="math:math" priority="1">
		<text>&lt;math&gt;</text>
		<apply-templates/>
		<text>&lt;/math&gt;</text>
	</template>
	
	
 	<!-- 
 		References
 	 -->
 	 
	<!-- TODO: text:bibliography-mark -->

 	<template match="text:reference-ref">
 		<variable name="reference-mark" select="key('reference-resolution', @text:ref-name)"/>
 		
 		<choose>
 			<when test="boolean($reference-mark)">
		 		<!-- 
		 			In wiki syntax, only a local reference to a heading can be inserted. 
		 			If the link target is a descendant of a heading element, a link can be
		 			inserted in the result. -->
		 		<variable name="header-anchor" select="$reference-mark/ancestor::text:h[1]"/>
		 		<if test="boolean($header-anchor)">
		 			<text>[[#</text>
		 			<value-of select="string($header-anchor)"/>
		 			<text>|</text>
		 		</if>
		 		
		 		<variable name="reference-text" select="string(.)"/>
		 		
		 		<choose>
		 			<!-- Check, whether the reference text is cached in the document. -->
		 			<when test="string-length($reference-text) &gt; 0">
		 				<value-of select="$reference-text"/>
		 			</when>
		 			
		 			<otherwise>
		 				<!-- 
		 					TODO: Evaluate the @text:reference-format attribute and 
		 					generate the replacement text (difficult).-->
		 				<text>(REFERENCE TEXT UNAVAILABLE: "</text>
		 				<value-of select="@text:ref-name"/>
		 				<text>")</text>
		 			</otherwise>
		 		</choose>
		 		
		 		<if test="boolean($header-anchor)">
		 			<text>]]</text>
		 		</if>
 			</when>

 			<otherwise>
 				<text>(UNDEFINED REFERENCE: "</text>
 				<value-of select="@text:ref-name"/>
 				<text>")</text>
 			</otherwise>
 		</choose>
 	</template>
 	
 	<template match="text:reference-mark">
 		<!-- TODO: Output an anchor. -->
 		<apply-templates/>
 	</template>

 	<template match="text:reference-mark-start">
 		<!-- TODO: Output an anchor. -->
 	</template>

	<!-- 
		== Plain text == 
	-->

	<template match="text:p/text() | text:h/text() | text:span/text() | text:sequence/text() | text:sequence-ref/text() | text:a/text() | text:bookmark-ref/text() | text:reference-mark/text() | text:date/text() | text:time/text() | text:page-number/text() | text:sender-firstname/text() | text:sender-lastname/text() | text:sender-initials/text() | text:sender-title/text() | text:sender-position/text() | text:sender-email/text() | text:sender-phone-private/text() | text:sender-fax/text() | text:sender-company/text() | text:sender-phone-work/text() | text:sender-street/text() | text:sender-city/text() | text:sender-postal-code/text() | text:sender-country/text() | text:sender-state-or-province/text() | text:author-name/text() | text:author-initials/text() | text:chapter/text() | text:file-name/text() | text:template-name/text() | text:sheet-name/text() | text:variable-get/text() | text:variable-input/text() | text:user-field-get/text() | text:user-field-input/text() | text:expression/text() | text:text-input/text() | text:initial-creator/text() | text:creation-date/text() | text:creation-time/text() | text:description/text() | text:user-defined/text() | text:print-date/text() | text:printed-by/text() | text:title/text() | text:subject/text() | text:keywords/text() | text:editing-cycles/text() | text:editing-duration/text() | text:modification-date/text() | text:creator/text() | text:modification-time/text() | text:page-count/text() | text:paragraph-count/text() | text:word-count/text() | text:character-count/text() | text:table-count/text() | text:image-count/text() | text:object-count/text() | text:database-display/text() | text:database-row-number/text() | text:database-name/text() | text:page-variable-get/text() | text:placeholder/text() | text:conditional-text/text() | text:hidden-text/text() | text:execute-macro/text() | text:dde-connection/text() | text:measure/text() | text:table-formula/text()">
		<choose>
			<when test="boolean(./ancestor::table:table-header-rows | ./ancestor::text:h)">
				<!-- 
					No explicit styles within table headings or section headings, 
					because those styles are consistently declared by the Wiki engine. -->
				<value-of select="."/>
			</when>
			
			<when test="string-length(.) &gt; 0">
				<variable name="style">
					<call-template name="mk-style-set">
						<with-param name="node" select="."/>
					</call-template>
				</variable>
				
				<variable name="current-paragraph" 
					select="./ancestor::text:p[1]"/>
				<variable name="paragraph-id" 
					select="generate-id($current-paragraph)"/>
				<variable name="frames" 
					select="$current-paragraph/descendant::draw:frame"/>
				<variable name="frame-count" 
					select="count($frames)"/>
					
				<!-- 
					The current style context consists of all text nodes that are 
					descendants of the paragraph ancestor of this text node but not 
					descendants of any frame nodes that are descendants of the current 
					text nodes paragraph.
				 -->
				<variable name="context" 
					select="$current-paragraph//text()[not(boolean(./ancestor::draw:frame[1]) and count(./ancestor::draw:frame[1] | $frames) = $frame-count)]"/>
				<variable name="context-size" select="count($context)"/>

				<variable name="context-index">
					<call-template name="mk-context-index">
						<with-param name="current-id" select="generate-id(.)"/>
						<with-param name="context" select="$context"/>
						<with-param name="test-index" select="1"/>
					</call-template>
				</variable>

				<variable name="style-left">
					<choose>
						<when test="$context-index &gt; 1">
							<variable name="left" select="$context[$context-index - 1]"/>
							<!-- 
								The preceding text node is a child of this nodes topmost 
								styled ancestor. This means that the result of the 
								transformation will be directly concatenated. 
								-->
							<call-template name="mk-style-set">
								<with-param name="node" select="$left"/>
							</call-template>
						</when>
						<otherwise>
							<value-of select="$NO_STYLE"/>
						</otherwise>
					</choose>
				</variable>
				<variable name="style-right">
					<choose>
						<when test="$context-index &lt; count($context)">
							<variable name="right" select="$context[$context-index + 1]"/>
							<!-- 
								The preceding text node is a child of this nodes topmost 
								styled ancestor. This means that the result of the 
								transformation will be directly concatenated. 
								-->
							<call-template name="mk-style-set">
								<with-param name="node" select="$right"/>
							</call-template>
						</when>
						<otherwise>
							<value-of select="$NO_STYLE"/>
						</otherwise>
					</choose>
				</variable>

				<variable name="bold" 
					select="($style mod (2 * $BOLD_BIT)) != 0"/>
				<variable name="italic" 
					select="($style mod (2 * $ITALIC_BIT)) - ($style mod ($ITALIC_BIT)) != 0"/>
				<variable name="superscript" 
					select="($style mod (2 * $SUPERSCRIPT_BIT)) - ($style mod ($SUPERSCRIPT_BIT)) != 0"/>
				<variable name="subscript" 
					select="($style mod (2 * $SUBSCRIPT_BIT)) - ($style mod ($SUBSCRIPT_BIT)) != 0"/>
				<variable name="code" 
					select="($style mod (2 * $CODE_BIT)) - ($style mod ($CODE_BIT)) != 0"/>
				<variable name="typewriter" 
					select="($style mod (2 * $TYPEWRITER_BIT)) - ($style mod ($TYPEWRITER_BIT)) != 0"/>

				<variable name="bold-left" 
					select="($style-left mod (2 * $BOLD_BIT)) != 0"/>
				<variable name="italic-left" 
					select="($style-left mod (2 * $ITALIC_BIT)) - ($style-left mod ($ITALIC_BIT)) != 0"/>
				<variable name="superscript-left" 
					select="($style-left mod (2 * $SUPERSCRIPT_BIT)) - ($style-left mod ($SUPERSCRIPT_BIT)) != 0"/>
				<variable name="subscript-left" 
					select="($style-left mod (2 * $SUBSCRIPT_BIT)) - ($style-left mod ($SUBSCRIPT_BIT)) != 0"/>
				<variable name="typewriter-left" 
					select="($style-left mod (2 * $TYPEWRITER_BIT)) - ($style-left mod ($TYPEWRITER_BIT)) != 0"/>

				<variable name="bold-right" 
					select="($style-right mod (2 * $BOLD_BIT)) != 0"/>
				<variable name="italic-right" 
					select="($style-right mod (2 * $ITALIC_BIT)) - ($style-right mod ($ITALIC_BIT)) != 0"/>
				<variable name="superscript-right" 
					select="($style-right mod (2 * $SUPERSCRIPT_BIT)) - ($style-right mod ($SUPERSCRIPT_BIT)) != 0"/>
				<variable name="subscript-right" 
					select="($style-right mod (2 * $SUBSCRIPT_BIT)) - ($style-right mod ($SUBSCRIPT_BIT)) != 0"/>
				<variable name="typewriter-right" 
					select="($style-right mod (2 * $TYPEWRITER_BIT)) - ($style-right mod ($TYPEWRITER_BIT)) != 0"/>

				<!-- Debugging: Add style infos to the output. -->
				<!-- 
				<value-of select="'{'"/>
				<value-of select="$style-left"/>
				<value-of select="'-'"/>
				<value-of select="$style"/>
				<value-of select="','"/>
				<value-of select="$context-size"/>
				<value-of select="'}'"/>
				 -->

				<if test="$superscript and not($superscript-left)">
					<text>&lt;sup&gt;</text>
				</if>
				<if test="$subscript and not($subscript-left)">
					<text>&lt;sub&gt;</text>
				</if>
				<if test="not($code) and $typewriter and not($typewriter-left)">
					<text>&lt;tt&gt;</text>
				</if>
				<if test="$bold and not($bold-left)">
					<text>'''</text>
				</if>
				<if test="$italic and not($italic-left)">
					<text>''</text>
				</if>

				<call-template name="render-quoted-text">
					<with-param name="text" select="."/>
				</call-template>

				<if test="$italic and not($italic-right)">
					<text>''</text>
				</if>
				<if test="$bold and not($bold-right)">
					<text>'''</text>
				</if>
				<if test="not($code) and $typewriter and not($typewriter-right)">
					<text>&lt;/tt&gt;</text>
				</if>
				<if test="$subscript and not($subscript-right)">
					<text>&lt;/sub&gt;</text>
				</if>
				<if test="$superscript and not($superscript-right)">
					<text>&lt;/sup&gt;</text>
				</if>

				<!-- Debugging: Add style details to the output. -->
				<!-- 
				<value-of select="'{'"/>
				<value-of select="$style"/>
				<value-of select="'-'"/>
				<value-of select="$style-right"/>
				<value-of select="'}'"/>
				 -->

			</when>
		</choose>
	</template>

	<!-- 
		Function for looking up the position of a node identified by the given 
		'current-id' within a node set 'context'. 
		
		The search starts with the the index 'test-index'. The search is recursive 
		in the 'test-index' argument. To save recursion depth, each recursive call 
		iteratively tests a fixed number of indexes (by loop unrolling).
	 -->
	<template name="mk-context-index">
		<param name="current-id"/>
		<param name="context"/>
		<param name="test-index"/>
		
		<variable name="context-size" select="count($context)"/>
		
		<choose>
			<when test="context-size &lt; $test-index">
			</when>
			<when test="$current-id = generate-id($context[$test-index])">
				<value-of select="$test-index"/>
			</when>
			<when test="context-size &lt; ($test-index + 1)">
			</when>
			<when test="$current-id = generate-id($context[$test-index + 1])">
				<value-of select="$test-index + 1"/>
			</when>
			<when test="context-size &lt; ($test-index + 2)">
			</when>
			<when test="$current-id = generate-id($context[$test-index + 2])">
				<value-of select="$test-index + 2"/>
			</when>
			<when test="context-size &lt; ($test-index + 3)">
			</when>
			<when test="$current-id = generate-id($context[$test-index + 3])">
				<value-of select="$test-index + 3"/>
			</when>
			<when test="context-size &lt; ($test-index + 4)">
			</when>
			<when test="$current-id = generate-id($context[$test-index + 4])">
				<value-of select="$test-index + 4"/>
			</when>
			<when test="context-size &lt; ($test-index + 5)">
			</when>
			<when test="$current-id = generate-id($context[$test-index + 5])">
				<value-of select="$test-index + 5"/>
			</when>
			<when test="context-size &lt; ($test-index + 6)">
			</when>
			<otherwise>
				<call-template name="mk-context-index">
					<with-param name="current-id" select="$current-id"/>
					<with-param name="context" select="$context"/>
					<with-param name="test-index" select="$test-index + 6"/>
				</call-template>
			</otherwise>
		</choose>
	</template>
	
	<template name="render-quoted-text">
		<param name="text"/>
		
		<choose>
			<when test="contains($text, '&lt;') or contains($text, '[') or starts-with($text, '----') or starts-with($text, '=') or starts-with($text, '*')  or starts-with($text, ';')  or starts-with($text, '#')">
				<text>&lt;nowiki&gt;</text>
				<choose>
					<when test="contains($text, '&lt;/nowiki&gt;')">
						<call-template name="render-escaped-text">
							<with-param name="text" select="$text"/>
						</call-template>
					</when>
					<otherwise>
						<call-template name="render-encoded-text">
							<with-param name="text" select="$text"/>
						</call-template>
					</otherwise>
				</choose>
				<text>&lt;/nowiki&gt;</text>			
			</when>
			<otherwise>
				<call-template name="render-encoded-text">
					<with-param name="text" select="$text"/>
				</call-template>
			</otherwise>
		</choose>
	</template>

	<template name="render-escaped-text">
		<param name="text"/>
		
		<choose>
			<when test="contains($text, '&lt;')">
				<call-template name="render-encoded-text">
					<with-param name="text" select="substring-before($text, '&lt;')"/>
				</call-template>
				<value-of select="'&amp;lt;'"/>
				<call-template name="render-escaped-text">
					<with-param name="text" select="substring-after($text, '&lt;')"/>
				</call-template>
			</when>
			<otherwise>
				<call-template name="render-encoded-text">
					<with-param name="text" select="$text"/>
				</call-template>
			</otherwise>
		</choose>
	</template>

	<template name="render-encoded-text">
		<param name="text"/>
		
		<choose>
			<when test="contains($text, '&#160;')">
				<value-of select="substring-before($text, '&#160;')"/>
				<value-of select="'&amp;nbsp;'"/>
				<call-template name="render-encoded-text">
					<with-param name="text" select="substring-after($text, '&#160;')"/>
				</call-template>
			</when>
			<otherwise>
				<value-of select="$text"/>
			</otherwise>
		</choose>
	</template>

	<!-- 
		== Wiki styles: bold, italics, ... == 
	 -->

	<template name="mk-style-set">
		<param name="node"/>
		
		<variable 
			name="context" 
			select="$node/ancestor-or-self::*[@text:style-name][1]"
		/>
		
		<choose>
			<when test="boolean($context)">
				<variable 
					name="style" 
					select="key('style-ref', $context/@text:style-name)"
				/>

				<!-- Debugging: Print inspected styles. -->				
				<!-- 
				<message>
					<value-of select="'=== '"/>
					<value-of select="$style/@style:name"/>
					<value-of select="' ==='"/>
				</message>
				 -->
		
				<call-template name="mk-style-set-internal">
					<with-param name="node" select="$context"/>
					<with-param name="style" select="$style"/>
					<with-param name="style-set" select="$NO_STYLE"/>
					<with-param name="style-mask" select="$NO_STYLE"/>
				</call-template>
			</when>
			<otherwise>
				<value-of select="$NO_STYLE"/>
			</otherwise>
		</choose>
	</template>
	
	<!-- 
		Compute the wiki style set that corresponds 
		to the given office style at the given context node. 
		
		@param node
			A node in which context the style is computed. If neither the given style 
			nor one of its linked styles does specify a style of the given type, 
			ancestor nodes of the given context node are considered.
		@param style
			A style:style element node. The style of the requested type is searched
			in the given style and its linked styles. 
		@style-set
			A bit set of styles already defined by the context.
		@style-mask
			A bit set of styles that must not be taken from the currently inspected 
			style, because those styles are already defined by the context.
			
		@return A bit set composed of the wiki style constants.
	-->
	<template name="mk-style-set-internal">
		<param name="node"/>
		<param name="style"/>
		<param name="style-set"/>
		<param name="style-mask"/>
		
		<variable name="text-properties" select="$style/style:text-properties"/>
		
		<!-- Decompose style-mask into individual bits. -->
		<variable name="bold-requested" 
			select="($style-mask mod (2 * $BOLD_BIT)) = 0"/>
		<variable name="italic-requested" 
			select="($style-mask mod (2 * $ITALIC_BIT)) - ($style-mask mod ($ITALIC_BIT)) = 0"/>
		<variable name="superscript-requested" 
			select="($style-mask mod (2 * $SUPERSCRIPT_BIT)) - ($style-mask mod ($SUPERSCRIPT_BIT)) = 0"/>
		<variable name="subscript-requested" 
			select="($style-mask mod (2 * $SUBSCRIPT_BIT)) - ($style-mask mod ($SUBSCRIPT_BIT)) = 0"/>
		<variable name="typewriter-requested" 
			select="($style-mask mod (2 * $TYPEWRITER_BIT)) - ($style-mask mod ($TYPEWRITER_BIT)) = 0"/>
		<variable name="code-requested" 
			select="($style-mask mod (2 * $CODE_BIT)) - ($style-mask mod ($CODE_BIT)) = 0"/>
		<variable name="center-requested" 
			select="($style-mask mod (2 * $CENTER_BIT)) - ($style-mask mod ($CENTER_BIT)) = 0"/>
		<variable name="right-requested" 
			select="($style-mask mod (2 * $RIGHT_BIT)) - ($style-mask mod ($RIGHT_BIT)) = 0"/>
		
		<!-- Extract styles that are not already defined by the context. -->
		<variable name="bold-style">
			<choose>
				<when test="$bold-requested and boolean($text-properties/@fo:font-weight='bold')">
					<!-- Bold found in current style. -->
					<value-of select="$BOLD_BIT"/>
				</when>
				<otherwise>
					<value-of select="$NO_STYLE"/>
				</otherwise>
			</choose>
		</variable>
		<variable name="bold-mask">
			<choose>
				<when test="$bold-requested and boolean($text-properties/@fo:font-weight)">
					<!-- 
						Other value than "bold" means that the character style is not 
						bold and no parent style must be considered.
					-->
					<value-of select="$BOLD_BIT"/>
				</when>
				<otherwise>
					<value-of select="$NO_STYLE"/>
				</otherwise>
			</choose>
		</variable>
		
		<variable name="italic-style">
			<choose>
				<when test="$italic-requested and boolean($text-properties/@fo:font-style='italic')">
					<!-- Italic found in current style. -->
					<value-of select="$ITALIC_BIT"/>
				</when>
				<otherwise>
					<value-of select="$NO_STYLE"/>
				</otherwise>
			</choose>
		</variable>
		<variable name="italic-mask">
			<choose>
				<when test="$italic-requested and boolean($text-properties/@fo:font-style)">
					<!-- 
						Other value than "italic" means that the character style is not 
						italic and no parent style must be considered.
					-->
					<value-of select="$ITALIC_BIT"/>
				</when>
				<otherwise>
					<value-of select="$NO_STYLE"/>
				</otherwise>
			</choose>
		</variable>

		<variable name="superscript-style">
			<choose>
				<when test="$superscript-requested and contains($text-properties/@style:text-position, 'super')">
					<value-of select="$SUPERSCRIPT_BIT"/>
				</when>
				<otherwise>
					<value-of select="$NO_STYLE"/>
				</otherwise>
			</choose>
		</variable>
		<variable name="superscript-mask">
			<choose>
				<when test="$superscript-requested and boolean($text-properties/@style:text-position)">
					<value-of select="$SUPERSCRIPT_BIT"/>
				</when>
				<otherwise>
					<value-of select="$NO_STYLE"/>
				</otherwise>
			</choose>
		</variable>

		<variable name="subscript-style">
			<choose>
				<when test="$subscript-requested and contains($text-properties/@style:text-position, 'sub')">
					<value-of select="$SUBSCRIPT_BIT"/>
				</when>
				<otherwise>
					<value-of select="$NO_STYLE"/>
				</otherwise>
			</choose>
		</variable>
		<variable name="subscript-mask">
			<choose>
				<when test="$subscript-requested and boolean($text-properties/@style:text-position)">
					<value-of select="$SUBSCRIPT_BIT"/>
				</when>
				<otherwise>
					<value-of select="$NO_STYLE"/>
				</otherwise>
			</choose>
		</variable>

		<variable name="typewriter-style">
			<choose>
				<when test="$typewriter-requested and ($style/@style:family='text') and boolean($text-properties/@style:font-name)">
					<variable name="font-face" 
						select="key('font-face-ref', $text-properties/@style:font-name)"/>
					<choose>
						<when test="$font-face/@style:font-pitch='fixed'">
							<value-of select="$TYPEWRITER_BIT"/>
						</when>
						<otherwise>
							<value-of select="$NO_STYLE"/>
						</otherwise>
					</choose>
				</when>
				<otherwise>
					<value-of select="$NO_STYLE"/>
				</otherwise>
			</choose>
		</variable>
		<variable name="typewriter-mask">
			<choose>
				<!-- Note: Suppress the typewriter style on text within a code paragraph. -->
				<when test="$typewriter-requested and boolean($text-properties/@style:font-name)">
					<value-of select="$TYPEWRITER_BIT"/>
				</when>
				<otherwise>
					<value-of select="$NO_STYLE"/>
				</otherwise>
			</choose>
		</variable>

		<variable name="code-style">
			<choose>
				<when test="$code-requested and ($style/@style:family='paragraph') and boolean($text-properties/@style:font-name)">
					<variable name="font-face" 
						select="key('font-face-ref', $text-properties/@style:font-name)"/>
					<choose>
						<when test="$font-face/@style:font-pitch='fixed' or (boolean(@style:display-name) and contains($CODE_STYLES, $style/@style:display-name))">
							<value-of select="$CODE_BIT"/>
						</when>
						<otherwise>
							<value-of select="$NO_STYLE"/>
						</otherwise>
					</choose>
				</when>
				<otherwise>
					<value-of select="$NO_STYLE"/>
				</otherwise>
			</choose>
		</variable>
		<variable name="code-mask">
			<choose>
				<when test="$code-requested and ($style/@style:family='paragraph') and boolean($text-properties/@style:font-name)">
					<value-of select="$CODE_BIT"/>
				</when>
				<otherwise>
					<value-of select="$NO_STYLE"/>
				</otherwise>
			</choose>
		</variable>
		
		<variable name="center-style">
			<choose>
				<when test="$center-requested and ($style/@style:family='paragraph') and boolean($style/style:paragraph-properties/@fo:text-align='center')">
					<value-of select="$CENTER_BIT"/>
				</when>
				<otherwise>
					<value-of select="$NO_STYLE"/>
				</otherwise>
			</choose>
		</variable>
		<variable name="center-mask">
			<choose>
				<when test="$center-requested and ($style/@style:family='paragraph') and boolean($style/style:paragraph-properties/@fo:text-align)">
					<value-of select="$CENTER_BIT"/>
				</when>
				<otherwise>
					<value-of select="$NO_STYLE"/>
				</otherwise>
			</choose>
		</variable>

		<variable name="right-style">
			<choose>
				<when test="$right-requested and ($style/@style:family='paragraph') and boolean($style/style:paragraph-properties/@fo:text-align='end')">
					<value-of select="$RIGHT_BIT"/>
				</when>
				<otherwise>
					<value-of select="$NO_STYLE"/>
				</otherwise>
			</choose>
		</variable>
		<variable name="right-mask">
			<choose>
				<when test="$center-requested and ($style/@style:family='paragraph') and boolean($style/style:paragraph-properties/@fo:text-align)">
					<value-of select="$RIGHT_BIT"/>
				</when>
				<otherwise>
					<value-of select="$NO_STYLE"/>
				</otherwise>
			</choose>
		</variable>
		

		<!-- Compute the updated styles and mask. -->
		<!-- 
			Note: The bit masks style-mask, bold-style, italic-style,... are 
			guaranteed to be disjoint, therefore, addition can be use instead 
			of bitwise or (which is missing in XPath). -->
		<variable name="updated-style" 
			select="$style-set + $bold-style + $italic-style + $superscript-style + $subscript-style + $code-style + $typewriter-style + $center-style + $right-style"/>
		<variable name="updated-mask" 
			select="$style-mask + $bold-mask + $italic-mask + $superscript-mask + $subscript-mask + $code-mask + $typewriter-mask + $center-mask + $right-mask"/>

		<!-- Inspect linked and nested styles. -->
		<choose>
			<when test="boolean($style/@style:parent-style-name)">
				<!-- Look through the style, the current style is based on. -->
				<call-template name="mk-style-set-internal">
					<with-param name="node" select="$node"/>
					<with-param name="style" select="key('style-ref', $style/@style:parent-style-name)"/>
					<with-param name="style-set" select="$updated-style"/>
					<with-param name="style-mask" select="$updated-mask"/>
				</call-template>
			</when>
			<otherwise>
				<variable name="ancestors" select="$node/ancestor::*[@text:style-name][1]"/>
				
				<!-- Debugging: Print currently inspected style.  -->
				<!-- 
				<message>
					<value-of select="'{'"/>
					<value-of select="$style/@style:name"/>
					<value-of select="','"/>
					<value-of select="$updated-style"/>
					<value-of select="','"/>
					<value-of select="$updated-mask"/>
					<value-of select="','"/>
					<value-of select="local-name($ancestors)"/>
					<value-of select="',('"/>
					<value-of select="$node"/>
					<value-of select="')'"/>
					<value-of select="'}'"/>
				</message>
				 -->
				 
				<!-- 
					If there is an ancestor that has a style, use that style, 
					otherwise, a style is not found. -->
				<choose>
					<when test="boolean($ancestors)">
						<!-- Look through the style of the nearest ancestor that has a style. -->
						<call-template name="mk-style-set-internal">
							<with-param name="node" select="$ancestors"/>
							<with-param name="style" select="key('style-ref', $ancestors/@text:style-name)"/>
							<with-param name="style-set" select="$updated-style"/>
							<with-param name="style-mask" select="$updated-mask"/>
						</call-template>
					</when>
					<otherwise>
						<!-- No more styles to inspect. Return the result. -->
						<value-of select="$updated-style"/>
					</otherwise>
				</choose>
			</otherwise>
		</choose>
	</template>


	<!-- 
		== Descending the tree == 
	-->

	<template match="/">
		<apply-templates/>
		<value-of select="$NL"/>
		<if test="boolean(//text:note[@text:note-class='footnote'])">
			<value-of select="$NL"/>
			<text>----</text>
			<value-of select="$NL"/>
			<text>&lt;references/&gt;</text>
			<value-of select="$NL"/>
		</if>
	</template>
	
	<template match="office:document-content">
		<apply-templates/>
	</template>

	<template match="office:body">
		<apply-templates/>
	</template>

	<template match="text:tracked-changes">
		<!-- Ignore change history. -->
	</template>

	<template match="office:* | text:* | draw:text-box | draw:a">
		<apply-templates/>
	</template>

	<template match="node()">
	</template>
</stylesheet>

<!--
  Local Variables:
	tab-width: 4
	sgml-indent-step: 4
  End:
-->
