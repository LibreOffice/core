:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   $RCSfile: smoketest.pl,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2004-08-11 09:19:55 $
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
# smoketest - do the smoketest
#

use File::Path;
use File::Copy;
use Getopt::Long;

########################
#                       #
#   Globale Variablen   #
#                       #
#########################
$is_debug = 0;
$is_command_infos = 0;
$is_protocol_test = 0;
$is_remove_on_error = 0;
$is_remove_at_end = 1;
$is_do_statistics = 0;
$is_oo = 1;

$gui = $ENV{GUI};
$temp_path = $ENV{TEMP};
$vcsid = $ENV{VCSID};
$sversion_saved = 0;
$FileURLPrefix = "file:///";
$userinstalldir = "Office";
$cygwin = "cygwin";
$prefered_lang = "en-US";
$global_instset_mask = "";

if (!defined($gui)) {
    print "The workstamp is missing. Please use setsolar\n";
    exit(1);
}

if (!defined($temp_path)) {
    print "Your temp value is missing. Please set your temp-variable\n";
    exit(1);
}

if (!((defined($ENV{UPDATER})) and ($ENV{UPDATER} eq "YES") and !defined($ENV{CWS_WORK_STAMP})) ) {
    $is_protocol_test = 0;
}

if ($^O =~ /$cygwin/) {
    $gui = $cygwin;
}

if ($gui eq "WNT") {
    $PathSeparator = '\\';
    $NewPathSeparator = ';';
    $dos = "$ENV{COMSPEC} -c";
    $PERL = "$dos $ENV{PERL}";
    $REMOVE_DIR = "$dos del /qsxyz";
    $REMOVE_FILE = "$dos del";
    $LIST_DIR = "$dos ls";
    $COPY_FILE = "$dos copy";
    $COPY_DIR = "$dos copy /s";
    $MK_DIR = "md";
    $RENAME_FILE = "ren";
    $nul = '> NUL';
    $RESPFILE="response_fat_wnt";
    $SVERSION_INI = $ENV{USERPROFILE} . $PathSeparator . "Anwendungsdaten" . $PathSeparator . "sversion.ini";
    $SOFFICEBIN = "soffice.exe";
    $bootstrapini = "bootstrap.ini";
    $bootstrapiniTemp = $bootstrapini . "_";
}
elsif ($gui eq "UNX") {
    $PathSeparator = '/';
    $NewPathSeparator = ':';
    $dos = "";
    $PERL = "$ENV{PERL}";
    $REMOVE_DIR = "rm -rf";
    $REMOVE_FILE = "rm";
    $LIST_DIR = "ls";
    $COPY_FILE = "cp -f";
    $COPY_DIR = "cp -rf";
    $MK_DIR = "mkdir";
    $RENAME_FILE = "mv";
    $nul = '> /dev/null';
    $RESPFILE="response_fat_unx";
    $SVERSION_INI = $ENV{HOME} . $PathSeparator . ".sversionrc";
    $SOFFICEBIN = "soffice";
    $bootstrapini = "bootstraprc";
    $bootstrapiniTemp = $bootstrapini . "_";
    $system = `uname -s`;
    chomp $system;
    $mach = `uname -m`;
    chomp $mach;
    if ( (defined($system)) && ($system eq "SunOS") && defined($mach) && ($mach eq sun4u) ) {
        $ENV{DBGSV_INIT} = "dbgsv.init";
    }
    else {
        $ENV{DBGSV_INIT} = "dbgsv.ini";
    }
}
elsif ($gui eq $cygwin) {
    $PathSeparator = '/';
    $NewPathSeparator = ':';
    $dos = "";
    $PERL = "$ENV{PERL}";
    $REMOVE_DIR = "rm -rf";
    $REMOVE_FILE = "rm";
    $LIST_DIR = "ls";
    $COPY_FILE = "cp -f";
    $COPY_DIR = "cp -rf";
    $MK_DIR = "mkdir";
    $RENAME_FILE = "mv";
    $nul = '> /dev/null';
    $RESPFILE="response_fat_wnt";
    $SVERSION_INI = $ENV{USERPROFILE} . $PathSeparator . "Anwendungsdaten" . $PathSeparator . "sversion.ini";
    $SOFFICEBIN = "soffice";
    $bootstrapini = "bootstrap.ini";
    $bootstrapiniTemp = $bootstrapini . "_";
    $CygwinLineends = $/;
    $WinLineends = "\r\n";
    &SetWinLineends();
}
else {
    print_error ("not supported system\n",1);
}

if ($is_oo) {
    @install_list = (   'instsetoo_native'
    );
}
else {
    @install_list = (   'instset_native'
    );
}

@error_messages = ( '',
            '',
            'Error during installation!',
            'Error: patching configuration failed!',
            'Error: starting office failed!',
            'Error during testing',
            '',
            'Error in setup log',
            'installationsset is not complete',
            'can not copy all basic scripts',
            'can not patch bottstrapini'
);

my $show_NoMessage = 0;
my $show_Message = 1;

my $error_setup = 2;
my $error_patchConfig = 3;
my $error_startOffice = 4;
my $error_testResult = 5;
my $error_deinstall = 6;
my $error_setup_log = 7;
my $error_installset = 8;
my $error_copyBasic = 9;
my $error_patchBootstrap = 10;

if ($is_oo) {
    $PRODUCT="OpenOffice";
}
else {
    $PRODUCT="StarOffice";
}

$StandDir = $ENV{SOLARSRC} . $PathSeparator;
$SHIP=$ENV{SHIPDRIVE} . $PathSeparator;
$PORDUCT = "$SHIP$ENV{INPATH}$PathSeparator$PRODUCT$PathSeparator";
$DATA="$ENV{DMAKE_WORK_DIR}$PathSeparator" . "data$PathSeparator";
$WORK_STAMP_LC=$ENV{WORK_STAMP};
$WORK_STAMP_LC =~ tr/A-Z/a-z/;

if (defined($ENV{INSTALLPATH_SMOKETEST})) {
    $installpath_without = $ENV{INSTALLPATH_SMOKETEST};
}
else {
    $installpath_without = $temp_path;
}

if (defined($vcsid)) {
    $installpath_without .= $PathSeparator . $vcsid;
}
$installpath_without .= $PathSeparator . "office";
$installpath = $installpath_without . $PathSeparator;

$ENV{STAR_REGISTRY}="";
$milestone = "";

#### options ####

$opt_nr = 0;
GetOptions('nr');

if ($opt_nr)  {
    $is_remove_at_end = 0; # do not remove installation
}
if ( $ARGV[0] ) {
    $milestone = $ARGV[0];
}


#### script id #####

( $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

$id_str = ' $Revision: 1.2 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

print "$script_name -- version: $script_rev\n";


#### main ####

if (!$is_debug) {
    removeOldInstallation($installpath);
    save_sversion ($SVERSION_INI);
}

($INSTSETNAME, $INSTALLSET) = getInstset();

print "Install: $INSTALLSET$INSTSETNAME\n";
prepare();
doTest();

if (!$is_debug) {
    restore_sversion ($SVERSION_INI);
    if ($is_remove_at_end) {
        removeInstallation($installpath);
    }
    else {
        print_notRemoved ($installpath);
    }
}
print "smoketest successful!\n";
exit (0);

#########################
#                       #
#      Procedures       #
#                       #
#########################

sub getSubFiles {
    my ($startDir, $DirArray_ref, $mask) = @_;
    my ($dir);
    opendir(DIR,"$startDir");
    while($dir = readdir(DIR)) {
    if (($dir =~ /\.$/) or ($dir !~ /$mask/)) {
        next;
    }
        push (@{$DirArray_ref}, "$dir");
    }
    closedir(DIR);

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

sub findSubDir {
    my ($DirArray_ref) = @_;
    my (@sortedArray, $dir, $instdir);
    @sortedArray = sort langsort @{$DirArray_ref};
    print "Langs: @sortedArray\n";
    foreach $dir (@sortedArray) {
        if ($dir =~ /log$/) {
            next;
        }
        $instdir = "$dir";
        return $instdir;
    }
    return "";
}

sub prepare {
    my ($Command);
    if ($gui eq "UNX") {
        $ENV{ignore_patch_check}="t";
    }

}

sub is_Installset_ok {
    my ($installpath, $installname) = @_;
    my ($is_ok, $infile);
    $is_ok = 1;
    $infile = $INSTALLSET. "log$PathSeparator";
    $infile =~ s/_pre_//;
    $infile =~ s/[\/\\]normal[\/\\]//;
    $infile =~ s/\.(\d+)$/_$1\.html/;
    #if ($installname =~ /(?:_pre_){0,1}(.*)\.(\d+)/) {
    #   $infile .= "$1_$2.html";
    #   print "protocol: $infile\n" if $is_debug;
    #}
    open INTABLE, "<$infile" or return errorFromOpen ($infile);
        while(<INTABLE>) {
                $line = $_;
        if ( $line =~ />(error)(.*)</ ) {
            print "ERROR: $1$2\n";
            $is_ok = 0;
        }
    }
    close INTABLE;
    return $is_ok;
}

sub errorFromOpen {
    my ($file) = @_;
    print "ERROR: can't open $file\n";
    return 0;
}

sub doTest {
    my ($Command);
    my (@Params, $basedir);

    # check installset (error 8)

    if (($INSTSETNAME ne "") and $is_protocol_test) {
        if (!is_Installset_ok ("$PORDUCT", $INSTSETNAME)) {
            print_error ($error_messages[$error_installset], $error_installset);
        }
    }

    # install office (error 2)

    $basedir = doInstall ("$INSTALLSET$INSTSETNAME$PathSeparator", $installpath);
    print "$basedir\n";
    $programpath = "$basedir". "program$PathSeparator";
    $userinstallpath_without = $basedir . $userinstalldir;
    $userinstallpath = $userinstallpath_without . $PathSeparator;
    $userpath = "$userinstallpath" . "user$PathSeparator";
    $basicpath = $userpath . "basic$PathSeparator";
    $standardbasicpath = "$basedir" . "user$PathSeparator" . "basic$PathSeparator";
    $LOGPATH="$userinstallpath" . "user" . $PathSeparator . "temp";

    if ($gui eq "UNX") {
        $Command = "chmod -R 777 \"$installpath\"*";
        execute_system ("$Command");
    }

    # check setup log file (error 7)

#   $Command = "$PERL check_setup.pl \"$programpath" . "setup.log\"";
#   execute_Command ($Command, $error_setup_log, $show_Message);

    # patch config (error 3)

    $Command = "$PERL config.pl \"$basedir \" \"$userinstallpath \" \"$DATA \"";
    execute_Command ($Command, $error_patchConfig, $show_Message);

    # copy basicscripts (error 9)

    $Command = "$COPY_FILE \"$DATA" . "scripts$PathSeparator" . "\"*.x* \"$standardbasicpath" . "Standard$PathSeparator\"";
    execute_Command_withoutOutput ($Command, $error_copyBasic, $show_Message);

    createPath("$basicpath", $error_copyBasic);
    $Command = "$COPY_DIR \"$standardbasicpath\"* \"$basicpath\"";
    execute_Command_withoutOutput ($Command, $error_copyBasic, $show_Message);


    # patching bootstrap (error 11)

    if (!patch_bootstrap ("$programpath$bootstrapini", "$programpath$bootstrapiniTemp")) {
        print_error ($error_messages[$error_patchBootstrap], $error_patchBootstrap);
    }

    # start office (error 4)

    print "Starting Testtool ($INSTSETNAME)\n";

    if ((defined($ENV{OS})) && (defined($ENV{PROEXT})) && ($ENV{OS} eq "LINUX") && ($ENV{PROEXT} eq ".pro") && $is_do_statistics)  {
        print "collecting statistic...\n";
        $Command = "$PERL stats.pl -p=\"$programpath" . "soffice\" -norestore -nocrashreport macro:///Standard.Global.StartTestWithDefaultOptions";
        execute_Command ($Command, $error_startOffice, $show_Message);
    }
    else {
        $Command = "\"$programpath" . "soffice\" -norestore -nocrashreport macro:///Standard.Global.StartTestWithDefaultOptions";
        execute_Command ($Command, $error_startOffice, $show_Message);
    }

    # test if smoketest is ok (error 5)

    $Command = "$PERL testlog.pl \"$LOGPATH\"";
    execute_Command ($Command, $error_testResult, $show_Message);

    # deinstall office (error 6)

    if ($is_remove_at_end) {
        print "Deinstalling Office\n";
    }
#   $Command = "$PERL deinstall.pl \"$LOGPATH\" \"$installpath\"";
#   execute_Command ($Command, $error_deinstall, $show_NoMessage);
}

sub doInstall {
    my ($installsetpath, $dest_installdir) = @_;
    my ($DirArray, $mask, $file, $Command, $optdir, $rpmdir, $system, $mach, $basedir);
    if (($gui eq "WNT") or ($gui eq $cygwin)) {
        $mask = "\\.msi\$";
        getSubFiles ("$installsetpath", \@DirArray, $mask);
        if ($#DirArray == -1) {
                    print_error ("Installationset in $installsetpath is incomplete", 2);
        }
        foreach $file (@DirArray) {
            $Command = "msiexec.exe -i $installsetpath$file -qn INSTALLLOCATION=$dest_installdir";
            execute_Command ($Command, $error_setup, $show_Message);
        }
        @DirArray = ();
        getSubDirsFullPath ($dest_installdir, \@DirArray);
        if ($#DirArray == 0) {
            $basedir = $DirArray[0] . $PathSeparator;
        }
        elsif ($#DirArray == -1) {
            print_error ($error_setup, $show_Message);
        }
        else {
            $basedir = $$dest_installdir;
         }
    }
    elsif ($gui eq "UNX") {
        $system = `uname -s`;
        chomp $system;
        $mach = `uname -m`;
        chomp $mach;
        if ( (defined($system)) && ($system eq "Linux") ) {
            $installsetpath .= "RPMS$PathSeparator";
            $optdir = "$dest_installdir" . "opt" . $PathSeparator;
            $rpmdir = "$dest_installdir" . "rpm" . $PathSeparator;
            createPath ($optdir, $error_setup);
            createPath ($rpmdir, $error_setup);
            $Command = "rpm --initdb --dbpath $rpmdir";
            execute_Command ($Command, $error_setup, $show_Message);
            $mask = "\\.rpm\$";
            getSubFiles ("$installsetpath", \@DirArray, $mask);
            if ($#DirArray == -1) {
                            print_error ("Installationset in $installsetpath is incomplete", 2);
            }
            foreach $file (@DirArray) {
                if ( ($file =~ /-menus-/) or ($file =~ /^adabas/) or () ) {
                    next;
                }
                $Command = "rpm --install --nodeps -vh --relocate /opt=$optdir --dbpath $rpmdir $installsetpath$file";
                execute_Command_withoutErrorcheck ($Command, $error_setup, $show_Message);
            }
        }
        elsif ( (defined($system)) && ($system eq "SunOS") ) {
            if ($mach eq sun4u) {
                $solarisdata = $DATA . "solaris$PathSeparator" . "sparc$PathSeparator";
            }
            else {
                $solarisdata = $DATA . "solaris$PathSeparator" . "x86$PathSeparator";
            }
            $installsetpath .= "packages$PathSeparator";
            $optdir = "$dest_installdir" . "opt" . $PathSeparator;
            createPath ($optdir, $error_setup);
            createPath ($dest_installdir . "usr$PathSeparator" . "bin", $error_setup);
            getSubDirs ("$installsetpath", \@DirArray);
            $ENV{LD_PRELOAD} = $solarisdata . "getuid.so";
            if ($#DirArray == -1) {
                            print_error ("Installationset in $installsetpath is incomplete", 2);
            }
            foreach $file (@DirArray) {
                if ( ($file =~ /-gnome/) or ($file =~ /-cde/) or () ) {
                    next;
                }
                $Command = "/usr/sbin/pkgadd -a $solarisdata" . "admin -d $installsetpath -R $dest_installdir $file";
                execute_Command_withoutErrorcheck ($Command, $error_setup, $show_Message);
            }
        }
        @DirArray = ();
        getSubDirsFullPath ($optdir, \@DirArray);
        if ($#DirArray == 0) {
            $basedir = $DirArray[0] . $PathSeparator;
        }
        elsif ($#DirArray == -1) {
            print_error ($error_setup, $show_Message);
        }
        else {
            $basedir = $optdir;
         }
    }
    return ($basedir);
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

sub getInstset {
    my ($INSTSET, $NEWINSTSET);
    my (@DirArray, $InstDir, $RootDir, $TestDir1, $TestDir2);
    print "get Instset\n" if $is_debug;
    $NEWINSTSET = "";
    if (($ENV{UPDATER} eq "YES") and !defined($ENV{CWS_WORK_STAMP})) {
        ($NEWINSTSET, $INSTSET) = getSetFromServer();
    }
    else {
        $InstDir="";
        $RootDir=$ENV{DMAKE_WORK_DIR};
        $RootDir=~s/\w+$//;
        foreach $project (@install_list) {
            @DirArray=();
            $TestDir1 = "$RootDir$project$PathSeparator$ENV{INPATH}$PathSeparator$PRODUCT$PathSeparator" . "install$PathSeparator";
            $TestDir2 = "$StandDir$project$PathSeparator$ENV{INPATH}$PathSeparator$PRODUCT$PathSeparator" . "install$PathSeparator";
            if (-e "$TestDir1") {
                $InstDir= $TestDir1;
            }
            elsif (-e "$TestDir2") {
                $InstDir="$TestDir2";
            }
            if ($InstDir eq "") {
                next;
            }
            getSubDirs ("$InstDir", \@DirArray);
            $INSTSET = findSubDir (\@DirArray);
            print "Lang-Sel: $INSTSET\n";
            if ($INSTSET ne "")  {
                $NEWINSTSET = $INSTSET;
                $INSTSET = $InstDir;
                print "new: $INSTSET\n";
            }
            if (($INSTSET ne "") and (-e $INSTSET)) {
                return ($NEWINSTSET, $INSTSET);
            }
        }
        print_error ("no installationset found\n",2);
    }
    return ($NEWINSTSET, $INSTSET);
}

sub get_milestone {
    my ( $ws, $pf ) = @_;
    my ($milestone, $upd, $path, $updext);

    if ( $ws =~ /^\D+(\d+)$/) {
        $upd = $1;
    }

    if (defined ($ENV{UPDMINOREXT})) {
        $updext = $ENV{UPDMINOREXT};
    }
    else {
        $updext = "";
    }

    $path = "$ENV{SOLAR_SOURCE_ROOT}$PathSeparator$ws$PathSeparator$pf$PathSeparator" . "inc$updext$PathSeparator$upd" . "minor.mk";
    print "$path\n" if $is_debug;
    if ( !open(MINORMK,$path) ) {
        print "FATAL: can't open $path\n";
        return (0,0);
    }

    if (!eof(MINORMK)) {
        while (<MINORMK>) {
            chomp;
            if ( /LAST_MINOR=(\w+)/ ) {
                $milestone = $1;
            }
        }

        close(MINORMK);
    }
    return ($milestone);
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

sub getSetFromServer {
    my ($DirName, $SetupFullPath);
    my $workspace = $ENV{WORK_STAMP};
    my $platform  = $ENV{INPATH};
    my $latestset;
    my (@DirArray, $mask);
    $SetupFullPath = $PORDUCT;
    if ( ! ( $workspace && $platform ) ) {
        print_error ( "Error: environment not set correctly.", 1);
    }
    # get latest broadcastet milestone and pack number
    $milestone = get_milestone( $workspace, $platform );

#    if ( $SetupFullPath =~ /^\/s.*\/install\// ) {
#        if ( $gui eq "UNX" ) {
#            $SetupFullPath = "/net/jumbo.germany" . $SetupFullPath;
#        } else {
#            $SetupFullPath = "\\\\jumbo" . $SetupFullPath;
#        }
#    }

#    my $ws_lc = lc $workspace;
    $mask = "^$workspace" . "_" . $milestone . "_native_packed-(\\d+)_en-US\\.\\d+";
    $global_instset_mask = $mask;
    getSubFiles ($SetupFullPath, \@DirArray, $mask);
    @DirArray = sort InstsetSort @DirArray;
    if ($#DirArray > -1) {
    $latestset = $DirArray [$#DirArray];
    $DirName = $latestset;
    }
    else {
            print_error ("Cannot find install set $SetupFullPath for $workspace $milestone", 2);
    }
    print "Latest install sets: $latestset\n" if $is_debug;

    print "$DirName\t $SetupFullPath\n" if $is_debug;

    # compare with file system
    # at the moment just the existence is checked. For security reasons it might be
    # better to additionally check whether there are newer sets (this must not happen,
    # but who knows ...)
    if ( -d $SetupFullPath ) {
        return ($DirName, $SetupFullPath);
    } else {
        print_error ("Cannot find install set $SetupFullPath for $workspace $milestone", 2);
        }
}

sub patch_bootstrap {
    my ($sourcefile, $destfile) = @_;
    my (@convert_split, $line);
    my ($Error) = 1;

    if ($is_debug) {
        print "patching bootstrap $sourcefile ...\n";
        return 1;
    }

    $Error &= move ("$sourcefile", "$destfile");

    open OUTFILE, ">$sourcefile" or return errorFromOpen ($sourcefile);
    open INFILE, "<$destfile" or return errorFromOpen ($destfile);
    while(<INFILE>) {
        $line = $_;

        if ( $line =~ /UserInstallation/ ) {
            @convert_split = split "=", $line;
            $line = $convert_split[0];
            $line .= "=";
            if ($gui eq $cygwin) {
                $line .= ConvertToFileURL(ConvertCygwinToWin($userinstallpath_without));
            }
            else {
                $line .= ConvertToFileURL($userinstallpath_without);
            }
            $line .= $/;
        }
        print OUTFILE "$line";

    }

    close OUTFILE;
    close INFILE;
    $Error &= unlink ($destfile);
    return ($Error);
}

sub SetWinLineends () {
    $/ = $WinLineends;
}

sub SetCygwinLineends () {
    $/ = $CygwinLineends;
}

sub ConvertToFileURL {
    my ($filename) = @_;
    my ($FileURL);

    $FileURL = $FileURLPrefix . $filename;
    $FileURL =~ s/\\/\//g;
    $FileURL =~ s/\/\/\/\//\/\/\//; # 4 slashes to 3 slashes
    return ($FileURL);
}

sub ConvertCygwinToWin_Shell {
    my ($cygwinpath) = @_;
    my ($winpath);
    $winpath = ConvertCygwinToWin ($cygwinpath);
    $winpath =~ s/\\/\\\\/g;
    return ($winpath);
}

sub ConvertCygwinToWin {
    my ($cygwinpath) = @_;
    my ($winpath);
    SetCygwinLineends();
    $winpath=`cygpath --windows $cygwinpath`;
    chomp($winpath);
    SetWinLineends();
    return ($winpath);
}

sub createPath {
    my ($path, $Error) = @_;
    if (!-d "$path") {
        if (!$is_debug) {
            eval {mkpath("$path", 0, 0777)};
            if ($@) {
                print_error ($error_messages[$Error], $Error);
            }
        }
        else {
            print "mkpath($path, 0, 0777)\n";
        }
    }
}

sub save_sversion {
    my ($sversion) = @_;
    my ($sversion_bak) = $sversion . "_";
    if (-e $sversion) {
        if (-e $sversion_bak) {
            execute_system("$REMOVE_FILE \"$sversion_bak\"");
        }
            execute_system("$COPY_FILE \"$sversion\" \"$sversion_bak\"");
        execute_system("$REMOVE_FILE \"$sversion\"");
        $sversion_saved = 1;
    }
}

sub restore_sversion {
    my ($sversion) = @_;
    my ($sversion_bak) = $sversion . "_";
    if ($sversion_saved) {
        if (-e $sversion) {
            execute_system("$REMOVE_FILE \"$sversion\"");
        }
            execute_system("$COPY_FILE \"$sversion_bak\" \"$sversion\"");
        execute_system("$REMOVE_FILE \"$sversion_bak\"");
        $sversion_saved = 0;
    }
}

sub removeOldInstallation  {
    my ($installpath) = @_;
    if (!$is_debug) {
        if (-e $installpath) {
            execute_system("$REMOVE_DIR \"$installpath\"");
        }
         if (-e $installpath) {
            print_error ("can not clear old installation in $installpath\n", 3);
        }
    }
}

sub removeInstallation  {
    my ($installpath) = @_;
    if (!$is_debug) {
        if (-e $installpath) {
            execute_system("$REMOVE_DIR \"$installpath\"");
        }
    }
}

sub setInstallpath {
    my ($infile, $outfile, $installpath) = @_;
    if (-e $outfile) {
        execute_system ("$REMOVE_FILE \"$outfile\"");

    }
    open OUTTABLE, ">$outfile" or die "Error: can\'t open solarfile $outfile";
    open INTABLE, "<$infile" or die "Error: can\'t open solarfile $infile";
        while(<INTABLE>) {
                $line = $_;
        if ( $line =~ /^DESTINATIONPATH=officeinstallpath/ ) {
            $line =~ s/officeinstallpath/$installpath/;
        }
        print OUTTABLE "$line";
    }
    close INTABLE;
    close OUTTABLE;
}

sub execute_Command {
    my ($Command, $Errorcode, $showMessage) = @_;
    my ($Returncode);
    if (!$is_debug) {
        print "$Command\n" if $is_command_infos;
        $Returncode = system ("$Command");
        if ($Returncode) {
            if ($showMessage) {
                print_error ($error_messages[$Errorcode], $Errorcode);
            }
            else {
                do_exit ($Errorcode);
            }
        }
    }
    else {
        print "$Command\n";
    }
}

sub execute_Command_withoutErrorcheck {
    my ($Command, $Errorcode, $showMessage) = @_;
    my ($Returncode);
    if (!$is_debug) {
        print "$Command\n" if $is_command_infos;
        $Returncode = system ("$Command");
        if ($Returncode) {
            if ($showMessage) {
                print_warning ($error_messages[$Errorcode], $Errorcode);
            }
        }
    }
    else {
        print "$Command\n";
    }
}

sub execute_Command_withoutOutput {
    my ($Command, $Errorcode, $showMessage) = @_;
    my ($Returncode);
    if (!$is_debug) {
        print "$Command\n" if $is_command_infos;
        $Returncode = execute_system ("$Command");
        if ($Returncode) {
            if ($showMessage) {
                print_error ($error_messages[$Errorcode], $Errorcode);
            }
            else {
                do_exit ($Errorcode);
            }
        }
    }
    else {
        print "$Command\n";
    }
}

sub execute_system {
    my ($command) = shift;

    if ( $is_command_infos ) {
        print STDERR "TRACE_SYSTEM: $command\n";
    }
    open( COMMAND, "$command 2>&1 |");
    close(COMMAND);
    return $?;
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

    do_exit($error_code);
}

sub print_notRemoved {
    my ($installpath) = @_;
    print "no deinstallation from $installpath\n";
}

sub do_exit
{
    my $error_code  = shift;

    if ($sversion_saved) {
    restore_sversion ($SVERSION_INI);
    }
    if ($is_remove_on_error) {
    removeInstallation($installpath);
    }
    else {
    print_notRemoved ($installpath);
    }
    if ( $error_code ) {
    print STDERR "\nFAILURE: $script_name aborted.\n";
    }
    exit($error_code);

}
