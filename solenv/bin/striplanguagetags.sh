#!/usr/bin/env bash
#take a .zip containing a flat hierarchy of odf files and strip out the
#language and country tags in each of them and repack it all up
#should convert templates so that documents based on them use
#the default-document-language rather than the hardcoded lang-tag

#All a bit hacky, but it should work

tempfoo=`basename $0`

XSL=`mktemp /tmp/${tempfoo}.XXXXXX`

# On Windows, xsltproc is a non-Cygwin program, so we can't pass
# a Cygwin /tmp path to it
[ "$COM" == MSC ] && XSL=`cygpath -m -s $XSL`

if [ $? -ne 0 ]; then
    echo "$0: Can't create temp file, exiting..."
    exit 1
fi
WRKDIR=`mktemp -d /tmp/${tempfoo}.XXXXXX`
if [ $? -ne 0 ]; then
    echo "$0: Can't create temp dir, exiting..."
    exit 1
fi

cat > $XSL << EOF
<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format" version="1.0">

<xsl:template match="node()|@*">
  <xsl:copy>
    <xsl:apply-templates select="@*|node()"/>
  </xsl:copy>
</xsl:template>

<xsl:template match="@fo:language"/>
<xsl:template match="@fo:country"/>

</xsl:stylesheet>
EOF

unzip -q $1 -d $WRKDIR
pushd $WRKDIR
for a in *; do
    unzip -qc $a styles.xml > styles.tmp
    eval "$CALLXSLTPROC -o styles.xml $XSL styles.tmp"
    zip -qr $a styles.xml
    rm styles.xml styles.tmp
done
popd
zip -qrj $1 $WRKDIR
rm -rf $WRKDIR
rm -f $XSL
