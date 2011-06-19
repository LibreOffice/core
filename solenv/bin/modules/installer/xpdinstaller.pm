#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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
#
#*************************************************************************
package installer::xpdinstaller;

use Cwd;
use installer::converter;
use installer::exiter;
use installer::globals;
use installer::languages;
use installer::logger;
use installer::pathanalyzer;
use installer::remover;
use installer::systemactions;


#######################################################
# Searching for the module name and description in the
# modules collector
#######################################################

sub get_module_name_description
{
    my ($modulesarrayref, $onelanguage, $gid, $type) = @_;

    my $found = 0;

    my $newstring = "";

    for ( my $i = 0; $i <= $#{$modulesarrayref}; $i++ )
    {
        my $onemodule = ${$modulesarrayref}[$i];

        if ( $onemodule->{'gid'} eq $gid )
        {
            my $typestring = $type . " " . "(" . $onelanguage . ")";
            if ( $onemodule->{$typestring} ) { $newstring = $onemodule->{$typestring}; }
            $found = 1;
        }

        if ( $found ) { last; }
    }

    # defaulting to english

    if ( ! $found )
    {
        my $defaultlanguage = "en-US";

        for ( my $i = 0; $i <= $#{$modulesarrayref}; $i++ )
        {
            my $onemodule = ${$modulesarrayref}[$i];

            if ( $onemodule->{'gid'} eq $gid )
            {
                my $typestring = $type . " " . "(" . $defaultlanguage . ")";
                if ( $onemodule->{$typestring} ) { $newstring = $onemodule->{$typestring}; }
                $found = 1;
            }

            if ( $found ) { last; }
        }
    }

    return $newstring;
}

###################################################
# Finding module, specified by the gid
###################################################

sub get_module
{
    my ($modulegid, $modulesarrayref) = @_;

    my $found = 0;
    my $searchmodule = "";

    for ( my $i = 0; $i <= $#{$modulesarrayref}; $i++ )
    {
        my $onemodule = ${$modulesarrayref}[$i];

        if ( $onemodule->{'gid'} eq $modulegid )
        {
            $searchmodule = $onemodule;
            $found = 1;
            last;
        }
    }

    return $searchmodule;
}

###################################################
# Creating package start tag
###################################################

sub get_package_tag
{
    my ( $module, $indent, $linkpackage ) = @_;

    my $modulegid = $module->{'gid'};
    if ( $linkpackage ) { $modulegid = $modulegid . "u"; }
    my $parentgid = "";
    if ( $module->{'ParentID'} ) { $parentgid = $module->{'ParentID'}; }
    if ( $parentgid eq "" ) { $parentgid = "root"; }
    if ( $module->{'XPDParentID'} ) { $parentgid = $module->{'XPDParentID'}; } # changing parent of "Prg" and "Opt" to "root"

    my $tag = $indent . "<package " . "name=" . "\"" . $modulegid . "\" " . "parent=" . "\"" . $parentgid . "\">" . "\n";

    return ( $tag, $parentgid );
}

###################################################
# Creating display start tag
###################################################

sub get_display_tag
{
    my ( $module, $indent ) = @_;

    # Styles=(HIDDEN_ROOT)
    my $styles = "";
    my $type = "";
    if ( $module->{'Styles'} ) { $styles = $module->{'Styles'}; }
    if ( $styles =~ /\bHIDDEN_ROOT\b/ ) { $type = "hidden"; }
    else { $type = "show"; }

    # special handling for language modules. Only visible in multilingual installation set.
    if (( $styles =~ /\bSHOW_MULTILINGUAL_ONLY\b/ ) && ( ! $installer::globals::ismultilingual )) { $type = "hidden"; }

    # special handling for the root module, which has no parent
    my $parentgid = "";
    if ( $module->{'ParentID'} ) { $parentgid = $module->{'ParentID'}; }
    if ( $parentgid eq "" ) { $type = "hidden"; }

    my $tag = $indent . "<display " . "type=" . "\"" . $type . "\"" . ">" . "\n";

    return $tag;
}

###################################################
# Creating installunit start tag
###################################################

sub get_installunit_tag
{
    my ( $indent ) = @_;

    my $type = $installer::globals::packageformat;

    my $tag = $indent . "<installunit " . "type=" . "\"" . $type . "\"" . ">" . "\n";

    return $tag;
}

###################################################
# Creating simple start tags
###################################################

sub get_start_tag
{
    my ( $tag, $indent ) = @_;

    my $starttag = $indent . "<" . $tag . ">" . "\n";
    return $starttag;
}

###################################################
# Creating end tags
###################################################

sub get_end_tag
{
    my ( $tag, $indent ) = @_;

    my $endtag = $indent . "</" . $tag . ">" . "\n";
    return $endtag;
}

###################################################
# Creating simple complete tag
###################################################

sub get_tag_line
{
    my ( $indent, $name, $value ) = @_;
    $value = '' unless defined $value;

    my $line = $indent . "<" . $name . ">" . $value . "</" . $name . ">" . "\n";

}

###################################################
# Asking module for sortkey entry
###################################################

sub get_sortkey_value
{
    my ( $module ) = @_;

    my $value = "9999";

    if ( $module->{'Sortkey'} ) { $value = $module->{'Sortkey'}; }

    return $value;
}

###################################################
# Asking module for default entry
###################################################

sub get_default_value
{
    my ( $module ) = @_;

    my $value = "";

    if ( $module->{'Default'} ) { $value = $module->{'Default'}; } # is YES or NO

    if ( $value =~ /\bNO\b/i ) { $value = "false"; }
    else { $value = "true"; }

    return $value;
}

###################################################
# Asking module for showinuserinstall entry
# scp style: DONTSHOWINUSERINSTALL
###################################################

sub get_showinuserinstall_value
{
    my ( $module ) = @_;

    my $value = "true";

    my $styles = "";
    if ( $module->{'Styles'} ) { $styles = $module->{'Styles'}; }
    if ( $styles =~ /\bDONTSHOWINUSERINSTALL\b/ ) { $value = "false"; }

    return $value;
}

###################################################
# Asking module for showinuserinstall entry
# scp style: USERINSTALLONLY
###################################################

sub get_userinstallonly_value
{
    my ( $module ) = @_;

    my $value = "false";

    my $styles = "";
    if ( $module->{'Styles'} ) { $styles = $module->{'Styles'}; }
    if ( $styles =~ /\bUSERINSTALLONLY\b/ ) { $value = "true"; }

    return $value;
}

###################################################
# Asking module for dontuninstall entry
# scp style: DONTUNINSTALL
###################################################

sub get_dontuninstall_value
{
    my ( $module ) = @_;

    my $value = "false";

    my $styles = "";
    if ( $module->{'Styles'} ) { $styles = $module->{'Styles'}; }
    if ( $styles =~ /\bDONTUNINSTALL\b/ ) { $value = "true"; }

    return $value;
}

###################################################
# Asking module for XpdCheckSolaris entry
# (belongs to scp module)
###################################################

sub get_checksolaris_value
{
    my ( $module ) = @_;

    my $value = "";
    if ( $module->{'XpdCheckSolaris'} ) { $value = $module->{'XpdCheckSolaris'}; }

    return $value;
}

###################################################
# Asking module for isupdatepackage entry
# scp style: ISUPDATEPACKAGE
###################################################

sub get_isupdatepackage_value
{
    my ( $module ) = @_;

    my $value = "false";

    my $styles = "";
    if ( $module->{'Styles'} ) { $styles = $module->{'Styles'}; }
    if ( $styles =~ /\bISUPDATEPACKAGE\b/ ) { $value = "true"; }

    return $value;
}

###################################################
# Asking module for showmultilingualonly entry
# scp style: SHOW_MULTILINGUAL_ONLY
###################################################

sub get_showmultilingualonly_value
{
    my ( $module ) = @_;

    my $value = "false";

    my $styles = "";
    if ( $module->{'Styles'} ) { $styles = $module->{'Styles'}; }
    if ( $styles =~ /\bSHOW_MULTILINGUAL_ONLY\b/ ) { $value = "true"; }

    return $value;
}

###################################################
# Asking module for showmultilingualonly entry
# scp style: SHOW_MULTILINGUAL_ONLY
###################################################

sub get_applicationmodule_value
{
    my ( $module ) = @_;

    my $value = "false";

    my $styles = "";
    if ( $module->{'Styles'} ) { $styles = $module->{'Styles'}; }
    if ( $styles =~ /\bAPPLICATIONMODULE\b/ ) { $value = "true"; }

    return $value;
}

###################################################
# Asking module for java module entry
# scp style: JAVAMODULE
###################################################

sub get_isjavamodule_value
{
    my ( $module ) = @_;

    my $value = "false";

    my $styles = "";
    if ( $module->{'Styles'} ) { $styles = $module->{'Styles'}; }
    if ( $styles =~ /\bJAVAMODULE\b/ ) { $value = "true"; }

    return $value;
}

#####################################################################
# Asking module, if installation shall use --force
# scp style: USEFORCE  (Linux only)
#####################################################################

sub get_useforce_value
{
    my ( $module ) = @_;

    my $value = "false";

    my $styles = "";
    if ( $module->{'Styles'} ) { $styles = $module->{'Styles'}; }
    if ( $styles =~ /\bUSEFORCE\b/ ) { $value = "true"; }

    return $value;
}

###################################################
# Asking module, if installation can fail
# scp style: INSTALLCANFAIL
###################################################

sub get_installcanfail_value
{
    my ( $module ) = @_;

    my $value = "false";

    my $styles = "";
    if ( $module->{'Styles'} ) { $styles = $module->{'Styles'}; }
    if ( $styles =~ /\bINSTALLCANFAIL\b/ ) { $value = "true"; }

    return $value;
}

###################################################
# Asking module, if installation can fail
# scp style: INSTALLCANFAIL
###################################################

sub get_forceintoupdate_value
{
    my ( $module ) = @_;

    my $value = "false";

    my $styles = "";
    if ( $module->{'Styles'} ) { $styles = $module->{'Styles'}; }
    if ( $styles =~ /\bFORCEINTOUPDATE\b/ ) { $value = "true"; }

    return $value;
}

###################################################
# Substituting all occurrences of "<" by "&lt;"
# and all occurrences of ">" by "&gt;"
###################################################

sub replace_brackets_in_string
{
    my ( $string ) = @_;

    if ( $string =~ /\</ ) { $string =~ s/\</\&lt\;/g; }
    if ( $string =~ /\>/ ) { $string =~ s/\>/\&gt\;/g; }

    return $string;
}

###################################################
# Substituting all occurrences of "\uUXYZ" by
# "&#xUXYZ;", because the use xml saxparser does
# not know anything about this encoding. Therfore
# the xml file can keep standard encoding "UTF-8"
# and all strings with "\uUXYZ" do not need to
# be converted from the Java installer.
###################################################

sub replace_javaencoding_in_string
{
    my ( $string ) = @_;

    while ( $string =~ /(\\u\w\w\w\w)/ )
    {
        my $oldvalue = $1;
        my $newvalue = "";
        if ( $oldvalue =~ /\\u(\w\w\w\w)/ )
        {
            my $number = $1;
            $newvalue = "&#x" . $number . ";";
        }

        $string =~ s/\Q$oldvalue\E/$newvalue/;
    }

    return $string;
}

###################################################
# Collecting language dependent entries from scp
# (Name and Description)
###################################################

sub collect_lang_values
{
    my ($indent, $module, $xpdfile, $searchentry, $saveentry) = @_;

    foreach $key (keys %{$module})
    {
        my $write_line = 0;
        my $javalanguage = "";

        if ( $key =~ /^\s*\Q$searchentry\E\s+\((\S+)\)\s*$/ )   # this are the language dependent keys
        {
            $language = $1;
            $javalanguage = installer::languages::get_java_language($language);
            $write_line = 1;
        }
        elsif ( $key =~ /^\s*\Q$searchentry\E\s*$/ )    # this are the language independent keys
        {
            $javalanguage = "en_US";
            $write_line = 1;
        }

        if ( $write_line )
        {
            my $value = $module->{$key};
            $value = replace_brackets_in_string($value);
            $value = replace_javaencoding_in_string($value);
            my $line = $indent . "<" . $saveentry . " lang=" . "\"" . $javalanguage . "\"" . ">" . $value . "<\/" . $saveentry . ">" . "\n";
            push(@{$xpdfile}, $line);
        }
    }
}

###################################################
# Removing language dependent entries from
# module hash (Name and Description)
###################################################

sub remove_lang_values
{
    my ($module, $searchentry) = @_;

    my $key = "";

    foreach $key (keys %{$module})
    {
        if ( $key =~ /^\s*\Q$searchentry\E\s+\((\S+)\)\s*$/ )   # this are the language dependent keys
        {
            delete($module->{$key});
        }
    }
}

###################################################
# Setting package install order
###################################################

sub get_order_value
{
    my ( $module ) = @_;

    my $value = "1000"; # Setting the default value

    if ( $module->{'InstallOrder'} ) { $value = $module->{'InstallOrder'}; }

    return $value;
}

###################################################
# Checking size of package
###################################################

sub get_size_value
{
    my ( $packagename, $xpdinfo ) = @_;

    my $value = "";

    if ( $xpdinfo->{'FileSize'} )
    {
        $value =  $xpdinfo->{'FileSize'};
        return $value;
    }

    my $isrpmfile = 0;
    if ( $packagename =~ /\.rpm\s*$/ ) { $isrpmfile = 1; }

    if (( $installer::globals::isrpmbuild ) && ( $isrpmfile ))
    {
        if ( ! $installer::globals::rpmquerycommand ) { $installer::globals::rpmquerycommand = "rpm"; }

        my $systemcall = "$installer::globals::rpmquerycommand -qp --queryformat \"\[\%\{FILESIZES\}\\n\]\" $packagename 2\>\&1 |";
        my $ld_library_backup = $ENV{LD_LIBRARY_PATH};
        if ( defined $ENV{SYSBASE}) {
            my $sysbase = $ENV{SYSBASE};
            if ( !defined ($ld_library_backup) or ("$ld_library_backup" eq "") ) {
                $ld_library_backup = "" if ! defined $ld_library_backup;
                $ENV{LD_LIBRARY_PATH} = "$sysbase/usr/lib";
            } else {
                $ENV{LD_LIBRARY_PATH} = "$ld_library_backup:$sysbase/lib";
            }
        }
        my ($rpmout, $error) = make_systemcall_allowing_error($systemcall, 0, 1);
        $ENV{LD_LIBRARY_PATH} = $ld_library_backup;
        # Evaluating an error, because of rpm problems with removed LD_LIBRARY_PATH
        if ( $error )
        {
            installer::logger::print_message( "... trying /usr/bin/rpm ...\n" );
            my $systemcall = "/usr/bin/rpm -qp --queryformat \"\[\%\{FILESIZES\}\\n\]\" $packagename 2\>\&1 |";
            ($rpmout, $error) = make_systemcall_allowing_error($systemcall, 0, 0);
            if ( $error ) { installer::exiter::exit_program("ERROR: rpm failed to query package!", "get_size_value"); }
        }
        $value = do_sum($rpmout);       # adding all filesizes in bytes
        $value = $value/1000;

        my $ganzzahl = int $value;
        if ($ganzzahl < $value) { $value = $ganzzahl + 1; }
        else { $value = $ganzzahl; }

        my $rpmname = $packagename;
        installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$rpmname);
        $infoline = "Filesize $rpmname : $value\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    if ( $installer::globals::issolarispkgbuild )
    {
        my $filename = "pkgmap";
        $filename = $packagename . $installer::globals::separator . $filename;
        $file = installer::files::read_file($filename);

        for ( my $i = 0; $i <= $#{$file}; $i++ )
        {
            my $line = ${$file}[$i];
            if ( $line =~ /^\s*\:\s+\d+\s+(\d+?)\s+/ )
            {
                $value = $1;
                if ( ! ( $value%2 == 0 )) { $value = $value + 1; }
                $value = $value/2;      # not blocks, but kB
                last;
            }
        }
    }

    if ( $value eq "" ) { $value = "0"; }

    return $value;
}

###################################################
# Checking md5 of package
###################################################

sub get_md5_value
{
    my ( $packagename, $xpdinfo ) = @_;

    my $value = "";

    if ( $xpdinfo->{'md5sum'} )
    {
        $value =  $xpdinfo->{'md5sum'};
        return $value;
    }

    if ( $installer::globals::isrpmbuild )
    {
        my $md5file = "/usr/bin/md5sum";

        if ( -x $md5file )
        {
            my $systemcall = "$md5file $packagename 2\>\&1 |";
            my $md5out = make_systemcall($systemcall, 1);
            $value = ${$md5out}[0];
            if ( $value =~ /^\s*(\S+?)\s+.*$/ )
            {
                $value = $1;
            }

            my $rpmname = $packagename;
            installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$rpmname);
            $infoline = "md5sum of $rpmname : $value\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }

    return $value;
}

###################################################
# Checking name of package
###################################################

sub get_name_value
{
    my ( $packagename ) = @_;

    my $value = $packagename;

    # $packagename contains the complete path to the package
    # Only the name of file or directory is required

    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$value);

    return $value;
}

###################################################
# Checking full package name (Linux only)
###################################################

sub get_fullpkgname_value
{
    my ( $packagename, $xpdinfo ) = @_;

    my $value = "";
    my $isrpmfile = 0;
    if ( $packagename =~ /\.rpm\s*$/ ) { $isrpmfile = 1; }

    if (( $installer::globals::isrpmbuild ) && ( $isrpmfile ))
    {
        if ( $xpdinfo->{'FullPackageName'} )
        {
            $value =  $xpdinfo->{'FullPackageName'};
            return $value;
        }

        if ( ! $installer::globals::rpmquerycommand ) { $installer::globals::rpmquerycommand = "rpm"; }
        my $systemcall = "$installer::globals::rpmquerycommand -qp $packagename |";
        my $ld_library_backup = $ENV{LD_LIBRARY_PATH};
        if ( defined $ENV{SYSBASE}) {
            my $sysbase = $ENV{SYSBASE};
            if ( !defined ($ld_library_backup) or ("$ld_library_backup" eq "") ) {
                $ld_library_backup = "" if ! defined $ld_library_backup;
                $ENV{LD_LIBRARY_PATH} = "$sysbase/usr/lib";
            } else {
                $ENV{LD_LIBRARY_PATH} = "$ld_library_backup:$sysbase/lib";
            }
        }
        my ($returnarray, $error) = make_systemcall_allowing_error($systemcall, 0, 1);
        $ENV{LD_LIBRARY_PATH} = $ld_library_backup;
        # Evaluating an error, because of rpm problems with removed LD_LIBRARY_PATH
        if ( $error )
        {
            installer::logger::print_message( "... trying /usr/bin/rpm ...\n" );
            my $systemcall = "/usr/bin/rpm -qp $packagename |";
            ($returnarray, $error) = make_systemcall_allowing_error($systemcall, 0, 0);
            if ( $error ) { installer::exiter::exit_program("ERROR: rpm failed to query package!", "get_fullpkgname_value"); }
        }
        $value = ${$returnarray}[0];
        installer::remover::remove_leading_and_ending_whitespaces(\$value);

        my $rpmname = $packagename;
        installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$rpmname);

        $infoline = "Full package name from $rpmname: $value\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    return $value;
}

###################################################
# Checking package version (Solaris only)
###################################################

sub get_pkgversion_value
{
    my ( $completepackagename, $xpdinfo ) = @_;

    my $value = "";

    if ( $xpdinfo->{'PkgVersion'} )
    {
        $value =  $xpdinfo->{'PkgVersion'};
        return $value;
    }

    if ( $installer::globals::issolarispkgbuild )
    {
        my $pkgfile = "pkgparam";
        my $packagepath = $completepackagename;
        installer::pathanalyzer::get_path_from_fullqualifiedname(\$packagepath);

        my $packagename = $completepackagename;
        installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$packagename);

        my $systemcall = "$pkgfile -d $packagepath $packagename param VERSION 2\>\&1 |";
        my $returnarray = make_systemcall($systemcall, 0);

        $value = ${$returnarray}[0];
        installer::remover::remove_leading_and_ending_whitespaces(\$value);
    }

    return $value;
}

###################################################
# Writing subdirectory into xpd file
###################################################

sub get_subdir_value
{
    my ( $packagename, $subdir, $module ) = @_;

    my $value = "";

    if ( $subdir ) { $value = $subdir; }

    if ( $module->{'Subdir'} ) { $value = $module->{'Subdir'}; }

    return $value;
}

###################################################
# Checking if package is relocatable
###################################################

sub get_relocatable_value
{
    my ( $module ) = @_;

    my $value = "true";

    my $styles = "";
    if ( $module->{'Styles'} ) { $styles = $module->{'Styles'}; }
    if ( $styles =~ /\bNOTRELOCATABLE\b/ ) { $value = "false"; }

    return $value;
}

###################################################
# Checking if package is relocatable
###################################################

sub get_languagespecific_value
{
    my ( $islanguagemodule ) = @_;

    my $value = "false";

    if ( defined $islanguagemodule && $islanguagemodule == 1 ) { $value = "true"; }

    return $value;
}

#######################################################
# Adding the values of the array
#######################################################

sub do_sum
{
    my ( $allnumbers ) = @_;

    my $sum = 0;

    for ( my $i = 0; $i <= $#{$allnumbers}; $i++ )
    {
        $sum = $sum + ${$allnumbers}[$i];
    }

    return $sum;
}

#######################################################
# Executing one system call
#######################################################

sub make_systemcall
{
    my ( $systemcall, $logreturn ) = @_;

    my @returns = ();

    installer::logger::print_message( "... $systemcall ...\n" );

    open (REG, "$systemcall");
    while (<REG>) {push(@returns, $_); }
    close (REG);

    my $returnvalue = $?;   # $? contains the return value of the systemcall

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ( $logreturn )
    {
        for ( my $j = 0; $j <= $#returns; $j++ ) { push( @installer::globals::logfileinfo, "$returns[$j]"); }
    }

    if ($returnvalue)
    {
        $infoline = "ERROR: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);
        $error_occurred = 1;
    }
    else
    {
        $infoline = "SUCCESS: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    return \@returns;
}

#######################################################
# Executing one system call
#######################################################

sub make_systemcall_allowing_error
{
    my ( $systemcall, $logreturn, $can_fail ) = @_;

    my @returns = ();

    installer::logger::print_message( "... $systemcall ...\n" );

    open (REG, "$systemcall");
    while (<REG>) {push(@returns, $_); }
    close (REG);

    my $returnvalue = $?;   # $? contains the return value of the systemcall

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ( $logreturn )
    {
        for ( my $j = 0; $j <= $#returns; $j++ ) { push( @installer::globals::logfileinfo, "$returns[$j]"); }
    }

    if ($returnvalue)
    {
        if ( $can_fail )
        {
            $infoline = "WARNING: Failed system call:  $systemcall\n";
            push( @installer::globals::logfileinfo, $infoline);
            $error_occurred = 1;
        }
        else
        {
            $infoline = "ERROR: $systemcall\n";
            push( @installer::globals::logfileinfo, $infoline);
            $error_occurred = 1;
        }
    }
    else
    {
        $infoline = "SUCCESS: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    return (\@returns, $returnvalue);
}

###################################################
# Setting product name tag
###################################################

sub get_product_tag
{
    my ($allvariables, $indent) = @_;

    my $productname = $allvariables->{'LCONEWORDPRODUCTNAME'};
    my $tag = $indent . "<product " . "name=" . "\"" . $productname . "\">" . "\n";

    return $tag;
}

###################################################
# Macro tags
###################################################

sub set_macro_tag
{
    my ($allvariables, $indent, $key) = @_;

    my $property = "";
    my $value = "";

    if ( $key eq "product_name" ) { $property = "PRODUCTNAME"; }
    elsif ( $key eq "product_version" ) { $property = "PRODUCTVERSION"; }
    elsif ( $key eq "product_suffix" ) { $property = "PRODUCTEXTENSION"; }
    elsif ( $key eq "product_fullname" ) { $property = "FULLPRODUCTNAME"; }

    if (( $property eq "PRODUCTNAME" ) || ( $property eq "PRODUCTVERSION" ) || ( $property eq "PRODUCTEXTENSION" ))
    {
        $value = $allvariables->{$property};
    }

    if ( $property eq "FULLPRODUCTNAME" )
    {
        $value = $allvariables->{"PRODUCTNAME"} . " " . $allvariables->{"PRODUCTVERSION"};
        if ( $allvariables->{"PRODUCTEXTENSION"} ) { $value = $value . " " . $allvariables->{"PRODUCTEXTENSION"}; }
    }

    my $tag = $indent . "<macro " . "key=" . "\"" . $key . "\">" . $value . "\<\/macro\>" . "\n";

    return $tag;

}

###################################################
# Setting the minor of the product version
# Required to check for Major Upgrades.
###################################################

sub set_minor_tag
{
    my ($allvariables, $indent) = @_;

    my $productminor = 0;
    if ( $allvariables->{"PACKAGEVERSION"} )
    {
        if ( $allvariables->{"PACKAGEVERSION"} =~ /^\s*\d+\.(\d+)/ ) { $productminor = $1; }
    }
    my $tag = $indent . "<productminor>" . $productminor . "</productminor>" . "\n";

    return $tag;
}

###################################################
# Setting the update behaviour
###################################################

sub set_update_tag
{
    my ($allvariables, $indent) = @_;

    my $updateflag = "false";
    if ( $allvariables->{"DONTUPDATE"} ) { $updateflag = "true"; }
    my $tag = $indent . "<dontupdate>" . $updateflag . "</dontupdate>" . "\n";

    return $tag;
}

###################################################
# Setting the license dialog behaviour
###################################################

sub set_hideeula_tag
{
    my ($allvariables, $indent) = @_;

    my $hidelicenseflag = "false";
    if ( $allvariables->{"HIDELICENSEDIALOG"} ) { $hidelicenseflag = "true"; }
    my $tag = $indent . "<hideeula>" . $hidelicenseflag . "</hideeula>" . "\n";

    return $tag;
}

###################################################
# Setting default directory
###################################################

sub set_defaultdir_tag
{
    my ($allvariables, $indent) = @_;

    my $defaultdir = "";
    if ( $allvariables->{"DEFAULTDESTPATH"} ) { $defaultdir = $allvariables->{"DEFAULTDESTPATH"}; }
    my $tag = $indent . "<defaultdir>" . $defaultdir . "</defaultdir>" . "\n";

    return $tag;
}

###################################################
# Setting product directory
###################################################

sub set_productdir_tag
{
    my ($allvariables, $indent) = @_;

    my $productdir = "";
    if ( $allvariables->{"UNIXPRODUCTNAME"} )
    {
        $productdir = $allvariables->{"UNIXPRODUCTNAME"};

        if ( $allvariables->{"BRANDPACKAGEVERSION"} )
        {
            $productdir = $productdir . $allvariables->{"BRANDPACKAGEVERSION"};
        }
        else
        {
            if ( $allvariables->{"PRODUCTVERSION"} )
            {
                $productdir = $productdir . $allvariables->{"PRODUCTVERSION"};
            }
        }
    }
    my $tag = $indent . "<productdir>" . $productdir . "</productdir>" . "\n";

    return $tag;
}

#####################################################
# Setting the package directory in installation set
#####################################################

sub set_packagedir_tag
{
    my ($indent) = @_;

    my $tag = $indent . "<packagedirectory>" . $installer::globals::epmoutpath . "</packagedirectory>" . "\n";

    return $tag;
}

###################################################
# Setting the packagetype of installation set
###################################################

sub set_packagetype_tag
{
    my ($indent) = @_;

    my $tag = $indent . "<packageformat>" . $installer::globals::packageformat . "</packageformat>" . "\n";

    return $tag;
}

###################################################
# Setting the architecture of installation set
###################################################

sub set_architecture_tag
{
    my ($indent) = @_;

    my $architecture = "";
    if ( $installer::globals::issolarissparcbuild ) { $architecture = "sparc"; }
    if ( $installer::globals::issolarisx86build ) { $architecture = "i386"; }

    my $tag = $indent . "<architecture>" . $architecture . "</architecture>" . "\n";

    return $tag;
}

###################################################
# Setting the multi language tag
###################################################

sub set_multilanguage_tag
{
    my ($indent) = @_;

    my $value = "false";
    if ( $installer::globals::ismultilingual == 1 ) { $value = "true"; }

    my $tag = $indent . "<multilingual>" . $value . "</multilingual>" . "\n";

    return $tag;
}

###################################################
# Setting the language tag
###################################################

sub set_language_tag
{
    my ($languagestringref, $indent) = @_;

    my $tag = $indent . "<languages>" . $$languagestringref . "</languages>" . "\n";

    return $tag;
}

###################################################
# Collecting content for product xpd file
###################################################

# <?xml version='1.0' encoding='utf-8'?>
#
# <!-- General application description -->
#
# <product name="openoffice">
#     <macro key="product_name">Sun OpenOffice.org</macro>
#     <macro key="product_version">1.0</macro>
#     <macro key="product_suffix">Mephisto</macro>
#     <macro key="product_fullname">Sun OpenOffice.org 1.0 Mephisto</macro>
#     <defaultdir>/opt/Sun/OpenOffice.org-Mephisto</defaultdir>
# </product>

sub get_setup_file_content
{
    my ($allvariables, $languagestringref) = @_;

    my @xpdfile = ();
    my $noindent = "";
    my $singleindent = "    ";

    my $line = "<?xml version='1.0' encoding='utf-8'?>\n\n";
    push(@xpdfile, $line);
    $line = "<!-- General application description -->\n\n";
    push(@xpdfile, $line);

    my $tag = get_product_tag($allvariables, $noindent);
    push(@xpdfile, $tag);

    $tag = set_macro_tag($allvariables, $singleindent, "product_name");
    push(@xpdfile, $tag);
    $tag = set_macro_tag($allvariables, $singleindent, "product_version");
    push(@xpdfile, $tag);
    $tag = set_macro_tag($allvariables, $singleindent, "product_suffix");
    push(@xpdfile, $tag);
    $tag = set_macro_tag($allvariables, $singleindent, "product_fullname");
    push(@xpdfile, $tag);

    $tag = set_defaultdir_tag($allvariables, $singleindent);
    push(@xpdfile, $tag);

    $tag = set_productdir_tag($allvariables, $singleindent);
    push(@xpdfile, $tag);

    $tag = set_minor_tag($allvariables, $singleindent);
    push(@xpdfile, $tag);

    $tag = set_update_tag($allvariables, $singleindent);
    push(@xpdfile, $tag);

    $tag = set_packagedir_tag($singleindent);
    push(@xpdfile, $tag);

    $tag = set_packagetype_tag($singleindent);
    push(@xpdfile, $tag);

    $tag = set_architecture_tag($singleindent);
    push(@xpdfile, $tag);

    $tag = set_multilanguage_tag($singleindent);
    push(@xpdfile, $tag);

    $tag = set_language_tag($languagestringref, $singleindent);
    push(@xpdfile, $tag);

    $tag = set_hideeula_tag($allvariables, $singleindent);
    push(@xpdfile, $tag);

    $tag = get_end_tag("product", $noindent);
    push(@xpdfile, $tag);

    return \@xpdfile;
}

###################################################
# Collecting content for xpd file
###################################################

sub get_file_content
{
    my ( $module, $packagename, $solslanguage, $linkpackage, $isemptyparent, $subdir, $islanguagemodule, $onelanguage, $xpdinfo ) = @_;

    my @xpdfile = ();
    my $value = "";
    my $line = "";
    my $noindent = "";
    my $singleindent = "    ";
    my $doubleindent = $singleindent . $singleindent;

    my ( $tag, $parentgid ) = get_package_tag($module, $noindent, $linkpackage);
    push(@xpdfile, $tag);

    # start of installunit tag -> using info from scp module

    $tag = get_display_tag($module, $singleindent);
    push(@xpdfile, $tag);

    $value = get_sortkey_value($module);
    $line = get_tag_line($doubleindent, "sortkey", $value);
    push(@xpdfile, $line);

    $value = get_default_value($module);
    $line = get_tag_line($doubleindent, "default", $value);
    push(@xpdfile, $line);

    $value = get_showinuserinstall_value($module);
    $line = get_tag_line($doubleindent, "showinuserinstall", $value);
    push(@xpdfile, $line);

    $value = get_userinstallonly_value($module);
    $line = get_tag_line($doubleindent, "showinuserinstallonly", $value);
    push(@xpdfile, $line);

    $value = get_dontuninstall_value($module);
    $line = get_tag_line($doubleindent, "dontuninstall", $value);
    push(@xpdfile, $line);

    $value = get_checksolaris_value($module);
    $line = get_tag_line($doubleindent, "checksolaris", $value);
    push(@xpdfile, $line);

    $value = get_isupdatepackage_value($module);
    $line = get_tag_line($doubleindent, "isupdatepackage", $value);
    push(@xpdfile, $line);

    $value = get_showmultilingualonly_value($module);
    $line = get_tag_line($doubleindent, "showmultilingualonly", $value);
    push(@xpdfile, $line);

    $value = get_applicationmodule_value($module);
    $line = get_tag_line($doubleindent, "applicationmodule", $value);
    push(@xpdfile, $line);

    $value = get_isjavamodule_value($module);
    $line = get_tag_line($doubleindent, "isjavapackage", $value);
    push(@xpdfile, $line);

    $value = get_installcanfail_value($module);
    $line = get_tag_line($doubleindent, "installcanfail", $value);
    push(@xpdfile, $line);

    $value = get_forceintoupdate_value($module);
    $line = get_tag_line($doubleindent, "forceintoupdate", $value);
    push(@xpdfile, $line);

    $value = get_useforce_value($module);
    $line = get_tag_line($doubleindent, "useforce", $value);
    push(@xpdfile, $line);

    # iterating over all languages to get names and descriptions
    collect_lang_values($doubleindent, $module, \@xpdfile, "Name", "name");
    collect_lang_values($doubleindent, $module, \@xpdfile, "Description", "description");

    $tag = get_end_tag("display", $singleindent);
    push(@xpdfile, $tag);

    # end of display tag

    if ( ! $isemptyparent )
    {
        # start of installunit tag -> using info from package defined in packagelist

        $tag = get_installunit_tag($singleindent);
        push(@xpdfile, $tag);

        $value = get_size_value($packagename, $xpdinfo);
        $line = get_tag_line($doubleindent, "size", $value);
        push(@xpdfile, $line);

        $value = get_order_value($module);
        $line = get_tag_line($doubleindent, "installorder", $value);
        push(@xpdfile, $line);

        $value = get_md5_value($packagename, $xpdinfo);
        $line = get_tag_line($doubleindent, "md5", $value);
        push(@xpdfile, $line);

        $value = get_name_value($packagename);
        $line = get_tag_line($doubleindent, "name", $value);
        push(@xpdfile, $line);

        $value = get_fullpkgname_value($packagename, $xpdinfo);
        $line = get_tag_line($doubleindent, "fullpkgname", $value);
        push(@xpdfile, $line);

        $value = get_pkgversion_value($packagename, $xpdinfo);
        $line = get_tag_line($doubleindent, "pkgversion", $value);
        push(@xpdfile, $line);

        $value = get_subdir_value($packagename, $subdir, $module);
        $line = get_tag_line($doubleindent, "subdir", $value);
        push(@xpdfile, $line);

        $value = get_relocatable_value($module);
        $line = get_tag_line($doubleindent, "relocatable", $value);
        push(@xpdfile, $line);

        $value = get_languagespecific_value($islanguagemodule);
        $line = get_tag_line($doubleindent, "languagespecific", $value);
        push(@xpdfile, $line);

        $value = $onelanguage;
        $line = get_tag_line($doubleindent, "language", $value);
        push(@xpdfile, $line);

        $line = get_tag_line($doubleindent, "solarislanguage", $solslanguage);
        push(@xpdfile, $line);

        $tag = get_end_tag("installunit", $singleindent);
        push(@xpdfile, $tag);

        # end of installunit tag
    }

    $tag = get_end_tag("package", $noindent);
    push(@xpdfile, $tag);

    return ( \@xpdfile, $parentgid );
}

###################################################
# Setting xpd file name
###################################################

sub get_xpd_filename
{
    my ($modulegid, $linkpackage) = @_;

    if ( $linkpackage ) { $modulegid = $modulegid . "u"; }

    my $filename = $modulegid . ".xpd";

    return $filename;
}

###################################################
# Determine, which package was created newly
###################################################

sub determine_new_packagename
{
    my ( $installdir, $subdir, $xpdinfo ) = @_;

    my $newpackage = "";
    $installdir =~ s/\Q$installer::globals::separator\E\s*$//;
    my $directory = $installdir . $installer::globals::separator . $subdir;
    $directory =~ s/\Q$installer::globals::separator\E\s*$//;

    if ( $xpdinfo->{'RealPackageName'} )
    {
        $newpackage = $directory . $installer::globals::separator . $xpdinfo->{'RealPackageName'};
        push(@installer::globals::currentcontent, $newpackage);
        return $newpackage;
    }

    my ($newcontent, $allcontent) = installer::systemactions::find_new_content_in_directory($directory, \@installer::globals::currentcontent);
    @installer::globals::currentcontent = ();
    foreach my $element ( @{$allcontent} ) { push(@installer::globals::currentcontent, $element); }

    my $newentriesnumber = $#{$newcontent} + 1;
    if ( $newentriesnumber > 1 ) { installer::exiter::exit_program("ERROR: More than one new package in directory $directory", "determine_new_packagename (xpdinstaller)"); }
    elsif ( $newentriesnumber < 1 )  { installer::exiter::exit_program("ERROR: No new package in directory $directory", "determine_new_packagename (xpdinstaller)"); }
    $newpackage = ${$newcontent}[0];

    return $newpackage;
}

###################################################
# Checking, if the parentgid is defined in
# another package
###################################################

sub is_empty_parent
{
    my ($gid, $packages) = @_;

    my $is_empty_parent = 1;

    for ( my $k = 0; $k <= $#{$packages}; $k++ )
    {
        my $onepackage = ${$packages}[$k];
        my $packagegid = $onepackage->{'module'};

        if ( $packagegid eq $gid )
        {
            $is_empty_parent = 0;
            last;
        }
    }

    return $is_empty_parent;
}

###################################################
# Creating additional xpd files for empty parents
###################################################

sub create_emptyparents_xpd_file
{
    my ($parentgid, $modulesarrayref, $xpddir) = @_;

    my $module = get_module($parentgid, $modulesarrayref);
    my $grandpagid = "";

    if ( $module ne "" )
    {
        my $packagename = "";
        # all content saved in scp is now available and can be used to create the xpd file
        my ( $xpdfile, $newparentgid ) = get_file_content($module, $packagename, "", 0, 1, "", 0, "", "");

        $grandpagid = $newparentgid;

        my $xpdfilename = get_xpd_filename($parentgid, 0);
        $xpdfilename = $xpddir . $installer::globals::separator . $xpdfilename;

        installer::files::save_file($xpdfilename, $xpdfile);
        push(@installer::globals::allxpdfiles, $xpdfilename);
        my $infoline = "Saving xpd file: $xpdfilename\n";
        push(@installer::globals::logfileinfo, $infoline);
    }

    push( @installer::globals::createdxpdfiles, $parentgid);

    return $grandpagid;
}

###################################################
# Creating additional xpd files for empty parents
###################################################

sub filter_content_from_xpdfile
{
    my ($xpdfile) = @_;

    my @newxpdfile = ();

    my $include = 1;

    for ( my $i = 0; $i <= $#{$xpdfile}; $i++ )
    {
        my $line = ${$xpdfile}[$i];

        if (( $line =~ /^\s*\<installunit/ ) && ( $include )) { $include = 0; }
        if ( $include ) { push(@newxpdfile, $line); }
        if (( $line =~ /^\s*\<\/installunit/ ) && ( ! $include )) { $include = 1; }
    }

    return \@newxpdfile;
}

##########################################################################
# Changing the parent inside the xpd file
# Old: <package name="gid_Module_Root" parent="root">
# New: <package name="gid_Module_Root_Files_1" parent="gid_Module_Root">
##########################################################################

sub change_parent_in_xpdfile
{
    my ($xpdfile, $modulename) = @_;

    for ( my $i = 0; $i <= $#{$xpdfile}; $i++ )
    {
        if ( ${$xpdfile}[$i] =~ /^\s*\<package name\s*=\s*\"(\S+?)\"\s+parent\s*=\s*\"(\S+?)\"\s*\>\s*$/ )
        {
            my $oldname = $1;
            my $oldparent = $2;

            my $newname = $modulename;
            my $newparent = $oldname;

            ${$xpdfile}[$i] =~ s/\"\Q$oldname\E\"/\"$newname\"/;
            ${$xpdfile}[$i] =~ s/\"\Q$oldparent\E\"/\"$newparent\"/;

            last;
        }
    }
}

###################################################
# Creating one xpd file for each package
###################################################

sub create_xpd_file
{
    my ($onepackage, $allpackages, $languagestringref, $allvariables, $modulesarrayref, $installdir, $subdir, $linkpackage, $xpdinfo) = @_;

    my $infoline = "";
    # creating the directory
    my $xpddir = installer::systemactions::create_directories("xpdinstaller", $languagestringref);
    $xpddir =~ s/\/\s*$//;
    $installer::globals::xpddir = $xpddir;

    my $modulegid = $onepackage->{'module'};

    my $onelanguage = "";   #
    my $solslanguage = "";  #
    my $islanguagemodule = 0;   #
    if ( $onepackage->{'islanguagemodule'} ) { $islanguagemodule = $onepackage->{'islanguagemodule'}; } #
    if ( $islanguagemodule )    #
    {
        $onelanguage = $onepackage->{'language'};   #
        if ( $installer::globals::issolarispkgbuild ) { $solslanguage = installer::epmfile::get_solaris_language_for_langpack($onelanguage); }  #
    }

    installer::logger::include_header_into_logfile("Creating xpd file ($modulegid):");

    my $module = get_module($modulegid, $modulesarrayref);

    if ( $module ne "" )
    {
        my $packagename = determine_new_packagename($installdir, $subdir, $xpdinfo);

        # all content saved in scp is now available and can be used to create the xpd file
        my ( $xpdfile, $parentgid ) = get_file_content($module, $packagename, $solslanguage, $linkpackage, 0, "", $islanguagemodule, $onelanguage, $xpdinfo);

        my $xpdfilename = get_xpd_filename($modulegid, $linkpackage);
        $xpdfilename = $xpddir . $installer::globals::separator . $xpdfilename;

        # Very special handling for Root module:
        # Because packages should only be assigned to leaves and not to knods,
        # the root module is divided into a knod without package and a new
        # leave with package. The name of the leave is defined at $module->{'XpdPackageName'}.
        if ( $module->{'XpdPackageName'} )
        {
            my $newxpdfilename = get_xpd_filename($module->{'XpdPackageName'}, 0);
            $newxpdfilename = $xpddir . $installer::globals::separator . $newxpdfilename;
            my $emptyfilecontent = filter_content_from_xpdfile($xpdfile);

            installer::files::save_file($xpdfilename, $emptyfilecontent);
            push(@installer::globals::allxpdfiles, $xpdfilename);
            $infoline = "Saving xpd file: $xpdfilename\n";
            push( @installer::globals::logfileinfo, $infoline);

            $xpdfilename = $newxpdfilename;
            change_parent_in_xpdfile($xpdfile, $module->{'XpdPackageName'});
        }

        installer::files::save_file($xpdfilename, $xpdfile);
        push( @installer::globals::createdxpdfiles, $modulegid);
        push(@installer::globals::allxpdfiles, $xpdfilename);
        $infoline = "Saving xpd file: $xpdfilename\n";
        push( @installer::globals::logfileinfo, $infoline);

        my $grandpagid = "root";
        if ( $parentgid ne "root" )
        {
            my $create_missing_parent = is_empty_parent($parentgid, $allpackages);

            if (( $create_missing_parent ) && ( ! installer::existence::exists_in_array($parentgid, \@installer::globals::createdxpdfiles) ))
            {
                $grandpagid = create_emptyparents_xpd_file($parentgid, $modulesarrayref, $xpddir);
            }
        }

        if ( $grandpagid ne "root" )
        {
            my $create_missing_parent = is_empty_parent($grandpagid, $allpackages);

            if (( $create_missing_parent ) && ( ! installer::existence::exists_in_array($grandpagid, \@installer::globals::createdxpdfiles) ))
            {
                create_emptyparents_xpd_file($grandpagid, $modulesarrayref, $xpddir);
             }
         }
    }
    else
    {
        installer::exiter::exit_program("ERROR: No module definition found for gid: $modulegid", "create_xpd_file (xpdinstaller)");
    }

}

###################################################
# Creating a xpd file for a copied package
###################################################

sub create_xpd_file_for_childproject
{
    my ($module, $destdir, $packagename, $allvariableshashref, $modulesarrayref) = @_;

    my $modulegid = $module->{'gid'};

    my $currentdir = cwd();
    $destdir =~ s/\/\s*$//;
    $currentdir =~ s/\/\s*$//;

    my $completepackage = $currentdir . $installer::globals::separator . $destdir . $installer::globals::separator . $packagename;

    # all content saved in scp is now available and can be used to create the xpd file
    my ( $xpdfile, $parentgid ) = get_file_content($module, $completepackage, "", 0, 0, "", 0, "", "");

    my $xpdfilename = get_xpd_filename($modulegid, 0);
    $xpdfilename = $installer::globals::xpddir . $installer::globals::separator . $xpdfilename;

    installer::files::save_file($xpdfilename, $xpdfile);
    push( @installer::globals::createdxpdfiles, $modulegid);
    push(@installer::globals::allxpdfiles, $xpdfilename);
    my $infoline = "Saving xpd file: $xpdfilename\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ( $parentgid ne "root" )
    {
        my $create_missing_parent = 1; # -> Always missing parent by child projects!
        # Parent is now created, if it was not created before. Attention: Parent module must not come later.
        if (( $create_missing_parent ) && ( ! installer::existence::exists_in_array($parentgid, \@installer::globals::createdxpdfiles) ))
        {
            create_emptyparents_xpd_file($parentgid, $modulesarrayref, $installer::globals::xpddir);
        }
    }
}

##############################################################
# Creating a xpd file for copied system integration package
##############################################################

sub create_xpd_file_for_systemintegration
{
    my ($module, $newcontent, $modulesarrayref, $subdir) = @_;

    my $parentgid = $module->{'gid'};

    # Create new visible module from scp info and create
    # new hidden module for each package inside in tar file

    for ( my $i = 0; $i <= $#{$newcontent}; $i++ )
    {
        my $newpackagename = ${$newcontent}[$i];

        my $infoline = "Creating xpd file for package: $newpackagename\n";
        push( @installer::globals::logfileinfo, $infoline);

        my $childmodule = {%{$module}};
        $childmodule->{'ParentID'} = $module->{'gid'};  # the module gid is the new parent
        $childmodule->{'InstallOrder'} = $installer::globals::defaultsystemintinstallorder;
        my $number = $i + 1;
        my $modulegid = $module->{'gid'} . "_child_" . $number; # setting a dynamic new gid
        $childmodule->{'gid'} = $modulegid;
        $childmodule->{'Styles'} =~ s/\)/\,HIDDEN_ROOT\)/;
        # iterating over all languages to get names and descriptions
        remove_lang_values($childmodule, "Name");
        remove_lang_values($childmodule, "Description");

        my $shortpackagename = $newpackagename;
        installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$shortpackagename);
        $childmodule->{'PackageName'} = $shortpackagename;
        $childmodule->{'Name'} = $modulegid;
        $childmodule->{'Description'} = $modulegid;

        # Checking, if installorder can be set:
        # scp syntax: InstallOrder = "desktop:1050, suse:1060";
        # The string before the number can be compared with $shortpackagename
        if ( $module->{'InstallOrder'} )
        {
            my $installorder = $module->{'InstallOrder'};
            $installorder =~ s/^\s*\"//g;
            $installorder =~ s/\"\s*$//g;
            # $installorder is comma separated list
            my $allorders = installer::converter::convert_stringlist_into_array(\$installorder, ",");
            for ( my $j = 0; $j <= $#{$allorders}; $j++ )
            {
                my $oneitem = ${$allorders}[$j];
                if ( $oneitem =~ /^\s*(\S+?)\s*:\s*(\S+?)\s*$/ )
                {
                    my $name = $1;
                    my $order = $2;

                    if ( $shortpackagename =~ /\Q$name\E/ ) { $childmodule->{'InstallOrder'} = $order; }
                }
            }
        }

        # all content saved in scp is now available and can be used to create the xpd file
        my ( $xpdfile, $parentgid_ ) = get_file_content($childmodule, $newpackagename, "", 0, 0, $subdir, 0, "", "");

        my $xpdfilename = get_xpd_filename($modulegid, 0);
        $xpdfilename = $installer::globals::xpddir . $installer::globals::separator . $xpdfilename;

        installer::files::save_file($xpdfilename, $xpdfile);
        push(@installer::globals::allxpdfiles, $xpdfilename);
        $infoline = "Saving xpd file: $xpdfilename\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    # Creating the top level visible xpd file
    create_emptyparents_xpd_file($parentgid, $modulesarrayref, $installer::globals::xpddir);
}

##############################################################
# Copying xpd files into installation set
##############################################################

sub copy_xpd_files
{
    my ( $destdir ) = @_;

    for ( my $i = 0; $i <= $#installer::globals::allxpdfiles; $i++ )
    {
        if ( ! -f $installer::globals::allxpdfiles[$i] ) { installer::exiter::exit_program("ERROR: Could not find xpd file: $installer::globals::allxpdfiles[$i]!", "copy_xpd_files"); }
        installer::systemactions::copy_one_file($installer::globals::allxpdfiles[$i], $destdir);
    }
}

##############################################################
# Copying all xpd files into the installation set
##############################################################

sub copy_xpd_files_into_installset
{
    my ($installdir) = @_;

    $installdir =~ s/\Q$installer::globals::separator\E\s*$//;

    my $instdir = $installdir . $installer::globals::separator . "installdata";
    installer::systemactions::create_directory($instdir);

    my $xpddir = $instdir . $installer::globals::separator . "xpd";
    installer::systemactions::create_directory($xpddir);
    copy_xpd_files($xpddir);
}

##############################################################
# Creating base xpd file with product information
##############################################################

sub create_setup_xpd
{
    my ($allvariables, $languagestringref) = @_;

    my ( $xpdfile ) = get_setup_file_content($allvariables, $languagestringref);

    my $xpdfilename = $installer::globals::productxpdfile;
    $xpdfilename = $installer::globals::xpddir . $installer::globals::separator . $xpdfilename;

    installer::files::save_file($xpdfilename, $xpdfile);
    push(@installer::globals::allxpdfiles, $xpdfilename);
    my $infoline = "Saving xpd file: $xpdfilename\n";
    push( @installer::globals::logfileinfo, $infoline);
}

###################################################
# Copying the files needed by the xpd installer
# into the installation directory
###################################################

sub create_xpd_installer
{
    my ( $installdir, $allvariables, $languagestringref) = @_;

    installer::logger::include_header_into_logfile("Creating xpd installer:");

    # create setup.xpd file
    create_setup_xpd($allvariables, $languagestringref);

    # copy xpd files into installation set
    copy_xpd_files_into_installset($installdir);
}

1;
