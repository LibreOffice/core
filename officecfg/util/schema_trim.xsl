<?xml version="1.0" encoding="utf-8" standalone="yes" ?>
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

<xsl:transform  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
		xmlns:xs="http://www.w3.org/2001/XMLSchema"		
		xmlns:oor="http://openoffice.org/2001/registry"		
		version="1.0">

<!-- Get the correct format -->
	<xsl:output method="xml" indent="yes"/>
	<xsl:namespace-alias stylesheet-prefix="xs" result-prefix="xs"></xsl:namespace-alias>


<!-- Remove all comments from the schema files -->
	<xsl:template match="*|@*">
	  <xsl:copy>
		<xsl:apply-templates select="*|@*"/>
	  </xsl:copy>
	</xsl:template>

<!-- suppress the location of the schema -->
	<xsl:template match = "@xsi:schemaLocation"/>

<!-- suppress the constraints of the schema
	<xsl:template match = "constraints"/>  -->

<!-- suppress all documentation items
	<xsl:template match = "info"/> -->

<!-- suppress constraints for deprecated items -->
	<xsl:template match = "constraints[../info/deprecated]"/>

<!-- suppress all documentation for deprecated items -->
	<xsl:template match = "desc[../deprecated]"/>
	<xsl:template match = "label[../deprecated]"/>

<!-- copy all other documentation with content -->
	<xsl:template match="desc|label">
		<xsl:copy>
			<xsl:apply-templates select="@*"/>
			<xsl:value-of select="."/>
		</xsl:copy>		
	</xsl:template>

<!-- suppress all author items -->
	<xsl:template match = "author"/>

<!-- suppress values, which are marked as nil -->
	<xsl:template match="value[@xsi:nil='true']" />

<!-- copy all other values with content -->
	<xsl:template match="value">
		<xsl:copy>
			<xsl:apply-templates select="@*"/>
			<xsl:copy-of select="node()"/>
		</xsl:copy>		
	</xsl:template>

</xsl:transform>
