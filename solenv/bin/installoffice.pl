:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;

#*************************************************************************
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: installoffice.pl,v $
#
# $Revision: 1.2.24.1 $
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
#***********************************************************************/

use strict;
use File::Find;
use File::Path;
my $script = $0;

( our $script_name = $script ) =~ s/^.*\b(\w+)\.pl$/$1/;
( our $script_path = $script ) =~ s/$script_name.*//;

# Prototypes
sub installOffice();

our $debug = 0;           # run without executing commands

our $is_command_infos = 1;   # print command details before exec
our $show_NoMessage = 0;
our $show_Message = 1;

# special handling for cwschackapi
our $cwsCheckApi = 0;

# remove existant office installation
our $cleanup = 0;

# should the office be startable without user interaction
our $autorun = 0;

# should impress open documents without autopilot
our $autoimpress = 0;

# force openOffice.org installation if StarOffice is available
our $is_ooo = 0;

parseArgs();

our $is_admin_installation = 1;

our $gui = $ENV{GUI};
our $temp_path = $ENV{TEMP};
if (!defined($temp_path)) {
   $temp_path = $ENV{TMP};
}

our $vcsid = $ENV{VCSID};
our $sversion_saved = 0;
our $FileURLPrefix = "file:///";
our $userinstalldir = "UserInstallation";
our $cygwin = "cygwin";
our $prefered_lang = "en-US";
our $global_instset_mask = "";
#$smoketest_install = $ENV{SMOKETESTINSTALLSET};

if (!defined($gui)) {
    print "The workstamp is missing. Please use setsolar\n";
    exit(1);
}

our $OfficeDestinationPath;
our $is_do_deinstall;
our $CygwinLineends;
our $WinLineends;
our $PS ;
our $NewPathSeparator ;
our $shell ;
our $shellSetEnvVar;
our $shellCommandSep;
our $cdCommand;
our $PERL ;
our $REMOVE_DIR ;
our $REMOVE_FILE ;
our $LIST_DIR ;
our $COPY_FILE ;
our $COPY_DIR ;
our $MK_DIR ;
our $RENAME_FILE ;
our $nul ;
our $RESPFILE;
our $SVERSION_INI ;
our $SOFFICEBIN ;
our $UNOPKGBIN;
our $bootstrapini ;
our $bootstrapiniTemp ;
our $packpackage ;
our $user;

if ($gui eq "WNT") {
    $gui = $cygwin;
}

if ($gui eq "WNT") {
    $user = $ENV{USERNAME};
    $PS = '\\';
    $NewPathSeparator = ';';
    $shell = "$ENV{COMSPEC} -c ";
    $shellSetEnvVar = "set ";
    $shellCommandSep = "& ";
    $cdCommand="cd /d ";
    $PERL = "$shell $ENV{PERL}";
    $REMOVE_DIR = "$shell del /qsxyz";
    $REMOVE_FILE = "$shell del /q";
    $LIST_DIR = "$shell ls";
    $COPY_FILE = "$shell copy";
    $COPY_DIR = "$shell copy /s";
    $MK_DIR = "md";
    $RENAME_FILE = "ren";
    $nul = '> NUL';
    $RESPFILE="response_fat_wnt";
    $SVERSION_INI = $ENV{USERPROFILE} . $PS . "Anwendungsdaten" . $PS . "sversion.ini";
    $SOFFICEBIN = "soffice.exe";
    $bootstrapini = "bootstrap.ini";
    $UNOPKGBIN="unopkg.exe";
    $bootstrapiniTemp = $bootstrapini . "_";
    $packpackage = "msi";
    if (!defined($temp_path)) {
        print "temp value is missing. Please set temp-variable\n";
        exit(1);
    }
}
elsif ($gui eq "UNX") {
    $user = $ENV{USER};
    $is_do_deinstall = 0;
    $PS = '/';
    $shell = "/bin/sh -c ";
    $shellSetEnvVar = "export ";
    $shellCommandSep = ";";
    $cdCommand="cd ";
    $NewPathSeparator = ':';
    $shell = "";
    $PERL = "$ENV{PERL}";
    $REMOVE_DIR = "rm -rf";
    $REMOVE_FILE = "rm -f";
    $LIST_DIR = "ls";
    $COPY_FILE = "cp -f";
    $COPY_DIR = "cp -rf";
    $MK_DIR = "mkdir";
    $RENAME_FILE = "mv";
    $nul = '> /dev/null';
    $RESPFILE="response_fat_unx";
    $SVERSION_INI = $ENV{HOME} . $PS . ".sversionrc";
    $SOFFICEBIN = "soffice";
    $bootstrapini = "bootstraprc";
    $UNOPKGBIN="unopkg";
    $bootstrapiniTemp = $bootstrapini . "_";
    $packpackage = $ENV{PKGFORMAT};
    if (!defined($temp_path)) {
        $temp_path="/tmp";
        if (! -e $temp_path){
            print "temp value is missing. Please set temp-variable\n";
            exit(1);
        }
    }
}
elsif ($gui eq $cygwin) {

    $PS = '/';
    $NewPathSeparator = ':';
    $shell = "/bin/sh -c ";
    $shellSetEnvVar = "export ";
    $shellCommandSep = ";";
    $cdCommand = "cd ";
    $PERL = "$ENV{PERL}";
    $REMOVE_DIR = "rm -rf";
    $REMOVE_FILE = "rm -f";
    $LIST_DIR = "ls";
    $COPY_FILE = "cp -f";
    $COPY_DIR = "cp -rf";
    $MK_DIR = "mkdir";
    $RENAME_FILE = "mv";
    $nul = '> /dev/null';
    $RESPFILE="response_fat_wnt";
    $SVERSION_INI = $ENV{USERPROFILE} . $PS . "Anwendungsdaten" . $PS . "sversion.ini";
    $SOFFICEBIN = "soffice";
    $bootstrapini = "bootstrap.ini";
    $bootstrapiniTemp = $bootstrapini . "_";
    $CygwinLineends = $/;
    $WinLineends = "\r\n";
    &SetWinLineends();
    $packpackage = "msi";
    if (!defined($temp_path)) {
        $temp_path="/tmp";
        if (! -e $temp_path){
            print "temp value is missing. Please set temp-variable\n";
            exit(1);
        }
    }
}
else {
    print_error ("not supported system\n",1);
}

my %PRODUCT1 = (Name => "StarOffice", instset => "instset_native");
my %PRODUCT2 = (Name => "OpenOffice", instset => "instsetoo_native");
our @PRODUCT = (\%PRODUCT1, \%PRODUCT2);

our $SHIP = defined $ENV{SHIPDRIVE} ? $ENV{SHIPDRIVE} . $PS : "shipdrive_not_set";

if (defined($ENV{CWS_WORK_STAMP})){
    print " found CWS\n";
}
elsif (isLocalEnv()){
    print " local environment\n";
}
elsif (defined $ENV{SHIPDRIVE}) {
    print " master version\n";
}
elsif (defined $ENV{SOLARSRC}) {
    print " OpenOffice master version\n";
}
else {
    print "Could not determine environment. Exit\n";
    exit 1
}


our @error_messages = ( '',
            'lock flag for pkgadd still exist. Installation not possible!',
            'Error during installation!',
            'Error: patching configuration failed!',
            'Error: starting office failed or office crashed!',
            'Error during testing',
            'can not copy extension',
            'Error in setup log',
            'installationsset is not complete',
            'can not copy all basic scripts',
            'can not patch bottstrapini',
            'msiexec failed. Maybe you have got an installed version',
            'deinstallation is incomplete'
);


my $success = 0;
$success = installOffice();

if ($success != 0){
    exit(1);
}
exit(0);

sub installOffice(){

    # <DISABLED>
    # my ($officeVersion, $instsetPath)= getInstset();
    # print "OFFICEVERSION:$officeVersion INSTSETPATH:$instsetPath\n" if $debug;
    # my $sourcePath=$instsetPath.$PS.$officeVersion;
    # </DISABLED>
    #
    # currently this installer does not install an office, it do pack a runnable office.
    # Therefore we do not need to search for an instSet
    my $sourcePath="dummy";
    my $officeVersion="dummy";

    my $destinationPath = getDestinationPath($officeVersion);
    print "destinationPath: $destinationPath\n" if $debug;

    removeOffice($destinationPath);
    mkdirs($destinationPath);

    my $installCommand = getInstallCommand($sourcePath, $destinationPath, $officeVersion);
    print "installCommand: $installCommand\n" if $debug;

    my $success=0;
    $success = system($installCommand);
    print "installoofice.pl::installoffice::success $success\n" if ($debug);

    if ($autorun) {
        makeAutoRun($destinationPath);
    }

    if ($autoimpress) {
        makeAutoImpress($destinationPath);
    }

    return $success;
}

sub makeAutoImpress(){
    my $destinationPath = shift;

    if (patchXCU ($destinationPath, $script_path.$script_name."_impress.oxt") != 0) {
        print_error("could not register ".$script_path.$script_name."_impress.oxt", "1");
    }
    return 0;
}


sub makeAutoRun(){

    my $destinationPath = shift;

    patchBootstraprc($destinationPath);

    if (patchXCU ($destinationPath, $script_path.$script_name.".oxt") != 0) {
        print_error("could not register ".$script_path.$script_name.".oxt", "1");
    }
    return 0;
}


sub patchBootstraprc(){
    my $destinationPath = shift;
    my $bootstraprc="";

    find sub { $bootstraprc=$File::Find::name if -e _ && /$bootstrapini$/ }, $destinationPath;

    print_error("could not find $bootstrapini", "1") if ( ! -e $bootstraprc );

    open(BSRC, "<$bootstraprc") or errorFromOpen(" for reading " .$bootstraprc);
    my @content = <BSRC>;
    close(BSRC);
    my @newContent;
    foreach(@content) {
        if ( /UserInstallation=./ ) {
            push(@newContent, "UserInstallation=\$ORIGIN/../UserInstallation\n");
        }
        else {
            push(@newContent,$_);
        }
    }

    chmod(0755,$bootstraprc);

    open OUTFILE, ">$bootstraprc" or return errorFromOpen (" for writing " . $bootstraprc);
    print OUTFILE @newContent;
    close OUTFILE;

}

sub patchXCU(){
    my $destinationPath = shift;
    my $oxt = shift;
    my $unopkg="";

    find sub { $unopkg=$File::Find::name if -e _ && /$UNOPKGBIN$/ }, $destinationPath;

    print_error("could not find $UNOPKGBIN", "1") if ( !-e $unopkg );
    print "unopkg: '$unopkg'\n" if $debug;

    if ($gui eq "WNT") {
        $unopkg = "\"$unopkg\"";
    }

    my $unopkgCommand = "$unopkg add $oxt";
    print "register oxt file ...\n" if $debug;
    print "call $unopkgCommand\n" if $debug;
    my $success=0;
    $success = system($unopkgCommand);

    return $success;
}

sub writeFile(){
    my $fileName = shift;
    my @content = @_;

    open OUTFILE, ">$fileName" or return errorFromOpen (" for writing " . $fileName);

    my $lf;
    if ($packpackage eq "msi"){
        $lf = "\r\n";
    } else {
        $lf = "\n";
    }

    my $line;
    foreach $line(@content) {
        print "LINE: $line\n" if $debug;
        print OUTFILE $line.$lf;
    }
    close(OUTFILE);
    return 0;
}

sub mkdirs(){
    my $directory = shift;
    my $splitter=$PS;
    if ( $PS eq "\\" ){
       $splitter="\\\\";
    }
    my @aFolder=split($splitter,$directory);

    my $dir;
    my $folder;
    foreach $folder(@aFolder){
        if ( (! defined($dir)) && ($PS eq "\\")){
            $dir=$folder;
        } else {
            $dir=$dir.$PS.$folder;
        }
        if (! -e $dir ){
            print "try to create $dir\n" if $debug;
            mkdir($dir);
        }
    }
}

sub removeOffice(){

    my $destPath=shift;

    if ($cleanup){
        print "remove old office installation...\n";

        print "remove $destPath...\n" if $debug;
        rmtree($destPath);

    }
}

sub getInstallCommand() {
    my $command;
    $command = getDmakeInstalledCommand(@_);
    # if ($packpackage eq "msi"){
    #     $command = getWindowsInallCommand(@_);
    # } else {
    #     $command = getUnixInstallCommand(@_);
    # }
    return $command;
}

sub getDmakeInstalledCommand(){
    my ($sourcePath, $destPath, $officeVersion) = @_;
    my $RootDir=$ENV{SRC_ROOT};

    my $ProductName;
    my $instset;

    # check if instset_native could be found
    foreach my $pointer (@PRODUCT) {
        $instset = $pointer->{'instset'};
        $ProductName = $pointer->{'Name'};
        my $instsetDir = "$RootDir$PS$instset";
        print "instsetDir: $instsetDir \n" if $debug;

        last if ($is_ooo && $ProductName eq "OpenOffice");
        last if ( ! $is_ooo && -e "$instsetDir");
    }
    print "Product: $ProductName\n" if $debug;;

    my $dmakeTarget=lc($ProductName)."_en-US";

    my $utilFolder = getInstsetUtilFolder($instset, $destPath, $RootDir);
    my $dmakeCommand = " $cdCommand \"$utilFolder\" $shellCommandSep dmake $dmakeTarget PKGFORMAT=installed -vt";
    my $envset;
    if ($gui eq "WNT") {
        $envset="set LOCALINSTALLDIR=$destPath & set LOCALUNPACKDIR=$destPath &";
    }else {
        $envset="LOCALINSTALLDIR=$destPath ; LOCALUNPACKDIR=$destPath ; export LOCALINSTALLDIR ; export LOCALUNPACKDIR ; ";
    }
    #my $command=$envset.$dmakeCommand;
    my $command=$dmakeCommand;
    return $command;
}

sub getInstsetUtilFolder(){
    my $instset = shift;
    my $destPath = shift;
    my $RootDir = shift;

    my $instsetFolder = $RootDir.$PS.$instset;
    my $utilFolder="";

    if( open(DATEI, ">$instsetFolder".$PS."touch"))
    {
        close(DATEI);
        unlink($instsetFolder.$PS."touch");
        print "$instsetFolder is writable \n" if $debug;
        $utilFolder = $RootDir.$PS.$instset.$PS."util";
    }
    else
    {
        print "$instsetFolder is NOT writable \n" if $debug;
        print "copy $instset to $destPath$PS..\n" if $debug;
        my $prjPath=$destPath.$PS."..";

        my $command = "$ENV{COPYPRJ} -x $instset $prjPath";
        print $command if $debug;
        system($command);
        $utilFolder=$prjPath.$PS.$ENV{WORK_STAMP}.$PS.$instset.$PS."util";
    }
    print "utilFolder: $utilFolder\n" if $debug;
    return $utilFolder;
}

# sub getInstsetFomInstsetNative(){
#     my (@DirArray, $TestDir1, $TestDir2);
#     my $instset;
#     my $ProductName;
#     my $instDir="";
#     my $lang;
#     my $RootDir=$ENV{SRC_ROOT};
#     my $StandDir = $ENV{SOLARSRC} . $PS;
#     #$RootDir=~s/\w+$//;

#     foreach my $pointer (@PRODUCT) {
#         $instset = $pointer->{'instset'};
#         $ProductName = $pointer->{'Name'};
#         @DirArray=();
#         $TestDir1 = "$RootDir$PS$instset$PS$ENV{INPATH}$PS$ProductName$PS$packpackage$PS" . "install$PS";
#         $TestDir2 = "$StandDir$instset$PS$ENV{INPATH}$PS$ProductName$PS$packpackage$PS" . "install$PS";
#         print "TestDir1: $TestDir1 \n";
#         print "TestDir2: $TestDir2 \n";
#         if (-e "$TestDir1") {
#             $instDir= $TestDir1;
#         }
#         elsif (-e "$TestDir2") {
#             $instDir="$TestDir2";
#         }
#         if ($instDir eq "") {
#             next;
#         }

#         getSubDirs ("$instDir", \@DirArray);
#         $lang = findSubDir (\@DirArray);
#         print "Lang-Sel: $lang\n" if $is_command_infos;
# ;
#         if (($instDir ne "") and  (-e $instDir)) {
#             return ($lang, $instDir, $ProductName, $instset);
#         }
#     }
#     print_error ("no installationset found\n",2);

# }

sub getWindowsInallCommand(){
    my ($sourcePath, $destPath) = @_;
    my $mask = "\\.msi\$";
    #my $DirArray;
    my @DirArray = ();
    getSubFiles ("$sourcePath", \@DirArray, $mask);
    if ($#DirArray == -1) {
        print_error ("Installationset in $sourcePath is incomplete", 2);
    }
    if ($#DirArray >= 1) {
        print_error ("Installationset in $sourcePath hat too many msi-files", 2);
        print "found the following msi-files: @DirArray\n" if $debug;
    }
    my $command = "msiexec.exe /a $sourcePath$PS$DirArray[0] -qn ALLUSERS=2 INSTALLLOCATION=$destPath";

    return $command;
}

sub getUnixInstallCommand() {
    my ($sourcePath, $destPath, $officeVersion) = @_;

    my $userland="unknown";

    if (defined($ENV{CWS_WORK_STAMP})){
        print " found CWS\n";
        $userland=$ENV{SOLARVERSION}.$PS.$ENV{INPATH}.$PS."bin".$ENV{UPDMINOREXT}.$PS."userscripts".$PS."install";
    }
    elsif (isLocalEnv()){
        print " local environment\n";
    }
    elsif (defined $ENV{SHIPDRIVE}) {
        print " master version\n";
        $userland=$ENV{SHIPDRIVE}.$PS.$ENV{INPATH}.$PS."UserScripts".$PS.$ENV{PKGFORMAT}.$PS.$officeVersion.$PS."install";
    }
    elsif (defined $ENV{SOLARSRC}) {
        print " OpenOffice master version\n";
        print " command to install Office not implementet yet\n";
        exit(1);
    }
    else {
        print "Could not determine environment. Exit\n";
        exit(1);
    }
    return "$userland $sourcePath $destPath";
;

}

sub getDestinationPath {
    # if (defined $ENV{SOLARROOT}){
    #     # seems to be a Sun environment
    #     if (! -e $OfficeDestinationPath){

    #     }
    # }
    my $officeVersion = shift;
    my $officename;

    if (defined($ENV{LOCALINSTALLDIR})){
        $OfficeDestinationPath=$ENV{LOCALINSTALLDIR};
        if(index($OfficeDestinationPath," ") >= 0) {
            my $msg="You environemt variable '\$LOCALINSTALLDIR=$OfficeDestinationPath' contains white spaces.";
            $msg = $msg." This is not allowed!";
            print_error($msg, "1");
        }
    }
    if (!defined($OfficeDestinationPath)){
        $officename = $officeVersion;
        if (defined($ENV{CWS_WORK_STAMP})){
            $officename=$ENV{CWS_WORK_STAMP};
        }
        $OfficeDestinationPath=$temp_path.$PS.$user.$PS."office".$PS.$officename;
    }

    return $OfficeDestinationPath;
}

# sub getInstset {

#     my ($INSTSET, $NEWINSTSET, $ProductName);

#     if (defined ($ENV{EPM}) && ($ENV{EPM} eq 'NO') && ($gui eq "UNX")) { # we do the install ourselves ...
#         return ();
#     }

#     print "get Instset\n" if $debug;
#     $NEWINSTSET = "";

#     if (!isLocalEnv() and !defined($ENV{CWS_WORK_STAMP}) and (-e $SHIP) and ($gui ne $cygwin)) {
#         ($NEWINSTSET, $INSTSET) = getSetFromServer();
#     }
#     else {
#         ($NEWINSTSET, $INSTSET, $ProductName) = getInstsetFomInstsetNative();
#     }
#     return ($NEWINSTSET, $INSTSET);
# }

# sub getSetFromServer {
#     my ($DirName, $SetupFullPath);
#     my $workspace = $ENV{WORK_STAMP};
#     my $platform  = $ENV{INPATH};
#     my $latestset;
#     my (@DirArray, $mask, $buildid, $milestone);

#     foreach my $pointer (@PRODUCT) {
#         my $ProductName = $pointer->{'Name'};

#         print "##PRODUCT: $ProductName\n";
#         my $SetupFullPath = "$SHIP$ENV{INPATH}$PS$ProductName$PS$packpackage$PS";
#         if ( ! ( $workspace && $platform ) ) {
#             print_error ( "Error: environment not set correctly.", 1);
#         }
#         # get latest broadcastet milestone and pack number
#         ($milestone, $buildid) = get_milestoneAndBuildID( $workspace, $platform );

#         if (defined $ENV{CWS_WORK_STAMP}) {
#             # /unxlngi6.pro/StarOffice/rpm/ SRC680_m245_native_packed-2_en-US.9267/
#             $mask = "^$workspace" . "_" . $milestone . "_native_packed-(\\d+)_en-US\\.$buildid";
#         } else {
#             # cws03/os110/OOH680/src.m4/instset_native/unxlngi6.pro/StarOffice/rpm install/en-US/RPMS
#             $mask = "^$workspace" . "_" . $milestone . "_native_packed-(\\d+)_en-US\\.$buildid";
#         }
#         print "MASK: $mask\n";
#         $global_instset_mask = $mask;
#         getSubFiles ($SetupFullPath, \@DirArray, $mask);
#         @DirArray = sort InstsetSort @DirArray;

#         if ($#DirArray > -1) {
#             $latestset = $DirArray [$#DirArray];
#             $DirName = $latestset;
#         }
#         else {
#                 print_error ("Cannot find install set $SetupFullPath for $workspace $milestone", 2);
#         }

#         print "Latest install sets: $latestset\n" if $debug;

#         print "$DirName\t $SetupFullPath\n" if $debug;
#         # compare with file system
#         # at the moment just the existence is checked. For security reasons it might be
#         # better to additionally check whether there are newer sets (this must not happen,
#         # but who knows ...)
#         if ( -d $SetupFullPath ) {
#             # if found => return and don't continue searching
#             # this is usefull to set a priority to find installsets in @PRODUCT
#             # first comes, first wins
#             return ($DirName, $SetupFullPath);
#         }
#     }

#     # compare with file system
#     # at the moment just the existence is checked. For security reasons it might be
#     # better to additionally check whether there are newer sets (this must not happen,
#     # but who knows ...)
#     if ( -d $SetupFullPath ) {
#         return ($DirName, $SetupFullPath);
#     } else {
#         print_error ("Cannot find install set $SetupFullPath for $workspace $milestone", 2);
#         }
# }

# sub get_milestoneAndBuildID {
#     my ( $ws, $pf ) = @_;
#     my ($milestone, $buildid, $upd, $path, $updext, $line);

#     if ( $ws =~ /^\D+(\d+)$/) {
#         $upd = $1;
#     }

#     if (defined ($ENV{UPDMINOREXT})) {
#         $updext = $ENV{UPDMINOREXT};
#     }
#     else {
#         $updext = "";
#     }

#     $path = "$ENV{SOLARVER}$PS$pf$PS" . "inc$updext$PS$upd" . "minor.mk";
#     print "$path\n" if $debug;
#     local *MINORMK;

#     if ( !open(MINORMK,$path) ) {
#         print "FATAL: can't open $path\n";
#         return (0,0);
#     }

#     if (!eof(MINORMK)) {
#         while ($line = <MINORMK>) {
#             chomp($line);
#             if ( $line =~ /LAST_MINOR=(\w+)/ ) {
#                 $milestone = $1;
#             }
#             elsif ( $line =~ /BUILD=(\d+)/ ) {
#                 $buildid = $1;
#             }
#         }

#         close(MINORMK);
#     }
#     if (!defined($milestone)) {
#             print_error ("Milestone ist not defined!", 2);
#     }
#     if (!defined($buildid)) {
#             print_error ("Build-ID ist not defined!", 2);
#     }

#     return ($milestone, $buildid);
# }

sub print_error
{
    my $message     = shift;
    my $error_code  = shift;

    print STDERR "ERROR: $message\n";
    doExit($error_code);
}

sub getSubFiles {
    my ($startDir, $DirArray_ref, $mask) = @_;
    my ($dir);
    local *DIR;
    opendir(DIR,"$startDir");
    while($dir = readdir(DIR)) {
        if (($dir =~ /\.$/) or ($dir !~ /$mask/)) {
            next;
        }
        push (@{$DirArray_ref}, "$dir");
    }
    closedir(DIR);

}

sub InstsetSort {
    my ($a1, $b1);
    if ($a =~ /$global_instset_mask/) {
        $a1 = $1;
    }
    if ($b =~ /$global_instset_mask/) {
        $b1 = $1;
    }
    $a1 <=> $b1;
}

sub isLocalEnv {
    my $returnvalue = 0;
    if (defined ($ENV{SOL_TMP}) && defined ($ENV{SOLARVERSION})) {
        my $mask = $ENV{SOL_TMP};
        $mask =~ s/\\/\\\\/;
        print "Mask: $mask\n" if $debug;
        if ($ENV{SOLARVERSION}=~ /$mask/) {
            $returnvalue = 1;
        }
    }
    return $returnvalue;
}

sub getSubDirs {
    my ($startDir, $DirArray_ref) = @_;
    my ($dir);
    opendir(DIR,"$startDir");
    while($dir = readdir(DIR)) {
        if (($dir !~ /\.$/) and ( -d "$startDir$dir") ) {
            push (@{$DirArray_ref}, "$dir");
        }
    }
    closedir(DIR);

}

sub findSubDir {
    my ($DirArray_ref) = @_;
    my (@sortedArray, $dir, $instdir);
    @sortedArray = sort langsort @{$DirArray_ref};
    print "Langs: @sortedArray\n" if $is_command_infos;
    foreach $dir (@sortedArray) {
        if ($dir =~ /log$/) {
            next;
        }
        $instdir = "$dir";
        return $instdir;
    }
    return "";
}

sub langsort {
    if ($a eq $prefered_lang) {
        return -1;
    }
    elsif ($b eq $prefered_lang) {
        return 1;
    }
    else {
        $a cmp $b;
    }
}

sub doExit
{
    my $error_code  = shift;
    # if ($sversion_saved) {
    # restore_sversion ($SVERSION_INI);
    # }
    # if ($is_remove_on_error) {
    #     if ($is_do_deinstall) {
    #         deinstallInstallation ($installpath);
    #     }
    #     removeInstallation($installpath);
    # }
    # else {
    #     print_notRemoved ($installpath);
    # }
    if ( $error_code ) {
        print STDERR "\nFAILURE: $script_name aborted.\n";
    }
    exit($error_code);

}


sub parseArgs
{
    for (my $i=0; $i<=$#ARGV; $i++) {
        if ( $ARGV[$i] =~ /^-cwscheckapi$/ ) {
            my $value = $ARGV[++$i];
            if ($value =~ /^true$/ || ($value =~ /^1$/) ) {
                $cwsCheckApi = 1;
                $cleanup = 1;
                $autorun = 1;
            };

        }

        if ( $ARGV[$i] =~ /^-dest$/ ) {
            $OfficeDestinationPath = $ARGV[++$i];
        }

        if ( $ARGV[$i] =~ /^-cleanup$/ ) {
            my $value = $ARGV[++$i];
            if ($value =~ /^true$/ || ($value =~ /^1$/ )) { $cleanup = 1 };
        }

        if ( $ARGV[$i] =~ /^-autorun$/ ) {
            my $value = $ARGV[++$i];
            if ($value =~ /^true$/ || ($value =~ /^1$/ )) { $autorun = 1 };
        }

        if ( $ARGV[$i] =~ /^-autoimpress$/ ) {
            my $value = $ARGV[++$i];
            if ($value =~ /^true$/ || ($value =~ /^1$/ )) { $autoimpress = 1 };
        }

        if ( $ARGV[$i] =~ /^-debug$/ ) {
            my $value = $ARGV[++$i];
            if ($value =~ /^true$/ || ($value =~ /^1$/ )) { $debug = 1 };
        }

        if ( $ARGV[$i] =~ /^-ooo$/ ) {
            my $value = $ARGV[++$i];
            if ($value =~ /^true$/ || ($value =~ /^1$/ )) { $is_ooo = 1 };
        }
    }

}

sub getSubDirsFullPath {
    my ($startDir, $DirArray_ref) = @_;
    my ($dir);
    opendir(DIR,"$startDir");
    while($dir = readdir(DIR)) {
        if (($dir !~ /\.$/) and ( -d "$startDir$dir") ) {
            push (@{$DirArray_ref}, "$startDir$dir");
        }
    }
    closedir(DIR);
}

sub errorFromOpen {
    my ($file) = @_;
    print_error( "can not open $file", "1");
}

