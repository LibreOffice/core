: # -*- perl -*-
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;

use File::Temp qw/ tempdir /;
use File::Basename;
use Cwd;

$tempdir = tempdir();
$dir = cwd();

sub unpack_rpm
{
    my ($package) = @_;

    system << "EOF"
rpm --query --queryformat "[trigger%{TRIGGERTYPE} script (through %{TRIGGERSCRIPTPROG}) -- %{TRIGGERNAME}  %{TRIGGERVERSION}\n%{TRIGGERSCRIPTS}\n]" --package $package > triggers
rpm --query --queryformat "%{PREIN}\n" --package $package > prein
rpm --query --queryformat "%{POSTIN}\n" --package $package > postin
rpm --query --queryformat "%{PREUN}\n" --package $package > preun
rpm --query --queryformat "%{POSTUN}\n" --package $package > postun
rpm --query --queryformat "[%{FILEMODES:perms} %{FILEUSERNAME}/%{FILEGROUPNAME} .%{FILENAMES} -> %{FILELINKTOS}\n]" --package $package | sed 's/ -> \$//' | sort --key=3 -o filelist

rpm2cpio $package | cpio --extract --make-directories

rm --force `sed --silent 's|^lrw.r..r..-* root/root \\./\\(.*\\) -> .*|\\1 |p' filelist | tr -d "\\012"`
EOF

# the last step removes all symbolic links from the extracted file tree as they
# are handled by diffing the filelist
}

sub unpack_deb
{
    my ($package) = @_;

    system << "EOF"
ar x $package control.tar.gz data.tar.gz
tar --extract --ungzip --file=control.tar.gz
rm --force control control.tar.gz
tar --extract --ungzip --file=data.tar.gz
tar --list --verbose --ungzip --file=data.tar.gz | sed -e 's| root/root .* \./|- root/root ./|' -e 's|^d\\(.*\\)/\$|d\\1|' | sort --key=3 -o filelist
rm --force data.tar.gz

rm --force `sed --silent 's|^lrw.r..r..- root/root \\./\\(.*\\) -> .*|\\1 |p' filelist | tr -d "\\012"`
EOF

# the last step removes all symbolic links from the extracted file tree as they
# are handled by diffing the filelist
}

sub unpack_solpkg
{
    my ($package) = @_;

    system << "EOF"
sed -e '1 d' -e 's/[0-9][0-9]* [0-9][0-9]* [0-9]\\{10\\}\$//' $package/pkgmap > filelist
grep -v "^PSTAMP=" $package/pkginfo > pkginfo
cp $package/install/* .
if [ -f $package/archive/none.bz2 ]; then
  bzcat $package/archive/none.bz2 | cpio -i -d
else
  cp -pr $package/reloc/* .
fi
EOF
}

sub unpack_tgz {
   my ($package) = @_;

  system << "EOF"
cat $package | gunzip | tar -xf -
EOF
}

my $script = basename($0);

die "Usage: $script <package 1> <package 2>\n" unless $#ARGV == 1;

my @pkgroot = ();

while ( $#ARGV >= 0 )
{
    my $package = shift;

    # make package paths absolute if necessary
    $package = $dir . "/" . $package unless $package =~ /^\//;

    my $basename = basename($package);

    # when comparing identically named packages, append a "-2"
    unless ( mkdir "$tempdir/$basename", 0777 ) {
        $basename = $basename . "-2";
        mkdir "$tempdir/$basename", 0777;
    }

    # change working directory, unpack the package and change back ..
    die "Unable to change to unpack directory $tempdir/$basename: $!\n" unless chdir "$tempdir/$basename";

    if ( $package =~ /\.rpm$/ )   { unpack_rpm( $package ); }
    elsif( $package =~ /\.deb$/ ) { unpack_deb( $package ); }
    elsif( -f "$package/pkgmap" ) { unpack_solpkg( $package ); }
    elsif( $package =~ /\.tgz$/ ) { unpack_tgz( $package ); }

    push @pkgroot, $basename;
    chdir $dir;
}

# print "$0\n";

die "Unable to change to working directory $tempdir: $!\n" unless chdir $tempdir;

system "diff -ru @pkgroot[0] @pkgroot[1]";
system "rm -rf *";

chdir $dir;
rmdir $tempdir;
#print STDERR "rm -rf $tempdir\n";

