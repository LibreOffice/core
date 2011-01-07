#!/usr/bin/env bash
#take a .zip containing a flat hierarchy of odf files and strip out the
#language and country tags in each of them and repack it all up
#should convert templates so that documents based on them use
#the default-document-language rather than the hardcoded lang-tag

#All a bit hacky, but it should work

XSL=`mktemp`

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

WRKDIR=`mktemp -d`
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
