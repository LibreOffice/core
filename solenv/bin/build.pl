#!/usr/solar/bin/perl
#
#
#

use Cwd;

#########################
#                       #
#   Globale Variablen   #
#                       #
#########################
$QuantityToBuild = 0;
%DependenciesHash = ();
%DepsArchive = ();
%BuildQueue = ();
%PathHash = ();
%PlatformHash = ();
%DeadDependencies = ();
%ParentDepsHash = ();
@UnresolvedParents = ();
%DeadParents = ();
$CurrentPrj = "";
$StandDir = GetStandDir();
$QuantityToBuild = GetQuantityToBuild();
$BuildAllParents = HowToBuild();

$dmake = GetDmakeCommando();
BuildAll();
@TotenEltern = keys %DeadParents;
if (($BuildAllParents) && ($#TotenEltern != -1)) {
    my ($DeadPrj);
    print "\nWARNING! Project(s):\n\n";
    foreach $DeadPrj (@TotenEltern) {
        print "$DeadPrj\n";
    };
    print "\nnot found and couldn't be built. Correct the build.lst's.\n";
};
#GetPrjDeps();
#BuildDependent();

#########################
#                       #
#      Procedures       #
#                       #
#########################

#
# Remove project to build ahead from dependencies and make an array
# of all from given project dependent projects
#
sub BuryDeadParent {
    #my ($ExclPrj, $i, $Prj, $DependenciesHash);
    #$ExclPrj = $_[0];
    #$DependenciesHash = $_[1];
    #foreach $Prj (keys $DependenciesHash) {
    #   PrjDepsLoop:
    #   foreach $i (0 .. $#{$DependenciesHash{$Prj}}) {
    #       #print $Prj, " $i ", ${$DependenciesHash{$Prj}}[$i], "\n";
    #       if (${$DependenciesHash{$Prj}}[$i] eq $ExclPrj) {
    #           splice (@{$DependenciesHash{$Prj}}, $i, 1);
    #           $i = 0;
    #           last PrjDepsLoop;
    #       };
    #   };
    #};
};


#
# Get dependencies of current and all parent projects
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
        BuryDeadParent($Parent, %ParentDepsHash);
    };
};


#
# Build everything that should be built
#
sub BuildAll {
    if ($BuildAllParents) {
        GetParentDeps();
    };
    GetPrjDeps(".");
    BuildDependent();
};


#
# Start build given project
#
sub MakeDir {
    my ($DirToBuild, $BuildDir, $error, );
    $DirToBuild = $_[0];
    $BuildDir = CorrectPath($StandDir.$PathHash{$DirToBuild});
    if ($ENV{GUI} eq "UNX") {
        use Cwd 'chdir';
    };
    chdir ($BuildDir);
    print $BuildDir, "\n";
    cwd();
    $error = system ("$dmake");
    if (!$error) {
        RemoveFromDependencies($DirToBuild);
    } else {
        print "Error $error occurred while making $BuildDir\n";
        exit();
    };
};


#
# Make dependeCheck if project should be built when all parent projects are built
#
sub GetParentsString {
    my ($ParentPrjs, @Arr, $PrjDir);
    $PrjDir = $_[0];
    if (!open (PrjBuildFile, $PrjDir."/prj/build.lst")) {
        return "";
    };
    @Arr = <PrjBuildFile>;
    $Arr[0] =~ /(\:)([\t | \s]+)/;
    $ParentPrjs = $';
    close PrjBuildFile;
    return $';
};

#
# Check if project should be built when all parent projects are built
#
sub HowToBuild {
    my ($i);
    foreach $i (0 .. $#ARGV) {
        if ($ARGV[$i] =~ /^-all/) {
            splice(@ARGV, $i, 1);
            return 1;
        };
    };
    return 0;
}

#
# Getting hashes of all internal dependencies and additional
# infos for given project
#
sub GetPrjDeps {
    my ($dummy);
    open (PrjBuildFile, "prj/build.lst");
    BuildLstLoop:
    while (<PrjBuildFile>) {
        s/\r\n//;
        if ($_ =~ /nmake/) {
            my ($Platform, $Dependencies, $Dir, $DirAlias, @Array);
            $Dependencies = $';
            $dummy = $`;
            $dummy =~ /(\w+)\t([\w | \\ | \.]+)/;
            $Dir = $2;
            $Dependencies =~ /(\t\-\t)(\w+)/; #(\t)(\S+)(\s)/;
            $Platform = $2;
            $Dependencies = $';
            while ($Dependencies =~ /,(\w+)/) {
                $Dependencies = $';
            };
            $Dependencies =~ /(\s)(\S+)(\s)/;
            $DirAlias = $2;
            if (!CheckPlatform($Platform)) {
                $DeadDependencies{$DirAlias} = 1;
                next BuildLstLoop;
            };
            $PlatformHash{$DirAlias} = 1;
            $Dependencies = $';
            @Array = GetDependenciesArray($Dependencies);
            $DependenciesHash{$DirAlias} = [@Array];
            $BuildQueue{$DirAlias} = 1;
            $PathHash{$DirAlias} = $Dir;
        };
    };
    %DepsArchive = %DependenciesHash;
    foreach $Dir (keys %DeadDependencies) {
        if (!IsHashNative($Dir)) {
            RemoveFromDependencies($Dir);
            delete $DeadDependencies{$Dir};
        };
    };
    BuildDependent();
};


#
# Convert path from abstract (with '\' and/or '/' delimiters)
# to system-dependent
#
sub CorrectPath {
    $_ = $_[0];
    if ($ENV{GUI} eq "UNX") {
        s/\\/\//g;
    } elsif (   ($ENV{GUI} eq "WNT") ||
                ($ENV{GUI} eq "WIN") ||
                ($ENV{GUI} eq "MAC") ||
                ($ENV{GUI} eq "OS2")) {
        s/\//\\/g;
    } else {
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

    $dmake .= " ".$ENV{PROFULLSWITCH};
    while ($arg = pop(@ARGV)) {
        $dmake .= " "."$arg";
    };
    return $dmake;
};


#
# Procedure returns quantity of folders to be built syncronously
#
sub GetQuantityToBuild {
    my ($i);
    foreach $i (0 .. $#ARGV) {
        if ($ARGV[$i] =~ /^-PP/) {
            splice(@ARGV, $i, 1);
            return $';
        };
    };
};


#
# Procedure retrieves list of projects to be built from build.lst
#
sub GetStandDir {
    my ($StandDir);
DirLoop:
    do {
        if (open(PrjBuildFile, "prj/build.lst")) {
            $StandDir = cwd();
            $StandDir =~ /(\w+$)/;
            $StandDir = $`;
            $CurrentPrj = $1;
            close(PrjBuildFile);
            return $StandDir;
        };
    }
    while (chdir(".."));
    die "Found no project to build\n";
};


#
# Build the entire project according to queue of dependencies
#
sub BuildDependent {
    my ($Prj);
    while ($Prj = PickPrjToBuild()) {
    MakeDir($Prj);
    $Prj = "";
    };
};


#
# Picks project which can be build now from hash and deletes it from hash
#
sub PickPrjToBuild {
    my ($Prj);
    $Prj = FindUndepPrj();
    delete $DependenciesHash{$Prj};
    return $Prj;
};


#
# Make a decision if the project should be built on this platform
#
sub CheckPlatform {
    my ($Platform);
    $Platform = $_[0];
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
# Find undependent project
#
sub FindUndepPrj {
    my ($Prj, @Prjs, @PrjDeps, $i);
    @Prjs = keys %DependenciesHash;
    if ($#Prjs != -1) {
        PrjLoop:
        foreach $Prj (@Prjs) {
            if (IsHashNative($Prj)) {
                next PrjLoop;
            };
            @PrjDeps = @{$DependenciesHash{$Prj}};
            if ($#PrjDeps == -1) {
                return $Prj;
            };
        };
        # If there are only dependent projects in hash - generate error
        print "\nError: projects";
        DeadPrjLoop:
        foreach $Prj (keys %DependenciesHash) {
            if (IsHashNative($Prj)) {
                next DeadPrjLoop;
            };
            $i = 0;
            print "\n", $Prj, " depends on:";
            foreach $i (0 .. $#{$DependenciesHash{$Prj}}) {
                print " ", ${$DependenciesHash{$Prj}}[$i];
            };
        };
        print "\nhave dead or circular dependencies\n\n";
        exit ();
    };
};



#
# Check if given entry is HASH-native, that is not a user-defined data
#
sub IsHashNative {
    my ($Prj);
    $Prj = $_[0];
    if ($Prj =~ /^HASH\(0x[\d | a | b | c | d | e | f]{6,}\)/) {
        return 1;
    } else {
        return 0;
    };
};


#
# Remove project to build ahead from dependencies and make an array
# of all from given project dependent projects
#
sub RemoveFromDependencies {
    my ($ExclPrj, $i, $Prj);
    $ExclPrj = $_[0];
    foreach $Prj (keys %DependenciesHash) {
        PrjDepsLoop:
        foreach $i (0 .. $#{$DependenciesHash{$Prj}}) {
            #print $Prj, " $i ", ${$DependenciesHash{$Prj}}[$i], "\n";
            if (${$DependenciesHash{$Prj}}[$i] eq $ExclPrj) {
                splice (@{$DependenciesHash{$Prj}}, $i, 1);
                $i = 0;
                last PrjDepsLoop;
            };
        };
    };
};


#
# Getting array of dependencies from the string given
#
sub GetDependenciesArray {
    my ($DepString, @Dependencies, $ParentPrj);
    @Dependencies = ();
    $DepString = $_[0];
    while (!($DepString =~ /^NULL/)) {
        $DepString =~ /(\S+)(\s)/;
        $ParentPrj = $1;
        $DepString = $';
        if ($ParentPrj =~ /(\S+)(\.)(\w)/) {
            $ParentPrj = $1;
            if (CheckPlatform($3)) {
                push(@Dependencies, $ParentPrj);
            };
        } else {
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
    $Path = $_[0];
    opendir(CurrentDirList, $Path);
    @DirectoryList = readdir(CurrentDirList);
    closedir(CurrentDirList);
    return @DirectoryList;
};

