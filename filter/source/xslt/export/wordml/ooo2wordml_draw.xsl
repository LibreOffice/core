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
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:w="http://schemas.microsoft.com/office/word/2003/wordml" xmlns:wx="http://schemas.microsoft.com/office/word/2003/auxHint" xmlns:w10="urn:schemas-microsoft-com:office:word" xmlns:o="urn:schemas-microsoft-com:office:office" xmlns:aml="http://schemas.microsoft.com/aml/2001/core" xmlns:dt="uuid:C2F41010-65B3-11d1-A29F-00AA00C14882" xmlns:v="urn:schemas-microsoft-com:vml" xmlns:office="urn:oasis:names:tc:openoffice:xmlns:office:1.0" xmlns:style="urn:oasis:names:tc:openoffice:xmlns:style:1.0" xmlns:text="urn:oasis:names:tc:openoffice:xmlns:text:1.0" xmlns:table="urn:oasis:names:tc:openoffice:xmlns:table:1.0" xmlns:draw="urn:oasis:names:tc:openoffice:xmlns:drawing:1.0" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:meta="urn:oasis:names:tc:openoffice:xmlns:meta:1.0" xmlns:number="urn:oasis:names:tc:openoffice:xmlns:datastyle:1.0" xmlns:svg="http://www.w3.org/2000/svg" xmlns:chart="urn:oasis:names:tc:openoffice:xmlns:chart:1.0" xmlns:dr3d="urn:oasis:names:tc:openoffice:xmlns:dr3d:1.0" xmlns:math="http://www.w3.org/1998/Math/MathML" xmlns:form="urn:oasis:names:tc:openoffice:xmlns:form:1.0" xmlns:script="urn:oasis:names:tc:openoffice:xmlns:script:1.0" xmlns:config="urn:oasis:names:tc:openoffice:xmlns:config:1.0" xmlns:ooo="http://openoffice.org/2004/office" xmlns:ooow="http://openoffice.org/2004/writer" xmlns:oooc="http://openoffice.org/2004/calc" xmlns:dom="http://www.w3.org/2001/xml-events" exclude-result-prefixes="office table style text draw svg  fo dc config xlink meta oooc dom ooo chart math dr3d form script ooow draw">
    <xsl:include href="ooo2wordml_custom_draw.xsl"/>
    <xsl:key name="stroke-dash-style" match="draw:stroke-dash" use="@draw:name"/>
    <xsl:key name="fill-image" match="draw:fill-image" use="@draw:name"/>
    <xsl:key name="draw-gradient" match="draw:gradient " use="@draw:name"/>
    <xsl:template name="PageLevelGraphic">
        <xsl:for-each select="//draw:*[@text:anchor-type='page']">
            <xsl:apply-templates select="."/>
        </xsl:for-each>
    </xsl:template>
    <xsl:template match="draw:*">
        <xsl:param name="TargetMeasure" select="'pt'"/>
        <xsl:param name="x-adjust" select="0"/>
        <xsl:param name="y-adjust" select="0"/>
        <xsl:param name="force-draw" select="'false'"/>
        <xsl:variable name="MeasureMark">
            <xsl:choose>
                <xsl:when test="$TargetMeasure = 'twip'"/>
                <xsl:otherwise>
                    <xsl:value-of select="$TargetMeasure"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:choose>
            <!--
                          deal with captions and frames first. draw:text-box is a powerfull element in OOo, its GUI name is frame. And OOo use it to contain Captions
                          Since there is not a corresponding object in word, so we draw the text-box itself and its children separately. If it look like a Caption only frame
                          we'll adjust the text-box position to make it look pretty
             -->
            <!-- skip all not force draw      children , must be first case -->
            <xsl:when test="ancestor::draw:text-box and $force-draw='false' "/>
            <xsl:when test="name() = 'draw:text-box'">
                <!-- draw the text-box itself -->
                <w:r>
                    <w:pict>
                        <xsl:variable name="text-y-adjust">
                            <xsl:choose>
                                <xsl:when test="count(text:p/draw:*) = 1 and (string-length(text:p/draw:*[position()=1]/@svg:x) =0 or number(concat('0',translate(text:p/draw:*[position()=1]/@svg:x,'abcdefghijklmnopqrstuvwxyz','') ))=0) and  (string-length(text:p/draw:*[position()=1]/@svg:y)=0 or  number(concat('0',translate(text:p/draw:*[position()=1]/@svg:x,'abcdefghijklmnopqrstuvwxyz','') ))=0 ) ">
                                    <xsl:variable name="pic-height">
                                        <xsl:choose>
                                            <xsl:when test="name(text:p/draw:*[position()=1]) = 'draw:g' or name(text:p/draw:*[position()=1]) = 'draw:a'">
                                                <xsl:variable name="BigestWindow">
                                                    <xsl:call-template name="GetWindowSize">
                                                        <xsl:with-param name="nodeSet" select="text:p/draw:*[position()=1]/draw:*"/>
                                                        <xsl:with-param name="x-adjust" select="$x-adjust"/>
                                                        <xsl:with-param name="y-adjust" select="$y-adjust"/>
                                                    </xsl:call-template>
                                                </xsl:variable>
                                                <xsl:value-of select="substring-after($BigestWindow,'y2:')  - substring-after(substring-before($BigestWindow,';x2'), 'y1:') + number(concat('0',translate($y-adjust,'abcdefghijklmnopqrstuvwxyz','')))"/>
                                            </xsl:when>
                                            <xsl:when test="text:p/draw:*[position()=1]/@svg:height">
                                                <xsl:value-of select="number(concat('0',translate($y-adjust,'abcdefghijklmnopqrstuvwxyz','') ))+  number(concat('0',translate(text:p/draw:*[position()=1]/@svg:height,'abcdefghijklmnopqrstuvwxyz','') ))"/>
                                            </xsl:when>
                                            <xsl:when test="text:p/draw:*[position()=1]/@fo:min-height">
                                                <xsl:value-of select="number(concat('0',translate($y-adjust,'abcdefghijklmnopqrstuvwxyz','') ))+ number(concat('0',translate(text:p/draw:*[position()=1]/@fo:min-height,'abcdefghijklmnopqrstuvwxyz','')))"/>
                                            </xsl:when>
                                            <xsl:when test="ancestor::draw:frame">
                                                <xsl:value-of select="number(concat('0',translate($y-adjust,'abcdefghijklmnopqrstuvwxyz','') ))+ number(concat('0',translate(ancestor::draw:frame/@svg:height,'abcdefghijklmnopqrstuvwxyz','')))"/>
                                            </xsl:when>
                                        </xsl:choose>
                                    </xsl:variable>
                                    <xsl:choose>
                                        <xsl:when test="number(concat('0',translate(@fo:min-height,'abcdefghijklmnopqrstuvwxyz','') ))- $pic-height &lt; 0.001">
                                            <!-- If control goes here, it much like that this text-box is used for containt graphic caption only -->
                                            <xsl:value-of select="$pic-height - 0.1"/>
                                        </xsl:when>
                                        <xsl:otherwise>
                                            <xsl:value-of select="'0'"/>
                                        </xsl:otherwise>
                                    </xsl:choose>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:value-of select="'0'"/>
                                </xsl:otherwise>
                            </xsl:choose>
                        </xsl:variable>
                        <xsl:variable name="shape-type">
                            <xsl:choose>
                                <xsl:when test="$text-y-adjust = 0">
                                    <xsl:value-of select="'#_x0000_t202'"/>
                                </xsl:when>
                            </xsl:choose>
                        </xsl:variable>
                        <xsl:call-template name="DrawElements">
                            <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                            <xsl:with-param name="x-adjust" select="$x-adjust"/>
                            <xsl:with-param name="y-adjust" select="$text-y-adjust"/>
                            <xsl:with-param name="force-draw" select="'true'"/>
                            <xsl:with-param name="shape-type" select="$shape-type"/>
                        </xsl:call-template>
                    </w:pict>
                </w:r>
                <!-- draw the real object first -->
                <xsl:if test="./text:p/draw:*">
                    <xsl:apply-templates select="./text:p/draw:*">
                        <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                        <xsl:with-param name="x-adjust" select="number(concat('0',translate(@svg:x,'abcdefghijklmnopqrstuvwxyz','') ))+ number(concat('0',translate($x-adjust,'abcdefghijklmnopqrstuvwxyz','') ))"/>
                        <xsl:with-param name="y-adjust" select="number(concat('0',translate(@svg:y,'abcdefghijklmnopqrstuvwxyz','') ))+ number(concat('0',translate($y-adjust,'abcdefghijklmnopqrstuvwxyz','') ))"/>
                        <xsl:with-param name="force-draw" select="'true'"/>
                    </xsl:apply-templates>
                </xsl:if>
            </xsl:when>
            <!-- end deal with captions and frames -->
            <xsl:when test=" name() = 'draw:frame' ">
                <xsl:variable name="BigestWindow">
                    <xsl:call-template name="GetWindowSize">
                        <xsl:with-param name="nodeSet" select="."/>
                        <xsl:with-param name="x-adjust" select="$x-adjust"/>
                        <xsl:with-param name="y-adjust" select="$y-adjust"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:apply-templates select="draw:* ">
                    <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                    <xsl:with-param name="x-adjust" select="concat(substring-after(substring-before($BigestWindow,';y1'), 'x1:'), 'in')"/>
                    <xsl:with-param name="y-adjust" select="concat(substring-after(substring-before($BigestWindow,';x2'), 'y1:') , 'in')"/>
                    <xsl:with-param name="force-draw" select="$force-draw"/>
                </xsl:apply-templates>
            </xsl:when>
            <xsl:when test="name() = 'draw:g'">
                <w:r>
                    <w:pict>
                        <xsl:element name="v:group">
                            <xsl:variable name="BigestWindow">
                                <xsl:choose>
                                    <xsl:when test="name() = 'draw:g'">
                                        <xsl:call-template name="GetWindowSize">
                                            <xsl:with-param name="nodeSet" select="draw:*"/>
                                            <xsl:with-param name="x-adjust" select="$x-adjust"/>
                                            <xsl:with-param name="y-adjust" select="$y-adjust"/>
                                        </xsl:call-template>
                                    </xsl:when>
                                    <xsl:when test="name() = 'draw:frame'">
                                        <xsl:call-template name="GetWindowSize">
                                            <xsl:with-param name="nodeSet" select="."/>
                                            <xsl:with-param name="x-adjust" select="$x-adjust"/>
                                            <xsl:with-param name="y-adjust" select="$y-adjust"/>
                                        </xsl:call-template>
                                    </xsl:when>
                                </xsl:choose>
                            </xsl:variable>
                            <xsl:variable name="x">
                                <xsl:call-template name="ConvertMeasure">
                                    <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                                    <xsl:with-param name="value" select="concat(substring-after(substring-before($BigestWindow,';y1'), 'x1:'), 'in')"/>
                                </xsl:call-template>
                            </xsl:variable>
                            <xsl:variable name="y">
                                <xsl:call-template name="ConvertMeasure">
                                    <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                                    <xsl:with-param name="value" select="concat(substring-after(substring-before($BigestWindow,';x2'), 'y1:') , 'in')"/>
                                </xsl:call-template>
                            </xsl:variable>
                            <xsl:variable name="width">
                                <xsl:call-template name="ConvertMeasure">
                                    <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                                    <xsl:with-param name="value" select="concat(substring-after(substring-before($BigestWindow,';y2'), 'x2:')  -  substring-after(substring-before($BigestWindow,';y1'), 'x1:') , 'in')"/>
                                </xsl:call-template>
                            </xsl:variable>
                            <xsl:variable name="height">
                                <xsl:call-template name="ConvertMeasure">
                                    <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                                    <xsl:with-param name="value" select="concat(substring-after($BigestWindow,'y2:')  - substring-after(substring-before($BigestWindow,';x2'), 'y1:'), 'in')"/>
                                </xsl:call-template>
                            </xsl:variable>
                            <xsl:attribute name="id">
                                <xsl:value-of select="generate-id()"/>
                            </xsl:attribute>
                            <xsl:variable name="absolute">
                                <xsl:choose>
                                    <xsl:when test="ancestor::draw:a"/>
                                    <xsl:when test="@text:anchor-type = 'as-char' or @text:anchor-type = 'to-char'"/>
                                    <xsl:otherwise>position:absolute</xsl:otherwise>
                                </xsl:choose>
                            </xsl:variable>
                            <xsl:if test="$TargetMeasure= 'pt'">
                                <xsl:attribute name="style">
                                    <xsl:if test="string-length($absolute) &gt; 0">
                                        <xsl:value-of select="concat($absolute, ';')"/>
                                    </xsl:if>
                                    <xsl:value-of select="concat('margin-left:',$x ,$MeasureMark,';margin-top:', $y,$MeasureMark ,';width:', $width ,$MeasureMark , ';height:', $height,$MeasureMark)"/>
                                </xsl:attribute>
                                <xsl:attribute name="coordorigin">
                                    <xsl:choose>
                                        <!-- if we are in a text-box then oo will use comparative positions on us-->
                                        <xsl:when test="name() = 'draw:frame' ">
                                            <xsl:value-of select=" '0 0' "/>
                                        </xsl:when>
                                        <xsl:otherwise>
                                            <xsl:value-of select="concat(round($x * 20), ',' , round($y * 20))"/>
                                        </xsl:otherwise>
                                    </xsl:choose>
                                </xsl:attribute>
                                <xsl:attribute name="coordsize">
                                    <xsl:value-of select="concat(round($width * 20),',', round($height * 20) )"/>
                                </xsl:attribute>
                            </xsl:if>
                            <xsl:if test="$TargetMeasure= 'twip' ">
                                <xsl:attribute name="style">
                                    <xsl:if test="string-length($absolute) &gt; 0">
                                        <xsl:value-of select="concat($absolute, ';')"/>
                                    </xsl:if>
                                    <xsl:value-of select="concat('left:',$x ,$MeasureMark,';top:', $y,$MeasureMark ,';width:', $width ,$MeasureMark , ';height:', $height,$MeasureMark)"/>
                                </xsl:attribute>
                                <xsl:attribute name="coordorigin">
                                    <xsl:choose>
                                        <!-- if we are in a text-box then oo will use comparative positions on us-->
                                        <xsl:when test="name() = 'draw:frame' ">
                                            <xsl:value-of select=" '0 0' "/>
                                        </xsl:when>
                                        <xsl:otherwise>
                                            <xsl:value-of select="concat($x , ',' , $y)"/>
                                        </xsl:otherwise>
                                    </xsl:choose>
                                </xsl:attribute>
                                <xsl:attribute name="coordsize">
                                    <xsl:value-of select="concat($width,',', $height )"/>
                                </xsl:attribute>
                            </xsl:if>
                            <xsl:choose>
                                <!-- if we are in a text-box then oo will use comparative positions on us-->
                                <xsl:when test="ancestor::draw:text-box">
                                    <xsl:apply-templates select="draw:*">
                                        <xsl:with-param name="TargetMeasure" select="'twip'"/>
                                        <xsl:with-param name="x-adjust" select="concat(substring-after(substring-before($BigestWindow,';y1'), 'x1:'), 'in')"/>
                                        <xsl:with-param name="y-adjust" select="concat(substring-after(substring-before($BigestWindow,';x2'), 'y1:') , 'in')"/>
                                        <xsl:with-param name="force-draw" select="$force-draw"/>
                                    </xsl:apply-templates>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:apply-templates select="draw:*">
                                        <xsl:with-param name="TargetMeasure" select="'twip'"/>
                                        <xsl:with-param name="force-draw" select="$force-draw"/>
                                    </xsl:apply-templates>
                                </xsl:otherwise>
                            </xsl:choose>
                        </xsl:element>
                    </w:pict>
                </w:r>
            </xsl:when>
            <xsl:otherwise>
                <xsl:choose>
                    <xsl:when test="name() = 'draw:a'">
                        <xsl:call-template name="export_hyoerlink">
                            <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                            <xsl:with-param name="x-adjust" select="$x-adjust"/>
                            <xsl:with-param name="y-adjust" select="$y-adjust"/>
                            <xsl:with-param name="force-draw" select="$force-draw"/>
                        </xsl:call-template>
                    </xsl:when>
                    <xsl:otherwise>
                        <w:r>
                            <w:pict>
                                <xsl:call-template name="DrawElements">
                                    <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                                    <xsl:with-param name="x-adjust" select="$x-adjust"/>
                                    <xsl:with-param name="y-adjust" select="$y-adjust"/>
                                    <xsl:with-param name="force-draw" select="$force-draw"/>
                                </xsl:call-template>
                            </w:pict>
                        </w:r>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="GetWindowSize">
        <xsl:param name="CurrPos" select="1"/>
        <xsl:param name="nodeSet"/>
        <xsl:param name="x-adjust" select="0"/>
        <xsl:param name="y-adjust" select="0"/>
        <xsl:variable name="CurrNodeWindow">
            <xsl:call-template name="GetNodeWindow">
                <xsl:with-param name="CurrNode" select=" $nodeSet[ $CurrPos ]"/>
                <xsl:with-param name="x-adjust" select="$x-adjust"/>
                <xsl:with-param name="y-adjust" select="$y-adjust"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:choose>
            <!-- if we got to the last node, return it directly, or return the max window of current one and follwing ones -->
            <xsl:when test="$CurrPos = count($nodeSet)">
                <xsl:value-of select="$CurrNodeWindow"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:variable name="CurrentWindow">
                    <xsl:variable name="FollowingWindow">
                        <xsl:call-template name="GetWindowSize">
                            <xsl:with-param name="nodeSet" select="$nodeSet"/>
                            <xsl:with-param name="CurrPos" select="$CurrPos + 1"/>
                            <xsl:with-param name="x-adjust" select="$x-adjust"/>
                            <xsl:with-param name="y-adjust" select="$y-adjust"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:call-template name="GetBigestWindows">
                        <xsl:with-param name="Window1" select="$CurrNodeWindow"/>
                        <xsl:with-param name="Window2" select="$FollowingWindow"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:value-of select="$CurrentWindow"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="GetNodeWindow">
        <xsl:param name="CurrNode"/>
        <xsl:param name="x-adjust" select="0"/>
        <xsl:param name="y-adjust" select="0"/>
        <xsl:choose>
            <xsl:when test="name($CurrNode)='draw:g'">
                <xsl:call-template name="GetWindowSize">
                    <xsl:with-param name="nodeSet" select="$CurrNode/draw:*"/>
                    <xsl:with-param name="x-adjust" select="$x-adjust"/>
                    <xsl:with-param name="y-adjust" select="$y-adjust"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <xsl:variable name="x" select="number(concat('0',translate($CurrNode/@svg:x,'abcdefghijklmnopqrstuvwxyz','') ))+ number(concat('0',translate($x-adjust,'abcdefghijklmnopqrstuvwxyz','')))"/>
                <xsl:variable name="y" select="number(concat('0',translate($CurrNode/@svg:y,'abcdefghijklmnopqrstuvwxyz','') ))+ number(concat('0',translate($y-adjust,'abcdefghijklmnopqrstuvwxyz','')))"/>
                <xsl:variable name="width" select="number(concat('0',translate($CurrNode/@svg:width,'abcdefghijklmnopqrstuvwxyz','')))"/>
                <xsl:variable name="height">
                    <xsl:choose>
                        <xsl:when test="$CurrNode/@svg:height">
                            <xsl:value-of select="number(concat('0',translate($CurrNode/@svg:height,'abcdefghijklmnopqrstuvwxyz','')))"/>
                        </xsl:when>
                        <xsl:when test="$CurrNode/@fo:min-height">
                            <xsl:value-of select="number(concat('0',translate($CurrNode/@fo:min-height,'abcdefghijklmnopqrstuvwxyz','')))"/>
                        </xsl:when>
                    </xsl:choose>
                </xsl:variable>
                <xsl:value-of select="concat('x1:' , $x, ';y1:' , $y, ';x2:' , string($x + $width), ';y2:', string($y + $height) ) "/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="GetBigestWindows">
        <xsl:param name="Window1"/>
        <xsl:param name="Window2"/>
        <xsl:variable name="w1x1" select="substring-after( substring-before($Window1,';y1'),'x1:') "/>
        <xsl:variable name="w2x1" select="substring-after( substring-before($Window2,';y1'),'x1:') "/>
        <xsl:variable name="w1y1" select="substring-after( substring-before($Window1,';x2'),'y1:') "/>
        <xsl:variable name="w2y1" select="substring-after( substring-before($Window2,';x2'),'y1:') "/>
        <xsl:variable name="w1x2" select="substring-after( substring-before($Window1,';y2'),'x2:') "/>
        <xsl:variable name="w2x2" select="substring-after( substring-before($Window2,';y2'),'x2:') "/>
        <xsl:variable name="w1y2" select="substring-after( $Window1,';y2:') "/>
        <xsl:variable name="w2y2" select="substring-after( $Window2,';y2:') "/>
        <xsl:variable name="x1">
            <xsl:choose>
                <xsl:when test="$w1x1 &gt; $w2x1">
                    <xsl:value-of select="$w2x1"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$w1x1"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="y1">
            <xsl:choose>
                <xsl:when test="$w1y1 &gt; $w2y1">
                    <xsl:value-of select="$w2y1"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$w1y1"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="x2">
            <xsl:choose>
                <xsl:when test="$w1x2 &gt; $w2x2">
                    <xsl:value-of select="$w1x2"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$w2x2"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="y2">
            <xsl:choose>
                <xsl:when test="$w1y2 &gt; $w2y2">
                    <xsl:value-of select="$w1y2"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$w2y2"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:value-of select="concat('x1:' , $x1 , ';y1:' , $y1 , ';x2:' , $x2, ';y2:' , $y2)"/>
    </xsl:template>
    <xsl:template name="ConvertMeasure">
        <xsl:param name="TargetMeasure" select="'pt'"/>
        <xsl:param name="value"/>
        <xsl:choose>
            <!-- remove the measure mark, if the value is null, the result should be 0. Must be the first case  -->
            <xsl:when test="string-length(translate($value,'abcdefghijklmnopqrstuvwxyz','')) = 0">0</xsl:when>
            <xsl:when test="$TargetMeasure = 'pt'">
                <xsl:call-template name="convert2pt">
                    <xsl:with-param name="value" select="$value"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:when test="$TargetMeasure = 'twip'">
                <xsl:call-template name="convert2twip">
                    <xsl:with-param name="value" select="$value"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:when test="$TargetMeasure = 'in'">
                <xsl:call-template name="convert2in">
                    <xsl:with-param name="value" select="$value"/>
                </xsl:call-template>
            </xsl:when>
        </xsl:choose>
    </xsl:template>
    <!-- convert percent value to x%  numeric x/100 -->
    <xsl:template name="ValueOfPercent">
        <xsl:param name="value"/>
        <xsl:choose>
            <xsl:when test="contains($value, '%')">
                <xsl:value-of select="substring-before($value, '%') div 100"/>
            </xsl:when>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="points2points">
        <xsl:param name="input_points"/>
        <xsl:param name="input_x"/>
        <xsl:param name="input_y"/>
        <xsl:param name="input_width"/>
        <xsl:param name="input_height"/>
        <xsl:param name="input_boxwidth"/>
        <xsl:param name="input_boxheight"/>
        <xsl:variable name="onepoint" select="substring ($input_points,1,string-length($input_points)  - string-length(substring-after($input_points,' '))  )"/>
        <xsl:if test="substring-before($input_points,' ')">
            <xsl:value-of select="round($input_x + (($input_boxwidth - substring-before($onepoint,',')) ) * $input_width div  $input_boxwidth)"/>
            <xsl:value-of select="'pt,'"/>
            <xsl:value-of select="round($input_y +  ((substring-after($onepoint,',')  ) ) * $input_height div $input_boxheight)"/>
            <xsl:value-of select="'pt'"/>
        </xsl:if>
        <xsl:if test="string-length(substring($input_points,string-length($onepoint) + 1)) &gt; 0">
            <xsl:value-of select="','"/>
            <xsl:call-template name="points2points">
                <xsl:with-param name="input_points" select="substring($input_points,string-length($onepoint) + 1)"/>
                <xsl:with-param name="input_x" select="$input_x"/>
                <xsl:with-param name="input_y" select="$input_y"/>
                <xsl:with-param name="input_width" select="$input_width"/>
                <xsl:with-param name="input_height" select="$input_height"/>
                <xsl:with-param name="input_boxwidth" select="$input_boxwidth"/>
                <xsl:with-param name="input_boxheight" select="$input_boxheight"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>
    <xsl:template name="path2path">
        <xsl:param name="input_points"/>
        <xsl:param name="x_or_y" select="'x'"/>
        <xsl:param name="input_x"/>
        <xsl:param name="input_y"/>
        <xsl:param name="input_width"/>
        <xsl:param name="input_height"/>
        <xsl:param name="input_boxwidth"/>
        <xsl:param name="input_boxheight"/>
        <xsl:variable name="space-pos" select="string-length($input_points) - string-length(substring-after($input_points,' '))"/>
        <xsl:variable name="minus-pos" select="string-length($input_points) - string-length(substring-after($input_points,'-'))"/>
        <xsl:variable name="m-pos" select="string-length($input_points) - string-length(substring-after($input_points,'m'))"/>
        <xsl:variable name="c-pos" select="string-length($input_points) - string-length(substring-after($input_points,'c'))"/>
        <xsl:variable name="e-pos" select="string-length($input_points) - string-length(substring-after($input_points,'e'))"/>
        <xsl:variable name="min1">
            <xsl:choose>
                <xsl:when test="$space-pos &lt; $minus-pos">
                    <xsl:value-of select="$space-pos"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$minus-pos"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="min2">
            <xsl:choose>
                <xsl:when test="$m-pos &lt; $min1">
                    <xsl:value-of select="$m-pos"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$min1"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="min3">
            <xsl:choose>
                <xsl:when test="$c-pos &lt; $min2">
                    <xsl:value-of select="$c-pos"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$min2"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="min4">
            <xsl:choose>
                <xsl:when test="$e-pos &lt; $min3">
                    <xsl:value-of select="$e-pos"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$min3"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="min-special-char-pos" select="$min4"/>
        <xsl:variable name="special-char" select="substring($input_points,$min-special-char-pos,1)"/>
        <xsl:variable name="one-value" select="substring($input_points,1,$min-special-char-pos - 1)"/>
        <xsl:variable name="left-points" select="substring($input_points,$min-special-char-pos + 1)"/>
        <xsl:if test="not($special-char = 'm')">
            <xsl:if test="$x_or_y = 'x'">
                <xsl:value-of select="round($input_x + $one-value * $input_width div  $input_boxwidth)"/>
            </xsl:if>
            <xsl:if test="$x_or_y = 'y'">
                <xsl:value-of select="round($input_y + $one-value * $input_height div $input_boxheight)"/>
            </xsl:if>
        </xsl:if>
        <!-- output the separator-->
        <xsl:choose>
            <xsl:when test="$special-char = '-' or $special-char = ' ' ">
                <xsl:value-of select="','"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$special-char"/>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:variable name="next-xy">
            <xsl:if test="$x_or_y = 'x'">
                <xsl:value-of select="'y'"/>
            </xsl:if>
            <xsl:if test="$x_or_y = 'y'">
                <xsl:value-of select="'x'"/>
            </xsl:if>
        </xsl:variable>
        <xsl:if test="string-length($left-points) &gt; 0">
            <xsl:call-template name="path2path">
                <xsl:with-param name="input_points" select="$left-points"/>
                <xsl:with-param name="x_or_y" select="$next-xy"/>
                <xsl:with-param name="input_x" select="$input_x"/>
                <xsl:with-param name="input_y" select="$input_y"/>
                <xsl:with-param name="input_width" select="$input_width"/>
                <xsl:with-param name="input_height" select="$input_height"/>
                <xsl:with-param name="input_boxwidth" select="$input_boxwidth"/>
                <xsl:with-param name="input_boxheight" select="$input_boxheight"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>
    <xsl:template name="get_dashstyle">
        <xsl:param name="stroke-width" select="0.1"/>
        <xsl:param name="style-name" select="@draw:style-name"/>
        <xsl:variable name="graph-style" select="key('graphics-style', $style-name)/style:graphic-properties"/>
        <xsl:variable name="dash-style" select="key('stroke-dash-style', $graph-style/@draw:stroke-dash)"/>
        <xsl:variable name="stroke">
            <xsl:choose>
                <xsl:when test="$graph-style/@draw:stroke">
                    <xsl:value-of select="$graph-style/@draw:stroke"/>
                </xsl:when>
                <xsl:when test="$dash-style/@draw:stroke">
                    <xsl:value-of select="$dash-style/@draw:stroke"/>
                </xsl:when>
            </xsl:choose>
        </xsl:variable>
        <xsl:choose>
            <xsl:when test="$stroke = 'solid' ">
                <xsl:value-of select="$stroke"/>
            </xsl:when>
            <xsl:when test="$stroke = 'dash'">
                <xsl:variable name="dots1">
                    <xsl:choose>
                        <xsl:when test="$graph-style/@draw:dots1">
                            <xsl:value-of select="$graph-style/@draw:dots1"/>
                        </xsl:when>
                        <xsl:when test="$dash-style/@draw:dots1">
                            <xsl:value-of select="$dash-style/@draw:dots1"/>
                        </xsl:when>
                        <xsl:otherwise>0</xsl:otherwise>
                    </xsl:choose>
                </xsl:variable>
                <xsl:variable name="dots2">
                    <xsl:choose>
                        <xsl:when test="$graph-style/@draw:dots1">
                            <xsl:value-of select="$graph-style/@draw:dots2"/>
                        </xsl:when>
                        <xsl:when test="$dash-style/@draw:dots1">
                            <xsl:value-of select="$dash-style/@draw:dots2"/>
                        </xsl:when>
                        <xsl:otherwise>0</xsl:otherwise>
                    </xsl:choose>
                </xsl:variable>
                <xsl:variable name="dots1-length">
                    <xsl:choose>
                        <xsl:when test="$graph-style/@draw:dots1-length">
                            <xsl:value-of select="$graph-style/@draw:dots1-length"/>
                        </xsl:when>
                        <xsl:when test="$dash-style/@draw:dots1-length">
                            <xsl:value-of select="$dash-style/@draw:dots1-length"/>
                        </xsl:when>
                        <xsl:otherwise>0</xsl:otherwise>
                    </xsl:choose>
                </xsl:variable>
                <xsl:variable name="dots2-length">
                    <xsl:choose>
                        <xsl:when test="$graph-style/@draw:dots2-length">
                            <xsl:value-of select="$graph-style/@draw:dots2-length"/>
                        </xsl:when>
                        <xsl:when test="$dash-style/@draw:dots2-length">
                            <xsl:value-of select="$dash-style/@draw:dots2-length"/>
                        </xsl:when>
                        <xsl:otherwise>0</xsl:otherwise>
                    </xsl:choose>
                </xsl:variable>
                <xsl:variable name="distance">
                    <xsl:choose>
                        <xsl:when test="$graph-style/@draw:distance">
                            <xsl:value-of select="$graph-style/@draw:distance"/>
                        </xsl:when>
                        <xsl:when test="$dash-style/@draw:distance">
                            <xsl:value-of select="$dash-style/@draw:distance"/>
                        </xsl:when>
                        <xsl:otherwise>0</xsl:otherwise>
                    </xsl:choose>
                </xsl:variable>
                <xsl:variable name="dot1-step">
                    <xsl:choose>
                        <xsl:when test="contains($dots1-length,'%')">
                            <xsl:variable name="dots-percent">
                                <xsl:call-template name="ValueOfPercent">
                                    <xsl:with-param name="value" select="$dots1-length"/>
                                </xsl:call-template>
                            </xsl:variable>
                            <xsl:value-of select="round($dots-percent)"/>
                        </xsl:when>
                        <xsl:when test="contains($dots1-length , 'in' ) and $stroke-width &gt; 0">
                            <xsl:value-of select="round( substring-before ($dots1-length,'in' ) div $stroke-width )"/>
                        </xsl:when>
                        <xsl:otherwise>0</xsl:otherwise>
                    </xsl:choose>
                </xsl:variable>
                <xsl:variable name="dot2-step">
                    <xsl:choose>
                        <xsl:when test="contains($dots2-length,'%')">
                            <xsl:variable name="dots-percent">
                                <xsl:call-template name="ValueOfPercent">
                                    <xsl:with-param name="value" select="$dots2-length"/>
                                </xsl:call-template>
                            </xsl:variable>
                            <xsl:value-of select="round($dots-percent)"/>
                        </xsl:when>
                        <xsl:when test="contains($dots2-length,'in') and $stroke-width &gt; 0">
                            <xsl:value-of select="round(substring-before($dots2-length,'in') div $stroke-width)"/>
                        </xsl:when>
                        <xsl:otherwise>0</xsl:otherwise>
                    </xsl:choose>
                </xsl:variable>
                <xsl:variable name="distance-step">
                    <xsl:choose>
                        <xsl:when test="contains($distance,'%')">
                            <xsl:variable name="dots-percent">
                                <xsl:call-template name="ValueOfPercent">
                                    <xsl:with-param name="value" select="$distance"/>
                                </xsl:call-template>
                            </xsl:variable>
                            <xsl:value-of select="round($dots-percent)"/>
                        </xsl:when>
                        <xsl:when test="contains($distance,'in') and $stroke-width &gt; 0">
                            <xsl:value-of select="round(substring-before($distance,'in') div $stroke-width)"/>
                        </xsl:when>
                        <xsl:otherwise>1</xsl:otherwise>
                    </xsl:choose>
                </xsl:variable>
                <xsl:variable name="dashstyle">
                    <xsl:choose>
                        <xsl:when test="$dots1 = 1 and $dots2 = 1 and $dot1-step = 0 and $dot2-step = 0 and $distance-step = 0">
                            <xsl:value-of select="'ShortDot'"/>
                        </xsl:when>
                        <xsl:when test="$dots2 = 0 and $dot1-step = 0 and $dot2-step = 0 and $distance-step &gt; 0">
                            <xsl:value-of select="concat('0 ', $distance-step)"/>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:call-template name="create_dashstyle">
                                <xsl:with-param name="dot-count" select="$dots1"/>
                                <xsl:with-param name="dot-step" select="$dot1-step"/>
                                <xsl:with-param name="distance-step" select="$distance-step"/>
                            </xsl:call-template>
                            <xsl:value-of select="' '"/>
                            <xsl:call-template name="create_dashstyle">
                                <xsl:with-param name="dot-count" select="$dots2"/>
                                <xsl:with-param name="dot-step" select="$dot2-step"/>
                                <xsl:with-param name="distance-step" select="$distance-step"/>
                            </xsl:call-template>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:variable>
                <xsl:value-of select="$dashstyle"/>
            </xsl:when>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="create_dashstyle">
        <xsl:param name="dot-count"/>
        <xsl:param name="dot-step"/>
        <xsl:param name="distance-step"/>
        <xsl:if test="$dot-count &gt; 0">
            <xsl:value-of select="concat($dot-step, ' ' , $distance-step )"/>
            <xsl:if test="$dot-count - 1 &gt; 0">
                <xsl:value-of select="' '"/>
                <xsl:call-template name="create_dashstyle">
                    <xsl:with-param name="dot-count" select="$dot-count - 1"/>
                    <xsl:with-param name="dot-step" select="$dot-step"/>
                    <xsl:with-param name="distance-step" select="$distance-step"/>
                </xsl:call-template>
            </xsl:if>
        </xsl:if>
    </xsl:template>
    <xsl:template name="get_borderstyle">
        <xsl:param name="border"/>
        <xsl:param name="border-line-width"/>
        <xsl:choose>
            <xsl:when test="contains($border,'solid')">
                <xsl:variable name="strokeweight">
                    <xsl:call-template name="ConvertMeasure">
                        <xsl:with-param name="value" select="substring-before($border, ' ')"/>
                        <xsl:with-param name="TargetMeasure" select="'pt'"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:value-of select="concat ( 'strokeweight:', $strokeweight)"/>
            </xsl:when>
            <xsl:when test="contains($border,'double')">
                <xsl:variable name="outside">
                    <xsl:call-template name="ConvertMeasure">
                        <xsl:with-param name="value" select="substring-after(substring-after($border-line-width, ' ') , ' ')"/>
                        <xsl:with-param name="TargetMeasure" select="'pt'"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="inside">
                    <xsl:call-template name="ConvertMeasure">
                        <xsl:with-param name="value" select="substring-before($border-line-width, ' ')"/>
                        <xsl:with-param name="TargetMeasure" select="'pt'"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="space">
                    <xsl:call-template name="ConvertMeasure">
                        <xsl:with-param name="value" select="substring-before(substring-after($border-line-width, ' ') , ' ')"/>
                        <xsl:with-param name="TargetMeasure" select="'pt'"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="linestyle">
                    <xsl:choose>
                        <xsl:when test="$outside = $inside">
                            <xsl:value-of select="'thinThin'"/>
                        </xsl:when>
                        <xsl:when test="$outside &gt; $inside">
                            <xsl:value-of select="'thickThin'"/>
                        </xsl:when>
                        <xsl:when test="$outside &lt; $inside">
                            <xsl:value-of select="'thinThick'"/>
                        </xsl:when>
                    </xsl:choose>
                </xsl:variable>
                <xsl:variable name="strokeweight" select="$inside + $outside + $space"/>
                <xsl:value-of select="concat( 'linestyle:' , $linestyle , ';' , 'strokeweight:' , $strokeweight )"/>
            </xsl:when>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="DrawElements">
        <xsl:param name="TargetMeasure" select="pt"/>
        <xsl:param name="x-adjust" select="0"/>
        <xsl:param name="y-adjust" select="0"/>
        <xsl:param name="force-draw" select="'false'"/>
        <xsl:param name="shape-type"/>
        <xsl:variable name="MeasureMark">
            <xsl:choose>
                <xsl:when test="$TargetMeasure = 'twip'"/>
                <xsl:otherwise>
                    <xsl:value-of select="$TargetMeasure"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="drawtextstyle" select="@draw:text-style-name"/>
        <xsl:variable name="org-z-index" select="number(concat('0',@draw-z-intex))"/>
        <xsl:variable name="run-though" select="key('graphics-style', @draw:style-name)/style:graphic-properties/@style:run-through"/>
        <xsl:variable name="org-wrap" select="key('graphics-style', @draw:style-name)/style:graphic-properties/@style:wrap"/>
        <xsl:variable name="draw-name">
            <xsl:choose>
                <xsl:when test="string-length(@draw:name) = 0">
                    <xsl:value-of select="translate(ancestor::draw:frame/@draw:name, ':/',  '__')"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="translate(@draw:name, ':/',  '__')"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="draw-fill-type" select="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:fill"/>
        <xsl:variable name="draw-gradient-name" select="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:fill-gradient-name"/>
        <xsl:variable name="fill-image-name" select="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:fill-image-name"/>
        <xsl:if test="$draw-fill-type = 'bitmap' ">
            <xsl:element name="w:binData">
                <xsl:attribute name="w:name">
                    <xsl:value-of select="concat( 'wordml://', $fill-image-name)"/>
                </xsl:attribute>
                <xsl:value-of select="translate(key('fill-image',$fill-image-name)/office:binary-data/text(),'&#9;&#10;&#13;&#32;','' ) "/>
                <!-- xsl:value-of select="office:binary-data/text()"/ -->
            </xsl:element>
        </xsl:if>
        <xsl:variable name="z-index">
            <xsl:choose>
                <xsl:when test="$run-though='foreground'">
                    <!-- make sure z-index >=0 -->
                    <xsl:choose>
                        <xsl:when test="$org-z-index &lt; 0">0</xsl:when>
                        <xsl:otherwise>
                            <xsl:value-of select="$org-z-index"/>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:when>
                <xsl:when test="$run-though='background'">
                    <!-- make sure z-index < 0 -->
                    <xsl:choose>
                        <xsl:when test="$org-z-index &lt; 0">
                            <xsl:value-of select="$org-z-index"/>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:value-of select="$org-z-index - 10"/>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:when>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="wrap">
            <xsl:choose>
                <xsl:when test="@text:anchor-type='as-char' ">none</xsl:when>
                <xsl:when test="$org-wrap='dynamic'">tight</xsl:when>
                <xsl:when test="$org-wrap='parallel'">square</xsl:when>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="ooshapename" select="substring-after(name(),':')"/>
        <xsl:variable name="element-name">
            <xsl:choose>
                <xsl:when test="$ooshapename='rect'">v:rect</xsl:when>
                <xsl:when test="$ooshapename='ellipse' and not(string-length(@draw:kind) &gt; 0)">v:oval</xsl:when>
                <xsl:when test="$ooshapename='ellipse' and string-length(@draw:kind) &gt; 0">v:arc</xsl:when>
                <xsl:when test="$ooshapename='circle' and string-length(@draw:kind) &gt; 0">v:arc</xsl:when>
                <xsl:when test="$ooshapename='line'">v:line</xsl:when>
                <xsl:when test="$ooshapename='polyline'">v:polyline</xsl:when>
                <xsl:when test="$ooshapename='polygon'">v:polyline</xsl:when>
                <xsl:when test="$ooshapename='text-box'">v:shape</xsl:when>
                <xsl:when test="$ooshapename='image'">v:shape</xsl:when>
                <xsl:when test="$ooshapename='frame'">v:shape</xsl:when>
                <xsl:when test="$ooshapename='path'">v:shape</xsl:when>
                <!-- This caption is not the "Caption", it's GUI name is Callouts-->
                <xsl:when test="$ooshapename='caption'">v:shape</xsl:when>
                <xsl:when test="$ooshapename='custom-shape' and draw:enhanced-geometry[1]/@draw:predefined-type = 'non-primitive' ">v:shape</xsl:when>
                <xsl:when test="$ooshapename='custom-shape' and draw:enhanced-geometry[1]/@draw:predefined-type = 'round-rectangle' ">v:roundrect</xsl:when>
                <xsl:when test="$ooshapename='custom-shape' and draw:enhanced-geometry[1]/@draw:predefined-type = 'rectangle' ">v:rect</xsl:when>
                <xsl:when test="$ooshapename='custom-shape' and draw:enhanced-geometry[1]/@draw:predefined-type = 'ellipse' ">v:oval</xsl:when>
                <xsl:when test="$ooshapename='custom-shape'">v:shape</xsl:when>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="ms-shape-type">
            <xsl:choose>
                <xsl:when test="string-length($shape-type) &gt; 0">
                    <xsl:value-of select="$shape-type"/>
                </xsl:when>
                <xsl:when test="$ooshapename='custom-shape' ">
                    <xsl:call-template name="ooo_custom_draw2ms_word_draw_map">
                        <xsl:with-param name="ooo_predefined_type" select="draw:enhanced-geometry[1]/@draw:predefined-type"/>
                    </xsl:call-template>
                </xsl:when>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="real-x-adjust">
            <xsl:call-template name="ConvertMeasure">
                <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                <xsl:with-param name="value" select="concat($x-adjust,'in')"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="real-y-adjust">
            <xsl:call-template name="ConvertMeasure">
                <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                <xsl:with-param name="value" select="concat($y-adjust,'in')"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="org-x">
            <xsl:call-template name="ConvertMeasure">
                <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                <xsl:with-param name="value" select="@svg:x"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="org-y">
            <xsl:call-template name="ConvertMeasure">
                <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                <xsl:with-param name="value" select="@svg:y"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="pagemaster" select="key('master-page','Standard')/@style:page-layout-name"/>
        <xsl:variable name="leftmargin-pt">
            <xsl:call-template name="ConvertMeasure">
                <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                <xsl:with-param name="value" select="key('page-layout',$pagemaster)/page-layout-properties/@fo:margin-left"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="topmargin-pt">
            <xsl:call-template name="ConvertMeasure">
                <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                <xsl:with-param name="value" select="key('page-layout',$pagemaster)/page-layout-properties/@fo:margin-top"/>
            </xsl:call-template>
        </xsl:variable>
        <!-- addjust the x and y values of the page archored objects-->
        <xsl:variable name="x">
            <xsl:choose>
                <xsl:when test="@text:anchor-type='page'">
                    <xsl:value-of select="$org-x - $leftmargin-pt"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$org-x + $real-x-adjust"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="y">
            <xsl:choose>
                <xsl:when test="@text:anchor-type='page'">
                    <xsl:value-of select="$org-y - $topmargin-pt"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$org-y + $real-y-adjust"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="width">
            <xsl:variable name="width-tmp">
                <xsl:choose>
                    <xsl:when test="string-length(@svg:width) = 0 and ancestor::draw:frame">
                        <xsl:value-of select="ancestor::draw:frame/@svg:width"/>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:value-of select="@svg:width"/>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:variable>
            <xsl:call-template name="ConvertMeasure">
                <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                <xsl:with-param name="value" select="$width-tmp"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="height">
            <xsl:choose>
                <xsl:when test="@svg:height">
                    <xsl:call-template name="ConvertMeasure">
                        <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                        <xsl:with-param name="value" select="@svg:height"/>
                    </xsl:call-template>
                </xsl:when>
                <xsl:when test="@fo:min-height">
                    <xsl:call-template name="ConvertMeasure">
                        <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                        <xsl:with-param name="value" select="@fo:min-height"/>
                    </xsl:call-template>
                </xsl:when>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="borderstyle">
            <xsl:call-template name="get_borderstyle">
                <xsl:with-param name="border" select="key('graphics-style', @draw:style-name)/style:graphic-properties/@fo:border"/>
                <xsl:with-param name="border-line-width" select="key('graphics-style', @draw:style-name)/style:graphic-properties/@style:border-line-width"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="stroke-weight-in-inch" select="number(concat('0',translate(key('graphics-style', @draw:style-name)/style:graphic-properties/@svg:stroke-width ,'abcdefghijklmnopqrstuvwxyz','') ))"/>
        <xsl:variable name="stroke-weight">
            <xsl:choose>
                <xsl:when test="contains($borderstyle , 'strokeweight')">
                    <xsl:call-template name="ConvertMeasure">
                        <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                        <xsl:with-param name="value" select="concat( substring-after($borderstyle, 'strokeweight:') , 'pt')"/>
                    </xsl:call-template>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:call-template name="ConvertMeasure">
                        <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                        <xsl:with-param name="value" select="key('graphics-style', @draw:style-name)/style:graphic-properties/@svg:stroke-width"/>
                    </xsl:call-template>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="linestyle">
            <xsl:if test="contains($borderstyle , 'strokeweight')">
                <xsl:value-of select="substring-before( substring-after($borderstyle, 'linestyle:') , ';strokeweight')"/>
            </xsl:if>
        </xsl:variable>
        <xsl:variable name="stroked">
            <xsl:if test="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:stroke = 'none'">
                <xsl:value-of select="'f'"/>
            </xsl:if>
        </xsl:variable>
        <xsl:variable name="dashstyle">
            <xsl:call-template name="get_dashstyle">
                <xsl:with-param name="stroke-width" select="$stroke-weight-in-inch"/>
                <xsl:with-param name="style-name" select="@draw:style-name"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="start-arrow">
            <xsl:choose>
                <xsl:when test="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:marker-end">
                    <xsl:call-template name="MapArrowStyle">
                        <xsl:with-param name="arrow-name" select="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:marker-end"/>
                    </xsl:call-template>
                </xsl:when>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="end-arrow">
            <xsl:choose>
                <xsl:when test="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:marker-start">
                    <xsl:call-template name="MapArrowStyle">
                        <xsl:with-param name="arrow-name" select="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:marker-start"/>
                    </xsl:call-template>
                </xsl:when>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="start-arrow-length">
            <xsl:choose>
                <xsl:when test="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:marker-end">
                    <xsl:call-template name="GetArrowLength">
                        <xsl:with-param name="arrow-name" select="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:marker-end"/>
                        <xsl:with-param name="arrow-width" select="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:marker-end-width"/>
                    </xsl:call-template>
                </xsl:when>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="start-arrow-width">
            <xsl:choose>
                <xsl:when test="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:marker-end">
                    <xsl:call-template name="GetArrowWidth">
                        <xsl:with-param name="arrow-name" select="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:marker-end"/>
                        <xsl:with-param name="arrow-width" select="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:marker-end-width"/>
                    </xsl:call-template>
                </xsl:when>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="end-arrow-length">
            <xsl:choose>
                <xsl:when test="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:marker-start">
                    <xsl:call-template name="GetArrowLength">
                        <xsl:with-param name="arrow-name" select="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:marker-start"/>
                        <xsl:with-param name="arrow-width" select="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:marker-start-width"/>
                    </xsl:call-template>
                </xsl:when>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="end-arrow-width">
            <xsl:choose>
                <xsl:when test="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:marker-start">
                    <xsl:call-template name="GetArrowWidth">
                        <xsl:with-param name="arrow-name" select="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:marker-start"/>
                        <xsl:with-param name="arrow-width" select="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:marker-start-width"/>
                    </xsl:call-template>
                </xsl:when>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="stroke-color">
            <xsl:choose>
                <xsl:when test="key('graphics-style', @draw:style-name)/style:graphic-properties/@svg:stroke-color">
                    <xsl:value-of select="key('graphics-style', @draw:style-name)/style:graphic-properties/@svg:stroke-color"/>
                </xsl:when>
                <xsl:when test="contains(key('graphics-style', @draw:style-name)/style:graphic-properties/@fo:border, '#')">
                    <xsl:value-of select="concat('#',  substring-after(key('graphics-style', @draw:style-name)/style:graphic-properties/@fo:border, '#') )"/>
                </xsl:when>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="stroke-opacity">
            <xsl:call-template name="ValueOfPercent">
                <xsl:with-param name="value" select="key('graphics-style', @draw:style-name)/style:graphic-properties/@svg:stroke-opacity"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="fill-color">
            <xsl:choose>
                <xsl:when test="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:fill-color">
                    <xsl:value-of select="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:fill-color"/>
                </xsl:when>
                <xsl:when test="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:fontwork-style">black</xsl:when>
                <xsl:when test="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:fill = 'none'"/>
                <xsl:when test="$draw-fill-type = 'gradient' ">
                    <xsl:value-of select="key('draw-gradient',$draw-gradient-name)/@draw:end-color "/>
                </xsl:when>
                <!-- for these need fill, set the default color we used in oo-->
                <xsl:when test="name()='draw:polygon' or name()='draw:custom-shape'  or name() = 'draw:rect' or (name() = 'draw:ellipse' and not(  @draw:kind='arc') )">#00B8FF</xsl:when>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="position" select="concat('left:', $x ,$MeasureMark ,  ';top:' , $y ,$MeasureMark  , ';width:', $width ,$MeasureMark , ';height:', $height ,$MeasureMark  )"/>
        <xsl:variable name="flip">
            <xsl:choose>
                <xsl:when test="@text:anchor-type='as-char' and $ooshapename='line'"/>
                <xsl:when test="$ooshapename='image'"/>
                <xsl:when test="$ooshapename='path'"/>
                <xsl:when test="$ooshapename='caption'"/>
                <xsl:when test="@draw:kind = 'arc' or @draw:kind = 'cut' or @draw:kind = 'section'"/>
                <xsl:when test="$ooshapename='custom-shape'"/>
                <xsl:otherwise>flip:x</xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="absolute">
            <xsl:choose>
                <xsl:when test="ancestor::draw:a"/>
                <xsl:when test="@text:anchor-type = 'as-char' or @text:anchor-type = 'to-char'"/>
                <xsl:otherwise>position:absolute</xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <!-- start line special -->
        <xsl:variable name="org-x1">
            <xsl:call-template name="ConvertMeasure">
                <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                <xsl:with-param name="value" select="@svg:x1"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="org-y1">
            <xsl:call-template name="ConvertMeasure">
                <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                <xsl:with-param name="value" select="@svg:y1"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="org-x2">
            <xsl:call-template name="ConvertMeasure">
                <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                <xsl:with-param name="value" select="@svg:x2"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="org-y2">
            <xsl:call-template name="ConvertMeasure">
                <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                <xsl:with-param name="value" select="@svg:y2"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="x1">
            <xsl:choose>
                <xsl:when test="@text:anchor-type='page'">
                    <xsl:value-of select="$org-x1 - $leftmargin-pt"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$org-x1"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="y1">
            <xsl:choose>
                <xsl:when test="@text:anchor-type='page'">
                    <xsl:value-of select="$org-y1 - $topmargin-pt"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$org-y1"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="x2">
            <xsl:choose>
                <xsl:when test="@text:anchor-type='page'">
                    <xsl:value-of select="$org-x2 - $leftmargin-pt"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$org-x2"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="y2">
            <xsl:choose>
                <xsl:when test="@text:anchor-type='page'">
                    <xsl:value-of select="$org-y2 - $topmargin-pt"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$org-y2"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <!-- end line special -->
        <xsl:variable name="style">
            <xsl:choose>
                <xsl:when test="$wrap='none'"/>
                <xsl:otherwise>
                    <xsl:if test="string-length($absolute) &gt; 0">
                        <xsl:value-of select="concat($absolute, ';')"/>
                    </xsl:if>
                    <xsl:value-of select="concat('z-index:', $z-index, ';')"/>
                </xsl:otherwise>
            </xsl:choose>
            <xsl:if test="string-length($flip) &gt; 0">
                <xsl:value-of select="concat($flip,';')"/>
            </xsl:if>
            <xsl:if test="not($ooshapename = 'line')">
                <xsl:value-of select="concat($position,';')"/>
            </xsl:if>
        </xsl:variable>
        <!-- image special: convert oo base64 binary data (77char/line) to word base64 binary data(73char/line) , a workthrough is removing all line breaks -->
        <xsl:if test="$ooshapename = 'image'">
            <xsl:element name="w:binData">
                <xsl:attribute name="w:name">
                    <xsl:value-of select="concat( 'wordml://', $draw-name )"/>
                </xsl:attribute>
                <xsl:value-of select="translate(office:binary-data/text(),'&#9;&#10;&#13;&#32;','' ) "/>
                <!-- xsl:value-of select="office:binary-data/text()"/ -->
            </xsl:element>
        </xsl:if>
        <!-- all element goes here -->
        <xsl:variable name="id">
            <xsl:choose>
                <xsl:when test="$ooshapename='line'">
                    <xsl:value-of select="concat('_x',$x1 , '_' ,$y1, '_' , $x2, '_' ,$y2 )"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="concat('_x',$x , '_' ,$y, '_' , $width, '_' ,$height )"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:element name="{$element-name}">
            <xsl:attribute name="id">
                <xsl:value-of select="$id"/>
            </xsl:attribute>
            <xsl:if test="string-length($ms-shape-type) &gt; 0">
                <xsl:attribute name="type">
                    <xsl:value-of select="$ms-shape-type"/>
                </xsl:attribute>
            </xsl:if>
            <xsl:attribute name="style">
                <xsl:value-of select="$style"/>
            </xsl:attribute>
            <xsl:if test="$stroke-weight &gt; 0">
                <xsl:attribute name="strokeweight">
                    <xsl:value-of select="concat($stroke-weight,$MeasureMark)"/>
                </xsl:attribute>
            </xsl:if>
            <xsl:if test="string-length($stroked) &gt; 0">
                <xsl:attribute name="stroked">
                    <xsl:value-of select="$stroked"/>
                </xsl:attribute>
            </xsl:if>
            <xsl:if test="string-length($stroke-color) &gt; 0">
                <xsl:attribute name="strokecolor">
                    <xsl:value-of select="$stroke-color"/>
                </xsl:attribute>
            </xsl:if>
            <xsl:if test="string-length($fill-color) &gt; 0">
                <xsl:attribute name="fillcolor">
                    <xsl:value-of select="$fill-color"/>
                </xsl:attribute>
                <xsl:attribute name="filled">true</xsl:attribute>
            </xsl:if>
            <xsl:if test="string-length($stroke-opacity) &gt; 0">
                <xsl:attribute name="opacity">
                    <xsl:value-of select="$stroke-opacity"/>
                </xsl:attribute>
            </xsl:if>
            <!-- arc special attribute -->
            <xsl:if test="@draw:kind = 'arc' or @draw:kind = 'cut' or @draw:kind = 'section'">
                <xsl:choose>
                    <xsl:when test="@draw:start-angle &gt; @draw:end-angle">
                        <xsl:attribute name="startangle">
                            <xsl:value-of select="round( 450 - (@draw:end-angle + 360)  )"/>
                        </xsl:attribute>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:attribute name="startangle">
                            <xsl:value-of select="round( 450 - @draw:end-angle  )"/>
                        </xsl:attribute>
                    </xsl:otherwise>
                </xsl:choose>
                <xsl:attribute name="endangle">
                    <xsl:value-of select="round(450 - @draw:start-angle)"/>
                </xsl:attribute>
            </xsl:if>
            <xsl:if test="@draw:kind = 'cut'">
                <xsl:attribute name="fill">
                    <xsl:value-of select="'true'"/>
                </xsl:attribute>
            </xsl:if>
            <!-- end arc special attribute -->
            <!-- line special attribute-->
            <xsl:if test="$ooshapename='line'">
                <xsl:attribute name="from">
                    <xsl:choose>
                        <xsl:when test="@text:anchor-type='as-char'">0,0</xsl:when>
                        <xsl:otherwise>
                            <xsl:value-of select="concat($x1,$MeasureMark, ',',$y2,$MeasureMark )"/>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:attribute>
                <xsl:attribute name="to">
                    <xsl:choose>
                        <xsl:when test="@text:anchor-type='as-char'">
                            <xsl:value-of select="concat($x2,$MeasureMark ,',',$y2,$MeasureMark )"/>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:value-of select="concat($x2,$MeasureMark ,',',$MeasureMark ,$y1)"/>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:attribute>
            </xsl:if>
            <!-- end line special attribute-->
            <!-- polyline and polygon special attribute-->
            <xsl:if test="$ooshapename='polyline' or $ooshapename='polygon' ">
                <!-- translate ' '  to in '  tranclate ',' to 'in,' -->
                <xsl:variable name="points">
                    <xsl:call-template name="points2points">
                        <xsl:with-param name="input_x" select="$x"/>
                        <xsl:with-param name="input_y" select="$y"/>
                        <xsl:with-param name="input_width" select="$width"/>
                        <xsl:with-param name="input_height" select="$height"/>
                        <xsl:with-param name="input_boxwidth" select="substring-before(substring-after(@svg:viewBox,'0 0 '),' ')"/>
                        <xsl:with-param name="input_boxheight" select="substring-after(substring-after(@svg:viewBox,'0 0 '),' ')"/>
                        <xsl:with-param name="input_points" select="concat(@draw:points,' ')"/>
                        <!-- add a space to the end of input_points -->
                    </xsl:call-template>
                </xsl:variable>
                <xsl:attribute name="points">
                    <xsl:value-of select="$points"/>
                </xsl:attribute>
            </xsl:if>
            <!-- end polyline and polygon special attribute-->
            <!-- callouts special attribute-->
            <xsl:if test="$ooshapename='caption'">
                <xsl:variable name="caption-point-x">
                    <xsl:call-template name="ConvertMeasure">
                        <xsl:with-param name="TargetMeasure" select="'twip'"/>
                        <xsl:with-param name="value" select="@draw:caption-point-x"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="caption-point-y">
                    <xsl:call-template name="ConvertMeasure">
                        <xsl:with-param name="TargetMeasure" select="'twip'"/>
                        <xsl:with-param name="value" select="@draw:caption-point-y"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:attribute name="type">
                    <!-- map to word line label 3-->
                    <xsl:value-of select="'#_x0000_t48'"/>
                </xsl:attribute>
                <xsl:attribute name="adj">
                    <xsl:value-of select=" concat($caption-point-x * 20 , ',' , $caption-point-y * 20 , ',' ,  $caption-point-x * 10 , ',,,,' , $caption-point-x * 20, ',' , $caption-point-y * 20) "/>
                </xsl:attribute>
            </xsl:if>
            <!-- end callouts special attribute-->
            <!-- path special attribute-->
            <xsl:if test="$ooshapename='path' or string-length(@svg:d) &gt; 0 or ( $ooshapename='custom-shape' and draw:enhanced-geometry[1]/@draw:predefined-type = 'non-primitive') ">
                <xsl:variable name="path">
                    <xsl:choose>
                        <xsl:when test="$ooshapename='path' or string-length(@svg:d) &gt; 0 ">
                            <xsl:call-template name="svgpath2vmlpath">
                                <xsl:with-param name="svg-path" select="@svg:d"/>
                            </xsl:call-template>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:call-template name="svgpath2vmlpath">
                                <xsl:with-param name="svg-path" select="draw:enhanced-geometry[1]/@draw:enhanced-path"/>
                            </xsl:call-template>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:variable>
                <xsl:attribute name="coordorigin">
                    <xsl:value-of select=" '0 0' "/>
                </xsl:attribute>
                <xsl:attribute name="coordsize">
                    <xsl:choose>
                        <xsl:when test="string-length(@svg:viewBox) &gt; 0 ">
                            <xsl:value-of select="substring-after(@svg:viewBox,'0 0 ')"/>
                        </xsl:when>
                        <xsl:when test="string-length(draw:enhanced-geometry[1]/@svg:viewBox) &gt; 0 ">
                            <xsl:value-of select="substring-after(draw:enhanced-geometry[1]/@svg:viewBox,'0 0 ')"/>
                        </xsl:when>
                        <!-- for custom shape use a default viewbox. right? -->
                        <xsl:otherwise>
                            <xsl:value-of select=" '21600 21600' "/>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:attribute>
                <xsl:attribute name="path">
                    <xsl:value-of select="$path"/>
                </xsl:attribute>
            </xsl:if>
            <!-- end path special attribute-->
            <!-- image special element -->
            <xsl:if test="$ooshapename='image'">
                <xsl:element name="v:imagedata">
                    <xsl:attribute name="src">
                        <xsl:value-of select="concat('wordml://', $draw-name)"/>
                    </xsl:attribute>
                    <xsl:attribute name="o:title">
                        <xsl:value-of select="$draw-name"/>
                    </xsl:attribute>
                </xsl:element>
            </xsl:if>
            <!-- end image special element -->
            <!-- start dash style  , line style and arrow style-->
            <xsl:if test="string-length($dashstyle) &gt; 0 or string-length($linestyle) &gt; 0  or string-length($start-arrow) &gt; 0  or string-length($end-arrow) &gt; 0 ">
                <xsl:element name="v:stroke">
                    <xsl:if test="string-length($dashstyle) &gt; 0">
                        <xsl:attribute name="dashstyle">
                            <xsl:value-of select="$dashstyle"/>
                        </xsl:attribute>
                    </xsl:if>
                    <xsl:if test="string-length($linestyle) &gt; 0">
                        <xsl:attribute name="linestyle">
                            <xsl:value-of select="$linestyle"/>
                        </xsl:attribute>
                    </xsl:if>
                    <xsl:if test="string-length($start-arrow) &gt; 0 ">
                        <xsl:attribute name="startarrow">
                            <xsl:value-of select="$start-arrow"/>
                        </xsl:attribute>
                    </xsl:if>
                    <xsl:if test="string-length($end-arrow) &gt; 0 ">
                        <xsl:attribute name="endarrow">
                            <xsl:value-of select="$end-arrow"/>
                        </xsl:attribute>
                    </xsl:if>
                    <xsl:if test="string-length($start-arrow-length) &gt; 0 ">
                        <xsl:attribute name="startarrowlength">
                            <xsl:value-of select="$start-arrow-length"/>
                        </xsl:attribute>
                    </xsl:if>
                    <xsl:if test="string-length($start-arrow-width) &gt; 0 ">
                        <xsl:attribute name="startarrowwidth">
                            <xsl:value-of select="$start-arrow-width"/>
                        </xsl:attribute>
                    </xsl:if>
                    <xsl:if test="string-length($end-arrow-length) &gt; 0 ">
                        <xsl:attribute name="endarrowlength">
                            <xsl:value-of select="$end-arrow-length"/>
                        </xsl:attribute>
                    </xsl:if>
                    <xsl:if test="string-length($end-arrow-width) &gt; 0 ">
                        <xsl:attribute name="endarrowwidth">
                            <xsl:value-of select="$end-arrow-width"/>
                        </xsl:attribute>
                    </xsl:if>
                </xsl:element>
            </xsl:if>
            <!-- end dash style , line style and arrow style -->
            <!-- start wrap type -->
            <xsl:if test="string-length($wrap) &gt; 0">
                <xsl:element name="w10:wrap">
                    <xsl:attribute name="type">
                        <xsl:value-of select="$wrap"/>
                    </xsl:attribute>
                </xsl:element>
            </xsl:if>
            <!-- end wrap type -->
            <!-- start fill image -->
            <xsl:if test="string-length($draw-fill-type ) &gt; 0">
                <xsl:element name="v:fill">
                    <xsl:choose>
                        <xsl:when test="$draw-fill-type = 'bitmap'">
                            <xsl:attribute name="src">
                                <xsl:value-of select="concat( 'wordml://', $fill-image-name)"/>
                            </xsl:attribute>
                            <xsl:attribute name="o:titile">
                                <xsl:value-of select="$fill-image-name"/>
                            </xsl:attribute>
                            <xsl:attribute name="recolor">
                                <xsl:value-of select=" 'true' "/>
                            </xsl:attribute>
                            <xsl:attribute name="rotate">
                                <xsl:value-of select=" 'true' "/>
                            </xsl:attribute>
                            <xsl:attribute name="type">
                                <xsl:value-of select=" 'frame' "/>
                            </xsl:attribute>
                        </xsl:when>
                        <xsl:when test="$draw-fill-type = 'gradient'">
                            <xsl:attribute name="type">
                                <xsl:value-of select=" 'gradient' "/>
                            </xsl:attribute>
                            <xsl:attribute name="color2">
                                <xsl:value-of select="key('draw-gradient',$draw-gradient-name)/@draw:start-color "/>
                            </xsl:attribute>
                        </xsl:when>
                    </xsl:choose>
                </xsl:element>
            </xsl:if>
            <!-- end fill image -->
            <!-- only draw:g can have child graphic -->
            <xsl:choose>
                <xsl:when test="name() = 'draw:g'">
                    <xsl:apply-templates select="draw:*">
                        <xsl:with-param name="TargetMeasure" select="$TargetMeasure"/>
                        <xsl:with-param name="x-adjust" select="$x-adjust"/>
                        <xsl:with-param name="y-adjust" select="$y-adjust"/>
                        <xsl:with-param name="force-draw" select="$force-draw"/>
                    </xsl:apply-templates>
                </xsl:when>
                <xsl:when test="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:fontwork-style">
                    <xsl:call-template name="FontWork"/>
                </xsl:when>
                <xsl:when test="text:*/* | text:*/text()">
                    <xsl:element name="v:textbox">
                        <xsl:if test="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:writing-mode = 'tb-rl'">
                            <xsl:attribute name="style">
                                <xsl:value-of select="'layout-flow:vertical'"/>
                            </xsl:attribute>
                        </xsl:if>
                        <w:txbxContent>
                            <xsl:apply-templates select="text() | text:*"/>
                        </w:txbxContent>
                    </xsl:element>
                </xsl:when>
            </xsl:choose>
        </xsl:element>
    </xsl:template>
    <xsl:template name="FontWork">
        <xsl:element name="v:path">
            <xsl:attribute name="textpathok">true</xsl:attribute>
        </xsl:element>
        <xsl:if test="not(key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:fontwork-shadow) or not(key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:fontwork-shadow = 'normal')">
            <xsl:element name="v:shadow">
                <xsl:attribute name="on">true</xsl:attribute>
                <xsl:attribute name="type">perspective</xsl:attribute>
                <xsl:attribute name="color">
                    <xsl:value-of select="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:fontwork-shadow-color"/>
                </xsl:attribute>
                <xsl:variable name="offset-x">
                    <xsl:call-template name="ConvertMeasure">
                        <xsl:with-param name="TargetMeasure" select="'twip'"/>
                        <xsl:with-param name="value" select="key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:fontwork-shadow-offset-x"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="offset-y" select="substring-before(key('graphics-style', @draw:style-name)/style:graphic-properties/@draw:fontwork-shadow-offset-y, 'in')"/>
                <xsl:attribute name="matrix">
                    <xsl:value-of select="concat(',,,' , round($offset-y div 0.000693) div 100, ',,')"/>
                </xsl:attribute>
                <xsl:attribute name="origin">-30%, -30%</xsl:attribute>
            </xsl:element>
        </xsl:if>
        <xsl:element name="v:textpath">
            <xsl:attribute name="on">true</xsl:attribute>
            <xsl:attribute name="fitpath">true</xsl:attribute>
            <xsl:attribute name="fitshape">true</xsl:attribute>
            <xsl:attribute name="style">
                <xsl:choose>
                    <xsl:when test="key('paragraph-style', text:p[1]/@text:style-name )/style:graphic-properties/@svg:font-family">
                        <xsl:value-of select="concat('font-family:&quot;' , key('paragraph-style', text:p[1]/@text:style-name )/style:graphic-properties/@svg:font-family ,  '&quot;') "/>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:variable name="default-graphics-properties" select="/office:document/office:styles/style:default-style[@style:family = 'graphics']/style:graphic-properties"/>
                        <xsl:value-of select="concat('font-family:&quot;' , $default-graphics-properties/@style:font-name ,  '&quot;') "/>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:attribute>
            <xsl:attribute name="string">
                <xsl:value-of select="text:p"/>
            </xsl:attribute>
        </xsl:element>
    </xsl:template>
    <xsl:template name="MapArrowStyle">
        <xsl:param name="arrow-name"/>
        <xsl:choose>
            <xsl:when test="$arrow-name = 'Arrow' ">Block</xsl:when>
            <xsl:when test="$arrow-name = 'Square' ">Diamond</xsl:when>
            <xsl:when test="$arrow-name = 'Small arrow' ">Block</xsl:when>
            <xsl:when test="$arrow-name = 'Dimension lines' ">Diamond</xsl:when>
            <xsl:when test="$arrow-name = 'Double Arrow' ">Block</xsl:when>
            <xsl:when test="$arrow-name = 'Rounded short arrow' ">Block</xsl:when>
            <xsl:when test="$arrow-name = 'Symmetric arrow' ">Block</xsl:when>
            <xsl:when test="$arrow-name = 'Line Arrow' ">Open</xsl:when>
            <xsl:when test="$arrow-name = 'Rounded large arrow' ">Block</xsl:when>
            <xsl:when test="$arrow-name = 'Circle' ">Oval</xsl:when>
            <xsl:when test="$arrow-name = 'Square 45' ">Diamond</xsl:when>
            <xsl:when test="$arrow-name = 'Arrow concave' ">Classic</xsl:when>
            <xsl:otherwise>Block</xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="GetArrowLength">
        <xsl:param name="arrow-name"/>
        <xsl:param name="arrow-width"/>
        <xsl:variable name="arrow-size">
            <xsl:choose>
                <xsl:when test="$arrow-width">
                    <xsl:value-of select="round(substring-before($arrow-width, 'in') div 0.02) "/>
                </xsl:when>
                <xsl:otherwise>3</xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:choose>
            <xsl:when test="$arrow-size &gt; 2">Long</xsl:when>
            <xsl:when test="$arrow-size &gt; 1">Medium</xsl:when>
            <xsl:when test="$arrow-size &gt; 0">Short</xsl:when>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="GetArrowWidth">
        <xsl:param name="arrow-name"/>
        <xsl:param name="arrow-width"/>
        <xsl:variable name="arrow-size">
            <xsl:choose>
                <xsl:when test="$arrow-width">
                    <xsl:value-of select="round(substring-before($arrow-width, 'in') div 0.02) "/>
                </xsl:when>
                <xsl:otherwise>3</xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:choose>
            <xsl:when test="$arrow-size &gt; 2">Wide</xsl:when>
            <xsl:when test="$arrow-size &gt; 1">Medium</xsl:when>
            <xsl:when test="$arrow-size &gt; 0">Narrow</xsl:when>
        </xsl:choose>
    </xsl:template>
</xsl:stylesheet>
