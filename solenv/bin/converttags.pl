#
# converttags - a perl script to coonvert some predefined tags
# to user specified values
#
# Copyright (c) 2000 Sun Microsystems, Inc.
#

if($#ARGV == -1)
{
    die "No parameters were specified.\nperl converttags.pl <mode> <title> <productname> [<color1>] [<color2>] file_1 [... file_n]\n";
}
if($#ARGV < 2)
{
    die "No file were specified -> no file must be converted!\n";
}

# mode = 1 -> convert
#      = 2 -> exit without conversion
$mode = shift @ARGV;

$title = shift @ARGV;
$productname = shift @ARGV;

$color1 = "";
$color2 = "";

if( $mode =~ s/2/$1/go )
{
    exit 0;
}

if( $ARGV[0] =~ s/(#[\w]{6})/$1/go )
{
    $color1 = shift @ARGV;
}
if( $ARGV[0] =~ s/(#[\w]{6})/$1/go )
{
    $color2 = shift @ARGV;
}

print "$title\n";
print "$productname\n";
print "$color1\n";
print "$color2\n";

$return = 0;

while (@ARGV)
{
    my $lineCount = 0;
    $ARGV = shift @ARGV;
    print "convert tags: $ARGV ";

    open ( FILEIN, $ARGV ) || die "could not open $ARGV for reading";
    @lines = <FILEIN>;
    close( FILEIN );
    open( FILEOUT, ">$ARGV.tmp" ) || die "could not open $ARGV.tmp for writing";


    foreach $_ (@lines)
    {
    $lineCount++;
    if ( $lineCount == 10 )
    {
        $lineCount = 0;
        print ".";
    }
    # change [TITLE] tag
    s#\[TITLE\]#$title#go;

    # change [PRODUCTNAME] tag
    s#\[PRODUCTNAME\]#$productname#go;

    # change color #003399 to #$color1 if color1 was specified!
    if ( ! "$color1" eq "" )
    {
        s/#003399/$color1/go;
    }

    # change color #99CCFF to #$color2 if color2 was specified!
    if ( ! "$color2" eq "" )
    {
        s/#99CCFF/$color2/go;
    }
    print FILEOUT $_;
    }
    print " OK\n";

    close FILEOUT;
    chmod 0666, $ARGV;
    rename "$ARGV.tmp", $ARGV || die "could not rename $ARGV.tmp to $ARGV";
}

exit $return;
