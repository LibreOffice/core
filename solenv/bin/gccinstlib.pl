:
eval 'exec perl -wS $0 ${1+"$@"}'
   if 0;
#*************************************************************************
#
#   $RCSfile: gccinstlib.pl,v $
#
# ... massive header block to go here ...
#
#*************************************************************************

%SearchDirs = GetGccSearchDirs ();

$LibPaths = $SearchDirs{'libraries'} || die 'Foo';

@Paths = split(':', $LibPaths);

$Dest = pop(@ARGV) || die "No destination to copy to";

if ($Dest =~ /--help/) {
    printf ("Syntax:\n  gcc-copy <libraries-in-libpath> <dest-directory>\n");
    exit (0);
}

foreach $Path (@Paths) {
    foreach $File (@ARGV) {
        if ( -e $Path.$File) {
            push (@CopySrc, $Path.$File);
        }
    }
}

foreach $Src (@CopySrc) {
    printf "copy $Src to $Dest\n";
    system ("/bin/cp $Src $Dest") && die "copy failed: $!";
}

exit (0);

sub GetGccSearchDirs {
    my %Dirs = ();
    my $cc;

    $cc = $ENV{'CC'} || die "No CC environment set";

    open (GCCOut, "$cc -print-search-dirs|") || die "Failed to exec $cc -print-search-dirs: $!";

    while (<GCCOut>) {
        if (/^([a-zA-Z]+): [=]{0,1}(.*)/) {
            $Dirs{$1} = $2;
        }
    }

    close (GCCOut);

    return %Dirs;
}
