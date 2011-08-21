:
    eval 'exec perl -S $0 ${1+"$@"}'
        if 0;
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************
#
# oochkpatch - check patch flags against CWS modules
#

require File::Temp;
require File::Find;
require Getopt::Long;
require Pod::Usage;
use Pod::Usage;
use Getopt::Long;
use File::Temp qw/ tempfile tempdir /;
use File::Find;


# configuration goes here
##########################################################

# uncomment this, if in pure OOo environment
#my $toplevel_module = "instsetoo_native";
#my $scp_module     = "scp2";
#my $setup_file     = "setup_osl";

# uncomment this, if within the StarOffice environment
my $toplevel_module = "instset_native";
my $scp_module      = "scp2so";
my $setup_file      = "setup";

my $deliver = "solenv/bin/deliver.pl";
my $build   = "solenv/bin/build.pl";

# list of hardcoded exceptions (files that are _never_ considered
# missing from the patch)
my %hardcoded_exceptions = ('build.lst' => 1);


# no configuration below this point, please!
##########################################################

# defaults
my $from_module = "";
my $verbose     = '';
my $help        = '';
my $man         = '';
my $modules     = '';
my $from        = '';
my $perl        = '';

GetOptions('help|?'  => \$help,
           'man'     => \$man,
           'verbose' => \$verbose,
           'from=s'  => \$from_module ) or pod2usage(2);
pod2usage(1) if $help;
pod2usage(-exitstatus => 0, -verbose => 2) if $man;

# process remaining args
print "Processing args...\n" if $verbose;
foreach my $argument (@ARGV)
{
    print " Checking module ", $argument, "\n" if $verbose;
    push @modules, $argument;
}

# platform-dependent stuff
if( $^O eq 'MSWin32' )
{
    $perl = "$ENV{COMSPEC} -c $ENV{PERL}";
    $setup_file = $setup_file . ".inf";
}
else
{
    $perl = 'perl';
    $setup_file = $setup_file . ".ins";
};

# read some SOLAR stuff from env
my $SRC_ROOT = $ENV{"SRC_ROOT"};
my $INPATH = $ENV{"INPATH"};

# process --from modules
if( $from_module )
{
    print "Checking all modules upwards and including ", $from_module, "\n" if $verbose;

    # append build.pl-generated list of modules
    chdir "$SRC_ROOT/$toplevel_module" or
        chdir "$SRC_ROOT/$toplevel_module.lnk" or die "ERROR: cannot cd to $SRC_ROOT/$toplevel_module!";
    open(ALLMODULES,
         "$perl $SRC_ROOT/$build --all:$from_module --show 2>&1 |") or die "ERROR: cannot build --show!\n";
    while(<ALLMODULES>)
    {
        if( /Building project/ )
        {
            my @module = split( /\s+/, $_ );
            print " which is ", $module[2], "\n" if $verbose;
            push(@modules,$module[2]);
        }
    }
}

die "ERROR: no modules to check!\n" if !@modules;

$tempdir = tempdir( TMPDIR => 1, CLEANUP => 1);

# generate list of files with PATCH flag
print "Generating list of files which have the PATCH flag...\n" if $verbose;

my $path_to_setup_file = $SRC_ROOT."/".$scp_module."/".$INPATH."/bin/osl/".$setup_file;
my $alternate_path_to_setup_file = $SRC_ROOT."/".$scp_module.".lnk/".$INPATH."/bin/osl/".$setup_file;
my $in_file_block=0;
my $patch_flag=0;
my $file_name='';
my $base;
my $ext;
my %pack_files;
open(SETUP, "<".$path_to_setup_file) or
   open(SETUP, "<".$alternate_path_to_setup_file) or die "ERROR: cannot open $path_to_setup_file!\n";
while(<SETUP>)
{
    if( /^File\s+/ && !$in_file_block )
    {
        $in_file_block = 1;
        $patch_flag=0;
        $file_name='';
    }
    elsif( /^End/ && $file_name ne '' && $in_file_block )
    {
        $file_name =~ s/["']//g;
        $pack_files{$file_name} = $patch_flag;

        if( $patch_flag )
        {
            print( " File $file_name included in patch\n") if $verbose;
        }
        else
        {
            print( " File $file_name NOT included in patch\n") if $verbose;
        }

        $in_file_block = 0;
    }
    elsif( /^\s+Styles\s*=\s*.*PATCH/ && $in_file_block )
    {
        $patch_flag = 1;
    }
    elsif( ($res) = /^\s+Name\s*=\s*(.*);/ )
    {
        $file_name = $res;
    }
}

# generate list of delivered files
print "Generating list of delivered libs...\n" if $verbose;

# first, deliver all modules to tempdir
foreach my $module (@modules)
{
    print " dummy-delivering $module...\n" if $verbose;
    chdir "$SRC_ROOT/$module" or
        chdir "$SRC_ROOT/$module.lnk" or die "ERROR: cannot cd to $SRC_ROOT/$module!";
    `$perl $SRC_ROOT/$deliver $tempdir`;
}

# now, check all files in delivered dirs for containedness in PATCH
# set
print "Checking against delivered files...\n" if $verbose;
find(\&wanted, $tempdir );

sub wanted
{
    my $fname;

    if( -f )
    {
        $fname = $_;
        if( !exists $pack_files{$fname} )
        {
            print " File $fname is not packed.\n" if $verbose;
        }
        elsif( $pack_files{$fname} == 0 )
        {
            if( !$hardcoded_exceptions{ $fname } )
            {
                # file not in patch set, and not in exception list
                print " File $fname is packed, but NOT included in patch set and part of delivered output\n" if $verbose;
                print "$fname\n" if !$verbose;
            }
            else
            {
                print " File $fname is NOT included in patch set, but member of hardcoded exception list\n" if $verbose;
            }
        }
        elsif( $pack_files{$fname} == 1 )
        {
            print " File $fname packed and patched.\n" if $verbose;
        }
    }
}


__END__

=head1 NAME

oochkpatch.pl - Verify patch flags against module libraries

=head1 SYNOPSIS

oochkpatch.pl [options] [module-name ...]

 Options:
   --help|-h         brief help message
   --man|-m          full documentation
   --verbose|-v      tell what's happening
   --from=module     check all modules from
                     given one upwards

=head1 OPTIONS

=over 8

=item B<--help>

Print a brief help message and exits.

=item B<--man>

Prints the manual page and exits.

=item B<--verbose>

Verbosely tell what's currently happening

=item B<--from=module>

Assumes OOo was built incompatibly from given module
upwards, and check against all libs from all upwards modules.
Further modules can be given at the command line, which are merged
with the ones generated from this option

=back

=head1 DESCRIPTION

B<This program> will compare all libs delivered from the specified modules
against the set of files marked with the B<patch> flag in scp2. Useful to check
if the patch set is complete. Please note that this program needs to be run in
a solar shell, i.e. the OOo build environment needs to be set up in the shell.

There's kind of a heuristic involved, to determine exactly which files
to check against includedness in the patch set (since e.g. all headers
are delivered, but clearly need not be checked against patch
flags). It works by first collecting all files that are mentioned in
the pack master file, and then checking all files delivered from the
specified modules against that pack list: if the file is not packed,
or if it's packed and has the patch flag set, all is well. Otherwise,
the file in question potentially misses the patch flag (because one of
the modified modules contains it).

=head1 EXAMPLE

To determine the set of libs not yet carrying the patch flag for a CWS
containing sfx2, svx, and vcl, which is incompatible from sfx2
upwards, use something like this:

oochkpatch.pl --from=sfx2 `cwsquery modules`

This puts every module upwards and including sfx2 in the check list,
plus vcl. Note that with this approach, you'll usually get a larger
set of files for the patch than necessary - but at least you get all
files that might have changed theoretically.

=cut
