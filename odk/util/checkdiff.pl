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
        print STDERR "files differ : ".substr( $_, 5 );
        $return++;
    }
    if( /^Binary/ )
    {
        print STDERR "$_";
        $return++;
    }
}
exit $return;
