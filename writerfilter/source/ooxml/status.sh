#!/bin/sh
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

SEARCHIN=$SRC_ROOT/writerfilter/source/dmapper

(
echo '<?xml version="1.0"?>'
echo "<stage1>"

xsltproc analyzemodel.xsl model.xml | tail -n +2

find $SEARCHIN -name "*.cxx" -exec grep -nH -A 1 "case NS_" {} \; |
grep -v "//.*case NS_" |
sed 's#'$SEARCHIN'##' |
sed 's#\(^[^:]*\):\([0-9]*\):#<qname file="\1" line="\2"#' |
sed 's#[/* ]*case \(NS_.*\):.*# qname="\1"/>#' |
sed 's#.*WRITERFILTERSTATUS: done: \([0-9]*\), planned: \([0-9.]*\), spent: \([0-9.]*\).*#<status done="\1" planned="\2" spent="\3"/>#' |
sed 's#.*WRITERFILTERSTATUS:.*#<status done="100"/>#' |
sed 's#^.*-[0-9][0-9]*-.*$#<nostatus/>#' |
grep -v "^--"

echo "</stage1>"
) |
xsltproc analyzestage2.xsl - |
xsltproc analyzestage3.xsl -
