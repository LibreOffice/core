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
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	
<xsl:template match="/">
    <stage2>
        <xsl:copy-of select="/stage1/analyze"/>
        <xsl:for-each select="/stage1/qname">
            <xsl:if test="not(local-name(preceding-sibling::*[1]) = 'qname')">
                <qnames>
                    <xsl:call-template name="handleqnames"/>
                </qnames>
            </xsl:if>
        </xsl:for-each>
    </stage2>
</xsl:template>

<xsl:template name="handleqnames">
    <xsl:choose>
        <xsl:when test="local-name() = 'qname'">
            <xsl:copy-of select="."/>
            <xsl:for-each select="following-sibling::*[1]">
                <xsl:call-template name="handleqnames"/>
            </xsl:for-each>
        </xsl:when>
        <xsl:when test="local-name() = 'nostatus' or local-name() = 'status'">
            <xsl:copy-of select="."/>
        </xsl:when>
    </xsl:choose>
</xsl:template>
</xsl:stylesheet>