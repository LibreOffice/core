:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   $RCSfile: packimages.pl,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: hjs $ $Date: 2004-06-07 13:41:42 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

#
# packimages.pl - pack images into archives
#

use strict;
use Getopt::Long;
use File::Basename;
use Archive::Zip qw(:ERROR_CODES :CONSTANTS);

#### globals ####

my $img_global = '%GLOBAL%';  # 'global' image prefix
my $img_module = '%MODULE%';  # 'module' image prefix
my $img_custom = '%CUSTOM%';  # 'custom' image prefix

my $out_file;                # path to output archive
my $global_path;             # path to global images directory
my $module_path;             # path to module images directory
my $custom_path;             # path to custom images directory
my $imagelist_path;          # path to directory containing the image lists
my $verbose;                 # be verbose
my $extra_verbose;           # be extra verbose

#### script id #####

( my $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

my $script_rev;
my $id_str = ' $Revision: 1.4 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

print "$script_name -- version: $script_rev\n";

#### main #####

parse_options();
my $image_lists_ref = get_image_lists();
my ($global_hash_ref, $module_hash_ref, $custom_hash_ref) = iterate_image_lists($image_lists_ref);
my $zip_hash_ref = create_zip_list($global_hash_ref, $module_hash_ref, $custom_hash_ref);
create_zip_archive($zip_hash_ref);

exit(0);

#### subroutines ####

sub parse_options
{
    my $opt_help;
    my $p = Getopt::Long::Parser->new();
    my $success =$p->getoptions(
                             '-h' => \$opt_help,
                             '-o=s' => \$out_file,
                             '-g=s' => \$global_path,
                             '-m=s' => \$module_path,
                             '-c=s' => \$custom_path,
                             '-l=s' => \$imagelist_path,
                             '-v'   => \$verbose,
                             '-vv'  => \$extra_verbose
                            );

    if ( $opt_help || !$success || !$out_file || !$global_path
        || !$module_path || !$custom_path || !$imagelist_path )
    {
        usage();
        exit(1);
    }

    # Sanity checks.

    # Check if out_file can be written.
    my $out_dir = dirname($out_file);
    print_error("no such directory: '$out_dir'", 2) if ! -d $out_dir;
    print_error("can't search directory: '$out_dir'", 2) if ! -x $out_dir;
    print_error("directory is not writable: '$out_dir'", 2) if ! -w $out_dir;

    # Check paths.
    foreach ($global_path, $module_path, $custom_path, $imagelist_path) {
        print_error("no such directory: '$_'", 2) if ! -d $_;
        print_error("can't search directory: '$_'", 2) if ! -x $_;
    }
}

sub get_image_lists
{
    my @image_lists = glob("$imagelist_path/*.ilst2");

    if ( !@image_lists ) {
        print_error("can't find any image lists in '$imagelist_path'", 3);
    }

    return wantarray ? @image_lists : \@image_lists;
}

sub iterate_image_lists
{
    my $image_lists_ref = shift;

    my %global_hash;
    my %module_hash;
    my %custom_hash;

    foreach ( @{$image_lists_ref} ) {
        parse_image_list($_, \%global_hash, \%module_hash, \%custom_hash);
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
    open(IMAGE_LIST, "< $image_list");
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
        if ( /^\Q$img_global\E\/(.*)$/o ) {
            $global_hash_ref->{$1}++;
            next;
        }
        if ( /^\Q$img_module\E\/(.*)$/o ) {
            $module_hash_ref->{$1}++;
            next;
        }
        if ( /^\Q$img_custom\E\/(.*)$/o ) {
            $custom_hash_ref->{$1}++;
            next;
        }
        # parse failed if we reach this point, bail out
        close(IMAGE_LIST);
        print_error("can't parse line $linecount from file '$image_list'", 4);
    }
    close(IMAGE_LIST);

    return ($global_hash_ref, $module_hash_ref, $custom_hash_ref);
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
            next;
        }
        # it's neither in 'module' nor 'custom', record it in zip hash
        $zip_hash{$_} = $global_path;
    }
    foreach ( keys %{$module_hash_ref} ) {
        if ( exists $custom_hash_ref->{$_} ) {
            next;
        }
        # it's not in 'custom', record it in zip hash
        $zip_hash{$_} = $module_path;
    }
    foreach ( keys %{$custom_hash_ref} ) {
        $zip_hash{$_} = $custom_path;
    }

    if ( @warn_list ) {
        foreach ( @warn_list ) {
            print_warning("$_ is duplicated in 'global' and 'module' list");
        }
    }

    return \%zip_hash
}

sub create_zip_archive
{
    my $zip_hash_ref = shift;

    print_message("creating image archive ...") if $verbose;
    my $zip = Archive::Zip->new();

    foreach ( sort keys %{$zip_hash_ref} ) {
        my $path = $zip_hash_ref->{$_} . "/$_";
        print_message("zipping '$path' ...") if $extra_verbose;
        if ( !$zip->addFile($path, $_) ) {
            print_error("can't add file '$path' to image zip archive: $!", 5);
        }
    }
    my $status = $zip->writeToFileNamed($out_file);
    if ( $status != AZ_OK ) {
        print_error("write image zip archive '$out_file' failed. Reason: $status", 6);
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
    print STDERR "    -l imagelist_path  path to directory containing the image lists\n";
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
