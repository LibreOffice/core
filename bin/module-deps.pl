#!/usr/bin/perl -w

use strict;

my $gnumake;
my $makefile_build;

sub read_deps()
{
    my $p;
    my $invalid_tolerance = 100;
    my $line_count = 0;
    my %deps;
    open ($p, "ENABLE_PRINT_DEPS=1 $gnumake -n -f $makefile_build all|") || die "can't launch make: $!";
#    open ($p, "/tmp/deps") || die "can't read deps: $!";
    $|=1;
    print STDERR "reading deps ";
    while (<$p>) {
	my $line = $_;
	$line_count++;
	print STDERR '.' if ($line_count % 10 == 0);
#	print STDERR $line;
	chomp ($line);
        if ($line =~ m/^LibraryDep:\s+(\S+) links against (.*)$/) {
#        if ($line =~ m/^LibraryDep:\s+(\S+)\s+links against/) {
	    $deps{$1} = ' ' if (!defined $deps{$1});
	    $deps{$1} = $deps{$1} . ' ' . $2;
        } elsif ($line =~ m/^LibraryDep:\s+links against/) {
#           these need fixing, we call gb_LinkTarget__use_$...
#           and get less than normal data back to gb_LinkTarget_use_libraries
#	    print STDERR "ignoring unhelpful external dep\n";
	} elsif ($invalid_tolerance < 0) {
#	    print "read all dependencies to: '$line'\n";
	    last;
	} else {
#	    print "no match '$line'\n";
	    $invalid_tolerance--;
	}
    }
    close ($p);
    print STDERR " done\n";

    return \%deps;
}

# first create nodes for each entry
sub build_tree($)
{
    my $deps = shift;
    for my $name (sort keys %{$deps}) {
	my $need_str = $deps->{$name};
	$need_str =~ s/^\s+//g;
	$need_str =~ s/\s+$//g;
	my @needs = split /\s+/, $need_str;
	$name =~ m/^([^_]+)_(\S+)$/ || die "invalid target name: '$name'";
	my $type = $1;
	my $target = $2;
	$type eq 'Executable' || $type eq 'Library' ||
	    $type eq 'CppunitTest' || die "Unknown type '$type'";
	print "$target ($type): " . join (',', @needs) . "\n";
    }
}

while (my $arg = shift @ARGV) {
    if (!defined $gnumake) {
	$gnumake = $arg;
    } elsif (!defined $makefile_build) {
	$makefile_build = $arg;
    } else {
	die "un-needed argument '$arg'";
    }
}

$gnumake = 'make' if (!defined $gnumake);
$makefile_build = 'Makefile.gbuild' if (!defined $makefile_build);

my $deps = read_deps();
my $tree = build_tree($deps);

