#!/usr/bin/perl

use strict;
use File::Find ();
use File::Basename ();
use File::Path ();
use Getopt::Std;
use Cwd;

my ($startdir,$outdir,$pfx);

sub usage() {
    print STDERR "usage: $0 -o <outdir> [-f <file>]\n"
}

$startdir=cwd();

# for the convenience of &wanted calls, including -eval statements:
use vars qw/*name *dir *prune/;
*name   = *File::Find::name;
*dir    = *File::Find::dir;
*prune  = *File::Find::prune;

if ( !getopts('o:f:') ) {
    usage();
    exit(1);
}

if ( defined($Getopt::Std::opt_o) ) {
    $outdir=$Getopt::Std::opt_o;
    $outdir=~s%\\%/%g;
} else {
    usage();
    exit(1);
}

if ( defined($Getopt::Std::opt_f) ) {
    convertfile($outdir,$Getopt::Std::opt_f);
} else {
    # Traverse desired filesystems
    $pfx="source";
    File::Find::find({wanted => \&wanted}, 'source');
}
exit;


sub wanted {
    my ($dev,$ino,$mode,$nlink,$uid,$gid);

    (
    /^.*\.html\z/s
    ||
        /^license\.txt\z/s
        ||
        /^LICENSE\z/s
        ||
        /^readme\.txt\z/s
        ||
        /^README\z/s
    ) &&
    ($nlink || (($dev,$ino,$mode,$nlink,$uid,$gid) = lstat($_))) &&
    eval {&doconv};
}

sub doconv {
    my($dest);
    $dest=$dir;
    $dest=~s/^$pfx//g;
    $dest=$outdir.$dest;
    convertfile($dest,$name);
}

sub convertfile {
    my ($dest,$file,$destfile);
    $dest=shift;
    $file=shift;
    $dest=~s%^./%%g;
    $dest=$startdir . "/" . $dest;
    $file=$startdir . "/" . $file;

    $destfile=$dest . "/" . File::Basename::basename($file);

    File::Path::mkpath($dest,0,0755);

    open(IN,"<$file") || die "can not open $file";
    open(OUT,">$destfile") || die "can not open $destfile";

    while (<IN> ) {
        chop while ( /\n$/ || /\r$/ );
        print OUT "$_\n";
    }
    close(IN);
    close(OUT);
}



