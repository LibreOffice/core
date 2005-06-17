#*************************************************************************
#
#   $RCSfile: worker.pm,v $
#
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

package installer::worker;

use installer::control;
use installer::existence;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::logger;
use installer::mail;
use installer::pathanalyzer;
use installer::scriptitems;
use installer::sorter;
use installer::systemactions;
use installer::windows::language;

#####################################################################
# Unpacking all files ending with tar.gz in a specified directory
#####################################################################

sub unpack_all_targzfiles_in_directory
{
    my ( $directory ) = @_;

    installer::logger::include_header_into_logfile("Unpacking tar.gz files:");

    installer::logger::print_message( "... unpacking tar.gz files ... \n" );

    my $localdirectory = $directory . $installer::globals::separator . "packages";
    my $alltargzfiles = installer::systemactions::find_file_with_file_extension("tar.gz", $localdirectory);

    for ( my $i = 0; $i <= $#{$alltargzfiles}; $i++ )
    {
        my $onefile = $localdirectory . $installer::globals::separator . ${$alltargzfiles}[$i];

        my $systemcall = "cd $localdirectory; cat ${$alltargzfiles}[$i] \| gunzip \| tar -xf -";
        $returnvalue = system($systemcall);

        my $infoline = "Systemcall: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);

        if ($returnvalue)
        {
            $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
        else
        {
            $infoline = "Success: Executed \"$systemcall\" successfully!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }
}

#########################################
# Copying installation sets to ship
#########################################

sub copy_install_sets_to_ship
{
    my ( $destdir, $shipinstalldir  ) = @_;

    installer::logger::include_header_into_logfile("Copying installation set to ship:");

    my $dirname = $destdir;
    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$dirname);
    $dirname = $dirname . "_inprogress";
    my $localshipinstalldir = $shipinstalldir . $installer::globals::separator . $dirname;
    if ( ! -d $localshipinstalldir ) { installer::systemactions::create_directory_structure($localshipinstalldir); }

    # copy installation set to /ship ($localshipinstalldir)
    installer::logger::print_message( "... copy installation set from " . $destdir . " to " . $localshipinstalldir . "\n" );
    installer::systemactions::copy_complete_directory($destdir, $localshipinstalldir);

    if (( ! $installer::globals::iswindowsbuild ) && ( $installer::globals::addjavainstaller ))
    {
        # Setting Unix rights for Java starter ("setup")
        my $localcall = "chmod 775 $localshipinstalldir/setup \>\/dev\/null 2\>\&1";
        system($localcall);
    }

    # unpacking the tar.gz file for Solaris
    if ( $installer::globals::issolarisbuild ) { unpack_all_targzfiles_in_directory($localshipinstalldir); }

    $localshipinstalldir = installer::systemactions::rename_string_in_directory($localshipinstalldir, "_inprogress", "");

    return $localshipinstalldir;
}

#########################################
# Copying installation sets to ship
#########################################

sub link_install_sets_to_ship
{
    my ( $destdir, $shipinstalldir  ) = @_;

    installer::logger::include_header_into_logfile("Linking installation set to ship:");

    my $infoline = "... destination directory: $shipinstalldir ...\n";
    installer::logger::print_message( $infoline );
    push( @installer::globals::logfileinfo, $infoline);

    if ( ! -d $shipinstalldir)
    {
        $infoline = "Creating directory: $shipinstalldir\n";
        push( @installer::globals::logfileinfo, $infoline);
        installer::systemactions::create_directory_structure($shipinstalldir);
        $infoline = "Created directory: $shipinstalldir\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    my $dirname = $destdir;
    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$dirname);

    my $localshipinstalldir = $shipinstalldir . $installer::globals::separator . $dirname;

    # link installation set to /ship ($localshipinstalldir)
    installer::logger::print_message( "... linking installation set from " . $destdir . " to " . $localshipinstalldir . "\n" );

    my $systemcall = "ln -s $destdir $localshipinstalldir";

    $returnvalue = system($systemcall);

    $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not create link \"$localshipinstalldir\"!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Success: Created link \"$localshipinstalldir\"!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    return $localshipinstalldir;
}

#########################################
# Create checksum file
#########################################

sub make_checksum_file
{
    my ( $filesref, $includepatharrayref ) = @_;

    my @checksum = ();

    my $checksumfileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$installer::globals::checksumfile, $includepatharrayref, 1);
    if ( $$checksumfileref eq "" ) { installer::exiter::exit_program("ERROR: Could not find file $installer::globals::checksumfile !", "make_checksum_file"); }

#   # very slow on Windows
#   for ( my $i = 0; $i <= $#{$filesref}; $i++ )
#   {
#       my $onefile = ${$filesref}[$i];
#       my $systemcall = "$$checksumfileref $onefile->{'sourcepath'} |";
#       open (CHECK, "$systemcall");
#       my $localchecksum = <CHECK>;
#       close (CHECK);
#       push(@checksum, $localchecksum);
#   }

    my $systemcall = "$$checksumfileref";

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];
        $systemcall = $systemcall . " " . $onefile->{'sourcepath'};     # very very long systemcall

        if ((( $i > 0 ) &&  ( $i%100 == 0 )) || ( $i == $#{$filesref} ))    # limiting to 100 files
        {
            $systemcall = $systemcall . " \|";

            my @localchecksum = ();
            open (CHECK, "$systemcall");
            @localchecksum = <CHECK>;
            close (CHECK);

            for ( my $j = 0; $j <= $#localchecksum; $j++ ) { push(@checksum, $localchecksum[$j]); }

            $systemcall = "$$checksumfileref";  # reset the system call
        }
    }

    return \@checksum;
}

#########################################
# Saving the checksum file
#########################################

sub save_checksum_file
{
    my ($current_install_number, $installchecksumdir, $checksumfile) = @_;

    my $numberedchecksumfilename = $installer::globals::checksumfilename;
    $numberedchecksumfilename =~ s/\./_$current_install_number\./;  # checksum.txt -> checksum_01.txt
    installer::files::save_file($installchecksumdir . $installer::globals::separator . $numberedchecksumfilename, $checksumfile);
}

###############################################################
# Removing all directories of a special language
# in the directory $basedir
###############################################################

sub remove_old_installation_sets
{
    my ($basedir) = @_;

    installer::logger::print_message( "... removing old installation directories ...\n" );

    my $removedir = $basedir;

    if ( -d $removedir ) { installer::systemactions::remove_complete_directory($removedir, 1); }

    # looking for non successful old installation sets

    $removedir = $basedir . "_witherror";
    if ( -d $removedir ) { installer::systemactions::remove_complete_directory($removedir, 1); }

    $removedir = $basedir . "_inprogress";
    if ( -d $removedir ) { installer::systemactions::remove_complete_directory($removedir, 1); }

    # finally the $basedir can be created empty

    installer::systemactions::create_directory($basedir);
}

###############################################################
# Removing all non successful installation sets on ship
###############################################################

sub remove_old_ship_installation_sets
{
    my ($fulldir, $counter) = @_;

    installer::logger::print_message( "... removing old installation directories ...\n" );

    my $basedir = $fulldir;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$basedir);

    # collecting all directories next to the new installation directory
    my $alldirs = installer::systemactions::get_all_directories($basedir);

    if ( $fulldir =~ /^\s*(.*?inprogress\-)(\d+)(.*?)\s*$/ )
    {
        my $pre_inprogress = $1;        # $pre still contains "inprogress"
        my $number = $2;
        my $post = $3;
        my $pre_witherror = $pre_inprogress;
        $pre_witherror =~ s/inprogress/witherror/;

        for ( my $i = 0; $i <= $#{$alldirs}; $i++ )
        {
            if ( ${$alldirs}[$i] eq $fulldir ) { next; }    # do not delete the newly created directory

            if ( ${$alldirs}[$i] =~ /^\s*\Q$pre_inprogress\E\d+\Q$post\E\s*$/ ) # removing old "inprogress" directories
            {
                installer::systemactions::remove_complete_directory(${$alldirs}[$i], 1);
            }

            if ( ${$alldirs}[$i] =~ /^\s*\Q$pre_witherror\E\d+\Q$post\E\s*$/ )  # removing old "witherror" directories
            {
                installer::systemactions::remove_complete_directory(${$alldirs}[$i], 1);
            }
        }
    }
}

###############################################################
# Creating the installation directory structure
###############################################################

sub create_installation_directory
{
    my ($shipinstalldir, $languagestringref, $current_install_number_ref) = @_;

    my $installdir = "";

    my $languageref = $languagestringref;
    if ( $installer::globals::is_unix_multi ) { $languageref = \$installer::globals::unixmultipath; }

    if ( $installer::globals::updatepack )
    {
        $installdir = $shipinstalldir;
        installer::systemactions::create_directory_structure($installdir);
        $$current_install_number_ref = installer::systemactions::determine_maximum_number($installdir, $languageref);
        $installdir = installer::systemactions::rename_string_in_directory($installdir, "number", $$current_install_number_ref);
        remove_old_ship_installation_sets($installdir);
    }
    else
    {
        $installdir = installer::systemactions::create_directories("install", $languageref);
        installer::logger::print_message( "... creating installation set in $installdir ...\n" );
        remove_old_installation_sets($installdir);
        my $inprogressinstalldir = $installdir . "_inprogress";
        installer::systemactions::rename_directory($installdir, $inprogressinstalldir);
        $installdir = $inprogressinstalldir;
    }

    $installer::globals::saveinstalldir = $installdir;  # saving directory globally, in case of exiting

    return $installdir;
}

###############################################################
# Analyzing and creating the log file
###############################################################

sub analyze_and_save_logfile
{
    my ($loggingdir, $installdir, $installlogdir, $allsettingsarrayref, $languagestringref, $current_install_number) = @_;

    my $is_success = 1;
    my $finalinstalldir = "";

    installer::logger::print_message( "... checking log file " . $loggingdir . $installer::globals::logfilename . "\n" );

    my $contains_error = installer::control::check_logfile(\@installer::globals::logfileinfo);

    # Dependent from the success, the installation directory can be renamed and mails can be send.

    if ( $contains_error )
    {
        my $errordir = installer::systemactions::rename_string_in_directory($installdir, "_inprogress", "_witherror");
        if ( $installer::globals::updatepack ) { installer::mail::send_fail_mail($allsettingsarrayref, $languagestringref, $errordir); }
        # Error output to STDERR
        for ( my $j = 0; $j <= $#installer::globals::errorlogfileinfo; $j++ )
        {
            my $line = $installer::globals::errorlogfileinfo[$j];
            $line =~ s/\s*$//g;
            installer::logger::print_error( $line );
        }
        $is_success = 0;

        $finalinstalldir = $errordir;
    }
    else
    {
        my $destdir = "";

        if ( $installer::globals::updatepack )
        {
            if ( $installdir =~ /_download_inprogress/ ) { $destdir = installer::systemactions::rename_string_in_directory($installdir, "_download_inprogress", "_download"); }
            elsif ( $installdir =~ /_jds_inprogress/ ) { $destdir = installer::systemactions::rename_string_in_directory($installdir, "_jds_inprogress", "_jds"); }
            else { $destdir = installer::systemactions::rename_string_in_directory($installdir, "_inprogress", "_packed"); }
            installer::mail::send_success_mail($allsettingsarrayref, $languagestringref, $destdir);
        }
        else
        {
            $destdir = installer::systemactions::rename_string_in_directory($installdir, "_inprogress", "");
        }

        $finalinstalldir = $destdir;
    }

    # Saving the logfile in the log file directory and additionally in a log directory in the install directory

    my $numberedlogfilename = $installer::globals::logfilename;
    if ( $installer::globals::updatepack ) { $numberedlogfilename =~ s /log_/log_$current_install_number\_/; }
    installer::logger::print_message( "... creating log file $numberedlogfilename \n" );
    installer::files::save_file($loggingdir . $numberedlogfilename, \@installer::globals::logfileinfo);
    installer::files::save_file($installlogdir . $installer::globals::separator . $numberedlogfilename, \@installer::globals::logfileinfo);

    # Saving the checksumfile in a checksum directory in the install directory
    # installer::worker::save_checksum_file($current_install_number, $installchecksumdir, $checksumfile);

    return ($is_success, $finalinstalldir);
}

###############################################################
# Removing all directories that are saved in the
# global directory @installer::globals::removedirs
###############################################################

sub clean_output_tree
{
    installer::logger::print_message( "... cleaning the output tree ...\n" );

    for ( my $i = 0; $i <= $#installer::globals::removedirs; $i++ )
    {
        if ( -d $installer::globals::removedirs[$i] )
        {
            installer::logger::print_message( "... removing directory $installer::globals::removedirs[$i] ...\n" );
            installer::systemactions::remove_complete_directory($installer::globals::removedirs[$i], 1);
        }
    }

    # Last try to remove the ship test directory

    if ( $installer::globals::shiptestdirectory )
    {
        if ( -d $installer::globals::shiptestdirectory )
        {
            my $infoline = "Last try to remove $installer::globals::shiptestdirectory . \n";
            push(@installer::globals::globallogfileinfo, $infoline);
            my $systemcall = "rmdir $installer::globals::shiptestdirectory";
            my $returnvalue = system($systemcall);
        }
    }
}

###########################################################
# Copying a reference array
###########################################################

sub copy_array_from_references
{
    my ( $arrayref ) = @_;

    my @newarray = ();

    for ( my $i = 0; $i <= $#{$arrayref}; $i++ )
    {
        push(@newarray, ${$arrayref}[$i]);
    }

    return \@newarray;
}

###########################################################
# Setting one language in the language independent
# array of include pathes with $(LANG)
###########################################################

sub get_language_specific_include_pathes
{
    my ( $patharrayref, $onelanguage ) = @_;

    my @patharray = ();

    for ( my $i = 0; $i <= $#{$patharrayref}; $i++ )
    {
        my $line = ${$patharrayref}[$i];
        $line =~ s/\$\(LANG\)/$onelanguage/g;
        push(@patharray ,$line);
    }

    return \@patharray;
}

##############################################################
# Returning the first item with a defined flag
##############################################################

sub return_first_item_with_special_flag
{
    my ($itemsref, $flag) = @_;

    my $firstitem = "";

    for ( my $i = 0; $i <= $#{$itemsref}; $i++ )
    {
        my $oneitem = ${$itemsref}[$i];
        my $styles = "";
        if ( $oneitem->{'Styles'} ) { $styles = $oneitem->{'Styles'} };

        if ( $styles =~ /\b$flag\b/ )
        {
            $firstitem = $oneitem;
            last;
        }
    }

    return $firstitem;
}

##############################################################
# Collecting all items with a defined flag
##############################################################

sub collect_all_items_with_special_flag
{
    my ($itemsref, $flag) = @_;

    my @allitems = ();

    for ( my $i = 0; $i <= $#{$itemsref}; $i++ )
    {
        my $oneitem = ${$itemsref}[$i];
        my $styles = "";
        if ( $oneitem->{'Styles'} ) { $styles = $oneitem->{'Styles'} };

        if ( $styles =~ /\b$flag\b/ )
        {
            push( @allitems, $oneitem );
        }
    }

    return \@allitems;
}

##############################################################
# Removing all items with a defined flag from collector
##############################################################

sub remove_all_items_with_special_flag
{
    my ($itemsref, $flag) = @_;

    my @allitems = ();

    for ( my $i = 0; $i <= $#{$itemsref}; $i++ )
    {
        my $oneitem = ${$itemsref}[$i];
        my $styles = "";
        if ( $oneitem->{'Styles'} ) { $styles = $oneitem->{'Styles'} };
        if ( $styles =~ /\b$flag\b/ )
        {
            my $infoline = "Attention: Removing from collector: $oneitem->{'Name'} !\n";
            push( @installer::globals::logfileinfo, $infoline);
            if ( $flag eq "BINARYTABLE_ONLY" ) { push(@installer::globals::binarytableonlyfiles, $oneitem); }
            next;
        }
        push( @allitems, $oneitem );
    }

    return \@allitems;
}

###########################################################
# Mechanism for simple installation without packing
###########################################################

sub install_simple ($$$$$)
{
    my ($packagename, $languagestring, $directoriesarray, $filesarray, $linksarray) = @_;

    installer::logger::print_message( "... installing module $packagename ...\n" );

    my $destdir = $installer::globals::destdir;
    my @lines = ();

    installer::logger::print_message( "DestDir: $destdir \n" );
    installer::logger::print_message( "Rootpath: $installer::globals::rootpath \n" );

    `mkdir -p $destdir` if $destdir ne "";
    `mkdir -p $destdir$installer::globals::rootpath`;

    # Create Directories
    for ( my $i = 0; $i <= $#{$directoriesarray}; $i++ )
    {
        my $onedir = ${$directoriesarray}[$i];
        my $dir = "";

        if ( $onedir->{'Dir'} ) { $dir = $onedir->{'Dir'}; }

        if ((!($dir =~ /\bPREDEFINED_/ )) || ( $dir =~ /\bPREDEFINED_PROGDIR\b/ ))
        {
            # printf "mkdir $destdir$onedir->{'HostName'}\n";
            mkdir $destdir . $onedir->{'HostName'};
            push @lines, "%dir " . $onedir->{'HostName'} . "\n";
        }
    }

    for ( my $i = 0; $i <= $#{$filesarray}; $i++ )
    {
        my $onefile = ${$filesarray}[$i];
        my $unixrights = $onefile->{'UnixRights'};
        my $destination = $onefile->{'destination'};
        my $sourcepath = $onefile->{'sourcepath'};

        # printf "mv $sourcepath $destdir$destination\n";
        `cp -af '$sourcepath' '$destdir$destination'`;
        `chmod $unixrights '$destdir$destination'`;
        push @lines, "$destination\n";
    }

    for ( my $i = 0; $i <= $#{$linksarray}; $i++ )
    {
        my $onelink = ${$linksarray}[$i];
        my $destination = $onelink->{'destination'};
        my $destinationfile = $onelink->{'destinationfile'};

        # print "link $destinationfile -> $destdir$destination\n";
        `ln -sf '$destinationfile' '$destdir$destination'`;
        push @lines, "$destination\n";
    }

    if ( $destdir ne "" )
    {
        my $filelist;
        my $fname = $installer::globals::destdir . "/$packagename";
        if ($installer::globals::languagepack) { $fname .= ".$languagestring"; }
        open ($filelist, ">$fname") || die "Can't open $fname: $!";
        print $filelist @lines;
        close ($filelist);
    }

}

###########################################################
# Adding shellnew files into files collector for
# user installation
###########################################################

sub add_shellnewfile_into_filesarray
{
    my ($filesref, $onefile, $inffile) = @_;

    my %shellnewfile = ();
    my $shellnewfileref = \%shellnewfile;

    installer::converter::copy_item_object($inffile, $shellnewfileref);

    $shellnewfileref->{'Name'} = $onefile->{'Name'};
    $shellnewfileref->{'sourcepath'} = $onefile->{'sourcepath'};
    $shellnewfileref->{'gid'} = $onefile->{'gid'} . "_Userinstall";

    # the destination has to be adapted
    my $destination = $inffile->{'destination'};
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$destination);
    $destination = $destination . $onefile->{'Name'};
    $shellnewfileref->{'destination'} = $destination;

    # add language specific inffile into filesarray
    push(@{$filesref}, $shellnewfileref);
}

###########################################################
# Replacing one placehoder in template file
###########################################################

sub replace_in_template_file
{
    my ($templatefile, $placeholder, $newstring) = @_;

    for ( my $i = 0; $i <= $#{$templatefile}; $i++ )
    {
        ${$templatefile}[$i] =~ s/\Q$placeholder\E/$newstring/g;
    }
}

###########################################################
# Replacing one placehoder with an array in template file
###########################################################

sub replace_array_in_template_file
{
    my ($templatefile, $placeholder, $arrayref) = @_;

    for ( my $i = 0; $i <= $#{$templatefile}; $i++ )
    {
        if ( ${$templatefile}[$i] =~ /\Q$placeholder\E/ )
        {
            my @return = splice(@{$templatefile}, $i, 1, @{$arrayref});
        }
    }
}

###########################################################
# Collecting all modules from registry items
###########################################################

sub collect_all_modules
{
    my ($registryitemsref) = @_;

    my @allmodules = ();

    for ( my $i = 0; $i <= $#{$registryitemsref}; $i++ )
    {
        $registryitem = ${$registryitemsref}[$i];
        my $module = $registryitem->{'ModuleID'};

        if ( ! installer::existence::exists_in_array($module, \@allmodules) )
        {
            push(@allmodules, $module);
        }
    }

    return \@allmodules;
}

###########################################################
# Changing the content of the inf file
###########################################################

sub write_content_into_inf_file
{
    my ($templatefile, $filesref, $registryitemsref, $folderref, $folderitemsref, $modulesref, $onelanguage, $inffile, $firstlanguage, $allvariableshashref) = @_;

    # First part: Shellnew files
    # SHELLNEWFILESPLACEHOLDER

    my $rootmodule = 0;
    if ( $inffile->{'modules'} =~ /Module_Root/i ) { $rootmodule = 1; }

    if ( $rootmodule )
    {
        my $shellnewstring = "";

        for ( my $i = 0; $i <= $#{$filesref}; $i++ )
        {
            my $onefile = ${$filesref}[$i];
            my $directory = $onefile->{'Dir'};

            if ( $directory =~ /\bPREDEFINED_OSSHELLNEWDIR\b/ )
            {
                $shellnewstring = $shellnewstring . $onefile->{'Name'} . "\n";
                if ( $firstlanguage ) { add_shellnewfile_into_filesarray($filesref, $onefile, $inffile); }
            }
        }

        $shellnewstring =~ s/\s*$//;
        replace_in_template_file($templatefile, "SHELLNEWFILESPLACEHOLDER", $shellnewstring);
    }

    # Second part: Start menu entries

    # The OfficeMenuFolder is defined as: $productname . " " . $productversion;

    my $productname = $allvariableshashref->{'PRODUCTNAME'};
    my $productversion = $allvariableshashref->{'PRODUCTVERSION'};
    my $productkey = $productname . " " . $productversion;

    replace_in_template_file($templatefile, "OFFICEFOLDERPLACEHOLDER", $productkey);

    # Setting name target and infotip for all applications

    for ( my $i = 0; $i <= $#{$folderitemsref}; $i++ )
    {
        my $folderitem = ${$folderitemsref}[$i];

        if (( ! $folderitem->{'ismultilingual'} ) || (( $folderitem->{'ismultilingual'} ) && ( $folderitem->{'specificlanguage'} eq $onelanguage )))
        {
            my $gid = $folderitem->{'gid'};
            my $app = $gid;
            $app =~ s/gid_Folderitem_//;
            $app = uc($app);

            my $name = $folderitem->{'Name'};
            my $placeholder = "PLACEHOLDER_FOLDERITEM_NAME_" . $app;
            replace_in_template_file($templatefile, $placeholder, $name);

            my $tooltip = $folderitem->{'Tooltip'};
            $placeholder = "PLACEHOLDER_FOLDERITEM_TOOLTIP_" . $app;
            replace_in_template_file($templatefile, $placeholder, $tooltip);

            my $executablegid = $folderitem->{'FileID'};
            my $exefile = installer::existence::get_specified_file($filesref, $executablegid);
            my $exefilename = $exefile->{'Name'};
            $placeholder = "PLACEHOLDER_FOLDERITEM_TARGET_" . $app;
            replace_in_template_file($templatefile, $placeholder, $exefilename);
        }
    }

    # Third part: Windows registry entries

    # collecting all modules

    my $allmodules = collect_all_modules($registryitemsref);

    my @registryitems = ();
    my $allsectionsstring = "";

    for ( my $j = 0; $j <= $#{$allmodules}; $j++ )
    {
        my $moduleid = ${$allmodules}[$j];

        my $inffilemodule = $inffile->{'modules'};
        if ( $inffilemodule =~ /Module_Root/i ) { $inffilemodule = "gid_Module_Root"; }

        if ( ! ( $moduleid eq $inffilemodule )) { next; }

        my $shortmodulename = $moduleid;
        $shortmodulename =~ s/gid_Module_//;
        my $sectionname = "InstRegKeys." . $shortmodulename;
        $allsectionsstring = $allsectionsstring . $sectionname . ",";
        my $sectionheader = "\[" . $sectionname . "\]" . "\n";
        push(@registryitems, $sectionheader);

        for ( my $i = 0; $i <= $#{$registryitemsref}; $i++ )
        {
            my $registryitem = ${$registryitemsref}[$i];

            if ( ! ( $registryitem->{'ModuleID'} eq $moduleid )) { next; }

            if (( ! $registryitem->{'ismultilingual'} ) || (( $registryitem->{'ismultilingual'} ) && ( $registryitem->{'specificlanguage'} eq $onelanguage )))
            {
                # Syntax: HKCR,".bau",,,"soffice.StarConfigFile.6"

                my $regroot = "";
                my $parentid = "";
                if ( $registryitem->{'ParentID'} ) { $parentid = $registryitem->{'ParentID'}; }
                if ( $parentid eq "PREDEFINED_HKEY_CLASSES_ROOT" ) { $regroot = "HKCR"; }
                if ( $parentid eq "PREDEFINED_HKEY_LOCAL_MACHINE" ) { $regroot = "HKCU"; }

                my $subkey = "";
                if ( $registryitem->{'Subkey'} ) { $subkey = $registryitem->{'Subkey'}; }
                if ( $subkey ne "" ) { $subkey = "\"" . $subkey . "\""; }

                my $valueentryname = "";
                if ( $registryitem->{'Name'} ) { $valueentryname = $registryitem->{'Name'}; }
                if ( $valueentryname ne "" ) { $valueentryname = "\"" . $valueentryname . "\""; }

                my $flag = "";

                my $value = "";
                if ( $registryitem->{'Value'} ) { $value = $registryitem->{'Value'}; }
                if ( $value =~ /\<progpath\>/ ) { $value =~ s/\\\"/\"\"/g; } # Quoting for INF is done by double ""
                $value =~ s/\\\"/\"/g;  # no more masquerading of '"'
                $value =~ s/\<progpath\>/\%INSTALLLOCATION\%/g;
                # $value =~ s/\%INSTALLLOCATION\%\\/\%INSTALLLOCATION\%/g;      # removing "\" after "%INSTALLLOCATION%"
                if ( $value ne "" ) { $value = "\"" . $value . "\""; }

                my $oneline = $regroot . "," . $subkey . "," . $valueentryname . "," . $flag . "," . $value . "\n";

                push(@registryitems, $oneline);
            }
        }

        push(@registryitems, "\n"); # empty line after each section
    }

    # replacing the $allsectionsstring
    $allsectionsstring =~ s/\,\s*$//;
    replace_in_template_file($templatefile, "ALLREGISTRYSECTIONSPLACEHOLDER", $allsectionsstring);

    # replacing the placeholder for all registry keys
    replace_array_in_template_file($templatefile, "REGISTRYKEYSPLACEHOLDER", \@registryitems);

}

###########################################################
# Creating inf files for local user system integration
###########################################################

sub create_inf_file
{
    my ($filesref, $registryitemsref, $folderref, $folderitemsref, $modulesref, $languagesarrayref, $languagestringref, $allvariableshashref) = @_;

    # collecting all files with flag INFFILE

    my $inf_files = collect_all_items_with_special_flag($filesref ,"INFFILE");

    if ( $#{$inf_files} > -1 )
    {
        # create new language specific inffile
        installer::logger::include_header_into_logfile("Creating inf files:");

        my $infdirname = "inffiles";
        my $infdir = installer::systemactions::create_directories($infdirname, $languagestringref);

        my $infoline = "Number of inf files: $#{$inf_files} + 1 \n";
        push( @installer::globals::logfileinfo, $infoline);

        # there are inffiles for all modules

        for ( my $i = 0; $i <= $#{$inf_files}; $i++ )
        {
            my $inffile = ${$inf_files}[$i];
            my $inf_file_name = $inffile->{'Name'};

            my $templatefilename = $inffile->{'sourcepath'};

            if ( ! -f $templatefilename ) { installer::exiter::exit_program("ERROR: Could not find file $templatefilename !", "create_inf_file");;  }

            # iterating over all languages

            for ( my $j = 0; $j <= $#{$languagesarrayref}; $j++ )   # iterating over all languages
            {
                my $firstlanguage = 0;
                if ( $j == 0 ) { $firstlanguage = 1; }

                my $onelanguage = ${$languagesarrayref}[$j];

                $infoline = "Templatefile: $inf_file_name, Language: $onelanguage \n";
                push( @installer::globals::logfileinfo, $infoline);

                my $templatefile = installer::files::read_file($templatefilename);

                my $linesbefore = $#{$templatefile};

                write_content_into_inf_file($templatefile, $filesref, $registryitemsref, $folderref, $folderitemsref, $modulesref, $onelanguage, $inffile, $firstlanguage, $allvariableshashref);

                $infoline = "Lines change: From $linesbefore to $#{$templatefile}.\n";
                push( @installer::globals::logfileinfo, $infoline);

                # rename language specific inffile
                my $language_inf_file_name = $inf_file_name;
                my $windowslanguage = installer::windows::language::get_windows_language($onelanguage);
                $language_inf_file_name =~ s/\.inf/_$windowslanguage\.inf/;

                my $sourcepath = $infdir . $installer::globals::separator . $language_inf_file_name;
                installer::files::save_file($sourcepath, $templatefile);

                $infoline = "Saving file: $sourcepath\n";
                push( @installer::globals::logfileinfo, $infoline);

                # creating new file object

                my %languageinffile = ();
                my $languageinifileref = \%languageinffile;

                if ( $j < $#{$languagesarrayref} ) { installer::converter::copy_item_object($inffile, $languageinifileref); }
                else { $languageinifileref = $inffile; }

                $languageinifileref->{'Name'} = $language_inf_file_name;
                $languageinifileref->{'sourcepath'} = $sourcepath;
                # destination and gid also have to be adapted
                $languageinifileref->{'gid'} = $languageinifileref->{'gid'} . "_" . $onelanguage;
                my $destination = $languageinifileref->{'destination'};
                installer::pathanalyzer::get_path_from_fullqualifiedname(\$destination);
                $destination = $destination . $language_inf_file_name;
                $languageinifileref->{'destination'} = $destination;

                # add language specific inffile into filesarray
                if ( $j < $#{$languagesarrayref} ) { push(@{$filesref}, $languageinifileref); }
            }
        }
    }
}

###########################################################
# Selecting patch items
###########################################################

sub select_patch_items
{
    my ( $itemsref, $itemname ) = @_;

    installer::logger::include_header_into_logfile("Selecting items for patches. Item: $itemname");

    my @itemsarray = ();

    for ( my $i = 0; $i <= $#{$itemsref}; $i++ )
    {
        my $oneitem = ${$itemsref}[$i];

        my $name = $oneitem->{'Name'};
        if (( $name =~ /\bLICENSE/ ) || ( $name =~ /\bREADME/ ))
        {
            push(@itemsarray, $oneitem);
            next;
        }

        # Items with style "PATCH" have to be included into the patch
        my $styles = "";
        if ( $oneitem->{'Styles'} ) { $styles = $oneitem->{'Styles'}; }
        if ( $styles =~ /\bPATCH\b/ ) { push(@itemsarray, $oneitem); }
    }

    return \@itemsarray;
}

###########################################################
# Selecting patch items
###########################################################

sub select_patch_items_without_name
{
    my ( $itemsref, $itemname ) = @_;

    installer::logger::include_header_into_logfile("Selecting RegistryItems for patches");

    my @itemsarray = ();

    for ( my $i = 0; $i <= $#{$itemsref}; $i++ )
    {
        my $oneitem = ${$itemsref}[$i];

        # Items with style "PATCH" have to be included into the patch
        my $styles = "";
        if ( $oneitem->{'Styles'} ) { $styles = $oneitem->{'Styles'}; }
        if ( $styles =~ /\bPATCH\b/ ) { push(@itemsarray, $oneitem); }
    }

    return \@itemsarray;
}

###########################################################
# Selecting patch items
###########################################################

sub select_langpack_items
{
    my ( $itemsref, $itemname ) = @_;

    installer::logger::include_header_into_logfile("Selecting RegistryItems for Language Packs");

    my @itemsarray = ();

    for ( my $i = 0; $i <= $#{$itemsref}; $i++ )
    {
        my $oneitem = ${$itemsref}[$i];

        # Items with style "LANGUAGEPACK" have to be included into the patch
        my $styles = "";
        if ( $oneitem->{'Styles'} ) { $styles = $oneitem->{'Styles'}; }
        if ( $styles =~ /\bLANGUAGEPACK\b/ ) { push(@itemsarray, $oneitem); }
    }

    return \@itemsarray;
}

###########################################################
# Searching if LICENSE and README, which are not removed
# in select_patch_items are really needed for the patch.
# If not, they are removed now.
###########################################################

sub analyze_patch_files
{
    my ( $filesref ) = @_;

    installer::logger::include_header_into_logfile("Analyzing patch files");

    my @filesarray = ();

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];
        my $styles = "";
        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }
        if ( !( $styles =~ /\bPATCH\b/) ) { next; } # removing all files without flag PATCH (LICENSE, README, ...)

        if ( $installer::globals::iswindowsbuild )
        {
            # all files of the Windows patch belong to the root module
            $onefile->{'modules'} = "gid_Module_Root";
        }

        push(@filesarray, $onefile);
    }

    return \@filesarray;
}

###########################################################
# Sorting an array
###########################################################

sub sort_array
{
    my ( $arrayref ) = @_;

    for ( my $i = 0; $i <= $#{$arrayref}; $i++ )
    {
        my $under = ${$arrayref}[$i];

        for ( my $j = $i + 1; $j <= $#{$arrayref}; $j++ )
        {
            my $over = ${$arrayref}[$j];

            if ( $under gt $over)
            {
                ${$arrayref}[$i] = $over;
                ${$arrayref}[$j] = $under;
                $under = $over;
            }
        }
    }
}

###########################################################
# Renaming linux files with flag LINUXLINK
###########################################################

sub prepare_linuxlinkfiles
{
    my ( $filesref ) = @_;

    @installer::globals::linuxlinks = (); # empty this array, because it could be already used
    @installer::globals::linuxpatchfiles = (); # empty this array, because it could be already used

    my @filesarray = ();

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];
        my %linkfilehash = ();
        my $linkfile = \%linkfilehash;
        installer::converter::copy_item_object($onefile, $linkfile);

        my $ispatchfile = 0;
        my $styles = "";
        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }
        if ( $styles =~ /\bPATCH\b/ ) { $ispatchfile = 1; }

        my $original_destination = $onefile->{'destination'};
        # $onefile->{'destination'} is used in the epm list file. This value can be changed now!

        if ( $ispatchfile ) { $onefile->{'destination'} = $onefile->{'destination'} . "\.$installer::globals::linuxlibrarypatchlevel"; }
        else { $onefile->{'destination'} = $onefile->{'destination'} . "\.$installer::globals::linuxlibrarybaselevel"; }

        my $infoline = "LINUXLINK: Changing file destination from $original_destination to $onefile->{'destination'} !\n";
        push( @installer::globals::logfileinfo, $infoline);

        # all files without PATCH flag are included into the RPM
        if ( ! $ispatchfile ) { push( @filesarray, $onefile); }
        else { push( @installer::globals::linuxpatchfiles, $onefile); }

        # Preparing the collector for the links
        # Setting the new file name as destination of the link
        my $linkdestination = $linkfile->{'Name'};
        installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$linkdestination);
        if ( $ispatchfile ) { $linkfile->{'destinationfile'} = $linkdestination . "\.$installer::globals::linuxlibrarypatchlevel"; }
        else { $linkfile->{'destinationfile'} = $linkdestination . "\.$installer::globals::linuxlibrarybaselevel"; }
        push( @installer::globals::linuxlinks, $linkfile );

        $infoline = "LINUXLINK: Created link: $linkfile->{'destination'} pointing to $linkfile->{'destinationfile'} !\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    return \@filesarray;
}

###########################################################
# Renaming Windows files in Patch and creating file
# patchfiles.txt
###########################################################

sub prepare_windows_patchfiles
{
    my ( $filesref, $languagestringref ) = @_;

    my @patchfiles = ();
    my $patchfilename = "patchlist.txt";
    my $patchfilename2 = "patchmsi.dll";

    my $header = "\[SwapFiles\]\n";
    push(@patchfiles, $header);

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];

        my $filename = $onefile->{'Name'};
        if (( $filename eq $patchfilename ) || ( $filename eq $patchfilename2 )) { next; }

        my $olddestination = $onefile->{'destination'};
        my $newdestination = $olddestination . "." . $installer::globals::windowsfilespatchlevel;
        my $line = "\"" . $olddestination . "\"" . "=" . "\"" . $newdestination . "\"" . "\n";
        $onefile->{'destination'} = $newdestination;

        my $newfilename = $onefile->{'Name'} . "." . $installer::globals::windowsfilespatchlevel;
        $onefile->{'Name'} = $newfilename;

        push(@patchfiles, $line);
    }

    my $winpatchdirname = "winpatch";
    my $winpatchdir = installer::systemactions::create_directories($winpatchdirname, $languagestringref);

    my $patchlistfile = installer::existence::get_specified_file_by_name($filesref, $patchfilename);

    # saving the file
    $patchfilename = $winpatchdir . $installer::globals::separator . $patchfilename;
    installer::files::save_file($patchfilename, \@patchfiles);

    # and assigning the new source
    $patchlistfile->{'sourcepath'} = $patchfilename;
}

###########################################################
# Replacing %-variables with the content
# of $allvariableshashref
###########################################################

sub replace_variables_in_string
{
    my ( $string, $variableshashref ) = @_;

    if ( $string =~ /^.*\%\w+.*$/ )
    {
        my $key;

        foreach $key (keys %{$variableshashref})
        {
            my $value = $variableshashref->{$key};
            $key = "\%" . $key;
            $string =~ s/\Q$key\E/$value/g;
        }
    }

    return $string;
}

###########################################################
# Replacing %-variables with the content
# of $allvariableshashref
###########################################################

sub replace_dollar_variables_in_string
{
    my ( $string, $variableshashref ) = @_;

    if ( $string =~ /^.*\$\{\w+\}.*$/ )
    {
        my $key;

        foreach $key (keys %{$variableshashref})
        {
            my $value = $variableshashref->{$key};
            $key = "\$\{" . $key . "\}";
            $string =~ s/\Q$key\E/$value/g;
        }
    }

    return $string;
}

###########################################################
# The list file contains the list of packages/RPMs that
# have to be copied.
###########################################################

sub get_all_files_from_filelist
{
    my ( $listfile, $section ) = @_;

    my @allpackages = ();

    for ( my $i = 0; $i <= $#{$listfile}; $i++ )
    {
        my $line = ${$listfile}[$i];
        if ( $line =~ /^\s*\#/ ) { next; } # this is a comment line
        if ( $line =~ /^\s*$/ ) { next; } # empty line
        $line =~ s/^\s*//;
        $line =~ s/\s*$//;
        push(@allpackages, $line);
    }

    return \@allpackages;
}

###########################################################
# Getting one section from a file. Section begins with
# [xyz] and ends with file end or next [abc].
###########################################################

sub get_section_from_file
{
    my ($file, $sectionname) = @_;

    my @section = ();
    my $record = 0;

    for ( my $i = 0; $i <= $#{$file}; $i++ )
    {
        my $line = ${$file}[$i];

        if (( $record ) && ( $line =~ /^\s*\[/ ))
        {
            $record = 0;
            last;
        }

        if ( $line =~ /^\s*\[\Q$sectionname\E\]\s*$/ ) { $record = 1; }

        if ( $line =~ /^\s*\[/ ) { next; } # this is a section line
        if ( $line =~ /^\s*\#/ ) { next; } # this is a comment line
        if ( $line =~ /^\s*$/ ) { next; }  # empty line
        $line =~ s/^\s*//;
        $line =~ s/\s*$//;
        if ( $record ) { push(@section, $line); }
    }

    return \@section;

}

#######################################################
# Substituting one variable in the xml file
#######################################################

sub replace_one_dollar_variable
{
    my ($file, $variable, $searchstring) = @_;

    for ( my $i = 0; $i <= $#{$file}; $i++ )
    {
        ${$file}[$i] =~ s/\$\{$searchstring\}/$variable/g;
    }
}

#######################################################
# Substituting the variables in the xml file
#######################################################

sub substitute_dollar_variables
{
    my ($file, $variableshashref) = @_;

    my $key;

    foreach $key (keys %{$variableshashref})
    {
        my $value = $variableshashref->{$key};
        replace_one_dollar_variable($file, $value, $key);
    }
}

#############################################################################
# Collecting all packages or rpms located in the installation directory
#############################################################################

sub get_all_packages_in_installdir
{
    my ($directory) = @_;

    my $infoline = "";

    my @allpackages = ();
    my $allpackages = \@allpackages;

    if ( $installer::globals::islinuxrpmbuild )
    {
        $allpackages = installer::systemactions::find_file_with_file_extension("rpm", $directory);
    }

    if ( $installer::globals::issolarisbuild )
    {
        $allpackages = installer::systemactions::get_all_directories($directory);
    }

    return $allpackages;
}

###############################################################
# The list of exclude packages can contain the
# beginning of the package name, not the complete name.
###############################################################

sub is_matching
{
    my ($onepackage, $allexcludepackages ) = @_;

    my $matches = 0;

    for ( my $i = 0; $i <= $#{$allexcludepackages}; $i++ )
    {
        my $oneexcludepackage = ${$allexcludepackages}[$i];

        if ( $onepackage =~ /^\s*$oneexcludepackage/ )
        {
            $matches = 1;
            last;
        }
    }

    return $matches;
}

###############################################################
# Copying all Solaris packages or RPMs from installation set
###############################################################

sub copy_all_packages
{
    my ($allexcludepackages, $sourcedir, $destdir) = @_;

    my $infoline = "";

    $sourcedir =~ s/\/\s*$//;
    $destdir =~ s/\/\s*$//;

    # $allexcludepackages is a list of RPMs and packages, that shall NOT be included into jds product

    my $allpackages = get_all_packages_in_installdir($sourcedir);

    for ( my $i = 0; $i <= $#{$allpackages}; $i++ )
    {
        my $onepackage = ${$allpackages}[$i];

        my $packagename = $onepackage;

        if ( $installer::globals::issolarispkgbuild )   # on Solaris $onepackage contains the complete path
        {
            installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$packagename);
        }

        if ( ! installer::existence::exists_in_array($packagename, $allexcludepackages))
        {
            if ( ! is_matching($packagename, $allexcludepackages ) )
            {

                if ( $installer::globals::islinuxrpmbuild )
                {
                    my $sourcepackage = $sourcedir . $installer::globals::separator . $packagename;
                    my $destfile = $destdir . $installer::globals::separator . $packagename;
                    if ( ! -f $sourcepackage ) { installer::exiter::exit_program("ERROR: Could not find RPM $sourcepackage!", "copy_all_packages"); }
                    installer::systemactions::hardlink_one_file($sourcepackage, $destfile);
                }

                if ( $installer::globals::issolarispkgbuild )
                {
                    my $destinationdir = $destdir . $installer::globals::separator . $packagename;
                    if ( ! -d $onepackage ) { installer::exiter::exit_program("ERROR: Could not find Solaris package $onepackage!", "copy_all_packages"); }
                    installer::systemactions::hardlink_complete_directory($onepackage, $destinationdir);
                }
            }
            else
            {
                $infoline = "Excluding package (matching): $onepackage\n";
                push( @installer::globals::logfileinfo, $infoline);
            }
        }
        else
        {
            $infoline = "Excluding package (precise name): $onepackage\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }
}

######################################################
# Making systemcall
######################################################

sub make_systemcall
{
    my ($systemcall) = @_;

    my $returnvalue = system($systemcall);

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Success: Executed \"$systemcall\" successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
}

###########################################################
# Copying all Solaris packages or RPMs from solver
###########################################################

sub copy_additional_packages
{
    my ($allcopypackages, $destdir, $includepatharrayref) = @_;

    my $infoline = "Copy additional packages into installation set.\n";
    push( @installer::globals::logfileinfo, $infoline);

    $destdir =~ s/\/\s*$//;

    for ( my $i = 0; $i <= $#{$allcopypackages}; $i++ )
    {
        my $onepackage = ${$allcopypackages}[$i];
        $infoline = "Copy package: $onepackage\n";
        push( @installer::globals::logfileinfo, $infoline);

        # this package must be delivered into the solver

        my $packagesourceref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$onepackage, $includepatharrayref, 0);
        if ($$packagesourceref eq "") { installer::exiter::exit_program("ERROR: Could not find jds file $onepackage!", "copy_additional_packages"); }

        my $destfile = $destdir . $installer::globals::separator . $onepackage;
        installer::systemactions::copy_one_file($$packagesourceref, $destfile);

        if (( $installer::globals::issolarispkgbuild ) && ( $onepackage =~ /\.tar\.gz\s*$/ ))
        {
            my $systemcall = "cd $destdir; cat $onepackage | gunzip | tar -xf -";
            make_systemcall($systemcall);

            # deleting the tar.gz files
            $systemcall = "cd $destdir; rm -f $onepackage";
            make_systemcall($systemcall);
        }
    }
}

###########################################################
# Creating jds installation sets
###########################################################

sub create_jds_sets
{
    my ($installationdir, $allvariableshashref, $languagestringref, $languagesarrayref, $includepatharrayref) = @_;

    installer::logger::print_message( "\n******************************************\n" );
    installer::logger::print_message( "... creating jds installation set ...\n" );
    installer::logger::print_message( "******************************************\n" );

    installer::logger::include_header_into_logfile("Creating jds installation sets:");

    # special handling for unix multi language installation sets
    if ( $installer::globals::is_unix_multi ) { $languagestringref = \$installer::globals::unixmultipath; }

    my $firstdir = $installationdir;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$firstdir);

    my $lastdir = $installationdir;
    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$lastdir);

    if ( $lastdir =~ /\./ ) { $lastdir =~ s/\./_jds_inprogress\./ }
    else { $lastdir = $lastdir . "_jds_inprogress"; }

    # removing existing directory "_native_packed_inprogress" and "_native_packed_witherror" and "_native_packed"

    my $jdsdir = $firstdir . $lastdir;
    if ( -d $jdsdir ) { installer::systemactions::remove_complete_directory($jdsdir); }

    my $olddir = $jdsdir;
    $olddir =~ s/_inprogress/_witherror/;
    if ( -d $olddir ) { installer::systemactions::remove_complete_directory($olddir); }

    $olddir = $jdsdir;
    $olddir =~ s/_inprogress//;
    if ( -d $olddir ) { installer::systemactions::remove_complete_directory($olddir); }

    # creating the new directory

    installer::systemactions::create_directory($jdsdir);

    $installer::globals::saveinstalldir = $jdsdir;

    # find and read jds files list
    my $filelistname = $installer::globals::jdsexcludefilename;

    # File has to be located next to the package list
    my $path = $installer::globals::packagelist;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$path);
    $filelistname = $path . $filelistname;

    if ( ! -f $filelistname ) { installer::exiter::exit_program("ERROR: Could not find jds list file $filelistname!", "create_jds_sets"); }
    my $listfile = installer::files::read_file($filelistname);

    my $infoline = "Found jds list file: $filelistname \n";
    push( @installer::globals::logfileinfo, $infoline);

    # substituting the variables
    substitute_dollar_variables($listfile, $allvariableshashref);

    # determining the packages/RPMs to copy
    # my $allexcludepackages = get_all_files_from_filelist($listfile, "excludefiles");
    my $allexcludepackages = get_section_from_file($listfile, "excludefiles");
    my $allcopypackages = get_section_from_file($listfile, "copyfiles");

    # determining the source directory
    my $alldirs = installer::systemactions::get_all_directories($installationdir);
    my $sourcedir = ${$alldirs}[0]; # there is only one directory

    # copy all packages/RPMs
    copy_all_packages($allexcludepackages, $sourcedir, $jdsdir);
    copy_additional_packages($allcopypackages, $jdsdir, $includepatharrayref);

    return $jdsdir;
}

#############################################################################
# Checking, whether this installation set contains the correct languages
#############################################################################

sub check_jds_language
{
    my ($allvariableshashref, $languagestringref) = @_;

    my $infoline = "";

    # languagesarrayref and $allvariableshashref->{'JDSLANG'}

    if ( ! $allvariableshashref->{'JDSLANG'} ) { installer::exiter::exit_program("ERROR: For building JDS installation sets \"JDSLANG\" must be defined!", "check_jds_language"); }
    my $languagestring = $allvariableshashref->{'JDSLANG'};

    my $sortedarray1 = installer::converter::convert_stringlist_into_array(\$languagestring, ",");

    installer::sorter::sorting_array_of_strings($sortedarray1);

    my $languagesstring = $$languagestringref;
    if ($installer::globals::is_unix_multi) { $languagestring = $installer::globals::unixmultipath; }

    my $sortedarray2 = installer::converter::convert_stringlist_into_array(\$languagestring, "_");
    installer::sorter::sorting_array_of_strings($sortedarray2);

    my $string1 = installer::converter::convert_array_to_comma_separated_string($sortedarray1);
    my $string2 = installer::converter::convert_array_to_comma_separated_string($sortedarray2);

    my $arrays_are_equal = compare_arrays($sortedarray1, $sortedarray2);

    return $arrays_are_equal;
}

###################################################################################
# Comparing two arrays. The arrays are equal, if the complete content is equal.
###################################################################################

sub compare_arrays
{
    my ($array1, $array2) = @_;

    my $arrays_are_equal = 1;

    # checking the size

    if ( ! ( $#{$array1} == $#{$array2} )) { $arrays_are_equal = 0; }   # different size

    if ( $arrays_are_equal ) # only make further investigations if size is equal
    {
        for ( my $i = 0; $i <= $#{$array1}; $i++ )
        {
            # ingnoring whitespaces at end and beginning
            ${$array1}[$i] =~ s/^\s*//;
            ${$array2}[$i] =~ s/^\s*//;
            ${$array1}[$i] =~ s/\s*$//;
            ${$array2}[$i] =~ s/\s*$//;

            if ( ! ( ${$array1}[$i] eq ${$array2}[$i] ))
            {
                $arrays_are_equal = 0;
                last;
            }
        }
    }

    return $arrays_are_equal;
}

#################################################################
# Copying the files defined as ScpActions into the
# installation set.
#################################################################

sub put_scpactions_into_installset
{
    my ($installdir) = @_;

    installer::logger::include_header_into_logfile("Start: Copying scp action files into installation set");

    for ( my $i = 0; $i <= $#installer::globals::allscpactions; $i++ )
    {
        my $onescpaction = $installer::globals::allscpactions[$i];

        my $subdir = "";
        if ( $onescpaction->{'Subdir'} ) { $subdir = $onescpaction->{'Subdir'}; }

        if ( $onescpaction->{'Name'} eq "loader.exe" ) { next; }    # do not copy this ScpAction loader

        my $destdir = $installdir;
        $destdir =~ s/\Q$installer::globals::separator\E\s*$//;
        if ( $subdir ) { $destdir = $destdir . $installer::globals::separator . $subdir; }
        installer::systemactions::create_directory($destdir);

        my $sourcefile = $onescpaction->{'sourcepath'};
        my $destfile = $destdir . $installer::globals::separator . $onescpaction->{'DestinationName'};

        installer::systemactions::copy_one_file($sourcefile, $destfile);
    }

    installer::logger::include_header_into_logfile("End: Copying scp action files into installation set");

}

#################################################################
# Collecting scp actions for all languages
#################################################################

sub collect_scpactions
{
    my ($allscpactions) = @_;

    for ( my $i = 0; $i <= $#{$allscpactions}; $i++ )
    {
        push(@installer::globals::allscpactions, ${$allscpactions}[$i]);
    }
}

1;
