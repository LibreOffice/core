:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   $RCSfile: build.pl,v $
#
#   $Revision: 1.46 $
#
#   last change: $Author: vg $ $Date: 2002-02-25 17:36:53 $
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

$id_str = ' $Revision: 1.46 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

print "$script_name -- version: $script_rev\n";

if ($ENV{GUI} eq 'UNX') {
    use Cwd 'chdir';
};

#########################
#                       #
#   Globale Variablen   #
#                       #
#########################
$QuantityToBuild = 0;
$cmd_file = '';
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
%ParentDepsHash = (); # hash of dependencies of the current project
@UnresolvedParents = ();
@dmake_args = ();
%DeadParents = ();
$CurrentPrj = '';
$StandDir = &get_stand_dir();
$build_from = '';
$build_from_opt = '';
$build_since = '';
$dlv_switch = '';

&get_options;

$deliver_commando = $ENV{DELIVER};
$deliver_commando .= ' '. $dlv_switch;
$ENV{mk_tmp}++;
%prj_platform = ();
$check_error_string = '';
$dmake = '';
$echo = '';
$new_line = "\n";
#### main ####

&get_commands();
unlink ($cmd_file);
if ($cmd_file) {
    if (open (CMD_FILE, ">>$cmd_file")) {
        select CMD_FILE;
        $echo = 'echo ';
        $new_line = $echo."\"\"\n";
        print "\@$echo off\npushd\n" if ($ENV{GUI} ne 'UNX');
    } else {
        &print_error ("Cannot open file $cmd_file");
    };
} elsif ($show) {
    select STDERR;
};

print $new_line;

&BuildAll();
@TotenEltern = keys %DeadParents;
if ($#TotenEltern != -1) {
    my ($DeadPrj);
    print $new_line.$new_line;
    print $echo."WARNING! Project(s):\n";
    foreach $DeadPrj (@TotenEltern) {
        print $echo."$DeadPrj\n";
    };
    print $new_line;
    print $echo."not found and couldn't be built. Correct build.lsts.\n";
    print $new_line;
};
if (($ENV{GUI} ne 'UNX') && $cmd_file) {
    print "popd\n";
};
$ENV{mk_tmp} = '';
if ($cmd_file) {
    close CMD_FILE;
    print STDOUT "Script $cmd_file generated\n";
};
exit(0);

#########################
#                       #
#      Procedures       #
#                       #
#########################

#
# Get dependencies hash of the current and all parent projects
#
sub GetParentDeps {
    my ($ParentsString, @DepsArray, $Prj, $parent);
    my $prj_dir = shift;
    my $deps_hash = shift;
    $ParentsString = &GetParentsString($prj_dir);
    @DepsArray = &GetDependenciesArray($ParentsString);
    @UnresolvedParents = @DepsArray;
    $$deps_hash{$prj_dir} = [@DepsArray];
    while ($Prj = pop(@UnresolvedParents)) {
        my (@DepsArray);
        if (!($ParentsString = &GetParentsString($StandDir.$Prj))) {
            $DeadParents{$Prj}++;
            $$deps_hash{$Prj} = [];
            next;
        };
        @DepsArray = &GetDependenciesArray($ParentsString, $Prj);
        $$deps_hash{$Prj} = [@DepsArray];
        foreach $Parent (@DepsArray) {
            if (!defined($$deps_hash{$Parent})) {
                push (@UnresolvedParents, $Parent);
            };
        };
    };
    foreach $parent (keys %DeadParents) {
        delete $ParentDepsHash{$parent};
        &RemoveFromDependencies($parent, $deps_hash);
    };
};

#
# Build everything that should be built
#
sub BuildAll {
    if ($BuildAllParents) {
        my ($Prj, $PrjDir);
        &GetParentDeps( $CurrentPrj, \%ParentDepsHash);
        if ($build_from) {
            &remove_extra_prjs(\%ParentDepsHash);
        };
        while ($Prj = &PickPrjToBuild(\%ParentDepsHash)) {
            if ($build_from_opt) {
                if ($build_from_opt ne $Prj) {
                    &RemoveFromDependencies($Prj, \%ParentDepsHash);
                    next;
                } else {
                    $build_from_opt = '';
                };
            };
            if ($build_since) {
                if ($build_since ne $Prj) {
                    &RemoveFromDependencies($Prj, \%ParentDepsHash);
                } else {
                    &RemoveFromDependencies($Prj, \%ParentDepsHash);
                    $build_since = '';
                };
                next;
            };
            print $new_line;
            print $echo.    "=============\n";
            print $echo.    "Building project $Prj\n";
            print $echo.    "=============\n";
            $PrjDir = &CorrectPath($StandDir.$Prj);
            chdir $PrjDir;
            cwd();
            &BuildPrj($PrjDir) if (!$deliver);
            if ($cmd_file) {
                print "$deliver_commando\n";
            } else {
                system ("$deliver_commando") if (!$show);
            };
            print $check_error_string;
            &RemoveFromDependencies($Prj, \%ParentDepsHash);
        };
    } else {
        &BuildPrj('.');
    };
};

#
# Start build given project
#
sub MakeDir {
    my ($DirToBuild, $BuildDir, $error);
    $DirToBuild = shift;
    $BuildDir = &CorrectPath($StandDir . $PathHash{$DirToBuild});
    if ($cmd_file) {
        print "cd $BuildDir\n";
        print $check_error_string;
        print $echo.$BuildDir."\n";
        print "$dmake\n";
        print $check_error_string;
    } else {
        print "$BuildDir\n";
    };
    &RemoveFromDependencies($DirToBuild, \%LocalDepsHash);
    if (!$cmd_file && !$show) {
        &print_error("\n$BuildDir not found!!\n") if (!(chdir ($BuildDir)));
        cwd();
        $error = system ("$dmake");
        if ($error) {
            &print_error("Error $error occurred while making $BuildDir");
        };
    };
};


#
# Get string (list) of parent projects to build
#
sub GetParentsString {
    my ($PrjDir);
    $PrjDir = shift;
    $PrjDir = '.' if ($PrjDir eq $CurrentPrj);
    if (!open (PrjBuildFile, $PrjDir.'/prj/build.lst')) {
        return '';
    };
    while (<PrjBuildFile>) {
        if ($_ =~ /#/) {
            if ($`) {
                $_ = $`;
            } else {
                next;
            };
        };
        s/\r\n//;
        if ($_ =~ /\:+\s+/) {
            close PrjBuildFile;
            return $';
        };
    };
    close PrjBuildFile;
    return 'NULL';
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
                    &print_error ("There is no correct alias set in the line $line!");
                };
                &mark_platform($alias, $platform);
            } else {
                &print_error("Misspelling in line: \n$_");
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
    open (PrjBuildFile, 'prj/build.lst');
    &get_prj_platform;
    while (<PrjBuildFile>) {
        if ($_ =~ /#/) {
            if ($`) {
                $_ = $`;
            } else {
                next;
            };
        };
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
                $DeadDependencies{$DirAlias}++;
                next;
            };
            $PlatformHash{$DirAlias}++;
            $Dependencies = $';
            @Array = GetDependenciesArray($Dependencies);
            $LocalDepsHash{$DirAlias} = [@Array];
            $BuildQueue{$DirAlias}++;
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
    &BuildDependent();
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
# to system-independent
#
sub CorrectPath {
    $_ = shift;
    if (($ENV{GUI} ne 'UNX') && $cmd_file) {
        s/\//\\/g;
    } else {;
        s/\\/\//g;
    };
    return $_;
};


#
# Get platform-dependent commands
#
sub get_commands {
    my $arg = '';
    # Setting alias for dmake
    $dmake = 'dmake';
    while ($arg = pop(@dmake_args)) {
        $dmake .= ' '.$arg;
    };
    if ($cmd_file) {
        if ($ENV{GUI} eq 'UNX') {
            $check_error_string = "if \"\$?\" != \"0\" exit\n";
        } else {
            $check_error_string = "if \"\%?\" != \"0\" quit\n";
        };
    };
};


#
# Procedure prooves if current dir is a root dir of the drive
#
sub IsRootDir {
    my ($Dir);
    $Dir = shift;
    if (        (($ENV{GUI} eq 'UNX') ||
                 ($ENV{GUI} eq 'MACOSX')) &&
                ($Dir eq '/')) {
        return 1;
    } elsif (   (($ENV{GUI} eq 'WNT') ||
                 ($ENV{GUI} eq 'WIN') ||
                 ($ENV{GUI} eq 'OS2')) &&
                ($Dir =~ /\S:\/$/)) {
        return 1;
    } else {
        return 0;
    };
};


#
# Procedure retrieves list of projects to be built from build.lst
#
sub get_stand_dir {
    if (!(defined $ENV{GUI})) {
        $ENV{mk_tmp} = '';
        die "No environment set\n";
    };
    my ($StandDir);
    do {
        $StandDir = cwd();
        if (open(PrjBuildFile, 'prj/build.lst')) {
            $StandDir =~ /(\w+$)/;
            $StandDir = $`;
            $CurrentPrj = $1;
            close(PrjBuildFile);
            return $StandDir;
        } elsif (&IsRootDir($StandDir)) {
            $ENV{mk_tmp} = '';
            &print_error ('Found no project to build');
        };
    }
    while (chdir '..');
};

#
# Build the entire project according to queue of dependencies
#
sub BuildDependent {
    my ($Dir);
    while ($Dir = &PickPrjToBuild(\%LocalDepsHash)) {
        &MakeDir($Dir);
        $Dir = '';
    };
};

#
# Removes projects which it is not necessary to build
#
sub remove_extra_prjs {
    my ($prj, $deps_hash);
    $deps_hash = shift;
    my %from_deps_hash = ();   # hash of dependencies of the -from project
    &GetParentDeps( $StandDir.$build_from,\%from_deps_hash);
    foreach $prj (keys %from_deps_hash) {
        delete $$deps_hash{$prj};
        &RemoveFromDependencies($prj, $deps_hash);
    };
};

#
# Picks project which can be build now from hash and deletes it from hash
#
sub PickPrjToBuild {
    my ($Prj, $DepsHash);
    $DepsHash = shift;
    $Prj = &FindIndepPrj($DepsHash);
    delete $$DepsHash{$Prj};
    return $Prj;
};


#
# Make a decision if the project should be built on this platform
#
sub CheckPlatform {
    my ($Platform);
    $Platform = shift;
    if ($Platform eq 'all') {
        return 1;
    } elsif (($ENV{GUI} eq 'WNT') &&
                            (($Platform eq 'w') || ($Platform eq 'n'))) {
        return 1;
    } elsif (($ENV{GUI} eq 'WIN') && ($Platform eq 'w')) {
        return 1;
    } elsif (($ENV{GUI} eq 'UNX') && ($Platform eq 'u')) {
        return 1;
    } elsif (($ENV{GUI} eq 'MACOSX') && ($Platform eq 'm')) {
        return 1;
    } elsif (($ENV{GUI} eq 'OS2') && ($Platform eq 'p')) {
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
        foreach $i (0 .. $#{$$Dependencies{$Prj}}) {
            if (${$$Dependencies{$Prj}}[$i] eq $ExclPrj) {
                splice (@{$$Dependencies{$Prj}}, $i, 1);
                $i = 0;
                last;
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
        foreach $Prj (@Prjs) {
            if (&IsHashNative($Prj)) {
                next;
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
        return '' if ($build_from);
        print STDERR "\nError: projects";
        foreach $Prj (keys %$Dependencies) {
            if (IsHashNative($Prj)) {
                next;
            };
            $i = 0;
            print STDERR "\n$Prj depends on:";
            foreach $i (0 .. $#{$$Dependencies{$Prj}}) {
                print STDERR (' ', ${$$Dependencies{$Prj}}[$i]);
            };
        };
        &print_error ("\nhave dead or circular dependencies\n");
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
    my ($DepString, @Dependencies, $ParentPrj, $prj, $string);
    @Dependencies = ();
    $DepString = shift;
    $string = $DepString;
    $prj = shift;
    while (!($DepString =~ /^NULL/)) {
        if (!$DepString) {
            &print_error("Project $prj has wrong written dependencies string:\n $string");
        };
        $DepString =~ /(\S+)\s*/;
        $ParentPrj = $1;
        $DepString = $';
        if ($ParentPrj =~ /\.(\w+)$/) {
            $ParentPrj = $`;
            if (($prj_platform{$ParentPrj} ne $1) &&
                ($prj_platform{$ParentPrj} ne 'all')) {
                &print_error ("$ParentPrj\.$1 is a wrong dependency identifier!\nCheck if it is platform dependent");
            };
            if (&CheckPlatform($1)) {
                $AliveDependencies{$ParentPrj}++;
            }
            push(@Dependencies, $ParentPrj);
        } else {
            if ((exists($prj_platform{$ParentPrj})) &&
                ($prj_platform{$ParentPrj} ne 'all') ) {
                &print_error("$ParentPrj is a wrong used dependency identifier!\nCheck if it is platform dependent");
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

sub print_error {
    my $message = shift;
    print STDERR "\nERROR: $message\n";
    $ENV{mk_tmp} = '';
    close CMD_FILE if ($cmd_file);
    unlink ($cmd_file);
    exit(1);
};

sub usage {
    print STDERR "\nbuild\n";
    print STDERR "Syntax:   build [--help|-all|-from|-from_opt|since prj_name|-file file_name|-dlv[_switch] dlvswitch] \n";
    print STDERR "Example:  build -from sfx2\n";
    print STDERR "              - build all projects including current one from sfx2\n";
    print STDERR "Example:  build -from_opt sfx2\n";
    print STDERR "              - the same as -from, but skip all projects that could have been built (no secure way, use ONLY when -all or -from is already been run and there no external dependensies\' changes occurred)\n";
    print STDERR "Keys:     -all        - build all projects from very beginning till current one\n";
    print STDERR "      -from       - build all projects beginning from the specified till current one\n";
    print STDERR "      -from_opt   - build all projects beginning from the specified till current one (optimized version)\n";
    print STDERR "      -since      - build all projects beginning from the specified till current one (optimized version, skips specified project)\n";
    print STDERR "      -show       - show what is going to be built\n";
    print STDERR "      -file       - generate command file file_name\n";
    print STDERR "      -deliver    - only deliver, no build (usable for \'-all\' and \'-from\' keys)\n";
    print STDERR "      -dlv_switch - use deliver with the switch specified\n";
    print STDERR "      -help       - print help info\n";
    print STDERR "Default:          - build current project\n";
    print STDERR "Keys that are not listed above would be passed to dmake\n";
};

#
# Get all options passed
#
sub get_options {
    my $arg;
    #&usage() && exit(0) if ($#ARGV == -1);
    #$QuantityToBuild
    while ($arg = shift @ARGV) {
        $arg =~ /^PP$/          and $QuantityToBuild = shift @ARGV  and next;
        $arg =~ /^-all$/        and $BuildAllParents = 1            and next;
        $arg =~ /^-show$/       and $show = 1                       and next;
        $arg =~ /^-deliver$/    and $deliver = 1                    and next;
        $arg =~ /^-dlv_switch$/ and $dlv_switch = &get_switch_options   and next;
        $arg =~ /^-dlv$/        and $dlv_switch = &get_switch_options   and next;
        $arg =~ /^-file$/       and $cmd_file = shift @ARGV         and next;
        $arg =~ /^-from$/       and $BuildAllParents = 1
                                and $build_from = shift @ARGV       and next;
        $arg =~ /^-from_opt$/   and $BuildAllParents = 1
                                and $build_from_opt = shift @ARGV   and next;

        $arg =~ /^-since$/      and $BuildAllParents = 1
                                and $build_since = shift @ARGV      and next;
        $arg =~ /^--help$/      and &usage                          and exit(0);
        push (@dmake_args, $arg);
    };
    if ($build_from && $build_from_opt) {
        &print_error('Switches -from an -from_opt collision');
    };

    if ($build_from && $build_since) {
        &print_error('Switches -from an -since collision');
    };
    @ARGV = @dmake_args;
    $cmd_file = '' if ($show);
};

#
# get all options without '-'
#
sub get_switch_options {
    my $string = '';
    my $option = '';
    while ($option = shift @ARGV) {
        if (!($option =~ /^-/)) {
            $string .= $option;
            $string .= ' ';
        } else {
            unshift(@ARGV, $option);
            last;
        };
    };
    $string =~ s/\s$//;
    return $string;
};
