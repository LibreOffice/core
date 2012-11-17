:
    eval 'exec perl -S $0 ${1+"$@"}'
        if 0;

use strict;
use Cwd ('cwd', 'realpath');

sub clean()
{
    system ("rm -Rf autom4te.cache");
    system ("rm -f missing install-sh mkinstalldirs libtool ltmain.sh");
    print "cleaned the build tree\n";
}

my $aclocal;

# check we have various vital tools
sub sanity_checks($)
{
    my $system = shift;
    my @path = split (':', $ENV{'PATH'});
    my %required =
      (
       'pkg-config' => "pkg-config is required to be installed",
       'autoconf'   => "autoconf is required",
       $aclocal     => "$aclocal is required",
      );

    for my $elem (@path) {
        for my $app (keys %required) {
            if (-f "$elem/$app") {
                delete $required{$app};
            }
        }
    }
    if ((keys %required) > 0) {
        print ("Various low-level dependencies are missing, please install them:\n");
        for my $app (keys %required) {
            print "\t $app: " . $required{$app} . "\n";
        }
        exit (1);
    }
}

# one argument per line
sub read_args($)
{
    my $file = shift;
    my $fh;
    my @lst;
    open ($fh, $file) || die "can't open file: $file";
    while (<$fh>) {
        chomp();
        # migrate from the old system
        if ( substr($_, 0, 1) eq "'" ) {
            print "Migrating options from the old autogen.lastrun format, using:\n";
            my @opts;
            @opts = split(/'/);
            foreach my $opt (@opts) {
                if ( substr($opt, 0, 1) eq "-" ) {
                    push @lst, $opt;
                    print "  $opt\n";
                }
            }
        } elsif ( substr($_, 0, 1) eq "#" ) {
            # comment
        } else {
            push @lst, $_;
        }
    }
    close ($fh);
    # print "read args from file '$file': @lst\n";
    return @lst;
}

sub invalid_distro($$)
{
    my ($config, $distro) = @_;
    print STDERR "Can't find distro option set: $config\nThis is not necessarily a problem.\n";
    print STDERR "Distros with distro option sets are:\n";
    my $dirh;
    opendir ($dirh, "distro-configs");
    while (($_ = readdir ($dirh))) {
        /(.*)\.conf$/ || next;
        print STDERR "\t$1\n";
    }
    closedir ($dirh);
}

# Handle help arguments first, so we don't clobber autogen.lastrun
for my $arg (@ARGV) {
    if ($arg =~ /^(--help|-h|-\?)$/) {
        system ("./configure --help");
        exit;
    }
}

my @cmdline_args = ();
if (!@ARGV) {
    my $lastrun = "autogen.lastrun";
    @cmdline_args = read_args ($lastrun) if (-f $lastrun);
} else {
    @cmdline_args = @ARGV;
}

my @args;
my $default_config = "distro-configs/default.conf";
if (-f $default_config) {
    print STDERR "Reading default config file: $default_config\n";
    push @args, read_args ($default_config);
}
for my $arg (@cmdline_args) {
    if ($arg eq '--clean') {
        clean();
    } elsif ($arg =~ m/--with-distro=(.*)$/) {
        my $config = "distro-configs/$1.conf";
        if (! -f $config) {
            invalid_distro ($config, $1);
        } else {
            push @args, read_args ($config);
        }
    } else {
        push @args, $arg;
    }
}
for my $arg (@args) {
    if ($arg =~ /^([A-Z]+)=(.*)/) {
        $ENV{$1} = $2;
    }
}

# Alloc $ACLOCAL to specify which aclocal to use
$aclocal = $ENV{ACLOCAL} ? $ENV{ACLOCAL} : 'aclocal';

my $system = `uname -s`;
chomp $system;

sanity_checks ($system) unless($system eq 'Darwin');

# since this looks crazy, if you have a symlink on a path up to and including
# the current directory, we need our configure to run in the realpath of that
# such that compiled (realpath'd) dependency filenames match the filenames
# used in our makefiles - ie. this gets dependencies right via SRC_ROOT
my $cwd_str = realpath(cwd());
chdir ($cwd_str);
# more amazingly, if you don't clobber 'PWD' shells will re-assert their
# old path from the environment, not cwd.
$ENV{PWD} = $cwd_str;

my $aclocal_flags = $ENV{ACLOCAL_FLAGS};

$aclocal_flags .= " -I ./m4";
$aclocal_flags .= " -I ./m4/mac" if ($system eq 'Darwin');

$ENV{AUTOMAKE_EXTRA_FLAGS} = '--warnings=no-portability' if (!($system eq 'Darwin'));

system ("$aclocal $aclocal_flags") && die "Failed to run aclocal";
unlink ("configure");
system ("autoconf") && die "Failed to run autoconf";
die "failed to generate configure" if (! -x "configure");

if (defined $ENV{NOCONFIGURE}) {
    print "Skipping configure process.";
} else {
    # Save autogen.lastrun only if we did get some arguments on the command-line
    if (@ARGV) {
        if (scalar(@cmdline_args) > 0) {
            # if there's already an autogen.lastrun, make a backup first
            if (-e "autogen.lastrun") {
                open (my $fh, "autogen.lastrun") || warn "can't open autogen.lastrun. \n";
                open (BAK, ">autogen.lastrun.bak") || warn "can't create backup file. \n";
                while (<$fh>) {
                    print BAK;
                }
                close (BAK) && close ($fh);
            }
            # print "writing args to autogen.lastrun\n";
            my $fh;
            open ($fh, ">autogen.lastrun") || die "can't open autogen.lastrun: $!";
            for my $arg (@cmdline_args) {
                print $fh "$arg\n";
            }
            close ($fh);
        }
    }
    elsif ( ! -e "autogen.lastrun")
    {
        open (my $fh, ">autogen.lastrun") || die "can't create autogen.lastrun";
        close ($fh);
    }
    print "running ./configure with '" . join ("' '", @args), "'\n";
    system ("./configure", @args) && die "Error running configure";
}

# Local Variables:
# mode: perl
# cperl-indent-level: 4
# tab-width: 4
# indent-tabs-mode: nil
# End:

# vim:set ft=perl shiftwidth=4 softtabstop=4 expandtab: #
