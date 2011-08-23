$ARGV = shift @ARGV;
my %tokens;

my @files = glob("$ARGV/*.rnc");

open( TOKEN, ">tokens.txt" ) || die "can't write token file";

foreach( @files )
{
    print( "parsing $_\n" );
    open ( XSD, $_ ) || die "can't open token file: $!";
    while( <XSD> )
    {
        chomp($_);
        if( /element (\S*:)?(\S*)/ )
        {
            $tokens{$2} = 1;
            print(".");
        }
        elsif( /attribute (\S*:)?(\S*)/ )
        {
            $tokens{$2} = 1;
            print(".");
        }
        elsif( /list\s*\{/ )
        {
            while( <XSD> )
            {
                chomp($_);
                last if( /^\s*\}/ );
                if( /"(\S*?)\"/ )
                {
                    $tokens{$1} = 1;
                    print(".");
                }
            }
        }
    }
    close ( XSD );

    print("\n" );
}

foreach( sort(keys(%tokens)) )
{
    print TOKEN "$_\n";
}
close( TOKEN );
