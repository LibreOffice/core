#!/bin/bash

SEARCHIN=$SRC_ROOT/writerfilter/source/dmapper

echo '<?xml version="1.0"?>'
echo "<analyze>"

xsltproc analyzemodel.xsl model.xml | tail -n +2

mdfind -onlyin $SEARCHIN "case NS_" |
xargs -J % grep -A 1 -n "case NS_" % |
grep -v "^//" |
grep "cxx" |
sed 's#'$SEARCHIN'##' |
sed 's#\(^[^:]*\):\([0-9]*\):#<qname file="\1" line="\2"#' |
sed 's#[/* ]*case \(NS_.*\):.*# tokenid="\1"/>#' |
sed 's#.*WRITERFILTERSTATUS: done: \([0-9]*\), planned: \([0-9]*\), spent: \([0-9]*\).*#<status done="\1" planned="\2" spent="\3"/>#' |
sed 's#^.*-[0-9][0-9]*-.*$#<nostatus/>#' |
grep -v "^--"

echo "</analyze>"