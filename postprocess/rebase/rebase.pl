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
# rebase.pl - rebase windows dlls
#
# This perl script is to rebase all windows dlls. In principle this could
# be done with one simple command line like f.e.
# rebase -b 0x68000000 -d -R foo_dir -N bar.txt $(SOLARBINDIR)$/*.dll
# That would work fine for creating complete office install sets, but it
# could fail as soon as we are going to ship single dlls for a product
# patch. Therefore, this wrapper perl script is used. It reads a given base
# address file and rebases all files mentioned to the same address as
# previously. New dlls get appended to the list.

use strict;

#### globals #####

my $myname         = '';
my $options_string = ''; # order of options is important
my %options_hash;
my $rebase_files;
my $misc_dir = $ENV{TEMP};
my $lastaddress;
my @old_files;
my @new_files;

#### main #####

$myname = script_id();
parse_options();
my %lastrun = read_coffbase( \$lastaddress );
# Get files specified on command line. Differ between those already
# listed in coffbase (%options_hash{'C'}) and additional ones.
get_files( \@old_files, \@new_files );
# Rebase libraries already listed in coffbase to the addresses given there.
rebase_again( \@old_files, \@new_files ) if ( @old_files );
# Rebase additional files.
rebase_initially( \@new_files, $lastaddress ) if ( @new_files );

exit 0;


#### subroutines ####

sub script_id
{
    ( my $script_name = $0 ) =~ s/^.*[\\\/]([\w\.]+)$/$1/;
    return $script_name;
}


sub parse_options
{
    use Getopt::Std;
    if ( !getopts('C:b:de:l:m:R:N:v', \%options_hash) || ($#ARGV < 0) ) {
        print STDERR "Error: invalid command line.\n\n";
        usage ();
        exit 1;
    }
    # create options string (we cannot rely on a hash because for some options the
    # order is important. -R option has to be specified before -N!)
    foreach my $var ( 'C', 'b', 'e', 'l', 'R', 'N' ) {
        if ($options_hash{$var} ) {
            $options_string .= "-$var $options_hash{$var} ";
        }
    }
    $options_string .= "-d " if $options_hash{"d"};
    $options_string .= "-v " if $options_hash{"v"};
    # some basic tests
    if ( ! $options_hash{'C'}) {
        print STDERR "Error: no coffbase specified\n\n";
        usage ();
        exit 2;
    }
    if ( ! $options_hash{'b'}) {
        print STDERR "Error: no initial base address specified\n\n";
        usage ();
        exit 2;
    }
    if ($options_hash{"m"}) {
        $misc_dir = $options_hash{"m"};
    }
    if ( ! -d $misc_dir ) {
        print STDERR "Error: no directory to write work files. Please specify with -m\n";
        usage ();
        exit 3;
    }
    if ( $misc_dir !~ /[\/\\]$/ ) {
        # append finishing path separator:
        if ( $misc_dir =~ /([\/\\])/ ) {
            $misc_dir .= $1;
        }
    }
    $rebase_files = join " ", @ARGV;
    # Cygwin's perl in a W32-4nt configuration wants / instead of \ .
    $rebase_files =~ s/\\/\//g;
    return;
}


sub read_coffbase
{
    my ($addref) = shift;
    my %baseaddresses;
    my @entry;
    if ( $options_hash{'C'} ) {
        my $filename = $options_hash{'C'};
        if ( -e $filename ) {
            print "Repeated run, $filename present\n";
            open( COFFBASE, $filename) or die "Error: cannot open $filename";
            while ( my $line = <COFFBASE> ) {
                # each row consists of three entries, separated by white space:
                # dll-name base-address size
                @entry = split /\s+/ , $line ;
                if ( $entry[3] || ( ! $entry[2] ) ) {
                    print STDERR "Warning: coffbase file structure invalid?\n";
                }
                $baseaddresses{$entry[0]} = $entry[1];
                if ( $entry[3] ) {
                    print STDERR "Warning: coffbase file structure invalid?\n";
                }
            }
            close( COFFBASE );
            $$addref = $entry[1];
        } else {
            print "Initial run, $filename not yet present\n";
        }
    } else {
        die "Error: no coffbase specified.";
    }
    return %baseaddresses;
}


sub get_files
{
    use File::Basename;
    my ( $oldfiles_ref, $newfiles_ref ) = @_;
    my @target = split / /,  $rebase_files;
    foreach my $pattern ( @target ) {
        foreach my $i ( glob( $pattern ) ) {
            my $lib = File::Basename::basename $i;
        $lib =~ s/\+/\\\+/g;
            if ( grep /^$lib$/i, (keys %lastrun) ) {
                push @$oldfiles_ref, $i;
            } else {
                push @$newfiles_ref, $i;
            }
        }
    }
    return;
}


sub rebase_again
# rebase using given coffbase file
{
    my $oldfiles_ref = shift;
    my $newfiles_ref = shift;
    my @grownfiles;
    my $solarbin ="$ENV{SOLARVERSION}/$ENV{INPATH}/bin$ENV{UPDMINOREXT}";
    my $command;
    if ( $ENV{REBASE} eq "download" ) {
        $command = $ENV{OUTDIR}."/bin/rebase " . $options_string;
    }
    else {
        $command = "rebase " . $options_string;
    }
    if ( $ENV{WRAPCMD} ) {
        $command = $ENV{WRAPCMD} . " " . $command;
    }
    $command =~ s/-C /-i /;
    $command =~ s/-d//;
    $command =~ s/-b $options_hash{'b'}//;
    my $fname = $misc_dir . "rebase_again.txt";
    open ( FILES, "> $fname") or die "Error: cannot open file $fname";
    my $filesstring = join " ", @$oldfiles_ref;
    print FILES "$filesstring\n";
    close FILES;
    $command .= "\@$fname";
    # Cygwin's perl needs escaped \ in system() and open( COMMAND ... )
    if ( "$^O" eq "cygwin" ) { $command =~ s/\\/\\\\/g; }
    print "\n$command\n";
    open( COMMAND, "$command 2>&1 |") or die "Error: Can't execute $command\n";
    if ( $? ) {
        die "Error: rebase failed: $?!\n";
    }
    while( <COMMAND> ) {
        print;
        # evaluate error messages
        if ( /REBASE: ([^\s]+).*Grew too large/ ) {
            my $toobig_name = $1;
            if ( -e "$solarbin/so/$toobig_name" ) {
                push @grownfiles, "$solarbin/so/$toobig_name";
                print "name was : $toobig_name\n";
                print "push $solarbin/so/$toobig_name\n";
            } else {
                push @grownfiles, "$solarbin/$toobig_name";
            }
        }
    }
    close( COMMAND );
    if ( @grownfiles ) {
        # Some files are larger than expected and therefore could not be rebased.
        # Remove respective entries from coffbase and schedule rebase in 'rebase_initially'.
        push @$newfiles_ref, @grownfiles;
        my $coffbase = $options_hash{'C'};
        my $coffbase_new = $options_hash{'C'} . ".new";
        open( COFFBASENEW, "> $coffbase_new") or die "Error: cannot open $coffbase_new";
        open( COFFBASE, $coffbase) or die "Error: cannot open $coffbase";
        my @entry;
        while ( my $line = <COFFBASE> ) {
            @entry = split /\s+/ , $line ;
            if ( $entry[3] ) {
                print STDERR "Warning: coffbase file structure invalid?\n";
            }
            grep /^$entry[0]$/, @grownfiles or print COFFBASENEW $line;
        }
        close( COFFBASE );
        close( COFFBASENEW );
        rename $coffbase, $coffbase . ".old" or warn "Error: cannot rename $coffbase";
        rename $coffbase_new, $coffbase or warn "Error: cannot rename $coffbase_new";
    }
}


sub rebase_initially
{
    my ($files_ref, $start_address) = @_;
    my $command;
    if ( $ENV{REBASE} eq "download" ) {
        $command = $ENV{OUTDIR}."/bin/rebase " . $options_string;
    }
    else {
        $command = "rebase " . $options_string;
    }
    if ( $ENV{WRAPCMD} ) {
        $command = $ENV{WRAPCMD} . " " . $command;
    }
    $command .= $options_string;
    if ( $start_address ) {
        $command =~ s/-b $options_hash{'b'}/ -b $start_address/;
    }
    my $fname = $misc_dir . "rebase_new.txt";
    open ( FILES, "> $fname") or die "Error: cannot open file $fname";
    my $filesstring = join " ", @$files_ref;
    print FILES "$filesstring\n";
    close FILES;
    $command .= "\@$fname";
    # Cygwin's perl needs escaped \ in system() and open( COMMAND ... )
    if ( "$^O" eq "cygwin" ) { $command =~ s/\\/\\\\/g; }
    print "\n$command\n";
    my $error = system("$command");
    if ($error) {
        $error /= 256;
        die "Error: rebase failed with exit code $error!\n";
    }
}


sub usage
{
    print "Usage:\t $myname <-C filename> <-b address> [-d] [-e <Size>] [-l <filename>] [-v] [-m dir] [-R <roordir>] [-N <filename>] <file[list]> \n";
    # Options similar to rebase binary. Additional options: -m misc-directory
    print "Options:\n";
    print "\t -C coffbase_filename    Write the list of base adresses to file coffbase_filename. ";
    print                            "Mandatory.\n";
    print "\t -b address              Initial base address. Mandatory.\n";
    print "\t -e SizeAdjustment       Extra size to allow for image growth.\n";
    print "\t -d                      Top down rebase.\n";
    print "\t -l filename             Write logfile filename.\n";
    print "\t -m directory            Directory to write work files.\n";
    print "\t -R directory            Root directory.\n";
    print "\t -N filename             Specify list of files not to be rebased.\n";
    print "\t -v                      Verbose.\n";
    return;
}


