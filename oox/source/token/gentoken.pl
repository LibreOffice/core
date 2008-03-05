$ARGV0 = shift @ARGV;
$ARGV1 = shift @ARGV;
$ARGV2 = shift @ARGV;
$ARGV3 = shift @ARGV;

open ( TOKENS, $ARGV0 ) || die "can't open token file: $!";
my %tokens;

while ( <TOKENS> )
{
    chomp( $_ );
    $_ =~ s/\s*//g;
    $_ =~ /^[a-zA-Z0-9-_]+$/ or die "invalid character in token '$_'";
    $id = "XML_$_";
    $id =~ s/-/_/g;
    $tokens{$_} = $id;
}
close ( TOKENS );

open ( HXX, ">$ARGV1" ) or die "can't open tokens.hxx file: $!";
open ( WORDS, ">$ARGV2" ) or die "can't open tokenwords.inl file: $!";
open ( GPERF, ">$ARGV3" ) or die "can't open tokens.gperf file: $!";

print ( HXX "#ifndef OOX_TOKENS_HXX\n" );
print ( HXX "#define OOX_TOKENS_HXX\n\n" );
print ( HXX "#include <sal/types.h>\n" );

print ( WORDS "static const sal_Char* tokentowordlist[] = {\n" );

print ( GPERF "%language=C++\n" );
print ( GPERF "%global-table\n" );
print ( GPERF "%null-strings\n" );
print ( GPERF "%struct-type\n" );
print ( GPERF "struct xmltoken {\n" );
print ( GPERF "    const sal_Char *name;\n" );
print ( GPERF "    sal_Int32 nToken;\n" );
print ( GPERF "};\n" );
print ( GPERF "%%\n" );

$i = 0;
foreach( sort( keys( %tokens ) ) )
{
    print ( HXX "const sal_Int32 $tokens{$_} = $i;\n" );
    print ( WORDS "    \"$_\",\n" );
    print ( GPERF "$_,$tokens{$_}\n" );
    ++$i;
}

print ( HXX "const sal_Int32 XML_TOKEN_COUNT = $i;\n" );
print ( HXX "const sal_Int32 XML_TOKEN_INVALID = -1;\n\n" );
print ( HXX "#endif\n" );

print ( WORDS "    \"\"\n" );
print ( WORDS "};\n" );

print ( GPERF "%%\n" );

close ( HXX );
close ( WORDS );
close ( GPERF );
