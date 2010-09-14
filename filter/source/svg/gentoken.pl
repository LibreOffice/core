# from oox/source/token - should really put this into solenv

$ARGV0 = shift @ARGV;
$ARGV1 = shift @ARGV;
$ARGV2 = shift @ARGV;

open ( TOKENS, $ARGV0 ) || die "can't open token file: $!";
my %tokens;

while ( defined ($line = <TOKENS>) )
{
    if( !($line =~ /^#/) )
    {
        chomp($line);
        @token = split(/\s+/,$line);
        if ( not defined ($token[1]) )
        {
            $token[1] = "XML_".$token[0];
            $token[1] =~ tr/\-\.\:/___/;
            $token[1] =~ s/\+/PLUS/g;
            $token[1] =~ s/\-/MINUS/g;
        }

        $tokens{$token[0]} = uc($token[1]);
    }
}
close ( TOKENS );

open ( HXX, ">$ARGV1" ) || die "can't open tokens.hxx file: $!";
open ( GPERF, ">$ARGV2" ) || die "can't open tokens.gperf file: $!";

print ( GPERF "%language=C++\n" );
print ( GPERF "%global-table\n" );
print ( GPERF "%null-strings\n" );
print ( GPERF "%struct-type\n" );
print ( GPERF "struct xmltoken\n" );
print ( GPERF "{\n" );
print ( GPERF "  const sal_Char *name; sal_Int32 nToken; \n" );
print ( GPERF "};\n" );
print ( GPERF "%%\n" );

print ( HXX "#ifndef INCLUDED_AUTOGEN_TOKEN_HXX\n" );
print ( HXX "#define INCLUDED_AUTOGEN_TOKEN_HXX\n\n" );
print ( HXX "#include <sal/types.h>\n\n" );

$i = 0;
foreach( sort(keys(%tokens)) )
{
    print( HXX "const sal_Int32 $tokens{$_} = $i;\n" );
    print( GPERF "$_,$tokens{$_}\n" );
    $i = $i + 1;
}
print ( GPERF "%%\n" );
print ( HXX "const sal_Int32 XML_TOKEN_COUNT = $i;\n" );
print ( HXX "const sal_Int32 XML_TOKEN_INVALID = -1;\n\n" );
print ( HXX "#endif\n" );
close ( HXX );
close ( GPERF );
