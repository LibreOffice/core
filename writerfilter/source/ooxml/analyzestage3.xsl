<?xml version="1.0" encoding="UTF-8"?>
<!--
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
-->

<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:key name="qnames" match="/stage2/qnames/qname" use="@qname"/>

<xsl:template match="/">
	<stage3 xmlns:rng="http://relaxng.org/ns/structure/1.0">
	<xsl:for-each select="/stage2/analyze">
		<xsl:for-each select="attribute|element">
			<xsl:variable name="qname" select="@qname"/>
			<xsl:copy>
				<xsl:copy-of select="@*"/>
					<xsl:for-each select="key('qnames', @qname)">
						<file>
							<xsl:attribute name="name">
								<xsl:value-of select="@file"/>
							</xsl:attribute>
							<xsl:attribute name="line">
								<xsl:value-of select="@line"/>
							</xsl:attribute>
                            <xsl:for-each select="ancestor::qnames/status|ancestor::qnames/nostatus">
                                <xsl:copy>
                                    <xsl:copy-of select="@*"/>
                                    <xsl:attribute name="qname-count">
                                        <xsl:value-of select="count(ancestor::qnames/qname)"/>
                                    </xsl:attribute>
                                </xsl:copy>
                            </xsl:for-each>
						</file>
					</xsl:for-each>
			</xsl:copy>
		</xsl:for-each>
	</xsl:for-each>
	</stage3>
</xsl:template>

</xsl:stylesheet>	