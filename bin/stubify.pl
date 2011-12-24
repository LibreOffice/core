#!/usr/bin/env perl

use Fcntl;
use POSIX;
use strict;

# simple pkgconfig goodness
my $destdir;
my $recursive = 0;
my $assembler_out = 0;
my %pkg_configs = ();
my @pkg_config_paths = split(/:/, $ENV{PKG_CONFIG_PATH});
push @pkg_config_paths, "/usr";

# Stubify a shared library ...
sub read_gen_symbols($$)
{
    my ($shlib, $fh) = @_;
    my $obj;

    print $fh "\t.file \"$shlib\"\n";
    open $obj, "objdump -T $shlib|" || die "Can't objdump $shlib: $!";

    while (my $line = <$obj>) {
	$line =~ /([0-9a-f]*)\s+([gw ])\s+..\s+(\S*)\s*([0-9a-f]+)..............(.*)/ || next;
	my ($address, $linkage, $type, $size, $symbol) = ($1, $2, $3, $4, $5);

	next if ($type eq '*UND*' || $type eq '*ABS*');

#	print "Symbol '$symbol' type '$type' '$linkage' addr $address, size $size\n";

	$symbol || die "no symbol for line $line";

	next if ($symbol eq '_init' || $symbol eq '_fini');

	$linkage =~ s/g//g;

	my $progbits = '@progbits';
	$progbits = '@nobits' if ($type eq '.bss');
	print $fh "\t.section $type.$symbol,\"a".$linkage."G\",$progbits,$symbol,comdat\n";
	print $fh ".globl $symbol\n";
	print $fh "\t.type $symbol,";
	if ($type eq '.text') {
	    print $fh "\@function\n";
	} else {
	    print $fh "\@object\n";
	}
	print $fh "$symbol:\n";
	if ($type eq '.text') {
	    print $fh "\tret\n";
	} else {
	    my $isize = hex($size);
	    print $fh "\t.size $symbol, $isize\n";
	    for (my $i = 0; $i < $isize; $i++) {
		print $fh "\t.byte 0\n";
	    }
	}
	print $fh "\n";
    }

    close $obj;
}

sub stubify($$)
{
    my $shlib = shift;
    my $output = shift;
    my ($pipe, $tmpf);

    my $tmpname;
    do {
	$tmpname = tmpnam();
    } until sysopen($tmpf, $tmpname, O_RDWR|O_CREAT|O_EXCL, 0666);
    close($tmpf);

    if ($assembler_out) {
	open ($pipe, ">-");
    } else {
	open ($pipe, "| as -o $tmpname") || die "can't start assembler: $!";
    }
    read_gen_symbols ($shlib, $pipe);
    close ($pipe) || die "Failed to assemble to: $tmpname: $!";

    system ("gcc -shared -o $output $tmpname") && die "failed to exec gcc: $!";
    unlink $tmpname;
}

sub help_exit()
{
    print "Usage: stubify <destdir> <pkg-config-names>\n";
    print "Converts libraries into stubs, and bundles them and their pkg-config files\n";
    print "into destdir\n";
    print "  -R        stubbify and include all dependent pkgconfig files\n";
    exit 1;
}

sub parse_pkgconfig($$)
{
    my $name = shift;
    my $file = shift;
    my $fh;
    my %hash;
    my @hashes;

    print "parse $file\n";
    open ($fh, $file) || die "Can't open $file: $!";
    while (<$fh>) {
	my ($key, $value);
	if (/^\s*([^=]+)\s*=\s*([^=]+)\s*$/) {
	    $key = $1; $value = $2;
	} elsif (/^\s*([^:]+)\s*:\s*([^:]+)\s*$/) {
	    $key = $1; $value = $2;
	} elsif (/^\s*$/) {
	    next;
	} else {
	    die "invalid pkgconfig line: $_\n";
	}
	chomp ($key); chomp ($value);
	$hash{$key} = $value;
    }
    close ($fh);
    for my $key (keys (%hash)) {
	print "\t'$key'\t=\t'$hash{$key}'\n";
    }

    $hash{_Name} = $name;
    $hash{_File} = $file;

    push @hashes, \%hash;
    if ($recursive &&
	!defined $pkg_configs{$name} &&
	defined $hash{Requires}) {
	my @reqs = ();
	for my $req (split (/[ ,]/, $hash{Requires})) {
	    print "parse $req of $name\n";
	    push @reqs, get_pc_files($req);
	}
	$hash{_Requires} = \@reqs;
	push @hashes, @reqs;
    }
    $pkg_configs{$name} = \%hash;
    return @hashes;
}

sub get_pc_files($)
{
    my $name = shift;
    for my $prefix (@pkg_config_paths) {
	my $path = "$prefix/lib/pkgconfig/$name.pc";
	return parse_pkgconfig ($name,$path) if (-f $path);
    }
    die "Failed to find pkg-config file for $name";
}

# primitive substitution
sub get_var($$)
{
    my ($pc, $var) = @_;
    my $val = $pc->{"$var"};
    while ($val =~ m/^(.*)\$\{\s*(\S+)\s*\}(.*)$/) {
	$val = $1 . get_var($pc, $2). $3;
    }
    return $val;
}

sub copy_lib($@)
{
    my $lib = shift;
    while (my $path = shift) {
	my $name = "$path/$lib";
	next if (! -f $name);

	# need to run ldconfig post install ...
	while (-l $name) {
	    my $dir = $name;
	    $dir =~ s/\/[^\/]*$//;
	    my $link = readlink($name);
	    if ($link =~ m/^\//) {
		$name = $link;
	    } else {
		$name = "$dir/$link";
	    }
	}

	# ignore /lib - they use monstrous symbol versioning
	if ($name =~ m/^\/lib/) {
	    print "\tskipping system library: $lib in $name\n";
	    return;
	}

	stubify ($name, "$destdir/$name");
    }
}

sub copy_and_stubify ($)
{
    my $pc = shift;

    `mkdir -p $destdir/usr/lib/pkgconfig`;
    `mkdir -p $destdir/$pc->{libdir}` if (defined $pc->{libdir});
    `mkdir -p $destdir/$pc->{includedir}` if (defined $pc->{includedir});

    # copy .pc across - FIXME, may need to re-write paths
    `cp -a $pc->{_File} $destdir/usr/lib/pkgconfig`;

    # copy includes across
    my @includes = split (/ /, get_var ($pc, "Cflags"));
    for my $arg (@includes) {
	if ($arg =~ m/^-I(.*)$/) {
	    my $srcdir = $1;
	    if (! -d $srcdir || $srcdir eq '/usr/include') {
		print "Warning: bogus include of '$srcdir' for pkg $pc->{_Name}\n";
	    } else {
		`mkdir -p $destdir/$srcdir`;
		`cp -a $srcdir/* $destdir/$srcdir`;
	    }
	}
    }

    # stubify libraries
    my @libs = split (/ /, get_var ($pc, "Libs"));
    my @libpath = ( "/lib", "/usr/lib" );
    for my $arg (@libs) {
	if ($arg =~ m/^-l(.*)$/) {
	    my $lib = "lib".$1.".so";
#	    print "lib $lib @libpath?\n";
	    copy_lib ($lib, @libpath);
	} elsif ($arg =~ m/^-L(.*)$/) {
	    my $path = $1;
	    push (@libpath, $path) if (! grep ($path, @libpath));
	}
    }
}

my @pcnames = ();
my @tostub;

for my $arg (@ARGV) {
    if ($arg eq '--help' || $arg eq '-h') {
	help_exit();
    } elsif ($arg eq '-r' || $arg eq '-R') {
	$recursive = 1;
    } elsif (!defined $destdir) {
	$destdir = $arg;
    } else {
	push @pcnames, $arg;
    }
}

help_exit() if (!defined $destdir);
`mkdir -p $destdir`;

for my $name (@pcnames) {
    push @tostub, get_pc_files($name);
}
print "stubify: ";
select STDERR; $| = 1;
for my $pc (@tostub) {
    print " " . $pc->{_Name} . "\n";
    copy_and_stubify ($pc);
}
print "\n";
