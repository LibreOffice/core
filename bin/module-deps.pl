#!/usr/bin/perl

use strict;
use warnings;
use Getopt::Long qw(GetOptions VersionMessage);
use Pod::Usage;

my $gnumake;
my $src_root;
my $makefile_build;
my $verbose = 0;
my $no_leaf;
my $from_file;
my $to_file;
my $graph_file;
my $preserve_libs = 0;

sub logit($)
{
    print STDERR shift if ($verbose);
}

sub read_deps()
{
    my $p;
    my $to;
    my $invalid_tolerance = 100;
    my $line_count = 0;
    my %deps;
    if (defined $to_file)
    {
        open($to, ">$to_file") or die "can not open file for writing $to_file";
    }
    if (defined $from_file) {
        open ($p, $from_file) || die "can't read deps from cache file: $!";
    } else {
        open ($p, "ENABLE_PRINT_DEPS=1 $gnumake -qrf $makefile_build|") || die "can't launch make: $!";
    }
    $|=1;
    print STDERR "reading deps ";
    while (<$p>) {
        my $line = $_;
        $line_count++;
        print STDERR '.' if ($line_count % 10 == 0);
        logit($line);
        print $to $line if defined $to_file;
        chomp ($line);
        if ($line =~ m/^LibraryDep:\s+(\S+) links against (.*)$/) {
#        if ($line =~ m/^LibraryDep:\s+(\S+)\s+links against/) {
            $deps{$1} = ' ' if (!defined $deps{$1});
            $deps{$1} = $deps{$1} . ' ' . $2;
        } elsif ($line =~ m/^LibraryDep:\s+links against/) {
#           these need fixing, we call gb_LinkTarget__use_$...
#           and get less than normal data back to gb_LinkTarget_use_libraries
#           print STDERR "ignoring unhelpful external dep\n";
        } elsif ($invalid_tolerance < 0) {
#           print "read all dependencies to: '$line'\n";
            last;
        } else {
#           print "no match '$line'\n";
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
            logit("warning -duplicate target: '$target'\n");
            delete($tree{$target});
        }
        $tree{$target} = \%result;

        logit("$target ($type): " . join (',', @clean_needs) . "\n");
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
        logit("node '$name' has flat-deps: '" . join(',', keys %flat_deps) . "' " .
            "vs. '" . join(',', @{$node->{deps}}) . "'\n");
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

# glob on libo directory
sub create_lib_module_map()
{
    my %l2m;
    # hardcode the libs that don't have a directory
    $l2m{'merged'} = 'merged';
    $l2m{'urelibs'} = 'urelibs';

    for (glob($src_root."/*/Library_*.mk"))
    {
        /.*\/(.*)\/Library_(.*)\.mk/;
        # add module -> module
        $l2m{$1} = $1;
        # add lib -> module
        $l2m{$2} = $1;
    }
    return \%l2m;
}

# call prune redundant_deps
# rewrite the deps array
sub optimize_tree($)
{
    my $tree = shift;
    prune_redundant_deps($tree);
    for my $name (sort keys %{$tree}) {
        my $result = $tree->{$name};
        logit("minimising deps for $result->{target}\n");
        my @newdeps;
        for my $dep (@{$result->{deps}}) {
            # is this implied by any other child ?
            logit("checking if '$dep' is redundant\n");
            my $preserve = 1;
            for my $other_dep (@{$result->{deps}}) {
                next if ($other_dep eq $dep);
                if (has_child_dep($tree,$dep,$other_dep)) {
                    logit("$dep is implied by $other_dep - ignoring\n");
                    $preserve = 0;
                    last;
                }
            }
            push @newdeps, $dep if ($preserve);
        }
        # re-write the shrunk set to accelerate things
        $result->{deps} = \@newdeps;
    }
    return $tree;
}

# walking through the library based graph and creating a module based graph.
sub collapse_lib_to_module($)
{
    my $tree = shift;
    my %digraph;
    my $l2m = create_lib_module_map();
    my %unknown_libs;
    for my $name (sort keys %{$tree}) {
        my $result = $tree->{$name};
        # sal has no dependencies, take care of it
        # otherwise it doesn't have target key
        if (!@{$result->{deps}}) {
            $digraph{$name}{target} = $result->{target};
        }
        for my $dep (@{$result->{deps}}) {
            $unknown_libs{$name} = 1 && next if (!grep {/$name/} keys $l2m);
            $name = $l2m->{$name};
            $dep = $l2m->{$dep};
            # ignore: two libraries from the same module depend on each other
            next if ($name eq $dep);
            if (exists($digraph{$name}))
            {
                my @deps = @{$digraph{$name}{deps}};
                # only add if we haven't seen already that edge?
                if (!grep {/$dep/} @deps)
                {
                    push @deps, $dep;
                    $digraph{$name}{deps} = \@deps;
                }
            }
            else
            {
                my @deps;
                push @deps, $dep;
                $digraph{$name}{deps} = \@deps;
                $digraph{$name}{target} = $result->{target};
            }
        }
    }
    logit("warn: no module for libs were found and dropped: [" .
          join(",", (sort (keys(%unknown_libs)))) . "]\n");
    return optimize_tree(\%digraph);
}

sub prune_leaves($)
{
    my $tree = shift;
    my %newtree;
    my %name_has_deps;

    # we like a few leaves around:
    for my $nonleaf ('desktop', 'sw', 'sc', 'sd', 'starmath') {
        $name_has_deps{$nonleaf} = 1;
    }

    # find which modules are depended on by others
    for my $name (keys %{$tree}) {
        for my $dep (@{$tree->{$name}->{deps}}) {
            $name_has_deps{$dep} = 1;
        }
    }

    # prune modules with no deps
    for my $name (keys %{$tree}) {
        delete $tree->{$name} if (!defined $name_has_deps{$name});
    }

    return optimize_tree($tree);
}


sub dump_graphviz($)
{
    my $tree = shift;
    my $to = \*STDOUT;
    open($to, ">$graph_file") if defined($graph_file);
    print $to <<END;
digraph LibreOffice {
node [shape="Mrecord", color="#BBBBBB"]
node  [fontname=Verdana, color="#BBBBBB", fontsize=10, height=0.02, width=0.02]
edge  [color="#31CEF0", len=0.4]
edge  [fontname=Arial, fontsize=10, fontcolor="#31CEF0"]
END
   for my $name (sort keys %{$tree}) {
       my $result = $tree->{$name};
       logit("minimising deps for $result->{target}\n");
       for my $dep (@{$result->{deps}}) {
           print $to "$name -> $dep;\n" ;
       }
    }
    print $to "}\n";
}

sub filter_targets($)
{
    my $tree = shift;
    for my $name (sort keys %{$tree})
    {
        my $result = $tree->{$name};
        if ($result->{type} eq 'CppunitTest' ||
            ($result->{type} eq 'Executable' &&
             $result->{target} ne 'soffice_bin'))
        {
            delete($tree->{$name});
        }
    }
}

sub parse_options()
{
    my %h = (
        'verbose|v' => \$verbose,
        'help|h' => \my $help,
        'man|m' => \my $man,
        'version|r' => sub {
            VersionMessage(-msg => "You are using: 1.0 of ");
        },
        'preserve-libs|p' => \$preserve_libs,
        'write-dep-file|w=s' => \$to_file,
        'read-dep-file|f=s' => \$from_file,
        'no-leaf|l' => \$no_leaf,
        'graph-file|o=s' => \$graph_file);
    GetOptions(%h) or pod2usage(2);
    pod2usage(1) if $help;
    pod2usage(-exitstatus => 0, -verbose => 2) if $man;
    ($gnumake, $makefile_build) = @ARGV if $#ARGV == 1;
    $gnumake = 'make' if (!defined $gnumake);
    $makefile_build = 'Makefile.gbuild' if (!defined $makefile_build);
    $src_root = defined $ENV{SRC_ROOT} ? $ENV{SRC_ROOT} : ".";
}

sub main()
{
    parse_options();
    my $deps = read_deps();
    my $tree = clean_tree($deps);
    filter_targets($tree);
    optimize_tree($tree);
    if (!$preserve_libs && !defined($ENV{PRESERVE_LIBS})) {
        $tree = collapse_lib_to_module($tree);
    }
    if ($no_leaf) {
        $tree = prune_leaves($tree);
    }
    dump_graphviz($tree);
}

main()

 __END__

=head1 NAME

module-deps - Generate module dependencies for LibreOffice build system

=head1 SYNOPSIS

module_deps [options] [gnumake] [makefile]

=head1 OPTIONS

=over 8

=item B<--help>

=item B<-h>

Print a brief help message and exits.

=item B<--man>

=item B<-m>

Prints the manual page and exits.

=item B<--version>

=item B<-v>

Prints the version and exits.

=item B<--preserve-libs>

=item B<-p>

Don't collapse libs to modules

=item B<--read-dep-file file>

=item B<-f>

Read dependency from file.

=item B<--write-dep-file file>

=item B<-w>

Write dependency to file.

=item B<--graph-file file>

=item B<-o>

Write output to graph file

=back

=head1 DESCRIPTION

B<This program> parses the output of LibreOffice make process
(or cached input file) and generates the digraph build dependency,
that must be piped to B<graphviz> program (typically B<dot>).

B<Hacking on it>:

The typical (optimized) B<workflow> includes 3 steps:

=over 3

=item 1
Create cache dependency file: module_deps --write-dep-file lo.dep

=item 2
Use cache dependency file: module_deps --read-dep-file lo.dep -o lo.graphviz

=item 3
Pipe the output to graphviz: cat lo.graphviz | dot -Tpng -o lo.png

=back

=head1 TODO

=over 2

=item 1
Add soft (include only) dependency

=item 2
Add dependency on external modules

=back

=head1 AUTHOR

=over 2

=item Michael Meeks

=item David Ostrovsky

=back

=cut
