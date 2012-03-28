<?xml version="1.0" encoding="UTF-8"?>
<!--***********************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 ***********************************************************-->

<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:rng="http://relaxng.org/ns/structure/1.0">

<xsl:output method="xml" indent="yes"/>

<xsl:template match="/">
    <todo>
    <xsl:for-each select="/stage3">
        <xsl:for-each select="attribute|element">
            <xsl:choose>
                <xsl:when test="@qname and @resource='Properties' and not(file)">
                    <xsl:copy-of select="."/>
                </xsl:when>
                <xsl:when test="@qname and file/status[number(@done) &lt; 100 and number(@planned) &gt; 0]">
                    <xsl:copy>
                        <xsl:copy-of select="@*"/>
                        <xsl:copy-of select="file[status[number(@done) &lt; 100 and number(@planned) &gt; 0]]"/>
                    </xsl:copy>
                </xsl:when>
            </xsl:choose>
        </xsl:for-each>
    </xsl:for-each>
    </todo>
</xsl:template>

</xsl:stylesheet>