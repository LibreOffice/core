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
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:draw="http://openoffice.org/2000/drawing" xmlns:o="urn:schemas-microsoft-com:office:office" xmlns:svg="http://www.w3.org/2000/svg" xmlns:v="urn:schemas-microsoft-com:vml" xmlns:w10="urn:schemas-microsoft-com:office:word" xmlns:w="http://schemas.microsoft.com/office/word/2003/wordml" xmlns:text="http://openoffice.org/2000/text" xmlns:style="http://openoffice.org/2000/style" xmlns:fo="http://www.w3.org/1999/XSL/Format" xmlns:office="http://openoffice.org/2000/office" exclude-result-prefixes="draw svg style office fo text">
    <xsl:template name="test">
        <xsl:call-template name="svgpath2vmlpath">
            <xsl:with-param name="svg-path" select="'M 36.0 162.0 C 38.0 168.0 39.0-172.0 40.0 176.0 S 42.0 184.0 144.0 188.0'"/>
        </xsl:call-template>
    </xsl:template>
    <xsl:template name="svgpath2vmlpath">
        <xsl:param name="svg-path"/>
        <xsl:param name="vml-path" select="''"/>
        <xsl:param name="position" select="1"/>
        <xsl:param name="last-command" select="'M'"/>
        <xsl:param name="current-x" select="'0'"/>
        <xsl:param name="current-y" select="'0'"/>
        <xsl:variable name="command-and-newpos">
            <xsl:call-template name="get-path-command">
                <xsl:with-param name="svg-path" select="$svg-path"/>
                <xsl:with-param name="position" select="$position"/>
                <xsl:with-param name="last-command" select="$last-command"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="command" select="substring-before($command-and-newpos , ':')"/>
        <xsl:variable name="newpos" select="substring-after($command-and-newpos , ':')"/>
        <xsl:choose>
            <xsl:when test="$command = 'M' ">
                <!-- absolute moveto -->
                <xsl:variable name="new-vml-path" select="concat($vml-path ,' m ' ) "/>
                <xsl:variable name="num-and-pos">
                    <xsl:call-template name="get-number-after">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="$newpos"/>
                        <xsl:with-param name="count" select="2"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:call-template name="svgpath2vmlpath">
                    <xsl:with-param name="svg-path" select="$svg-path"/>
                    <xsl:with-param name="vml-path" select=" concat($new-vml-path , substring-before( $num-and-pos , ':')  , ' ') "/>
                    <xsl:with-param name="position" select=" substring-after( $num-and-pos , ':')  "/>
                    <xsl:with-param name="last-command" select="'L'"/>
                    <xsl:with-param name="current-x" select=" substring-before( substring-before( $num-and-pos , ':')   , ' ') "/>
                    <xsl:with-param name="current-y" select=" substring-after( substring-before( $num-and-pos , ':')   , ' ') "/>
                </xsl:call-template>
            </xsl:when>
            <xsl:when test="$command = 'm' ">
                <!-- relative moveto -->
                <xsl:variable name="new-vml-path" select="concat($vml-path ,' t ' ) "/>
                <xsl:variable name="num-and-pos">
                    <xsl:call-template name="get-number-after">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="$newpos"/>
                        <xsl:with-param name="count" select="2"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:call-template name="svgpath2vmlpath">
                    <xsl:with-param name="svg-path" select="$svg-path"/>
                    <xsl:with-param name="vml-path" select=" concat($new-vml-path , substring-before( $num-and-pos , ':')  , ' ') "/>
                    <xsl:with-param name="position" select=" substring-after( $num-and-pos , ':')  "/>
                    <xsl:with-param name="last-command" select="'l'"/>
                    <xsl:with-param name="current-x" select=" substring-before( substring-before( $num-and-pos , ':')    , ' ')  + $current-x"/>
                    <xsl:with-param name="current-y" select=" substring-after( substring-before( $num-and-pos , ':')    , ' ') + $current-y "/>
                </xsl:call-template>
            </xsl:when>
            <xsl:when test="$command = 'L' ">
                <!-- absolute lineto -->
                <xsl:variable name="new-vml-path" select="concat($vml-path ,' l ' ) "/>
                <xsl:variable name="num-and-pos">
                    <xsl:call-template name="get-number-after">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="$newpos"/>
                        <xsl:with-param name="count" select="2"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:call-template name="svgpath2vmlpath">
                    <xsl:with-param name="svg-path" select="$svg-path"/>
                    <xsl:with-param name="vml-path" select=" concat($new-vml-path , substring-before( $num-and-pos , ':')  , ' ') "/>
                    <xsl:with-param name="position" select=" substring-after( $num-and-pos , ':')  "/>
                    <xsl:with-param name="last-command" select="$command"/>
                    <xsl:with-param name="current-x" select=" substring-before( substring-before( $num-and-pos , ':')   , ' ') "/>
                    <xsl:with-param name="current-y" select=" substring-after( substring-before( $num-and-pos , ':')   , ' ') "/>
                </xsl:call-template>
            </xsl:when>
            <xsl:when test="$command = 'l' ">
                <!-- relative lineto -->
                <xsl:variable name="new-vml-path" select="concat($vml-path ,' r ' ) "/>
                <xsl:variable name="num-and-pos">
                    <xsl:call-template name="get-number-after">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="$newpos"/>
                        <xsl:with-param name="count" select="2"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:call-template name="svgpath2vmlpath">
                    <xsl:with-param name="svg-path" select="$svg-path"/>
                    <xsl:with-param name="vml-path" select=" concat($new-vml-path , substring-before( $num-and-pos , ':')  , ' ') "/>
                    <xsl:with-param name="position" select=" substring-after( $num-and-pos , ':')  "/>
                    <xsl:with-param name="last-command" select="$command"/>
                    <xsl:with-param name="current-x" select=" substring-before( substring-before( $num-and-pos , ':')  , ' ')  + $current-x  "/>
                    <xsl:with-param name="current-y" select=" substring-after( substring-before( $num-and-pos , ':')    , ' ')  + $current-y "/>
                </xsl:call-template>
            </xsl:when>
            <xsl:when test="$command = 'H' ">
                <!-- absolute horizontal  lineto -->
                <xsl:variable name="new-vml-path" select="concat($vml-path ,' l ' ) "/>
                <xsl:variable name="num-and-pos">
                    <xsl:call-template name="get-number-after">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="$newpos"/>
                        <xsl:with-param name="count" select="1"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:call-template name="svgpath2vmlpath">
                    <xsl:with-param name="svg-path" select="$svg-path"/>
                    <xsl:with-param name="vml-path" select=" concat($new-vml-path , substring-before( $num-and-pos , ':')  , ' ' , $current-y , ' ') "/>
                    <xsl:with-param name="position" select=" substring-after( $num-and-pos , ':')  "/>
                    <xsl:with-param name="last-command" select="$command"/>
                    <xsl:with-param name="current-x" select=" substring-before( $num-and-pos , ':')  "/>
                    <xsl:with-param name="current-y" select=" $current-y"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:when test="$command = 'h' ">
                <!-- relative horizontal  lineto -->
                <xsl:variable name="new-vml-path" select="concat($vml-path ,' l ' ) "/>
                <xsl:variable name="num-and-pos">
                    <xsl:call-template name="get-number-after">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="$newpos"/>
                        <xsl:with-param name="count" select="1"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:call-template name="svgpath2vmlpath">
                    <xsl:with-param name="svg-path" select="$svg-path"/>
                    <xsl:with-param name="vml-path" select=" concat($new-vml-path , substring-before( $num-and-pos , ':') + $current-x  , ' ' , $current-y , ' ') "/>
                    <xsl:with-param name="position" select=" substring-after( $num-and-pos , ':')  "/>
                    <xsl:with-param name="last-command" select="$command"/>
                    <xsl:with-param name="current-x" select=" substring-before( $num-and-pos , ':')  + $current-x"/>
                    <xsl:with-param name="current-y" select=" $current-y"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:when test="$command = 'V' ">
                <!-- absolute vertical  lineto -->
                <xsl:variable name="new-vml-path" select="concat($vml-path ,' l ' ) "/>
                <xsl:variable name="num-and-pos">
                    <xsl:call-template name="get-number-after">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="$newpos"/>
                        <xsl:with-param name="count" select="1"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:call-template name="svgpath2vmlpath">
                    <xsl:with-param name="svg-path" select="$svg-path"/>
                    <xsl:with-param name="vml-path" select=" concat($new-vml-path , $current-x , ' ' , substring-before( $num-and-pos , ':')  , ' ' ) "/>
                    <xsl:with-param name="position" select=" substring-after( $num-and-pos , ':')  "/>
                    <xsl:with-param name="last-command" select="$command"/>
                    <xsl:with-param name="current-x" select=" $current-x"/>
                    <xsl:with-param name="current-y" select=" substring-before( $num-and-pos , ':')  "/>
                </xsl:call-template>
            </xsl:when>
            <xsl:when test="$command = 'v' ">
                <!-- relative horizontal  lineto -->
                <xsl:variable name="new-vml-path" select="concat($vml-path ,' l ' ) "/>
                <xsl:variable name="num-and-pos">
                    <xsl:call-template name="get-number-after">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="$newpos"/>
                        <xsl:with-param name="count" select="1"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:call-template name="svgpath2vmlpath">
                    <xsl:with-param name="svg-path" select="$svg-path"/>
                    <xsl:with-param name="vml-path" select=" concat($new-vml-path , $current-x , ' ' , substring-before( $num-and-pos , ':')  + $current-y , ' ' ) "/>
                    <xsl:with-param name="position" select=" substring-after( $num-and-pos , ':')  "/>
                    <xsl:with-param name="last-command" select="$command"/>
                    <xsl:with-param name="current-x" select=" $current-x"/>
                    <xsl:with-param name="current-y" select=" substring-before( $num-and-pos , ':')  "/>
                </xsl:call-template>
            </xsl:when>
            <xsl:when test="$command = 'C' ">
                <!-- absolute curveto -->
                <xsl:variable name="new-vml-path" select="concat($vml-path ,' c ' ) "/>
                <xsl:variable name="control-and-pos">
                    <xsl:call-template name="get-number-after">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="$newpos"/>
                        <xsl:with-param name="count" select="4"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="num-and-pos">
                    <xsl:call-template name="get-number-after">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="substring-after( $control-and-pos , ':') "/>
                        <xsl:with-param name="count" select="2"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:call-template name="svgpath2vmlpath">
                    <xsl:with-param name="svg-path" select="$svg-path"/>
                    <xsl:with-param name="vml-path" select=" concat($new-vml-path , substring-before( $control-and-pos , ':')  , ' ' ,  substring-before( $num-and-pos , ':')  , ' ') "/>
                    <xsl:with-param name="position" select=" substring-after( $num-and-pos , ':')  "/>
                    <xsl:with-param name="last-command" select="$command"/>
                    <xsl:with-param name="current-x" select=" substring-before( substring-before( $num-and-pos , ':')   , ' ') "/>
                    <xsl:with-param name="current-y" select=" substring-after( substring-before( $num-and-pos , ':')   , ' ') "/>
                </xsl:call-template>
            </xsl:when>
            <xsl:when test="$command = 'c' ">
                <!-- relative curveto -->
                <xsl:variable name="new-vml-path" select="concat($vml-path ,' v ' ) "/>
                <xsl:variable name="control-and-pos">
                    <xsl:call-template name="get-number-after">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="$newpos"/>
                        <xsl:with-param name="count" select="4"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="num-and-pos">
                    <xsl:call-template name="get-number-after">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="substring-after( $control-and-pos , ':') "/>
                        <xsl:with-param name="count" select="2"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:call-template name="svgpath2vmlpath">
                    <xsl:with-param name="svg-path" select="$svg-path"/>
                    <xsl:with-param name="vml-path" select=" concat($new-vml-path , substring-before( $control-and-pos , ':')  , ' ' ,  substring-before( $num-and-pos , ':')  , ' ') "/>
                    <xsl:with-param name="position" select=" substring-after( $num-and-pos , ':')  "/>
                    <xsl:with-param name="last-command" select="$command"/>
                    <xsl:with-param name="current-x" select=" substring-before( substring-before( $num-and-pos , ':')  , ' ')  + $current-x  "/>
                    <xsl:with-param name="current-y" select=" substring-after( substring-before( $num-and-pos , ':')    , ' ')  + $current-y "/>
                </xsl:call-template>
            </xsl:when>
            <xsl:when test="$command = 'S' ">
                <!-- absolute shorthand/smooth curveto -->
                <xsl:variable name="new-vml-path" select="concat($vml-path ,' c ' ) "/>
                <xsl:variable name="control-and-pos">
                    <xsl:call-template name="get-number-after">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="$newpos"/>
                        <xsl:with-param name="count" select="2"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="num-and-pos">
                    <xsl:call-template name="get-number-after">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="substring-after( $control-and-pos , ':') "/>
                        <xsl:with-param name="count" select="2"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="control-1">
                    <xsl:choose>
                        <xsl:when test="string-length(translate($last-command, 'CcSs','')  )= 0 ">
                            <xsl:variable name="previous-control-2">
                                <xsl:call-template name="get-number-before">
                                    <xsl:with-param name="svg-path" select="$svg-path"/>
                                    <xsl:with-param name="position" select="$position"/>
                                    <xsl:with-param name="count" select="2"/>
                                    <xsl:with-param name="skipcount" select="2"/>
                                </xsl:call-template>
                            </xsl:variable>
                            <xsl:value-of select="substring-before($previous-control-2 , ':') "/>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:value-of select="substring-before($control-and-pos, ':') "/>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:variable>
                <xsl:call-template name="svgpath2vmlpath">
                    <xsl:with-param name="svg-path" select="$svg-path"/>
                    <xsl:with-param name="vml-path" select=" concat($new-vml-path ,  $control-1 , ' ' ,  substring-before( $control-and-pos , ':')  , ' ' ,  substring-before( $num-and-pos , ':')  , ' ') "/>
                    <xsl:with-param name="position" select=" substring-after( $num-and-pos , ':')  "/>
                    <xsl:with-param name="last-command" select="$command"/>
                    <xsl:with-param name="current-x" select=" substring-before( substring-before( $num-and-pos , ':')   , ' ') "/>
                    <xsl:with-param name="current-y" select=" substring-after( substring-before( $num-and-pos , ':')   , ' ') "/>
                </xsl:call-template>
            </xsl:when>
            <xsl:when test="$command = 's' ">
                <!-- absolute shorthand/smooth curveto -->
                <xsl:variable name="new-vml-path" select="concat($vml-path ,' v ' ) "/>
                <xsl:variable name="control-and-pos">
                    <xsl:call-template name="get-number-after">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="$newpos"/>
                        <xsl:with-param name="count" select="2"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="num-and-pos">
                    <xsl:call-template name="get-number-after">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="substring-after( $control-and-pos , ':') "/>
                        <xsl:with-param name="count" select="2"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="control-1">
                    <xsl:choose>
                        <xsl:when test="string-length(translate($last-command, 'CcSs' , '')) = 0 ">
                            <xsl:variable name="previous-control-2">
                                <xsl:call-template name="get-number-before">
                                    <xsl:with-param name="svg-path" select="$svg-path"/>
                                    <xsl:with-param name="position" select="$position"/>
                                    <xsl:with-param name="count" select="2"/>
                                    <xsl:with-param name="skipcount" select="2"/>
                                </xsl:call-template>
                            </xsl:variable>
                            <xsl:value-of select="substring-before($previous-control-2 , ':') "/>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:value-of select="substring-before($control-and-pos, ':') "/>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:variable>
                <xsl:call-template name="svgpath2vmlpath">
                    <xsl:with-param name="svg-path" select="$svg-path"/>
                    <xsl:with-param name="vml-path" select=" concat($new-vml-path ,  $control-1 , ' ' ,  substring-before( $control-and-pos , ':')  , ' ' ,  substring-before( $num-and-pos , ':')  , ' ') "/>
                    <xsl:with-param name="position" select=" substring-after( $num-and-pos , ':')  "/>
                    <xsl:with-param name="last-command" select="$command"/>
                    <xsl:with-param name="current-x" select=" substring-before( substring-before( $num-and-pos , ':')  , ' ')  + $current-x  "/>
                    <xsl:with-param name="current-y" select=" substring-after( substring-before( $num-and-pos , ':')    , ' ')  + $current-y "/>
                </xsl:call-template>
            </xsl:when>
            <xsl:when test="$command = 'Q' ">
                <!-- absolute quadratic  bézier curves  -->
                <xsl:variable name="new-vml-path" select="concat($vml-path ,' qb ' ) "/>
                <xsl:variable name="control-and-pos">
                    <xsl:call-template name="get-number-after">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="$newpos"/>
                        <xsl:with-param name="count" select="2"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="num-and-pos">
                    <xsl:call-template name="get-number-after">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="substring-after( $control-and-pos , ':') "/>
                        <xsl:with-param name="count" select="2"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:call-template name="svgpath2vmlpath">
                    <xsl:with-param name="svg-path" select="$svg-path"/>
                    <xsl:with-param name="vml-path" select=" concat($new-vml-path , substring-before( $control-and-pos , ':')  , ' ' ,  substring-before( $num-and-pos , ':')  , ' ') "/>
                    <xsl:with-param name="position" select=" substring-after( $num-and-pos , ':')  "/>
                    <xsl:with-param name="last-command" select="$command"/>
                    <xsl:with-param name="current-x" select=" substring-before( substring-before( $num-and-pos , ':')   , ' ') "/>
                    <xsl:with-param name="current-y" select=" substring-after( substring-before( $num-and-pos , ':')   , ' ') "/>
                </xsl:call-template>
            </xsl:when>
            <xsl:when test="$command = 'q' ">
                <!-- relative  quadratic  bézier curves -->
                <xsl:variable name="control-and-pos">
                    <xsl:call-template name="get-number-after">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="$newpos"/>
                        <xsl:with-param name="count" select="2"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="control" select="substring-before( $control-and-pos , ':') "/>
                <xsl:variable name="new-vml-path" select="concat($vml-path ,' qb ' ,  substring-before($control,' ') + $current-x , ' '  , substring-after($control , ' ') + $current-y ) "/>
                <xsl:variable name="num-and-pos">
                    <xsl:call-template name="get-number-after">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="substring-after( $control-and-pos , ':') "/>
                        <xsl:with-param name="count" select="2"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="number" select="substring-before($num-and-pos, ':')"/>
                <xsl:variable name="absolute-number" select="concat(substring-before($number, ' ') + $current-x  , ' ' , substring-after($number, ' ') + $current-y)"/>
                <xsl:call-template name="svgpath2vmlpath">
                    <xsl:with-param name="svg-path" select="$svg-path"/>
                    <xsl:with-param name="vml-path" select=" concat($new-vml-path   , ' ' ,  $absolute-number  , ' ') "/>
                    <xsl:with-param name="position" select=" substring-after( $num-and-pos , ':')  "/>
                    <xsl:with-param name="last-command" select="$command"/>
                    <xsl:with-param name="current-x" select=" substring-before( $absolute-number  , ' ') "/>
                    <xsl:with-param name="current-y" select=" substring-after( $absolute-number   , ' ') "/>
                </xsl:call-template>
            </xsl:when>
            <xsl:when test="$command = 'Z' or $command = 'z' ">
                <!-- closepath -->
                <xsl:variable name="new-vml-path" select="concat($vml-path ,' x ' ) "/>
                <xsl:call-template name="svgpath2vmlpath">
                    <xsl:with-param name="svg-path" select="$svg-path"/>
                    <xsl:with-param name="vml-path" select=" concat($new-vml-path , ' ') "/>
                    <xsl:with-param name="position" select=" $newpos  "/>
                    <xsl:with-param name="last-command" select="$command"/>
                    <xsl:with-param name="current-x" select=" $current-x "/>
                    <xsl:with-param name="current-y" select=" $current-y"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$vml-path"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="get-number-before">
        <!--  get $count number of number before current position , output format:number1 number2 ... numberN:newpostion 
            skip $skipcount of numbers
        -->
        <xsl:param name="svg-path"/>
        <xsl:param name="position" select="1"/>
        <xsl:param name="count" select="1"/>
        <xsl:param name="skipcount" select="0"/>
        <xsl:param name="number" select="''"/>
        <xsl:choose>
            <xsl:when test="$count = 0">
                <xsl:value-of select=" concat($number ,   ':' , $position) "/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:variable name="num-pos">
                    <xsl:call-template name="get-number-position">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="$position"/>
                        <xsl:with-param name="direction" select="-1"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="previous-num-and-pos">
                    <xsl:call-template name="get-previous-number">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="$num-pos"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:if test="$skipcount &gt; 0">
                    <xsl:call-template name="get-number-before">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="substring-after($previous-num-and-pos , ':')"/>
                        <xsl:with-param name="count" select="$count"/>
                        <xsl:with-param name="skipcount" select="$skipcount - 1"/>
                        <xsl:with-param name="number" select="$number"/>
                    </xsl:call-template>
                </xsl:if>
                <xsl:if test="$skipcount = 0">
                    <xsl:variable name="new-number">
                        <xsl:if test="not($count  = 1)">
                            <xsl:value-of select="' '"/>
                        </xsl:if>
                        <xsl:value-of select=" concat( substring-before($previous-num-and-pos , ':')  , $number ) "/>
                    </xsl:variable>
                    <xsl:call-template name="get-number-before">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="substring-after($previous-num-and-pos , ':')"/>
                        <xsl:with-param name="count" select="$count - 1"/>
                        <xsl:with-param name="skipcount" select="0"/>
                        <xsl:with-param name="number" select="$new-number"/>
                    </xsl:call-template>
                </xsl:if>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="get-number-after">
        <!--  get $count number of number after current position, output format:number1 number2 ... numberN:newpostion 
            skip $skipcount of numbers
        -->
        <xsl:param name="svg-path"/>
        <xsl:param name="position" select="1"/>
        <xsl:param name="count" select="1"/>
        <xsl:param name="skipcount" select="0"/>
        <xsl:param name="number" select="''"/>
        <xsl:choose>
            <xsl:when test="$count = 0">
                <xsl:value-of select=" concat($number ,   ':' , $position) "/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:variable name="num-pos">
                    <xsl:call-template name="get-number-position">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="$position"/>
                        <xsl:with-param name="direction" select="1"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="next-num-and-pos">
                    <xsl:call-template name="get-next-number">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="$num-pos"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:if test="$skipcount &gt; 0">
                    <xsl:call-template name="get-number-after">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="substring-after($next-num-and-pos , ':')"/>
                        <xsl:with-param name="count" select="$count"/>
                        <xsl:with-param name="skipcount" select="$skipcount - 1"/>
                        <xsl:with-param name="number" select="$number"/>
                    </xsl:call-template>
                </xsl:if>
                <xsl:if test="$skipcount = 0">
                    <xsl:variable name="new-number">
                        <xsl:value-of select=" concat( $number , substring-before($next-num-and-pos , ':') ) "/>
                        <xsl:if test="not($count  = 1)">
                            <xsl:value-of select="' '"/>
                        </xsl:if>
                    </xsl:variable>
                    <xsl:call-template name="get-number-after">
                        <xsl:with-param name="svg-path" select="$svg-path"/>
                        <xsl:with-param name="position" select="substring-after($next-num-and-pos , ':')"/>
                        <xsl:with-param name="count" select="$count - 1"/>
                        <xsl:with-param name="skipcount" select="0"/>
                        <xsl:with-param name="number" select="$new-number"/>
                    </xsl:call-template>
                </xsl:if>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="get-number-position">
        <!-- get the next number start position, direction should be 1  or -1-->
        <xsl:param name="svg-path"/>
        <xsl:param name="position"/>
        <xsl:param name="direction" select="1"/>
        <xsl:choose>
            <xsl:when test="$direction  = 1 and $position &gt; string-length($svg-path) ">0</xsl:when>
            <xsl:when test="$direction  = -1 and not($position &gt; 0)">0</xsl:when>
            <xsl:otherwise>
                <xsl:variable name="curr-char">
                    <xsl:if test="$direction = 1">
                        <xsl:value-of select="substring($svg-path, $position , 1)"/>
                    </xsl:if>
                    <xsl:if test="$direction = -1">
                        <xsl:value-of select="substring($svg-path, $position -1 , 1)"/>
                    </xsl:if>
                </xsl:variable>
                <xsl:choose>
                    <xsl:when test="string-length(translate($curr-char ,  '+-.0123456789' ,'')) = 0 ">
                        <!-- number start-->
                        <xsl:value-of select="$position"/>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:call-template name="get-number-position">
                            <xsl:with-param name="svg-path" select="$svg-path"/>
                            <xsl:with-param name="position" select="$position + $direction"/>
                            <xsl:with-param name="direction" select="$direction"/>
                        </xsl:call-template>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="get-next-number">
        <!-- get the next number from current position-->
        <xsl:param name="svg-path"/>
        <xsl:param name="position"/>
        <xsl:param name="number" select="''"/>
        <xsl:choose>
            <xsl:when test="$position &gt; string-length($svg-path) ">
                <xsl:value-of select=" concat(round($number) ,  ':' , $position) "/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:variable name="curr-char" select="substring($svg-path, $position , 1)"/>
                <xsl:choose>
                    <xsl:when test="string-length(translate($curr-char ,  '.0123456789' ,'')) = 0 ">
                        <!-- is number -->
                        <xsl:call-template name="get-next-number">
                            <xsl:with-param name="svg-path" select="$svg-path"/>
                            <xsl:with-param name="position" select="$position +1"/>
                            <xsl:with-param name="number" select="concat( $number, $curr-char) "/>
                        </xsl:call-template>
                    </xsl:when>
                    <xsl:when test="string-length(translate($curr-char ,  '+-' ,'') ) = 0  and string-length($number) = 0">
                        <!-- is number -->
                        <xsl:call-template name="get-next-number">
                            <xsl:with-param name="svg-path" select="$svg-path"/>
                            <xsl:with-param name="position" select="$position +1"/>
                            <xsl:with-param name="number" select="concat( $number, $curr-char) "/>
                        </xsl:call-template>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:value-of select="concat( round($number) ,  ':' , $position)"/>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="get-previous-number">
        <!-- get the previous number from current position-->
        <xsl:param name="svg-path"/>
        <xsl:param name="position"/>
        <xsl:param name="number" select="''"/>
        <xsl:choose>
            <xsl:when test="not($position &gt; 0)">
                <xsl:value-of select="concat( round($number ),  ':0')"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:variable name="curr-char" select="substring($svg-path, $position -1 , 1)"/>
                <xsl:choose>
                    <xsl:when test="string-length(translate($curr-char ,  '.0123456789' ,'')) = 0 ">
                        <!-- is number -->
                        <xsl:call-template name="get-previous-number">
                            <xsl:with-param name="svg-path" select="$svg-path"/>
                            <xsl:with-param name="position" select="$position -1"/>
                            <xsl:with-param name="number" select="concat($curr-char ,  $number) "/>
                        </xsl:call-template>
                    </xsl:when>
                    <xsl:when test="string-length(translate($curr-char ,  '+-' ,'') ) = 0  and string-length($number) = 0">
                        <!-- skip it -->
                        <xsl:call-template name="get-previous-number">
                            <xsl:with-param name="svg-path" select="$svg-path"/>
                            <xsl:with-param name="position" select="$position -1"/>
                            <xsl:with-param name="number" select="$number "/>
                        </xsl:call-template>
                    </xsl:when>
                    <xsl:when test="string-length(translate($curr-char ,  '+-' ,'') ) = 0  and string-length($number) &gt; 0">
                        <!-- finsh it with +/- -->
                        <xsl:value-of select="concat( round( concat( $curr-char, $number)) ,  ':' , $position)"/>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:value-of select="concat( round($number) ,  ':' , $position)"/>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="get-path-command">
        <xsl:param name="svg-path"/>
        <xsl:param name="position" select="1"/>
        <xsl:param name="last-command"/>
        <xsl:choose>
            <xsl:when test="$position &gt; string-length($svg-path) "/>
            <xsl:otherwise>
                <xsl:variable name="curr-char" select="substring($svg-path, $position , 1)"/>
                <xsl:choose>
                    <xsl:when test="string-length(translate($curr-char ,  'MmZzLlHhVvCcSsQqTtAa' ,'')) = 0 ">
                        <!-- "MmZzLlHhVvCcSsQqTtAa" are all possiable  command chars -->
                        <xsl:value-of select="concat( $curr-char , ':'  , $position +1)"/>
                    </xsl:when>
                    <xsl:when test="string-length(translate($curr-char ,  '+-.0123456789' ,'')) = 0 ">
                        <!-- number start, use last command -->
                        <xsl:if test="string-length($last-command) = 0">
                            <xsl:message>ooo2wordml_path.xsl: Find undefined command</xsl:message>
                        </xsl:if>
                        <xsl:value-of select="concat( $last-command  , ':'  , $position )"/>
                    </xsl:when>
                    <xsl:when test="string-length(translate($curr-char ,  ',&#9;&#10;&#13;&#32;' ,'')) = 0 ">
                        <!-- space or ',' should be skip -->
                        <xsl:call-template name="get-path-command">
                            <xsl:with-param name="svg-path" select="$svg-path"/>
                            <xsl:with-param name="position" select="$position +1"/>
                            <xsl:with-param name="last-command" select="$last-command"/>
                        </xsl:call-template>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:message>ooo2wordml_path.xsl: Find undefined command:<xsl:value-of select="$curr-char"/>
                        </xsl:message>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
</xsl:stylesheet>
