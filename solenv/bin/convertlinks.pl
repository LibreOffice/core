#
# convertlinks - a perl script to make hrefs to
# http://api.openoffice.org/common/ref relativ.
#
# Copyright (c) 2000 Sun Microsystems, Inc.
#

use File::Find;

# for the convenience of &wanted calls, including -eval statements:
use vars qw/*name *dir/;
*name   = *File::Find::name;
*dir    = *File::Find::dir;
@files = ();

find(\&wanted, "$ARGV[0]");

$return = 1;
foreach $i (@files)
{
    open ( FILEIN, $i->{filename} ) || die "could not open $i->{filename} for reading";

    $relPath = ".";
    if( $i->{directory} =~ /.*www((\/|\\)(.*))/ )
    {
        $relPath = $3;
        $relPath =~ s#\w+#\.\.#go
    }

    @lines = <FILEIN>;
    close( FILEIN );
    open( FILEOUT, ">$i->{filename}.tmp" ) || die "could not open $i->{filename} for writing";
    foreach $_ (@lines)
    {
        if("$ARGV[1]" eq "udk_" | "$ARGV[1]" eq "odk_")
        {
            s#((\")(index.html\"))#$2$ARGV[1]$3#go;
            s#((\/|\")(faq.html\"))#$2$ARGV[1]$3#go;
            s#((\/|\")(bylaws.html\"))#$2$ARGV[1]$3#go;
        }

        s#((http:\/\/api\.openoffice\.org\/)(common\/ref[^\"]+))#$relPath\/$3#go;
        print FILEOUT $_;
    }
    close FILEOUT;
    rename "$i->{filename}.tmp", $i->{filename} || die "could not rename $i->{filename}.tmp to $i->{filename}";
    $return = 0;
}

exit $return;


sub wanted {
    %file = (
        directory => $dir,
        filename  => $name
    );
    push @files, {%file} if /^.*\.html\z/s;
}