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
$group = 0;
$i = 1;
while ( <NAMESPACES> )
{
    chomp( $_ );
    $_ =~ s/\s*//g;
    if ( $_ =~ m/^$/ )
    {
        # Start a new group
        $i = 0;
        $group++;
    } 
    elsif ( $_ =~ m/^[^#]/ )
    {
        # Neither an empty line nor a comment
        $_ =~ /^[a-zA-Z0-9-_]+$/ or die "Invalid namespace token $_";
        $_ =~ s/-/_/g;
        $no = $group*10 + $i;
        print <<EOF;
      <xsl:when test="\$id = '$_'">
        <xsl:text>$no</xsl:text>
      </xsl:when>
EOF
        ++$i;
    }
}

print <<EOF;
    </xsl:choose>
  </xsl:template>

</xsl:stylesheet>
EOF
