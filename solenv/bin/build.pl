:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   $RCSfile: build.pl,v $
#
#   $Revision: 1.32 $
#
#   last change: $Author: vg $ $Date: 2001-08-10 14:20:25 $
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
# build - build entire project
#

use Cwd;

#### script id #####

( $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

$id_str = ' $Revision: 1.32 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

print "$script_name -- version: $script_rev\n";

#########################
#                       #
#   Globale Variablen   #
#                       #
#########################
$QuantityToBuild = 0;
$BuildAllParents = 0;
$show = 0;
$deliver = 0;
%LocalDepsHash = ();
%DepsArchive = ();
%BuildQueue = ();
%PathHash = ();
%PlatformHash = ();
%DeadDependencies = ();
%AliveDependencies = ();
%ParentDepsHash = ();
@UnresolvedParents = ();
@dmake_args = ();
%DeadParents = ();
$CurrentPrj = "";
$StandDir = GetStandDir();
$build_from = "";
$is_from_built = 0;
#$BuildAllParents = HowToBuild();
#$QuantityToBuild = GetQuantityToBuild();
&get_options;
$ENV{mk_tmp} = "1";
%prj_platform = ();

#### main ####

$dmake = GetDmakeCommando();
BuildAll();
@TotenEltern = keys %DeadParents;
if ($#TotenEltern != -1) {
    my ($DeadPrj);
    print "\nWARNING! Project(s):\n\n";
    foreach $DeadPrj (@TotenEltern) {
        print "$DeadPrj\n";
    };
    print "\nnot found and couldn't be built. Correct build.lsts.\n";
};
$ENV{mk_tmp} = "";

#########################
#                       #
#      Procedures       #
#                       #
#########################

#
# Get dependencies hash of the current and all parent projects
#
sub GetParentDeps {
    my ($ParentsString, @DepsArray, $Prj, $Parent, @TotenEltern);
    $ParentsString = GetParentsString(".");
    @DepsArray = GetDependenciesArray($ParentsString);
    @UnresolvedParents = @DepsArray;
    $ParentDepsHash{$CurrentPrj} = [@DepsArray];
    ResolveParentsLoop:
    while ($Prj = pop(@UnresolvedParents)) {
        my (@DepsArray);
        if (!($ParentsString = GetParentsString($StandDir.$Prj))) {
            $DeadParents{$Prj} = 1;
            $ParentDepsHash{$Prj} = [];
            next ResolveParentsLoop;
        };
        @DepsArray = GetDependenciesArray($ParentsString);
        $ParentDepsHash{$Prj} = [@DepsArray];
        foreach $Parent (@DepsArray) {
            if (!defined($ParentDepsHash{$Parent})) {
                push (@UnresolvedParents, $Parent);
            };
        };
    };
    print "\n";
    @TotenEltern = keys %DeadParents;
    foreach $Parent (@TotenEltern) {
        RemoveFromDependencies($Parent, %ParentDepsHash);
    };
};


#
# Build everything that should be built
#
sub BuildAll {
    if ($BuildAllParents) {
        my ($Prj, $PrjDir, $DeadPrj, @TotenEltern);
        GetParentDeps();
        @TotenEltern = keys %DeadParents;
        foreach $DeadPrj (@TotenEltern) {
            delete $ParentDepsHash{$DeadPrj};
            RemoveFromDependencies($DeadPrj, \%ParentDepsHash);
        };
        if ($build_from) {
            &remove_extra_prjs(\%ParentDepsHash);
        };
        while ($Prj = PickPrjToBuild(\%ParentDepsHash)) {
            print "\n=============\n";
            print "Building project $Prj\n";
            print   "=============\n";
            $PrjDir = CorrectPath($StandDir.$Prj);
            if ($ENV{GUI} eq "UNX") {
                use Cwd 'chdir';
            };
            chdir $PrjDir;
            cwd();
            BuildPrj($PrjDir) if (!$deliver);
            system ("$ENV{DELIVER}") if (!$show);
            RemoveFromDependencies($Prj, \%ParentDepsHash);
        };
    } else {
        BuildPrj(".");
    };
};


#
# Start build given project
#
sub MakeDir {
    my ($DirToBuild, $BuildDir, $error);
    $DirToBuild = shift;
    $BuildDir = CorrectPath($StandDir.$PathHash{$DirToBuild});
    if ($ENV{GUI} eq "UNX") {
        use Cwd 'chdir';
    };
    if (chdir ($BuildDir)) {
        print "$BuildDir\n";
    } else {
        print STDERR "\n$BuildDir not found!!\n\n";
        exit (1);
    };
    cwd();
    if (!$show) {
        $error = system ("$dmake");
        if (!$error) {
            RemoveFromDependencies($DirToBuild, \%LocalDepsHash);
        } else {
            print STDERR "Error $error occurred while making $BuildDir\n";
            $ENV{mk_tmp} = "";
            exit(1);
        };
    } else {
        RemoveFromDependencies($DirToBuild, \%LocalDepsHash);
    };
};


#
# Get string (list) of parent projects to build
#
sub GetParentsString {
    my ($PrjDir);
    $PrjDir = shift;
    if (!open (PrjBuildFile, $PrjDir."/prj/build.lst")) {
        return "";
    };
    while (<PrjBuildFile>) {
        s/\r\n//;
        if ($_ =~ /([\:]+)([\t | \s]+)/) {
            close PrjBuildFile;
            return $';
        };
    };
    close PrjBuildFile;
    return "NULL";
};

#
# get folders' platform infos
#
sub get_prj_platform {
    my ($prj_alias, $line);
    while(<PrjBuildFile>) {
        s/\r\n//;
        $line++;
        if ($_ =~ /nmake/) {
            if ($' =~ /\s+-\s+(\w+)[,\S+]*\s+(\S+)/ ) {
                my $platform = $1;
                my $alias = $2;
                if ($alias eq 'NULL') {
                    print "There is no correct alias set in the line $line!\n";
                    exit (1);
                };
                &mark_platform($alias, $platform);
            } else {
                print STDERR "Misspelling in line: \n$_\n";
                exit(1);
            };
        };
    };
    seek(PrjBuildFile, 0, 0);
};

#
# Getting hashes of all internal dependencies and additional
# infos for given project
#
sub BuildPrj {
    my ($dummy, $PrjToBuild);
    $PrjToBuild = shift;
    open (PrjBuildFile, "prj/build.lst");
    &get_prj_platform;
    BuildLstLoop:
    while (<PrjBuildFile>) {
        s/\r\n//;
        if ($_ =~ /nmake/) {
            my ($Platform, $Dependencies, $Dir, $DirAlias, @Array);
            $Dependencies = $';
            $dummy = $`;
            $dummy =~ /(\S+)\s+(\S+)/;
            $Dir = $2;
            $Dependencies =~ /(\w+)/;
            $Platform = $1;
            $Dependencies = $';
            while ($Dependencies =~ /,(\w+)/) {
                $Dependencies = $';
            };
            $Dependencies =~ /\s+(\S+)\s+/;
            $DirAlias = $1;
            if (!&CheckPlatform($Platform)) {
                $DeadDependencies{$DirAlias} = 1;
                next BuildLstLoop;
            };
            $PlatformHash{$DirAlias} = 1;
            $Dependencies = $';
            @Array = GetDependenciesArray($Dependencies);
            $LocalDepsHash{$DirAlias} = [@Array];
            $BuildQueue{$DirAlias} = 1;
            $PathHash{$DirAlias} = $Dir;
        };
    };
    close PrjBuildFile;
    %DepsArchive = %LocalDepsHash;
    foreach $Dir (keys %DeadDependencies) {
        next if defined $AliveDependencies{$Dir};
        if (!&IsHashNative($Dir)) {
            &RemoveFromDependencies($Dir, \%LocalDepsHash);
            delete $DeadDependencies{$Dir};
        };
    };
    BuildDependent();
};

#
# mark platform in order to proof if alias is used according to specs
#
sub mark_platform {
    my $prj_alias = shift;
    if (exists $prj_platform{$prj_alias}) {
        $prj_platform{$prj_alias} = 'all';
    } else {
        $prj_platform{$prj_alias} = shift;
    };
};


#
# Convert path from abstract (with '\' and/or '/' delimiters)
# to system-dependent
#
sub CorrectPath {
    $_ = shift;
    if ($ENV{GUI} eq "UNX") {
        s/\\/\//g;
    } elsif (   ($ENV{GUI} eq "WNT") ||
                ($ENV{GUI} eq "WIN") ||
                ($ENV{GUI} eq "MACOSX") ||
                ($ENV{GUI} eq "OS2")) {
        s/\//\\/g;
    } else {
        $ENV{mk_tmp} = "";
        die "\nNo environment set\n";
    };
    return $_;
};


#
# Get platform-dependent dmake commando
#
sub GetDmakeCommando {
    my ($dmake, $arg);

    # Setting alias for dmake
    $dmake = "dmake";
    while ($arg = pop(@dmake_args)) {
        $dmake .= " "."$arg";
    };
    return $dmake;
};


#
# Procedure prooves if current dir is a root dir of the drive
#
sub IsRootDir {
    my ($Dir);
    $Dir = shift;
    if (        (($ENV{GUI} eq "UNX") ||
                 ($ENV{GUI} eq "MACOSX")) &&
                ($Dir eq "\/")) {
        return 1;
    } elsif (   (($ENV{GUI} eq "WNT") ||
                 ($ENV{GUI} eq "WIN") ||
                 ($ENV{GUI} eq "OS2")) &&
                ($Dir =~ /\S:\/$/)) {
        return 1;
    } else {
        return 0;
    };
};


#
# Procedure retrieves list of projects to be built from build.lst
#
sub GetStandDir {
    my ($StandDir);
DirLoop:
    do {
        $StandDir = cwd();
        if (open(PrjBuildFile, "prj/build.lst")) {
            $StandDir =~ /(\w+$)/;
            $StandDir = $`;
            $CurrentPrj = $1;
            close(PrjBuildFile);
            return $StandDir;
        } elsif (IsRootDir($StandDir)) {
            $ENV{mk_tmp} = "";
            die "Found no project to build\n"
        };
    }
    while (chdir '..');
};


#
# Build the entire project according to queue of dependencies
#
sub BuildDependent {
    my ($Dir);
    while ($Dir = PickPrjToBuild(\%LocalDepsHash)) {
        MakeDir($Dir);
        $Dir = "";
    };
};


#
# Removes projects which it is not necessary to build
#
sub remove_extra_prjs {
    my ($Prj, $DepsHash, @build_from_deps);
    $DepsHash = shift;
    if (!(defined ($$DepsHash{$build_from}))) {
        print "No direct dependency to or no project $build_from found\n";
        exit (1);
    };
    delete $$DepsHash{$build_from};
    while ($Prj = FindIndepPrj($DepsHash)) {
        RemoveFromDependencies($Prj, $DepsHash);
        delete $$DepsHash{$Prj};
    };
    $$DepsHash{$build_from} = ();
    $is_from_built = 1;
};


#
# Picks project which can be build now from hash and deletes it from hash
#
sub PickPrjToBuild {
    my ($Prj, $DepsHash);
    $DepsHash = shift;
    $Prj = FindIndepPrj($DepsHash);
    delete $$DepsHash{$Prj};
    return $Prj;
};


#
# Make a decision if the project should be built on this platform
#
sub CheckPlatform {
    my ($Platform);
    $Platform = shift;
    if ($Platform eq "all") {
        return 1;
    } elsif (($ENV{GUI} eq "WNT") &&
                            (($Platform eq "w") || ($Platform eq "n"))) {
        return 1;
    } elsif (($ENV{GUI} eq "WIN") && ($Platform eq "w")) {
        return 1;
    } elsif (($ENV{GUI} eq "UNX") && ($Platform eq "u")) {
        return 1;
    } elsif (($ENV{GUI} eq "MACOSX") && ($Platform eq "m")) {
        return 1;
    } elsif (($ENV{GUI} eq "OS2") && ($Platform eq "p")) {
        return 1;
    };
    return 0;
};


#
# Remove project to build ahead from dependencies and make an array
# of all from given project dependent projects
#
sub RemoveFromDependencies {
    my ($ExclPrj, $i, $Prj, $Dependencies);
    $ExclPrj = shift;
    $Dependencies = shift;
    foreach $Prj (keys %$Dependencies) {
        PrjDepsLoop:
        foreach $i (0 .. $#{$$Dependencies{$Prj}}) {
            if (${$$Dependencies{$Prj}}[$i] eq $ExclPrj) {
                splice (@{$$Dependencies{$Prj}}, $i, 1);
                $i = 0;
                last PrjDepsLoop;
            };
        };
    };
};


#
# Find undependent project
#
sub FindIndepPrj {
    my ($Prj, @Prjs, @PrjDeps, $Dependencies, $i);
    $Dependencies = shift;
    @Prjs = keys %$Dependencies;
    if ($#Prjs != -1) {
        PrjLoop:
        foreach $Prj (@Prjs) {
            if (IsHashNative($Prj)) {
                next PrjLoop;
            };
            if (!(defined $$Dependencies{$Prj})) {
                return $Prj;
            };
            @PrjDeps = @{$$Dependencies{$Prj}};
            if ($#PrjDeps == -1) {
                return $Prj;
            };
        };
        # If there are only dependent projects in hash - generate error
        if ($build_from && !($is_from_built)) {
            return "";
        };
        print "\nError: projects";
        DeadPrjLoop:
        foreach $Prj (keys %$Dependencies) {
            if (IsHashNative($Prj)) {
                next DeadPrjLoop;
            };
            $i = 0;
            print "\n", $Prj, " depends on:";
            foreach $i (0 .. $#{$$Dependencies{$Prj}}) {
                print " ", ${$$Dependencies{$Prj}}[$i];
            };
        };
        print "\nhave dead or circular dependencies\n\n";
        $ENV{mk_tmp} = "";
        exit (1);
    };
};


#
# Check if given entry is HASH-native, that is not a user-defined data
#
sub IsHashNative {
    my ($Prj);
    $Prj = shift;
    if ($Prj =~ /^HASH\(0x[\d | a | b | c | d | e | f]{6,}\)/) {
        return 1;
    } else {
        return 0;
    };
};


#
# Getting array of dependencies from the string given
#
sub GetDependenciesArray {
    my ($DepString, @Dependencies, $ParentPrj);
    @Dependencies = ();
    $DepString = shift;
    while (!($DepString =~ /^NULL/)) {
        $DepString =~ /(\S+)\s+/;
        $ParentPrj = $1;
        $DepString = $';
        if ($ParentPrj =~ /\.(\w+)$/) {
            $ParentPrj = $`;
            if (($prj_platform{$ParentPrj} ne $1) &&
                ($prj_platform{$ParentPrj} ne 'all')) {
                print "$ParentPrj\.$1 is a wrong dependency identifier!\nCheck if it is platform dependent\n";
                exit (1);
            };
            if (&CheckPlatform($1)) {
                $AliveDependencies{$ParentPrj} = 1;
            }
            push(@Dependencies, $ParentPrj);
        } else {
            if ((exists($prj_platform{$ParentPrj})) &&
                ($prj_platform{$ParentPrj} ne 'all') ) {
                print "$ParentPrj is a wrong used dependency identifier!\nCheck if it is platform dependent\n";
                exit (1);
            };
            push(@Dependencies, $ParentPrj);
        };
    };
    return @Dependencies;
};


#
# Getting current directory list
#
sub GetDirectoryList {
    my ($Path);
    $Path = shift;
    opendir(CurrentDirList, $Path);
    @DirectoryList = readdir(CurrentDirList);
    closedir(CurrentDirList);
    return @DirectoryList;
};

#
# Get all options passed
#
sub get_options {
    my $arg;
    #&usage() && exit(0) if ($#ARGV == -1);
    #$QuantityToBuild
    while ($arg = shift @ARGV) {
        $arg =~ /^PP$/      and $QuantityToBuild = shift @ARGV  and next;
        $arg =~ /^-all$/    and $BuildAllParents = 1            and next;
        $arg =~ /^-show$/   and $show = 1                       and next;
        $arg =~ /^-deliver$/and $deliver = 1                    and next;
        $arg =~ /^-from$/   and $BuildAllParents = 1
                            and $build_from = shift @ARGV       and next;
        $arg =~ /^-help$/   and &usage                          and exit(0);
        push (@dmake_args, $arg);
    };
    @ARGV = @dmake_args;
};

sub print_error {
    my $message = shift;
    print STDERR "\nERROR: $message\n";
};

sub usage {
    print STDERR "\nbuild\n";
    print STDERR "Syntax:   build [-help|-all|-from prj_name] \n";
    print STDERR "Example:  build -from sfx2\n";
    print STDERR "              - build all projects including current one from sfx2\n";
    print STDERR "Keys:     -all        - build all projects from very beginning till current one\n";
    print STDERR "      -from       - build all projects beginning from the specified till current one\n";
    print STDERR "      -show       - show what is gonna be built\n";
    print STDERR "      -deliver    - only deliver, no build (usable for \'-all\' and \'-from\' keys)\n";
    print STDERR "      -help       - print help info\n";
    print STDERR "Default:          - build current project\n";
    print STDERR "Keys that are not listed above would be passed to dmake\n";
};
