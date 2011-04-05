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
    push @lst, $_;
    }
    close ($fh);
#    print "read args from file '$file': @lst\n";
    return @lst;
}

sub invalid_distro($$)
{
    my ($config, $distro) = @_;
    print STDERR "can't find distro option set: $config\n";
    print STDERR "valid values are:\n";
    my $dirh;
    opendir ($dirh, "distro-configs");
    while (readdir ($dirh)) {
    /(.*)\.conf$/ || next;
    print STDERR "\t$1\n";
    }
    closedir ($dirh);
    exit (1);
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
    }
    push @args, read_args ($config);
    } else {
    push @args, $arg;
    }
}

system ("touch ChangeLog");

my $system = `uname -s`;

my $aclocal_flags = $ENV{ACLOCAL_FLAGS};
$aclocal_flags = "-I ./m4/mac" if (!defined $aclocal_flags && $system eq 'Darwin');
$aclocal_flags = "" if (!defined $aclocal_flags);

$ENV{AUTOMAKE_EXTRA_FLAGS} = '--warnings=no-portability' if (!$system eq 'Darwin');

system ("aclocal $aclocal_flags") && die "Failed to run aclocal";
system ("autoconf") && die "Failed to run autoconf";

if (defined $ENV{NOCONFIGURE}) {
    print "Skipping configure process.";
} else {
    if (@ARGV > 0) {
    print "writing args to autogen.lastrun\n";
    my $fh;
    open ($fh, ">autogen.lastrun") || die "can't open autogen.lastrun: $!";
    for my $arg (@ARGV) {
        print $fh "$arg\n";
    }
    close ($fh);
    }
    print "running ./configure with '" . join ("' '", @args), "'\n";
    system ("./configure", @args);
}
