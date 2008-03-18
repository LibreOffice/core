:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: smoketest.pl,v $
#
#   $Revision: 1.31 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 14:30:44 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

#
# smoketest - do the smoketest
#

use File::Basename;
use File::Path;
use File::Copy;
use Getopt::Long;

########################
#                       #
#   Globale Variablen   #
#                       #
#########################
$is_debug = 0;           # run without executing commands
$is_command_infos = 0;   # print command details before exec
$is_protocol_test = 0;
$is_remove_on_error = 0;
$is_remove_at_end = 1;
$is_do_statistics = 0;
$is_do_deinstall = 0;
$is_admin_installation = 1;
$is_oo = 1;

$gui = $ENV{GUI};
$temp_path = $ENV{TEMP};
$vcsid = $ENV{VCSID};
$sversion_saved = 0;
$FileURLPrefix = "file:///";
$userinstalldir = "UserInstallation";
$cygwin = "cygwin";
$prefered_lang = "en-US";
$global_instset_mask = "";
$smoketest_install = $ENV{SMOKETESTINSTALLSET};

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

if (($gui eq "WNT") and ($ENV{USE_SHELL} ne "4nt")) {
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
    $packpackage = "msi";
}
elsif ($gui eq "UNX") {
    $is_do_deinstall = 0;
    $PathSeparator = '/';
    $NewPathSeparator = ':';
    $dos = "";
    $PERL = "$ENV{PERL}";
    $REMOVE_DIR = "rm -rf";
    $REMOVE_FILE = "rm -f";
    $LIST_DIR = "ls";
    $COPY_FILE = "cp -f";
    $MK_DIR = "mkdir";
    $RENAME_FILE = "mv";
    $nul = '> /dev/null';
    $RESPFILE="response_fat_unx";
    $SVERSION_INI = $ENV{HOME} . $PathSeparator . ".sversionrc";
    $bootstrapini = "bootstraprc";
    $bootstrapiniTemp = $bootstrapini . "_";
    $packpackage = $ENV{PKGFORMAT};
    if ($ENV{OS} eq "MACOSX") {
        $SOFFICEBIN = "soffice.bin";
        $COPY_DIR = "cp -RPfp";
    }
    else {
        $SOFFICEBIN = "soffice";
        $COPY_DIR = "cp -rf";
    }
}
elsif ($gui eq $cygwin) {
    $PathSeparator = '/';
    $NewPathSeparator = ':';
    $dos = "";
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
    $SVERSION_INI = $ENV{USERPROFILE} . $PathSeparator . "Anwendungsdaten" . $PathSeparator . "sversion.ini";
    $SOFFICEBIN = "soffice";
    $bootstrapini = "bootstrap.ini";
    $bootstrapiniTemp = $bootstrapini . "_";
    $CygwinLineends = $/;
    $WinLineends = "\r\n";
    &SetWinLineends();
    $packpackage = "msi";
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

my $show_NoMessage = 0;
my $show_Message = 1;

my $error_logflag = 1;
my $error_setup = 2;
my $error_patchConfig = 3;
my $error_startOffice = 4;
my $error_testResult = 5;
my $error_copyExtension = 6;
my $error_setup_log = 7;
my $error_installset = 8;
my $error_copyBasic = 9;
my $error_patchBootstrap = 10;
my $error_msiexec = 11;
my $error_deinst = 12;

my $command_normal = 0;
my $command_withoutErrorcheck = 1;
my $command_withoutOutput = 2;

if ($is_oo) {
    $PRODUCT="OpenOffice";
}
else {
    $PRODUCT="StarOffice";
}

if ($ENV{PKGFORMAT} eq "installed") {
    $packpackage = $ENV{PKGFORMAT}; # take it for all environments
}
$StandDir = $ENV{SOLARSRC} . $PathSeparator;
$SHIP = defined $ENV{SHIPDRIVE} ? $ENV{SHIPDRIVE} . $PathSeparator : "shipdrive_not_set";
$PORDUCT = "$SHIP$ENV{INPATH}$PathSeparator$PRODUCT$PathSeparator$packpackage$PathSeparator";
$DATA="$ENV{DMAKE_WORK_DIR}$PathSeparator" . "data$PathSeparator";
$WORK_STAMP_LC=$ENV{WORK_STAMP};
$WORK_STAMP_LC =~ tr/A-Z/a-z/;
$ENV{DBGSV_INIT} = $DATA . "dbgsv.ini";
$ExtensionDir = $ENV{DMAKE_WORK_DIR} . $PathSeparator . $ENV{OUTPATH} . $ENV{PROEXT} . $PathSeparator . "bin" . $PathSeparator;
if (defined($ENV{INSTALLPATH_SMOKETEST})) {
    $installpath_without = $ENV{INSTALLPATH_SMOKETEST};
}
else {
    $installpath_without = $temp_path;
}

if (defined($vcsid)) {
    $installpath_without .= $PathSeparator . $vcsid;
}
if (!$is_oo) {
    $installpath_without .= $PathSeparator . "StarOffice";
}
else {
    $installpath_without .= $PathSeparator . "OpenOffice";
}

$installpath = $installpath_without . $PathSeparator;
$installpath = glob $installpath;

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

$id_str = ' $Revision: 1.31 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

print "$script_name -- version: $script_rev\n";


#### main ####

if (!$is_debug) {
    if ($is_do_deinstall) {
        deinstallInstallation ($installpath);
    }
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
        if ($is_do_deinstall) {
            deinstallInstallation ($installpath);
        }
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

sub findMultiple {
    my ($base, $name) = @_;
    my ($f, @s, @res);
    opendir(DIR, $base) or die "cannot open dir $base";
    while ($f = readdir(DIR)) {
        push @res, "$base" if $f eq $name;
        push @s, $f if
            -d "$base$f" && ! -l "$base$f" && $f ne '.' && $f ne '..';
    }
    closedir(DIR);
    foreach $f (@s) {
        push @res, findMultiple("$base$f$PathSeparator", $name);
    }
    return @res;
}

sub findUnique {
    my ($base, $name) = @_;
    my @res = findMultiple($base, $name);
    die "no unique $name in $base" unless @res == 1;
    return $res[0];
}

sub prepare {
    if ($gui eq "UNX") {
        $ENV{ignore_patch_check}="t";
        $ENV{OOO_FORCE_DESKTOP} = "none";
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
    my (@Params, $basedir, $basisdir, $branddir);

    # check installset (error 8)

    if (($INSTSETNAME ne "") and $is_protocol_test) {
        if (!is_Installset_ok ("$PORDUCT", $INSTSETNAME)) {
            print_error ($error_messages[$error_installset], $error_installset);
        }
    }

    # install office (error 2)

    $basedir = doInstall ("$INSTALLSET$INSTSETNAME$PathSeparator", $installpath);
    print "$basedir\n";
    $basisdir = findUnique($basedir, 'ure-link');
    $branddir = findUnique($basedir, 'basis-link');
    if ( (defined($ENV{OS})) && ($ENV{OS} eq "MACOSX") ) {
        $programpath = "$branddir". "MacOS$PathSeparator";
    }
    else {
        $programpath = "$branddir". "program$PathSeparator";
    }
    $userinstallpath_without = $installpath . $userinstalldir;
    $userinstallpath = $userinstallpath_without . $PathSeparator;
    $userpath = "$userinstallpath" . "user$PathSeparator";
    $basicpath = $userpath . "basic$PathSeparator";
    $standardbasicpath = "$basisdir" . "presets$PathSeparator" . "basic$PathSeparator";
    $LOGPATH="$userinstallpath" . "user" . $PathSeparator . "temp";

    if ($gui eq "UNX") {
        $Command = "chmod -R 777 \"$basedir\"*";
        execute_system ("$Command");
    }

    # check setup log file (error 7)

#   $Command = "$PERL check_setup.pl \"$programpath" . "setup.log\"";
#   execute_Command ($Command, $error_setup_log, $show_Message, $command_normal );

    # patch config (error 3)

    $Command = "$PERL config.pl \"$basisdir \" \"$userinstallpath \" \"$DATA \"";
    execute_Command ($Command, $error_patchConfig, $show_Message, $command_normal );

    # copy basicscripts (error 9)

    $Command = "$COPY_FILE \"$DATA" . "scripts$PathSeparator" . "\"*.x* \"$standardbasicpath" . "Standard$PathSeparator\"";
    execute_Command ($Command, $error_copyBasic, $show_Message, $command_withoutOutput);

    createPath("$basicpath", $error_copyBasic);
    $Command = "$COPY_DIR \"$standardbasicpath\"* \"$basicpath\"";
    execute_Command ($Command, $error_copyBasic, $show_Message, $command_withoutOutput);

    # patching bootstrap (error 11)

    if (!patch_bootstrap ("$programpath$bootstrapini", "$programpath$bootstrapiniTemp")) {
        print_error ($error_messages[$error_patchBootstrap], $error_patchBootstrap);
    }

    # copy extension (error 6)

    createPath ($LOGPATH, $error_copyExtension);
    $Command = "$COPY_FILE \"$ExtensionDir" . "TestExtension.oxt\" " . "\"$LOGPATH$PathSeparator\"";
    execute_Command ($Command, $error_copyExtension, $show_Message, $command_withoutOutput);


    # start office (error 4)

    print "starting office ($INSTSETNAME)\n";

    if ($gui eq "UNX") {
        delete $ENV{LD_LIBRARY_PATH};
    }
    if ($ENV{OS} eq "MACOSX") {
        delete $ENV{DYLD_LIBRARY_PATH};
    }
    if ((defined($ENV{OS})) && (defined($ENV{PROEXT})) && ($ENV{OS} eq "LINUX") && ($ENV{PROEXT} eq ".pro") && $is_do_statistics)  {
        print "collecting statistic...\n";
        $Command = "$PERL stats.pl -p=\"$programpath" . "$SOFFICEBIN\" -norestore -nocrashreport macro:///Standard.Global.StartTestWithDefaultOptions";
        execute_Command ($Command, $error_startOffice, $show_Message, $command_normal);
    }
    else {
        $Command = "\"$programpath" . "$SOFFICEBIN\" -norestore -nocrashreport macro:///Standard.Global.StartTestWithDefaultOptions";
        if ( (defined($ENV{OS})) && ($ENV{OS} eq "MACOSX") ) {
            $Command = "cd \"$programpath\"; " . $Command;
        }
        execute_Command ($Command, $error_startOffice, $show_Message, $command_normal);
    }

    # test if smoketest is ok (error 5)

    $Command = "$PERL testlog.pl \"$LOGPATH\"";
    execute_Command ($Command, $error_testResult, $show_Message, $command_normal);

    # deinstall office (error 6)

    if ($is_remove_at_end) {
        print "Deinstalling Office\n";
    }
#   $Command = "$PERL deinstall.pl \"$LOGPATH\" \"$installpath\"";
#   execute_Command ($Command, $error_deinstall, $show_NoMessage, $command_normal);
}

sub doInstall {
    my ($installsetpath, $dest_installdir) = @_;
    my ($DirArray, $mask, $file, $Command, $optdir, $rpmdir, $system, $mach, $basedir, $output_ref, $olddir, $newdir);
    if ($ENV{PKGFORMAT} eq "installed") {
                createPath ($dest_installdir, $error_setup);
                $Command = "$COPY_DIR \"$installsetpath\" \"$dest_installdir\"";
            execute_Command ($Command, $error_setup, $show_Message, $command_withoutOutput);
        @DirArray = ();
        getSubDirsFullPath ($dest_installdir, \@DirArray);
        if ($#DirArray == 0) {
            $basedir = $DirArray[0] . $PathSeparator;
        }
        elsif ($#DirArray == -1) {
            print_error ($error_setup, $show_Message);
        }
        else {
            $basedir = $dest_installdir;
        }
    }
    elsif (($gui eq "WNT") or ($gui eq $cygwin)) {
        $mask = "\\.msi\$";
        getSubFiles ("$installsetpath", \@DirArray, $mask);
        if ($#DirArray == -1) {
                    print_error ("Installationset in $installsetpath is incomplete", 2);
        }
        foreach $file (@DirArray) {
            if ($gui eq $cygwin) {
                my $convertinstallset = ConvertCygwinToWin_Shell("$installsetpath$file");
                my $convertdestdir = ConvertCygwinToWin_Shell($dest_installdir);
                $_inst_cmd=$ENV{SMOKETEST_SOINSTCMD};
                if ( defined($_inst_cmd) ) {
                    $Command = $_inst_cmd . " $convertinstallset -qn TARGETDIR=$convertdestdir";
                }
                else {
                    if ($is_admin_installation) {
                        $Command = "msiexec.exe /a $convertinstallset -qn TARGETDIR=$convertdestdir ALLUSERS=2";
                    }
                    else {
                        $Command = "msiexec.exe -i $convertinstallset -qn INSTALLLOCATION=$convertdestdir";
                    }
                }
            }
            else {
                $_inst_cmd=$ENV{SMOKETEST_SOINSTCMD};
                if ( defined($_inst_cmd) ) {
                    $Command = $_inst_cmd . " $installsetpath$file -qn TARGETDIR=$dest_installdir";
                }
                else {
                    if ($is_admin_installation)
                    {
                        $Command = "msiexec.exe /a $installsetpath$file -qn TARGETDIR=$dest_installdir ALLUSERS=2";
                    }
                    else {
                        $Command = "msiexec.exe -i $installsetpath$file -qn INSTALLLOCATION=$dest_installdir";
                    }
                }
            }
            if (!$is_oo and !$is_admin_installation) {
                if ($gui eq $cygwin) {
                    my $convertdata = ConvertCygwinToWin_Shell($DATA);
                    $Command .= " TRANSFORMS=$convertdata" . "staroffice.mst";
                }
                else {
                    $Command .= " TRANSFORMS=$DATA" . "staroffice.mst";
                }
            }
            execute_Command ($Command, $error_msiexec, $show_Message,  $command_normal);
        }
        if (!$is_admin_installation) {
            $Command = "$COPY_FILE \"$installsetpath" . "setup.ini" . "\" \"$dest_installdir\"";
            execute_Command ($Command, $error_setup, $show_Message, $command_withoutOutput);
        }
        $basedir = $dest_installdir;
    }
    elsif ($gui eq "UNX") {
        $system = `uname -s`;
        chomp $system;
        $mach = `uname -m`;
        chomp $mach;
        if (defined ($ENV{EPM}) && ($ENV{EPM} eq 'NO')) { # do the install ourselves ...
# FIXME - this tool should work nicely without such hacks
# cut/paste from ooo-build/bin/ooinstall
            my $instoo_dir = "$ENV{SOLARROOT}/instsetoo_native";

            if ( $ENV{'SYSTEM_MOZILLA'} eq 'YES' ) {
            $ENV{'LD_LIBRARY_PATH'} = "$ENV{'MOZ_LIB'}:$ENV{'LD_LIBRARY_PATH'}";
            }
            $ENV{'PYTHONPATH'} = "$ENV{SOLARROOT}/instsetoo_native/$ENV{INPATH}/bin:$ENV{SOLARVERSION}/$ENV{INPATH}/lib";
            $ENV{OUT} = "../$ENV{INPATH}";
            $ENV{LOCAL_OUT} = "../$ENV{INPATH}";
            $ENV{LOCAL_COMMON_OUT} = "../$ENV{INPATH}";
            my $sane_destdir = $dest_installdir;
            $sane_destdir .= "oootest";
            createPath ($sane_destdir, $error_setup);
            $Command = "cd $instoo_dir/util ; perl -w $ENV{SOLARENV}/bin/make_installer.pl " .
            "-f openoffice.lst -l en-US -p OpenOffice " .
            "-packagelist ../inc_openoffice/unix/packagelist.txt " .
            "-addpackagelist ../inc_openoffice/unix/packagelist_language.txt " .
            "-buildid \"smoketestoo\" -simple $sane_destdir";
# FIXME - this tool should work nicely without such evil
            execute_Command ($Command, $error_setup, $show_Message, $command_normal);
            return "$sane_destdir/"

        } elsif ( (defined($system)) && ($system eq "Linux") ) {
            if ($ENV{PKGFORMAT} eq "deb") { # default is rpm
                $installsetpath .= "DEBS$PathSeparator";
            $optdir = "$dest_installdir" . "opt" . $PathSeparator;
                createPath ($optdir, $error_setup);
                $mask = "\\.deb\$";
                getSubFiles ("$installsetpath", \@DirArray, $mask);
                if ($#DirArray == -1) {
                       print_error ("Installationset in $installsetpath is incomplete", 2);
                }
                foreach $file (@DirArray) {
                    if ( ($file =~ /-menus-/) or ($file =~ /^adabas/) or (/^j2re-/) or ($file =~ /-gnome-/) ) {
                        next;
                    }
                    $Command = "dpkg-deb -x $installsetpath$file $dest_installdir";
                    execute_Command ($Command, $error_setup, $show_Message, $command_withoutErrorcheck | $command_withoutOutput);
                }
            }
            else {
                $installsetpath .= "RPMS$PathSeparator";
                $optdir = "$dest_installdir" . "opt" . $PathSeparator;
                $rpmdir = "$dest_installdir" . "rpm" . $PathSeparator;
                createPath ($optdir, $error_setup);
                createPath ($rpmdir, $error_setup);
                $Command = "rpm --initdb --dbpath $rpmdir";
                execute_Command ($Command, $error_setup, $show_Message, $command_withoutOutput);
                $mask = "\\.rpm\$";
                getSubFiles ("$installsetpath", \@DirArray, $mask);
                if ($#DirArray == -1) {
                                print_error ("Installationset in $installsetpath is incomplete", 2);
                }
                foreach $file (@DirArray) {
                    if ( ($file =~ /-menus-/) or ($file =~ /^adabas/) or (/^j2re-/) or ($file =~ /-gnome-/) ) {
                        next;
                    }
                    $Command = "rpm --install --ignoresize --nodeps -vh --relocate /opt=${dest_installdir}opt --dbpath $rpmdir $installsetpath$file";
                    execute_Command ($Command, $error_setup, $show_Message, $command_withoutErrorcheck | $command_withoutOutput);
                }
            }
        }
        elsif ( (defined($system)) && ($system eq "SunOS") ) {
            @DirArray = ();
                $mask = "^.ai.pkg.zone.lock";
                        getSubFiles ("/tmp", \@DirArray, $mask);
            if ($#DirArray >= 0) {
                foreach $file (@DirArray) {
                                $Command = "$REMOVE_FILE /tmp/$file";
                    execute_Command ($Command, $error_logflag, $show_Message, $command_withoutOutput);
                            }
            }
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
            @DirArray = ();
            getSubDirs ("$installsetpath", \@DirArray);
            my $ld_preload = $ENV{LD_PRELOAD};
            $ENV{LD_PRELOAD} = $solarisdata . "getuid.so";
            if ($#DirArray == -1) {
                            print_error ("Installationset in $installsetpath is incomplete", 2);
            }
            foreach $file (@DirArray) {
                if ( ($file =~ /-gnome/) or ($file =~ /-cde/) or ($file =~ /adabas/) or ($file =~ /j3/) or ($file =~ /-desktop-/) ) {
                    next;
                }
                $Command = "pkgparam -d $installsetpath $file BASEDIR";
                $output_ref = execute_Command ($Command, $error_setup, $show_Message, $command_withoutOutput);
                if (($#{@$output_ref} > -1) and ($$output_ref[0] ne "") ) {
                    createPath ("$dest_installdir$$output_ref[0]", $error_setup);
                }
                $Command = "pkgadd -a $solarisdata" . "admin -d $installsetpath -R $dest_installdir $file";
                execute_Command ($Command, $error_setup, $show_Message, $command_withoutErrorcheck | $command_withoutOutput);
            }
            my $pkgadd_tmpfile = "/tmp/.ai.pkg.zone.lock*";
            $Command = "$REMOVE_FILE $pkgadd_tmpfile";
            execute_Command ($Command, $error_setup, $show_NoMessage, $command_withoutErrorcheck | $command_withoutOutput);
            $ENV{LD_PRELOAD} = $ld_preload;
        }
        elsif ( (defined($ENV{OS})) && ($ENV{OS} eq "MACOSX") ) {
            @DirArray = ();
            my $install_dmg;
            getSubFiles ("$installsetpath", \@DirArray, "^[a-zA-Z0-9].*\\.dmg\$");
            if ($#DirArray == 0) {
                $install_dmg = "$installsetpath" . $DirArray[0];
            }
            elsif ($#DirArray > 0) {
                print_error ("more than one installset found in $installsetpath", 2);
            }
            else {
                print_error ("no installset found in $installsetpath", 2);
            }
            $Command = "hdiutil attach " . $install_dmg;
            my $output_ref;
            $output_ref = execute_Command ($Command, $error_setup, $show_Message, $command_withoutOutput);
            my $volumeinfo = $$output_ref[$#{@{$output_ref}}];
            if (!$volumeinfo =~ /OpenOffice/) {
                print_error ("mount of $install_dmg failed", 2);
            }
            $volumeinfo =~ s/\s{2,}/;/g;
            my @volumeinfos = split(/;/,$volumeinfo);
            my $detachpath = $volumeinfos[0];
            my $newinstallsetpath = $volumeinfos[2] . $PathSeparator;
            createPath ($dest_installdir, $error_setup);
            $Command = "$COPY_DIR \"$newinstallsetpath\" \"$dest_installdir\"";
            execute_Command ($Command, $error_setup, $show_Message, $command_withoutOutput);
            $Command = "hdiutil detach " . $detachpath;
            execute_Command ($Command, $error_setup, $show_Message, $command_withoutOutput);
            @DirArray = ();
            getSubFiles ("$dest_installdir", \@DirArray, "\\.app");
            if ($#DirArray == 0) {
                $optdir = "$dest_installdir" . $DirArray[0] . $PathSeparator;
                my $PListFile = "Info.plist";
                                my $officeDir = "$optdir" . "Contents" . $PathSeparator;
                my $programmDir = "$officeDir" . "MacOS" . $PathSeparator;
                $Command = "$COPY_FILE \"$officeDir$PListFile\" \"$programmDir$PListFile\"";
                execute_Command ($Command, $error_setup, $show_Message, $command_withoutOutput);
            }
            else {
                print_error ("Installation in $dest_installdir is incomplete", 2);
            }
        }
                else {
            print_error ("Plattform is not supported", 2);
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

    if (defined ($ENV{EPM}) && ($ENV{EPM} eq 'NO') && ($gui eq "UNX")) { # we do the install ourselves ...
        return ();
    }

    print "get Instset\n" if $is_debug;
    $NEWINSTSET = "";
    if (defined($smoketest_install)) {
        my $mask = "\\" . $PathSeparator . "\$";
        $smoketest_install =~ s/$mask//;
        my ($sufix);
        ($NEWINSTSET, $INSTSET, $sufix) = fileparse ($smoketest_install);
        return ($NEWINSTSET, $INSTSET);
    }
    if (!isLocalEnv() and !defined($ENV{CWS_WORK_STAMP}) and (-e $SHIP) and ($gui ne $cygwin)) {
        ($NEWINSTSET, $INSTSET) = getSetFromServer();
    }
    else {
        $InstDir="";
        $RootDir=$ENV{DMAKE_WORK_DIR};
        $RootDir=~s/\w+$//;
        foreach $project (@install_list) {
            @DirArray=();
            $TestDir1 = "$RootDir$project$PathSeparator$ENV{INPATH}$PathSeparator$PRODUCT$PathSeparator$packpackage$PathSeparator" . "install$PathSeparator";
            $TestDir2 = "$StandDir$project$PathSeparator$ENV{INPATH}$PathSeparator$PRODUCT$PathSeparator$packpackage$PathSeparator" . "install$PathSeparator";
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
            print "Lang-Sel: $INSTSET\n" if $is_command_infos;
;
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

sub isLocalEnv {
    my $returnvalue = 0;
    if (defined ($ENV{SOL_TMP}) && defined ($ENV{SOLARVERSION})) {
        my $mask = $ENV{SOL_TMP};
        $mask =~ s/\\/\\\\/;
        print "Mask: $mask\n" if $is_debug;
        if ($ENV{SOLARVERSION}=~ /$mask/) {
            $returnvalue = 1;
        }
    }
    return $returnvalue;
}

sub get_milestoneAndBuildID {
    my ( $ws, $pf ) = @_;
    my ($milestone, $buildid, $upd, $path, $updext);

    if ( $ws =~ /^\D+(\d+)$/) {
        $upd = $1;
    }

    if (defined ($ENV{UPDMINOREXT})) {
        $updext = $ENV{UPDMINOREXT};
    }
    else {
        $updext = "";
    }

    $path = "$ENV{SOLARVER}$PathSeparator$pf$PathSeparator" . "inc$updext$PathSeparator$upd" . "minor.mk";
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
            elsif ( /BUILD=(\d+)/ ) {
                $buildid = $1;
            }
        }

        close(MINORMK);
    }
    return ($milestone, $buildid);
}

sub get_productcode {
    my ( $installpath ) = @_;
    my ($path, $productcode);
    $productcode = "";
    $path = "$installpath" . "setup.ini";
    print "$path\n" if $is_debug;
    if ( !open(SETUP,$path) ) {
        print "FATAL: can't open $path\n" if ($is_command_infos);
        return ($productcode);
    }

    if (!eof(SETUP)) {
        while (<SETUP>) {
            chomp;
            if ( /productcode=(\{[\d\w-]+\})/ ) {
                $productcode = $1;
            }
        }

        close(SETUP);
    }
    return ($productcode);
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
    my (@DirArray, $mask, $buildid);
    $SetupFullPath = $PORDUCT;
    if ( ! ( $workspace && $platform ) ) {
        print_error ( "Error: environment not set correctly.", 1);
    }
    # get latest broadcastet milestone and pack number
    ($milestone, $buildid) = get_milestoneAndBuildID( $workspace, $platform );
    if (!defined($milestone)) {
            print_error ("Milestone ist not defined!", 2);
    }
    if (!defined($buildid)) {
            print_error ("Build-ID ist not defined!", 2);
    }

#    if ( $SetupFullPath =~ /^\/s.*\/install\// ) {
#        if ( $gui eq "UNX" ) {
#            $SetupFullPath = "/net/jumbo.germany" . $SetupFullPath;
#        } else {
#            $SetupFullPath = "\\\\jumbo" . $SetupFullPath;
#        }
#    }

#    my $ws_lc = lc $workspace;
    $mask = "^$workspace" . "_" . $milestone . "_native_packed-(\\d+)_en-US\\.$buildid";
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
    my ($lineend);

    if ($is_debug) {
        print "patching bootstrap $sourcefile ...\n";
        return 1;
    }

    $Error &= move ("$sourcefile", "$destfile");

    open OUTFILE, ">$sourcefile" or return errorFromOpen ($sourcefile);
    open INFILE, "<$destfile" or return errorFromOpen ($destfile);
    binmode(OUTFILE);
    binmode(INFILE);
    while(<INFILE>) {
        $line = $_;

        if ( $line =~ /UserInstallation/ ) {
            if ($line =~ /(\r\n)/) {
                $lineend = $1;
            }
            elsif ($line =~ /(\n)/) {
                $lineend = $1;
            }
            else {
                $lineend = $/;
            }
            @convert_split = split "=", $line;
            $line = $convert_split[0];
            $line .= "=";
            if ($gui eq $cygwin) {
                $line .= ConvertToFileURL(ConvertCygwinToWin($userinstallpath_without));
            }
            else {
                $line .= ConvertToFileURL($userinstallpath_without);
            }
            $line .= $lineend;
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

sub deinstallInstallation  {
    my ($installpath) = @_;
    my ($productcode);
    if ($gui eq "UNX") { return; }
    if (!$is_debug) {
        if (-e $installpath) {
            $productcode = get_productcode ($installpath);
            print "Productcode: $productcode\n" if ($is_command_infos);
            if ($productcode ne "") {
                print "deinstalling $productcode ...\n";
                $Command = "msiexec.exe -x $productcode -qn";
                execute_Command ($Command, $error_deinst, $show_Message, $command_withoutErrorcheck | $command_withoutOutput);
            }
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
    my ($Command, $Errorcode, $showMessage, $command_action) = @_;
    my ($Returncode, $output_ref);
    if (!$is_debug) {
        if ( ($command_action & $command_withoutOutput) == $command_withoutOutput) {
            ($Returncode, $output_ref) = execute_system ("$Command");
        }
        else {
            print "$Command\n" if $is_command_infos;
            $Returncode = system ("$Command");
        }
        if ($Returncode) {
            if ($showMessage) {
                if (($command_action & $command_withoutErrorcheck) == $command_withoutErrorcheck) {
                    print_warning ($error_messages[$Errorcode], $Errorcode);
                }
                else {
                    print_error ($error_messages[$Errorcode], $Errorcode);
                }
            }
            else {
                if (($command_action & $command_withoutErrorcheck) != $command_withoutErrorcheck) {
                    do_exit ($Errorcode);
                }
            }
        }
    }
    else {
        print "$Command\n";
    }
    return $output_ref;
}

sub execute_system {
    my ($command) = shift;
    my (@output_array, $line);
    if ( $is_command_infos ) {
        print STDERR "TRACE_SYSTEM: $command\n";
    }
    open( COMMAND, "$command 2>&1 |");
    while ($line = <COMMAND>) {
        chomp $line;
        push (@output_array, $line);
    }
    close(COMMAND);
    return $?, \@output_array;
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
    if ($is_do_deinstall) {
        deinstallInstallation ($installpath);
    }
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
