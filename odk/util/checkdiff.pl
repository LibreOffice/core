#
# checkit - a perl script to check the output of 'dircmp'
#
# Copyright (c) 2000 Sun Microsystems, Inc.
#

$return = 0;
while( <STDIN> )
{
    if( /^diff/ )
    {
        print STDERR "ERROR : files differ ".substr( $_, 5 );
        $return++;
    }
    if( /^Binary/ )
    {
        print STDERR "ERROR : $_";
        $return++;
    }
}
if( $return != 0 )
{
    unlink "$ARGV[0]";
}
exit $return;
