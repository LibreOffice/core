:
    eval 'exec perl -S $0 ${1+"$@"}'
        if 0;

use strict;

sub clean()
{
    system ("rm -Rf autom4te.cache");
    system ("rm -f missing install-sh mkinstalldirs libtool ltmain.sh");
    print "cleaned the build tree\n";
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
    }
    else {
        push @lst, $_;
    }
    }
    close ($fh);
#    print "read args from file '$file': @lst\n";
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

my @cmdline_args = ();
if (!@ARGV) {
    my $lastrun = "autogen.lastrun";
    @cmdline_args = read_args ($lastrun) if (-f $lastrun);
} else {
    @cmdline_args = @ARGV;
}

my @args;
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

system ("touch ChangeLog");

my $system = `uname -s`;
chomp $system;

my $aclocal_flags = $ENV{ACLOCAL_FLAGS};

$aclocal_flags = "-I ./m4/mac" if (($aclocal_flags eq "") && ($system eq 'Darwin'));

$ENV{AUTOMAKE_EXTRA_FLAGS} = '--warnings=no-portability' if (!($system eq 'Darwin'));

system ("aclocal $aclocal_flags") && die "Failed to run aclocal";
unlink ("configure");
system ("autoconf") && die "Failed to run autoconf";
die "failed to generate configure" if (! -x "configure");

if (defined $ENV{NOCONFIGURE}) {
    print "Skipping configure process.";
} else {
    # Save autogen.lastrun only if we did get some arguments on the command-line
    if (@ARGV) {
        if ($#cmdline_args > 0) {
            # print "writing args to autogen.lastrun\n";
            my $fh;
            open ($fh, ">autogen.lastrun") || die "can't open autogen.lastrun: $!";
            for my $arg (@cmdline_args) {
                print $fh "$arg\n";
            }
            close ($fh);
        }
    }
    print "running ./configure with '" . join ("' '", @args), "'\n";
    system ("./configure", @args);
}
