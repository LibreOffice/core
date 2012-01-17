#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************



#*************************************************************************
#
# createPDBRelocators - create for pdb relocator files
#                       PDB relocator files are used to find debug infos
#                       for analysis of creash reports
#
# usage: create_pdb_relocators($inpath, $milestoneext, $pre);
#
#*************************************************************************

package CreatePDBRelocators;

use strict;
use File::Basename;
use SourceConfig;

sub new
{
    my $Object = shift;
    my $solarversion = shift;
    my $workdir;
    my $relworkdir;
    my $self = {};
    my @basedirs;
    my @repos;

    if (!defined ($solarversion)) {
        $solarversion = $ENV{SOLARVERSION};
    }
    if ( !$solarversion ) {
        print STDERR "can't determine SOLARVERSION.\n";
        exit (1);
    }

    $self->{SOLARVERSION} = $solarversion;

    $workdir = $ENV{WORKDIR};
    if ( !$workdir ) {
        print STDERR "can't determine WORKDIR.\n";
        exit (1);
    }

    if ( $workdir =~ /^$solarversion/ ) {
        $relworkdir = $workdir;
        $relworkdir =~ s/^$solarversion\///;
    } else {
        print STDERR "ERROR: workdir outside $solarversion unsupported\n";
        exit (2);
    }
    my $SourceConfigObj = SourceConfig->new();
    @repos = $SourceConfigObj->get_repositories();
    if ( defined $ENV{UPDMINOREXT} ) {
        foreach my $onedir ( @repos ) {
            push( @basedirs, $onedir.$ENV{UPDMINOREXT} );
        }
    }
    # basdirs is repositories (dmake) + workdir (gnu make)
    push(@basedirs, $relworkdir);
    if (!scalar @basedirs) {
        print STDERR "no basedir and no working directory found.\n";
        exit (2);
    }
    $self->{BASEDIRS} = \@basedirs;
    bless($self, $Object);
    return $self;
}

sub create_pdb_relocators
{
    my $self = shift;
    my $inpath   = shift;
    my $milestoneext    = shift;
    my $pre      = shift;

    my $solarversion = $self->{SOLARVERSION};
    my $root_dir = "$solarversion/$inpath";

    # sanitize path
    $root_dir =~ s/\\/\//g;
    my $o =~ s/\\/\//g;
    my $premilestoneext = $milestoneext;
    if ( $pre ne "" ) {
        $premilestoneext = ~ s/^\.//;
        $premilestoneext = ".pre$premilestoneext";
    }
    my $pdb_dir = $root_dir . "/pdb$premilestoneext";
    my $pdb_so_dir = $root_dir . "/pdb$premilestoneext/so";

    # create pdb directories if necessary
    if ( ! -d $pdb_dir ) {
        if ( !mkdir($pdb_dir, 0775) ) {
            print STDERR "can't create directory '$pdb_dir'\n";
                return undef;
        }
    }
    if ( ! -d $pdb_so_dir ) {
        if ( !mkdir($pdb_so_dir, 0775) ) {
            print STDERR "can't create directory '$pdb_so_dir'\n";
                return undef;
        }
    }

    # collect files
    foreach my $basedir (@{$self->{BASEDIRS}}) {
        my @pdb_files;
        my $o = $self->{SOLARVERSION} . "/$basedir";
        $basedir =~ s/(.*?)\.(.*)/$1/;
        $self->collect_files( $o, $inpath, \@pdb_files);

        foreach (@pdb_files) {
            my $relocator = basename($_) . ".location";
            /$o\/(.*)/i;

            my $src_location = $1;

            my $location = "";
            my $target = "";
            if ( $src_location =~ /\/so\// )
            {
                $location = "../../../$basedir$milestoneext/" . $src_location;
                $target = "$pdb_dir/so/$relocator";
            }
            else
            {
                $location = "../../$basedir$milestoneext/" . $src_location;
                $target = "$pdb_dir/$relocator";
            }

            if ( !open(RELOCATOR, ">$target") ) {
                print STDERR "can't write file '$target'\n";
                return undef;
            }
            print RELOCATOR "$location\n";
            close(RELOCATOR);
        }
    }
    return 1;
}

sub collect_files_from_all_basedirs
{
    my $self = shift;
    my ($platform, $filesref) = @_;
    my $basedir;
    my $ret;
    foreach $basedir (@{$self->{BASEDIRS}}) {
        my $srcdir = $self->{SOLARVERSION} . "/$basedir";
        $ret |= $self->collect_files ($srcdir, $platform, $filesref);
    }
    return $ret;
}

sub collect_files
{
    my $self = shift;
    my ($srcdir, $platform, $filesref) = @_;
    my $template = "$srcdir/*/$platform";
    my $template2 = "$srcdir/LinkTarget";
    if ( $ENV{GUI} eq "WNT" ) {
        # collect all pdb files on o:
        # regular glob does not work with two wildcard on WNT
        my @bin    = glob("$template/bin/*.pdb");
        my @bin_so = glob("$template/bin/so/*.pdb");
        my @workdir = glob("$template2/*/*.pdb");
        # we are only interested in pdb files which are accompanied by
        # .exe or .dll which the same name
        foreach (@bin, @bin_so, @workdir) {
            my $dir  = dirname($_);
            my $base = basename($_, ".pdb");
            my $exe = "$dir/$base.exe";
            my $dll = "$dir/$base.dll";
            if ( -e $exe || -e $dll ) {
                push(@$filesref, $_);
            }
        }
    }
    else {
        # collect all shared libraries on o:
        my @lib = glob("$template/lib/*.so*");
        my @workdir_lib = glob("$template2/Library/*.so*");
        my @lib_so = glob("$template/lib/so/*.so*");
        my @mac_lib = glob("$template/lib/*.dylib*");
        my @mac_workdir_lib = glob("$template2/Library/*.dylib*");
        my @mac_lib_so = glob("$template/lib/so/*.dylib*");
        # collect all binary executables on o:
        my @bin = $self->find_binary_execs("$template/bin");
        my @workdir_bin = $self->find_binary_execs("$template2/Executable");
        my @bin_so = $self->find_binary_execs("$template/bin/so");
        push(@$filesref, (@lib, @lib_so, @workdir_lib, @mac_lib, @mac_workdir_lib, @mac_lib_so, @bin, @workdir_bin, @bin_so));
    }
    return 1;
}

sub find_binary_execs
{
    my $self = shift;
    my $path = shift;
    my @files = glob("$path/*");
    my @execs = grep(-x $_, @files);
    my @elf_files = grep(`file $_` =~ /ELF/, @execs);
    my @MachO_files = grep(`file $_` =~ /Mach\-O/, @execs);
    return ( @elf_files, @MachO_files );
}

1; # required

