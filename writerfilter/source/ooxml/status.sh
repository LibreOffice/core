#!/bin/bash

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
