:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

#
# packconfig.pl - pack xml configuration into archives
#

use strict;
use Getopt::Long;
use File::Find;
use File::Basename;
use File::Spec;
use Archive::Zip qw(:ERROR_CODES :CONSTANTS);

#### globals ####

my $out_path;                # path to output archives in
my $files_path;              # path to look for desired files
my $verbose;                 # be verbose
my $extra_verbose;           # be extra verbose
my $current_lang;            # big fat global because File::Find is a pig

#### script id #####

( my $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

#### main #####

parse_options();

#pack the .ui translations
my @langs = split(/\s+/, $ENV{WITH_LANG_LIST});
foreach (@langs) {
    next if ($_ eq "en-US");
    my %files_hash;
    my $file_ref = get_lang_files(\%files_hash, $_);
    my $out_file="$out_path"."uiconfig_".$_.".zip";
    packzip(\%files_hash, $out_file);
}

#pack the core files
my %files_hash;
my $file_ref = get_core_files(\%files_hash);
my $out_file="$out_path"."uiconfig.zip";
packzip(\%files_hash, $out_file);

exit(0);

#### subroutines ####

sub packzip
{
    my $file_hash_ref = shift;
    my $output_file = shift;

    # Check if output_file can be written.
    my $out_dir = dirname($output_file);
    print_error("no such directory: '$out_dir'", 2) if ! -d $out_dir;
    print_error("can't search directory: '$out_dir'", 2) if ! -x $out_dir;
    print_error("directory is not writable: '$out_dir'", 2) if ! -w $out_dir;

    # is rebuilding zipfile required?
    my $do_rebuild = is_file_newer($file_hash_ref, $output_file);

    if ( $do_rebuild == 1 ) {
        #temporary intermediate output file
        my $tmp_out_file="$output_file"."$$".$ENV{INPATH};
        create_zip_archive($file_hash_ref, $tmp_out_file);
        replace_file($tmp_out_file, $output_file);
        print_message("packing  $output_file finished.");
    } else {
        print_message("$output_file up to date. nothing to do.");
    }
}

sub parse_options
{
    my $opt_help;
    my $p = Getopt::Long::Parser->new();
    my $success =$p->getoptions(
                             '-h' => \$opt_help,
                             '-o=s' => \$out_path,
                             '-i=s' => \$files_path,
                             '-v'   => \$verbose,
                             '-vv'  => \$extra_verbose
                            );

    if ( $opt_help || !$success || !$out_path || !$files_path )
    {
        usage();
        exit(1);
    }

    # Check paths.
    foreach ($files_path) {
        print_error("no such directory: '$_'", 2) if ! -d $_;
        print_error("can't search directory: '$_'", 2) if ! -x $_;
    }
}

sub get_core_files
{
    local @main::file_list;

    my $files_hash_ref = shift;
    find_core_files($files_hash_ref);

    if ( !keys %$files_hash_ref ) {
        print_error("can't find any core config files in '$files_path'", 3);
    }

    return wantarray ? @main::file_list : \@main::file_list;
}

sub find_core_files
{
    my $files_hash_ref = shift;
    find({ wanted => \&wanted_core, no_chdir => 0 }, "$files_path");
    foreach ( @main::file_list ) {
        /^\Q$files_path\E\/(.*)$/o;
        $files_hash_ref->{$1}++;
    }
}

sub get_lang_files
{
    local @main::file_list;

    my $files_hash_ref = shift;
    my $lang = shift;
    find_lang_files($files_hash_ref, $lang);

    if ( !keys %$files_hash_ref ) {
        print_error("can't find any lang config files in '$files_path'", 3);
    }

    return wantarray ? @main::file_list : \@main::file_list;
}

sub find_lang_files
{
    my $files_hash_ref = shift;
    $current_lang = shift;
    find({ wanted => \&wanted_lang, no_chdir => 0 }, "$files_path");
    foreach ( @main::file_list ) {
        /^\Q$files_path\E\/(.*)$/o;
        $files_hash_ref->{$1}++;
    }
}

sub wanted_core
{
    my $file = $_;

    if ( $file =~ /.*\.(ui|xml)$/ && -f $file ) {
        push @main::file_list, $File::Find::name;
    }
}

sub wanted_lang
{
    my $file = $_;

    my @dirs = File::Spec->splitdir($File::Find::dir);

    if ($dirs[-1] eq $current_lang) {
        push @main::file_list, $File::Find::name;
    }
}

sub is_file_newer
{
    my $test_hash_ref = shift;
    my $zip_file = shift;
    my $reference_stamp = 0;

    print_message("checking timestamps for $zip_file ...") if $verbose;
    if ( -e $zip_file ) {
        $reference_stamp = (stat($zip_file))[9];
        print_message("found $zip_file with $reference_stamp ...") if $verbose;
    }
    return 1 if $reference_stamp == 0;

    foreach ( sort keys %{$test_hash_ref} ) {
        my $path = $files_path;
        $path .= "/" if "$path" ne "";
        $path .= "$_";
        print_message("checking '$path' ...") if $extra_verbose;
        my $mtime = (stat($path))[9];
        return 1 if $reference_stamp < $mtime;
    }
    return 0;
}

sub create_zip_archive
{
    my $zip_hash_ref = shift;
    my $zip_output_file = shift;
    print_message("creating config archive ...") if $verbose;
    my $zip = Archive::Zip->new();

    my $platform = $ENV{INPATH};

    foreach ( sort keys %{$zip_hash_ref} ) {
        my $path = "$files_path/$_";
    # only Mac OS X is concerned here
    # but changes for other platforms can easely be added following the same principle
    if ( ( $platform =~ /^.*macx*/) && ($path =~ /^.*menubar.xml/ ) ) {
        $path = modify_mac_menus($path);
    }
    print_message("zipping '$path' ...") if $extra_verbose;
    if ( !$zip->addFile($path, $_) ) {
        print_error("can't add file '$path' to config zip archive: $!", 5);
    }
    }
    my $status = $zip->writeToFileNamed($zip_output_file);
    if ( $status != AZ_OK ) {
        print_error("write image zip archive '$zip_output_file' failed. Reason: $status", 6);
    }
    return;
}

sub modify_mac_menus
{
    my $new_file_name = "$ENV{'WORKDIR'}/CustomTarget/postprocess/misc/$_";

    my $new_directory = $new_file_name;
    $new_directory =~ s/\/menubar.xml//;
    if ( ! -e $new_directory) {
    `mkdir -p "$new_directory"`;
    }

    my $old_file_name = "$files_path/$_";

    `cp $old_file_name $new_file_name`;

    my $temp_file_name = "$new_file_name"."_tmp";
    my $xsl_file = "macosx/macosx_menubar_modification.xsl";

    my $result = `xsltproc $xsl_file $new_file_name > $temp_file_name`;

    if ( $result != 0) {
    print_error("xsltproc '$xsl_file' '$new_file_name'> '$temp_file_name' failed",1)
    }

    replace_file( $temp_file_name, $new_file_name );
    return $new_file_name;
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
    print STDERR "Usage: packconfig.pl [-h] -o out_file -i file_path\n";
    print STDERR "Creates archive of userinterface config files\n";
    print STDERR "Options:\n";
    print STDERR "    -h                 print this help\n";
    print STDERR "    -o out_path        path to output archive\n";
    print STDERR "    -i file_path       path to directory containing the config files\n";
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
