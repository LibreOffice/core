#
# convertlinks - a perl script to make hrefs to
# http://api.openoffice.org/common/ref relativ.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#

use File::Find;

# for the convenience of &wanted calls, including -eval statements:
use vars qw/*name *dir/;
*name   = *File::Find::name;
*dir    = *File::Find::dir;
@files = ();

if($#ARGV == 1)
{
    $pattern = "www";
} else
{
    $pattern = $ARGV[2];
}

find(\&wanted, "$ARGV[0]");

$return = 1;

foreach $i (@files)
{
    next if( $i->{directory} =~ /.*common((\/|\\)ref(.*))/ ||
         $i->{directory} =~ /.*cpp((\/|\\)ref(.*))/ ||
         $i->{directory} =~ /.*java((\/|\\)ref(.*))/ );

    open ( FILEIN, $i->{filename} ) || die "could not open $i->{filename} for reading";

    $relPath = ".";
    $relToSource = ".";
    if( $i->{directory} =~ /.*$pattern((\/|\\)(.*))/ )
    {
        $relPath = $3;
        $relPath =~ s#\w+#\.\.#go;
        if($pattern eq "examples")
        {
            $relPath = "\.\.\/$relPath";
        }
        if($pattern eq "www")
        {
            $relToSource = "\.\.\/$relPath";
        } else
        {
            $relToSource = $relPath;
        }
    } else
    {
        if($pattern eq "examples")
        {
            $relPath = "\.\.";
        }
        if($pattern eq "www")
        {
            $relToSource = "\.\.";
        } else
        {
            $relToSource = $relPath;
        }
    }

    @lines = <FILEIN>;
    close( FILEIN );
    open( FILEOUT, ">$i->{filename}.tmp" ) || die "could not open $i->{filename} for writing";
    foreach $_ (@lines)
    {
        # change the refenreces to the index in dependency of UDK or ODK
        if("$ARGV[1]" eq "udk_" | "$ARGV[1]" eq "odk_")
        {
            s#((\")(index.html\"))#$2$ARGV[1]$3#go;
            s#((\/|\")(faq.html\"))#$2$ARGV[1]$3#go;
            s#((\/|\")(bylaws.html\"))#$2$ARGV[1]$3#go;
        }

        s#((http:\/\/api\.openoffice\.org\/)(common\/ref[^\"]+))#$relPath\/$3#go;
        s#((http:\/\/api\.openoffice\.org\/unbranded-source\/)(.*)(examples\/examples.html))#$relToSource\/$4#go;

        if($pattern eq "examples")
        {
            # change the links for the C++/Java examples in the ODK
            s#((http:\/\/api\.openoffice\.org\/source\/browse\/api\/odk\/examples\/)(java\/*))#$3#go;
            s#((http:\/\/api\.openoffice\.org\/source\/browse\/api\/odk\/examples\/)(cpp\/*))#$3#go;
            s#((http:\/\/api\.openoffice\.org\/source\/browse\/api\/odk\/examples\/)(basic\/*))#$3#go;
            s#((http:\/\/api\.openoffice\.org\/source\/browse\/api\/odk\/examples\/)(OLE\/*))#$3#go;

            # change link api specific stuff
            s#((http:\/\/api\.openoffice\.org\/)(design_guide.html))#$relPath\/www\/$3#go;
            s#(http:\/\/api\.openoffice\.org\/index.html)#$relPath\/www\/odk_index.html#go;

            # change the links for the C++ examples in the UDK
            s#((http:\/\/udk\.openoffice\.org\/source\/browse\/udk\/product\/examples\/)(cpp\/*))#$3#go;

            # change the links to udk.openoffice.org to relativ links
            s#(http:\/\/udk\.openoffice\.org\/index.html)#$relPath\/www\/udk_index.html#go;
            s#((http:\/\/udk\.openoffice\.org)(\/*))#$relPath\/www$3#go;

            # change the link to tutorial
            s#((http:\/\/api\.openoffice\.org\/)(basic\/man\/tutorial\/tutorial.pdf))#$relPath\/www\/$3#go;
        }
        print FILEOUT $_;
    }
    close FILEOUT;
    chmod 0666, $i->{filename};
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
