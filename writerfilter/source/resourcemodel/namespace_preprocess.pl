#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$ARGV0 = shift @ARGV;

print <<EOF;
<?xml version="1.0"?>
<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="xml"/>

  <xsl:include href="./modelpreprocess.xsl"/>

  <xsl:template match="namespace-alias[\@id]">
    <xsl:variable name="value">
      <xsl:call-template name="getnamespaceid">
        <xsl:with-param name="id" select="\@id" />
      </xsl:call-template>
    </xsl:variable>
    <xsl:copy>
      <xsl:apply-templates select="@*"/>
      <xsl:attribute name="id">
        <xsl:value-of select="\$value"/>
      </xsl:attribute>
    </xsl:copy>
  </xsl:template>

  <xsl:template name="getnamespaceid">
    <xsl:param name='id'/>
    <xsl:choose>
EOF


# print the mapping
open ( NAMESPACES, $ARGV0 ) || die "can't open namespace file: $!";
while ( <NAMESPACES> )
{
    chomp( $_ );
    # line format is: numeric-id short-name namespace-URL
    $_ =~ /^([0-9]+)\s+([a-zA-Z]+)\s+([a-zA-Z0-9-.:\/]+)\s*$/ or die "Error: invalid character in input data";
    print <<EOF;
      <xsl:when test="\$id = '$2'">
        <xsl:text>$1</xsl:text>
      </xsl:when>
EOF
}

print <<EOF;
    </xsl:choose>
  </xsl:template>

</xsl:stylesheet>
EOF
