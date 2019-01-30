:
#
# This script checks various configure parameters and uses three files:
#   * autogen.input (ro)
#   * autogen.lastrun (rw)
#   * autogen.lastrun.bak (rw)
#
# If _no_ parameters:
#   Read args from autogen.input or autogen.lastrun
# Else
#   Backup autogen.lastrun as autogen.lastrun.bak
#   Write autogen.lastrun with new commandline args
#
# Run configure with checked args
#
    eval 'exec perl -S $0 ${1+"$@"}'
        if 0;

use strict;
use Cwd ('cwd', 'realpath');
use File::Basename;

my $src_path=dirname(realpath($0));
my $build_path=realpath(cwd());
# since this looks crazy, if you have a symlink on a path up to and including
# the current directory, we need our configure to run in the realpath of that
# such that compiled (realpath'd) dependency filenames match the filenames
# used in our makefiles - ie. this gets dependencies right via SRC_ROOT
chdir ($build_path);
# more amazingly, if you don't clobber 'PWD' shells will re-assert their
# old path from the environment, not cwd.
$ENV{PWD} = $build_path;

sub clean()
{
    system ("rm -Rf autom4te.cache");
    system ("rm -f missing install-sh mkinstalldirs libtool ltmain.sh");
    print "Cleaned the build tree\n";
}

my $aclocal;
my $autoconf;

# check we have various vital tools
sub sanity_checks($)
{
    my $system = shift;
    my @path = split (':', $ENV{'PATH'});
    my %required =
      (
       'pkg-config' => "pkg-config is required to be installed",
       $autoconf    => "autoconf is required",
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
        s/^\s+//;
        s/\s+$//;
        # migrate from the old system
        if ( substr($_, 0, 1) eq "'" ) {
            print STDERR "Migrating options from the old autogen.lastrun format, using:\n";
            my @opts;
            @opts = split(/'/);
            foreach my $opt (@opts) {
                if ( substr($opt, 0, 1) eq "-" ) {
                    push @lst, $opt;
                    print STDERR "  $opt\n";
                }
            }
        } elsif ( substr($_, 0, 1) eq "#" ) {
            # comment
        } elsif ( length == 0 ) {
            # empty line
        } else {
            push @lst, $_;
        }
    }
    close ($fh);
    # print "read args from file '$file': @lst\n";
    return @lst;
}

sub show_distro_configs($$)
{
    my ($prefix, $path) = @_;
    my $dirh;
    opendir ($dirh, "$path");
    while (($_ = readdir ($dirh))) {
        if (-d "$path/$_") {
            show_distro_configs(
                    $prefix eq "" ? "$_/" : "$prefix/$_/", "$path/$_")
                unless $_ eq '.' || $_ eq '..';
            next;
        }
        /(.*)\.conf$/ || next;
        print STDERR "\t$prefix$1\n";
    }
    closedir ($dirh);
}

sub invalid_distro($$)
{
    my ($config, $distro) = @_;
    print STDERR "Can't find distro option set: $config\n";
    print STDERR "Distros with distro option sets are:\n";
    show_distro_configs("", "$src_path/distro-configs");
    exit (1);
}

# Avoid confusing "aclocal: error: non-option arguments are not accepted: '.../m4'." error message.
die "\$src_path must not contain spaces, but it is '$src_path'." if ($src_path =~ / /);

# Alloc $ACLOCAL to specify which aclocal to use
$aclocal = $ENV{ACLOCAL} ? $ENV{ACLOCAL} : 'aclocal';
# Alloc $AUTOCONF to specify which autoconf to use
# (e.g. autoconf268 from a backports repo)
$autoconf = $ENV{AUTOCONF} ? $ENV{AUTOCONF} : 'autoconf';

my $system = `uname -s`;
chomp $system;

sanity_checks ($system) unless($system eq 'Darwin');

# If we are running in a LODE env, make sure we find the right aclocal
# by making sure that LODE_HOME/opt/bin is in the PATH
if (defined $ENV{LODE_HOME})
{
    my $lode_path = quotemeta "$ENV{LODE_HOME}/opt/bin";
    if($ENV{PATH} !~ $lode_path)
    {
        $ENV{PATH}="$ENV{LODE_HOME}/opt/bin:$ENV{PATH}";
        print STDERR "add LODE_HOME/opt/bin in PATH\n";
    }
}

my $aclocal_flags = $ENV{ACLOCAL_FLAGS};

$aclocal_flags .= " -I $src_path/m4";
$aclocal_flags .= " -I $src_path/m4/mac" if ($system eq 'Darwin');
$aclocal_flags .= " -I /opt/freeware/share/aclocal" if ($system eq 'AIX');

$ENV{AUTOMAKE_EXTRA_FLAGS} = '--warnings=no-portability' if (!($system eq 'Darwin'));

if ($src_path ne $build_path)
{
    system ("ln -sf $src_path/configure.ac configure.ac");
    system ("ln -sf $src_path/g g");
    my $src_path_win=$src_path;
    if ($system =~ /CYGWIN.*/) {
        $src_path_win=`cygpath -m $src_path`;
        chomp $src_path_win;
    }
    my @modules = <$src_path/*/Makefile>;
    foreach my $module (@modules)
    {
        my $dir = basename (dirname ($module));
        mkdir ($dir);
        system ("rm -f $dir/Makefile");
        system ("printf 'module_directory:=$src_path_win/$dir/\ninclude \$(module_directory)/../solenv/gbuild/partial_build.mk\n' > $dir/Makefile");
    }
    my @external_modules = <$src_path/external/*/Makefile>;
    mkdir ("external");
    system ("ln -sf $src_path/external/Module_external.mk external/");
    foreach my $module (@external_modules)
    {
        my $dir = basename (dirname ($module));
        mkdir ("external/$dir");
        system ("rm -f external/$dir/Makefile");
        system ("printf 'module_directory:=$src_path_win/external/$dir/\ninclude \$(module_directory)/../../solenv/gbuild/partial_build.mk\n' > external/$dir/Makefile");
    }
}
system ("$aclocal $aclocal_flags") && die "Failed to run aclocal";
unlink ("configure");
system ("$autoconf -I ${src_path}") && die "Failed to run autoconf";
die "Failed to generate the configure script" if (! -f "configure");

# Handle help arguments first, so we don't clobber autogen.lastrun
for my $arg (@ARGV) {
    if ($arg =~ /^(--help|-h|-\?)$/) {
        print STDOUT "autogen.sh - libreoffice configuration helper\n";
        print STDOUT "   --clean        forcibly re-generate configuration\n";
        print STDOUT "   --best-effort  don't fail on un-known configure with/enable options\n";
        print STDOUT "\nOther arguments passed directly to configure:\n\n";
        system ("./configure --help");
        exit;
    }
}

my @cmdline_args = ();

my $input = "autogen.input";
my $lastrun = "autogen.lastrun";

if (!@ARGV) {
    if (-f $input) {
        if (-f $lastrun) {
            print STDERR <<WARNING;
********************************************************************
*
*   Reading $input and ignoring $lastrun!
*   Consider removing $lastrun to get rid of this warning.
*
********************************************************************
WARNING
        }
        @cmdline_args = read_args ($input);
    } elsif (-f $lastrun) {
        print STDERR "Reading $lastrun. Please rename it to $input to avoid this message.\n";
        @cmdline_args = read_args ($lastrun);
    }
} else {
    if (-f $input) {
        print STDERR <<WARNING;
********************************************************************
*
*   Using commandline arguments and ignoring $input!
*
********************************************************************
WARNING
    }
    @cmdline_args = @ARGV;
}

my @args;
my $default_config = "$src_path/distro-configs/default.conf";
my $option_checking = 'fatal';

if (-f $default_config) {
    print STDERR "Reading default config file: $default_config.\n";
    push @args, read_args ($default_config);
}
for my $arg (@cmdline_args) {
    if ($arg eq '--clean') {
        clean();
    } elsif ($arg =~ m/--with-distro=(.*)$/) {
        my $config = "$src_path/distro-configs/$1.conf";
        if (! -f $config) {
            invalid_distro ($config, $1);
        }
        push @args, read_args ($config);
    } elsif ($arg =~ m/--best-effort$/) {
        $option_checking = 'warn';
    } else {
        push @args, $arg;
    }
}

if (defined $ENV{NOCONFIGURE}) {
    print "Skipping configure process.";
} else {
    # Save autogen.lastrun only if we did get some arguments on the command-line
    if (! -f $input && @ARGV) {
        if (scalar(@cmdline_args) > 0) {
            # if there's already an autogen.lastrun, make a backup first
            if (-e $lastrun) {
                open (my $fh, $lastrun) || warn "Can't open $lastrun.\n";
                open (BAK, ">$lastrun.bak") || warn "Can't create backup file $lastrun.bak.\n";
                while (<$fh>) {
                    print BAK;
                }
                close (BAK) && close ($fh);
            }
            # print "Saving command-line args to $lastrun\n";
            my $fh;
            open ($fh, ">autogen.lastrun") || die "Can't open autogen.lastrun: $!";
            for my $arg (@cmdline_args) {
                print $fh "$arg\n";
            }
            close ($fh);
        }
    }
    push @args, "--srcdir=$src_path";
    push @args, "--enable-option-checking=$option_checking";

    print "Running ./configure with '" . join (" ", @args), "'\n";
    system ("./configure", @args) && die "Error running configure";
}

# Local Variables:
# mode: perl
# cperl-indent-level: 4
# tab-width: 4
# indent-tabs-mode: nil
# End:

# vim:set ft=perl shiftwidth=4 softtabstop=4 expandtab: #
