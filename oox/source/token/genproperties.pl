$ARGV0 = shift @ARGV;
$ARGV1 = shift @ARGV;
$ARGV2 = shift @ARGV;

open ( PROPS, $ARGV0 ) || die "can't open properties source file: $!";
my %props;

while ( <PROPS> )
{
    chomp( $_ );
    $_ =~ s/\s*//g;
    $_ =~ /^[A-Z][a-zA-Z0-9]*$/ or die "invalid character in property '$_'";
    $id = "PROP_$_";
    $props{$_} = $id;
}
close ( TOKENS );

open ( HXX, ">$ARGV1" ) or die "can't open properties.hxx file: $!";
open ( WORDS, ">$ARGV2" ) or die "can't open propertynames.inc file: $!";

print ( HXX "#ifndef OOX_PROPERTIES_HXX\n" );
print ( HXX "#define OOX_PROPERTIES_HXX\n\n" );
print ( HXX "#include <sal/types.h>\n" );
print ( HXX "namespace oox {\n\n" );

print ( WORDS "static const sal_Char* propertywordlist[] = {\n" );

$i = 0;
foreach( sort( keys( %props ) ) )
{
    print ( HXX "const sal_Int32 $props{$_} = $i;\n" );
    print ( WORDS "    \"$_\",\n" );
    ++$i;
}

print ( HXX "const sal_Int32 PROP_COUNT = $i;\n" );
print ( HXX "const sal_Int32 PROP_INVALID = -1;\n\n" );
print ( HXX "} // namespace oox\n" );
print ( HXX "#endif\n" );

print ( WORDS "    \"\"\n" );
print ( WORDS "};\n" );

close ( HXX );
close ( WORDS );

