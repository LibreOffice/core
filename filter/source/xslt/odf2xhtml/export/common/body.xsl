<?xml version="1.0" encoding="UTF-8"?>
<!--

  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
  
  Copyright 2008 by Sun Microsystems, Inc.
 
  OpenOffice.org - a multi-platform office productivity suite
 
  $RCSfile: body.xsl,v $
 
  $Revision: 1.3 $
 
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
<!--
	For further documentation and updates visit http://xml.openoffice.org/odf2xhtml
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:chart="urn:oasis:names:tc:opendocument:xmlns:chart:1.0" xmlns:config="urn:oasis:names:tc:opendocument:xmlns:config:1.0" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dom="http://www.w3.org/2001/xml-events" xmlns:dr3d="urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0" xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" xmlns:form="urn:oasis:names:tc:opendocument:xmlns:form:1.0" xmlns:math="http://www.w3.org/1998/Math/MathML" xmlns:meta="urn:oasis:names:tc:opendocument:xmlns:meta:1.0" xmlns:number="urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0" xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" xmlns:ooo="http://openoffice.org/2004/office" xmlns:oooc="http://openoffice.org/2004/calc" xmlns:ooow="http://openoffice.org/2004/writer" xmlns:script="urn:oasis:names:tc:opendocument:xmlns:script:1.0" xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" xmlns:xforms="http://www.w3.org/2002/xforms" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:java="http://xml.apache.org/xslt/java" xmlns:urlencoder="http://www.jclark.com/xt/java/java.net.URLEncoder" exclude-result-prefixes="chart config dc dom dr3d draw fo form math meta number office ooo oooc ooow script style svg table text xforms xlink xsd xsi java urlencoder">


	<xsl:include href="table_of_content.xsl"/>


	<!-- ****************** -->
	<!-- *** Whitespace *** -->
	<!-- ****************** -->

	<xsl:template match="text:s">
		<xsl:call-template name="write-breakable-whitespace">
			<xsl:with-param name="whitespaces" select="@text:c"/>
		</xsl:call-template>
	</xsl:template>


	<!--write the number of 'whitespaces' -->
	<xsl:template name="write-breakable-whitespace">
		<xsl:param name="whitespaces"/>

		<!--write two space chars as the normal white space character will be stripped
			and the other is able to break -->
		<xsl:text>&#160;</xsl:text>
		<xsl:if test="$whitespaces >= 2">
			<xsl:call-template name="write-breakable-whitespace-2">
				<xsl:with-param name="whitespaces" select="$whitespaces - 1"/>
			</xsl:call-template>
		</xsl:if>
	</xsl:template>


	<!--write the number of 'whitespaces' -->
	<xsl:template name="write-breakable-whitespace-2">
		<xsl:param name="whitespaces"/>
		<!--write two space chars as the normal white space character will be stripped
			and the other is able to break -->
		<xsl:text> </xsl:text>
		<xsl:if test="$whitespaces >= 2">
			<xsl:call-template name="write-breakable-whitespace">
				<xsl:with-param name="whitespaces" select="$whitespaces - 1"/>
			</xsl:call-template>
		</xsl:if>
	</xsl:template>

	<!-- currentSolution: 8 non-breakable-spaces instead of a TAB is an approximation.
		 Sometimes less spaces than 8 might be needed and the output might be more difficult to read-->
	<xsl:template match="text:tab">
		<xsl:param name="globalData"/>

		<xsl:call-template name="write-breakable-whitespace">
			<xsl:with-param name="whitespaces" select="8"/>
		</xsl:call-template>
	</xsl:template>



	<!-- *************** -->
	<!-- *** Textbox *** -->
	<!-- *************** -->

	<!-- ID / NAME of text-box -->
	<xsl:template match="@draw:name">
		<xsl:attribute name="id">
			<!-- a simplified string to ID conversion -->
			<xsl:value-of select="translate(., '.,;: %()[]/\+', '_____________')"/>
		</xsl:attribute>
	</xsl:template>


	<xsl:template match="text:line-break">
		<xsl:param name="listIndent"/>

		<xsl:element namespace="{$namespace}" name="br"/>

		<!-- line breaks in lists need an indent similar to the list label -->
		<xsl:if test="$listIndent">
			<xsl:element namespace="{$namespace}" name="span">
				<xsl:attribute name="style">margin-left:<xsl:value-of select="$listIndent"/>cm</xsl:attribute>
			</xsl:element>
		</xsl:if>
	</xsl:template>


	<!-- currently there have to be an explicit call of the style attribute nodes, maybe the attributes nodes have no priority only order relevant-->
	<xsl:template name="apply-styles-and-content">
		<xsl:param name="globalData"/>
		<xsl:param name="footnotePrefix" />
		<xsl:apply-templates select="@*">
			<xsl:with-param name="globalData" select="$globalData"/>
		</xsl:apply-templates>
		<!-- the footnote symbol is the prefix for a footnote in the footer -->
		<xsl:copy-of select="$footnotePrefix"/>
		<xsl:apply-templates select="node()">
			<xsl:with-param name="globalData" select="$globalData"/>
		</xsl:apply-templates>
	</xsl:template>


	<!-- ******************* -->
	<!-- *** References  *** -->
	<!-- ******************* -->

	<xsl:template match="text:reference-ref | text:sequence-ref">
		<xsl:param name="globalData"/>
		<xsl:element namespace="{$namespace}" name="a">
			<xsl:attribute name="href">
				<xsl:text>#</xsl:text>
				<xsl:value-of select="translate(@text:ref-name, '.,;: %()[]/\+', '_____________')"/>
			</xsl:attribute>
			<xsl:if test="@text:name">
				<xsl:attribute name="title">
					<xsl:value-of select="@text:name"/>
				</xsl:attribute>
			</xsl:if>

			<xsl:apply-templates select="@* | node()">
				<xsl:with-param name="globalData" select="$globalData"/>
			</xsl:apply-templates>
		</xsl:element>
	</xsl:template>

	<xsl:template match="text:sequence">
		<xsl:param name="globalData"/>

		<xsl:element namespace="{$namespace}" name="a">
			<xsl:apply-templates select="@*" />
			<xsl:attribute name="name">
				<xsl:value-of select="translate(@text:ref-name, '.,;: %()[]/\+', '_____________')"/>
			</xsl:attribute>
		</xsl:element>

		<xsl:apply-templates>
			<xsl:with-param name="globalData" select="$globalData"/>
		</xsl:apply-templates>
	</xsl:template>

	<xsl:template match="text:reference-mark">
		<xsl:param name="globalData"/>

		<xsl:element namespace="{$namespace}" name="a">
			<xsl:apply-templates select="@*" />
			<xsl:attribute name="name">
				<xsl:value-of select="translate(@text:name, '.,;: %()[]/\+', '_____________')"/>
			</xsl:attribute>
		</xsl:element>

		<xsl:apply-templates>
			<xsl:with-param name="globalData" select="$globalData"/>
		</xsl:apply-templates>
	</xsl:template>


	<xsl:template match="text:reference-mark-start">
		<xsl:param name="globalData"/>

		<xsl:element namespace="{$namespace}" name="a">
			<xsl:apply-templates select="@*" />
		</xsl:element>
	</xsl:template>

	<xsl:template match="text:bibliography-mark">
		<xsl:param name="globalData"/>

		<xsl:element namespace="{$namespace}" name="a">
			<xsl:apply-templates select="@* | node()">
				<xsl:with-param name="globalData" select="$globalData"/>
			</xsl:apply-templates>
		</xsl:element>
	</xsl:template>

	<!-- @text:title exist only in text:bibliography-mark -->
	<xsl:template match="@text:title">
		<xsl:attribute name="name">
			<xsl:value-of select="translate(., '.,;: %()[]/\+', '_____________')"/>
		</xsl:attribute>
		<xsl:attribute name="title">
			<xsl:value-of select="."/>
		</xsl:attribute>
	</xsl:template>

	<!-- @text:url exist only in text:bibliography-mark -->
	<xsl:template match="@text:url">
		<xsl:attribute name="href">
			<xsl:value-of select="."/>
		</xsl:attribute>
	</xsl:template>

	<xsl:template match="text:user-defined">
		<xsl:apply-templates/>
	</xsl:template>

	<!-- *************** -->
	<!-- *** HELPER  *** -->
	<!-- *************** -->


	<xsl:template name="create-href">
		<xsl:param name="href"/>

		<xsl:choose>
			<!-- internal OOo URL used in content tables -->
			<xsl:when test="contains($href, '%7Coutline')">
				<!-- the simple workaround for content tables in a single document is to create create an anchor from every heading element
					 work-around downside: Multiple identical headings won't refer always to the first.
				-->
				<xsl:text>#</xsl:text>
				<xsl:variable name="title">
					<xsl:apply-templates mode="concatenate"/>
				</xsl:variable>

				<xsl:value-of select="translate($title, '.,;: %()[]/\+', '_____________')"/>
			</xsl:when>
			<xsl:when test="self::draw:image[office:binary-data]">
				<xsl:text>data:image/*;base64,</xsl:text><xsl:value-of select="office:binary-data"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:choose>
					 <!-- in case of packed open office document -->
					<xsl:when test="starts-with($sourceBaseURL, 'jar:') or $isPackageFormat">
						<xsl:choose>
							 <!-- for images relative to open office document -->
							<xsl:when test="starts-with($href, '../')">
								<!-- creating an absolute http URL to the packed image file (removing the '.')-->
								<xsl:value-of select="concat(substring-after(substring-before($sourceBaseURL, '!'), 'jar:'), '/', $href, $optionalURLSuffix)"/>
							</xsl:when>
							 <!-- for absolute URLs & absolute paths -->
							<xsl:when test="contains($href, ':') or starts-with($href, '/')">
								<xsl:value-of select="concat($href, $optionalURLSuffix)"/>
							</xsl:when>
							<!-- for images jared in open office document -->
							<xsl:otherwise>
								<xsl:value-of select="concat($sourceBaseURL, $href, $optionalURLSuffix)"/>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:when>
					<xsl:otherwise>
						<xsl:choose>
							 <!-- for absolute URLs & Paths -->
							<xsl:when test="contains($href, ':') or starts-with($href, '/')">
								<xsl:value-of select="concat($href, $optionalURLSuffix)"/>
							</xsl:when>
							 <!-- for relative URLs -->
							<xsl:otherwise>
								<xsl:value-of select="concat($sourceBaseURL, $href, $optionalURLSuffix)"/>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>

	</xsl:template>

	<xsl:template match="text()" mode="concatenate">
		<xsl:value-of select="."/>
	</xsl:template>
	<xsl:template match="*" mode="concatenate">
		<xsl:apply-templates mode="concatenate"/>
	</xsl:template>


	<!-- ******************** -->
	<!-- *** Common Rules *** -->
	<!-- ******************** -->

	<!-- ignore / neglect the following elements -->
	<xsl:template match="office:forms | text:alphabetical-index-mark | text:alphabetical-index-mark-end | text:alphabetical-index-mark-start | text:bibliography-source | text:reference-mark-end | text:sequence-decls | text:soft-page-break | text:table-of-content-source | text:tracked-changes | text:user-field-decls"/>

	<!-- default template used by purpose-->
	<xsl:template match="text:bibliography | text:change-end | text:change-start">
		<xsl:param name="globalData"/>

		<xsl:apply-templates>
			<xsl:with-param name="globalData" select="$globalData"/>
		</xsl:apply-templates>
	</xsl:template>

	<!-- default template for not recognized elements -->
	<xsl:template match="*">
		<xsl:param name="globalData"/>
		<xsl:message>Using default element rule for ODF element '<xsl:value-of select="name()"/>'.</xsl:message>

		<xsl:apply-templates>
			<xsl:with-param name="globalData" select="$globalData"/>
		</xsl:apply-templates>
	</xsl:template>

	<xsl:template match="@*"/>

	<!-- allowing all matched text nodes -->
	<xsl:template match="text()">
		<xsl:value-of select="."/>
	</xsl:template>

</xsl:stylesheet>
