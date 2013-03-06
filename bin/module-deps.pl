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
    if (defined $ENV{DEP_CACHE_FILE}) {
	open ($p, $ENV{DEP_CACHE_FILE}) || die "can't read deps from cache: $!";
    } else {
	open ($p, "ENABLE_PRINT_DEPS=1 $gnumake -n -f $makefile_build all|") || die "can't launch make: $!";
    }
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

# graphviz etc. don't like some names
sub clean_name($)
{
    my $name = shift;
    $name =~ s/[\-\/\.]/_/g;
    return $name;
}

# first create nodes for each entry
sub clean_tree($)
{
    my $deps = shift;
    my %tree;
    for my $name (sort keys %{$deps}) {
	my $need_str = $deps->{$name};
	$need_str =~ s/^\s+//g;
	$need_str =~ s/\s+$//g;
	my @needs = split /\s+/, $need_str;
	$name =~ m/^([^_]+)_(\S+)$/ || die "invalid target name: '$name'";
	my $type = $1;
	my $target = clean_name ($2);
	$type eq 'Executable' || $type eq 'Library' ||
	    $type eq 'CppunitTest' || die "Unknown type '$type'";

	my %result;
	$result{type} = $type;
	$result{target} = $target;
	$result{generation} = 0;
	my @clean_needs;
	for my $need (@needs) {
	    push @clean_needs, clean_name($need);
	}
	$result{deps} = \@clean_needs;
	if (defined $tree{$target}) {
	    print STDERR "warning -duplicate target: '$target'\n";
	}
	$tree{$target} = \%result;

#	print "$target ($type): " . join (',', @clean_needs) . "\n";
    }
    return \%tree;
}

sub has_child_dep($$$)
{
    my ($tree,$search,$name) = @_;
    my $node = $tree->{$name};
    return defined $node->{flat_deps}->{$search};
}

# flatten deps recursively into a single hash per module
sub build_flat_dep_hash($$);
sub build_flat_dep_hash($$)
{
    my ($tree, $name) = @_;
    my %flat_deps;

    my $node = $tree->{$name};
    return if (defined $node->{flat_deps});

    # build flat deps for children
    for my $child (@{$node->{deps}}) {
	build_flat_dep_hash($tree, $child)
    }

    for my $child (@{$node->{deps}}) {
	$flat_deps{$child} = 1;
	for my $dep (@{$tree->{$child}->{deps}}) {
	    $flat_deps{$dep} = 1;
	}
    }
    $node->{flat_deps} = \%flat_deps;

    # useful debugging ...
    if (defined $ENV{DEP_CACHE_FILE}) {
	print "node '$name' has flat-deps: '" . join(',', keys %flat_deps) . "' " .
	    "vs. '" . join(',', @{$node->{deps}}) . "'\n";
    }
}

# many modules depend on vcl + sal, but vcl depends on sal
# so we want to strip sal out - and the same for many
# similar instances
sub prune_redundant_deps($)
{
    my $tree = shift;
    for my $name (sort keys %{$tree}) {
	build_flat_dep_hash($tree, $name);
    }
}

sub dump_graphviz($)
{
    my $tree = shift;
    print "digraph LibreOffice {\n";
    for my $name (sort keys %{$tree}) {
	my $result = $tree->{$name};
	if ($result->{type} eq 'CppunitTest' ||
	    ($result->{type} eq 'Executable' && $result->{target} ne 'soffice_bin')) {
	    next; # de-bloat the tree
	}

#	print STDERR "minimising deps for $result->{target}\n";
	my @newdeps;
	for my $dep (@{$result->{deps}}) {
	    my $print = 1;
	    # is this implied by any other child ?
#	    print STDERR "checking if '$dep' is redundant\n";
	    for my $other_dep (@{$result->{deps}}) {
		next if ($other_dep eq $dep);
		if (has_child_dep($tree,$dep,$other_dep)) {
		    $print = 0;
#		    print STDERR "$dep is implied by $other_dep - ignoring\n";
		}
	    }
	    print "$name -> $dep;\n" if ($print);
	    push @newdeps, $dep;
	}
	# re-write the shrunk set to accelerate things
	$result->{deps} = \@newdeps;
    }
    print "}\n";
}

my $graphviz = 1;

while (my $arg = shift @ARGV) {
    if ($arg eq '--graph' || $arg eq '-g') {
	$graphviz = 1;
    } elsif (!defined $gnumake) {
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
my $tree = clean_tree($deps);

prune_redundant_deps($tree);

dump_graphviz($tree);
