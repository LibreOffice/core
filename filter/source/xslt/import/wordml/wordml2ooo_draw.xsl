<?xml version="1.0" encoding="UTF-8"?>
<!--
   The Contents of this file are made available subject to the terms of
   either of the following licenses

          - GNU Lesser General Public License Version 2.1
          - Sun Industry Standards Source License Version 1.1

   Sun Microsystems Inc., October, 2000

   GNU Lesser General Public License Version 2.1
   =============================================
   Copyright 2000 by Sun Microsystems, Inc.
   901 San Antonio Road, Palo Alto, CA 94303, USA

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License version 2.1, as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
   MA  02111-1307  USA


   Sun Industry Standards Source License Version 1.1
   =================================================
   The contents of this file are subject to the Sun Industry Standards
   Source License Version 1.1 (the "License"); You may not use this file
   except in compliance with the License. You may obtain a copy of the
   License at http://www.openoffice.org/license.html.

   Software provided under this License is provided on an "AS IS" basis,
   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
   See the License for the specific provisions governing your rights and
   obligations concerning the Software.

   The Initial Developer of the Original Code is: Sun Microsystems, Inc.

   Copyright © 2002 by Sun Microsystems, Inc.

   All Rights Reserved.

   Contributor(s): _______________________________________
   
 -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format" xmlns:w="http://schemas.microsoft.com/office/word/2003/wordml" xmlns:wx="http://schemas.microsoft.com/office/word/2003/auxHint" xmlns:o="urn:schemas-microsoft-com:office:office" xmlns:aml="http://schemas.microsoft.com/aml/2001/core" xmlns:dt="uuid:C2F41010-65B3-11d1-A29F-00AA00C14882" xmlns:v="urn:schemas-microsoft-com:vml" xmlns:office="urn:oasis:names:tc:openoffice:xmlns:office:1.0" xmlns:style="urn:oasis:names:tc:openoffice:xmlns:style:1.0" xmlns:text="urn:oasis:names:tc:openoffice:xmlns:text:1.0" xmlns:table="urn:oasis:names:tc:openoffice:xmlns:table:1.0" xmlns:draw="urn:oasis:names:tc:openoffice:xmlns:drawing:1.0" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:meta="urn:oasis:names:tc:openoffice:xmlns:meta:1.0" xmlns:number="urn:oasis:names:tc:openoffice:xmlns:datastyle:1.0" xmlns:svg="http://www.w3.org/2000/svg" xmlns:chart="urn:oasis:names:tc:openoffice:xmlns:chart:1.0" xmlns:dr3d="urn:oasis:names:tc:openoffice:xmlns:dr3d:1.0" xmlns:math="http://www.w3.org/1998/Math/MathML" xmlns:form="urn:oasis:names:tc:openoffice:xmlns:form:1.0" xmlns:script="urn:oasis:names:tc:openoffice:xmlns:script:1.0" xmlns:config="urn:oasis:names:tc:openoffice:xmlns:config:1.0" xmlns:ooo="http://openoffice.org/2004/office" xmlns:ooow="http://openoffice.org/2004/writer" xmlns:oooc="http://openoffice.org/2004/calc" xmlns:dom="http://www.w3.org/2001/xml-events" exclude-result-prefixes="w wx aml o dt fo v">
    <xsl:include href="wordml2ooo_custom_draw.xsl"/>
    <xsl:key name="imagedata" match="w:binData" use="@w:name"/>
    <xsl:template match="w:pict" mode="style4dash_mark">
        <xsl:if test="descendant::v:line or descendant::v:rect or descendant::v:oval or descendant::v:arc or descendant::v:shape">
            <!--Changed-->
            <xsl:variable name="vchild" select="./v:*"/>
            <xsl:variable name="def" select="$vchild/v:stroke"/>
            <xsl:if test="$def">
                <!--<draw:marker draw:name="Stupid33" svg:viewBox="0 0 20 30" svg:d="m10 0-10 30h20z"/> -->
                <xsl:variable name="wdashstyle" select="$vchild/v:stroke/@dashstyle"/>
                <xsl:variable name="stroke-num">
                    <xsl:number from="/w:wordDocument/w:body" level="any" count="v:stroke" format="1"/>
                </xsl:variable>
                <xsl:variable name="ptweight">
                    <xsl:choose>
                        <xsl:when test="$vchild/@strokeweight">
                            <xsl:call-template name="ConvertMeasure">
                                <xsl:with-param name="TargetMeasure" select="'pt'"/>
                                <xsl:with-param name="value" select="$vchild/@strokeweight"/>
                            </xsl:call-template>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:call-template name="ConvertMeasure">
                                <xsl:with-param name="TargetMeasure" select="'pt'"/>
                                <xsl:with-param name="value" select="'1pt'"/>
                            </xsl:call-template>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:variable>
                <xsl:if test="$wdashstyle and not ($wdashstyle = 'solid')">
                    <xsl:variable name="strdashdot">
                        <xsl:call-template name="getstrdashdot">
                            <xsl:with-param name="dashstyle" select="$wdashstyle"/>
                            <!--<xsl:with-param name="weight" select="$vchild/@strokeweight"/>-->
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:variable name="draw-style">
                        <xsl:variable name="end-cap" select="$vchild/v:stroke/@endcap"/>
                        <xsl:choose>
                            <xsl:when test="$end-cap = 'round'">round</xsl:when>
                            <xsl:otherwise>rect</xsl:otherwise>
                        </xsl:choose>
                    </xsl:variable>
                    <xsl:variable name="dot1-length">
                        <xsl:call-template name="get-middle-substring">
                            <xsl:with-param name="string" select="$strdashdot"/>
                            <xsl:with-param name="prefix" select="'dol:'"/>
                            <xsl:with-param name="suffix" select="';don'"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:variable name="dot1n">
                        <xsl:call-template name="get-middle-substring">
                            <xsl:with-param name="string" select="$strdashdot"/>
                            <xsl:with-param name="prefix" select="'don:'"/>
                            <xsl:with-param name="suffix" select="';dist'"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:variable name="dist-length">
                        <xsl:call-template name="get-middle-substring">
                            <xsl:with-param name="string" select="$strdashdot"/>
                            <xsl:with-param name="prefix" select="'dist:'"/>
                            <xsl:with-param name="suffix" select="';dtl'"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:variable name="dot2-length">
                        <xsl:call-template name="get-middle-substring">
                            <xsl:with-param name="string" select="$strdashdot"/>
                            <xsl:with-param name="prefix" select="'dtl:'"/>
                            <xsl:with-param name="suffix" select="';dtn'"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:variable name="dot2n">
                        <xsl:call-template name="get-middle-substring">
                            <xsl:with-param name="string" select="$strdashdot"/>
                            <xsl:with-param name="prefix" select="'dtn:'"/>
                            <xsl:with-param name="suffix" select="';eddtn'"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:element name="draw:stroke-dash">
                        <!--dol:8;don:1;dist:2;dtl:1;dtn:2;eddtn" />-->
                        <xsl:attribute name="draw:name">
                            <xsl:value-of select="concat('Tdash',$stroke-num)"/>
                        </xsl:attribute>
                        <xsl:attribute name="draw:style">
                            <xsl:value-of select="$draw-style"/>
                        </xsl:attribute>
                        <xsl:if test="(string-length($dot1n) &gt; 0 ) and not ($dot1n ='-1')">
                            <xsl:attribute name="draw:dots1">
                                <xsl:value-of select="$dot1n"/>
                            </xsl:attribute>
                        </xsl:if>
                        <xsl:if test="(string-length($dot2n) &gt; 0 ) and not ($dot2n ='-1')">
                            <xsl:attribute name="draw:dots2">
                                <xsl:value-of select="$dot2n"/>
                            </xsl:attribute>
                        </xsl:if>
                        <xsl:if test="(string-length($dot1-length) &gt; 0 ) and not ($dot1-length ='-1')">
                            <xsl:attribute name="draw:dots1-length">
                                <xsl:call-template name="ConvertMeasure">
                                    <xsl:with-param name="TargetMeasure" select="'in'"/>
                                    <xsl:with-param name="value" select="concat($dot1-length,'pt')"/>
                                </xsl:call-template>in</xsl:attribute>
                        </xsl:if>
                        <xsl:if test="(string-length($dot2-length) &gt; 0 ) and not ($dot2-length ='-1')">
                            <xsl:attribute name="draw:dots2-length">
                                <xsl:call-template name="ConvertMeasure">
                                    <xsl:with-param name="TargetMeasure" select="'in'"/>
                                    <xsl:with-param name="value" select="concat($dot2-length,'pt')"/>
                                </xsl:call-template>in</xsl:attribute>
                        </xsl:if>
                        <xsl:if test="(string-length($dist-length) &gt; 0 ) and not ($dist-length ='-1')">
                            <xsl:variable name="valdistance-length">
                                <xsl:call-template name="ConvertMeasure">
                                    <xsl:with-param name="TargetMeasure" select="'in'"/>
                                    <xsl:with-param name="value" select="concat($dist-length,'pt')"/>
                                </xsl:call-template>
                            </xsl:variable>
                            <xsl:attribute name="draw:distance">
                                <xsl:value-of select="$valdistance-length*$ptweight"/>in</xsl:attribute>
                        </xsl:if>
                    </xsl:element>
                </xsl:if>
                <xsl:if test="$vchild/v:stroke/@startarrow">
                    <xsl:call-template name="CreateArrowStyle">
                        <xsl:with-param name="arrow-name" select="$vchild/v:stroke/@startarrow"/>
                        <xsl:with-param name="namenumber" select="concat('markerstart',$stroke-num)"/>
                    </xsl:call-template>
                </xsl:if>
                <xsl:if test="$vchild/v:stroke/@endarrow">
                    <xsl:call-template name="CreateArrowStyle">
                        <xsl:with-param name="arrow-name" select="$vchild/v:stroke/@endarrow"/>
                        <xsl:with-param name="namenumber" select="concat('markerend',$stroke-num)"/>
                    </xsl:call-template>
                </xsl:if>
                <!--<v:stroke dashstyle="1 1" startarrow="diamond" startarrowwidth="wide" startarrowlength="long" endarrow="block" 
									endarrowwidth="wide" endarrowlength="long" endcap="round"/>
					<draw:stroke-dash draw:name="2 2dots 1 dash" draw:style="rect" draw:dots1="2" draw:dots2="1" draw:dots2-length="0.0795in" 
													draw:distance="0.102in"/>
					Hehe,It need to be revised-->
            </xsl:if>
        </xsl:if>
    </xsl:template>
    <xsl:template name="CreateArrowStyle">
        <!--<draw:marker draw:name="Stupid33" svg:viewBox="0 0 20 30" svg:d="m10 0-10 30h20z"/> -->
        <xsl:param name="arrow-name"/>
        <xsl:param name="namenumber"/>
        <xsl:param name="arrow-weight"/>
        <xsl:variable name="svg-box">
            <xsl:choose>
                <xsl:when test="$arrow-name = 'block' ">0 0 1131 902</xsl:when>
                <xsl:when test="$arrow-name = 'diamond' ">0 0 10 10</xsl:when>
                <xsl:when test="$arrow-name = 'open' ">0 0 1122 2243</xsl:when>
                <xsl:when test="$arrow-name = 'oval' ">0 0 1131 1131</xsl:when>
                <xsl:when test="$arrow-name = 'diamond' ">0 0 1131 1131</xsl:when>
                <xsl:when test="$arrow-name = 'classic' ">0 0 1131 1580</xsl:when>
                <xsl:otherwise>0 0 1122 2243</xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="svg-d">
            <xsl:choose>
                <xsl:when test="$arrow-name = 'block' ">m564 0-564 902h1131z</xsl:when>
                <!--Symmetric Arrow-->
                <xsl:when test="$arrow-name = 'diamond' ">m0 0h10v10h-10z</xsl:when>
                <xsl:when test="$arrow-name = 'open' ">m0 2108v17 17l12 42 30 34 38 21 43 4 29-8 30-21 25-26 13-34 343-1532 339 1520 13 42 29 34 39 21 42 4 42-12 34-30 21-42v-39-12l-4 4-440-1998-9-42-25-39-38-25-43-8-42 8-38 25-26 39-8 42z</xsl:when>
                <xsl:when test="$arrow-name = 'oval' ">m462 1118-102-29-102-51-93-72-72-93-51-102-29-102-13-105 13-102 29-106 51-102 72-89 93-72 102-50 102-34 106-9 101 9 106 34 98 50 93 72 72 89 51 102 29 106 13 102-13 105-29 102-51 102-72 93-93 72-98 51-106 29-101 13z</xsl:when>
                <xsl:when test="$arrow-name = 'diamond' ">m0 564 564 567 567-567-567-564z</xsl:when>
                <xsl:when test="$arrow-name = 'classic' ">m1013 1491 118 89-567-1580-564 1580 114-85 136-68 148-46 161-17 161 13 153 46z</xsl:when>
                <xsl:otherwise>m0 2108v17 17l12 42 30 34 38 21 43 4 29-8 30-21 25-26 13-34 343-1532 339 1520 13 42 29 34 39 21 42 4 42-12 34-30 21-42v-39-12l-4 4-440-1998-9-42-25-39-38-25-43-8-42 8-38 25-26 39-8 42z</xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:element name="draw:marker">
            <xsl:attribute name="draw:name">
                <xsl:value-of select="$namenumber"/>
            </xsl:attribute>
            <xsl:attribute name="svg:viewBox">
                <xsl:value-of select="$svg-box"/>
            </xsl:attribute>
            <xsl:attribute name="svg:d">
                <xsl:value-of select="$svg-d"/>
            </xsl:attribute>
        </xsl:element>
    </xsl:template>
    <!-- The template MapDashConst Map the const dashstyle to a number manner
          It calls the template kickblanks to delete the extra blanks in the dash style here is the map
•	Solid (default)  •	ShortDash "2 2"
•	ShortDot "0 2" •	ShortDashDot "2 2 0 2"
•	ShortDashDotDot "2 2 0 2 0 2" •	Dot "1 2"
•	Dash "4 2" •	LongDash "8 2"
•	DashDot "4 2 1 2"•	LongDashDot "8 2 1 2"
•	LongDashDotDot "8 2 1 2 1 2"
-->
    <xsl:template name="MapDashConst">
        <xsl:param name="dashstyle"/>
        <xsl:choose>
            <xsl:when test="$dashstyle='shortDash'"> 2 2 </xsl:when>
            <xsl:when test="$dashstyle='shortDot'"> 0 2 </xsl:when>
            <xsl:when test="$dashstyle='shortDashDot'"> 2 2 0 2 </xsl:when>
            <xsl:when test="$dashstyle='shortDashDotDot'"> 2 2 0 2 0 2 </xsl:when>
            <xsl:when test="$dashstyle='dot'"> 1 2 </xsl:when>
            <xsl:when test="$dashstyle='dash'"> 4 2 </xsl:when>
            <xsl:when test="$dashstyle='longDash'"> 8 2 </xsl:when>
            <xsl:when test="$dashstyle='dashDot'"> 4 2 1 2 </xsl:when>
            <xsl:when test="$dashstyle='longDashDot'"> 8 2 1 2 </xsl:when>
            <xsl:when test="$dashstyle='longDashDotDot'"> 8 2 1 2 1 2 </xsl:when>
            <xsl:otherwise>
                <xsl:variable name="str-style" select="concat(' ',$dashstyle ,' ')"/>
                <xsl:variable name="cleanstyle">
                    <xsl:call-template name="kickblanks">
                        <xsl:with-param name="str" select="translate($str-style,' ','-')"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:value-of select="translate($cleanstyle,'-',' ')"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <!--The template is used to delete the extra blanks from a string.-->
    <xsl:template name="kickblanks">
        <xsl:param name="str"/>
        <xsl:variable name="tmpstr">
            <xsl:choose>
                <xsl:when test="contains($str,'--')">
                    <xsl:variable name="str-before">
                        <xsl:value-of select="substring-before($str,'--')"/>
                    </xsl:variable>
                    <xsl:variable name="str-after">
                        <xsl:value-of select="substring-after($str,'--')"/>
                    </xsl:variable>
                    <xsl:value-of select="concat($str-before,'-',$str-after)"/>
                </xsl:when>
                <xsl:when test="contains($str,'  ')">
                    <xsl:variable name="str-before">
                        <xsl:value-of select="substring-before($str,'  ')"/>
                    </xsl:variable>
                    <xsl:variable name="str-after">
                        <xsl:value-of select="substring-after($str,'  ')"/>
                    </xsl:variable>
                    <xsl:value-of select="concat($str-before,' ',$str-after)"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$str"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:choose>
            <xsl:when test="not ( string-length($str) = string-length($tmpstr) )">
                <xsl:variable name="restr">
                    <xsl:call-template name="kickblanks">
                        <xsl:with-param name="str" select="$tmpstr"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:value-of select="$restr"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$str"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="substrcount">
        <xsl:param name="str"/>
        <xsl:param name="substr"/>
        <xsl:choose>
            <xsl:when test="$substr and string-length($str) and contains($str,$substr) and string-length(substring-before($str,$substr)) = 0">
                <xsl:variable name="restr" select="substring-after($str,$substr)"/>
                <xsl:variable name="num">
                    <xsl:call-template name="substrcount">
                        <xsl:with-param name="str" select="$restr"/>
                        <xsl:with-param name="substr" select="$substr"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:value-of select="$num+1"/>
            </xsl:when>
            <xsl:otherwise>0</xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="lastest-substr-after">
        <xsl:param name="str"/>
        <xsl:param name="substr"/>
        <xsl:choose>
            <xsl:when test="contains($str,$substr) and string-length(substring-before($str,$substr)) = 0">
                <xsl:variable name="restr" select="substring-after($str,$substr)"/>
                <xsl:call-template name="lastest-substr-after">
                    <xsl:with-param name="str" select="$restr"/>
                    <xsl:with-param name="substr" select="$substr"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$str"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="getstrdashdot">
        <!--Remember the robust!if dashstyle is '2'?What  to do!-->
        <xsl:param name="dashstyle"/>
        <xsl:variable name="dstyle">
            <xsl:variable name="tmpstyle">
                <xsl:call-template name="MapDashConst">
                    <xsl:with-param name="dashstyle" select="$dashstyle"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:value-of select="substring-after($tmpstyle,' ')"/>
        </xsl:variable>
        <xsl:variable name="dot1" select="substring-before($dstyle,' ')"/>
        <xsl:variable name="dot1after" select="substring-after($dstyle,' ')"/>
        <xsl:variable name="dot1distance" select="substring-before($dot1after,' ')"/>
        <xsl:variable name="modesubstr1" select="concat($dot1,' ',$dot1distance,' ')"/>
        <xsl:variable name="dot1n">
            <xsl:call-template name="substrcount">
                <xsl:with-param name="str" select="$dstyle"/>
                <xsl:with-param name="substr" select="$modesubstr1"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="dot2substr">
            <xsl:call-template name="lastest-substr-after">
                <xsl:with-param name="str" select="$dstyle"/>
                <xsl:with-param name="substr" select="$modesubstr1"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="dot2">
            <xsl:choose>
                <xsl:when test="string-length($dot2substr) &gt; 3">
                    <xsl:value-of select="substring-before($dot2substr,' ')"/>
                </xsl:when>
                <xsl:otherwise>-1</xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="dot2distance">
            <xsl:choose>
                <xsl:when test="string-length($dot2substr) &gt; 3">
                    <xsl:variable name="tmpstr" select="substring-after($dot2substr,' ')"/>
                    <xsl:value-of select="substring-before($tmpstr,' ')"/>
                </xsl:when>
                <xsl:otherwise>-1</xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="modesubstr2" select="concat($dot2,' ',$dot2distance,' ') "/>
        <xsl:variable name="dot2n">
            <xsl:choose>
                <xsl:when test="string-length($dot2substr) &gt; 3">
                    <xsl:call-template name="substrcount">
                        <xsl:with-param name="str" select="$dot2substr"/>
                        <xsl:with-param name="substr" select="$modesubstr2"/>
                    </xsl:call-template>
                </xsl:when>
                <xsl:otherwise>-1</xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="valDistance">
            <!--Over all distance is the larger one!-->
            <xsl:choose>
                <xsl:when test="$dot2distance &gt;  $dot1distance">
                    <xsl:value-of select="$dot2distance"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$dot1distance"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:value-of select="concat('dol:',$dot1,';don:',$dot1n,';dist:',$valDistance,';dtl:',$dot2,';dtn:',$dot2n,';eddtn')"/>
        <!--<xsl:choose>
			<xsl:when test="$dashstyle='1 1' or $dashstyle='Dots'">
				<xsl:variable name="do"><xsl:value-of select="'1'"/></xsl:variable>
				<xsl:variable name="dol"><xsl:value-of select="$do * $cmweight"/></xsl:variable>
				<xsl:variable name="ds"><xsl:value-of select="'1'"/></xsl:variable>
				<xsl:variable name="dsl"><xsl:value-of select="$ds * $cmweight"/></xsl:variable>
				<xsl:variable name="dt"><xsl:value-of select="'-1'"/></xsl:variable>
				<xsl:variable name="dtl"><xsl:value-of select="'-1'"/></xsl:variable>
				<xsl:value-of select="concat('dsl:',$dsl,'edsl','do:',$do, 'edo;','do')"/>
			</xsl:when>
		</xsl:choose>-->
    </xsl:template>
    <xsl:template match="w:pict" mode="style">
        <xsl:if test="descendant::v:line or descendant::v:rect or descendant::v:oval or descendant::v:arc or descendant::v:shape">
            <!--Changed-->
            <xsl:variable name="vchild" select="./v:*"/>
            <xsl:variable name="def" select="$vchild/@stroke or $vchild/@stroked or $vchild/@strokecolor or $vchild/v:stroke or $vchild/@strokeweight or $vchild/@wrapcoords or $vchild/@fillcolor"/>
            <xsl:if test="$def">
                <xsl:element name="style:style">
                    <xsl:attribute name="style:name">Tgr<xsl:number from="/w:wordDocument/w:body" level="any" count="w:pict" format="1"/>
                    </xsl:attribute>
                    <xsl:attribute name="style:family">graphic</xsl:attribute>
                    <xsl:variable name="stroke-num">
                        <xsl:if test="$vchild/v:stroke">
                            <xsl:number from="/w:wordDocument/w:body" level="any" count="v:stroke" format="1"/>
                        </xsl:if>
                    </xsl:variable>
                    <xsl:variable name="draw-stroke">
                        <xsl:variable name="dashstyle" select="$vchild/v:stroke/@dashstyle"/>
                        <xsl:choose>
                            <xsl:when test="$vchild/@stroked and $vchild/@stroked='f'">none</xsl:when>
                            <xsl:when test="$dashstyle and not ($dashstyle = 'solid')">
                                <xsl:value-of select="concat('Tdash',$stroke-num)"/>
                            </xsl:when>
                            <xsl:otherwise/>
                        </xsl:choose>
                    </xsl:variable>
                    <xsl:element name="style:graphic-properties">
                        <xsl:if test="string-length($draw-stroke) &gt; 0">
                            <!--draw:stroke="dash" draw:stroke-dash="Ohon!Ultrafine dashed" -->
                            <xsl:choose>
                                <xsl:when test="not ($draw-stroke = 'none')">
                                    <xsl:attribute name="draw:stroke">
                                        <xsl:value-of select="'dash'"/>
                                    </xsl:attribute>
                                    <xsl:attribute name="draw:stroke-dash">
                                        <xsl:value-of select="$draw-stroke"/>
                                    </xsl:attribute>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:attribute name="draw:stroke">
                                        <xsl:value-of select="'none'"/>
                                    </xsl:attribute>
                                </xsl:otherwise>
                            </xsl:choose>
                        </xsl:if>
                        <xsl:if test="$vchild/v:stroke/@startarrow">
                            <!--<v:stroke startarrow="block" startarrowwidth="wide" startarrowlength="long"/-->
                            <xsl:attribute name="draw:marker-start">
                                <xsl:value-of select="concat('markerstart',$stroke-num)"/>
                            </xsl:attribute>
                        </xsl:if>
                        <xsl:if test="$vchild/v:stroke/@endarrow">
                            <!--<v:stroke startarrow="block" startarrowwidth="wide" startarrowlength="long"/-->
                            <xsl:attribute name="draw:marker-end">
                                <xsl:value-of select="concat('markerend',$stroke-num)"/>
                            </xsl:attribute>
                        </xsl:if>
                        <xsl:if test="$vchild/@strokeweight">
                            <xsl:attribute name="svg:stroke-width">
                                <xsl:call-template name="convert-with-scale-and-measure">
                                    <xsl:with-param name="value" select="$vchild/@strokeweight"/>
                                </xsl:call-template>
                            </xsl:attribute>
                        </xsl:if>
                        <xsl:if test="$vchild/@strokecolor">
                            <xsl:attribute name="svg:stroke-color">
                                <xsl:call-template name="MapConstColor">
                                    <xsl:with-param name="color" select="$vchild/@strokecolor"/>
                                </xsl:call-template>
                            </xsl:attribute>
                        </xsl:if>
                        <xsl:if test="$vchild/@fillcolor">
                            <xsl:attribute name="draw:fill-color">
                                <xsl:call-template name="MapConstColor">
                                    <xsl:with-param name="color" select="$vchild/@fillcolor"/>
                                </xsl:call-template>
                            </xsl:attribute>
                        </xsl:if>
                        <xsl:if test="$vchild/v:fill/@opacity">
                        </xsl:if>
                        <!--<xsl:apply-templates select="current()"/> this line can create a good match-->
                    </xsl:element>
                </xsl:element>
            </xsl:if>
            <xsl:if test="not ($def)">
                <!--Default style which will surely be removed during imported from a .flat file to SO-->
                <xsl:element name="style:style">
                    <xsl:attribute name="style:name">Tgr<xsl:number from="/w:wordDocument/w:body" level="any" count="w:pict" format="1"/>
                    </xsl:attribute>
                    <xsl:attribute name="style:family">graphic</xsl:attribute>
                    <style:graphic-properties draw:textarea-horizontal-align="center" draw:textarea-vertical-align="middle" style:run-through="foreground" style:wrap="run-through"/>
                </xsl:element>
            </xsl:if>
        </xsl:if>
    </xsl:template>
    <xsl:template match="w:pict">
        <xsl:param name="x-scale" select="1"/>
        <xsl:param name="y-scale" select="1"/>
        <xsl:param name="group-left" select="0"/>
        <xsl:param name="group-top" select="0"/>
        <xsl:param name="coord-left" select="0"/>
        <xsl:param name="coord-top" select="0"/>
        <xsl:param name="MeasureMark"/>
        <xsl:apply-templates>
            <xsl:with-param name="x-scale" select="$x-scale"/>
            <xsl:with-param name="y-scale" select="$x-scale"/>
            <xsl:with-param name="MeasureMark" select="$MeasureMark"/>
            <xsl:with-param name="group-left" select="$group-left"/>
            <xsl:with-param name="group-top" select="$group-top"/>
            <xsl:with-param name="coord-left" select="$coord-left"/>
            <xsl:with-param name="coord-top" select="$coord-top"/>
        </xsl:apply-templates>
    </xsl:template>
    <xsl:template name="get-middle-substring">
        <xsl:param name="string"/>
        <xsl:param name="prefix"/>
        <xsl:param name="suffix"/>
        <xsl:if test="contains($string, $prefix)">
            <xsl:choose>
                <xsl:when test="contains(substring-after( $string, $prefix), $suffix)">
                    <xsl:value-of select="substring-before(substring-after( $string, $prefix), $suffix)"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="substring-after( $string, $prefix)"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:if>
    </xsl:template>
    <xsl:template match="w:binData">
    </xsl:template>
    <xsl:template match="v:group">
        <xsl:param name="x-scale" select="1"/>
        <xsl:param name="y-scale" select="1"/>
        <xsl:param name="MeasureMark"/>
        <xsl:param name="group-left" select="0"/>
        <xsl:param name="group-top" select="0"/>
        <xsl:param name="coord-left" select="0"/>
        <xsl:param name="coord-top" select="0"/>
        <xsl:variable name="style" select="concat(@style, ';')"/>
        <xsl:variable name="left">
            <xsl:variable name="direct-left" select="substring-before(substring-after($style,';left:'),';')"/>
            <xsl:variable name="margin-left" select="substring-before( substring-after($style,'margin-left:')  ,';')"/>
            <xsl:call-template name="Add-with-Measure">
                <xsl:with-param name="value1" select="$margin-left"/>
                <xsl:with-param name="value2" select="$direct-left"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="top">
            <xsl:variable name="direct-top" select="substring-before(substring-after($style,';top:'),';')"/>
            <xsl:variable name="margin-top" select="substring-before( substring-after($style,'margin-top:')  ,';')"/>
            <xsl:call-template name="Add-with-Measure">
                <xsl:with-param name="value1" select="$margin-top"/>
                <xsl:with-param name="value2" select="$direct-top"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="width" select="substring-before( substring-after($style,'width:') ,';')"/>
        <xsl:variable name="height" select="substring-before(substring-after($style,'height:'),';')"/>
        <xsl:variable name="Current-coord-left">
            <xsl:call-template name="get-number">
                <xsl:with-param name="value" select="substring-before(@coordorigin, ',' )"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="Current-coord-top">
            <xsl:call-template name="get-number">
                <xsl:with-param name="value" select="substring-after(@coordorigin, ',' )"/>
            </xsl:call-template>
        </xsl:variable> select="substring-after(@coordorigin, ',' ) "/>
        <xsl:variable name="Current-coord-width">
            <xsl:call-template name="get-number">
                <xsl:with-param name="value" select="substring-before(@coordsize, ',' )"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="Current-coord-height">
            <xsl:call-template name="get-number">
                <xsl:with-param name="value" select="substring-after(@coordsize, ',' )"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="Current-MeasureMark">
            <xsl:choose>
                <xsl:when test="string-length($MeasureMark) &gt; 0">
                    <xsl:value-of select="$MeasureMark"/>
                </xsl:when>
                <xsl:when test="string-length( translate($left,'-.0123456789 ','') ) &gt; 0">
                    <xsl:value-of select="translate($left,'-.0123456789 ','') "/>
                </xsl:when>
                <xsl:when test="string-length( translate($width,'-.0123456789 ','')  ) &gt; 0">
                    <xsl:value-of select="translate($width,'-.0123456789 ','') "/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select=" 'pt' "/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="left-value">
            <xsl:variable name="adjusted-left">
                <xsl:call-template name="convert-with-scale-and-measure">
                    <xsl:with-param name="value" select="$left"/>
                    <xsl:with-param name="scale" select="$x-scale"/>
                    <xsl:with-param name="MeasureMark" select="$Current-MeasureMark"/>
                    <xsl:with-param name="Target-Measure" select="$Current-MeasureMark"/>
                    <xsl:with-param name="group-value" select="$group-left"/>
                    <xsl:with-param name="coord-value" select="$coord-left"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:call-template name="get-number">
                <xsl:with-param name="value" select="$adjusted-left"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="top-value">
            <xsl:variable name="adjusted-top">
                <xsl:call-template name="convert-with-scale-and-measure">
                    <xsl:with-param name="value" select="$top"/>
                    <xsl:with-param name="scale" select="$x-scale"/>
                    <xsl:with-param name="MeasureMark" select="$Current-MeasureMark"/>
                    <xsl:with-param name="Target-Measure" select="$Current-MeasureMark"/>
                    <xsl:with-param name="group-value" select="$group-left"/>
                    <xsl:with-param name="coord-value" select="$coord-left"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:call-template name="get-number">
                <xsl:with-param name="value" select="$adjusted-top"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="width-value">
            <xsl:call-template name="get-number">
                <xsl:with-param name="value" select="$width"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="height-value">
            <xsl:call-template name="get-number">
                <xsl:with-param name="value" select="$height"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="Current-x-scale" select="( $Current-coord-width div $width-value ) * $x-scale"/>
        <xsl:variable name="Current-y-scale" select="( $Current-coord-height div $height-value ) * $x-scale"/>
        <xsl:element name="draw:g">
            <xsl:apply-templates select="w:r/w:pict | v:*">
                <xsl:with-param name="x-scale" select="$Current-x-scale"/>
                <xsl:with-param name="y-scale" select="$Current-y-scale"/>
                <xsl:with-param name="MeasureMark" select="$Current-MeasureMark"/>
                <xsl:with-param name="group-left" select="$left-value"/>
                <xsl:with-param name="group-top" select="$top-value"/>
                <xsl:with-param name="coord-left" select="$Current-coord-left"/>
                <xsl:with-param name="coord-top" select="$Current-coord-top"/>
            </xsl:apply-templates>
        </xsl:element>
    </xsl:template>
    <xsl:template match="v:*">
        <xsl:param name="x-scale" select="1"/>
        <xsl:param name="y-scale" select="1"/>
        <xsl:param name="MeasureMark"/>
        <xsl:param name="group-left" select="0"/>
        <xsl:param name="group-top" select="0"/>
        <xsl:param name="coord-left" select="0"/>
        <xsl:param name="coord-top" select="0"/>
        <xsl:if test="not (name() = 'v:shapetype' )">
            <xsl:call-template name="DrawElements">
                <xsl:with-param name="x-scale" select="$x-scale"/>
                <xsl:with-param name="y-scale" select="$x-scale"/>
                <xsl:with-param name="MeasureMark" select="$MeasureMark"/>
                <xsl:with-param name="group-left" select="$group-left"/>
                <xsl:with-param name="group-top" select="$group-top"/>
                <xsl:with-param name="coord-left" select="$coord-left"/>
                <xsl:with-param name="coord-top" select="$coord-top"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>
    <xsl:template name="DrawElements">
        <xsl:param name="x-scale" select="1"/>
        <xsl:param name="y-scale" select="1"/>
        <xsl:param name="MeasureMark"/>
        <xsl:param name="group-left" select="0"/>
        <xsl:param name="group-top" select="0"/>
        <xsl:param name="coord-left" select="0"/>
        <xsl:param name="coord-top" select="0"/>
        <xsl:param name="force-draw" select="'false'"/>
        <xsl:param name="shape-type"/>
        <xsl:variable name="wordshapename" select="substring-after(name(),':')"/>
        <xsl:variable name="custom_shapename">
            <xsl:if test="$wordshapename='shape' and not (v:imagedata) and @type">
                <xsl:call-template name="ms_word_draw_map2ooo_custom_draw">
                    <xsl:with-param name="ms_word_draw_type" select="@type"/>
                </xsl:call-template>
            </xsl:if>
        </xsl:variable>
        <xsl:variable name="is-image" select="$wordshapename='shape' and v:imagedata"/>
        <xsl:variable name="element-name">
            <xsl:choose>
                <xsl:when test="$wordshapename='line'">draw:line</xsl:when>
                <xsl:when test="$wordshapename='rect'">draw:rect</xsl:when>
                <xsl:when test="$wordshapename='oval'">draw:ellipse</xsl:when>
                <xsl:when test="$wordshapename='arc'">draw:ellipse</xsl:when>
                <xsl:when test="$wordshapename='polyline'">draw:polyline</xsl:when>
                <xsl:when test="$wordshapename='shape' and v:imagedata">draw:frame</xsl:when>
                <xsl:when test="$wordshapename='shape' and not (v:imagedata) and @type">
                    <xsl:choose>
                        <xsl:when test="string-length($custom_shapename) &gt; 0">draw:custom-shape</xsl:when>
                        <xsl:otherwise>draw:path</xsl:otherwise>
                        <!--if nothing match it, we prefer path-->
                    </xsl:choose>
                </xsl:when>
                <!--changed here!-->
                <xsl:otherwise>draw:path</xsl:otherwise>
                <!--
				 <xsl:when test="$ooshapename='ellipse' and not(string-length(@draw:kind) &gt; 0)">v:oval</xsl:when>
				 <xsl:when test="$ooshapename='ellipse' and string-length(@draw:kind) &gt; 0">v:arc</xsl:when>
				 <xsl:when test="$ooshapename='circle' and string-length(@draw:kind) &gt; 0">v:arc</xsl:when>
				 <xsl:when test="$ooshapename='polyline'">v:polyline</xsl:when>
				 <xsl:when test="$ooshapename='text-box'">v:shape</xsl:when>
				 <xsl:when test="$ooshapename='image'">v:shape</xsl:when>
				 <xsl:when test="$ooshapename='path'">v:shape</xsl:when>
				    *This caption is not the "Caption", it's GUI name is Callouts*
				 <xsl:when test="$ooshapename='caption'">v:shape</xsl:when>-->
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="wfill" select="@fill"/>
        <xsl:variable name="draw-kind">
            <xsl:if test="$wordshapename='arc' and string-length($wfill) &gt; 0">arc</xsl:if>
            <!--Means this is a Segment of Circle-->
        </xsl:variable>
        <!--Get the position,left,top,width,height,z-index,flip from Style-->
        <xsl:variable name="style" select="concat(@style, ';')"/>
        <xsl:variable name="position" select="substring-before(substring-after($style,'position:'),';')"/>
        <xsl:variable name="left">
            <xsl:variable name="direct-left" select="substring-before(substring-after($style,';left:'),';')"/>
            <xsl:variable name="margin-left" select="substring-before( substring-after($style,'margin-left:')  ,';')"/>
            <xsl:call-template name="Add-with-Measure">
                <xsl:with-param name="value1" select="$margin-left"/>
                <xsl:with-param name="value2" select="$direct-left"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="top">
            <xsl:variable name="direct-top" select="substring-before(substring-after($style,';top:'),';')"/>
            <xsl:variable name="margin-top" select="substring-before( substring-after($style,'margin-top:')  ,';')"/>
            <xsl:call-template name="Add-with-Measure">
                <xsl:with-param name="value1" select="$margin-top"/>
                <xsl:with-param name="value2" select="$direct-top"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="width" select="substring-before( substring-after($style,'width:') ,';')"/>
        <xsl:variable name="height" select="substring-before(substring-after($style,'height:'),';')"/>
        <xsl:variable name="flip" select="substring-before(substring-after($style,'flip:'),';')"/>
        <xsl:variable name="z-index" select="substring-before(substring-after($style,'z-index:'),';')"/>
        <!--these are used for wrap margins get from style-->
        <xsl:variable name="mso-wrap-distance-lefttmp" select="substring-after($style,'mso-wrap-distance-left:')"/>
        <xsl:variable name="mso-wrap-distance-left" select="substring-before($mso-wrap-distance-lefttmp,';')"/>
        <xsl:variable name="mso-wrap-distance-toptmp" select="substring-after($style,'mso-wrap-distance-top:')"/>
        <xsl:variable name="mso-wrap-distance-top" select="substring-before($mso-wrap-distance-toptmp,';')"/>
        <xsl:variable name="mso-wrap-distance-righttmp" select="substring-after($style,'mso-wrap-distance-right:')"/>
        <xsl:variable name="mso-wrap-distance-right" select="substring-before($mso-wrap-distance-righttmp,';')"/>
        <xsl:variable name="mso-wrap-distance-bottomtmp" select="substring-after($style,'mso-wrap-distance-bottom:')"/>
        <xsl:variable name="mso-wrap-distance-bottom" select="substring-before($mso-wrap-distance-bottomtmp,';')"/>
        <xsl:variable name="anchor-type">
            <xsl:choose>
                <!--Notice that anchor-type=page have not been mapped so decide it later /*NOTICE*/-->
                <xsl:when test="$position='absolute'">paragraph</xsl:when>
                <xsl:otherwise>as-char</xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="text-style-name">
            <xsl:choose>
                <xsl:when test="descendant::v:textbox">P1</xsl:when>
                <!--HeHe,Here what to do???-->
                <xsl:otherwise>P1</xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:element name="{$element-name}">
            <xsl:if test="$element-name ='draw:path'">
                <xsl:attribute name="svg:d">M 0,0 L 0,0</xsl:attribute>
                <xsl:attribute name="svg:viewBox">0 0 1000 1000</xsl:attribute>
            </xsl:if>
            <!--{$element-name}"-->
            <xsl:attribute name="text:anchor-type">
                <xsl:value-of select="$anchor-type"/>
                <!--This need to be checkout and built!-->
            </xsl:attribute>
            <xsl:if test="string-length($z-index) &gt; 0">
                <xsl:if test="$z-index &lt; 0">
                    <xsl:message>
                                        We meet a negative z-index:<xsl:value-of select="$z-index"/>. How can I deal with it?
                                    </xsl:message>
                    <xsl:attribute name="draw:z-index">
                        <xsl:value-of select="'0'"/>
                    </xsl:attribute>
                </xsl:if>
                <xsl:if test="not($z-index &lt; 0)">
                    <xsl:attribute name="draw:z-index">
                        <xsl:value-of select="$z-index"/>
                    </xsl:attribute>
                </xsl:if>
            </xsl:if>
            <xsl:attribute name="draw:style-name">Tgr<xsl:number from="/w:wordDocument/w:body" level="any" count="w:pict" format="1"/>
            </xsl:attribute>
            <xsl:attribute name="draw:text-style-name">
                <xsl:value-of select="$text-style-name"/>
                <!--This is difficult!!-->
            </xsl:attribute>
            <xsl:if test="$wordshapename='line'">
                <xsl:variable name="fromx" select="substring-before(@from,',')"/>
                <xsl:variable name="fromy" select="substring-after(@from,',')"/>
                <xsl:variable name="tox" select="substring-before(@to,',')"/>
                <xsl:variable name="toy" select="substring-after(@to,',')"/>
                <xsl:variable name="valfromx"> </xsl:variable>
                <xsl:if test="$anchor-type='as-char'">
                    <xsl:attribute name="svg:x1">
                        <xsl:call-template name="convert-with-scale-and-measure">
                            <xsl:with-param name="value" select="$fromx"/>
                            <xsl:with-param name="scale" select="$x-scale"/>
                            <xsl:with-param name="MeasureMark" select="$MeasureMark"/>
                            <xsl:with-param name="group-value" select="$group-left"/>
                            <xsl:with-param name="coord-value" select="$coord-left"/>
                        </xsl:call-template>
                    </xsl:attribute>
                    <xsl:attribute name="svg:y1">
                        <xsl:call-template name="convert-with-scale-and-measure">
                            <xsl:with-param name="value" select="$fromy"/>
                            <xsl:with-param name="scale" select="$y-scale"/>
                            <xsl:with-param name="MeasureMark" select="$MeasureMark"/>
                            <xsl:with-param name="group-value" select="$group-top"/>
                            <xsl:with-param name="coord-value" select="$coord-top"/>
                        </xsl:call-template>
                    </xsl:attribute>
                    <xsl:attribute name="svg:x2">
                        <xsl:call-template name="convert-with-scale-and-measure">
                            <xsl:with-param name="value" select="$tox"/>
                            <xsl:with-param name="scale" select="$x-scale"/>
                            <xsl:with-param name="MeasureMark" select="$MeasureMark"/>
                            <xsl:with-param name="group-value" select="$group-left"/>
                            <xsl:with-param name="coord-value" select="$coord-left"/>
                        </xsl:call-template>
                    </xsl:attribute>
                    <xsl:attribute name="svg:y2">
                        <xsl:call-template name="convert-with-scale-and-measure">
                            <xsl:with-param name="value" select="$toy"/>
                            <xsl:with-param name="scale" select="$y-scale"/>
                            <xsl:with-param name="MeasureMark" select="$MeasureMark"/>
                            <xsl:with-param name="group-value" select="$group-top"/>
                            <xsl:with-param name="coord-value" select="$coord-top"/>
                        </xsl:call-template>
                    </xsl:attribute>
                </xsl:if>
                <xsl:if test="not ($anchor-type='as-char')">
                    <xsl:attribute name="svg:x1">
                        <xsl:call-template name="convert-with-scale-and-measure">
                            <xsl:with-param name="value" select="$fromx"/>
                            <xsl:with-param name="scale" select="$x-scale"/>
                            <xsl:with-param name="MeasureMark" select="$MeasureMark"/>
                            <xsl:with-param name="group-value" select="$group-left"/>
                            <xsl:with-param name="coord-value" select="$coord-left"/>
                        </xsl:call-template>
                    </xsl:attribute>
                    <xsl:attribute name="svg:y1">
                        <xsl:call-template name="convert-with-scale-and-measure">
                            <xsl:with-param name="value" select="$toy"/>
                            <xsl:with-param name="scale" select="$y-scale"/>
                            <xsl:with-param name="MeasureMark" select="$MeasureMark"/>
                            <xsl:with-param name="group-value" select="$group-top"/>
                            <xsl:with-param name="coord-value" select="$coord-top"/>
                        </xsl:call-template>
                    </xsl:attribute>
                    <xsl:attribute name="svg:x2">
                        <xsl:call-template name="convert-with-scale-and-measure">
                            <xsl:with-param name="value" select="$tox"/>
                            <xsl:with-param name="scale" select="$x-scale"/>
                            <xsl:with-param name="MeasureMark" select="$MeasureMark"/>
                            <xsl:with-param name="group-value" select="$group-left"/>
                            <xsl:with-param name="coord-value" select="$coord-left"/>
                        </xsl:call-template>
                    </xsl:attribute>
                    <xsl:attribute name="svg:y2">
                        <xsl:call-template name="convert-with-scale-and-measure">
                            <xsl:with-param name="value" select="$fromy"/>
                            <xsl:with-param name="scale" select="$y-scale"/>
                            <xsl:with-param name="MeasureMark" select="$MeasureMark"/>
                            <xsl:with-param name="group-value" select="$group-top"/>
                            <xsl:with-param name="coord-value" select="$coord-top"/>
                        </xsl:call-template>
                    </xsl:attribute>
                </xsl:if>
            </xsl:if>
            <xsl:if test="$wordshapename='rect' or $wordshapename='oval'  or $wordshapename='arc' or $wordshapename='shape' or $wordshapename='polyline' ">
                <xsl:if test="$anchor-type='as-char'">
                    <xsl:attribute name="svg:width">
                        <xsl:call-template name="convert-with-scale-and-measure">
                            <xsl:with-param name="value" select="$width"/>
                            <xsl:with-param name="scale" select="$x-scale"/>
                            <xsl:with-param name="MeasureMark" select="$MeasureMark"/>
                        </xsl:call-template>
                    </xsl:attribute>
                    <xsl:attribute name="svg:height">
                        <xsl:call-template name="convert-with-scale-and-measure">
                            <xsl:with-param name="value" select="$height"/>
                            <xsl:with-param name="scale" select="$y-scale"/>
                            <xsl:with-param name="MeasureMark" select="$MeasureMark"/>
                        </xsl:call-template>
                    </xsl:attribute>
                    <xsl:attribute name="svg:x">
                        <xsl:call-template name="convert-with-scale-and-measure">
                            <xsl:with-param name="value" select="$left"/>
                            <xsl:with-param name="scale" select="$x-scale"/>
                            <xsl:with-param name="MeasureMark" select="$MeasureMark"/>
                            <xsl:with-param name="group-value" select="$group-left"/>
                            <xsl:with-param name="coord-value" select="$coord-left"/>
                        </xsl:call-template>
                    </xsl:attribute>
                    <xsl:attribute name="svg:y">
                        <xsl:call-template name="convert-with-scale-and-measure">
                            <xsl:with-param name="value" select="$top"/>
                            <xsl:with-param name="scale" select="$y-scale"/>
                            <xsl:with-param name="MeasureMark" select="$MeasureMark"/>
                            <xsl:with-param name="group-value" select="$group-top"/>
                            <xsl:with-param name="coord-value" select="$coord-top"/>
                        </xsl:call-template>
                    </xsl:attribute>
                </xsl:if>
                <xsl:if test="not ($anchor-type='as-char')">
                    <!--Something has to be down because We have Margin-top options-->
                    <xsl:attribute name="svg:width">
                        <xsl:call-template name="convert-with-scale-and-measure">
                            <xsl:with-param name="value" select="$width"/>
                            <xsl:with-param name="scale" select="$x-scale"/>
                            <xsl:with-param name="MeasureMark" select="$MeasureMark"/>
                        </xsl:call-template>
                    </xsl:attribute>
                    <xsl:attribute name="svg:height">
                        <xsl:call-template name="convert-with-scale-and-measure">
                            <xsl:with-param name="value" select="$height"/>
                            <xsl:with-param name="scale" select="$y-scale"/>
                            <xsl:with-param name="MeasureMark" select="$MeasureMark"/>
                        </xsl:call-template>
                    </xsl:attribute>
                    <xsl:attribute name="svg:x">
                        <xsl:call-template name="convert-with-scale-and-measure">
                            <xsl:with-param name="value" select="$left"/>
                            <xsl:with-param name="scale" select="$x-scale"/>
                            <xsl:with-param name="MeasureMark" select="$MeasureMark"/>
                            <xsl:with-param name="group-value" select="$group-left"/>
                            <xsl:with-param name="coord-value" select="$coord-left"/>
                        </xsl:call-template>
                    </xsl:attribute>
                    <xsl:attribute name="svg:y">
                        <xsl:call-template name="convert-with-scale-and-measure">
                            <xsl:with-param name="value" select="$top"/>
                            <xsl:with-param name="scale" select="$y-scale"/>
                            <xsl:with-param name="MeasureMark" select="$MeasureMark"/>
                            <xsl:with-param name="group-value" select="$group-top"/>
                            <xsl:with-param name="coord-value" select="$coord-top"/>
                        </xsl:call-template>
                    </xsl:attribute>
                </xsl:if>
            </xsl:if>
            <xsl:if test="$element-name='draw:ellipse' and string-length($draw-kind) &gt; 0">
                <xsl:attribute name="draw:kind">
                    <xsl:value-of select="$draw-kind"/>
                </xsl:attribute>
            </xsl:if>
            <!--<draw:area-polygon …    svg:x="0" svg:y="0" svg:width="2.0cm" svg:height="2.0cm"   svg:viewBox="0 0 2000 2000"   svg:points="400,1500 1600,1500 1000,400"/>
					The element shown in the following example defines a triangle that is located in the middle of a 2cm by 2cm image. The bounding box covers an area of 2cm by 1.5cm. One view box unit corresponds to 0.01mm.-->
            <xsl:if test="$wordshapename='polyline'">
                <xsl:variable name="MeasureMark_Here" select="'cm'"/>
                <!--MeasureMarkHere is cm because One view box unit corresponds to 0.01mm-->
                <xsl:variable name="width_cm">
                    <xsl:call-template name="convert-with-scale-and-measure">
                        <xsl:with-param name="value" select="$width"/>
                        <xsl:with-param name="scale" select="$x-scale"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="height_cm">
                    <xsl:call-template name="convert-with-scale-and-measure">
                        <xsl:with-param name="value" select="$height"/>
                        <xsl:with-param name="scale" select="$x-scale"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="widthval">
                    <xsl:if test="contains($width_cm,'cm')">
                        <xsl:value-of select="round(substring-before($width_cm,'cm')*1000)"/>
                    </xsl:if>
                </xsl:variable>
                <xsl:variable name="heightval">
                    <xsl:if test="contains($height_cm,'cm')">
                        <xsl:value-of select="round(substring-before($height_cm,'cm')*1000)"/>
                    </xsl:if>
                </xsl:variable>
                <xsl:variable name="viewBoxstr" select="concat('0 0 ',$widthval,' ',$heightval)"/>
                <xsl:attribute name="svg:viewBox">
                    <xsl:value-of select="$viewBoxstr"/>
                </xsl:attribute>
                <xsl:variable name="inputx_cm">
                    <xsl:call-template name="convert-with-scale-and-measure">
                        <xsl:with-param name="value" select="$left"/>
                        <xsl:with-param name="scale" select="$x-scale"/>
                        <xsl:with-param name="group-value" select="$group-left"/>
                        <xsl:with-param name="coord-value" select="$coord-left"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="inputy_cm">
                    <xsl:call-template name="convert-with-scale-and-measure">
                        <xsl:with-param name="value" select="$top"/>
                        <xsl:with-param name="scale" select="$y-scale"/>
                        <xsl:with-param name="group-value" select="$group-top"/>
                        <xsl:with-param name="coord-value" select="$coord-top"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="inputx_val">
                    <xsl:choose>
                        <xsl:when test="contains($inputx_cm,'cm')">
                            <xsl:value-of select="substring-before($inputx_cm,'cm')"/>
                        </xsl:when>
                        <xsl:otherwise>0</xsl:otherwise>
                    </xsl:choose>
                </xsl:variable>
                <xsl:variable name="inputy_val">
                    <xsl:choose>
                        <xsl:when test="contains($inputy_cm,'cm')">
                            <xsl:value-of select="substring-before($inputy_cm,'cm')"/>
                        </xsl:when>
                        <xsl:otherwise>0</xsl:otherwise>
                    </xsl:choose>
                </xsl:variable>
                <xsl:variable name="inputboxwidth" select="$widthval"/>
                <xsl:variable name="oopoints">
                    <xsl:call-template name="wordpoints2oopoints">
                        <xsl:with-param name="input_x" select="$inputx_val"/>
                        <xsl:with-param name="input_y" select="$inputy_val"/>
                        <xsl:with-param name="input_width" select="$width"/>
                        <xsl:with-param name="input_height" select="$height"/>
                        <xsl:with-param name="input_boxwidth" select="$widthval"/>
                        <xsl:with-param name="input_boxheight" select="$heightval"/>
                        <xsl:with-param name="input_points" select="concat(@points,',')"/>
                        <!-- add a space to the end of input_points -->
                    </xsl:call-template>
                </xsl:variable>
                <xsl:if test="string-length($oopoints &gt; 0 )">
                    <xsl:attribute name="draw:points">
                        <xsl:value-of select="$oopoints"/>
                    </xsl:attribute>
                </xsl:if>
            </xsl:if>
            <xsl:if test="string-length($custom_shapename) &gt; 0">
                <xsl:element name="draw:enhanced-geometry">
                    <xsl:attribute name="draw:type">
                        <xsl:value-of select="$custom_shapename"/>
                    </xsl:attribute>
                    <xsl:if test="@adj">
                        <xsl:variable name="adjust-x" select="substring-before(@adj,',')"/>
                        <xsl:variable name="adjust-y" select="substring-after(@adj,',')"/>
                        <xsl:variable name="adjuststr">
                            <xsl:if test="$adjust-x and $adjust-y">
                                <xsl:value-of select="concat( $adjust-x div 20 , ',' ,$adjust-y div 20)"/>
                            </xsl:if>
                        </xsl:variable>
                        <xsl:attribute name="draw:modifiers">
                            <xsl:value-of select="$adjuststr"/>
                        </xsl:attribute>
                    </xsl:if>
                    <!--draw:adjustments="2557.41056218058,9249.91482112436"/>-->
                </xsl:element>
            </xsl:if>
            <xsl:if test="$is-image">
                <xsl:variable name="the-image" select="key('imagedata',v:imagedata/@src)"/>
                <xsl:if test="v:imagedata/@o:title">
                    <xsl:attribute name="draw:name">
                        <xsl:value-of select="v:imagedata/@o:title"/>
                    </xsl:attribute>
                </xsl:if>
                <draw:image>
                    <xsl:element name="office:binary-data">
                        <xsl:value-of select="translate($the-image/text(),'&#9;&#10;&#13;&#32;','' ) "/>
                    </xsl:element>
                </draw:image>
            </xsl:if>
        </xsl:element>
    </xsl:template>
    <!--this template map word's points to svg:viewbox's point they are quite differect because word's use pt but svg's use 0.001cm as a unit-->
    <xsl:template name="wordpoints2oopoints">
        <xsl:param name="input_points"/>
        <xsl:param name="input_x"/>
        <xsl:param name="input_y"/>
        <xsl:param name="input_width"/>
        <xsl:param name="input_height"/>
        <xsl:param name="input_boxwidth"/>
        <xsl:param name="input_boxheight"/>
        <xsl:variable name="ptx" select="substring-before($input_points,',')"/>
        <xsl:variable name="tempstr" select="substring-after($input_points,',')"/>
        <xsl:variable name="pty" select="substring-before($tempstr,',')"/>
        <xsl:variable name="nextinput" select="substring-after ($tempstr,',')"/>
        <xsl:if test="$ptx and $pty">
            <xsl:variable name="val_ptx">
                <xsl:call-template name="ConvertMeasure">
                    <xsl:with-param name="value" select="$ptx"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="val_pty">
                <xsl:call-template name="ConvertMeasure">
                    <xsl:with-param name="value" select="$pty"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:value-of select="round( $input_boxwidth  -  ( $val_ptx - $input_x ) * 1000 )"/>
            <xsl:value-of select="','"/>
            <xsl:value-of select="round( ( $val_pty  -  $input_y ) * 1000 )"/>
        </xsl:if>
        <xsl:if test="contains($nextinput,',')">
            <xsl:value-of select="' '"/>
            <!--Leave a blank first as mark of points group-->
            <xsl:call-template name="wordpoints2oopoints">
                <xsl:with-param name="input_points" select="$nextinput"/>
                <xsl:with-param name="input_x" select="$input_x"/>
                <xsl:with-param name="input_y" select="$input_y"/>
                <xsl:with-param name="input_width" select="$input_width"/>
                <xsl:with-param name="input_height" select="$input_height"/>
                <xsl:with-param name="input_boxwidth" select="$input_boxwidth"/>
                <xsl:with-param name="input_boxheight" select="$input_boxheight"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>
    <!--template MapConstColor:input is a color in form of const e.g 'red' or number e.g '#ff0010' output is a number color-->
    <xsl:template name="MapConstColor">
        <xsl:param name="color"/>
        <xsl:choose>
            <xsl:when test="$color='black'">#000000</xsl:when>
            <xsl:when test="$color='olive'">#808000</xsl:when>
            <xsl:when test="$color='red'">#ff0000</xsl:when>
            <xsl:when test="$color='teal'">#008080</xsl:when>
            <xsl:when test="$color='green'">#00ff00</xsl:when>
            <xsl:when test="$color='gray'">#808080</xsl:when>
            <xsl:when test="$color='blue'">#0000ff</xsl:when>
            <xsl:when test="$color='navy'">#000080</xsl:when>
            <xsl:when test="$color='white'">#ffffff</xsl:when>
            <xsl:when test="$color='lime'">#00ff00</xsl:when>
            <xsl:when test="$color='yellow'">#ffff00</xsl:when>
            <xsl:when test="$color='fuchsia'">#ff00ff</xsl:when>
            <xsl:when test="$color='purple'">#800080</xsl:when>
            <xsl:when test="$color='aqua'">#00ffff</xsl:when>
            <xsl:when test="$color='maroon'">#800000</xsl:when>
            <xsl:when test="$color='silver'">#c0c0c0</xsl:when>
            <xsl:otherwise>
                <xsl:choose>
                    <xsl:when test="string-length($color) =7">
                        <xsl:value-of select="$color"/>
                    </xsl:when>
                    <xsl:when test="string-length($color) =4">
                        <!--short form representation of color-->
                        <xsl:variable name="valr">
                            <xsl:value-of select="concat(substring($color,2,1),substring($color,2,1))"/>
                            <!--<xsl:call-template name="shortcolorconv"><xsl:with-param name="value" select="substring($color,2,1)"/></xsl:call-template>-->
                        </xsl:variable>
                        <xsl:variable name="valg" select="concat(substring($color,3,1),substring($color,3,1))"/>
                        <xsl:variable name="valb" select="concat(substring($color,4,1),substring($color,4,1))"/>
                        <xsl:value-of select="concat('#',$valr,$valg,$valb)"/>
                    </xsl:when>
                    <xsl:otherwise>#000000</xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
        <!--						
              <xsl:when test="$color='black'">#000000</xsl:when>
            <xsl:when test="$color='yellow'">#ffff00</xsl:when>
            <xsl:when test="$color='green'">#00ff00</xsl:when>
            <xsl:when test="$color='cyan'">#00ffff</xsl:when>
            <xsl:when test="$color='magenta'">#ff00ff</xsl:when>
            <xsl:when test="$color='blue'">#0000ff</xsl:when>
            <xsl:when test="$color='red'">#ff0000</xsl:when>
            <xsl:when test="$color='dark-blue'">#000080</xsl:when>
            <xsl:when test="$color='dark-cyan'">#008080</xsl:when>
            <xsl:when test="$color='dark-green'">#008000</xsl:when>
            <xsl:when test="$color='dark-magenta'">#800080</xsl:when>
            <xsl:when test="$color='dark-red'">#800000</xsl:when>
            <xsl:when test="$color='dark-yellow'">#808000</xsl:when>
            <xsl:when test="$color='dark-gray'">#808080</xsl:when>
            <xsl:when test="$color='light-gray'">#c0c0c0</xsl:when>-->
    </xsl:template>
    <xsl:template name="shortcolorconv">
        <xsl:param name="value"/>
        <xsl:choose>
            <xsl:when test="$value='1'">11</xsl:when>
            <xsl:when test="$value='2'">22</xsl:when>
            <xsl:when test="$value='3'">33</xsl:when>
            <xsl:when test="$value='4'">44</xsl:when>
            <xsl:when test="$value='5'">55</xsl:when>
            <xsl:when test="$value='6'">66</xsl:when>
            <xsl:when test="$value='7'">77</xsl:when>
            <xsl:when test="$value='8'">88</xsl:when>
            <xsl:when test="$value='9'">99</xsl:when>
            <xsl:when test="$value='a'">aa</xsl:when>
            <xsl:when test="$value='b'">bb</xsl:when>
            <xsl:when test="$value='c'">cc</xsl:when>
            <xsl:when test="$value='d'">dd</xsl:when>
            <xsl:when test="$value='e'">ee</xsl:when>
            <xsl:when test="$value='f'">ff</xsl:when>
            <!--I just guess it, maybe it is not right-->
        </xsl:choose>
    </xsl:template>
    <xsl:template name="MapArrowStyle">
        <!--What is a block?normal arrow?-->
        <xsl:param name="arrow-name"/>
        <xsl:choose>
            <xsl:when test="$arrow-name = 'Block' ">Arrow</xsl:when>
            <xsl:when test="$arrow-name = 'Diamond' ">Square</xsl:when>
            <xsl:when test="$arrow-name = 'Open' ">Line Arrow</xsl:when>
            <xsl:when test="$arrow-name = 'Oval' ">Circle</xsl:when>
            <xsl:when test="$arrow-name = 'Diamond' ">Square 45</xsl:when>
            <xsl:when test="$arrow-name = 'Classic' ">Arrow concave</xsl:when>
            <xsl:otherwise>Arrow</xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="Add-with-Measure">
        <xsl:param name="value1"/>
        <xsl:param name="value2"/>
        <xsl:variable name="Current-MeasureMark">
            <xsl:choose>
                <xsl:when test="string-length(translate($value1 ,'-.0123456789 ','' )) &gt; 0">
                    <xsl:value-of select="translate($value1 ,'-.0123456789 ','' )"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="translate($value2 ,'-.0123456789 ','' )"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="number-value1">
            <xsl:call-template name="get-number">
                <xsl:with-param name="value" select="$value1"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="number-value2">
            <xsl:call-template name="get-number">
                <xsl:with-param name="value" select="$value2"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:value-of select="concat( $number-value1 + $number-value2 , $Current-MeasureMark)"/>
    </xsl:template>
    <xsl:template name="convert-with-scale-and-measure">
        <xsl:param name="value"/>
        <xsl:param name="group-value" select="0"/>
        <xsl:param name="coord-value" select="0"/>
        <xsl:param name="scale" select="1"/>
        <xsl:param name="MeasureMark" select="''"/>
        <xsl:param name="Target-Measure" select="''"/>
        <xsl:variable name="Current-MeasureMark">
            <xsl:choose>
                <xsl:when test="string-length($MeasureMark) &gt; 0">
                    <xsl:value-of select="$MeasureMark"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="translate($value ,'-.0123456789 ','' )  "/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="number-value">
            <xsl:call-template name="get-number">
                <xsl:with-param name="value" select="$value"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="value-string" select="( $number-value - $coord-value)  div $scale + $group-value"/>
        <xsl:choose>
            <xsl:when test="$value-string = 0">0cm</xsl:when>
            <xsl:when test="$Target-Measure = $Current-MeasureMark">
                <xsl:value-of select="concat($value-string , $Current-MeasureMark)"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:call-template name="ConvertMeasure">
                    <xsl:with-param name="value" select="concat($value-string , $Current-MeasureMark)"/>
                </xsl:call-template>
                <xsl:value-of select=" 'cm' "/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="get-number">
        <xsl:param name="value"/>
        <xsl:choose>
            <xsl:when test="translate($value,'abcdefghijklmnopqrstuvwxyz','') = '' ">0</xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="number(translate($value,'abcdefghijklmnopqrstuvwxyz',''))"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
</xsl:stylesheet>
