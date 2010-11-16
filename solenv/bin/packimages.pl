:
eval 'exec perl -wS $0 ${1+"$@"}'
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
# packimages.pl - pack images into archives
#

use strict;
use Getopt::Long;
use File::Find;
use File::Basename;
use Archive::Zip qw(:ERROR_CODES :CONSTANTS);

#### globals ####

my $img_global = '%GLOBALRES%';  # 'global' image prefix
my $img_module = '%MODULE%';  # 'module' image prefix

my $out_file;                # path to output archive
my $tmp_out_file;            # path to temporary output file
my $global_path;             # path to global images directory
my $module_path;             # path to module images directory
my $sort_file;               # path to file containing sorting data
my @custom_path;             # path to custom images directory
my @imagelist_path;          # pathes to directories containing the image lists
my $verbose;                 # be verbose
my $extra_verbose;           # be extra verbose
my $do_rebuild = 0;          # is rebuilding zipfile required?

my @custom_list;
#### script id #####

( my $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

my $script_rev;
my $id_str = ' $Revision: 1.17 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

print "$script_name -- version: $script_rev\n";

#### main #####

parse_options();
my $image_lists_ref = get_image_lists();
my %image_lists_hash;
foreach ( @{$image_lists_ref} ) {
    $image_lists_hash{$_}="";
}
$do_rebuild = is_file_newer(\%image_lists_hash) if $do_rebuild == 0;
my ($global_hash_ref, $module_hash_ref, $custom_hash_ref) = iterate_image_lists($image_lists_ref);
# custom_hash filled from filesystem lookup
find_custom($custom_hash_ref);
my $zip_hash_ref = create_zip_list($global_hash_ref, $module_hash_ref, $custom_hash_ref);
$do_rebuild = is_file_newer($zip_hash_ref) if $do_rebuild == 0;
if ( $do_rebuild == 1 ) {
    create_zip_archive($zip_hash_ref);
    replace_file($tmp_out_file, $out_file);
    print_message("packing  $out_file finished.");
} else {
    print_message("$out_file up to date. nothing to do.");
}

exit(0);

#### subroutines ####

sub parse_options
{
    my $opt_help;
    my $p = Getopt::Long::Parser->new();
    my @custom_path_list;
    my $custom_path_extended;
    my $success =$p->getoptions(
                             '-h' => \$opt_help,
                             '-o=s' => \$out_file,
                             '-g=s' => \$global_path,
                 '-s=s' => \$sort_file,
                             '-m=s' => \$module_path,
                             '-c=s' => \@custom_path_list,
                             '-e=s' => \$custom_path_extended,
                             '-l=s' => \@imagelist_path,
                             '-v'   => \$verbose,
                             '-vv'  => \$extra_verbose
                            );
    push @custom_path_list, $custom_path_extended if ($custom_path_extended);
    if ( $opt_help || !$success || !$out_file || !$global_path
        || !$module_path || !@custom_path_list || !@imagelist_path )
    {
        usage();
        exit(1);
    }
    #define intermediate output file
    $tmp_out_file="$out_file"."$$".$ENV{INPATH};
    # Sanity checks.

    # Check if out_file can be written.
    my $out_dir = dirname($out_file);

    # Check paths.
    foreach ($out_dir, $global_path, $module_path, @imagelist_path) {
        print_error("no such directory: '$_'", 2) if ! -d $_;
        print_error("can't search directory: '$_'", 2) if ! -x $_;
    }
    print_error("directory is not writable: '$out_dir'", 2) if ! -w $out_dir;

    # Use just the working paths
    @custom_path = ();
    foreach (@custom_path_list) {
        if ( ! -d $_ ) {
            print_warning("skipping non-existing directory: '$_'", 2);
        }
        elsif ( ! -x $_ ) {
            print_error("can't search directory: '$_'", 2);
        }
        else {
            push @custom_path, $_;
        }
    }
}

sub get_image_lists
{
    my @image_lists;
    my $glob_imagelist_path;

    foreach ( @imagelist_path ) {
        $glob_imagelist_path = $_;
        # cygwin perl
        chomp( $glob_imagelist_path = qx{cygpath -u "$glob_imagelist_path"} ) if "$^O" eq "cygwin";
        push @image_lists, glob("$glob_imagelist_path/*.ilst");
    }
    if ( !@image_lists ) {
        print_error("can't find any image lists in '@imagelist_path'", 3);
    }

    return wantarray ? @image_lists : \@image_lists;
}

sub iterate_image_lists
{
    my $image_lists_ref = shift;

    my %global_hash;
    my %module_hash;
    my %custom_hash;

    foreach my $i ( @{$image_lists_ref} ) {
        parse_image_list($i, \%global_hash, \%module_hash, \%custom_hash);
    }

    return (\%global_hash, \%module_hash, \%custom_hash);
}

sub parse_image_list
{
    my $image_list      = shift;
    my $global_hash_ref = shift;
    my $module_hash_ref = shift;
    my $custom_hash_ref = shift;

    print_message("parsing '$image_list' ...") if $verbose;
    my $linecount = 0;
    open(IMAGE_LIST, "< $image_list") or die "ERROR: can't open $image_list: $!";
    while ( <IMAGE_LIST> ) {
        $linecount++;
        next if /^\s*#/;
        next if /^\s*$/;
        # clean up trailing whitespace
        tr/\r\n//d;
        s/\s+$//;
        # clean up backslashes and double slashes
        tr{\\}{/}s;
        tr{/}{}s;
        # hack "res" back into globals
        if ( /^\Q$img_global\E\/(.*)$/o ) {
            $global_hash_ref->{"res/".$1}++;
            next;
        }
        if ( /^\Q$img_module\E\/(.*)$/o ) {
            $module_hash_ref->{$1}++;
            next;
        }
        # parse failed if we reach this point, bail out
        close(IMAGE_LIST);
        print_error("can't parse line $linecount from file '$image_list'", 4);
    }
    close(IMAGE_LIST);

    return ($global_hash_ref, $module_hash_ref, $custom_hash_ref);
}

sub find_custom
{
    my $custom_hash_ref = shift;
    my $keep_back;
    for my $path (@custom_path) {
    find({ wanted => \&wanted, no_chdir => 0 }, $path);
    foreach ( @custom_list ) {
        if ( /^\Q$path\E\/(.*)$/ ) {
        $keep_back=$1;
        if (!defined $custom_hash_ref->{$keep_back}) {
            $custom_hash_ref->{$keep_back} = $path;
        }
        }
    }
    }
}

sub wanted
{
    my $file = $_;

    if ( $file =~ /.*\.png$/ && -f $file ) {
        push @custom_list, $File::Find::name;
    }
}

sub create_zip_list
{
    my $global_hash_ref = shift;
    my $module_hash_ref = shift;
    my $custom_hash_ref = shift;

    my %zip_hash;
    my @warn_list;

    print_message("assemble image list ...") if $verbose;
    foreach ( keys %{$global_hash_ref} ) {
        # check if in 'global' and in 'module' list and add to warn list
        if ( exists $module_hash_ref->{$_} ) {
            push(@warn_list, $_);
            next;
        }
        if ( exists $custom_hash_ref->{$_} ) {
            $zip_hash{$_} = $custom_hash_ref->{$_};
            next;
        }
        # it's neither in 'module' nor 'custom', record it in zip hash
        $zip_hash{$_} = $global_path;
    }
    foreach ( keys %{$module_hash_ref} ) {
        if ( exists $custom_hash_ref->{$_} ) {
            $zip_hash{$_} = $custom_hash_ref->{$_};
            next;
        }
        # it's not in 'custom', record it in zip hash
        $zip_hash{$_} = $module_path;
    }

    if ( @warn_list ) {
        foreach ( @warn_list ) {
            print_warning("$_ is duplicated in 'global' and 'module' list");
        }
    }

    return \%zip_hash
}

sub is_file_newer
{
    my $test_hash_ref = shift;
    my $reference_stamp = 0;

    print_message("checking timestamps ...") if $verbose;
    if ( -e $out_file ) {
        $reference_stamp = (stat($out_file))[9];
        print_message("found $out_file with $reference_stamp ...") if $verbose;
    }
    return 1 if $reference_stamp == 0;

    foreach ( sort keys %{$test_hash_ref} ) {
        my $path = $test_hash_ref->{$_};
        $path .= "/" if "$path" ne "";
        $path .= "$_";
        print_message("checking '$path' ...") if $extra_verbose;
        my $mtime = (stat($path))[9];
        return 1 if $reference_stamp < $mtime;
    }
    return 0;
}

sub optimize_zip_layout($)
{
    my $zip_hash_ref = shift;

    if (!defined $sort_file) {
    print_message("no sort file - sorting alphabetically ...") if $verbose;
    return sort keys %{$zip_hash_ref};
    }
    print_message("sorting from $sort_file ...") if $verbose;

    my $orderh;
    my %included;
    my @sorted;
    open ($orderh, $sort_file) || die "Can't open $sort_file: $!";
    while (<$orderh>) {
    /^\#.*/ && next; # comments
    s/[\r\n]*$//;
    /^\s*$/ && next;
    my $file = $_;
    if (!defined $zip_hash_ref->{$file}) {
        print "unknown file '$file'\n" if ($extra_verbose);
    } else {
        push @sorted, $file;
        $included{$file} = 1;
    }
    }
    close ($orderh);

    for my $img (sort keys %{$zip_hash_ref}) {
    push @sorted, $img if (!$included{$img});
    }

    print_message("done sort ...") if $verbose;

    return @sorted;
}

sub create_zip_archive
{
    my $zip_hash_ref = shift;

    print_message("creating image archive ...") if $verbose;
    my $zip = Archive::Zip->new();

# FIXME: test - $member = addfile ... $member->desiredCompressionMethod( COMPRESSION_STORED );
# any measurable performance win/loss ?
    foreach ( optimize_zip_layout($zip_hash_ref) ) {
        my $path = $zip_hash_ref->{$_} . "/$_";
        print_message("zipping '$path' ...") if $extra_verbose;
        my $member = $zip->addFile($path, $_, COMPRESSION_STORED);
        if ( !$member ) {
            print_error("can't add file '$path' to image zip archive: $!", 5);
        }
    }
    my $status = $zip->writeToFileNamed($tmp_out_file);
    if ( $status != AZ_OK ) {
        print_error("write image zip archive '$tmp_out_file' failed. Reason: $status", 6);
    }
    return;
}

sub replace_file
{
    my $source_file = shift;
    my $dest_file = shift;
    my $result = 0;

    $result = unlink($dest_file) if -f $dest_file;
    if ( $result != 1 && -f $dest_file ) {
        unlink $source_file;
        print_error("couldn't remove '$dest_file'",1);
    }  else {
        if ( !rename($source_file, $dest_file)) {
            unlink $source_file;
            print_error("couldn't rename '$source_file'",1);
        }
    }
    return;
}

sub usage
{
    print STDERR "Usage: packimages.pl [-h] -o out_file -g g_path -m m_path -c c_path -l imagelist_path\n";
    print STDERR "Creates archive of images\n";
    print STDERR "Options:\n";
    print STDERR "    -h                 print this help\n";
    print STDERR "    -o out_file        path to output archive\n";
    print STDERR "    -g g_path          path to global images directory\n";
    print STDERR "    -m m_path          path to module images directory\n";
    print STDERR "    -c c_path          path to custom images directory\n";
    print STDERR "    -s sort_file       path to image sort order file\n";
    print STDERR "    -l imagelist_path  path to directory containing image lists (may appear mutiple times)\n";
    print STDERR "    -v                 verbose\n";
    print STDERR "    -vv                very verbose\n";
}

sub print_message
{
    my $message     = shift;

    print "$script_name: ";
    print "$message\n";
    return;
}

sub print_warning
{
    my $message     = shift;

    print STDERR "$script_name: ";
    print STDERR "WARNING $message\n";
    return;
}

sub print_error
{
    my $message     = shift;
    my $error_code  = shift;

    print STDERR "$script_name: ";
    print STDERR "ERROR: $message\n";

    if ( $error_code ) {
        print STDERR "\nFAILURE: $script_name aborted.\n";
        exit($error_code);
    }
    return;
}
